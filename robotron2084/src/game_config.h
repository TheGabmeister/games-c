#ifndef GAME_CONFIG_H
#define GAME_CONFIG_H

// --- Window ---
#define WINDOW_WIDTH        1200
#define WINDOW_HEIGHT       900
#define TARGET_FPS          60

// --- Player ---
#define PLAYER_START_X      (WINDOW_WIDTH * 0.5f)
#define PLAYER_START_Y      (WINDOW_HEIGHT * 0.5f)
#define PLAYER_LIVES        3
#define PLAYER_RADIUS       14.0f
#define PLAYER_SPEED        360.0f
#define RESPAWN_INVULN_TIME 1.25f
#define PLAYER_DEATH_PAUSE  1.2f

// --- Bullets ---
#define MAX_BULLETS         96
#define BULLET_RADIUS       5.0f
#define BULLET_SPEED        760.0f
#define BULLET_LIFETIME     0.8f
#define BULLET_FIRE_RATE    0.08f

// --- Grunts ---
#define MAX_GRUNTS          96
#define GRUNT_RADIUS        13.0f
#define GRUNT_BASE_SPEED    115.0f
#define GRUNT_WAVE_SPEED_STEP 5.5f
#define GRUNT_WAVE_SPEED_CAP 130.0f
#define GRUNT_SCORE         100

// --- Hulks ---
#define MAX_HULKS           18
#define HULK_RADIUS         22.0f
#define HULK_BASE_SPEED     78.0f
#define HULK_WAVE_SPEED_STEP 2.0f
#define HULK_WAVE_SPEED_CAP 60.0f
#define HULK_BULLET_STUN_TIME 0.18f
#define HULK_KNOCKBACK      24.0f

// --- Spawners and Shooters ---
#define MAX_SPHEROIDS       12
#define MAX_ENFORCERS       36
#define MAX_QUARKS          12
#define MAX_TANKS           28
#define SPHEROID_RADIUS     17.0f
#define ENFORCER_RADIUS     15.0f
#define QUARK_RADIUS        17.0f
#define TANK_RADIUS         18.0f
#define SPHEROID_BASE_SPEED 92.0f
#define ENFORCER_BASE_SPEED 105.0f
#define QUARK_BASE_SPEED    82.0f
#define TANK_BASE_SPEED     70.0f
#define SPHEROID_WAVE_SPEED_STEP 1.1f
#define ENFORCER_WAVE_SPEED_STEP 1.4f
#define QUARK_WAVE_SPEED_STEP 0.9f
#define TANK_WAVE_SPEED_STEP 0.8f
#define SPHEROID_WAVE_SPEED_CAP 42.0f
#define ENFORCER_WAVE_SPEED_CAP 55.0f
#define QUARK_WAVE_SPEED_CAP 35.0f
#define TANK_WAVE_SPEED_CAP 32.0f
#define SPHEROID_SCORE      1000
#define ENFORCER_SCORE      150
#define QUARK_SCORE         1000
#define TANK_SCORE          200

// Timers are in centiseconds when they are fed directly to GetRandomValue().
#define DRIFTER_EDGE_MARGIN 56.0f
#define SPHEROID_INITIAL_SPAWN_MIN_CS 280
#define SPHEROID_INITIAL_SPAWN_MAX_CS 430
#define SPHEROID_RETARGET_MIN_CS 120
#define SPHEROID_RETARGET_MAX_CS 240
#define SPHEROID_SPAWN_BASE_TIME 3.7f
#define SPHEROID_SPAWN_WAVE_STEP 0.06f
#define SPHEROID_SPAWN_MIN_TIME 2.0f
#define SPHEROID_SPAWN_RANDOM_MAX_CS 100
#define ENFORCER_INITIAL_SHOOT_MIN_CS 90
#define ENFORCER_INITIAL_SHOOT_MAX_CS 170
#define ENFORCER_RETARGET_MIN_CS 80
#define ENFORCER_RETARGET_MAX_CS 180
#define ENFORCER_SHOOT_BASE_TIME 1.55f
#define ENFORCER_SHOOT_WAVE_STEP 0.025f
#define ENFORCER_SHOOT_MIN_TIME 0.75f
#define ENFORCER_SHOOT_RANDOM_MAX_CS 60
#define ENFORCER_AIM_JITTER_PIXELS 85
#define QUARK_INITIAL_SPAWN_MIN_CS 360
#define QUARK_INITIAL_SPAWN_MAX_CS 520
#define QUARK_RETARGET_MIN_CS 140
#define QUARK_RETARGET_MAX_CS 280
#define QUARK_SPAWN_BASE_TIME 4.4f
#define QUARK_SPAWN_WAVE_STEP 0.04f
#define QUARK_SPAWN_MIN_TIME 2.4f
#define QUARK_SPAWN_RANDOM_MAX_CS 120
#define TANK_INITIAL_SHOOT_MIN_CS 120
#define TANK_INITIAL_SHOOT_MAX_CS 220
#define TANK_RETARGET_MIN_CS 130
#define TANK_RETARGET_MAX_CS 260
#define TANK_SHOOT_BASE_TIME 2.1f
#define TANK_SHOOT_WAVE_STEP 0.025f
#define TANK_SHOOT_MIN_TIME 1.05f
#define TANK_SHOOT_RANDOM_MAX_CS 90

