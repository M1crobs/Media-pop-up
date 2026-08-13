// CMakeApp.cpp : Defines the entry point for the application.

#include "main.h"
#include "mainwindow.h"
#include "controller.h"
#include <QApplication>
#include <QFile>
#include <spdlog/spdlog.h>
#include <string>
#include <iostream>

std::string qssFilePath = ":/styles/stylesheet.qss";

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    MainWindow window;
    window.setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

    AppController controller(&window);
    
    // load style sheet
    QFile styleFile(QString::fromStdString(qssFilePath));
    if (styleFile.open(QFile::ReadOnly))
    {
        QString styleSheet = QLatin1String(styleFile.readAll());
        qApp->setStyleSheet(styleSheet);
        styleFile.close();
        spdlog::info("Loaded style sheet from .qss file: {}", qssFilePath);
    }
    else
    {
        spdlog::info("Failed to load style sheet from .qss file: '{}'", qssFilePath);
    }

    window.show();
    return app.exec();
}
