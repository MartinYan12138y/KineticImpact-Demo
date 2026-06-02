#pragma once
// COMP710 - KineticImpact
// Wreck.h - Static destroyed-tank sprite shown at the kill position

#include "GameObject.h"
#include <SDL.h>

class Wreck : public GameObject
{
public:
    explicit Wreck(Vec2 position);
    ~Wreck();

    void update(float dt)                              override {}  // Static — never moves
    void render(SDL_Renderer* renderer, float cameraX) override;
    void onCollision(GameObject* other)                override {}  // Non-collidable

private:
    SDL_Texture* m_texture = nullptr;
};
