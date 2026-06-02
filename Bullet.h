#pragma once
// COMP710 - KineticImpact
// Bullet.h - Machine-gun round: damages infantry only, short range  (TDD §9.1)

#include "Projectile.h"

class Bullet : public Projectile
{
public:
    Bullet(Vec2 position, Vec2 direction, Faction faction);

    void onHit(GameObject* target) override;
    void onCollision(GameObject* other) override;
};
