#pragma once

#include <QObject>
#include <winrt/Windows.Media.Control.h>
#include <winrt/Windows.Foundation.Collections.h>

using namespace winrt;
using namespace Windows::Media::Control;

class MediaPlayerWorker : public QObject
{
	Q_OBJECT

public:
	MediaPlayerWorker(QObject *parent = nullptr);
	~MediaPlayerWorker();

public slots:
    void initialize();
    void extractMediaProperties();
    void playPause();

signals:
    void mediaInfoExtracted(QString appID, QString title, QString artist);

private:
    bool m_initialized = false;
    GlobalSystemMediaTransportControlsSessionManager m_sessionManager{ nullptr };
    GlobalSystemMediaTransportControlsSession m_currentSession{ nullptr };
    QString m_title;
    QString m_artist;
    QString m_appID;
};

