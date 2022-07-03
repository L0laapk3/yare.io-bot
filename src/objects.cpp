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

	for (int i = 0; i < bCount; i++) {
		Base base;
		base.position = Interface::Base::position(i);
		base.energyCapacity = Interface::Base::energyCapacity(i);
		base.energy = Interface::Base::energy(i);
		base.controlledBy = Interface::Base::controlledBy(i);
		base.type = Base::TYPE;
		base.index = i;
		// base.spiritCost = Interface::Base::currentSpiritCost(i);
		base.spiritCosts = std::vector<std::pair<int, int>>(Interface::Base::spiritCostCount(i));
		for (int j = 0; j < base.spiritCosts.size(); j++)
			base.spiritCosts[base.spiritCosts.size() - 1 - j] = { Interface::Base::spiritCostTreshold(i, j), Interface::Base::spiritCostValue(i, j) };

		if (base.controlledBy == myPlayerId)
			bases.insert(bases.begin(), base);
		else
			bases.push_back(base);
	}

	int oCount = Interface::Outpost::count();
	for (int i = 0; i < oCount; i++) {
		Outpost outpost;
		outpost.index = i;
		outpost.position = Interface::Outpost::position(i);
		outpost.energyCapacity = Interface::Outpost::energyCapacity(i);
		outpost.energy = Interface::Outpost::energy(i);
		outpost.controlledBy = Interface::Outpost::controlledBy(i);
		outpost.range = Interface::Outpost::range(i);
		outpost.type = Outpost::TYPE;
		outposts.emplace_back(outpost);
	}

	int pCount = Interface::Outpost::count();
	for (int i = 0; i < pCount; i++) {
		Pylon pylon;
		pylon.index = i;
		pylon.position = Interface::Pylon::position(i);
		pylon.energyCapacity = Interface::Pylon::energyCapacity(i);
		pylon.energy = Interface::Pylon::energy(i);
		pylon.controlledBy = Interface::Pylon::controlledBy(i);
		pylon.type = Pylon::TYPE;
		pylons.emplace_back(pylon);
	}

	int stCount = Interface::Star::count();
	for (int i = 0; i < stCount; i++) {
		Star star{
			Interface::Star::position(i),
			.energyCapacity = 1000,//Interface::Star::energyCapacity(i),
			.energy = Interface::Star::energy(i),
			.index = i
		};
		star.activatesIn = 0;
		star.energyGenFlat = Interface::Star::energyGenFlat(i);
		star.energyGenScaling = Interface::Star::energyGenScaling(i);
		stars.emplace_back(star);
	}
	std::sort(stars.begin(), stars.end(), [&](auto& a, auto& b){
		return dist(a, bases[0]) < dist(b, bases[0]);
	});

	int sCount = Interface::Spirit::count();
	for (int i = 0; i < sCount; i++) {
		if (Interface::Spirit::hp(i) <= 0)
			continue;
		Spirit spirit{
			Interface::Spirit::position(i),
			.index = i,
			.size = Interface::Spirit::size(i),
			.shape = Interface::Spirit::shape(i),
			.energyCapacity = Interface::Spirit::energyCapacity(i),
			.energy = Interface::Spirit::energy(i),
			.id = Interface::Spirit::id(i),
		};
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



Object::operator Position() {
	return position;
}

int shapeSize(Shape& shape) {
	switch (shape) {
	case Shape::CIRCLE:
		return 1;
	case Shape::SQUARE:
		return 10;
	case Shape::TRIANGLE:
		return 3;
	}
}


template<>
int Base::spiritCost<Shape::CIRCLE>(int spirits) {
	if (spirits > 200)
		return 150;
	if (spirits > 100)
		return 90;
	if (spirits > 50)
		return 50;
	return 25;
}
template<>
int Base::spiritCost<Shape::SQUARE>(int spirits) {
	if (spirits > 17)
		return 700;
	if (spirits > 10)
		return 500;
	return 360;
}
template<>
int Base::spiritCost<Shape::TRIANGLE>(int spirits) {
	if (spirits > 120)
		return 300;
	if (spirits > 30)
		return 160;
	return 90;
}
int Base::spiritCost(Shape shape, int totalTeamSpirits) {
	switch (shape) {
		case Shape::CIRCLE:
		default:
			return spiritCost<Shape::CIRCLE>(totalTeamSpirits);
		case Shape::SQUARE:
			return spiritCost<Shape::SQUARE>(totalTeamSpirits);
		case Shape::TRIANGLE:
			return spiritCost<Shape::TRIANGLE>(totalTeamSpirits);
	}
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