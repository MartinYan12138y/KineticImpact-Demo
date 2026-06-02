#pragma once
// COMP710 - KineticImpact
// AudioManager.h - FMOD Core wrapper  (TDD §2 - deferred to Gold build)
// Stub is compile-safe; methods are no-ops until FMOD is wired up.

#include <fmod.hpp>

class AudioManager
{
public:
    AudioManager()  = default;
    ~AudioManager();

    bool initialise();      // Create FMOD::System, set up channels
    void update();          // Must be called once per frame (FMOD requirement)
    void shutdown();

    void playSound(const char* path);   // One-shot fire-and-forget
    void stopAll();

    // Background music: streams the file and loops it indefinitely
    void playMusic(const char* path);
    void stopMusic();

private:
    FMOD::System*  m_system       = nullptr;

    // Music track — kept alive for the duration of playback
    FMOD::Sound*   m_musicSound   = nullptr;
    FMOD::Channel* m_musicChannel = nullptr;
};
