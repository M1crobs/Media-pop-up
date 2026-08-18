#include "mainwindow.h"
#include "mediaplayerworker.h"
#include <QGraphicsOpacityEffect>
#include <QDesktopServices>
#include <QFile>
#include <spdlog/spdlog.h>
#include <format>

// the width of the main window
int windowWidth = 320;

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	initializeTrayIcon();

	// move window off screen
	this->move(1920, 200);
	// move background (black rectangle) off the main window
	ui.backgroundWgt->move(windowWidth, ui.backgroundWgt->y());

	initializeAnimations();

	// setup animatin timer
	m_singleShotTimer.setSingleShot(true);
	connect(&m_singleShotTimer, &QTimer::timeout, this, [this]() { m_flyOut->start(); });

	// Setup worker and move it to a different thread
	// The worker exists to call WinRT functions in a different thread
	// So they will not stop the main one
	m_worker = new MediaPlayerWorker();
	m_worker->moveToThread(&m_workerThread);
	m_workerThread.start();

	// Setup timer to check media data every 300ms
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
	// truncate if the number of characters exceeds certain number
	if (text.length() > 26) {
		QString textCut = text.left(26);
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
	// truncate if the number of characters exceeds certain number
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
	// ingnore the call if nothing's changed
	std::vector<QString> mediaProperties = {appID, title, artist};
	if (mediaProperties == m_mediaProperties) {
		return;
	}

	m_mediaProperties = mediaProperties;

	std::string titleStd = title.toStdString();
	std::string artistStd = artist.toStdString();
	std::string appIDStd = appID.toStdString();

	spdlog::info("New media info:\nTitle: {}\nArtist: {}\nAppID: {}",
		titleStd, artistStd, appIDStd);

	m_trayIcon->setToolTip(QString::fromStdString(std::format("{} - {}", artistStd, titleStd)));

	startAnimations();

	// Display properties in UI
	m_singleShotTimer.singleShot(50, [this, title, artist, appID]() {
		setTitleLblText(title);
		setArtistLblText(artist);
		setCommentLblText(appID);
		emit MediaInfoExtracted(appID, title, artist); });
}

void MainWindow::initializeTrayIcon() {
	m_trayMenu = new QMenu(this);
	m_trayMenu->setObjectName("trayMenu");
	m_editQssAction = m_trayMenu->addAction("Edit QSS", []() {
		QDesktopServices::openUrl(QUrl::fromLocalFile("stylesheet.qss"));
		});
	m_updateQssAction = m_trayMenu->addAction("Update QSS", [this]() {
		updateStyleSheet();
		});
	m_trayMenu->addSeparator();
	m_quitAction = m_trayMenu->addAction("Quit", qApp, &QApplication::quit);
	
	m_trayIcon = new QSystemTrayIcon(this);
	m_trayIcon->setIcon(QIcon(":/icons/icons/appIcon.png"));
	m_trayIcon->setContextMenu(m_trayMenu);
	m_trayIcon->setToolTip("Media Pop-up");

	connect(m_trayIcon, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason) {
		if (reason == QSystemTrayIcon::ActivationReason::DoubleClick) {
			startAnimations();
		}
		});

	m_trayIcon->show();
}

void MainWindow::updateStyleSheet() {
	QFile styleFile(QString::fromStdString(qssFilePath));
	if (styleFile.open(QFile::ReadOnly))
	{
		QString styleSheet = QLatin1String(styleFile.readAll());
		qApp->setStyleSheet(styleSheet);
		styleFile.close();
		spdlog::info("Loaded style sheet from .qss file: {}", qssFilePath);
	}
	else // if failed to open file
	{
		spdlog::warn("Failed to load style sheet from .qss file: '{}'", qssFilePath);

		// load default style sheet if failed
		QFile styleFile(QString::fromStdString(qssDefaultFilePath));
		if (styleFile.open(QFile::ReadOnly))
		{
			QString styleSheet = QLatin1String(styleFile.readAll());
			qApp->setStyleSheet(styleSheet);
			styleFile.close();
			spdlog::info("Loaded default style sheet from .qss file: {}", qssDefaultFilePath);
		}
		else // if failed to open file
		{
			spdlog::warn("Failed to load default style sheet from .qss file: '{}'", qssDefaultFilePath);
		}
	}
}

void MainWindow::startAnimations() {
	// If the window is currently collapsing
	if (m_flyOut->state() == QAbstractAnimation::Running) {
		spdlog::debug("Animation state: the window is currently collapsing");
		// Cancel the previously called collapsing/extending on timer
		m_singleShotTimer.stop();

		// Expand after finished collapsing
		m_singleShotTimer.singleShot(m_flyOut->duration() + 100, [this]() { m_flyIn->start(); });

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
}

void MainWindow::initializeAnimations() {
	// Setup flyIn animations
	// 1. Window animation (colored rectangle)
	m_windowFlyIn = new QPropertyAnimation(this, "pos");
	m_windowFlyIn->setStartValue(QPoint(1920, this->y()));
	m_windowFlyIn->setEndValue(QPoint(1920-windowWidth, this->y()));
	m_windowFlyIn->setEasingCurve(QEasingCurve::OutCubic);
	m_windowFlyIn->setDuration(300);

	// 2. Background animation (black rectangle)
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
	// 1. Background animation (black rectangle)
	m_backgroundFlyOut = new QPropertyAnimation(ui.backgroundWgt, "pos");
	m_backgroundFlyOut->setStartValue(QPoint(6, ui.backgroundWgt->y()));
	m_backgroundFlyOut->setEndValue(QPoint(windowWidth, ui.backgroundWgt->y()));
	m_backgroundFlyOut->setEasingCurve(QEasingCurve::InCubic);
	m_backgroundFlyOut->setDuration(300);

	// 2. Window animation (colored rectangle)
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