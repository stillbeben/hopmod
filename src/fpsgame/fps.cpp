#include "pch.h"
#include "cube.h"
#include "iengine.h"
#include "igame.h"
#include "game.h"
#include "fpsserver.h"
#include "crypto.h"

REGISTERGAME(fpsgame, "fps", NULL, new fpsserver());
