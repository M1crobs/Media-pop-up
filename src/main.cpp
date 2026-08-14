// CMakeApp.cpp : Defines the entry point for the application.

#include "main.h"
#include "mainwindow.h"
#include "controller.h"
#include <QApplication>
#include <QFile>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <string>

// default style sheet - embedded into .exe
std::string qssDefaultFilePath = ":/styles/defaultStylesheet.qss";

// customizable style sheet - separate file
std::string qssFilePath = "stylesheet.qss";

int main(int argc, char* argv[])
{
    // redirect the logger into file
    auto fileLogger = spdlog::basic_logger_mt("fileLogger", "logs/appLog.txt", true);
    spdlog::set_default_logger(fileLogger);
    fileLogger->set_level(spdlog::level::info);

    // set logging level to debug if running a debug build
    #ifdef _DEBUG
        fileLogger->set_level(spdlog::level::debug);
    #endif

    QApplication app(argc, argv);

    MainWindow window; // remove window default frame and make it always stay on top
    window.setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

    AppController controller(&window);
    
    // load customizable style sheet
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
        QFile styleFile(QString::fromStdString(qssFilePath));
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

    window.show();
    return app.exec();
}
