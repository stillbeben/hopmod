#include "pch.h"
#include "cube.h"
#include "iengine.h"
#include "igame.h"
#include "game.h"
#include "fpsserver.hpp"
#include "crypto.h"

REGISTERGAME(fpsgame, "fps", NULL, new fpsserver());
