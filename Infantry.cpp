// COMP710 - KineticImpact
// Infantry.cpp - Auto-follow + auto-fire FSM; T-key repair mode
// TDD §3.5 / §4.4 / §6.4

#include "Infantry.h"
#include "Tank.h"
#include "Bullet.h"
#include "Scene.h"
#include <SDL.h>
#include <SDL_image.h>
#include <cmath>
#include <algorithm>

Infantry::Infantry(Vec2 position, Faction faction)
    : GameObject(position, { 32.0f, 32.0f }, faction)
{
}

Infantry::~Infantry()
{
    if (m_texture) SDL_DestroyTexture(m_texture);
}

// ── Update: run FSM (TDD §4.4) ────────────────────────────────────────────────
void Infantry::update(float dt)
{
    if (m_attackCooldown > 0.0f)
        m_attackCooldown -= dt;

    if (!m_scene) { m_state = AIState::IDLE; return; }

    // Repair mode: friendly infantry, T held, tank stationary, in range (TDD §3.5)
    if (m_repairMode && m_followTarget && m_followTarget->isAlive())
    {
        float dist  = m_position.distance(m_followTarget->getPosition());
        bool  still = (m_followTarget->getVelocity().length() < 0.1f);
        if (still && dist < Constants::REPAIR_RANGE)
        {
            m_state = AIState::REPAIR;
            repairTank(dt);
            return;
        }
    }

    // Auto-assign follow target: friendly infantry scans for the nearest friendly tank
    if (m_faction == Faction::PLAYER && (!m_followTarget || !m_followTarget->isAlive()))
        m_followTarget = findNearestFriendlyTank();

    // Find nearest enemy within attack range
    m_attackTarget = findNearestEnemy(m_scene, Constants::INFANTRY_ATTACK_R);

    // ATTACK: enemy is in range → shoot and stay put (TDD §4.4)
    if (m_attackTarget && m_attackTarget->isAlive())
    {
        m_state = AIState::ATTACK;
        shootAt(m_attackTarget, m_scene);
        return;
    }

    // FOLLOW / advance toward goal
    GameObject* goal     = nullptr;
    float       stopDist = 0.0f;

    if (m_faction == Faction::PLAYER)
    {
        // Follow friendly tank; stop when within INFANTRY_FOLLOW_R
        goal     = m_followTarget;
        stopDist = Constants::INFANTRY_FOLLOW_R;
    }
    else
    {
        // Enemy infantry: advance toward nearest player-faction entity (unlimited range)
        goal     = findNearestEnemy(m_scene, 1.0e30f);
        stopDist = Constants::INFANTRY_ATTACK_R * 0.75f;
    }

    if (goal && goal->isAlive())
    {
        // Friendly infantry aim for their formation slot (tank pos + offset).
        // Enemy infantry aim straight at the goal with no offset.
        Vec2 targetPos = goal->getPosition();
        if (m_faction == Faction::PLAYER)
            targetPos = targetPos + m_formationOffset;

        float dist = m_position.distance(targetPos);
        if (dist > stopDist)
        {
            m_state = AIState::FOLLOW;
            Vec2 dir = (targetPos - m_position).normalised();
            m_position += dir * (Constants::INFANTRY_SPEED * dt);
            return;
        }
    }

    m_state = AIState::IDLE;
}

void Infantry::render(SDL_Renderer* renderer, float cameraX)
{
    if (!m_texture)
    {
        const char* path = (m_faction == Faction::PLAYER)
            ? "infantry_friendly.png"
            : "infantry_enemy.png";
        m_texture = IMG_LoadTexture(renderer, path);
    }

    int sx = (int)(m_position.x - cameraX);
    int sy = (int)m_position.y;

    SDL_Rect dst = { sx - 16, sy - 16, 32, 32 };

    if (m_texture)
    {
        SDL_RenderCopy(renderer, m_texture, nullptr, &dst);
    }
    else
    {
        Uint8 r = (m_faction == Faction::PLAYER) ? 50  : 180;
        Uint8 g = (m_faction == Faction::PLAYER) ? 180 : 50;
        SDL_SetRenderDrawColor(renderer, r, g, 50, 255);
        SDL_RenderFillRect(renderer, &dst);
    }

    if (m_state == AIState::REPAIR)
    {
        SDL_SetRenderDrawColor(renderer, 0, 255, 100, 200);
        SDL_Rect bar = { dst.x, dst.y - 6, 32, 4 };
        SDL_RenderFillRect(renderer, &bar);
    }
}

void Infantry::onCollision(GameObject* /*other*/)
{
    // Handled by CollisionManager / Bullet::onHit
}

// ── Repair (TDD §3.5 / §6.4) ─────────────────────────────────────────────────
// Rate: 1% of section maxHp per second; priority: destroyed sections first
void Infantry::repairTank(float dt)
{
    if (!m_followTarget) return;

    ArmorSection* target = m_followTarget->getMostDamagedSection();
    if (!target) return;

    int heal = static_cast<int>(target->maxHp * Constants::REPAIR_RATE * dt);
    if (heal < 1) heal = 1;

    target->hp = std::min(target->maxHp, target->hp + heal);

    // Restore destroyed flag once HP is above zero (TDD §3.5)
    if (target->hp > 0 && target->destroyed)
        target->destroyed = false;
}

// ── Shoot (creates a Bullet toward target) ────────────────────────────────────
void Infantry::shootAt(GameObject* target, Scene* scene)
{
    if (!target || !scene)      return;
    if (m_attackCooldown > 0.0f) return;

    Vec2 dir = (target->getPosition() - m_position).normalised();
    scene->addObject(new Bullet(m_position, dir, m_faction));
    m_attackCooldown = Constants::INFANTRY_COOLDOWN;
}

// ── Private helpers ───────────────────────────────────────────────────────────
GameObject* Infantry::findNearestEnemy(Scene* scene, float maxDist) const
{
    if (!scene) return nullptr;

    GameObject* nearest = nullptr;
    float       minDist = maxDist;
    Faction     enemy   = (m_faction == Faction::PLAYER) ? Faction::ENEMY : Faction::PLAYER;

    for (auto* obj : scene->getObjects())
    {
        if (!obj->isAlive() || obj->getFaction() != enemy) continue;
        float d = m_position.distance(obj->getPosition());
        if (d < minDist) { minDist = d; nearest = obj; }
    }
    return nearest;
}

Tank* Infantry::findNearestFriendlyTank() const
{
    if (!m_scene) return nullptr;

    Tank*  nearest = nullptr;
    float  minDist = 1.0e30f;

    for (auto* obj : m_scene->getObjects())
    {
        if (!obj->isAlive() || obj->getFaction() != m_faction) continue;
        Tank* tank = dynamic_cast<Tank*>(obj);
        if (!tank) continue;
        float d = m_position.distance(tank->getPosition());
        if (d < minDist) { minDist = d; nearest = tank; }
    }
    return nearest;
}
