#pragma once

#include "position.h"

enum Shape : int {
	NEUTRAL = -1,
	CIRCLE = 0,
	SQUARE = 1,
	TRIANGLE = 2,
};


#define IMPORT(module, name) __attribute__((import_module(module), import_name(name)))
#define EXPORT(name) __attribute__((export_name(name)))

namespace Interface {
	namespace Spirit {
		IMPORT("spirits", "count") int count();
		IMPORT("spirits", "nameAlloc") char* nameAlloc(int);
		IMPORT("spirits", "positionX") float positionX(int);
		IMPORT("spirits", "positionY") float positionY(int);
		// IMPORT("spirits", "position") Position position(int);
		inline Position position(int i) { return Position{ positionX(i), positionY(i) }; }
		IMPORT("spirits", "size") int size(int);
		IMPORT("spirits", "shape") Shape shape(int);
		IMPORT("spirits", "energyCapacity") int energyCapacity(int);
		IMPORT("spirits", "energy") int energy(int);
		IMPORT("spirits", "id") int id(int);
		IMPORT("spirits", "playerId") int playerId(int);
		IMPORT("spirits", "hp") int hp(int);
		IMPORT("spirits", "range") int range(int);
		IMPORT("spirits", "locked") bool locked(int);

		IMPORT("spirits", "energize") void energize(int, int);
		IMPORT("spirits", "energizeBase") void energizeBase(int, int);
		IMPORT("spirits", "energizeOutpost") void energizeOutpost(int, int);
		IMPORT("spirits", "energizePylon") void energizePylon(int, int);
		IMPORT("spirits", "move") void move(int, float, float);
		IMPORT("spirits", "merge") void merge(int, int);
		IMPORT("spirits", "divide") void divide(int);
		IMPORT("spirits", "lock") void lock(int);
		IMPORT("spirits", "unlock") void unlock(int);
		IMPORT("spirits", "jump") void jump(int, float, float);
		IMPORT("spirits", "explode") void explode(int);
		IMPORT("spirits", "shout") void shout(int, const char*);
	};

	namespace Base {
		IMPORT("bases", "count") int count();
		IMPORT("bases", "nameAlloc") char* nameAlloc(int);
		IMPORT("bases", "positionX") float positionX(int);
		IMPORT("bases", "positionY") float positionY(int);
		// IMPORT("bases", "position") Position position(int);
		inline Position position(int i) { return Position{ positionX(i), positionY(i) }; }
		IMPORT("spirits", "shape") Shape shape(int);
		IMPORT("bases", "energyCapacity") int energyCapacity(int);
		IMPORT("bases", "energy") int energy(int);
		IMPORT("bases", "controlledBy") int controlledBy(int);
		IMPORT("bases", "currentSpiritCost") int currentSpiritCost(int);
		IMPORT("bases", "spiritCostCount") int spiritCostCount(int);
		IMPORT("bases", "spiritCostTreshold") int spiritCostTreshold(int, int);
		IMPORT("bases", "spiritCostValue") int spiritCostValue(int, int);
	};

	namespace Outpost {
		IMPORT("outposts", "count") int count();
		IMPORT("outposts", "nameAlloc") char* nameAlloc(int);
		IMPORT("outposts", "positionX") float positionX(int);
		IMPORT("outposts", "positionY") float positionY(int);
		// IMPORT("outposts", "position") Position position(int);
		inline Position position(int i) { return Position{ positionX(i), positionY(i) }; }
		IMPORT("outposts", "energyCapacity") int energyCapacity(int);
		IMPORT("outposts", "energy") int energy(int);
		IMPORT("outposts", "range") float range(int);
		IMPORT("outposts", "controlledBy") int controlledBy(int);
	};

	namespace Pylon {
		IMPORT("pylons", "count") int count();
		IMPORT("pylons", "nameAlloc") char* nameAlloc(int);
		IMPORT("pylons", "positionX") float positionX(int);
		IMPORT("pylons", "positionY") float positionY(int);
		// IMPORT("pylons", "position") Position position(int);
		inline Position position(int i) { return Position{ positionX(i), positionY(i) }; }
		IMPORT("pylons", "energyCapacity") int energyCapacity(int);
		IMPORT("pylons", "energy") int energy(int);
		IMPORT("pylons", "controlledBy") int controlledBy(int);
	};

	namespace Star {
		IMPORT("stars", "count") int count();
		IMPORT("stars", "nameAlloc") char* nameAlloc(int);
		IMPORT("stars", "positionX") float positionX(int);
		IMPORT("stars", "positionY") float positionY(int);
		// IMPORT("stars", "position") Position position(int);
		inline Position position(int i) { return Position{ positionX(i), positionY(i) }; }
		IMPORT("stars", "energyCapacity") int energyCapacity(int);
		IMPORT("stars", "energy") int energy(int);
		IMPORT("stars", "energyGenFlat") int energyGenFlat(int);
		IMPORT("stars", "energyGenScaling") float energyGenScaling(int);
	};

	namespace Player {
		IMPORT("players", "count") int count();
		char* name(int);
		IMPORT("players", "nameAlloc") char* nameAlloc(int);
		IMPORT("players", "me") int me();
	};

	namespace Console {
		IMPORT("console", "log") void log(const char*);
	};
};
