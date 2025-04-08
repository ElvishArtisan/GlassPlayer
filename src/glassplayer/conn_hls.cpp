// conn_hls.cpp
//
// Server connector for HTTP live streams (HLS).
//
//   (C) Copyright 2014-2025 Fred Gleason <fredg@paravelsystems.com>
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License version 2 as
//   published by the Free Software Foundation.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public
//   License along with this program; if not, write to the Free Software
//   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//

#include <QByteArray>
#include <QStringList>

#include <tbytevector.h>

#include "codec.h"
#include "conn_hls.h"
#include "logging.h"

#ifdef CONN_HLS_DUMP_SEGMENTS
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif  // CONN_HLS_DUMP_SEGMENTS

size_t __Hls_WriteCallback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
  QByteArray *data=(QByteArray *)userdata;

  *data+=QByteArray(ptr,size*nmemb);

  return size*nmemb;
}


Hls::Hls(const QString &mimetype,QObject *parent)
  : Connector(mimetype,parent)
{
#ifdef CONN_HLS_DUMP_SEGMENTS
  hls_segment_fd=-1;
#endif  // CONN_HLS_DUMP_SEGMENTS

  if((hls_curl_handle=curl_easy_init())==NULL) {
    Log(LOG_ERR,"curl initialization failed");
    exit(GLASS_EXIT_GENERAL_ERROR);
  }

  //
  // Index Processor
  //
  hls_index_playlist=new M3uPlaylist();
  hls_id3_parser=new Id3Parser();
  connect(hls_id3_parser,SIGNAL(tagReceived(uint64_t,Id3Tag *)),
	  this,SLOT(tagReceivedData(uint64_t,Id3Tag *)));
  hls_index_timer=new QTimer(this);
  hls_index_timer->setSingleShot(true);
  connect(hls_index_timer,SIGNAL(timeout()),this,SLOT(indexProcessStartData()));

  //
  // Media Processor
  //
  hls_download_average=new MeterAverage(3);

  hls_media_timer=new QTimer(this);
  hls_media_timer->setSingleShot(true);
  connect(hls_media_timer,SIGNAL(timeout()),this,SLOT(mediaProcessStartData()));
}


Hls::~Hls()
{
  delete hls_media_timer;
  delete hls_index_timer;
  delete hls_index_playlist;
}


Connector::ServerType Hls::serverType() const
{
  return Connector::HlsServer;
}


void Hls::reset()
{
}


void Hls::connectToHostConnector()
{
  hls_index_url=serverUrl();
  hls_index_timer->start(0);
}


void Hls::disconnectFromHostConnector()
{
}


void Hls::loadStats(QStringList *hdrs,QStringList *values,bool is_first)
{
  if(is_first) {
    hdrs->push_back("Connector|Type");
    values->push_back("HLS");

    hdrs->push_back("Connector|Server");
    values->push_back(hls_server);

    hdrs->push_back("Connector|ContentType");
    values->push_back(hls_content_type);
  }

  hdrs->push_back("Connector|Download Speed");
  values->push_back(QString::asprintf("%7.0f kbit/sec",
			 hls_download_average->average()/1000.0).trimmed());

  hdrs->push_back("Connector|HLS Version");
  values->push_back(QString::asprintf("%d",hls_index_playlist->version()));

  hdrs->push_back("Connector|HLS Target Duration");
  values->
    push_back(QString::asprintf("%d",hls_index_playlist->targetDuration()));

  hdrs->push_back("Connector|HLS Media Sequence");
  values->
    push_back(QString::asprintf("%d",hls_index_playlist->mediaSequence()));

  hdrs->push_back("Connector|HLS Segment Quantity");
  values->
    push_back(QString::asprintf("%u",hls_index_playlist->segmentQuantity()));

  for(unsigned i=0;i<hls_index_playlist->segmentQuantity();i++) {
    if(!hls_index_playlist->segmentTitle(i).isEmpty()) {
      hdrs->push_back(QString::asprintf("Connector|HLS Segment%u Title",i+1));
      values->push_back(hls_index_playlist->segmentTitle(i));
    }
    hdrs->push_back(QString::asprintf("Connector|HLS Segment%u Url",i+1));
    values->push_back(hls_index_playlist->segmentUrl(i).toString());

    hdrs->push_back(QString::asprintf("Connector|HLS Segment%u Duration",i+1));;
    values->push_back(QString::asprintf("%8.5lf",
			       hls_index_playlist->segmentDuration(i)));

    if(hls_index_playlist->segmentDateTime(i).isValid()) {
      hdrs->push_back(QString::asprintf("Connector|HLS Segment%u DateTime",i+1));
      values->push_back(hls_index_playlist->segmentDateTime(i).
			toString("yyyy-mm-dd hh::mm:ss"));
    }
  }
}


