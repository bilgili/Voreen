TEMPLATE	= app
LANGUAGE	= C++
TARGET          = ShowWidgets

CONFIG	+= qt warn_on thread
QT += opengl

include(../../config.txt)

include(../../commonconf.txt)

include(../../voreenapp.txt)

SOURCES	+= \
           showwidgetsmain.cpp \
           sliderspinboxwidget.cpp \
	       thresholdwidget.cpp \
           transferfuncwidget.cpp

HEADERS	+= \
           ../../include/voreen/qt/thresholdwidget.h \
           ../../include/voreen/qt/sliderspinboxwidget.h \
           ../../include/voreen/qt/transferfuncwidget.h

FORMS	= sliderspinboxwidget.ui \
          thresholdwidget.ui \
          transferfuncwidgetexample.ui

unix {
   UI_DIR      = .ui
   MOC_DIR     = .moc
   OBJECTS_DIR = .obj
}
