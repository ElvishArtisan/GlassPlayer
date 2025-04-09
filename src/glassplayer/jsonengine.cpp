// jsonengine.cpp
//
// JSON update generator
//
//   (C) Copyright 2019-2025 Fred Gleason <fredg@paravelsystems.com>
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

#include <QJsonDocument>
#include <QJsonObject>

#include "jsonengine.h"

JsonEngine::JsonEngine()
{
}


void JsonEngine::addEvent(const QString &str)
{
  QStringList f0=str.split("|",Qt::KeepEmptyParts);
  QString key=f0.at(0);
  f0.removeFirst();
  QStringList f1=f0.join("|").split(":",Qt::KeepEmptyParts);
  for(int i=2;i<f1.size();i++) {
    f1[1]+=":"+f1.at(i);
  }
  for(int i=2;i<f1.size();i++) {
    f1.removeLast();
  }
  f1[1]=f1.at(1).trimmed();
  json_events.insert(key,f1);
}


void JsonEngine::addEvents(const QString &str)
{
  QStringList f0=str.split("\n",Qt::SkipEmptyParts);
  for(int i=0;i<f0.size();i++) {
    addEvent(f0.at(i));
  }
}


QString JsonEngine::generate() const
{
  QJsonDocument jdoc;

  if(json_events.size()>0) {
    QJsonObject jo0;
    QJsonObject jo1;
    for(QMultiMap<QString,QStringList>::const_iterator it=json_events.begin();
	it!=json_events.end();it++) {
      if(it.value().size()==2) {
	jo0.insert(it.value().at(0),it.value().at(1));
      }
      jo1.insert(it.key(),jo0);
    }
    jdoc.setObject(jo1);
  }

  return QString::fromUtf8(jdoc.toJson());
}


void JsonEngine::clear()
{
  json_events.clear();
}


QString JsonEngine::escape(const QString &str)
{
  QString ret="";

  for(int i=0;i<str.length();i++) {
    QChar c=str.at(i);
    switch(c.category()) {
    case QChar::Other_Control:
      ret+=QString::asprintf("\\u%04X",c.unicode());
      break;

    default:
      switch(c.unicode()) {
      case 0x22:   // Quote
	ret+="\\\"";
	break;

      case 0x5C:   // Backslash
	ret+="\\\\";
	break;

      default:
	ret+=c;
	break;
      }
      break;
    }
  }

  return ret;
}
