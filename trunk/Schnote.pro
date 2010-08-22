#-------------------------------------------------
#
# Project created by QtCreator 2010-07-17T18:33:19
#
#-------------------------------------------------

QT       += core gui sql script

TARGET = Schnote
TEMPLATE = app

!win32 {
    LIBS += -L/usr/include/curl -lcurl
}

win32 {
    LIBS += -Lc:/curl/lib -lcurldll
    INCLUDEPATH += c:/curl/include/
}

SOURCES += src/main.cpp\
    src/ui/mainwindow.cpp \
    src/ui/notewindow.cpp \
    src/simplenote/api.cpp \
    src/lib/curlobject.cpp \
    src/ui/widgets/notetextedit.cpp \
    src/ui/widgets/noteitemdelegate.cpp \
    src/data/notelistmodel.cpp \
    src/data/notedb.cpp \
    src/data/notesync.cpp \
    src/ui/settingswindow.cpp \
    src/data/settings.cpp


HEADERS  += src/ui/mainwindow.h \
    src/ui/notewindow.h \
    src/simplenote/api.h \
    src/lib/curlobject.h \
    src/ui/widgets/notetextedit.h \
    src/ui/widgets/noteitemdelegate.h \
    src/data/notelistmodel.h \
    src/data/notedb.h \
    src/data/notedata.h \
    src/data/notesync.h \
    src/ui/settingswindow.h \
    src/data/settings.h


FORMS    += ui/mainwindow.ui \
    ui/settingswindow.ui

CONFIG += mobility
MOBILITY = 

symbian {
    TARGET.UID3 = 0xe62bd33d
    # TARGET.CAPABILITY += 
    TARGET.EPOCSTACKSIZE = 0x14000
    TARGET.EPOCHEAPSIZE = 0x020000 0x800000
}
