
#include "interface.h"

float x;
EXPORT("tick") void tick(int tick, bool initialize) {
    Interface::Spirit::move(0, 100, x++);
}