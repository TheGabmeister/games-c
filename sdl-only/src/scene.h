#ifndef SCENE_H
#define SCENE_H

typedef struct Scene {
    void (*init)(void);
    void (*update)(float dt);
    void (*draw)(void);
    void (*cleanup)(void);
} Scene;

void scene_set(Scene scene);
void scene_apply_pending(void);
Scene scene_get(void);
void scene_update(float dt);
void scene_draw(void);
void scene_cleanup(void);

#endif // SCENE_H
