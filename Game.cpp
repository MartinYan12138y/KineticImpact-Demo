// COMP710 - KineticImpact
// Game.cpp - SDL window + OpenGL; fixed-timestep loop; scene + state management
// TDD §4.1 / §6.8

// GLEW must be included before ANY other OpenGL / SDL / ImGui headers
#include <GL/glew.h>

#include "Game.h"
#include "Scene.h"
#include "UIManager.h"
#include "LevelManager.h"
#include "CollisionManager.h"
#include "PlayerTank.h"
#include "EnemyTank.h"
#include "Infantry.h"

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include <cstdio>

// Forward declare CollisionManager for local ownership
static CollisionManager s_collision;

// ── Constructor / Destructor ──────────────────────────────────────────────────
Game::Game() = default;

Game::~Game()
{
    // Cleanup ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    if (m_glContext) SDL_GL_DeleteContext(m_glContext);
    if (m_renderer)  SDL_DestroyRenderer(m_renderer);
    if (m_window)    SDL_DestroyWindow(m_window);

    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

// ── Initialise ────────────────────────────────────────────────────────────────
bool Game::initialise()
{
    // SDL core
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0)
    {
        printf("[Game] SDL_Init failed: %s\n", SDL_GetError());
        return false;
    }

    // SDL_image
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG))
    {
        printf("[Game] IMG_Init failed: %s\n", IMG_GetError());
        return false;
    }

    // SDL_ttf
    if (TTF_Init() < 0)
    {
        printf("[Game] TTF_Init failed: %s\n", TTF_GetError());
        return false;
    }

    // OpenGL hints for ImGui backend
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    // Window
    m_window = SDL_CreateWindow(
        "KineticImpact - COMP710",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        Constants::SCREEN_W, Constants::SCREEN_H,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    if (!m_window)
    {
        printf("[Game] SDL_CreateWindow failed: %s\n", SDL_GetError());
        return false;
    }

    // GL context
    m_glContext = SDL_GL_CreateContext(m_window);
    if (!m_glContext)
    {
        printf("[Game] SDL_GL_CreateContext failed: %s\n", SDL_GetError());
        return false;
    }
    SDL_GL_SetSwapInterval(1);  // VSync

    // GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        printf("[Game] GLEW init failed\n");
        return false;
    }

    // SDL renderer (used alongside OpenGL for SDL_RenderCopyEx sprite rendering)
    m_renderer = SDL_CreateRenderer(m_window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!m_renderer)
    {
        // Fallback: software renderer
        m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_SOFTWARE);
        if (!m_renderer) { printf("[Game] Renderer creation failed\n"); return false; }
    }

    // ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForOpenGL(m_window, m_glContext);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Subsystems
    m_scene        = std::make_unique<Scene>();
    m_ui           = std::make_unique<UIManager>();
    m_levelManager = std::make_unique<LevelManager>();

    m_ui->initialise(m_renderer);
    s_collision.initialise();

    // Audio: initialise FMOD and start background music
    if (m_audio.initialise())
        m_audio.playMusic("sound.mp3");

    // Start game at splash screen
    m_state       = GameState::SPLASH;
    m_splashTimer = Constants::SPLASH_DURATION;

    return true;
}

// ── Main loop: fixed-timestep (TDD §6.8) ─────────────────────────────────────
void Game::run()
{
    m_running = true;
    Uint64       prev = SDL_GetPerformanceCounter();
    const double freq = (double)SDL_GetPerformanceFrequency();

    while (m_running)
    {
        Uint64 now     = SDL_GetPerformanceCounter();
        float  elapsed = (float)((now - prev) / freq);
        prev = now;

        // Cap to 250ms to avoid spiral-of-death on long frame
        if (elapsed > 0.25f) elapsed = 0.25f;
        m_accumulator += elapsed;

        processEvents();

        // Fixed physics steps at 60 Hz
        while (m_accumulator >= Constants::FIXED_TIMESTEP)
        {
            update(Constants::FIXED_TIMESTEP);
            m_accumulator -= Constants::FIXED_TIMESTEP;
        }

        m_audio.update();   // FMOD requires one update per frame
        render();
    }
}

