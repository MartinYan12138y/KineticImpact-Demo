// COMP710 - KineticImpact
// EnemyTank.cpp - Seek player, maintain range, fire AP cannon  (TDD §4.1 / §9.1)

#include "EnemyTank.h"
#include "Scene.h"
#include "Wreck.h"
#include <SDL.h>
#include <SDL_image.h>
#include <cmath>

EnemyTank::EnemyTank(Vec2 position)
    : Tank(position, { 96.0f, 96.0f }, Faction::ENEMY)
{
    // Enemy tanks have slightly less HP
    m_hull.maxHp  = Constants::ENEMY_TANK_HP;
    m_hull.hp     = Constants::ENEMY_TANK_HP;
    m_turret.maxHp = Constants::ENEMY_TANK_HP;
    m_turret.hp    = Constants::ENEMY_TANK_HP;
}

EnemyTank::~EnemyTank()
{
    if (m_hullTex)   SDL_DestroyTexture(m_hullTex);
    if (m_turretTex) SDL_DestroyTexture(m_turretTex);
}

void EnemyTank::update(float dt)
{
    AI_update(dt, m_scene);   // Use cached scene pointer

    // Spawn wreck sprite the moment hull or turret is destroyed (once only)
    if (!m_wreckSpawned && m_scene && (m_hull.destroyed || m_turret.destroyed))
    {
        m_scene->addObject(new Wreck(m_position));
        m_wreckSpawned = true;
    }

    Tank::update(dt);  // Calls markDead() if a section is destroyed
}

void EnemyTank::render(SDL_Renderer* renderer, float cameraX)
{
    if (!m_hullTex)
        m_hullTex   = IMG_LoadTexture(renderer, "enemy_hull.png");
    if (!m_turretTex)
        m_turretTex = IMG_LoadTexture(renderer, "enemy_turret.png");

    int sx = (int)(m_position.x - cameraX);
    int sy = (int)m_position.y;

    SDL_Point centre = { 48, 48 };
    SDL_Rect  dst    = { sx - 48, sy - 48, 96, 96 };

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

// ── AI: seek player and shoot (TDD §9.1) ─────────────────────────────────────
void EnemyTank::AI_update(float dt, Scene* scene)
{
    if (scene) m_scene = scene;  // Cache so update() can access it
    if (!m_target || !m_target->isAlive()) return;

    Vec2  toTarget = m_target->getPosition() - m_position;
    float dist     = toTarget.length();
    Vec2  dir      = toTarget.normalised();

    // ── Hull: rotate toward and approach target ───────────────────────────────
    if (!m_hull.destroyed)
    {
        float targetAngle = dir.angleDeg();
        float angleDiff   = targetAngle - m_bodyAngle;

        // Normalise angle diff to [-180, 180]
        while (angleDiff >  180.0f) angleDiff -= 360.0f;
        while (angleDiff < -180.0f) angleDiff += 360.0f;

        float rotStep = Constants::TANK_ROT_SPEED * dt;
        if (std::fabsf(angleDiff) < rotStep)
            m_bodyAngle = targetAngle;
        else
            m_bodyAngle += (angleDiff > 0 ? rotStep : -rotStep);

        // Advance if outside preferred combat range
        if (dist > Constants::ENEMY_CANNON_RANGE * 0.7f)
            m_velocity = Vec2::fromAngleDeg(m_bodyAngle) * Constants::ENEMY_SPEED;
        else
            m_velocity = Vec2::zero();
    }

    // ── Turret: always aim at target ─────────────────────────────────────────
    if (!m_turret.destroyed)
        m_turretAngle = dir.angleDeg();

    // ── Cannon: fire when in range and cooled down ────────────────────────────
    m_cannonCooldown -= dt;
    if (dist <= Constants::ENEMY_CANNON_RANGE && m_cannonCooldown <= 0.0f && scene)
    {
        m_selectedAmmo = AmmoType::AP;  // Enemy always fires AP
        fireCannon(m_target->getPosition(), scene);
        m_cannonCooldown = Constants::ENEMY_CANNON_CD;
        startReload();   // Begin reloading after each shot
    }
}
