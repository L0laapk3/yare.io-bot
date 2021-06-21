#pragma once

#include "position.h"

#include <vector>
#include <map>


enum Shape {
	Circle = 0,
	Square = 1,
	Triangle = 2,
};


struct Object {
	Position position;
	operator Position();
};

struct Star : public Object {
	int energyCapacity;
	int energy;
	int activatesIn;
};

struct Base : public Object {
	int index;
	int energyCapacity;
	int energy;
	int spiritCost;
	int playerId;
};

struct Outpost : public Object {
	int energyCapacity;
	int energy;
	float range;
	int controlledBy;

	float strength();
	bool isFriendly();
};

struct Spirit : public Object {
	int index;
	int size;
	Shape shape;
	int energyCapacity;
	int energy;
	int id;
	const int range = 200;
	float db;
	float ds;
	float deb;
	float des;

	float strength();
	float maxStrength();
};


struct EnemySpirit : public Spirit {
	Position velocity{ 0, 0 };
};

struct MySpirit : public Spirit {
	bool usedEnergize = false;
	bool usedMove = false;

	void charge(Star&);
	void attack(EnemySpirit&);
	void energize(MySpirit&);
	void _energize(const Spirit&);
	void energizeBase(Base&);
	void attackBase(Base&);
	void _energizeBase(const Base&);
	void energizeOutpost(Outpost&);
	void _energizeOutpost(const Outpost&);
	void move(const Position&);
	void merge(const Spirit&);
	void divide();
	void jump(const Position&);
	void shout(const char*);
};


extern int myPlayerId;
extern int currentTick;

extern std::vector<Star> stars;
extern std::vector<Base> bases;
extern std::vector<Outpost> outposts;
extern std::vector<MySpirit> units;
extern std::vector<MySpirit*> available;
extern std::vector<EnemySpirit> enemies;
extern std::vector<EnemySpirit*> enemiesSort;
extern std::map<int, Position> lastEnemyPositions;

extern float myStrength;
extern float enemyStrength;


void parseTick(int tick);



#include "printf.h"
