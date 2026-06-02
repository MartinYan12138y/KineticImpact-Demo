// COMP710 - KineticImpact
// Projectile.cpp - Base movement + simplified range limit  (TDD §3.8 / §6.6)

#include "Projectile.h"
#include <SDL.h>

Projectile::Projectile(Vec2 position, Vec2 direction, ProjectileType type,
                       float penetration, float speed, float maxRange, Faction faction)
    : GameObject(position, { 18.0f, 12.0f }, faction)
    , m_type(type)
    , m_penetration(penetration)
    , m_speed(speed)
    , m_maxRange(maxRange)
    , m_distanceTravelled(0.0f)
    , m_direction(direction.normalised())
{
}

void Projectile::update(float dt)
{
    // Move along direction vector
    Vec2 delta = m_direction * (m_speed * dt);
    m_position += delta;

    // Simplified range limit (TDD §3.8 / §6.6)
    m_distanceTravelled += delta.length();
    if (m_distanceTravelled > m_maxRange)
        markDead();
}

void Projectile::render(SDL_Renderer* renderer, float cameraX)
{
    // Colour by type
    if (m_type == ProjectileType::BULLET)
        SDL_SetRenderDrawColor(renderer, 255, 255, 100, 255);   // yellow
    else if (m_type == ProjectileType::AP)
        SDL_SetRenderDrawColor(renderer, 80, 180, 255, 255);    // blue-white
    else
        SDL_SetRenderDrawColor(renderer, 255, 140, 0, 255);     // orange (HE)

    // Screen-space centre
    float cx = m_position.x - cameraX;
    float cy = m_position.y;

    // Half-length along travel direction:
    //   bullet: unchanged (4 px)  |  AP/HE shell: 3x longer (8 -> 24 px)
    float half = (m_type == ProjectileType::BULLET) ? 4.0f : 24.0f;

    int x1 = (int)(cx - m_direction.x * half);
    int y1 = (int)(cy - m_direction.y * half);
    int x2 = (int)(cx + m_direction.x * half);
    int y2 = (int)(cy + m_direction.y * half);

    // Perpendicular unit vector: (-dy, dx)
    float px = -m_direction.y;
    float py =  m_direction.x;

    if (m_type == ProjectileType::BULLET)
    {
        // Bullets: 2-pixel wide (unchanged)
        SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
        SDL_RenderDrawLine(renderer,
            x1 + (int)(px + 0.5f), y1 + (int)(py + 0.5f),
            x2 + (int)(px + 0.5f), y2 + (int)(py + 0.5f));
    }
    else
    {
        // AP / HE shells: 4-pixel wide (2x wider), drawn as 4 parallel lines
        for (float w = -1.5f; w <= 2.0f; w += 1.0f)
        {
            SDL_RenderDrawLine(renderer,
                (int)(x1 + px * w), (int)(y1 + py * w),
                (int)(x2 + px * w), (int)(y2 + py * w));
        }
    }
}

void Projectile::onCollision(GameObject* other)
{
    if (!other || !other->isAlive()) return;
    onHit(other);
    markDead();
}
