// COMP710 - KineticImpact
// AudioManager.cpp - FMOD Core wrapper  (TDD §2: deferred to Gold build)
// All methods are functional stubs that compile cleanly.

#include "AudioManager.h"
#include <cstdio>

AudioManager::~AudioManager()
{
    shutdown();
}

bool AudioManager::initialise()
{
    FMOD_RESULT result = FMOD::System_Create(&m_system);
    if (result != FMOD_OK)
    {
        printf("[AudioManager] FMOD::System_Create failed: %d\n", result);
        return false;
    }

    result = m_system->init(32, FMOD_INIT_NORMAL, nullptr);
    if (result != FMOD_OK)
    {
        printf("[AudioManager] FMOD::System::init failed: %d\n", result);
        return false;
    }

    return true;
}

void AudioManager::update()
{
    if (m_system) m_system->update();
}

void AudioManager::shutdown()
{
    stopMusic();

    if (m_system)
    {
        m_system->close();
        m_system->release();
        m_system = nullptr;
    }
}

void AudioManager::playSound(const char* path)
{
    if (!m_system || !path) return;

    FMOD::Sound*   sound   = nullptr;
    FMOD::Channel* channel = nullptr;

    m_system->createSound(path, FMOD_DEFAULT, nullptr, &sound);
    if (sound) m_system->playSound(sound, nullptr, false, &channel);
}

void AudioManager::stopAll()
{
    // Stop all channels - FMOD master channel group
    if (!m_system) return;
    FMOD::ChannelGroup* master = nullptr;
    m_system->getMasterChannelGroup(&master);
    if (master) master->stop();
}

void AudioManager::playMusic(const char* path)
{
    if (!m_system || !path) return;

    stopMusic();   // Release any previously loaded track

    // FMOD_CREATESTREAM: decodes on-the-fly (good for large MP3s)
    // FMOD_LOOP_NORMAL:  loops indefinitely
    FMOD_RESULT result = m_system->createStream(
        path,
        FMOD_CREATESTREAM | FMOD_LOOP_NORMAL | FMOD_2D,
        nullptr,
        &m_musicSound);

    if (result != FMOD_OK || !m_musicSound)
    {
        printf("[AudioManager] playMusic: failed to load '%s' (error %d)\n", path, result);
        m_musicSound = nullptr;
        return;
    }

    m_system->playSound(m_musicSound, nullptr, false, &m_musicChannel);
}

void AudioManager::stopMusic()
{
    if (m_musicChannel)
    {
        m_musicChannel->stop();
        m_musicChannel = nullptr;
    }
    if (m_musicSound)
    {
        m_musicSound->release();
        m_musicSound = nullptr;
    }
}
