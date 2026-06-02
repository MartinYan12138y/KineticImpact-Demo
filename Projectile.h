#pragma once
// COMP710 - KineticImpact
// Projectile.h - Base projectile: type, penetration, range limit  (TDD §3.8 / §4.1 / §4.3)

#include "GameObject.h"
#include "Types.h"
#include "Vec2.h"

class Projectile : public GameObject
{
public:
    Projectile(Vec2 position, Vec2 direction, ProjectileType type,
               float penetration, float speed, float maxRange, Faction faction);
    virtual ~Projectile() = default;

    void update(float dt) override;
    void render(SDL_Renderer* renderer, float cameraX) override;
    void onCollision(GameObject* other) override;

    // Override in derived classes to apply damage / splash (TDD §4.3)
    virtual void onHit(GameObject* target) = 0;

    // ── Accessors ──────────────────────────────────────────────────────────────
    ProjectileType getType()        const { return m_type; }
    float          getPenetration() const { return m_penetration; }

protected:
    ProjectileType m_type;
    float          m_penetration;          // mm – used by Tank::takeDamage
    float          m_speed;               // px / s
    float          m_maxRange;            // px – destroy when exceeded (TDD §3.8 / §6.6)
    float          m_distanceTravelled = 0.0f;
    Vec2           m_direction;           // Unit vector (normalised at construction)
};
