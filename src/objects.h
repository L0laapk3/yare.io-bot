#pragma once

#include "interface.h"

#include <vector>
#include <utility>
#include <map>


struct Object {
	Position position;
	operator Position();
};

struct Star : public Object {
	
// interface
	int energyCapacity;
	int energy;
	int index;
	char* name();
	int energyGenFlat = 2;
	float energyGenScaling = .02f;
	int activatesIn;

// self defined
	float usedEnergyGeneration = 0;

	static Star build(int i);
};


struct MySpirit;
struct ChargeTarget : public Object {
	enum Type {
		BASE,
		OUTPOST,
		PYLON,
	};
	char* name();
	int energyCapacity;
	int energy;
	int controlledBy;
	bool isFriendly();
	int index;
	Type type;

	void energizeResolveIntf(int sindex);
};

struct Base : public ChargeTarget {
	constexpr static Type TYPE = Type::BASE;
	char* name();
	Shape shape;
	int spiritCost(int totalTeamSpirits);
	std::vector<std::pair<int, int>> spiritCosts;

	static Base build(int i);
};

struct Outpost : public ChargeTarget {
	constexpr static Type TYPE = Type::OUTPOST;
	char* name();
	float range;

	float strength();

	static Outpost build(int i);
};

struct Pylon : public ChargeTarget {
	constexpr static Type TYPE = Type::PYLON;
	char* name();
	constexpr static float minRange = 200.f;
	constexpr static float maxRange = 400.f;

	float strength();

	static Pylon build(int i);
};

struct Spirit : public Object {
	int index;
	char* name();
	int size;
	Shape shape;
	int energyCapacity;
	int energy;
	int id;
	bool locked;
	int range;
	int minRange;
	int maxRange;
	int rangeGrowth;
	
	static constexpr int moveSpeed = 20;

	float db;
	float ds;
	float deb;
	float des;

	float strength();
	float maxStrength();

	static Spirit build(int i);
};

int shapeSize(Shape& shape);


struct EnemySpirit : public Spirit {
	Position velocity{ 0, 0 };

	static EnemySpirit build(int i);
};

struct MySpirit : public Spirit {
	bool usedEnergize = false;
	bool usedMove = false;

// interface
	void charge(Star&);
	void attack(EnemySpirit&);
	void energize(MySpirit&);
	void energize(ChargeTarget&);
	void attack(ChargeTarget&);
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

	static MySpirit build(int i);
};


extern int myPlayerId;
extern int currentTick;

extern std::vector<Star> stars;
extern std::vector<Base> bases;
extern std::vector<Outpost> outposts;
extern std::vector<Pylon> pylons;
extern std::vector<MySpirit> units;
extern std::vector<MySpirit*> available;
extern std::vector<EnemySpirit> enemies;
extern std::vector<EnemySpirit*> enemiesSort;
extern std::map<int, Position> lastEnemyPositions;

extern float myStrength;
extern float enemyStrength;

extern std::vector<char*> stringAllocs;
void dealloc();

void parseTick(int tick);
