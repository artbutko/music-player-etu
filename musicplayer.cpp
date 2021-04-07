#include "musicplayer.h"
#include "volumebutton.h"

#include <QtWidgets>
#include <QtWinExtras>

MusicPlayer::MusicPlayer(QWidget *parent) : QWidget(parent)
{
    createWidgets();

    connect(&mediaPlayer, &QMediaPlayer::positionChanged, this, &MusicPlayer::updatePosition);
    connect(&mediaPlayer, &QMediaPlayer::durationChanged, this, &MusicPlayer::updateDuration);
    connect(&mediaPlayer, &QMediaObject::metaDataAvailableChanged, this, &MusicPlayer::updateInfo);
    connect(&mediaPlayer, &QMediaPlayer::stateChanged,this, &MusicPlayer::updateState);
}

void MusicPlayer::openFile()
{
    QFileDialog fileDialog(this);
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog.setWindowTitle(tr("Открыть файл"));
    fileDialog.setDirectory(QStandardPaths::standardLocations(QStandardPaths::MusicLocation).value(0, QDir::homePath()));
    if (fileDialog.exec() == QDialog::Accepted) {
       playUrl(fileDialog.selectedUrls().constFirst());
    }
}

void MusicPlayer::playUrl(const QUrl &url)
{
    playButton->setEnabled(true);
    const QString filePath = url.toLocalFile();
    setWindowFilePath(filePath);
    infoLabel->setText(QDir::toNativeSeparators(filePath));
    fileName = QFileInfo(filePath).fileName();
    mediaPlayer.setMedia(url);
    mediaPlayer.play();
}

void MusicPlayer::togglePlayback()
{
    if (mediaPlayer.mediaStatus() == QMediaPlayer::NoMedia) {
        openFile();
    }
    else if (mediaPlayer.state() == QMediaPlayer::PlayingState) {
        mediaPlayer.pause();
    }
    else {
        mediaPlayer.play();
    }
}

void MusicPlayer::updateState(QMediaPlayer::State state)
{
    if (state == QMediaPlayer::PlayingState) {
        playButton->setIcon(QIcon(":/images/pause.ico"));
    } else {
        playButton->setIcon(QIcon(":/images/play.ico"));
    }
}

static QString formatTime(qint64 timeMilliSeconds)
{
    qint64 seconds = timeMilliSeconds / 1000;
    const qint64 minutes = seconds / 60;
    seconds -= minutes * 60;
    return QStringLiteral("%1:%2")
        .arg(minutes, 2, 10, QLatin1Char('0'))
        .arg(seconds, 2, 10, QLatin1Char('0'));
}

void MusicPlayer::updatePosition(qint64 position)
{
    positionSlider->setValue(position);
    positionLabel->setText(formatTime(position));
}

void MusicPlayer::updateDuration(qint64 duration)
{
    positionSlider->setRange(0, duration);
    positionSlider->setEnabled(duration > 0);
    positionSlider->setPageStep(duration / 10);
    updateInfo();
}

void MusicPlayer::setPosition(int position)
{
    if (qAbs(mediaPlayer.position() - position) > 99)
        mediaPlayer.setPosition(position);
}

void MusicPlayer::updateInfo()
{
    QStringList info;
    if (!fileName.isEmpty())
        info.append(fileName);
    if (mediaPlayer.isMetaDataAvailable()) {
        QString author = mediaPlayer.metaData(QStringLiteral("Автор")).toString();
        if (!author.isEmpty())
            info.append(author);
        QString title = mediaPlayer.metaData(QStringLiteral("Название")).toString();
        if (!title.isEmpty())
            info.append(title);
    }
    info.append(formatTime(mediaPlayer.duration()));
    infoLabel->setText(info.join(tr(" - ")));
}

void MusicPlayer::createWidgets()
{
    playButton = new QToolButton(this);
    playButton->setEnabled(false);
    playButton->setIcon(QIcon(":/images/play.ico"));
    connect(playButton, &QAbstractButton::clicked, this, &MusicPlayer::togglePlayback);

    QAbstractButton *openButton = new QToolButton(this);
    openButton->setText(tr("..."));
    openButton->setFixedSize(playButton->sizeHint());
    connect(openButton, &QAbstractButton::clicked, this, &MusicPlayer::openFile);

    volumeButton = new VolumeButton(this);
    volumeButton->setVolume(mediaPlayer.volume());
    connect(volumeButton, &VolumeButton::volumeChanged, &mediaPlayer, &QMediaPlayer::setVolume);

    positionSlider = new QSlider(Qt::Horizontal, this);
    positionSlider->setEnabled(false);
    connect(positionSlider, &QAbstractSlider::valueChanged, this, &MusicPlayer::setPosition);

    infoLabel = new QLabel(this);
    positionLabel = new QLabel(tr("00:00"), this);
    positionLabel->setMinimumWidth(positionLabel->sizeHint().width());

    QBoxLayout *controlLayout = new QHBoxLayout;
    controlLayout->setMargin(0);
    controlLayout->addWidget(openButton);
    controlLayout->addWidget(playButton);
    controlLayout->addWidget(positionSlider);
    controlLayout->addWidget(positionLabel);
    controlLayout->addWidget(volumeButton);

    QBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(infoLabel);
    mainLayout->addLayout(controlLayout);
}
