#include "mediaplayerworker.h"
#include <spdlog/spdlog.h>

MediaPlayerWorker::MediaPlayerWorker(QObject *parent)
	: QObject(parent)
{

}

MediaPlayerWorker::~MediaPlayerWorker()
{}

void MediaPlayerWorker::initialize()
{
    // Initialize the Windows Runtime apartment
    try {
        winrt::init_apartment();
        m_initialized = true;
    }
    catch (const winrt::hresult_error& e) {
        spdlog::error("Failed to initialize WinRT apartment");
    }

    // Get the session manager
    m_sessionManager = GlobalSystemMediaTransportControlsSessionManager::RequestAsync().get();

    // Get the current (focused) media session
    m_currentSession = m_sessionManager.GetCurrentSession();
}

void MediaPlayerWorker::extractMediaProperties() {
    initialize();
    if (m_currentSession)
    {
        try {
            // Get the app's name (SourceAppUserModelId is a unique identifier)
            auto appId = m_currentSession.SourceAppUserModelId();
            m_appID = QString::fromStdString(winrt::to_string(appId.c_str()));

            // Get media properties (title, artist, etc.)
            auto mediaProperties = m_currentSession.TryGetMediaPropertiesAsync().get();
            if (mediaProperties)
            {
                // Get title
                m_title = QString::fromStdString(winrt::to_string(mediaProperties.Title().c_str()));

                // Get artist
                m_artist = QString::fromStdString(winrt::to_string(mediaProperties.Artist().c_str()));

            }
            emit mediaInfoExtracted(m_appID, m_title, m_artist);
        }
        catch (const winrt::hresult_error& e) {
            initialize();
            extractMediaProperties();
        }
    }
}

using namespace winrt::Windows::Media::Control;

void MediaPlayerWorker::playPause() {
    if (m_currentSession) {
        // Get playback status
        auto playbackStatus = m_currentSession.GetPlaybackInfo().PlaybackStatus();

        // If paused
        if (playbackStatus == GlobalSystemMediaTransportControlsSessionPlaybackStatus::Paused) {
            bool resumed = m_currentSession.TryPlayAsync().get();
            if (resumed) {
                spdlog::info("Playback resumed");
            }
        }
        // If playing
        else if (playbackStatus == GlobalSystemMediaTransportControlsSessionPlaybackStatus::Playing) {
            bool paused = m_currentSession.TryPauseAsync().get();
            if (paused) {
                spdlog::info("Playback paused");
            }
        }
    }
}