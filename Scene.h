#pragma once
// COMP710 - KineticImpact
// Scene.h - Owns all GameObjects; deferred add/remove; delegates update/render
// TDD §4.1 / §3.9

#include <vector>
#include <SDL.h>

class GameObject;

class Scene
{
public:
    Scene();
    ~Scene();   // Frees all owned GameObjects (TDD §3.9)

    void update(float dt);
    void render(SDL_Renderer* renderer, float cameraX);

    // Deferred add/remove: changes take effect at the start of next update tick
    void addObject(GameObject* obj);
    void removeObject(GameObject* obj);

    const std::vector<GameObject*>& getObjects() const { return m_objects; }

private:
    void flushPending();   // Commit deferred adds/removes (TDD §3.9)

    std::vector<GameObject*> m_objects;        // Live objects (owned by Scene)
    std::vector<GameObject*> m_pendingAdd;     // Queued for next tick
    std::vector<GameObject*> m_pendingRemove;  // Queued for deletion
};
