#pragma once

#include <QWidget>
#include <QMediaPlayer>

QT_FORWARD_DECLARE_CLASS(QLabel)
QT_FORWARD_DECLARE_CLASS(QSlider)
QT_FORWARD_DECLARE_CLASS(QAbstractButton)

class MusicPlayer : public QWidget
{
    Q_OBJECT
public:
    explicit MusicPlayer(QWidget *parent = nullptr);

public slots:
    void openFile();
    void playUrl(const QUrl& url);

private:
    void createWidgets();

    QMediaPlayer mediaPlayer;
    QAbstractButton *playButton = nullptr;
    QSlider *positionSlider = nullptr;
    QLabel *positionLabel = nullptr;
    QLabel *infoLabel = nullptr;
    QPoint offset;
    QString fileName;
};