void Hls::tagReceivedData(uint64_t bytes,Id3Tag *tag)
{
  TagLib::ID3v2::FrameList frames=tag->frameList();
  bool initialize=false;

  for(unsigned i=0;i<frames.size();i++) {
    TagLib::ByteVector raw_bytes=frames[i]->frameID();
    QString id(QByteArray(raw_bytes.data(),raw_bytes.size()).constData());
    QString str=QString::fromUtf8(frames[i]->toString().toCString(true));
    if((id=="PRIV")&&(str=="com.apple.streaming.transportStreamTimestamp")) {
      return;
    }
  }

  if(hls_meta_event.isEmpty()) {
    initialize=true;
  }
  hls_meta_event.clear();
  for(unsigned i=0;i<frames.size();i++) {
    TagLib::ByteVector raw_bytes=frames[i]->frameID();
    QString id(QByteArray(raw_bytes.data(),raw_bytes.size()).constData());
    QString str=QString::fromUtf8(frames[i]->toString().toCString(true));
    hls_meta_event.setField(id,str);
    if(initialize) {
      setMetadataField(0,id,str);
    }
  }

  emit metadataReceived(bytes,&hls_meta_event);
}


void Hls::indexProcessStartData()
{
  Log(LOG_DEBUG,QString::asprintf("downloading \"%s\"",
				  hls_index_url.toDisplayString().
				  toUtf8().constData()));

  long resp_code=0;
  QByteArray data;
  char curl_errs[CURL_ERROR_SIZE];

  curl_easy_reset(hls_curl_handle);

  //
  // Authentication
  //
  if((!serverUsername().isEmpty())||(!serverPassword().isEmpty())) {
    curl_easy_setopt(hls_curl_handle,CURLOPT_USERPWD,
		     (serverUsername()+":"+serverPassword()).
		     toUtf8().constData());
  }

  //
  // Transaction
  //
  curl_easy_setopt(hls_curl_handle,CURLOPT_ERRORBUFFER,curl_errs);
  curl_easy_setopt(hls_curl_handle,CURLOPT_WRITEFUNCTION,__Hls_WriteCallback);
  curl_easy_setopt(hls_curl_handle,CURLOPT_HEADER,1);
  curl_easy_setopt(hls_curl_handle,CURLOPT_WRITEDATA,&data);
  curl_easy_setopt(hls_curl_handle,CURLOPT_URL,
		   hls_index_url.toEncoded().constData());
  curl_easy_setopt(hls_curl_handle,CURLOPT_FOLLOWLOCATION,1);
  curl_easy_setopt(hls_curl_handle,CURLOPT_USERAGENT,GLASSPLAYER_USER_AGENT);
  CURLcode code=curl_easy_perform(hls_curl_handle);
  if(code==CURLE_OK) {
    curl_easy_getinfo(hls_curl_handle,CURLINFO_RESPONSE_CODE,&resp_code);
    if(((resp_code<200)||(resp_code>=300))&&(resp_code!=0)) {
      Log(LOG_WARNING,
	  QString::asprintf("download of \"%s\" returned code %lu",
	  hls_index_url.toDisplayString().toUtf8().constData(),resp_code));
      return;
    }
  }
  else {
    Log(LOG_WARNING,
	QString::asprintf("download of \"%s\" failed: %s",
			  hls_index_url.toDisplayString().toUtf8().constData(),
			  curl_errs));
    return;
  }

  data=ReadHeaders(data);
  M3uPlaylist *playlist=new M3uPlaylist();
  if(playlist->parse(data,hls_index_url)) {
    if(playlist->isMaster()) {  // Recurse to target playlist
      hls_index_url=playlist->target();
      hls_index_timer->start(0);
    }
    else {
      if(*playlist!=*hls_index_playlist) {
	*hls_index_playlist=*playlist;
	if(isConnected()||hls_index_playlist->segmentQuantity()>=3) {
	  hls_media_timer->start(0);
	}
	else {
	  if(global_log_verbose) {
	    Log(LOG_INFO,"waiting from stream to fill");
	  }
	}
	hls_index_timer->start(1000*hls_index_playlist->targetDuration());
      }
      else {
	hls_index_timer->start(1000);
      }
    }
  }
  else {
    Log(LOG_WARNING,"error parsing playlist");
  }
  delete playlist;
}


