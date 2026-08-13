#include "mainwindow.h"
#include "mediaplayerworker.h"
#include <QTimer>
#include <QGraphicsOpacityEffect>
#include <spdlog/spdlog.h>

int windowWidth = 320;

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	spdlog::set_level(spdlog::level::debug);

	ui.setupUi(this);
	this->move(1920, 200);
	ui.backgroundWgt->move(windowWidth, ui.backgroundWgt->y());
	initializeAnimations();

	// Setup worker and move it to a different thread
	m_worker = new MediaPlayerWorker();
	m_worker->moveToThread(&m_workerThread);
	m_workerThread.start();

	// Setup timer to check data every 300ms
	auto extractMediaPropsTimer = new QTimer(this);
	connect(extractMediaPropsTimer, &QTimer::timeout, m_worker, &MediaPlayerWorker::extractMediaProperties);
	extractMediaPropsTimer->start(300);

	// Connect signals to slots
	connect(m_worker, &MediaPlayerWorker::mediaInfoExtracted, this, &MainWindow::onMediaInfoExtracted);
}

MainWindow::~MainWindow()
{
	m_workerThread.quit();
	m_workerThread.wait();
}

void MainWindow::setTitleLblText(const QString& text) {
	if (text.length() > 27) {
		QString textCut = text.left(27);
		textCut.append("...");
		ui.titleLbl->setText(textCut);
	}
	else if (text.isEmpty()) {
		ui.titleLbl->setText("Unknown");
	}
	else {
		ui.titleLbl->setText(text);
	}
}

void MainWindow::setArtistLblText(const QString& text) {
	if (text.length() > 30) {
		QString textCut = text.left(30);
		textCut.append("...");
		ui.artistLbl->setText(textCut);
	}
	else if (text.isEmpty()) {
		ui.artistLbl->setText("Unknown");
	}
	else {
		ui.artistLbl->setText(text);
	}
}

void MainWindow::setCommentLblText(const QString& text) {
	ui.commentLbl->setText(text);
}

void MainWindow::onMediaInfoExtracted(QString appID, QString title, QString artist) {
	m_singleShotTimer.setSingleShot(true);
	connect(&m_singleShotTimer, &QTimer::timeout, this, [this]() { m_flyOut->start(); });

	// Check if something's changed
	std::vector<QString> mediaProperties = {appID, title, artist};
	if (mediaProperties != m_mediaProperties) {
		m_mediaProperties = mediaProperties;

		spdlog::info("New media info:\nTitle: {}\nArtist: {}\nAppID: {}",
			title.toStdString(), artist.toStdString(), appID.toStdString());

		// If the window is currently collapsing
		if (m_flyOut->state() == QAbstractAnimation::Running) {
			spdlog::debug("Animation state: the window is currently collapsing");
			// Cancel the previously called collapsing/extending on timer
			m_singleShotTimer.stop();

			// Expand after finished collapsing
			m_singleShotTimer.singleShot(m_flyOut->duration()+100, [this]() { m_flyIn->start(); });

			// Wait 5s and collapse
			m_singleShotTimer.setInterval(5000);
			m_singleShotTimer.start();
		}

		// If the window is fully extended
		if (this->x() == 1920 - windowWidth) {
			spdlog::debug("Animation state: the window is fully extended");
			// Cancel the previously called collapsing/extending on timer
			m_singleShotTimer.stop();

			// Collapse
			m_flyOut->start();

			// Expand after finished collapsing
			m_singleShotTimer.singleShot(m_flyOut->duration() + 100, [this]() { m_flyIn->start(); });

			// Wait 5s and collapse
			m_singleShotTimer.setInterval(5000);
			m_singleShotTimer.start();
		}

		// If the window is currently expanding
		if (m_flyIn->state() == QAbstractAnimation::Running) {
			spdlog::debug("Animation state: the window is currently expanding");
			// Cancel the previously called collapsing/extending on timer
			m_singleShotTimer.stop();

			// Wait 5s and collapse
			m_singleShotTimer.start();
		}

		// If the window is fully collapsed
		if (this->x() == 1920) {
			spdlog::debug("Animation state: the window is fully collapsed");
			// Cancel the previously called collapsing/extending on timer
			m_singleShotTimer.stop();

			// Expand
			m_flyIn->start();
			// Wait 5s and collapse
			m_singleShotTimer.setInterval(5000);
			m_singleShotTimer.start();
		}

		// Display properties in UI
		m_singleShotTimer.singleShot(50, [this, title, artist, appID]() {
			setTitleLblText(title);
			setArtistLblText(artist);
			setCommentLblText(appID);
			emit MediaInfoExtracted(appID, title, artist); });
	}
}

