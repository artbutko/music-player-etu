#pragma once

#include <QWidget>
#include <QMediaPlayer>

class VolumeButton;

QT_FORWARD_DECLARE_CLASS(QLabel)
QT_FORWARD_DECLARE_CLASS(QSlider)
QT_FORWARD_DECLARE_CLASS(QAbstractButton)

class MusicPlayer : public QWidget
{
    Q_OBJECT
public:
    explicit MusicPlayer(QWidget *parent = nullptr);
    static QStringList supportedMimeTypes();

public slots:
    void openFile();
    void playUrl(const QUrl& url);
    void togglePlayback();

private slots:
    void updateState(QMediaPlayer::State state);
    void updatePosition(qint64 position);
    void updateDuration(qint64 duration);
    void setPosition(int position);
    void updateInfo();

private:
    void createWidgets();

    QMediaPlayer mediaPlayer;
    QAbstractButton *playButton = nullptr;
    VolumeButton *volumeButton = nullptr;
    QSlider *positionSlider = nullptr;
    QLabel *positionLabel = nullptr;
    QLabel *infoLabel = nullptr;
    QPoint offset;
    QString fileName;
};