void Hls::mediaProcessStartData()
{
  QByteArray data;
  char curl_errs[CURL_ERROR_SIZE];

  //
  // Find next media segment
  //
  unsigned segno=0;
  for(unsigned i=0;i<hls_index_playlist->segmentQuantity();i++) {
    if(hls_index_playlist->segmentUrl(i)==hls_last_media_segment) {
      segno=i+1;
    }
  }
  if(segno<hls_index_playlist->segmentQuantity()) {
    hls_current_media_segment=hls_index_playlist->segmentUrl(segno);

#ifdef CONN_HLS_DUMP_SEGMENTS
    QString filename=
      CONN_HLS_DUMP_SEGMENTS+"/"+
      hls_current_media_segment.toString().split("/",QString::SkipEmptyParts).back();
    hls_segment_fd=open(filename.toUtf8(),O_CREAT|O_TRUNC|O_WRONLY,S_IRUSR|S_IWUSR);
    if(hls_segment_fd>=0) {
      fprintf(stderr,
	      "creating media segment: %s\n",filename.toUtf8().constData()); 
    }
    else {
      fprintf(stderr,"unable to open media segment: %s [%s]\n",
	      filename.toUtf8().constData(),strerror(errno)); 
    }
 #endif  // CONN_HLS_DUMP_SEGMENTS

    Log(LOG_DEBUG,QString::asprintf("downloading \"%s\"",
				    hls_current_media_segment.toDisplayString().
				    toUtf8().constData()));
    long resp_code=0;
    curl_easy_reset(hls_curl_handle);
    data.clear();

    //
    // Authentication
    //
    if((!serverUsername().isEmpty())||(!serverPassword().isEmpty())) {
      curl_easy_setopt(hls_curl_handle,CURLOPT_USERPWD,
		       (serverUsername()+":"+serverPassword()).
		       toUtf8().constData());
    }

    //
    // Transaction
    //
    curl_easy_setopt(hls_curl_handle,CURLOPT_ERRORBUFFER,curl_errs);
    curl_easy_setopt(hls_curl_handle,CURLOPT_WRITEFUNCTION,__Hls_WriteCallback);
    curl_easy_setopt(hls_curl_handle,CURLOPT_WRITEDATA,&data);
    curl_easy_setopt(hls_curl_handle,CURLOPT_URL,
		     hls_current_media_segment.toEncoded().constData());
    curl_easy_setopt(hls_curl_handle,CURLOPT_FOLLOWLOCATION,1);
    curl_easy_setopt(hls_curl_handle,CURLOPT_USERAGENT,GLASSPLAYER_USER_AGENT);
    hls_download_start_datetime=QDateTime::currentDateTime();
    CURLcode code=curl_easy_perform(hls_curl_handle);
    if(code==CURLE_OK) {
      curl_easy_getinfo(hls_curl_handle,CURLINFO_RESPONSE_CODE,&resp_code);
      if(((resp_code<200)||(resp_code>=300))&&(resp_code!=0)) {
	Log(LOG_WARNING,
	    QString::asprintf("download of \"%s\" returned code %lu",
			      hls_current_media_segment.toDisplayString().toUtf8().constData(),
			      resp_code));
	return;
      }
    }
    else {
      Log(LOG_WARNING,
      	  QString::asprintf("download of \"%s\" failed: %s",
      	      hls_current_media_segment.toDisplayString().toUtf8().constData(),
	      curl_errs));
    }
#ifdef CONN_HLS_DUMP_SEGMENTS
    if(hls_segment_fd>=0) {
      if(write(hls_segment_fd,hls_media_segment_data.data(),
	       hls_media_segment_data.size())<0) {
	fprintf(stderr,
		"  ERROR writing media segment data: %s\n",strerror(errno));
      }
      close(hls_segment_fd);
      hls_segment_fd=-1;
    }
#endif  // CONN_HLS_DUMP_SEGMENTS

    // *******************************************************************


    //
    // Calculate Download Speed
    //
    float msecs=
      hls_download_start_datetime.msecsTo(QDateTime::currentDateTime());
    float bytes_per_sec=(float)data.size()/(msecs/1000.0);
    hls_download_average->addValue(bytes_per_sec);

    //
    // Extract Timed Metadata
    //
    hls_id3_parser->parse(data);

    //
    // Forward Data
    //
    while(data.size()>4096) {
      emit dataReceived(data.left(4096),false);
      data.remove(0,4096);
    }
    emit dataReceived(data,false);

    if(!isConnected()) {
      QStringList f0=hls_current_media_segment.toString().split(".");
      for(int i=1;i<Codec::TypeLast;i++) {
	if(Codec::acceptsExtension((Codec::Type)i,f0[f0.size()-1])) {
	  setCodecType((Codec::Type)i);
	  setConnected(true);
	}
      }
      if(!isConnected()) {
	Log(LOG_ERR,tr("unsupported codec")+" ["+f0[f0.size()-1]+"]");
	exit(GLASS_EXIT_UNSUPPORTED_CODEC_ERROR);
      }
    }
    hls_last_media_segment=hls_current_media_segment;
    hls_current_media_segment="";
    hls_media_timer->start(0);
  }
}


QByteArray Hls::ReadHeaders(QByteArray &data)
{
  QString line;
  unsigned used=0;

  for(int i=0;i<data.size();i++) {
    switch(0xFF&data.constData()[i]) {
    case 13:
      break;

    case 10:
      if(line.isEmpty()) {
	return data.right(data.length()-used);
      }
      ProcessHeader(line);
      line="";
      break;

    default:
      line+=data.constData()[i];
      break;
    }
    used++;
  }
  return QByteArray();
}


void Hls::ProcessHeader(const QString &str)
{
  QStringList f0=str.split(":");

  if(f0.size()>=2) {
    QString hdr=f0[0].toLower().trimmed();
    f0.erase(f0.begin());
    QString value=f0.join(":").trimmed();

    if(hdr=="server") {
      hls_server=value;
    }
    if(hdr=="content-type") {
      hls_content_type=value;
    }
  }
}


void Hls::StopProcess(QProcess *proc)
{
  if((proc!=NULL)&&(proc->state()!=QProcess::NotRunning)) {
    proc->disconnect();
    proc->kill();
    proc->waitForFinished();
  }
}
