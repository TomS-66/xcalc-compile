# xcalc.pro für Qt 6
QT += core6 gui6 widgets  # WICHTIG: "core6 gui6 widgets" statt "core gui"
CONFIG += c++17
DEFINES += QT_DEPRECATED_WARNINGS

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
