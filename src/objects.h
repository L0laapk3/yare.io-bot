#pragma once

#include "position.h"

#include <vector>
#include <map>


enum Shape {
	circle = 0,
	square = 1,
	triangle = 2,
};


struct Object {
	Position position;
	operator Position();
};

struct Star : public Object {
};

struct Base : public Object {
	int index;
	int size;
	int energyCapacity;
	int energy;
	int spiritCost;
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
};


struct EnemySpirit : public Spirit {
	Position velocity{ 0, 0 };
};

struct MySpirit : public Spirit {
	bool usedEnergize = false;
	bool usedMove = false;

	void charge();
	void attack(EnemySpirit&);
	void energize(MySpirit&);
	void _energize(const Spirit&);
	void energizeBase(Base&);
	void attackBase(Base&);
	void _energizeBase(const Base&);
	void move(const Position&);
	void merge(const Spirit&);
	void divide();
	void jump(const Position&);
	void shout(const char*);
};


extern float myStrength;
extern float enemyStrength;

extern int currentTick;

extern std::vector<Star> stars;
extern std::vector<Base> bases;
extern std::vector<MySpirit> units;
extern std::vector<MySpirit*> available;
extern std::vector<EnemySpirit> enemies;
extern std::vector<EnemySpirit*> enemiesSort;
extern std::map<int, Position> lastEnemyPositions;

void parseTick(int tick);



#include "printf.h"
