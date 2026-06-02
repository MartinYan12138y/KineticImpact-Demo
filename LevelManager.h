#pragma once
// COMP710 - KineticImpact
// LevelManager.h - Level index, enemy count, trigger zones, spawn events
// TDD §3.6 / §4.1 / §5.1 / §5.2 / §6.5

#include "Types.h"
#include <vector>

class Scene;

class LevelManager
{
public:
    LevelManager() = default;

    // Called every frame from Game::update (TDD §6.5)
    void update(float playerX, Scene* scene);

    // Load entities for the given level index (1 = infantry, 2 = armoured)
    void spawnLevel(int level, Scene* scene);

    // Called by Scene/CollisionManager when an enemy is destroyed
    void onEnemyDestroyed();

    // ── Accessors ──────────────────────────────────────────────────────────────
    int  getCurrentLevel()     const { return m_currentLevel; }
    int  getEnemiesRemaining() const { return m_enemiesRemaining; }
    bool isLevelComplete()     const;   // 0 enemies alive AND all trigger zones fired

private:
    // Check each zone and fire spawn events when player crosses threshold (TDD §6.5)
    void checkTriggerZones(float playerX, Scene* scene);
    void fireSpawnEvent(SpawnEvent evt, Scene* scene);
    void spawnLevel1(Scene* scene);
    void spawnLevel2(Scene* scene);

    int                      m_currentLevel      = 1;
    int                      m_enemiesRemaining   = 0;
    std::vector<TriggerZone> m_triggerZones;       // Stored in ascending triggerX order
};
