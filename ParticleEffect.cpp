// COMP710 - KineticImpact
// ParticleEffect.cpp - Muzzle-flash particle burst (spawned on cannon fire)

#include "ParticleEffect.h"
#include <SDL.h>
#include <cmath>
#include <cstdlib>
#include <algorithm>

// Local helper: uniform float in [0, 1)
static float rf() { return (float)rand() / ((float)RAND_MAX + 1.0f); }

// ── Constructor: generate spark burst ────────────────────────────────────────
ParticleEffect::ParticleEffect(Vec2 origin, Vec2 fireDir, int count)
    : GameObject(origin, { 1.0f, 1.0f }, Faction::PLAYER)   // tiny hitbox – never collides
{
    m_particles.reserve(count);

    const float kSpread = 50.0f * (3.14159265f / 180.0f);   // ±50 degrees cone

    for (int i = 0; i < count; ++i)
    {
        // Random rotation within the spread cone
        float angle = (rf() - 0.5f) * 2.0f * kSpread;
        float cosA  = std::cosf(angle);
        float sinA  = std::sinf(angle);

        Vec2 dir = {
            fireDir.x * cosA - fireDir.y * sinA,
            fireDir.x * sinA + fireDir.y * cosA
        };

        Particle p;
        p.pos     = origin + dir * (rf() * 12.0f);  // slight forward scatter
        p.vel     = dir * (80.0f + rf() * 220.0f);  // 80–300 px/s
        p.maxLife = 0.10f + rf() * 0.25f;           // 0.10–0.35 s
        p.life    = p.maxLife;
        p.radius  = 2.0f + rf() * 3.0f;             // 2–5 px initial radius

        // Colour: white-hot -> yellow -> orange gradient
        int col = rand() % 3;
        if      (col == 0) { p.r = 255; p.g = 255; p.b = 220; }  // near-white
        else if (col == 1) { p.r = 255; p.g = 200; p.b =  30; }  // yellow
        else               { p.r = 255; p.g = 110; p.b =   0; }  // orange

        m_particles.push_back(p);
    }
}

// ── Update: move + drag + lifetime ───────────────────────────────────────────
void ParticleEffect::update(float dt)
{
    bool anyAlive = false;

    for (auto& p : m_particles)
    {
        if (p.life <= 0.0f) continue;

        p.pos  += p.vel * dt;
        p.vel  *= 0.88f;    // velocity drag per frame (independent of dt for simplicity)
        p.life -= dt;

        if (p.life > 0.0f) anyAlive = true;
    }

    if (!anyAlive) markDead();
}

// ── Render: alpha-blended squares that shrink as they fade ───────────────────
void ParticleEffect::render(SDL_Renderer* renderer, float cameraX)
{
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    for (const auto& p : m_particles)
    {
        if (p.life <= 0.0f) continue;

        float alpha = p.life / p.maxLife;                       // 1.0 -> 0.0
        Uint8 a     = (Uint8)(alpha * 255.0f);
        int   cx    = (int)(p.pos.x - cameraX);
        int   cy    = (int)(p.pos.y);
        int   r     = std::max(1, (int)(p.radius * alpha + 0.5f));  // shrinks over life

        SDL_SetRenderDrawColor(renderer, p.r, p.g, p.b, a);
        SDL_Rect rect = { cx - r, cy - r, r * 2, r * 2 };
        SDL_RenderFillRect(renderer, &rect);
    }

    // Restore default blend mode so nothing else is affected
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}
