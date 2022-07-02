#pragma once

#include "interface.h"

#include <vector>
#include <map>


struct Object {
	Position position;
	operator Position();
};

struct Star : public Object {
	int energyCapacity;
	int energy;
	int index;
	int energyGenFlat = 2;
	float energyGenScaling = .02f;
	int activatesIn;
};

struct MySpirit;
struct ChargeTarget : public Object {
	int energyCapacity;
	int energy;
	int controlledBy;
	int index;
	virtual void intf_energizedBy(MySpirit& s) = 0;
};

struct Base : public ChargeTarget {
	static int spiritCost(Shape shape, int totalTeamSpirits);
	template<Shape shape>
	static int spiritCost(int totalTeamSpirits);
	void intf_energizedBy(MySpirit& s) override;
};

struct Outpost : public ChargeTarget {
	float range;

	float strength();
	bool isFriendly();
	void intf_energizedBy(MySpirit& s) override;
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

// interface
	void charge(Star&);
	void attack(EnemySpirit&);
	void energize(MySpirit&);
	void energize(ChargeTarget*);
	void attack(ChargeTarget*);
	void move(const Position&);
	void merge(const Spirit&);
	void divide();
	void jump(const Position&);
	void shout(const char*);

// private:
	void _energize(const Spirit&);
	void _energizeBase(const Base&);
	void _energizeOutpost(const Outpost&);

// self defined functions
	void safeMove(const Position& to);
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
