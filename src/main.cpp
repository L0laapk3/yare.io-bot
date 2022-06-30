
#include "interface.h"
#include "objects.h"
#include "combat.h"
#include "farm.h"
#include "printf.h"



bool attacking = false;

EXPORT("tick")
void tick(int currentTick) {

	parseTick(currentTick);
	// processAttacks();
	// defend();
	farm();

	// if (myStrength > enemyStrength * 1.1f + 200.f || units.size() >= 500 || (!outposts[0].isFriendly() && outposts[0].range > 400.f))
	// 	attacking = true;
	// if (attacking)
	// 	attack();
	// else
	// 	farm();
}