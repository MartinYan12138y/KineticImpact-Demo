#pragma once
// COMP710 - KineticImpact
// Tank.h - Base tank class: dual ArmorSection, reload state machine, Lock-HP
// TDD §3.1 / §3.2 / §3.3 / §3.4 / §4.1 / §4.3

#include "GameObject.h"
#include "Types.h"
#include "Vec2.h"

class Scene;

class Tank : public GameObject
{
public:
    Tank(Vec2 position, Vec2 size, Faction faction);
    virtual ~Tank() = default;

    virtual void update(float dt) override;
    virtual void render(SDL_Renderer* renderer, float cameraX) override;
    virtual void onCollision(GameObject* other) override;

    // ── Combat (TDD §3.1 / §3.2 / §3.3) ──────────────────────────────────────
    // pen       = shell penetration value (mm)
    // angleDeg  = impact angle from face normal (degrees)
    // hitTurret = true -> test against turret, false -> test against hull
    // spallPct: fraction of section maxHp deducted on non-penetration
    //   AP default = 10%  (Constants::AP_SPALL_PCT)
    //   HE passes  = 20%  (Constants::HE_SPALL_PCT)
    void takeDamage(float pen, float angleDeg, bool hitTurret,
                    float spallPct = Constants::AP_SPALL_PCT);

    // Reload state machine (TDD §3.4 / §6.3)
    void startReload();
    void onHit(ProjectileType type);   // AP/HE hit resets reload timer

    // Fire actions (add projectile to scene)
    void fireMachineGun(Vec2 dir, Scene* scene);
    void fireCannon(Vec2 target, Scene* scene);

    // ── Accessors ──────────────────────────────────────────────────────────────
    bool             isHullDestroyed()    const { return m_hull.destroyed; }
    bool             isTurretDestroyed()  const { return m_turret.destroyed; }
    bool             isShellLoaded()      const { return m_shellLoaded; }
    bool             isReloading()        const { return m_isReloading; }
    float            getReloadProgress()  const;   // 0.0 (just started) -> 1.0 (done)
    const ArmorSection& getHull()         const { return m_hull; }
    const ArmorSection& getTurret()       const { return m_turret; }
    ArmorSection& getHull()                     { return m_hull; }     // For HE splash
    ArmorSection& getTurret()                   { return m_turret; }   // For HE splash
    ArmorSection*    getMostDamagedSection();      // Priority: destroyed sections first (TDD §3.5)
    float            getBodyAngle()       const { return m_bodyAngle; }
    float            getTurretAngle()     const { return m_turretAngle; }
    AmmoType         getSelectedAmmo()    const { return m_selectedAmmo; }
    Vec2             getVelocity()        const { return m_velocity; }
    int              getApAmmo()          const { return m_apAmmo; }
    int              getHeAmmo()          const { return m_heAmmo; }

protected:
    // ── Armour (TDD §3.3) ──────────────────────────────────────────────────────
    ArmorSection m_hull;    // 100 mm base, movement locked when destroyed
    ArmorSection m_turret;  // 150 mm base, firing locked when destroyed

    // ── Movement ───────────────────────────────────────────────────────────────
    Vec2  m_velocity      = Vec2::zero();
    float m_bodyAngle     = 0.0f;         // degrees (clockwise from right)
    float m_turretAngle   = 0.0f;         // degrees (clockwise from right)

    // ── Reload state machine (TDD §3.4) ───────────────────────────────────────
    bool  m_isReloading   = false;
    bool  m_shellLoaded   = true;
    float m_reloadTimer   = 0.0f;         // counts down from RELOAD_TIME

    // ── Weapons ────────────────────────────────────────────────────────────────
    AmmoType m_selectedAmmo = AmmoType::AP;
    float    m_mgCooldown   = 0.0f;       // machine-gun fire rate limiter
    int      m_apAmmo       = Constants::AP_AMMO_MAX;  // remaining AP shells (TDD §9.2)
    int      m_heAmmo       = Constants::HE_AMMO_MAX;  // remaining HE shells (TDD §9.2)
};
