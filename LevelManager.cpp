// COMP710 - KineticImpact
// LevelManager.cpp - Level index, enemy count, trigger zones, spawn events
// TDD §3.6 / §5.1 / §5.2 / §6.5

#include "LevelManager.h"
#include "Scene.h"
#include "PlayerTank.h"
#include "EnemyTank.h"
#include "Infantry.h"
#include "Types.h"

// ── Per-frame update: check trigger zones + recount alive enemies (TDD §6.5) ──
// IMPORTANT: must be called AFTER Scene::update() and CollisionManager::update()
// so that entities killed this frame are already marked dead (isAlive() == false).
void LevelManager::update(float playerX, Scene* scene)
{
    checkTriggerZones(playerX, scene);

    // Recount alive enemies directly from the scene.
    // This replaces the never-called onEnemyDestroyed() callback approach.
    if (scene)
    {
        int alive = 0;
        for (auto* obj : scene->getObjects())
            if (obj->isAlive() && obj->getFaction() == Faction::ENEMY) ++alive;
        m_enemiesRemaining = alive;
    }
}

// ── Level complete: no enemies alive AND every trigger zone has fired ──────────
// The trigger-zone check prevents early completion in Level 2 where tank waves
// spawn only when the player advances far enough.
bool LevelManager::isLevelComplete() const
{
    if (m_enemiesRemaining > 0) return false;
    for (const auto& zone : m_triggerZones)
        if (!zone.fired) return false;
    return true;
}

// ── Spawn a level's initial entities ─────────────────────────────────────────
void LevelManager::spawnLevel(int level, Scene* scene)
{
    m_currentLevel    = level;
    m_enemiesRemaining = 0;
    m_triggerZones.clear();

    if (level == 1)
        spawnLevel1(scene);
    else if (level == 2)
        spawnLevel2(scene);
}

// ── Enemy destroyed callback ──────────────────────────────────────────────────
void LevelManager::onEnemyDestroyed()
{
    if (m_enemiesRemaining > 0) --m_enemiesRemaining;
}

// ── Private: trigger zone checker (TDD §3.6 / §6.5) ─────────────────────────
void LevelManager::checkTriggerZones(float playerX, Scene* scene)
{
    // Zones stored in ascending triggerX order; checked sequentially (TDD §8)
    for (auto& zone : m_triggerZones)
    {
        if (!zone.fired && playerX >= zone.triggerX)
        {
            fireSpawnEvent(zone.event, scene);
            zone.fired = true;
        }
    }
}

void LevelManager::fireSpawnEvent(SpawnEvent evt, Scene* scene)
{
    switch (evt)
    {
    // Level 2 Phase 1: single stationary enemy tank (TDD §5.2)
    case SpawnEvent::SPAWN_ENEMY_TANK_1:
    {
        auto* tank = new EnemyTank({ 1200.0f, 360.0f });
        scene->addObject(tank);
        ++m_enemiesRemaining;
        break;
    }
    // Level 2 Phase 2: two flanking tanks (TDD §5.2)
    case SpawnEvent::SPAWN_ENEMY_TANKS_2_3:
    {
        auto* t2 = new EnemyTank({ 2000.0f, 250.0f });
        auto* t3 = new EnemyTank({ 2100.0f, 470.0f });
        scene->addObject(t2);
        scene->addObject(t3);
        m_enemiesRemaining += 2;
        break;
    }
    }
}

// ── Level 1: Infantry Assault (TDD §5.1) ─────────────────────────────────────
void LevelManager::spawnLevel1(Scene* scene)
{
    // Formation offsets: 3 soldiers spread behind the tank (TDD §9.2)
    // Negative X = behind (tank faces right), Y spread = left/centre/right
    static const Vec2 kFormation[3] = {
        { -70.0f, -55.0f },   // rear-left
        { -80.0f,   0.0f },   // directly behind
        { -70.0f,  55.0f },   // rear-right
    };

    // Friendly infantry x3 (auto-follow player tank, T-key repair)
    for (int i = 0; i < 3; ++i)
    {
        auto* inf = new Infantry({ 80.0f, 200.0f + i * 80.0f }, Faction::PLAYER);
        inf->setScene(scene);
        inf->setFormationOffset(kFormation[i]);
        scene->addObject(inf);
    }

    // Enemy infantry groups spread across the ~2000px map
    float xPositions[] = { 400, 500, 700, 800, 1000, 1100, 1300, 1400,
                            1600, 1700, 1900, 2000 };
    for (float x : xPositions)
    {
        float y = 150.0f + (float)(rand() % 420);
        auto* inf = new Infantry({ x, y }, Faction::ENEMY);
        inf->setScene(scene);
        scene->addObject(inf);
        ++m_enemiesRemaining;
    }
}

// ── Level 2: Armoured Breakthrough ───────────────────────────────────────────
// 6 friendly infantry (formation around player) + 3 enemy tanks spread across map
void LevelManager::spawnLevel2(Scene* scene)
{
    // Formation offsets for 6 friendly infantry around the player tank
    static const Vec2 kFormation[6] = {
        { -70.0f, -80.0f },   // rear far-left
        { -80.0f, -40.0f },   // rear left
        { -80.0f,   0.0f },   // directly behind
        { -80.0f,  40.0f },   // rear right
        { -70.0f,  80.0f },   // rear far-right
        { -50.0f,   0.0f },   // close behind (bodyguard)
    };

    for (int i = 0; i < 6; ++i)
    {
        auto* inf = new Infantry({ 80.0f, 200.0f + i * 55.0f }, Faction::PLAYER);
        inf->setScene(scene);
        inf->setFormationOffset(kFormation[i]);
        scene->addObject(inf);
    }

    // 3 enemy tanks spread across the map
    auto* t1 = new EnemyTank({ 900.0f,  200.0f });
    auto* t2 = new EnemyTank({ 1400.0f, 500.0f });
    auto* t3 = new EnemyTank({ 2000.0f, 360.0f });
    scene->addObject(t1);
    scene->addObject(t2);
    scene->addObject(t3);
    m_enemiesRemaining += 3;

    // No trigger zones in Level 2 — all enemies present from the start
    m_triggerZones.clear();
}
