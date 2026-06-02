#pragma once
// COMP710 - KineticImpact
// Game.h - SDL window + OpenGL context; fixed-timestep game loop; scene switching
// TDD §4.1 / §6.8

#include "Types.h"
#include "AudioManager.h"
#include <SDL.h>
#include <memory>

class Scene;
class UIManager;
class LevelManager;

class Game
{
public:
    Game();
    ~Game();

    bool initialise();        // Create window, init SDL, OpenGL, ImGui
    void run();               // Main loop: fixed-timestep at 60 Hz (TDD §6.8)
    void quit() { m_running = false; }

    SDL_Renderer* getRenderer() const { return m_renderer; }

private:
    void processEvents();
    void update(float dt);
    void render();

    void transitionToState(GameState next);
    void toggleFullscreen();

    // ── SDL / GL handles ───────────────────────────────────────────────────────
    SDL_Window*   m_window    = nullptr;
    SDL_Renderer* m_renderer  = nullptr;
    SDL_GLContext m_glContext  = nullptr;

    // ── Owned subsystems ───────────────────────────────────────────────────────
    std::unique_ptr<Scene>        m_scene;
    std::unique_ptr<UIManager>    m_ui;
    std::unique_ptr<LevelManager> m_levelManager;
    AudioManager                  m_audio;

    // ── Loop state ─────────────────────────────────────────────────────────────
    bool      m_running       = false;
    bool      m_fullscreen    = false;
    GameState m_state         = GameState::SPLASH;
    float     m_splashTimer   = 0.0f;   // seconds on splash screen
    float     m_accumulator   = 0.0f;   // fixed-timestep remainder
    int       m_selectedLevel = 1;      // level chosen on the select screen
};
