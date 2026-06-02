// COMP710 - KineticImpact
// APProjectile.cpp - AP shell: angle-based penetration + Lock-HP spall  (TDD §3.1 / §6.1)

#include "APProjectile.h"
#include "Tank.h"
#include <cmath>
#include <algorithm>

APProjectile::APProjectile(Vec2 position, Vec2 direction, Faction faction)
    : Projectile(position, direction, ProjectileType::AP,
                 Constants::AP_PENETRATION,
                 Constants::AP_SHELL_SPEED,
                 Constants::AP_MAX_RANGE,
                 faction)
{
}

void APProjectile::onHit(GameObject* target)
{
    if (!target) return;

    Tank* tank = dynamic_cast<Tank*>(target);
    if (!tank) return;

    // ── Determine hit section (TDD §3.3) ─────────────────────────────────────
    // Impact angle relative to tank body forward: ±45° = hull, otherwise = turret
    Vec2  toTarget   = (tank->getPosition() - m_position).normalised();
    Vec2  tankForward = Vec2::fromAngleDeg(tank->getBodyAngle());
    float dot        = m_direction.dot(tankForward);
    float impactDeg  = std::acos(std::max(-1.0f, std::min(1.0f, dot)))
                       * (180.0f / 3.14159265f);

    bool hitTurret = (impactDeg > 45.0f && impactDeg < 135.0f);

    // ── Apply damage (AP spall = 10%) (TDD §3.1 / §6.2) ─────────────────────
    tank->takeDamage(m_penetration, impactDeg, hitTurret, Constants::AP_SPALL_PCT);

    // ── Reload interrupt (TDD §3.4) ───────────────────────────────────────────
    tank->onHit(ProjectileType::AP);
}

void APProjectile::onCollision(GameObject* other)
{
    if (!other || !other->isAlive()) return;
    if (other->getFaction() == m_faction) return;  // No friendly fire
    onHit(other);
    markDead();
}
