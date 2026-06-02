// COMP710 - KineticImpact
// Wreck.cpp - Static destroyed-tank sprite

#include "Wreck.h"
#include <SDL_image.h>

Wreck::Wreck(Vec2 position)
    : GameObject(position, { 96.0f, 96.0f }, Faction::PLAYER)
    // PLAYER faction so LevelManager doesn't count it as a remaining enemy
{
}

Wreck::~Wreck()
{
    if (m_texture) SDL_DestroyTexture(m_texture);
}

void Wreck::render(SDL_Renderer* renderer, float cameraX)
{
    if (!m_texture)
        m_texture = IMG_LoadTexture(renderer, "Tankui1.png");

    int sx = (int)(m_position.x - cameraX);
    int sy = (int)m_position.y;

    SDL_Rect dst = {
        sx - (int)(m_size.x * 0.5f),
        sy - (int)(m_size.y * 0.5f),
        (int)m_size.x, (int)m_size.y
    };

    if (m_texture)
        SDL_RenderCopy(renderer, m_texture, nullptr, &dst);
}
