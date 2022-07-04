#include "objects.h"

#include "safeMove.h"
#include "interface.h"
#include "printf.h"

#include <algorithm>




int currentTick = 0;
int myPlayerId;

float myStrength;
float enemyStrength;

std::vector<Star> stars;
std::vector<Base> bases;
std::vector<Outpost> outposts;
std::vector<Pylon> pylons;
std::vector<MySpirit> units;
std::vector<MySpirit*> available;
std::vector<EnemySpirit> enemies;
std::vector<EnemySpirit*> enemiesSort;
std::map<int, Position> lastEnemyPositions;

void parseTick(int tick) {
	currentTick = tick;
	myStrength = 0;
	enemyStrength = 0;
	myPlayerId = Interface::Player::me();
	stars.clear();
	bases.clear();
	outposts.clear();
	units.clear();
	available.clear();
	enemies.clear();
	enemiesSort.clear();

	int bCount = Interface::Base::count();
	for (int i = 0; i < bCount; i++)
		bases.push_back(Base::build(i));

	int oCount = Interface::Outpost::count();
	for (int i = 0; i < oCount; i++)
		outposts.emplace_back(Outpost::build(i));

	int pCount = Interface::Outpost::count();
	for (int i = 0; i < pCount; i++)
		pylons.emplace_back(Pylon::build(i));

	int stCount = Interface::Star::count();
	for (int i = 0; i < stCount; i++)
		stars.emplace_back(Star::build(i));

	int sCount = Interface::Spirit::count();
	for (int i = 0; i < sCount; i++) {
		Spirit spirit = Spirit::build(i);

		spirit.db = dist(spirit, bases[0]);
		spirit.ds = dist(spirit, stars[0]);
		spirit.deb = dist(spirit, bases[bCount-1]);
		spirit.des = dist(spirit, stars[stCount-1]);

		if (Interface::Spirit::playerId(i) == myPlayerId) {
			myStrength += spirit.maxStrength();
			auto& mySpirit = units.emplace_back(MySpirit{ spirit });
			if (mySpirit.energy < mySpirit.energyCapacity)
				mySpirit._energize(spirit); // default action
		} else {
			enemyStrength += spirit.maxStrength();
			const auto& lastPosIt = lastEnemyPositions.find(spirit.id);
			auto& enemySpirit = enemies.emplace_back(EnemySpirit{
				spirit,
				.velocity = lastPosIt != lastEnemyPositions.end() ? spirit.position - lastPosIt->second : Position{ 0, 0 },
			});
		}
	}

	safeMoveReserve(units.size(), enemies.size(), outposts.size());

	lastEnemyPositions.clear();
	for (auto& s : enemies) {
		enemiesSort.push_back(&s);
		lastEnemyPositions.emplace(s.id, s.position);
	}
	for (auto& s : units)
		available.push_back(&s);

	println("%.0f (%i) vs %.0f (%i) at tick %i", myStrength, units.size(), enemyStrength, enemies.size(), currentTick);
}


Base Base::build(int i) {
	Base base;
	base.position = Interface::Base::position(i);
	base.shape = Interface::Base::shape(i),
	base.energyCapacity = Interface::Base::energyCapacity(i);
	base.energy = Interface::Base::energy(i);
	base.controlledBy = Interface::Base::controlledBy(i);
	base.type = Base::TYPE;
	base.index = i;
	// base.spiritCost = Interface::Base::currentSpiritCost(i);
	base.spiritCosts = std::vector<std::pair<int, int>>(Interface::Base::spiritCostCount(i));
	for (int j = 0; j < base.spiritCosts.size(); j++)
		base.spiritCosts[base.spiritCosts.size() - 1 - j] = { Interface::Base::spiritCostTreshold(i, j), Interface::Base::spiritCostValue(i, j) };
	return base;
}
Outpost Outpost::build(int i) {
	Outpost outpost;
	outpost.index = i;
	outpost.position = Interface::Outpost::position(i);
	outpost.energyCapacity = Interface::Outpost::energyCapacity(i);
	outpost.energy = Interface::Outpost::energy(i);
	outpost.controlledBy = Interface::Outpost::controlledBy(i);
	outpost.range = Interface::Outpost::range(i);
	outpost.type = Outpost::TYPE;
	return outpost;
}
Pylon Pylon::build(int i) {
	Pylon pylon;
	pylon.index = i;
	pylon.position = Interface::Pylon::position(i);
	pylon.energyCapacity = Interface::Pylon::energyCapacity(i);
	pylon.energy = Interface::Pylon::energy(i);
	pylon.controlledBy = Interface::Pylon::controlledBy(i);
	pylon.type = Pylon::TYPE;
	return pylon;
}
Star Star::build(int i) {
	Star star;
	star.index = i;
	star.position = Interface::Star::position(i);
	star.energyCapacity = Interface::Star::energyCapacity(i);
	star.energy = Interface::Star::energy(i);
	star.activatesIn = 0;
	star.energyGenFlat = Interface::Star::energyGenFlat(i);
	star.energyGenScaling = Interface::Star::energyGenScaling(i);
	return star;
}
Spirit Spirit::build(int i) {
	Spirit spirit;
	spirit.position = Interface::Spirit::position(i),
	spirit.index = i;
	spirit.size = Interface::Spirit::size(i);
	spirit.shape = Interface::Spirit::shape(i);
	spirit.energyCapacity = Interface::Spirit::energyCapacity(i);
	spirit.energy = Interface::Spirit::energy(i);
	spirit.id = Interface::Spirit::id(i);
	spirit.locked = Interface::Spirit::locked(i);
	spirit.range = Interface::Spirit::range(i);

	spirit.minRange = spirit.shape == Shape::SQUARE ? 200 : spirit.range;
	spirit.maxRange = spirit.shape == Shape::SQUARE ? 300 : spirit.range;
	spirit.rangeGrowth = spirit.shape == Shape::SQUARE ? 25 : 0;
	return spirit;
}