// --- Brain Waves ---
#define MAX_BRAINS          16
#define MAX_PROGS           40
#define BRAIN_RADIUS        18.0f
#define PROG_RADIUS         12.0f
#define BRAIN_BASE_SPEED    88.0f
#define PROG_BASE_SPEED     185.0f
#define BRAIN_WAVE_SPEED_STEP 1.2f
#define PROG_WAVE_SPEED_STEP 2.4f
#define BRAIN_WAVE_SPEED_CAP 52.0f
#define PROG_WAVE_SPEED_CAP 95.0f
#define BRAIN_SCORE         500
#define PROG_SCORE          100
#define BRAIN_INITIAL_SHOOT_MIN_CS 110
#define BRAIN_INITIAL_SHOOT_MAX_CS 210
#define BRAIN_WOBBLE_SPEED  4.0f
#define BRAIN_WEAVE_AMOUNT  0.45f
#define BRAIN_SHOOT_BASE_TIME 1.9f
#define BRAIN_SHOOT_WAVE_STEP 0.025f
#define BRAIN_SHOOT_MIN_TIME 0.9f
#define BRAIN_SHOOT_RANDOM_MAX_CS 70

// --- Enemy Projectiles ---
#define MAX_PROJECTILES     160
#define SPARK_RADIUS        6.0f
#define SHELL_RADIUS        8.0f
#define CRUISE_RADIUS       7.0f
#define SPARK_SPEED         275.0f
#define SHELL_SPEED         235.0f
#define CRUISE_SPEED        245.0f
#define SPARK_LIFETIME      3.8f
#define SHELL_LIFETIME      5.5f
#define CRUISE_LIFETIME     6.0f
#define SPARK_SCORE         25
#define SHELL_SCORE         50
#define CRUISE_SCORE        25
#define CRUISE_WOBBLE_SPEED 8.0f
#define CRUISE_WEAVE_AMOUNT 0.55f
#define CRUISE_CURRENT_WEIGHT 0.88f
#define CRUISE_DESIRED_WEIGHT 0.12f

// --- Humans ---
#define MAX_HUMANS          40
#define HUMAN_RADIUS        10.0f
#define HUMAN_SPEED         55.0f
#define HUMAN_RETARGET_MIN_CS 80
#define HUMAN_RETARGET_MAX_CS 180
#define WAVE_HUMAN_MAX      12

// --- Electrodes ---
#define MAX_ELECTRODES      48
#define ELECTRODE_RADIUS    12.0f
#define WAVE_ELECTRODE_MAX  24
#define ELECTRODE_PULSE_SPEED 5.0f

// --- Floating Text ---
#define MAX_FLOAT_TEXT      48
#define FLOAT_TEXT_LIFETIME 0.85f
#define FLOAT_TEXT_RISE_SPEED 42.0f

// --- Particles ---
#define MAX_PARTICLES       360
#define PARTICLE_DRAG       0.92f

// --- Waves and Scoring ---
#define WAVE_INTRO_TIME     1.6f
#define EXTRA_LIFE_SCORE    25000

// --- Presentation ---
#define SCREEN_SHAKE_DURATION 0.28f
#define SCREEN_FLASH_DURATION 0.18f
#define WAVE_CLEAR_FLASH_TIME 0.12f

// --- Sound Mix ---
#define PLAYER_SHOOT_VOLUME 0.28f
#define ENEMY_SHOOT_VOLUME  0.38f
#define BRAIN_MISSILE_VOLUME 0.42f
#define HULK_HIT_VOLUME     0.55f

#endif
