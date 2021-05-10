#include "engine.h"
#include "levelmeter.h"
#include "mainwidget.h"
#include "waveform.h"
#include "progressbar.h"
#include "spectrograph.h"
#include "utils.h"

#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStyle>
#include <QMenu>
#include <QFileDialog>
#include <QTimerEvent>
#include <QMessageBox>


const int NullTimerId = -1;

MainWidget::MainWidget(QWidget *parent)
    :   QWidget(parent)
    ,   m_engine(new Engine(this))
#ifndef DISABLE_WAVEFORM
    ,   m_waveform(new Waveform(this))
#endif
    ,   m_progressBar(new ProgressBar(this))
    ,   m_spectrograph(new Spectrograph(this))
    ,   m_levelMeter(new LevelMeter(this))
    ,   m_modeButton(new QPushButton(this))
    ,   m_pauseButton(new QPushButton(this))
    ,   m_playButton(new QPushButton(this))
    ,   m_infoMessage(new QLabel(tr("Select a file to begin"), this))
    ,   m_infoMessageTimerId(NullTimerId)
    ,   m_modeMenu(new QMenu(this))
    ,   m_loadFileAction(0)
{
    m_spectrograph->setParams(SpectrumNumBands, SpectrumLowFreq, SpectrumHighFreq);

    createUi();
    connectUi();
}

MainWidget::~MainWidget()
{

}


//-----------------------------------------------------------------------------
// Public slots
//-----------------------------------------------------------------------------

void MainWidget::stateChanged(QAudio::Mode mode, QAudio::State state)
{
    Q_UNUSED(mode);

    updateButtonStates();

    if (QAudio::ActiveState != state &&
        QAudio::SuspendedState != state &&
        QAudio::InterruptedState != state) {
        m_levelMeter->reset();
        m_spectrograph->reset();
    }
}

void MainWidget::formatChanged(const QAudioFormat &format)
{
   infoMessage(formatToString(format), NullMessageTimeout);

#ifndef DISABLE_WAVEFORM
    if (QAudioFormat() != format) {
        m_waveform->initialize(format, WaveformTileLength,
                               WaveformWindowDuration);
    }
#endif
}

void MainWidget::spectrumChanged(qint64 position, qint64 length,
                                 const FrequencySpectrum &spectrum)
{
    m_progressBar->windowChanged(position, length);
    m_spectrograph->spectrumChanged(spectrum);
}

void MainWidget::infoMessage(const QString &message, int timeoutMs)
{
    m_infoMessage->setText(message);

    if (NullTimerId != m_infoMessageTimerId) {
        killTimer(m_infoMessageTimerId);
        m_infoMessageTimerId = NullTimerId;
    }

    if (NullMessageTimeout != timeoutMs)
        m_infoMessageTimerId = startTimer(timeoutMs);
}

void MainWidget::errorMessage(const QString &heading, const QString &detail)
{
    QMessageBox::warning(this, heading, detail, QMessageBox::Close);
}

void MainWidget::timerEvent(QTimerEvent *event)
{
    Q_ASSERT(event->timerId() == m_infoMessageTimerId);
    Q_UNUSED(event) // suppress warnings in release builds
    killTimer(m_infoMessageTimerId);
    m_infoMessageTimerId = NullTimerId;
    m_infoMessage->setText("");
}

void MainWidget::audioPositionChanged(qint64 position)
{
#ifndef DISABLE_WAVEFORM
    m_waveform->audioPositionChanged(position);
#else
    Q_UNUSED(position)
#endif
}

void MainWidget::bufferLengthChanged(qint64 length)
{
    m_progressBar->bufferLengthChanged(length);
}


//-----------------------------------------------------------------------------
// Private slots
//-----------------------------------------------------------------------------

void MainWidget::showFileDialog()
{
    const QString dir;
    const QStringList fileNames = QFileDialog::getOpenFileNames(this, tr("Open WAV file"), dir, "*.wav");
    if (fileNames.count()) {
        reset();
        m_engine->loadFile(fileNames.front());
        updateButtonStates();
    }
}


//-----------------------------------------------------------------------------
// Private functions
//-----------------------------------------------------------------------------