Object::operator Position() {
	return position;
}

int shapeSize(Shape& shape) {
	switch (shape) {
	case Shape::CIRCLE:
	default:
		return 1;
	case Shape::SQUARE:
		return 10;
	case Shape::TRIANGLE:
		return 3;
	}
}

int Base::spiritCost(int spirits) {
	for (int i = 0; i < spiritCosts.size() - 1; i++)
		if (spirits >= spiritCosts[i].first)
			return spiritCosts[i].second;
	return spiritCosts.back().second;
}

float Outpost::strength() {
	return energy;
}
bool ChargeTarget::isFriendly() {
	return controlledBy == myPlayerId;
}

float Pylon::strength() {
	return 0.f;
}

float Spirit::strength() {
	return energy * (shape == Shape::SQUARE ? 112.f/200.f : 1.f);
}
float Spirit::maxStrength() {
	return energyCapacity * (shape == Shape::SQUARE ? 112.f/200.f : 1.f);
}


void MySpirit::charge(Star& s) {
	_energize(*this);
	s.energy -= size;
	energy = std::min(energy + size, energyCapacity);
	usedEnergize = true;
}
void MySpirit::attack(EnemySpirit& s) {
	shout("⚔️");
	_energize(s);
	s.energy -= 2 * std::min(energy, size);
	energy -= std::min(energy, size);
	usedEnergize = true;
}
void MySpirit::energize(MySpirit& s) {
	_energize(s);
	s.energy = std::min(s.energy + std::min(energy, size), s.energyCapacity);
	energy -= std::min(energy, size);
	usedEnergize = true;
}


void ChargeTarget::energizeResolveIntf(int sindex) {
	switch (type) {
		case Base::TYPE:
		default:
			Interface::Spirit::energizeBase(sindex, index);
			break;
		case Outpost::TYPE:
			Interface::Spirit::energizeOutpost(sindex, index);
			break;
		case Pylon::TYPE:
			Interface::Spirit::energizePylon(sindex, index);
			break;
	}
}

void MySpirit::energize(ChargeTarget& b) {
	b.energizeResolveIntf(index);
	b.energy = std::min(b.energy + std::min(energy, size), b.energyCapacity);
	energy -= std::min(energy, size);
	usedEnergize = true;
}
void MySpirit::attack(ChargeTarget& b) {
	shout("⚔️");
	b.energizeResolveIntf(index);
	b.energy -= 2 * std::min(energy, size);
	energy -= std::min(energy, size);
	usedEnergize = true;
}


void MySpirit::_energize(const Spirit& s) {
	Interface::Spirit::energize(index, s.index);
}
void MySpirit::_energizeBase(const Base& b) {
	Interface::Spirit::energizeBase(index, b.index);
}
void MySpirit::move(const Position& p) {
	Interface::Spirit::move(index, p.x, p.y);
	usedMove = true;
}
void MySpirit::merge(const Spirit& s) {
	Interface::Spirit::merge(index, s.index);
}
void MySpirit::divide() {
	Interface::Spirit::divide(index);
}
void MySpirit::jump(const Position& p) {
	Interface::Spirit::jump(index, p.x, p.y);
}
void MySpirit::shout(const char* str) {
	Interface::Spirit::shout(index, str);
}




std::vector<char*> stringAllocs;
char* Spirit::name() {
    char* cStr = Interface::Spirit::nameAlloc(index);
	stringAllocs.push_back(cStr);
	return cStr;
}

char* ChargeTarget::name() {
	switch (type) {
		case Base::TYPE:
		default:
			return ((Base*)this)->name();
		case Outpost::TYPE:
			return ((Outpost*)this)->name();
		case Pylon::TYPE:
			return ((Pylon*)this)->name();
	}
}

char* Base::name() {
    char* cStr = Interface::Base::nameAlloc(index);
	stringAllocs.push_back(cStr);
	return cStr;
}

char* Outpost::name() {
    char* cStr = Interface::Outpost::nameAlloc(index);
	stringAllocs.push_back(cStr);
	return cStr;
}

char* Pylon::name() {
    char* cStr = Interface::Pylon::nameAlloc(index);
	stringAllocs.push_back(cStr);
	return cStr;
}

char* Star::name() {
    char* cStr = Interface::Star::nameAlloc(index);
	stringAllocs.push_back(cStr);
	return cStr;
}

void dealloc() {
	for (auto i = stringAllocs.end(); i --> stringAllocs.begin(); ) {
		free(stringAllocs.back());
		stringAllocs.pop_back();
	}
}