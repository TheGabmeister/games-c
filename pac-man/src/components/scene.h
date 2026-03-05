#ifndef SCENE_COMPONENT_H
#define SCENE_COMPONENT_H

#include <flecs.h>
#include <engine.h>
#include <SDL3/SDL.h>

typedef enum SceneName
{
    SCENE_SPLASH,
    SCENE_TITLE,
    SCENE_LEVEL,
    SCENE_GAME_OVER,
    MAX_SCENES
} SceneName;

typedef struct Scene
{
    SceneName id;
    float time;
    float value;
    color color;
    SDL_Texture *texture;
} Scene;

ECS_COMPONENT_DECLARE(Scene);

#endif
