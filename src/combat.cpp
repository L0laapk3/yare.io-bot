#include "combat.h"

#include "objects.h"

#include <algorithm>
#include <vector>
#include <map>
#include <math.h>

void processAttacks() {

	for (auto& s : enemies)
		if (s.energy > 0) {
			if (dist(s, bases[0]) <= s.range)
				goto attack;
			for (auto& t : units)
				if (dist(s, t) <= s.range)
					goto attack;
			continue;

			attack:
			s.energy = std::max(s.energy - s.size, 0);
		}
		

	// might be a bit, hopefully its ok
	std::sort(enemiesSort.begin(), enemiesSort.end(), [](auto*& a, auto*& b){
		return a->energy < b->energy;
	});
	std::sort(available.begin(), available.end(), [](auto*& a, auto*& b){
		return a->energy > b->energy;
	});

	for (auto it = available.begin(); it < available.end(); it++) {
		auto*& s = *it;
		if (s->energy > 0) {
			if (dist(*s, bases[1]) <= s->range) {
				s->attackBase(bases[1]);
				continue;
			}
			for (auto*& t : enemiesSort)
				if (t->energy >= (t->ds <= 200 || t->des <= 200 ? -t->size : 0) && dist(*s, *t) <= s->range) {
					s->attack(*t);
					break;
				}
		}
	}
}





enum SweepPointType {
	Add,
	Subtract,
	Interest,
};
struct SweepPoint {
	float angle;
	float strength;
	SweepPointType type;
};
std::vector<SweepPoint> angles;
void moveCombatAdvantage(MySpirit& s, Position& targetPosition);

std::map<int, int> pastDefenders;
void defend() {
	angles.reserve(available.size() * 2 + enemies.size() * 2 + 1);

	constexpr int tooClose = 500;
	std::vector<MySpirit*> defenders;
	for (auto& t : enemies) {
		if (t.db > 200 + tooClose + (t.shape == Shape::Square ? 300 : 0))
			continue;
		

		float attackStrength = t.strength();


		bool isStill = norm(t.velocity) == 0;
		auto velDir = normalize(t.velocity);
		while (attackStrength > 0) {
			float minCost = std::numeric_limits<float>::infinity(), minDist = std::numeric_limits<float>::infinity();
			std::vector<MySpirit*>::iterator best;
			if (!isStill) {
				// intercept code
				for (auto it = available.begin(); it < available.end(); it++) {
					auto*& s = *it;
					if (s->energy <= 0 || s->usedMove)
						continue;
					auto ts = *s - t;
					float d = norm(ts);
					if (d >= minDist * 2 + 200)
						continue;
					float travelDist = isStill ? d : d <= 200 ? 0 : (dot(ts, ts) - 200*200) / (2 * (200 + dot(velDir, ts)));
					if (travelDist < 800) {
						float cost = travelDist;
						if (t.energy >= s->energy)
							cost += 10 * (t.energy - s->energy + 1);
						if (travelDist >= 0 && cost < minCost) {
							minCost = cost;
							minDist = travelDist;
							best = it;
						}
					}
				}
			}
			if (minDist == std::numeric_limits<float>::infinity() && t.db <= tooClose) {
				isStill = true;
				for (auto it = available.begin(); it < available.end(); it++) {
					auto*& s = *it;
					if (s->energy <= 0 || s->usedMove)
						continue;
					float travelDist = dist(*s, t);
					float cost = travelDist;
					if (t.energy >= s->energy)
						cost += 10 * (t.energy - s->energy + 1);
					if (cost < minCost) {
						minCost = cost;
						minDist = travelDist;
						best = it;
					}
				}
			}

			if (minCost != std::numeric_limits<float>::infinity()) {
				auto*& s = *best;
				defenders.push_back(s);
				available.erase(best);

				attackStrength -= s->strength();

				auto interceptPos = isStill ? t : t + velDir * minDist;

				// todo: grouping
				if (t.db < 450)
					s->move(interceptPos);
				else
					s->move(inDirection(interceptPos, *s, 200));
				
			} else
				break; // cannot find any more defenders
		}
	}

	for (auto*& d : defenders) {
		d->shout("🛡️");
		for (auto*& s : available) {
			if (d->energy >= d->energyCapacity)
				break;
			if (s->energy > 0 && !s->usedEnergize)
				s->energize(*d);
		}
	}
}



void attack() {
	for (auto*& s : available)
		s->move(inDirection(bases[1], *s, 200));
}









void moveCombatAdvantage(MySpirit& s, Position& targetPosition) {

	constexpr float moveDist = 20;
	constexpr float attackDist = 203;
	constexpr float targetWeight = 0.1 / (2*moveDist);

	angles.clear();
	angles.emplace_back(SweepPoint{
		.angle = atan2(targetPosition - s),
		.type = SweepPointType::Interest,
	});
	
	float currentStrength = s.strength();

	// for (auto it = available.begin(); it != available.end(); it++) {
	// 	auto*& other = *it;
	// 	float d = dist(s, *other);

	// 	if (&s != other && d < 2*2*moveDist) {
	// 		float strength = other->strength();
	// 		if (d == 0) {
	// 			currentStrength += strength;
	// 			continue;
	// 		}
	// 		float B = acosf(d / (2*2*moveDist));
	// 		float A = atan2(*other - s);
	// 		angles.emplace_back(SweepPoint{
	// 			.angle = A - B,
	// 			.strength = strength,
	// 			.type = SweepPointType::Add,
	// 		});
	// 		angles.emplace_back(SweepPoint{
	// 			.angle = A + B,
	// 			.strength = strength,
	// 			.type = SweepPointType::Subtract,
	// 		});
	// 		// angles.emplace_back(SweepPoint{
	// 		// 	.angle = A,
	// 		// 	.type = SweepPointType::Interest,
	// 		// });
	// 	}
	// }
	// for (auto& other : enemies) {
	// 	float d = dist(s, other);
	// 	float strength = other.strength();
	// 	if (d >= attackDist + moveDist)
	// 		continue; // never in range
			
	// 	currentStrength -= strength;

	// 	if (d <= attackDist - moveDist) 
	// 		continue; // always in range

	// 	float B = acosf((d - (attackDist - moveDist)) / (2*moveDist));
	// 	float A = atan2(other - s);
	// 	angles.emplace_back(SweepPoint{
	// 		.angle = A - B,
	// 		.strength = strength,
	// 		.type = SweepPointType::Add,
	// 	});
	// 	angles.emplace_back(SweepPoint{
	// 		.angle = A + B,
	// 		.strength = strength,
	// 		.type = SweepPointType::Subtract,
	// 	});
	// 	// angles.emplace_back(SweepPoint{
	// 	// 	.angle = A,
	// 	// 	.type = SweepPointType::Interest,
	// 	// });
	// }

	std::sort(angles.begin(), angles.end(), [](auto& a, auto& b){
		return a.angle < b.angle;
	});

	float bestScore = -std::numeric_limits<float>::infinity();
	float bestAngle;
	for (auto it = angles.begin(); it != angles.end(); it++) {
		if (it->type == SweepPointType::Add)
			currentStrength += it->strength;

		auto nextPos = s + moveDist * fromAngle(it->angle);
		float score = std::max(0.f, currentStrength) + targetWeight * dist(nextPos, targetPosition);
		if (score > bestScore) {
			bestScore = score;
			bestAngle = it->angle;
		}

		if (it->type == SweepPointType::Subtract)
			currentStrength -= it->strength;
	}
	println("d %f", bestAngle);

	s.move(s + (moveDist + 1) * fromAngle(bestAngle));
}