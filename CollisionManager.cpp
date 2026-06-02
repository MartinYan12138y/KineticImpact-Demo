// COMP710 - KineticImpact
// CollisionManager.cpp - Box2D world + simple AABB overlap for game logic
// TDD §4.1

#include "CollisionManager.h"
#include "Scene.h"
#include "Projectile.h"
#include "Tank.h"
#include "Infantry.h"

CollisionManager::CollisionManager()  = default;

CollisionManager::~CollisionManager()
{
    delete m_world;
}

bool CollisionManager::initialise()
{
    // Top-down 2D: zero gravity (TDD §4.1)
    b2Vec2 gravity(0.0f, 0.0f);
    m_world = new b2World(gravity);
    m_world->SetContactListener(this);
    return m_world != nullptr;
}

// ── Per-frame update ──────────────────────────────────────────────────────────
void CollisionManager::update(float dt, Scene* scene)
{
    // Step Box2D (bodies can be added per-entity in future; trigger zones use this)
    if (m_world)
        m_world->Step(dt, 6, 2);

    if (!scene) return;

    // ── Simple AABB/circle overlap for projectile hits ────────────────────────
    // This replaces the need for per-entity Box2D bodies at this stage.
    // A full Box2D integration would attach b2Bodies to each entity.
    const auto& objects = scene->getObjects();

    for (auto* a : objects)
    {
        auto* proj = dynamic_cast<Projectile*>(a);
        if (!proj || !proj->isAlive()) continue;

        for (auto* b : objects)
        {
            if (b == a || !b->isAlive()) continue;
            if (b->getFaction() == proj->getFaction()) continue;  // No friendly fire

            // Skip projectile-projectile collisions
            if (dynamic_cast<Projectile*>(b)) continue;

            // Circle overlap: use average of target's size as hit radius
            float hitRadius = (b->getSize().x + b->getSize().y) * 0.25f;
            float dist      = proj->getPosition().distance(b->getPosition());

            if (dist < hitRadius)
            {
                proj->onCollision(b);
                break;  // Each projectile hits at most one target per frame
            }
        }
    }
}

// ── b2ContactListener callbacks (for future trigger zones / Box2D bodies) ─────
void CollisionManager::BeginContact(b2Contact* /*contact*/) {}
void CollisionManager::EndContact(b2Contact*   /*contact*/) {}
