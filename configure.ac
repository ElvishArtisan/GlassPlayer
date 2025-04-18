dnl configure.ac
dnl
dnl Autoconf configuration
dnl
dnl Use autoconf to process this into a configure script
dnl
dnl   (C) Copyright 2014-2025 Fred Gleason <fredg@paravelsystems.com>
dnl
dnl   This program is free software; you can redistribute it and/or modify
dnl   it under the terms of the GNU General Public License version 2 as
dnl   published by the Free Software Foundation.
dnl
dnl   This program is distributed in the hope that it will be useful,
dnl   but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl   GNU General Public License for more details.
dnl
dnl   You should have received a copy of the GNU General Public
dnl   License along with this program; if not, write to the Free Software
dnl   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
dnl

AC_INIT(glassplayer,m4_esyscmd(cat PACKAGE_VERSION))
AM_INIT_AUTOMAKE
AC_SUBST(RPM_RELEASE,1)
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

#
# Basic Compiler Checks
#
AC_PROG_CXX
AC_LANG(C++)

# to avoid rpath usage :
# http://wiki.debian.net/index.cgi?RpathIssue
case ${host} in
  *-pc-linux-gnu)
    AC_MSG_RESULT([Fixing libtool for -rpath problems.])
    sed < libtool > libtool-2 \
    's/^hardcode_libdir_flag_spec.*$/hardcode_libdir_flag_spec=" -D__LIBTOOL_IS_A_FOOL__ "/'
    mv libtool-2 libtool
    chmod 755 libtool
  ;;
esac

#
# Option Switches
#
AC_ARG_ENABLE(libmad,[  --disable-libmad        disable MPEG-1 support],
		      [LIBMAD_DISABLED=yes],[])
AC_ARG_ENABLE(fdkaac,[  --disable-fdkaac        disable MPEG-2/4 support],
	      [FDKAAC_DISABLED=yes],[]) 
AC_ARG_ENABLE(ogg,[  --disable-ogg           disable Ogg Opus/Vorbis support],
	      [OGG_DISABLED=yes],[]) 
AC_ARG_ENABLE(alsa,[  --disable-alsa          disable ALSA sound support],
		      [ALSA_DISABLED=yes],[])
AC_ARG_ENABLE(asihpi,[  --disable-asihpi        disable AudioScience HPI sound support],
		      [ASIHPI_DISABLED=yes],[])
AC_ARG_ENABLE(file,[  --disable-file          disable FILE sound support],
		      [SNDFILE_DISABLED=yes],[])
AC_ARG_ENABLE(jack,[  --disable-jack          disable JACK sound support],
		      [JACK_DISABLED=yes],[])
AC_ARG_ENABLE(mme,[  --disable-mme           disable MME sound support],
		      [MME_DISABLED=yes],[])
AC_ARG_ENABLE(docbook,[  --disable-docbook       disable building of DocBook documentation],[DOCBOOK_DISABLED=yes],[])

#
# Check for Qt (Mandatory)
#
AQ_FIND_QT6(QT6,Core Gui Widgets Network,[],[AC_MSG_ERROR([*** Qt6 not found ***])])
#PKG_CHECK_MODULES(QT6CLI,Qt6Core Qt6Network,[],[AC_MSG_ERROR([*** Qt6 not found ***])])
#PKG_CHECK_MODULES(QT6GUI,Qt6Widgets Qt6Gui Qt6Core Qt6Network,[],[AC_MSG_ERROR([*** Qt6 not found ***])])
#AC_CHECK_PROG(MOC_NAME,moc-qt6,[moc-qt6],[moc])
#AC_SUBST(MOC,$MOC_NAME)

#
# Check for LibCurl
#
PKG_CHECK_MODULES(LIBCURL,libcurl,[],[AC_MSG_ERROR([*** LibCURL not found ***])])

#
# Check for libsndfile (needed for FILE support)
#
PKG_CHECK_MODULES(SNDFILE,sndfile,[SNDFILE_FOUND=y],[])
if test $SNDFILE_FOUND ; then
  if test -z $SNDFILE_DISABLED ; then
    AC_DEFINE(SNDFILE,yes)
    USING_SNDFILE=yes
  fi
fi

#
# Check for Secret Rabbit Code (Mandatory)
#
PKG_CHECK_MODULES(SAMPLERATE,samplerate,[SAMPLERATE_FOUND=y],[])

#
# Check for TagLib (Mandatory)
#
PKG_CHECK_MODULES(TAGLIB,taglib,[TAGLIB_FOUND=y],[])

