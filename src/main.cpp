
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

	// if (myStrength > enemyStrength * 1.1f + 20.f || units.size() >= 500)
	// 	attacking = true;
	if (attacking)
		attack();
	else
		farm();
}