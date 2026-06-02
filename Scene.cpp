// COMP710 - KineticImpact
// Scene.cpp - Owns all GameObjects; deferred add/remove  (TDD §3.9 / §4.1)

#include "Scene.h"
#include "GameObject.h"
#include <algorithm>

Scene::Scene()  = default;

Scene::~Scene()
{
    // Free all live objects (TDD §3.9: all entities freed in Scene destructor)
    for (auto* obj : m_objects)     delete obj;
    for (auto* obj : m_pendingAdd)  delete obj;
    m_objects.clear();
    m_pendingAdd.clear();
    m_pendingRemove.clear();
}

void Scene::update(float dt)
{
    flushPending();

    for (auto* obj : m_objects)
        if (obj->isAlive()) obj->update(dt);

    // Queue dead objects for removal at next flush
    for (auto* obj : m_objects)
        if (!obj->isAlive()) m_pendingRemove.push_back(obj);
}

void Scene::render(SDL_Renderer* renderer, float cameraX)
{
    for (auto* obj : m_objects)
        if (obj->isAlive()) obj->render(renderer, cameraX);
}

void Scene::addObject(GameObject* obj)
{
    m_pendingAdd.push_back(obj);
}

void Scene::removeObject(GameObject* obj)
{
    // Guard against double-queuing
    if (std::find(m_pendingRemove.begin(), m_pendingRemove.end(), obj) == m_pendingRemove.end())
        m_pendingRemove.push_back(obj);
}

// ── Private ───────────────────────────────────────────────────────────────────
void Scene::flushPending()
{
    // Commit additions
    for (auto* obj : m_pendingAdd)
        m_objects.push_back(obj);
    m_pendingAdd.clear();

    // Commit removals and free memory (TDD §3.9)
    for (auto* dead : m_pendingRemove)
    {
        auto it = std::find(m_objects.begin(), m_objects.end(), dead);
        if (it != m_objects.end())
        {
            delete *it;
            m_objects.erase(it);
        }
    }
    m_pendingRemove.clear();
}
