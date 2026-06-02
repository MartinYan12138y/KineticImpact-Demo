// COMP710 - KineticImpact
// HEProjectile.cpp - HE shell: direct hit + radial splash with distance falloff
// TDD §6.2 / §6.7

#include "HEProjectile.h"
#include "Scene.h"
#include "Tank.h"
#include "Infantry.h"
#include <cmath>
#include <algorithm>

HEProjectile::HEProjectile(Vec2 position, Vec2 direction, Faction faction)
    : Projectile(position, direction, ProjectileType::HE,
                 Constants::HE_PENETRATION,
                 Constants::HE_SHELL_SPEED,
                 Constants::HE_MAX_RANGE,
                 faction)
    , m_explosionRadius(Constants::HE_RADIUS)
{
}

void HEProjectile::onHit(GameObject* target)
{
    // HE damage is handled entirely through explode()
    if (m_scene) explode(m_position, m_scene);
}

void HEProjectile::onCollision(GameObject* other)
{
    if (!other || !other->isAlive()) return;
    if (other->getFaction() == m_faction) return;  // No friendly fire

    // Trigger penetration check on the direct-hit target, then explode
    Tank* tank = dynamic_cast<Tank*>(other);
    if (tank)
    {
        // Compute impact angle for the direct hit
        Vec2  tankForward = Vec2::fromAngleDeg(tank->getBodyAngle());
        float dot         = m_direction.dot(tankForward);
        float impactDeg   = std::acos(std::max(-1.0f, std::min(1.0f, dot)))
                            * (180.0f / 3.14159265f);
        bool  hitTurret   = (impactDeg > 45.0f && impactDeg < 135.0f);

        // HE spall = 20% on non-penetration (TDD §6.2)
        tank->takeDamage(m_penetration, impactDeg, hitTurret, Constants::HE_SPALL_PCT);
        tank->onHit(ProjectileType::HE);
    }

    // Radial splash (TDD §6.7) - also damages the direct-hit target's neighbours
    if (m_scene) explode(m_position, m_scene);

    markDead();
}

// TDD §6.7: linear distance falloff within explosionRadius
void HEProjectile::explode(Vec2 centre, Scene* scene)
{
    for (auto* obj : scene->getObjects())
    {
        if (!obj->isAlive()) continue;
        if (obj->getFaction() == m_faction) continue;  // No friendly fire on splash

        float dist = centre.distance(obj->getPosition());
        if (dist >= m_explosionRadius) continue;

        // Scale: 1.0 at centre, 0.0 at edge
        float scale = 1.0f - dist / m_explosionRadius;
        int   dmg   = static_cast<int>(Constants::HE_DMG * scale);

        Tank*    tank = dynamic_cast<Tank*>(obj);
        Infantry* inf = dynamic_cast<Infantry*>(obj);

        if (tank)
        {
            // Splash bypasses armour angle - apply flat spall to hull (Lock-HP applies)
            tank->getHull().applySpall(dmg);     // Note: needs non-const accessor
        }
        else if (inf)
        {
            // Any HE splash kills infantry instantly
            inf->markDead();
        }
    }
}
