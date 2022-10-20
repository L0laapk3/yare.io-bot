
#include "interface.h"
#include "objects.h"
#include "combat.h"
#include "farm.h"
#include "printf.h"



bool attacking = false;

EXPORT("tick")
void tick(int currentTick) {

	parseTick(currentTick);
	processAttacks();

	defend();
	farm();
	attack();

	dealloc();
}