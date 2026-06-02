// COMP710 - KineticImpact
// PlayerTank.cpp - WASD movement, mouse-aim turret, E reload, Q/E ammo switch, T repair
// TDD §9.1 / §3.4 / §3.5

#include "PlayerTank.h"
#include "Scene.h"
#include <SDL.h>
#include <SDL_image.h>
#include <cmath>

PlayerTank::PlayerTank(Vec2 position)
    : Tank(position, { 96.0f, 96.0f }, Faction::PLAYER)
{
}

PlayerTank::~PlayerTank()
{
    if (m_hullTex)   SDL_DestroyTexture(m_hullTex);
    if (m_turretTex) SDL_DestroyTexture(m_turretTex);
}

void PlayerTank::update(float dt)
{
    Tank::update(dt);  // Reload timer + velocity application
}

void PlayerTank::render(SDL_Renderer* renderer, float cameraX)
{
    if (!m_hullTex)
        m_hullTex   = IMG_LoadTexture(renderer, "player_hull.png");
    if (!m_turretTex)
        m_turretTex = IMG_LoadTexture(renderer, "player_turret.png");

    int sx = (int)(m_position.x - cameraX);
    int sy = (int)m_position.y;

    SDL_Point centre = { (int)(m_size.x * 0.5f), (int)(m_size.y * 0.5f) };
    SDL_Rect  dst    = {
        sx - (int)(m_size.x * 0.5f),
        sy - (int)(m_size.y * 0.5f),
        (int)m_size.x, (int)m_size.y
    };

    if (m_hullTex)
        SDL_RenderCopyEx(renderer, m_hullTex, nullptr, &dst,
                         m_bodyAngle, &centre, SDL_FLIP_NONE);
    else
        Tank::render(renderer, cameraX);

    if (m_turretTex)
    {
        SDL_Rect  tDst    = { sx - 40, sy - 40, 80, 80 };
        SDL_Point tCentre = { 40, 40 };
        SDL_RenderCopyEx(renderer, m_turretTex, nullptr, &tDst,
                         m_turretAngle, &tCentre, SDL_FLIP_NONE);
    }
}

// ── Input ─────────────────────────────────────────────────────────────────────
void PlayerTank::handleInput(float dt, Scene* scene)
{
    const Uint8* keys = SDL_GetKeyboardState(nullptr);

    // ── Hull movement (only if hull intact)  (TDD §3.3) ──────────────────────
    if (!m_hull.destroyed)
    {
        float forward = 0.0f;
        if (keys[SDL_SCANCODE_W]) forward =  Constants::TANK_SPEED;
        if (keys[SDL_SCANCODE_S]) forward = -Constants::TANK_SPEED * 0.6f;  // reverse slower

        if (keys[SDL_SCANCODE_A]) m_bodyAngle -= Constants::TANK_ROT_SPEED * dt;
        if (keys[SDL_SCANCODE_D]) m_bodyAngle += Constants::TANK_ROT_SPEED * dt;

        m_velocity = Vec2::fromAngleDeg(m_bodyAngle) * forward;
    }
    else
    {
        m_velocity = Vec2::zero();
    }

    // ── Turret mouse-aim (only if turret intact)  (TDD §9.1) ─────────────────
    // Convert screen mouse coords to world coords by adding the camera offset
    float camX = std::max(0.0f, std::min(
        m_position.x - Constants::SCREEN_W * 0.5f,
        Constants::WORLD_W - (float)Constants::SCREEN_W));

    if (!m_turret.destroyed)
    {
        int mx, my;
        SDL_GetMouseState(&mx, &my);
        Vec2 toMouse = Vec2((float)mx + camX, (float)my) - m_position;
        if (toMouse.length() > 1.0f)
            m_turretAngle = toMouse.angleDeg();
    }

    // ── Ammo selection  (TDD §9.1) ────────────────────────────────────────────
    if (keys[SDL_SCANCODE_1]) m_selectedAmmo = AmmoType::AP;
    if (keys[SDL_SCANCODE_2]) m_selectedAmmo = AmmoType::HE;

    // ── Manual reload: E key  (TDD §3.4 / §6.3) ──────────────────────────────
    if (keys[SDL_SCANCODE_E]) startReload();

    // ── Repair mode: T key  (TDD §3.5) ───────────────────────────────────────
    // toggleRepairMode() is called externally via key event (see Game::processEvents)

    // ── Firing ────────────────────────────────────────────────────────────────
    int  mouseX, mouseY;
    Uint32 mouse = SDL_GetMouseState(&mouseX, &mouseY);
    Vec2   mousePos((float)mouseX + camX, (float)mouseY);  // world space

    // Left click -> machine gun
    if (mouse & SDL_BUTTON(SDL_BUTTON_LEFT))
        fireMachineGun(Vec2::fromAngleDeg(m_turretAngle), scene);

    // Right click -> cannon
    if (mouse & SDL_BUTTON(SDL_BUTTON_RIGHT))
        fireCannon(mousePos, scene);
}

void PlayerTank::toggleRepairMode()
{
    m_repairMode = !m_repairMode;
}
