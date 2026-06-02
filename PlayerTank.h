#pragma once
// COMP710 - KineticImpact
// PlayerTank.h - Player input, repair mode toggle  (TDD §3.5 / §4.3)

#include "Tank.h"
#include <SDL.h>

class Scene;

class PlayerTank : public Tank
{
public:
    explicit PlayerTank(Vec2 position);
    ~PlayerTank();

    void update(float dt) override;
    void render(SDL_Renderer* renderer, float cameraX) override;

    // Process keyboard/mouse input and drive tank (called each frame by Game)
    void handleInput(float dt, Scene* scene);

    // Toggle T-key repair mode (TDD §3.5)
    // Active only when: tank stationary + friendly infantry alive + within REPAIR_RANGE
    void toggleRepairMode();
    bool isRepairModeActive() const { return m_repairMode; }

private:
    bool m_repairMode = false;

    // Sprite textures (loaded from asset files)
    SDL_Texture* m_hullTex   = nullptr;
    SDL_Texture* m_turretTex = nullptr;
};
