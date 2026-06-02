#pragma once
// COMP710 - KineticImpact
// CollisionManager.h - Box2D world wrapper + ContactListener  (TDD §4.1 / §3.1)
// NOTE: Requires ThirdParty/box2d/include in AdditionalIncludeDirectories
//       and box2d source files compiled into the project.

#include <box2d/box2d.h>

class Scene;

class CollisionManager : public b2ContactListener
{
public:
    CollisionManager();
    ~CollisionManager();

    bool initialise();                     // Create b2World with zero gravity (top-down)
    void update(float dt, Scene* scene);   // Step physics, then check trigger zones

    b2World* getWorld() const { return m_world; }

    // ── b2ContactListener callbacks ────────────────────────────────────────────
    void BeginContact(b2Contact* contact) override;
    void EndContact(b2Contact* contact)   override;

private:
    b2World* m_world = nullptr;
};
