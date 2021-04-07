#include "musicplayer.h"

#include <QApplication>
#include <QDesktopWidget>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QGuiApplication::setApplicationDisplayName("MusicPlayerETU");
    MusicPlayer player;

    player.show();
    return app.exec();
}
