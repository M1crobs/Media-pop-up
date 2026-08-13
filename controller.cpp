#include "controller.h"
#include "mainwindow.h"
#include "logic.h"
#include <spdlog/spdlog.h>
#include <string>

AppController::AppController(MainWindow* view, QObject* parent)
	: QObject(parent), m_view(view)
{
	m_logic = new Logic();
	connect(m_view, &MainWindow::MediaInfoExtracted, this, [this](QString appID, QString title, QString artist){
		std::string comment = m_logic->chooseComment(title.toStdString(), artist.toStdString());
		spdlog::info("Chosen new comment: {}", comment);
		m_view->setCommentLblText(QString::fromStdString(comment));
		});
}

AppController::~AppController()
{
}