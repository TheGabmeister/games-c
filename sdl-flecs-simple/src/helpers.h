#include <engine.h>
#include <chipmunk/chipmunk.h>

//==============================================================================

static inline cpVect _from_vector(vector2 position)
{
  return (cpVect){position.x, position.y};
}

//------------------------------------------------------------------------------

static inline vector2 _to_vector(cpVect position)
{
  return (vector2){position.x, position.y};
}

//------------------------------------------------------------------------------

static inline color _to_color(cpSpaceDebugColor col)
{
	color result;

    result.r = (unsigned char)(col.r*255.0f);
    result.g = (unsigned char)(col.g*255.0f);
    result.b = (unsigned char)(col.b*255.0f);
    result.a = (unsigned char)(col.a*255.0f);

    return result;

}
