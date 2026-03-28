#include "platform.h"
#include <box2d/box2d.h>

//==============================================================================

static inline b2Vec2 _from_vector(vector2 position)
{
  return (b2Vec2){position.x, position.y};
}

//------------------------------------------------------------------------------

static inline vector2 _to_vector(b2Vec2 position)
{
  return (vector2){position.x, position.y};
}
