#include "Navigation.h"

void rotate_direction(Direction &dir, int t)
{
    dir = static_cast<Direction>((dir + t) % 4);
}

// String -> Direction map
std::unordered_map<std::string, Direction> directionMap =
{
    { "up"    , UP           },
    { "right" , RIGHT        },
    { "down"  , DOWN         },
    { "left"  , LEFT         },
    { "none"  , NO_DIRECTION },
    { "u"     , UP           },
    { "r"     , RIGHT        },
    { "d"     , DOWN         },
    { "l"     , LEFT         },
    { "n"     , NO_DIRECTION }
};

// Direction -> Vector map
std::unordered_map<Direction, Pos2D<int>> directionVectorMap =
{
    { LEFT         , { -1,  0 } },
    { UP           , {  0, -1 } },
    { RIGHT        , {  1,  0 } },
    { DOWN         , {  0,  1 } },
    { NO_DIRECTION , {  0,  0 } }
};