#
# Check for LibTool libltdl
#
AC_CHECK_HEADER(ltdl.h,[],[AC_MSG_ERROR([*** Libtool ltdl not found ***])])

#
# Check for AudioScience HPI
#
AC_CHECK_HEADER(asihpi/hpi.h,[ASIHPI_FOUND=yes],[])
if test $ASIHPI_FOUND ; then
  if test -z $ASIHPI_DISABLED ; then
    AC_DEFINE(ASIHPI,yes)
    AC_SUBST(ASIHPI_LIBS,-lhpi)
    USING_ASIHPI=yes
  else
    AC_SUBST(ASIHPI_LIBS,"")
  fi
fi

#
# Check for ALSA
#
PKG_CHECK_MODULES(ALSA,alsa,[ALSA_FOUND=y],[PKGCONFIG_FAILED=y])
if test $ALSA_FOUND ; then
  if test -z $ALSA_DISABLED ; then
    AC_DEFINE(ALSA,yes)
    USING_ALSA=yes
  fi
fi

#
# Check for Jack Audio Connection Kit
#
PKG_CHECK_MODULES(JACK,jack,[JACK_FOUND=y],[JACK_NOT_FOUND=y])
if test $JACK_FOUND ; then
  if test -z $JACK_DISABLED ; then
    AC_DEFINE(JACK,yes)
    USING_JACK=yes
  fi
fi

#
# Check for Windows MME
#
AC_CHECK_HEADER(windows.h,[MME_FOUND=yes],[])
if test $MME_FOUND ; then
  if test -z $MME_DISABLED ; then
    AC_DEFINE(MME)
    AC_SUBST(MME_LIBS,-lwinmm)
    USING_MME=yes
  else
    AC_SUBST(MME_LIBS,"")
  fi
else
  AC_SUBST(MME_LIBS,"")
fi

#
# Check for Ogg and friends (Optional, required for Opus and Vorbis support)
#
PKG_CHECK_MODULES(OGG,ogg opus vorbis,[OGG_FOUND=y],[OGG_FAILED=y])
if test $OGG_FOUND ; then
  if test -z $OGG_DISABLED ; then
    AC_DEFINE(HAVE_OGG,yes)
    USING_OGG=yes
  fi
fi

#
# Check for libmad (Optional, required for MPEG-1 support)
#
AC_CHECK_HEADER(mad.h,[LIBMAD_FOUND=yes],[])
if test $LIBMAD_FOUND ; then
  if test -z $LIBMAD_DISABLED ; then
    AC_DEFINE(HAVE_LIBMAD)
    USING_LIBMAD=yes
  fi
fi

#
# Check for Fraunhofer FDK (Optional, required for MPEG-4 AAC-HE support)
#
AC_CHECK_HEADER(fdk-aac/aacenc_lib.h,[FDKAAC_FOUND=yes],[])
if test $FDKAAC_FOUND ; then
  if test -z $FDKAAC_DISABLED ; then
    AC_DEFINE(HAVE_FDKAAC)
    USING_FDKAAC=yes
  fi
fi

#
# Check for DocBook Toolchain
#
if test -z $DOCBOOK_DISABLED ; then
  USING_DOCBOOK=yes
fi
AM_CONDITIONAL([DOCBOOK_AM], [test "$USING_DOCBOOK" = yes])

#
# Determine the target architecture
#
AR_GCC_TARGET()
AC_SUBST(VENDOR,$ar_gcc_distro)
AC_SUBST(ARCH,$ar_gcc_arch)
if test $ar_gcc_arch = i686 ; then
  USING_WIN32=yes
  AC_DEFINE(WIN32)
  AC_SUBST(PLATFORM_LIBS,"")
  AC_SUBST(DEFAULT_AUDIO_DEVICE,AudioDevice::Mme)
  AC_SUBST(DEFAULT_AUDIO_DEVICE_NAME,MME)
else
  AC_SUBST(PLATFORM_LIBS,"-ldl -lpthread")
  AC_SUBST(DEFAULT_AUDIO_DEVICE,AudioDevice::Alsa)
  AC_SUBST(DEFAULT_AUDIO_DEVICE_NAME,ALSA)
fi

#
# Determine number of available CPU cores for building packages
#
AX_COUNT_CPUS([],[])
if test $CPU_COUNT -lt 2 ; then
   AC_SUBST(CPUS_AVAIL,1)
else
   if test $CPU_COUNT -gt 16 ; then
      AC_SUBST(CPUS_AVAIL,16)
   else
      AC_SUBST(CPUS_AVAIL,$(($CPU_COUNT-1)))
   fi
fi

