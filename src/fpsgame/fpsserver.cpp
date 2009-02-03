#include "tools.h"
#include "hopmod/tools.hpp"
#include "hash.h"
#include "fpsserver.h"

REGISTERGAME(fpsgame, "fps", NULL, new fpsserver());
