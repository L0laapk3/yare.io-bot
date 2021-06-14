#include "objects.h"

#include "interface.h"

#include <algorithm>

#include "printf.h"



int currentTick = 0;

float myStrength;
float enemyStrength;

std::vector<Star> stars;
std::vector<Base> bases;
std::vector<MySpirit> units;
std::vector<MySpirit*> available;
std::vector<EnemySpirit> enemies;
std::vector<EnemySpirit*> enemiesSort;
std::map<int, Position> lastEnemyPositions;

void parseTick(int tick) {
	currentTick = tick;
	myStrength = 0;
	enemyStrength = 0;
	stars.clear();
	bases.clear();
	units.clear();
	available.clear();
	enemies.clear();
	enemiesSort.clear();

	int bCount = Interface::baseCount();
	for (int i = 0; i < bCount; i++) {
		if (Interface::baseHp(i) <= 0)
			continue;
		Base base{
			{ Interface::basePositionX(i), Interface::basePositionY(i) },
			.index = i,
			.size = Interface::baseSize(i),
			.energyCapacity = Interface::baseEnergyCapacity(i),
			.energy = Interface::baseEnergy(i),
			.spiritCost = Interface::baseCurrentSpiritCost(i),
		};
		if (base.energy >= base.spiritCost)
			base.energy -= base.spiritCost;
		bases.push_back(base);
	}

	int stCount = Interface::starCount();
	for (int i = 0; i < stCount; i++) {
		stars.push_back({
			{ Interface::starPositionX(i), Interface::starPositionY(i) },
		});
	}
	std::sort(stars.begin(), stars.end(), [&](auto& a, auto& b){
		return dist(a, bases[0]) < dist(b, bases[0]);
	});

	int sCount = Interface::spiritCount();
	for (int i = 0; i < sCount; i++) {
		if (Interface::spiritHp(i) <= 0)
			continue;
		Spirit spirit{
			{ Interface::spiritPositionX(i), Interface::spiritPositionY(i) },
			.index = i,
			.size = Interface::spiritSize(i),
			.shape = Interface::spiritShape(i),
			.energyCapacity = Interface::spiritEnergyCapacity(i),
			.energy = Interface::spiritEnergy(i),
			.id = Interface::spiritId(i),
		};
		spirit.db = dist(spirit, bases[0]);
		spirit.ds = dist(spirit, stars[0]);
		spirit.deb = dist(spirit, bases[1]);
		spirit.des = dist(spirit, stars[1]);

		if (Interface::spiritIsFriendly(i)) {
			myStrength += spirit.strength();
			auto& mySpirit = units.emplace_back(MySpirit{ spirit });
			mySpirit._energize(spirit); // default action
		} else {
			enemyStrength += spirit.strength();
			const auto& lastPosIt = lastEnemyPositions.find(spirit.id);
			auto& enemySpirit = enemies.emplace_back(EnemySpirit{
				spirit,
				.velocity = lastPosIt != lastEnemyPositions.end() ? spirit.position - lastPosIt->second : Position{ 0, 0 },
			});
		}
	}

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


float Spirit::strength() {
	return size * (shape == Shape::Square ? 112.f/200.f : 1.f);
}


void MySpirit::charge() {
	_energize(*this);
	energy = std::min(energy + size, energyCapacity);
	usedEnergize = true;
}
void MySpirit::attack(EnemySpirit& s) {
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
void MySpirit::energizeBase(Base& b) {
	_energizeBase(b);
	b.energy = std::min(b.energy + std::min(energy, size), b.energyCapacity);
	energy -= std::min(energy, size);
	usedEnergize = true;
}
void MySpirit::attackBase(Base& b) {
	_energizeBase(b);
	b.energy -= 2 * std::min(energy, size);
	energy -= std::min(energy, size);
	usedEnergize = true;
}


void MySpirit::_energize(const Spirit& s) {
	Interface::spiritEnergize(index, s.index);
}
void MySpirit::_energizeBase(const Base& b) {
	Interface::spiritEnergizeBase(index, b.index);
}
void MySpirit::move(const Position& p) {
	Interface::spiritMove(index, p.x, p.y);
	usedMove = true;
}
void MySpirit::merge(const Spirit& s) {
	Interface::spiritMerge(index, s.index);
}
void MySpirit::divide() {
	Interface::spiritDivide(index);
}
void MySpirit::jump(const Position& p) {
	Interface::spiritJump(index, p.x, p.y);
}
void MySpirit::shout(const char* str) {
	Interface::spiritShout(index, str);
}