#pragma once

#include "ui_mainwindow.h"
#include <QMainWindow>
#include <QThread>
#include <QSequentialAnimationGroup>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QTimer>
#include <QSystemTrayIcon>
#include <QMenu>
#include <vector>

class MediaPlayerWorker;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = nullptr);
	~MainWindow();

private:
	void initializeAnimations();
	void initializeTrayIcon();
	void updateStyleSheet();
	void startAnimations();

	Ui::MainWindow ui;
	MediaPlayerWorker* m_worker;
	QThread m_workerThread;
	std::vector<QString> m_mediaProperties;
	QTimer m_singleShotTimer;
	QSystemTrayIcon* m_trayIcon;
	QMenu* m_trayMenu;
	QAction* m_quitAction;
	QAction* m_editQssAction;
	QAction* m_updateQssAction;
	QAction* m_editCommentListAction;

	QPropertyAnimation* m_windowFlyIn;
	QPropertyAnimation* m_backgroundFlyIn;
	QParallelAnimationGroup* m_textFadeIn;
	QSequentialAnimationGroup* m_flyIn;

	QPropertyAnimation* m_windowFlyOut;
	QPropertyAnimation* m_backgroundFlyOut;
	QParallelAnimationGroup* m_textFadeOut;
	QSequentialAnimationGroup* m_flyOut;

public slots:
	void setTitleLblText(const QString&);
	void setArtistLblText(const QString&);
	void setCommentLblText(const QString&);
	void onMediaInfoExtracted(QString appID, QString title, QString artist);

signals:
	void MediaInfoExtracted(QString appID, QString title, QString artist);
};

// default style sheet - embedded into .exe
inline std::string qssDefaultFilePath = ":/styles/styles/defaultStylesheet.qss";

// customizable style sheet - separate file
inline std::string qssFilePath = "stylesheet.qss";