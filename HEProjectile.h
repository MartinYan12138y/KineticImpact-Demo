#pragma once
// COMP710 - KineticImpact
// HEProjectile.h - High-Explosive shell: splash damage with distance falloff
// TDD §6.2 / §6.7

#include "Projectile.h"

class Scene;

class HEProjectile : public Projectile
{
public:
    HEProjectile(Vec2 position, Vec2 direction, Faction faction);

    void onHit(GameObject* target) override;
    void onCollision(GameObject* other) override;

    // Radial splash: linear falloff over HE_RADIUS (TDD §6.7)
    void explode(Vec2 centre, Scene* scene);

    void setScene(Scene* scene) { m_scene = scene; }

private:
    float  m_explosionRadius;  // Initialised to Constants::HE_RADIUS
    Scene* m_scene = nullptr;  // Needed by explode() to iterate game objects
};