// ── Event processing ──────────────────────────────────────────────────────────
void Game::processEvents()
{
    SDL_Event evt;
    while (SDL_PollEvent(&evt))
    {
        ImGui_ImplSDL2_ProcessEvent(&evt);

        if (evt.type == SDL_QUIT)
            quit();

        // Key-down events (one-shot, not held)
        if (evt.type == SDL_KEYDOWN)
        {
            switch (evt.key.keysym.scancode)
            {
            case SDL_SCANCODE_ESCAPE: quit(); break;
            case SDL_SCANCODE_F11: toggleFullscreen(); break;

            // T: toggle repair mode on PlayerTank
            case SDL_SCANCODE_T:
                if (m_state == GameState::PLAYING && m_scene)
                {
                    for (auto* obj : m_scene->getObjects())
                    {
                        auto* pt = dynamic_cast<PlayerTank*>(obj);
                        if (pt) { pt->toggleRepairMode(); break; }
                    }
                }
                break;

            default: break;
            }
        }

        // Splash → Menu on any key; Menu → Level Select on any key
        if (evt.type == SDL_KEYDOWN && m_state == GameState::SPLASH)
            transitionToState(GameState::MENU);
        if (evt.type == SDL_KEYDOWN && m_state == GameState::MENU)
            transitionToState(GameState::LEVEL_SELECT);

        // Level Select: Q = Level 1, W = Level 2, E = open UI2 help screen
        if (evt.type == SDL_KEYDOWN && m_state == GameState::LEVEL_SELECT)
        {
            if (evt.key.keysym.scancode == SDL_SCANCODE_Q)
            {
                m_selectedLevel = 1;
                transitionToState(GameState::PLAYING);
            }
            else if (evt.key.keysym.scancode == SDL_SCANCODE_W)
            {
                m_selectedLevel = 2;
                transitionToState(GameState::PLAYING);
            }
            else if (evt.key.keysym.scancode == SDL_SCANCODE_E)
            {
                transitionToState(GameState::HELP);
            }
        }

        // Help screen (UI2): any key → back to level select
        else if (evt.type == SDL_KEYDOWN && m_state == GameState::HELP)
            transitionToState(GameState::LEVEL_SELECT);

        // End screens → back to Level Select on any key
        if (evt.type == SDL_KEYDOWN &&
            (m_state == GameState::LEVEL_WIN || m_state == GameState::GAME_OVER))
            transitionToState(GameState::LEVEL_SELECT);
    }
}

// ── Per-step update ───────────────────────────────────────────────────────────
void Game::update(float dt)
{
    switch (m_state)
    {
    case GameState::SPLASH:
        m_splashTimer -= dt;
        if (m_splashTimer <= 0.0f)
            transitionToState(GameState::MENU);
        break;

    case GameState::MENU:
        break;  // Handled in processEvents

    case GameState::LEVEL_SELECT:
    case GameState::HELP:
        break;  // Input handled in processEvents, rendering in render()

    case GameState::PLAYING:
    {
        if (!m_scene) break;

        // Find player tank for input + level update
        PlayerTank* player = nullptr;
        for (auto* obj : m_scene->getObjects())
        {
            player = dynamic_cast<PlayerTank*>(obj);
            if (player) break;
        }

        if (player)
        {
            // Player is dead
            if (!player->isAlive())
            {
                transitionToState(GameState::GAME_OVER);
                break;
            }

            player->handleInput(dt, m_scene.get());

            // Pass scene to enemy AI (so they can fire into scene)
            for (auto* obj : m_scene->getObjects())
            {
                auto* enemy = dynamic_cast<EnemyTank*>(obj);
                if (enemy)
                {
                    if (!enemy->getTarget())
                        enemy->setTarget(player);
                    enemy->AI_update(dt, m_scene.get());
                }
            }
        }

        // Scene update (all entities) — flushPending() runs here
        m_scene->update(dt);

        // Collision detection — deaths are marked here
        s_collision.update(dt, m_scene.get());

        // Level manager update AFTER physics so the enemy recount sees
        // this frame's deaths (isAlive() == false) rather than last frame's state.
        if (player)
            m_levelManager->update(player->getPosition().x, m_scene.get());

        // Level complete?
        if (m_levelManager->isLevelComplete())
        {
            int nextLevel = m_levelManager->getCurrentLevel() + 1;
            if (nextLevel <= 2)
            {
                m_levelManager->spawnLevel(nextLevel, m_scene.get());
            }
            else
            {
                transitionToState(GameState::LEVEL_WIN);
            }
        }
        break;
    }

    case GameState::LEVEL_WIN:
    case GameState::GAME_OVER:
        // Wait for any key (handled in processEvents)
        break;
    }
}

