#ifndef TRANSFORM_COMPONENT_H
#define TRANSFORM_COMPONENT_H

#include <cglm/cglm.h>
#include <flecs.h>

typedef struct { 
    vec2 position; 
    float rotation; 
    vec2 scale;
} Transform;

ECS_COMPONENT_DECLARE(Transform);

#endif