// COMP710 - KineticImpact
// Bullet.cpp - Machine-gun round: one-shots infantry, zero damage to tanks (TDD §9.1)

#include "Bullet.h"
#include "Infantry.h"
#include "Tank.h"

Bullet::Bullet(Vec2 position, Vec2 direction, Faction faction)
    : Projectile(position, direction, ProjectileType::BULLET,
                 0.0f,                          // penetration = 0, cannot damage tanks
                 Constants::BULLET_SPEED,
                 Constants::BULLET_MAX_RANGE,
                 faction)
{
    // Bullets are smaller than cannon shells — restore the original 6x6 hitbox
    m_size = { 6.0f, 6.0f };
}

void Bullet::onHit(GameObject* target)
{
    if (!target) return;

    // Bullets only damage infantry - tanks are immune (TDD §9.1 / Constants::BULLET_DMG = 0)
    Infantry* inf = dynamic_cast<Infantry*>(target);
    if (inf && inf->getFaction() != m_faction)
    {
        inf->markDead();   // One-shot kill (BULLET_INF_DMG = 1, infantry has 1 HP)
    }
    // No-op for tanks: BULLET_DMG = 0
}

void Bullet::onCollision(GameObject* other)
{
    if (!other || !other->isAlive()) return;
    if (other->getFaction() == m_faction) return;  // No friendly fire
    onHit(other);
    markDead();
}
