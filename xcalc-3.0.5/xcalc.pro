QT += core gui
QMAKE_CXXFLAGS += -mfpmath=387
TARGET = xcalc
TEMPLATE = app
SOURCES += xcalc.cpp \
 aschar.cpp \
 qengine.cpp \
 xcalcwindow.cpp \
 convert.cpp \
 register.cpp \
 util.cpp \
 xcalcapp.cpp \
 xcalcconfig.cpp \
 profile.cpp \
	xcalcmain.cpp \
	xcalcutil.cpp
HEADERS += xcalcwindow.h \
 aschar.h \
 qengine.h \
 xcalc.h \
 xcalcrc.h \
 register.h \
 typedef.h \
 xcalcconfig.h \
 profile.h \
 util.h \
	xcalcutil.h
FORMS += xcalcconfig.ui
OTHER_FILES += copying.txt versions.txt
CONFIG += warn_on