void MainWindow::initializeAnimations() {
	// Setup flyIn animations
	// 1. Window animation (colored rect)
	m_windowFlyIn = new QPropertyAnimation(this, "pos");
	m_windowFlyIn->setStartValue(QPoint(1920, this->y()));
	m_windowFlyIn->setEndValue(QPoint(1920-windowWidth, this->y()));
	m_windowFlyIn->setEasingCurve(QEasingCurve::OutCubic);
	m_windowFlyIn->setDuration(300);

	// 2. Background animation (black rect)
	m_backgroundFlyIn = new QPropertyAnimation(ui.backgroundWgt, "pos");
	m_backgroundFlyIn->setStartValue(QPoint(windowWidth, ui.backgroundWgt->y()));
	m_backgroundFlyIn->setEndValue(QPoint(6, ui.backgroundWgt->y()));
	m_backgroundFlyIn->setEasingCurve(QEasingCurve::OutCubic);
	m_backgroundFlyIn->setDuration(200);

	// 3. Text animation (media info and comment)
	// Title
	QGraphicsOpacityEffect* titleLblEffect = new QGraphicsOpacityEffect(ui.titleLbl);
	ui.titleLbl->setGraphicsEffect(titleLblEffect);
	titleLblEffect->setOpacity(0.0);
	QPropertyAnimation* titleFadeIn = new QPropertyAnimation(titleLblEffect, "opacity");
	titleFadeIn->setStartValue(0.0);
	titleFadeIn->setEndValue(1.0);
	titleFadeIn->setEasingCurve(QEasingCurve::OutCubic);
	titleFadeIn->setDuration(400);

	// Artist
	QGraphicsOpacityEffect* artistLblEffect = new QGraphicsOpacityEffect(ui.artistLbl);
	ui.artistLbl->setGraphicsEffect(artistLblEffect);
	artistLblEffect->setOpacity(0.0);
	QPropertyAnimation* artistFadeIn = new QPropertyAnimation(artistLblEffect, "opacity");
	artistFadeIn->setStartValue(0.0);
	artistFadeIn->setEndValue(1.0);
	artistFadeIn->setEasingCurve(QEasingCurve::OutCubic);
	artistFadeIn->setDuration(400);

	// Comment
	QGraphicsOpacityEffect* commentLblEffect = new QGraphicsOpacityEffect(ui.commentLbl);
	ui.commentLbl->setGraphicsEffect(commentLblEffect);
	commentLblEffect->setOpacity(0.0);
	QPropertyAnimation* commentFadeIn = new QPropertyAnimation(commentLblEffect, "opacity");
	commentFadeIn->setStartValue(0.0);
	commentFadeIn->setEndValue(1.0);
	commentFadeIn->setEasingCurve(QEasingCurve::OutCubic);
	commentFadeIn->setDuration(400);

	// Assemble animations
	m_textFadeIn = new QParallelAnimationGroup();
	m_textFadeIn->addAnimation(titleFadeIn);
	m_textFadeIn->addAnimation(artistFadeIn);
	m_textFadeIn->addAnimation(commentFadeIn);

	m_flyIn = new QSequentialAnimationGroup();
	m_flyIn->addAnimation(m_windowFlyIn);
	m_flyIn->addAnimation(m_backgroundFlyIn);
	m_flyIn->addAnimation(m_textFadeIn);

	// ----------------------------------------

	// Setup flyOut animations
	// 1. Background animation (black rect)
	m_backgroundFlyOut = new QPropertyAnimation(ui.backgroundWgt, "pos");
	m_backgroundFlyOut->setStartValue(QPoint(6, ui.backgroundWgt->y()));
	m_backgroundFlyOut->setEndValue(QPoint(windowWidth, ui.backgroundWgt->y()));
	m_backgroundFlyOut->setEasingCurve(QEasingCurve::InCubic);
	m_backgroundFlyOut->setDuration(300);

	// 2. Window animation (colored rect)
	m_windowFlyOut = new QPropertyAnimation(this, "pos");
	m_windowFlyOut->setStartValue(QPoint(1920-windowWidth, this->y()));
	m_windowFlyOut->setEndValue(QPoint(1920, this->y()));
	m_windowFlyOut->setDuration(200);

	// 3. Text animation (media info and comment)
	// Title
	ui.titleLbl->setGraphicsEffect(titleLblEffect);
	QPropertyAnimation* titleFadeOut = new QPropertyAnimation(titleLblEffect, "opacity");
	titleFadeOut->setStartValue(1.0);
	titleFadeOut->setEndValue(0.0);
	titleFadeOut->setEasingCurve(QEasingCurve::InCubic);
	titleFadeOut->setDuration(50);

	// Artist
	ui.artistLbl->setGraphicsEffect(artistLblEffect);
	QPropertyAnimation* artistFadeOut = new QPropertyAnimation(artistLblEffect, "opacity");
	artistFadeOut->setStartValue(1.0);
	artistFadeOut->setEndValue(0.0);
	artistFadeOut->setEasingCurve(QEasingCurve::InCubic);
	artistFadeOut->setDuration(50);

	// Comment
	ui.commentLbl->setGraphicsEffect(commentLblEffect);
	QPropertyAnimation* commentFadeOut = new QPropertyAnimation(commentLblEffect, "opacity");
	commentFadeOut->setStartValue(1.0);
	commentFadeOut->setEndValue(0.0);
	commentFadeOut->setEasingCurve(QEasingCurve::InCubic);
	commentFadeOut->setDuration(50);

	// Assemble animations
	m_textFadeOut = new QParallelAnimationGroup();
	m_textFadeOut->addAnimation(titleFadeOut);
	m_textFadeOut->addAnimation(artistFadeOut);
	m_textFadeOut->addAnimation(commentFadeOut);

	m_flyOut = new QSequentialAnimationGroup();
	m_flyOut->addAnimation(m_textFadeOut);
	m_flyOut->addAnimation(m_backgroundFlyOut);
	m_flyOut->addAnimation(m_windowFlyOut);
}