#
# Determine Distro
#
AR_GET_DISTRO()
AC_SUBST(DISTRO,$ar_gcc_distro)

#
# Configure RPM Build
#
AC_CHECK_PROG(RPMBUILD_FOUND,rpmbuild,[yes],[])
if test -z $RPMBUILD_FOUND ; then
  AC_SUBST(RPMBUILD,rpm)
else
  AC_SUBST(RPMBUILD,rpmbuild)
fi
if test -d /usr/src/redhat ; then
  AC_SUBST(RPM_ROOT,/usr/src/redhat)
  AC_SUBST(VENDOR,redhat)
else
  AC_SUBST(RPM_ROOT,/usr/src/packages)
  AC_SUBST(VENDOR,suse)
fi 

AC_CONFIG_FILES([ \
    debian/Makefile \
    debian/source/Makefile \
    docs/glassplayer.xml \
    docs/Makefile \
    src/Makefile \
    src/common/glasslimits.h \
    src/common/Makefile \
    src/glassplayer/Makefile \
    src/glassplayergui/Makefile \
    glassplayer.spec \
    build_debs.sh \
    Makefile ])
AC_OUTPUT()
chmod 755 build_debs.sh

#
# Link Common Elements
#
rm -f src/glassplayer/cmdswitch.cpp
ln -s ../../src/common/cmdswitch.cpp src/glassplayer/cmdswitch.cpp
rm -f src/glassplayer/cmdswitch.h
ln -s ../../src/common/cmdswitch.h src/glassplayer/cmdswitch.h
rm -f src/glassplayergui/cmdswitch.cpp
ln -s ../../src/common/cmdswitch.cpp src/glassplayergui/cmdswitch.cpp
rm -f src/glassplayergui/cmdswitch.h
ln -s ../../src/common/cmdswitch.h src/glassplayergui/cmdswitch.h

rm -f src/glassplayer/codec.cpp
ln -s ../../src/common/codec.cpp src/glassplayer/codec.cpp
rm -f src/glassplayer/codec.h
ln -s ../../src/common/codec.h src/glassplayer/codec.h
rm -f src/glassplayergui/codec.cpp
ln -s ../../src/common/codec.cpp src/glassplayergui/codec.cpp
rm -f src/glassplayergui/codec.h
ln -s ../../src/common/codec.h src/glassplayergui/codec.h

rm -f src/glassplayer/connector.cpp
ln -s ../../src/common/connector.cpp src/glassplayer/connector.cpp
rm -f src/glassplayer/connector.h
ln -s ../../src/common/connector.h src/glassplayer/connector.h
rm -f src/glassplayergui/connector.cpp
ln -s ../../src/common/connector.cpp src/glassplayergui/connector.cpp
rm -f src/glassplayergui/connector.h
ln -s ../../src/common/connector.h src/glassplayergui/connector.h

rm -f src/glassplayer/logging.cpp
ln -s ../../src/common/logging.cpp src/glassplayer/logging.cpp
rm -f src/glassplayer/logging.h
ln -s ../../src/common/logging.h src/glassplayer/logging.h
rm -f src/glassplayergui/logging.cpp
ln -s ../../src/common/logging.cpp src/glassplayergui/logging.cpp
rm -f src/glassplayergui/logging.h
ln -s ../../src/common/logging.h src/glassplayergui/logging.h

rm -f src/glassplayer/metaevent.cpp
ln -s ../../src/common/metaevent.cpp src/glassplayer/metaevent.cpp
rm -f src/glassplayer/metaevent.h
ln -s ../../src/common/metaevent.h src/glassplayer/metaevent.h
rm -f src/glassplayergui/metaevent.cpp
ln -s ../../src/common/metaevent.cpp src/glassplayergui/metaevent.cpp
rm -f src/glassplayergui/metaevent.h
ln -s ../../src/common/metaevent.h src/glassplayergui/metaevent.h

rm -f src/glassplayer/ringbuffer.cpp
ln -s ../../src/common/ringbuffer.cpp src/glassplayer/ringbuffer.cpp
rm -f src/glassplayer/ringbuffer.h
ln -s ../../src/common/ringbuffer.h src/glassplayer/ringbuffer.h
rm -f src/glassplayergui/ringbuffer.cpp
ln -s ../../src/common/ringbuffer.cpp src/glassplayergui/ringbuffer.cpp
rm -f src/glassplayergui/ringbuffer.h
ln -s ../../src/common/ringbuffer.h src/glassplayergui/ringbuffer.h