void MainWidget::createUi()
{

    setWindowTitle(tr("Spectrum Analyser"));

    QVBoxLayout *windowLayout = new QVBoxLayout(this);

    m_infoMessage->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_infoMessage->setAlignment(Qt::AlignHCenter);
    windowLayout->addWidget(m_infoMessage);

#ifdef SUPERIMPOSE_PROGRESS_ON_WAVEFORM
    QScopedPointer<QHBoxLayout> waveformLayout(new QHBoxLayout);
    waveformLayout->addWidget(m_progressBar);
    m_progressBar->setMinimumHeight(m_waveform->minimumHeight());
    waveformLayout->setContentsMargins(0, 0, 0, 0);
    waveformLayout.take();
#else
#ifndef DISABLE_WAVEFORM
    windowLayout->addWidget(m_waveform);
#endif // DISABLE_WAVEFORM
    windowLayout->addWidget(m_progressBar);
#endif // SUPERIMPOSE_PROGRESS_ON_WAVEFORM

    // Spectrograph and level meter

    QScopedPointer<QHBoxLayout> analysisLayout(new QHBoxLayout);
    analysisLayout->addWidget(m_spectrograph);
    analysisLayout->addWidget(m_levelMeter);
    windowLayout->addLayout(analysisLayout.data());
    analysisLayout.take();

    // Button panel

    const QSize buttonSize(30, 30);

    m_modeButton->setText(tr("Select file"));
    m_modeButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    m_pauseButton->setIcon(QIcon(QPixmap(":/images/pause.png")));
    m_pauseButton->setEnabled(false);
    m_pauseButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_pauseButton->setMinimumSize(buttonSize);

    m_playButton->setIcon(QIcon(QPixmap(":/images/play.png")));
    m_playButton->setEnabled(false);
    m_playButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_playButton->setMinimumSize(buttonSize);

    QScopedPointer<QHBoxLayout> buttonPanelLayout(new QHBoxLayout);
    buttonPanelLayout->addStretch();
    buttonPanelLayout->addWidget(m_modeButton);
    buttonPanelLayout->addWidget(m_pauseButton);
    buttonPanelLayout->addWidget(m_playButton);

    QWidget *buttonPanel = new QWidget(this);
    buttonPanel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    buttonPanel->setLayout(buttonPanelLayout.data());
    buttonPanelLayout.take(); // ownership transferred to buttonPanel

    QScopedPointer<QHBoxLayout> bottomPaneLayout(new QHBoxLayout);
    bottomPaneLayout->addWidget(buttonPanel);
    windowLayout->addLayout(bottomPaneLayout.data());
    bottomPaneLayout.take(); // ownership transferred to windowLayout

    // Apply layout

    setLayout(windowLayout);
}

void MainWidget::connectUi()
{

    connect(m_modeButton, &QPushButton::clicked,
            this, &MainWidget::showFileDialog);

    connect(m_pauseButton, &QPushButton::clicked,
            m_engine, &Engine::suspend);

    connect(m_playButton, &QPushButton::clicked,
            m_engine, &Engine::startPlayback);

    connect(m_engine, &Engine::stateChanged,
            this, &MainWidget::stateChanged);

    connect(m_engine, &Engine::formatChanged,
            this, &MainWidget::formatChanged);

    m_progressBar->bufferLengthChanged(m_engine->bufferLength());

    connect(m_engine, &Engine::bufferLengthChanged,
            this, &MainWidget::bufferLengthChanged);

    connect(m_engine, &Engine::dataLengthChanged,
            this, &MainWidget::updateButtonStates);

    connect(m_engine, &Engine::recordPositionChanged,
            m_progressBar, &ProgressBar::recordPositionChanged);

    connect(m_engine, &Engine::playPositionChanged,
            m_progressBar, &ProgressBar::playPositionChanged);

    connect(m_engine, &Engine::recordPositionChanged,
            this, &MainWidget::audioPositionChanged);

    connect(m_engine, &Engine::playPositionChanged,
            this, &MainWidget::audioPositionChanged);

    connect(m_engine, &Engine::levelChanged,
            m_levelMeter, &LevelMeter::levelChanged);

    connect(m_engine, QOverload<qint64, qint64, const FrequencySpectrum&>::of(&Engine::spectrumChanged),
            this, QOverload<qint64, qint64, const FrequencySpectrum&>::of(&MainWidget::spectrumChanged));

    connect(m_engine, &Engine::infoMessage,
            this, &MainWidget::infoMessage);

    connect(m_engine, &Engine::errorMessage,
            this, &MainWidget::errorMessage);

    connect(m_spectrograph, &Spectrograph::infoMessage,
            this, &MainWidget::infoMessage);

}


void MainWidget::updateButtonStates()
{

    const bool pauseEnabled = (QAudio::ActiveState == m_engine->state() ||
                               QAudio::IdleState == m_engine->state());
    m_pauseButton->setEnabled(pauseEnabled);

    const bool playEnabled = (/*m_engine->dataLength() &&*/
                              (QAudio::AudioOutput != m_engine->mode() ||
                               (QAudio::ActiveState != m_engine->state() &&
                                QAudio::IdleState != m_engine->state() &&
                                QAudio::InterruptedState != m_engine->state())));
    m_playButton->setEnabled(playEnabled);
}

void MainWidget::reset()
{

    m_engine->reset();
    m_levelMeter->reset();
    m_spectrograph->reset();
}

