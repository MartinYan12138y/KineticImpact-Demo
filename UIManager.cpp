// COMP710 - KineticImpact
// UIManager.cpp - HUD (dual HP bars, reload bar, ammo type) + minimap + ImGui overlay
// TDD §3.7 / §5.3

#include "UIManager.h"
#include "Scene.h"
#include "Tank.h"
#include "imgui.h"
#include <SDL.h>
#include <SDL_image.h>
#include <algorithm>

void UIManager::initialise(SDL_Renderer* renderer)
{
    m_renderer = renderer;
}

// ── Full HUD ──────────────────────────────────────────────────────────────────
void UIManager::renderHUD(SDL_Renderer* renderer, const Tank* player, int enemiesRemaining)
{
    if (!player) return;

    const int BAR_W  = 200;
    const int BAR_H  = 16;
    const int LEFT   = 10;
    const int TOP    = 10;
    const int STRIDE = 24;

    // ── Hull HP bar ───────────────────────────────────────────────────────────
    {
        const ArmorSection& h = player->getHull();
        float pct = (h.maxHp > 0) ? (float)h.hp / h.maxHp : 0.0f;

        // Background
        SDL_Rect bg = { LEFT, TOP, BAR_W, BAR_H };
        SDL_SetRenderDrawColor(renderer, 60, 60, 60, 200);
        SDL_RenderFillRect(renderer, &bg);

        // Fill (green -> red based on HP%)
        Uint8 r = (Uint8)std::min(255.0f, (1.0f - pct) * 510.0f);
        Uint8 g = (Uint8)std::min(255.0f, pct * 510.0f);
        SDL_Rect fill = { LEFT, TOP, (int)(BAR_W * pct), BAR_H };
        SDL_SetRenderDrawColor(renderer, r, g, 30, 255);
        SDL_RenderFillRect(renderer, &fill);

        // Destroyed indicator
        if (h.destroyed)
        {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            SDL_RenderDrawRect(renderer, &bg);
        }
    }

    // ── Turret HP bar ─────────────────────────────────────────────────────────
    {
        const ArmorSection& t = player->getTurret();
        float pct = (t.maxHp > 0) ? (float)t.hp / t.maxHp : 0.0f;

        SDL_Rect bg = { LEFT, TOP + STRIDE, BAR_W, BAR_H };
        SDL_SetRenderDrawColor(renderer, 60, 60, 60, 200);
        SDL_RenderFillRect(renderer, &bg);

        Uint8 r = (Uint8)std::min(255.0f, (1.0f - pct) * 510.0f);
        Uint8 g = (Uint8)std::min(255.0f, pct * 510.0f);
        SDL_Rect fill = { LEFT, TOP + STRIDE, (int)(BAR_W * pct), BAR_H };
        SDL_SetRenderDrawColor(renderer, r, g, 80, 255);
        SDL_RenderFillRect(renderer, &fill);

        if (t.destroyed)
        {
            SDL_SetRenderDrawColor(renderer, 255, 100, 0, 255);
            SDL_RenderDrawRect(renderer, &bg);
        }
    }

    // ── Reload progress bar ───────────────────────────────────────────────────
    if (player->isReloading())
    {
        float prog = player->getReloadProgress();
        SDL_Rect bg   = { LEFT, TOP + STRIDE * 2, BAR_W, BAR_H };
        SDL_Rect fill = { LEFT, TOP + STRIDE * 2, (int)(BAR_W * prog), BAR_H };
        SDL_SetRenderDrawColor(renderer, 40, 40, 80, 200);
        SDL_RenderFillRect(renderer, &bg);
        SDL_SetRenderDrawColor(renderer, 80, 120, 255, 255);
        SDL_RenderFillRect(renderer, &fill);
    }

    // ── Ammo type indicator + remaining count (TDD §9.2) ─────────────────────
    {
        AmmoType ammo    = player->getSelectedAmmo();
        int      apAmmo  = player->getApAmmo();
        int      heAmmo  = player->getHeAmmo();

        // Coloured square showing selected ammo type
        SDL_Rect dot = { LEFT + BAR_W + 8, TOP, 16, 16 };
        if (ammo == AmmoType::AP)
            SDL_SetRenderDrawColor(renderer, 80, 180, 255, 255);   // blue = AP
        else
            SDL_SetRenderDrawColor(renderer, 255, 140, 0, 255);    // orange = HE
        SDL_RenderFillRect(renderer, &dot);

        // AP shell count: blue squares, row 1
        for (int i = 0; i < Constants::AP_AMMO_MAX; ++i)
        {
            SDL_Rect sq = { LEFT + BAR_W + 8 + i * 10, TOP + 22, 7, 7 };
            if (i < apAmmo)
                SDL_SetRenderDrawColor(renderer, 80, 180, 255, 255);  // filled
            else
                SDL_SetRenderDrawColor(renderer, 40, 70, 100, 180);   // empty/used
            SDL_RenderFillRect(renderer, &sq);
        }

        // HE shell count: orange squares, row 2
        for (int i = 0; i < Constants::HE_AMMO_MAX; ++i)
        {
            SDL_Rect sq = { LEFT + BAR_W + 8 + i * 10, TOP + 34, 7, 7 };
            if (i < heAmmo)
                SDL_SetRenderDrawColor(renderer, 255, 140, 0, 255);   // filled
            else
                SDL_SetRenderDrawColor(renderer, 90, 50, 10, 180);    // empty/used
            SDL_RenderFillRect(renderer, &sq);
        }
    }

    // ── Enemy count (bottom-left simple) ─────────────────────────────────────
    // (Text rendering requires SDL_ttf; shown as red squares for now)
    for (int i = 0; i < std::min(enemiesRemaining, 20); ++i)
    {
        SDL_Rect dot = { LEFT + i * 12, Constants::SCREEN_H - 20, 8, 8 };
        SDL_SetRenderDrawColor(renderer, 220, 50, 50, 255);
        SDL_RenderFillRect(renderer, &dot);
    }
}

