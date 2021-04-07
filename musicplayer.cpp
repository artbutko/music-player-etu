#include "musicplayer.h"

#include <QtWidgets>
#include <QtWinExtras>

MusicPlayer::MusicPlayer(QWidget *parent) : QWidget(parent)
{
    createWidgets();

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

void MusicPlayer::createWidgets()
{
    playButton = new QToolButton(this);
    playButton->setEnabled(false);
    playButton->setIcon(QIcon(":/images/play.ico"));

    QAbstractButton *openButton = new QToolButton(this);
    openButton->setText(tr("..."));
    openButton->setFixedSize(playButton->sizeHint());
    connect(openButton, &QAbstractButton::clicked, this, &MusicPlayer::openFile);

    positionSlider = new QSlider(Qt::Horizontal, this);
    positionSlider->setEnabled(false);

    infoLabel = new QLabel(this);
    positionLabel = new QLabel(tr("00:00"), this);
    positionLabel->setMinimumWidth(positionLabel->sizeHint().width());

    QBoxLayout *controlLayout = new QHBoxLayout;
    controlLayout->setMargin(0);
    controlLayout->addWidget(openButton);
    controlLayout->addWidget(playButton);
    controlLayout->addWidget(positionSlider);
    controlLayout->addWidget(positionLabel);

    QBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(infoLabel);
    mainLayout->addLayout(controlLayout);
}