// ── Render ────────────────────────────────────────────────────────────────────
void Game::render()
{
    // ImGui frame must be started every tick regardless of game state
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    // SDL clear
    SDL_SetRenderDrawColor(m_renderer, 30, 30, 30, 255);
    SDL_RenderClear(m_renderer);

    switch (m_state)
    {
    case GameState::SPLASH:
    {
        SDL_Texture* splash = IMG_LoadTexture(m_renderer, "splash_aut.png");
        if (splash)
        {
            SDL_RenderCopy(m_renderer, splash, nullptr, nullptr);
            SDL_DestroyTexture(splash);
        }
        break;
    }

    case GameState::MENU:
    {
        SDL_Texture* menu = IMG_LoadTexture(m_renderer, "menu_background.png");
        if (menu)
        {
            SDL_RenderCopy(m_renderer, menu, nullptr, nullptr);
            SDL_DestroyTexture(menu);
        }
        break;
    }

    case GameState::LEVEL_SELECT:
    {
        // Show UI1.png fullscreen — Q = Level 1, W = Level 2, E = Help
        SDL_Texture* ui1 = IMG_LoadTexture(m_renderer, "UI1.png");
        if (ui1)
        {
            SDL_RenderCopy(m_renderer, ui1, nullptr, nullptr);
            SDL_DestroyTexture(ui1);
        }
        break;
    }

    case GameState::HELP:
    {
        // Show UI2.png fullscreen — any key returns to level select
        SDL_Texture* ui2 = IMG_LoadTexture(m_renderer, "UI2.png");
        if (ui2)
        {
            SDL_RenderCopy(m_renderer, ui2, nullptr, nullptr);
            SDL_DestroyTexture(ui2);
        }
        break;
    }

    case GameState::PLAYING:
    {
        // Find player first so we can compute the camera offset
        PlayerTank* player = nullptr;
        if (m_scene)
        {
            for (auto* obj : m_scene->getObjects())
            {
                player = dynamic_cast<PlayerTank*>(obj);
                if (player) break;
            }
        }

        // Camera: centre on player, clamped to world bounds
        float cameraX = 0.0f;
        if (player)
        {
            cameraX = player->getPosition().x - Constants::SCREEN_W * 0.5f;
            cameraX = std::max(0.0f, std::min(cameraX,
                               Constants::WORLD_W - (float)Constants::SCREEN_W));
        }

        // Background: render at world position so it scrolls with the camera
        SDL_Texture* bg = IMG_LoadTexture(m_renderer, "game_background.png");
        if (bg)
        {
            int texW, texH;
            SDL_QueryTexture(bg, nullptr, nullptr, &texW, &texH);
            SDL_Rect dst = { -(int)cameraX, 0, texW, texH };
            SDL_RenderCopy(m_renderer, bg, nullptr, &dst);
            SDL_DestroyTexture(bg);
        }

        // Scene (all game objects)
        if (m_scene) m_scene->render(m_renderer, cameraX);

        if (m_ui)
        {
            m_ui->renderHUD(m_renderer, player, m_levelManager->getEnemiesRemaining());
            m_ui->renderMinimap(m_renderer, m_scene.get(), player);
            m_ui->renderDebugOverlay(player);
        }
        break;
    }

    case GameState::LEVEL_WIN:
    {
        SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);  // black fallback
        SDL_RenderClear(m_renderer);
        if (m_ui) m_ui->renderEndScreen(true);
        break;
    }

    case GameState::GAME_OVER:
    {
        SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);  // black fallback
        SDL_RenderClear(m_renderer);
        if (m_ui) m_ui->renderEndScreen(false);
        break;
    }
    }

    // Flush ImGui draw data on top of everything
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    SDL_RenderPresent(m_renderer);
}

// ── Fullscreen toggle ─────────────────────────────────────────────────────────
void Game::toggleFullscreen()
{
    m_fullscreen = !m_fullscreen;
    SDL_SetWindowFullscreen(m_window,
        m_fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
}

// ── State transitions ─────────────────────────────────────────────────────────
void Game::transitionToState(GameState next)
{
    m_state = next;

    if (next == GameState::PLAYING)
    {
        // Fresh scene for the selected level
        m_scene = std::make_unique<Scene>();

        // Spawn player tank
        auto* player = new PlayerTank({ 120.0f, 360.0f });
        m_scene->addObject(player);

        // Spawn the level chosen on the select screen
        m_levelManager->spawnLevel(m_selectedLevel, m_scene.get());
    }
    else if (next == GameState::SPLASH)
    {
        m_splashTimer = Constants::SPLASH_DURATION;
    }
}
