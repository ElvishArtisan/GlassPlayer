## Makefile.am
##
## Makefile for the glassplayergui(1) Audio Player front-end.
##
## (C) Copyright 2016-2025 Fred Gleason <fredg@paravelsystems.com>
##
##   This program is free software; you can redistribute it and/or modify
##   it under the terms of the GNU General Public License version 2 as
##   published by the Free Software Foundation.
##
##   This program is distributed in the hope that it will be useful,
##   but WITHOUT ANY WARRANTY; without even the implied warranty of
##   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##   GNU General Public License for more details.
##
##   You should have received a copy of the GNU General Public
##   License along with this program; if not, write to the Free Software
##   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
##
##
## Use automake to process this into a Makefile.in

AM_CPPFLAGS = -Wall -DPREFIX=\"$(prefix)\" -Wno-strict-aliasing @QT6_CFLAGS@ @SAMPLERATE_CFLAGS@ -std=c++17 -fPIC
MOC = @QT6_MOC@

# The dependency for qt's Meta Object Compiler (moc)
moc_%.cpp:	%.h
	$(MOC) $< -o $@

bin_PROGRAMS = glassplayergui

dist_glassplayergui_SOURCES = combobox.cpp combobox.h\
                              glassplayergui.cpp glassplayergui.h\
                              jsonparser.cpp jsonparser.h\
                              playmeter.cpp playmeter.h\
                              segmeter.cpp segmeter.h\
                              statsdialog.cpp statsdialog.h\
                              statspanel.cpp statspanel.h

nodist_glassplayergui_SOURCES = audiodevice.cpp audiodevice.h\
                                cmdswitch.cpp cmdswitch.h\
                                codec.cpp codec.h\
                                connector.cpp connector.h\
                                glasslimits.h\
                                logging.cpp logging.h\
                                metaevent.cpp metaevent.h\
                                moc_audiodevice.cpp\
                                moc_codec.cpp\
                                moc_combobox.cpp\
                                moc_connector.cpp\
                                moc_glassplayergui.cpp\
                                moc_jsonparser.cpp\
                                moc_playmeter.cpp\
                                moc_segmeter.cpp\
                                moc_statsdialog.cpp\
                                moc_statspanel.cpp\
                                ringbuffer.cpp ringbuffer.h

glassplayergui_LDADD = @QT6_LIBS@ @SAMPLERATE_LIBS@

CLEANFILES = *~\
             moc_*\
             *.obj\
             *.idb\
             *.pdb\
             *ilk

DISTCLEANFILES = audiodevice.cpp audiodevice.h\
                 cmdswitch.cpp cmdswitch.h\
                 codec.cpp codec.h\
                 connector.cpp connector.h\
                 glasslimits.h\
                 logging.cpp logging.h\
                 metaevent.cpp metaevent.h\
                 ringbuffer.cpp ringbuffer.h

MAINTAINERCLEANFILES = *~\
                       Makefile.in
