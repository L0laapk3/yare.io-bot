#include "objects.h"

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
		Base base{
			Interface::Base::position(i),
			.index = i,
			.energyCapacity = Interface::Base::energyCapacity(i),
			.energy = Interface::Base::energy(i),
			.spiritCost = Interface::Base::currentSpiritCost(i),
			.controlledBy = Interface::Base::controlledBy(i),
		};
		if (base.energy >= base.spiritCost)
			base.energy -= base.spiritCost;

		if (base.controlledBy == myPlayerId)
			bases.insert(bases.begin(), base);
		else
			bases.push_back(base);
	}

	int oCount = Interface::Outpost::count();
	for (int i = 0; i < oCount; i++) {
		outposts.emplace_back(Outpost{
			Interface::Outpost::position(i),
			.energyCapacity = Interface::Outpost::energyCapacity(i),
			.energy = Interface::Outpost::energy(i),
			.range = Interface::Outpost::range(i),
			.controlledBy = Interface::Outpost::controlledBy(i),
		});
	}

	int stCount = Interface::Star::count();
	for (int i = 0; i < stCount; i++) {
		Star star{
			Interface::Star::position(i),
			.energyCapacity = 1000,//Interface::Star::energyCapacity(i),
			.energy = Interface::Star::energy(i),
		};
		star.activatesIn = star.energy == 0 ? 100 - currentTick : 0;
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


float Outpost::strength() {
	return energy;
}
bool Outpost::isFriendly() {
	return controlledBy == myPlayerId;
}

float Spirit::strength() {
	return energy * (shape == Shape::Square ? 112.f/200.f : 1.f);
}
float Spirit::maxStrength() {
	return energyCapacity * (shape == Shape::Square ? 112.f/200.f : 1.f);
}


void MySpirit::charge(Star& s) {
	_energize(*this);
	s.energy -= size;
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