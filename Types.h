#pragma once
// COMP710 - KineticImpact
// Types.h - Global enums, structs, and constants

// ── Ammo type selected by player ──────────────────────────────────────────────
enum class AmmoType
{
    AP,   // Armour-Piercing  – single target, penetration-based
    HE    // High-Explosive   – area damage on impact
};

// ── Projectile type tag ───────────────────────────────────────────────────────
enum class ProjectileType
{
    BULLET,  // Machine-gun round  – damages infantry only, short range
    AP,      // Armour-Piercing shell – high penetration, flat trajectory
    HE       // High-Explosive shell – splash damage, triggers Lock-HP spall
};

// ── Infantry AI states  (TDD §4.4 state machine) ─────────────────────────────
enum class AIState
{
    IDLE,    // Standing still, no target in range
    FOLLOW,  // Moving toward follow target (friendly tank)
    ATTACK,  // Enemy in range and attack cooldown ready
    REPAIR   // Repairing tank (T held + tank stationary + within REPAIR_RANGE)
};

// ── Entity faction tag ────────────────────────────────────────────────────────
enum class Faction
{
    PLAYER,   // Player tank + friendly infantry
    ENEMY     // Enemy tanks + enemy infantry
};

// ── Game / scene states ───────────────────────────────────────────────────────
enum class GameState
{
    SPLASH,       // AUT / team splash screens
    MENU,         // Main menu
    LEVEL_SELECT, // Level selection screen (UI1)
    HELP,         // Help / info overlay (UI2) — E on level select, any key back
    PLAYING,      // Active gameplay
    LEVEL_WIN,    // All enemies destroyed - transition
    GAME_OVER     // Player tank destroyed
};

// ── Level 2 spawn events  (TDD §3.6 / §5.2) ──────────────────────────────────
enum class SpawnEvent
{
    SPAWN_ENEMY_TANK_1,     // Phase 1: single stationary enemy tank
    SPAWN_ENEMY_TANKS_2_3   // Phase 2: two flanking enemy tanks
};

// ── Armour section  (hull or turret)  TDD §3.1 / §3.2 / §4.2 ─────────────────
struct ArmorSection
{
    float armor     = 100.0f;  // Base armour thickness in mm
    int   hp        = 100;     // Current durability
    int   maxHp     = 100;     // Maximum durability
    bool  destroyed = false;   // True only when penetrated to 0 HP

    // Non-penetrating spall: cannot reduce below 1% maxHp  (Lock-HP rule)
    // TDD §3.2 / §6.2
    void applySpall(int damage)
    {
        int minHp = (maxHp / 100 > 0) ? maxHp / 100 : 1;  // 1% floor
        hp = (hp - damage < minHp) ? minHp : hp - damage;
    }
};

// ── Trigger zone  (LevelManager)  TDD §3.6 / §4.2 ───────────────────────────
struct TriggerZone
{
    float      triggerX = 0.0f;
    SpawnEvent event    = SpawnEvent::SPAWN_ENEMY_TANK_1;
    bool       fired    = false;
};

// ── Screen and world constants ────────────────────────────────────────────────
namespace Constants
{
    // Window
    constexpr int   SCREEN_W        = 1280;
    constexpr int   SCREEN_H        = 720;
    constexpr float FIXED_TIMESTEP  = 1.0f / 60.0f;  // 60 Hz physics step

    // World (horizontal-scrolling map)
    constexpr float WORLD_W         = 2800.0f;        // Level 2 width (px)
    constexpr float WORLD_H         = 720.0f;

    // Splash
    constexpr float SPLASH_DURATION = 2.0f;           // seconds per splash screen

    // Tank - HP & armour (TDD §10)
    constexpr int   PLAYER_MAX_HP   = 100;
    constexpr int   ENEMY_TANK_HP   = 80;
    constexpr float HULL_ARMOR      = 100.0f;         // mm - base thickness
    constexpr float TURRET_ARMOR    = 150.0f;         // mm - base thickness

