#pragma once
// COMP710 - KineticImpact
// GameObject.h - Abstract base class for all game entities  (TDD §4.1)

#include "Vec2.h"
#include "Types.h"
#include <SDL.h>

class GameObject
{
public:
    GameObject(Vec2 position, Vec2 size, Faction faction);
    virtual ~GameObject() = default;

    // ── Pure-virtual interface (TDD §4.1) ─────────────────────────────────────
    virtual void update(float dt)                              = 0;
    virtual void render(SDL_Renderer* renderer, float cameraX) = 0;
    virtual void onCollision(GameObject* other)                = 0;

    // ── Accessors ──────────────────────────────────────────────────────────────
    Vec2    getPosition() const { return m_position; }
    Vec2    getSize()     const { return m_size; }
    Faction getFaction()  const { return m_faction; }
    bool    isAlive()     const { return m_alive; }

    void    setPosition(Vec2 pos) { m_position = pos; }
    void    markDead()            { m_alive = false; }

protected:
    Vec2    m_position;
    Vec2    m_size;
    Faction m_faction;
    bool    m_alive = true;
};
