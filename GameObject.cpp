// COMP710 - KineticImpact
// GameObject.cpp

#include "GameObject.h"

GameObject::GameObject(Vec2 position, Vec2 size, Faction faction)
    : m_position(position)
    , m_size(size)
    , m_faction(faction)
    , m_alive(true)
{
}
