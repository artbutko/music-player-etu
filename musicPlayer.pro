TEMPLATE = app
TARGET = musicplayer

QT += widgets multimedia winextras

HEADERS = \
    musicplayer.h \
    volumebutton.h

SOURCES = \
    main.cpp \
    musicplayer.cpp \
    volumebutton.cpp

RC_ICONS = images/volume.ico

target.path = $$[QT_INSTALL_EXAMPLES]/winextras/musicplayer
INSTALLS += target

RESOURCES += \
    icons.qrc
