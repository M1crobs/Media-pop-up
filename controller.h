#pragma once
#include <QObject>

class MainWindow;
class Logic;

class AppController : QObject {
	Q_OBJECT

public:
	explicit AppController(MainWindow* view, QObject* parent = nullptr);
	~AppController();

private:
	MainWindow* m_view;
	Logic* m_logic;
};