// ── Minimap (TDD §3.7 / §5.3) ────────────────────────────────────────────────
// View window = SCREEN_W x SCREEN_H world pixels, centred on the player tank.
void UIManager::renderMinimap(SDL_Renderer* renderer, Scene* scene, const Tank* player)
{
    if (!scene) return;

    // Background + border
    SDL_Rect bg = { MM_X, MM_Y, MM_W, MM_H };
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
    SDL_RenderFillRect(renderer, &bg);
    SDL_SetRenderDrawColor(renderer, 120, 120, 120, 255);
    SDL_RenderDrawRect(renderer, &bg);

    // View area in world space: one screen-sized window centred on the player
    constexpr float VIEW_W = (float)Constants::SCREEN_W;   // 1280 px
    constexpr float VIEW_H = (float)Constants::SCREEN_H;   // 720  px
    constexpr float SCALE_X = (float)MM_W / VIEW_W;        // 160 / 1280 = 0.125
    constexpr float SCALE_Y = (float)MM_H / VIEW_H;        // 90  / 720  = 0.125

    float centerX, centerY;
    if (player)
    {
        // Clamp so the view window never extends outside the world
        centerX = std::max(VIEW_W * 0.5f,
                    std::min(player->getPosition().x, Constants::WORLD_W - VIEW_W * 0.5f));
        centerY = std::max(VIEW_H * 0.5f,
                    std::min(player->getPosition().y, Constants::WORLD_H - VIEW_H * 0.5f));
    }
    else
    {
        // Fallback: full-world overview
        centerX = Constants::WORLD_W * 0.5f;
        centerY = Constants::WORLD_H * 0.5f;
    }

    float viewLeft = centerX - VIEW_W * 0.5f;
    float viewTop  = centerY - VIEW_H * 0.5f;

    // Entity dots
    for (auto* obj : scene->getObjects())
    {
        if (!obj->isAlive()) continue;

        int dotX = MM_X + (int)((obj->getPosition().x - viewLeft) * SCALE_X) - 1;
        int dotY = MM_Y + (int)((obj->getPosition().y - viewTop)  * SCALE_Y) - 1;

        // Clip dots outside the minimap rect
        if (dotX < MM_X || dotX + 3 > MM_X + MM_W) continue;
        if (dotY < MM_Y || dotY + 3 > MM_Y + MM_H) continue;

        SDL_Rect dot = { dotX, dotY, 3, 3 };

        if (obj->getFaction() == Faction::PLAYER)
            SDL_SetRenderDrawColor(renderer, 0, 220, 0, 255);    // bright green (TDD §5.3)
        else
            SDL_SetRenderDrawColor(renderer, 220, 0, 0, 255);    // red (TDD §5.3)

        SDL_RenderFillRect(renderer, &dot);
    }
}