    // Movement
    constexpr float TANK_SPEED       = 150.0f;        // px / s
    constexpr float TANK_ROT_SPEED   = 120.0f;        // deg / s
    constexpr float TURRET_ROT_SPEED = 180.0f;        // deg / s

    // Weapons - reload (TDD §3.4 / §6.3)
    constexpr float MG_COOLDOWN     = 0.12f;          // s between MG shots
    constexpr float RELOAD_TIME     = 5.0f;           // s - manual cannon reload

    // Projectile speeds (px/s)
    constexpr float BULLET_SPEED    = 600.0f;
    constexpr float AP_SHELL_SPEED  = 800.0f;
    constexpr float HE_SHELL_SPEED  = 600.0f;

    // Penetration in mm (TDD §10)
    constexpr float AP_PENETRATION  = 105.0f;
    constexpr float HE_PENETRATION  = 60.0f;

    // Damage values
    constexpr int   BULLET_DMG      = 0;              // No damage to tanks
    constexpr int   BULLET_INF_DMG  = 1;              // One-shot infantry
    constexpr int   HE_DMG          = 20;             // HE splash damage (flat, before falloff)
    constexpr int   RIFLE_DMG       = 10;             // Enemy infantry vs player

    // Cannon ammo limits (TDD §9.2)
    constexpr int   AP_AMMO_MAX      = 10;             // AP shells per life
    constexpr int   HE_AMMO_MAX      = 5;              // HE shells per life

    // Lock-HP spall percentages - applied to section maxHp (TDD §3.2 / §6.2)
    constexpr float AP_SPALL_PCT    = 0.10f;          // 10% maxHp on AP non-pen
    constexpr float HE_SPALL_PCT    = 0.20f;          // 20% maxHp on HE non-pen
    constexpr float LOCK_HP_FLOOR   = 0.01f;          // 1% minimum HP floor

    // Ricochet (TDD §6.1)
    constexpr float RICOCHET_THRESHOLD = 0.05f;       // min cos(theta) before ricochet

    // HE explosion (TDD §6.7 / §10)
    constexpr float HE_RADIUS       = 96.0f;          // px - explosion area radius

    // Projectile max travel distances (TDD §3.8 / §10)
    constexpr float BULLET_MAX_RANGE = 300.0f;        // px
    constexpr float AP_MAX_RANGE     = 1200.0f;       // px
    constexpr float HE_MAX_RANGE     = 900.0f;        // px

    // Infantry (TDD §3.5 / §6.4)
    constexpr float INFANTRY_SPEED    = 80.0f;        // px / s
    constexpr float INFANTRY_FOLLOW_R = 120.0f;       // px - follow radius
    constexpr float INFANTRY_ATTACK_R = 200.0f;       // px - attack range
    constexpr float INFANTRY_COOLDOWN = 1.5f;         // s between shots
    constexpr float REPAIR_RATE       = 0.01f;        // 1% of section maxHp per second
    constexpr float REPAIR_RANGE      = 80.0f;        // px - max distance for repair

    // Enemy tank AI
    constexpr float ENEMY_SPEED       = 80.0f;        // px / s
    constexpr float ENEMY_CANNON_RANGE = 400.0f;      // px
    constexpr float ENEMY_CANNON_CD   = 3.5f;         // s between enemy cannon shots

    // Level 2 trigger zones (TDD §3.6 / §5.2)
    constexpr float TRIGGER_ZONE_1_X  = 800.0f;      // px - spawn EnemyTank #1
    constexpr float TRIGGER_ZONE_2_X  = 1600.0f;     // px - spawn EnemyTank #2 + #3

    // Minimap (TDD §5.3)
    constexpr int MM_X = SCREEN_W - 170;              // top-right, 10px inset
    constexpr int MM_Y = 10;
    constexpr int MM_W = 160;
    constexpr int MM_H = 90;
}
