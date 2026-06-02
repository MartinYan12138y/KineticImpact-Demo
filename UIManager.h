#pragma once
// COMP710 - KineticImpact
// UIManager.h - HUD (dual HP bars, reload bar, ammo type) + minimap dots + ImGui overlay
// TDD §3.7 / §4.1 / §5.3

#include "Types.h"
#include <SDL.h>

class Scene;
class Tank;

class UIManager
{
public:
    UIManager() = default;

    void initialise(SDL_Renderer* renderer);

    // ── Per-frame render calls ─────────────────────────────────────────────────
    // Full HUD: hull/turret HP bars, reload progress, ammo type, enemy count
    void renderHUD(SDL_Renderer* renderer, const Tank* playerTank, int enemiesRemaining);

    // Minimap: black background + coloured 3x3 dots per entity (TDD §3.7 / §5.3)
    // View is centred on the player tank when player != nullptr
    void renderMinimap(SDL_Renderer* renderer, Scene* scene, const Tank* player);

    // Dear ImGui overlay: armour values, penetration, reload timer (TDD §1.2)
    void renderDebugOverlay(const Tank* playerTank);

    // Victory / Game-Over end screen with "press any key" prompt
    void renderEndScreen(bool victory);

private:
    SDL_Renderer* m_renderer = nullptr;
    SDL_Texture*  m_font     = nullptr;   // SDL_ttf rendered font texture (optional)

    // ── Minimap layout constants (TDD §5.3) ───────────────────────────────────
    static constexpr int MM_X = Constants::MM_X;
    static constexpr int MM_Y = Constants::MM_Y;
    static constexpr int MM_W = Constants::MM_W;
    static constexpr int MM_H = Constants::MM_H;
};