rm -f src/glassplayer/audiodevice.cpp
ln -s ../../src/common/audiodevice.cpp src/glassplayer/audiodevice.cpp
rm -f src/glassplayer/audiodevice.h
ln -s ../../src/common/audiodevice.h src/glassplayer/audiodevice.h
rm -f src/glassplayergui/audiodevice.cpp
ln -s ../../src/common/audiodevice.cpp src/glassplayergui/audiodevice.cpp
rm -f src/glassplayergui/audiodevice.h
ln -s ../../src/common/audiodevice.h src/glassplayergui/audiodevice.h

rm -f src/glassplayer/glasslimits.h
ln -s ../../src/common/glasslimits.h src/glassplayer/glasslimits.h
rm -f src/glassplayergui/glasslimits.h
ln -s ../../src/common/glasslimits.h src/glassplayergui/glasslimits.h


AC_MSG_NOTICE()
AC_MSG_NOTICE("|-----------------------------------------------------|")
AC_MSG_NOTICE("|      *** GLASSPLAYER CONFIGURATION SUMMARY ***      |")
AC_MSG_NOTICE("|-----------------------------------------------------|")
AC_MSG_NOTICE("| Decoding Formats:                                   |")
if test -z $USING_LIBMAD ; then
AC_MSG_NOTICE("|                       MPEG-1 (via LibMad) ... No    |")
else
AC_MSG_NOTICE("|                       MPEG-1 (via LibMad) ... Yes   |")
fi
if test -z $USING_FDKAAC ; then
AC_MSG_NOTICE("|               MPEG-4 HE-AAC+ (via FDKAAC) ... No    |")
else
AC_MSG_NOTICE("|               MPEG-4 HE-AAC+ (via FDKAAC) ... Yes   |")
fi
if test -z $USING_OGG ; then
AC_MSG_NOTICE("|         Ogg Opus (via libogg and libopus) ... No    |")
AC_MSG_NOTICE("|     Ogg Vorbis (via libogg and libvorbis) ... No    |")
else
AC_MSG_NOTICE("|         Ogg Opus (via libogg and libopus) ... Yes   |")
AC_MSG_NOTICE("|     Ogg Vorbis (via libogg and libvorbis) ... Yes   |")
fi
#if test -z $USING_SNDFILE ; then
#AC_MSG_NOTICE("|                    PCM16 (via libsndfile) ... No    |")
#else
#AC_MSG_NOTICE("|                    PCM16 (via libsndfile) ... Yes   |")
#fi
AC_MSG_NOTICE("|                                                     |")
AC_MSG_NOTICE("| Audio Outputs:                                      |")
if test -z $USING_ALSA ; then
AC_MSG_NOTICE("|  Advanced Linux Sound Architecture (ALSA) ... No    |")
else
AC_MSG_NOTICE("|  Advanced Linux Sound Architecture (ALSA) ... Yes   |")
fi
#if test -z $USING_ASIHPI ; then
#AC_MSG_NOTICE("|                 AudioScience HPI (ASIHPI) ... No    |")
#else
#AC_MSG_NOTICE("|                 AudioScience HPI (ASIHPI) ... Yes   |")
#fi
if test -z $USING_SNDFILE ; then
AC_MSG_NOTICE("|                       Save to File (FILE) ... No    |")
else
AC_MSG_NOTICE("|                       Save to File (FILE) ... Yes   |")
fi
if test -z $USING_JACK ; then
AC_MSG_NOTICE("|          Jack Audio Connection Kit (JACK) ... No    |")
else
AC_MSG_NOTICE("|          Jack Audio Connection Kit (JACK) ... Yes   |")
fi
if test -z $USING_MME ; then
AC_MSG_NOTICE("|       Windows Multimedia Extensions (MME) ... No    |")
else
AC_MSG_NOTICE("|       Windows Multimedia Extensions (MME) ... Yes   |")
fi
AC_MSG_NOTICE("|                  Standard Output (STDOUT) ... Yes   |")
AC_MSG_NOTICE("|                                                     |")
AC_MSG_NOTICE("| Miscellaneous:                                      |")
if test -z $USING_WIN32 ; then
AC_MSG_NOTICE("|                                  Platform ... POSIX |")
else
AC_MSG_NOTICE("|                                  Platform ... Win32 |")
fi
if test -z $USING_DOCBOOK ; then
AC_MSG_NOTICE("|                     DocBook Documentation ... No    |")
else
AC_MSG_NOTICE("|                     DocBook Documentation ... Yes   |")
fi
AC_MSG_NOTICE("|-----------------------------------------------------|")
AC_MSG_NOTICE()
AC_MSG_NOTICE()
AC_MSG_NOTICE(Now enter 'make' to build the software.)
AC_MSG_NOTICE()
