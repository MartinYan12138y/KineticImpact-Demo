#pragma once
// COMP710 - KineticImpact
// Infantry.h - Auto-follow + auto-fire FSM; T-key repair mode  (TDD §3.5 / §4.3 / §4.4)

#include "GameObject.h"
#include "Types.h"
#include "Vec2.h"
#include <SDL.h>

class Tank;
class Scene;

class Infantry : public GameObject
{
public:
    Infantry(Vec2 position, Faction faction);
    ~Infantry();

    void update(float dt) override;
    void render(SDL_Renderer* renderer, float cameraX) override;
    void onCollision(GameObject* other) override;

    // ── Repair (TDD §3.5 / §6.4) ──────────────────────────────────────────────
    // Applies 1% of section maxHp per second; prioritises destroyed sections
    void repairTank(float dt);

    // ── Combat ─────────────────────────────────────────────────────────────────
    void shootAt(GameObject* target, Scene* scene);

    // ── Configuration ──────────────────────────────────────────────────────────
    void setFollowTarget(Tank* tank)    { m_followTarget = tank; }
    void setScene(Scene* scene)         { m_scene = scene; }
    void setRepairMode(bool enabled)    { m_repairMode = enabled; }
    bool isInRepairMode()         const { return m_repairMode; }

    // Formation offset: world-space displacement from the tank centre (TDD §9.2)
    void setFormationOffset(Vec2 offset) { m_formationOffset = offset; }

private:
    // maxDist = search radius; pass a large value to search without limit
    GameObject* findNearestEnemy(Scene* scene, float maxDist = Constants::INFANTRY_ATTACK_R) const;
    Tank*       findNearestFriendlyTank() const;

    // ── State ──────────────────────────────────────────────────────────────────
    AIState     m_state          = AIState::IDLE;
    bool        m_repairMode     = false;

    // ── Targets ────────────────────────────────────────────────────────────────
    Tank*       m_followTarget   = nullptr;  // Friendly tank to follow / repair
    GameObject* m_attackTarget   = nullptr;  // Current enemy target
    Scene*      m_scene          = nullptr;  // Needed to search for targets each tick

    // ── Formation (TDD §9.2) ───────────────────────────────────────────────────
    Vec2        m_formationOffset = { 0.0f, 0.0f };   // offset from tank centre

    // ── Timers ─────────────────────────────────────────────────────────────────
    float       m_attackCooldown = 0.0f;

    SDL_Texture* m_texture = nullptr;
};
