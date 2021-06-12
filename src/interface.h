#pragma once

#include "objects.h"



#define IMPORT(module, name) __attribute__((import_module(module), import_name(name)))
#define EXPORT(name) __attribute__((export_name(name)))

namespace Interface {
	IMPORT("spirits", "count") int spiritCount();
	IMPORT("spirits", "isFriendly") bool spiritIsFriendly(int);
	IMPORT("spirits", "position") Position spiritPosition(int);
	IMPORT("spirits", "positionX") float spiritPositionX(int);
	IMPORT("spirits", "positionY") float spiritPositionY(int);
	IMPORT("spirits", "size") int spiritSize(int);
	IMPORT("spirits", "shape") Shape spiritShape(int);
	IMPORT("spirits", "energyCapacity") int spiritEnergyCapacity(int);
	IMPORT("spirits", "energy") int spiritEnergy(int);
	IMPORT("spirits", "id") int spiritId(int);
	IMPORT("spirits", "hp") int spiritHp(int);

	IMPORT("spirits", "energize") void spiritEnergize(int, int);
	IMPORT("spirits", "energizeBase") void spiritEnergizeBase(int, int);
	IMPORT("spirits", "move") void spiritMove(int, float, float);
	IMPORT("spirits", "merge") void spiritMerge(int, int);
	IMPORT("spirits", "divide") void spiritDivide(int);
	IMPORT("spirits", "jump") void spiritJump(int, float, float);
	IMPORT("spirits", "shout") void spiritShout(int, const char*);

	IMPORT("bases", "count") int baseCount();
	IMPORT("bases", "position") Position basePosition(int);
	IMPORT("bases", "positionX") float basePositionX(int);
	IMPORT("bases", "positionY") float basePositionY(int);
	IMPORT("bases", "size") int baseSize(int);
	IMPORT("bases", "energyCapacity") int baseEnergyCapacity(int);
	IMPORT("bases", "energy") int baseEnergy(int);
	IMPORT("bases", "currentSpiritCost") int baseCurrentSpiritCost(int);
	IMPORT("bases", "hp") int baseHp(int);

	IMPORT("stars", "count") int starCount();
	IMPORT("stars", "position") Position starPosition(int);
	IMPORT("stars", "positionX") float starPositionX(int);
	IMPORT("stars", "positionY") float starPositionY(int);

	IMPORT("console", "log") void log(const char*);
};
