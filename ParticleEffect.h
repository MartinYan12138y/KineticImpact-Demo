#pragma once
// COMP710 - KineticImpact
// ParticleEffect.h - Short-lived muzzle-flash particle burst spawned on cannon fire

#include "GameObject.h"
#include "Vec2.h"
#include <vector>

// ── Single spark particle ─────────────────────────────────────────────────────
struct Particle
{
    Vec2  pos;
    Vec2  vel;
    float life;       // remaining lifetime (seconds)
    float maxLife;    // total lifetime (seconds)
    Uint8 r, g, b;   // base colour
    float radius;     // initial pixel radius (shrinks as life fades)
};

// ── Burst effect (not a Projectile – collision system ignores it) ─────────────
class ParticleEffect : public GameObject
{
public:
    // origin  = world-space muzzle position
    // fireDir = normalised direction the cannon fired
    // count   = number of sparks (default 12)
    ParticleEffect(Vec2 origin, Vec2 fireDir, int count = 12);

    void update(float dt)                              override;
    void render(SDL_Renderer* renderer, float cameraX) override;
    void onCollision(GameObject* /*other*/)            override {}  // cosmetic only

private:
    std::vector<Particle> m_particles;
};
