QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17 lrelease
#embed_translations

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    MtbModules.cpp \
    configfile.cpp \
    main.cpp \
    mainwindow.cpp \
    tcpsocket.cpp \
    winconfig.cpp \
    winlist.cpp

HEADERS += \
    MtbModules.h \
    configfile.h \
    mainwindow.h \
    tcpsocket.h \
    winconfig.h \
    winlist.h

FORMS += \
    mainwindow.ui \
    winconfig.ui \
    winlist.ui

#TRANSLATIONS += \
#  english.ts

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
  resources.qrc
