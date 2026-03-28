#include "platform.h"

#include "../defines.h"

#include "../components/input.h"
#include "../components/viewport.h"

#include "../managers/physics.h"

#include "viewport.h"

//==============================================================================

void update_viewport(ecs_iter_t *it)
{
  Input *input = ecs_field(it, Input, 0);
  Viewport *viewport = ecs_field(it, Viewport, 1);
  for (int i = it->count - 1; i >= 0; --i)
  {
    viewport[i].active = false;
    viewport[i].pointer = (vector2){
        viewport[i].size.x * (input->pointer.x - viewport[i].dst.x) / viewport[i].dst.w,
        viewport[i].size.y * (viewport[i].dst.h - input->pointer.y + viewport[i].dst.y) / viewport[i].dst.h};
  }
}
