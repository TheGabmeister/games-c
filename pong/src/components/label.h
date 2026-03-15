#ifndef LABEL_COMPONENT_H
#define LABEL_COMPONENT_H

#include <flecs.h>
#include <SDL3/SDL.h>

#define LABEL_MAX_TEXT 64

typedef struct Label
{
    char      text[LABEL_MAX_TEXT];
    SDL_Color color;
    float     scale;
} Label;

ECS_COMPONENT_DECLARE(Label);

#endif
