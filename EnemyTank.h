#pragma once
// COMP710 - KineticImpact
// EnemyTank.h - Enemy tank with seek-and-fire AI  (TDD §4.1 / §4.3)

#include "Tank.h"
#include <SDL.h>

class Scene;

class EnemyTank : public Tank
{
public:
    explicit EnemyTank(Vec2 position);
    ~EnemyTank();

    void update(float dt) override;
    void render(SDL_Renderer* renderer, float cameraX) override;

    // AI: seek player, maintain distance, fire AP cannon (TDD §9.1)
    void AI_update(float dt, Scene* scene);

    void        setTarget(GameObject* target) { m_target = target; }
    GameObject* getTarget()             const { return m_target; }

private:
    GameObject* m_target         = nullptr;  // Usually the PlayerTank
    float       m_cannonCooldown = 0.0f;     // Seconds until next cannon shot
    Scene*      m_scene          = nullptr;  // Cached so update() can spawn wreck
    bool        m_wreckSpawned   = false;    // Spawn wreck only once on death

    SDL_Texture* m_hullTex   = nullptr;
    SDL_Texture* m_turretTex = nullptr;
};
