#include "system.h"

#include "../systems/render.h"
#include "../systems/input.h"
#include "../systems/audio.h"

#ifdef DEBUG
#include "../systems/debug.h"
#endif

static void _fini(ecs_world_t *world, void *context)
{
}


void system_manager_init(ecs_world_t *world)
{
    ecs_atfini(world, _fini, NULL);
//   ECS_SYSTEM(world, refresh_display, EcsOnLoad, [out] Display);
//   ECS_SYSTEM(world, progress_time, EcsOnLoad, [inout] Time);
   ECS_SYSTEM(world, process_input, EcsOnLoad, [out] Input);
//    ECS_SYSTEM(world, gui_input, EcsPostLoad, [inout] Interface, [in] Input, [in] Settings, [in] Window);
//    ECS_SYSTEM(world, gui_update, EcsPreUpdate, [inout] Interface, [in] Window(up), [inout] Widget);
//    ECS_SYSTEM(world, transition, EcsPreUpdate, [in] Time, [inout] Transition);
//    ECS_SYSTEM(world, state_machine, EcsPreUpdate, [in] Time, [inout] Stateful);
//    ECS_SYSTEM(world, update_viewport, EcsOnUpdate, [in] Input, [inout] Viewport);
//    ECS_SYSTEM(world, update_scene, EcsOnUpdate, [inout] Scene, [inout] Stateful, [out] Transition, [inout] Time, [in] Input, [inout] Settings);
//    ECS_SYSTEM(world, physics_update, EcsOnValidate, [in] Time, [inout] Physics);
//    ECS_SYSTEM(world, physics_collide, EcsOnValidate, [in] Physics, [inout] Collision);
#ifdef DEBUG
    ECS_SYSTEM(world, debug_input, EcsPostUpdate, [in] Input, [out] Debug);
    ECS_SYSTEM(world, debug_scene, EcsPostUpdate, [in] Scene, [in] Stateful, [out] Debug);
#endif
//    ECS_SYSTEM(world, play_music, EcsPreStore, [inout] Track);
//    ECS_SYSTEM(world, play_sounds, EcsPreStore, [inout] Audible);
//    ECS_SYSTEM(world, animate, EcsOnStore, [inout] Animated, [out] Sprite);
//    ECS_SYSTEM(world, render_scene, EcsOnStore, [in] Scene, [in] Stateful, [in] Transition, [out] Display, [in] Time);
//    ECS_SYSTEM(world, render_viewports, EcsOnStore, [in] Viewport);
//    ECS_SYSTEM(world, render_physical, EcsOnStore, [in] Sprite, [in] Physical, [in] Tinted, [in] ?Transition);
//    ECS_SYSTEM(world, gui_render, EcsOnStore, [inout] Interface);
    ECS_SYSTEM(world, render_sprites, EcsOnStore, [in] Sprite, [in] Transform);
//    ECS_SYSTEM(world, render_labels, EcsOnStore, [in] Time, [in] Label, [in] Aligned, [in] Spatial, [in] Tinted);
#ifdef DEBUG
    ECS_SYSTEM(world, debug_physics, EcsOnStore, [in] Physics, [in] Debug, [inout] Viewport);
    ECS_SYSTEM(world, debug_render, EcsOnStore, [in] Label, [in] Debug, DebugTag);
    ECS_SYSTEM(world, debug_display, EcsOnStore, [in] Display, [in] Debug);
#endif
//    ECS_SYSTEM(world, composite_display, EcsOnStore, [in] Display);


//    _viewport_query = ecs_system_get(world, render_viewports)->query;
    
}