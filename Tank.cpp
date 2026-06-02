// COMP710 - KineticImpact
// Tank.cpp - Dual ArmorSection, reload state machine, Lock-HP  (TDD §3.1-3.4 / §6.1-6.3)

#include "Tank.h"
#include "Scene.h"
#include "Bullet.h"
#include "APProjectile.h"
#include "HEProjectile.h"
#include "ParticleEffect.h"
#include <cmath>
#include <algorithm>
#include <SDL.h>

// ── Constructor ───────────────────────────────────────────────────────────────
Tank::Tank(Vec2 position, Vec2 size, Faction faction)
    : GameObject(position, size, faction)
{
    // Initialise hull and turret armour sections (TDD §3.3 / §10)
    m_hull.armor  = Constants::HULL_ARMOR;
    m_hull.maxHp  = Constants::PLAYER_MAX_HP;
    m_hull.hp     = Constants::PLAYER_MAX_HP;

    m_turret.armor  = Constants::TURRET_ARMOR;
    m_turret.maxHp  = Constants::PLAYER_MAX_HP;
    m_turret.hp     = Constants::PLAYER_MAX_HP;
}

// ── Update ────────────────────────────────────────────────────────────────────
void Tank::update(float dt)
{
    // Reload state machine (TDD §3.4 / §6.3)
    if (m_isReloading)
    {
        m_reloadTimer -= dt;
        if (m_reloadTimer <= 0.0f)
        {
            m_reloadTimer  = 0.0f;
            m_isReloading  = false;
            m_shellLoaded  = true;
        }
    }

    // Machine-gun cooldown
    if (m_mgCooldown > 0.0f)
        m_mgCooldown -= dt;

    // Hull destroyed: freeze movement (TDD §3.3)
    if (m_hull.destroyed)
        m_velocity = Vec2::zero();

    // Apply velocity
    m_position += m_velocity * dt;

    // Either section fully penetrated → tank knocked out (TDD §3.3)
    if (m_hull.destroyed || m_turret.destroyed)
        markDead();
}

void Tank::render(SDL_Renderer* renderer, float cameraX)
{
    int sx = (int)(m_position.x - cameraX);
    int sy = (int)m_position.y;

    Uint8 r = (m_faction == Faction::PLAYER) ? 30  : 80;
    Uint8 g = (m_faction == Faction::PLAYER) ? 100 : 30;
    Uint8 b = (m_faction == Faction::PLAYER) ? 30  : 30;
    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
    SDL_Rect hull = {
        sx - (int)(m_size.x * 0.5f),
        sy - (int)(m_size.y * 0.5f),
        (int)m_size.x, (int)m_size.y
    };
    SDL_RenderFillRect(renderer, &hull);

    SDL_SetRenderDrawColor(renderer, r + 40, g + 40, b + 40, 255);
    int tw = (int)(m_size.x * 0.5f);
    int th = (int)(m_size.y * 0.5f);
    SDL_Rect turret = { sx - tw / 2, sy - th / 2, tw, th };
    SDL_RenderFillRect(renderer, &turret);
}

void Tank::onCollision(GameObject* /*other*/)
{
    // Handled by CollisionManager and Projectile::onCollision
}

// ── Combat: angle-based penetration + Lock-HP spall  (TDD §3.1 / §3.2 / §6.1) ───
void Tank::takeDamage(float pen, float angleDeg, bool hitTurret, float spallPct)
{
    ArmorSection& section = hitTurret ? m_turret : m_hull;
    if (section.destroyed) return;

    // cos(theta): clamped to RICOCHET_THRESHOLD (TDD §6.1)
    float rad      = angleDeg * (3.14159265f / 180.0f);
    float cosTheta = std::max(Constants::RICOCHET_THRESHOLD, std::fabsf(std::cosf(rad)));

    // Near-parallel shot -> ricochet, zero damage
    if (cosTheta <= Constants::RICOCHET_THRESHOLD) return;

    float effectiveArmor = section.armor / cosTheta;

    if (pen >= effectiveArmor)
    {
        // Full penetration: section destroyed (TDD §3.1)
        section.hp        = 0;
        section.destroyed = true;

        // Hull destroyed -> immobilised (TDD §3.3)
        if (!hitTurret) m_velocity = Vec2::zero();
    }
    else
    {
        // Lock-HP spall: cannot reduce below 1% maxHp (TDD §3.2 / §6.2)
        int spallDmg = static_cast<int>(section.maxHp * spallPct);
        section.applySpall(spallDmg);
    }
}

// ── Reload state machine  (TDD §3.4 / §6.3) ──────────────────────────────────
void Tank::startReload()
{
    // Only reload if shell has been fired and we are not already reloading
    if (!m_shellLoaded && !m_isReloading)
    {
        m_isReloading = true;
        m_reloadTimer = Constants::RELOAD_TIME;
    }
}

void Tank::onHit(ProjectileType type)
{
    // Any AP or HE hit during reload resets the timer (TDD §3.4)
    if (m_isReloading && type != ProjectileType::BULLET)
        m_reloadTimer = Constants::RELOAD_TIME;
}

// ── Fire machine gun  ─────────────────────────────────────────────────────────
void Tank::fireMachineGun(Vec2 dir, Scene* scene)
{
    if (m_mgCooldown > 0.0f) return;
    if (m_turret.destroyed)  return;   // No turret -> cannot fire

    scene->addObject(new Bullet(m_position, dir.normalised(), m_faction));
    m_mgCooldown = Constants::MG_COOLDOWN;
}

// ── Fire cannon ───────────────────────────────────────────────────────────────
void Tank::fireCannon(Vec2 target, Scene* scene)
{
    if (!m_shellLoaded)      return;   // Must reload first (TDD §9.1)
    if (m_turret.destroyed)  return;   // Turret destroyed -> cannot fire

    Vec2 dir = (target - m_position).normalised();

    if (m_selectedAmmo == AmmoType::AP)
    {
        if (m_apAmmo <= 0) return;     // Out of AP ammo (TDD §9.2)
        scene->addObject(new APProjectile(m_position, dir, m_faction));
        --m_apAmmo;
    }
    else
    {
        if (m_heAmmo <= 0) return;     // Out of HE ammo (TDD §9.2)
        auto* he = new HEProjectile(m_position, dir, m_faction);
        he->setScene(scene);
        scene->addObject(he);
        --m_heAmmo;
    }

    // Muzzle-flash particle burst at the barrel tip (~50 px ahead of tank centre)
    Vec2 muzzle = m_position + dir * 50.0f;
    scene->addObject(new ParticleEffect(muzzle, dir));

    m_shellLoaded = false;
}

// ── Accessors ─────────────────────────────────────────────────────────────────
float Tank::getReloadProgress() const
{
    if (!m_isReloading) return m_shellLoaded ? 1.0f : 0.0f;
    return 1.0f - (m_reloadTimer / Constants::RELOAD_TIME);
}

// Priority: destroyed sections first; then lowest HP%  (TDD §3.5)
ArmorSection* Tank::getMostDamagedSection()
{
    if (m_hull.destroyed)   return &m_hull;
    if (m_turret.destroyed) return &m_turret;

    float hullPct   = (float)m_hull.hp   / (float)m_hull.maxHp;
    float turretPct = (float)m_turret.hp / (float)m_turret.maxHp;
    return (hullPct <= turretPct) ? &m_hull : &m_turret;
}