// ── Victory / Game-Over end screen ───────────────────────────────────────────
// Renders a full-screen image (vic1ory_enemy.png / defeated_enemy.png) then
// overlays a centred "Press any key" prompt via ImGui.
void UIManager::renderEndScreen(bool victory)
{
    // ── Full-screen background image ──────────────────────────────────────────
    const char* imgPath = victory ? "vic1ory_enemy.png" : "defeated_enemy.png";
    SDL_Texture* bg = IMG_LoadTexture(m_renderer, imgPath);
    if (bg)
    {
        SDL_RenderCopy(m_renderer, bg, nullptr, nullptr);
        SDL_DestroyTexture(bg);
    }

    // ── "Press any key" subtitle overlay ─────────────────────────────────────
    ImVec4 txtCol = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);   // white

    ImGui::PushStyleColor(ImGuiCol_WindowBg,  ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_Border,    ImVec4(0, 0, 0, 0));
    ImGui::PushStyleVar (ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar (ImGuiStyleVar_WindowPadding,    ImVec2(0, 0));

    constexpr ImGuiWindowFlags kFlags =
        ImGuiWindowFlags_NoResize    | ImGuiWindowFlags_NoMove     |
        ImGuiWindowFlags_NoCollapse  | ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_AlwaysAutoResize;

    ImGui::SetNextWindowPos(
        ImVec2(Constants::SCREEN_W * 0.5f, Constants::SCREEN_H * 0.88f),
        ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::Begin("##endsub", nullptr, kFlags);
    ImGui::SetWindowFontScale(1.5f);
    ImGui::TextColored(txtCol, "Press any key to play again");
    ImGui::SetWindowFontScale(1.0f);
    ImGui::End();

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(2);
}

// ── Dear ImGui debug overlay (TDD §1.2) ──────────────────────────────────────
void UIManager::renderDebugOverlay(const Tank* player)
{
    if (!player) return;

    ImGui::SetNextWindowPos(ImVec2(Constants::SCREEN_W - 260.0f, 10.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(250.0f, 200.0f), ImGuiCond_Always);
    ImGui::Begin("Debug", nullptr,
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);

    const ArmorSection& hull   = player->getHull();
    const ArmorSection& turret = player->getTurret();

    ImGui::Text("Hull   | Armor: %.0fmm | HP: %d/%d %s",
        hull.armor, hull.hp, hull.maxHp, hull.destroyed ? "[DEST]" : "");
    ImGui::Text("Turret | Armor: %.0fmm | HP: %d/%d %s",
        turret.armor, turret.hp, turret.maxHp, turret.destroyed ? "[DEST]" : "");

    ImGui::Separator();
    ImGui::Text("Reload: %.2fs  Loaded: %s",
        player->isReloading() ? Constants::RELOAD_TIME * (1.0f - player->getReloadProgress()) : 0.0f,
        player->isShellLoaded() ? "YES" : "NO");

    ImGui::Text("Ammo: %s",
        player->getSelectedAmmo() == AmmoType::AP ? "AP (105mm)" : "HE (60mm)");

    ImGui::Text("Vel: (%.1f, %.1f)", player->getVelocity().x, player->getVelocity().y);

    ImGui::End();
}
