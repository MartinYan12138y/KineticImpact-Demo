#pragma once
// COMP710 - KineticImpact
// APProjectile.h - Armour-Piercing shell: angle-based penetration check  (TDD §3.1 / §6.1)

#include "Projectile.h"

class APProjectile : public Projectile
{
public:
    APProjectile(Vec2 position, Vec2 direction, Faction faction);

    // Calls Tank::takeDamage with penetration + impact angle (TDD §3.1)
    void onHit(GameObject* target) override;
    void onCollision(GameObject* other) override;
};
