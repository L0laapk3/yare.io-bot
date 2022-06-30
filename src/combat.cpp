#include "combat.h"

#include "objects.h"

#include <algorithm>
#include <vector>
#include <map>
#include <math.h>


constexpr float F_EPS = std::numeric_limits<float>::epsilon() * 100;

void processAttacks() {

	for (auto& s : enemies)
		if (s.energy > 0) {
			for (auto& base : bases)
				if (base.controlledBy == myPlayerId && dist(s, base) <= s.range)
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
			for (auto& base : bases) {
				if (base.controlledBy != myPlayerId && base.controlledBy != -1)
					if (dist(*s, base) <= s->range) {
						s->attackBase(base);
						continue;
					}
			}
			for (auto*& t : enemiesSort)
				if (t->energy >= (t->ds <= 200 || t->des <= 200 ? -t->size : 0) && dist(*s, *t) <= s->range) {
					s->attack(*t);
					break;
				}
		}
	}
}



void moveCombatAdvantage(MySpirit& s, const Position& targetPosition);
void attackOrCharge(MySpirit& s, const Position& target) {
	Star* closestStar;
	float closestStarDistance = std::numeric_limits<float>::infinity();
	for (auto& star : stars) {
		float d = dist(star, s);
		if (d < closestStarDistance) {
			closestStarDistance = d;
			closestStar = &star;
		}
	}

	if (closestStarDistance < 200 && s.energy < s.energyCapacity)
		s.charge(*closestStar);

	if (std::max(closestStarDistance - 200, 1.f) * s.energy < std::max(dist(s, target) - 200, 1.f) * (s.energyCapacity - s.energy))
		moveCombatAdvantage(s, inDirection(*closestStar, closestStarDistance > 180 ? s : target, 179.9));
	else
		moveCombatAdvantage(s, inDirection(target, s, 179.9));
}




enum SweepPointType {
	Add,
	Subtract,
	Target,
};
struct SweepPoint {
	float angle;
	float strength;
	SweepPointType type;
};
std::vector<SweepPoint> angles;

std::map<int, int> pastDefenders;
void defend() {
	angles.reserve(1 + available.size() * 2 + enemies.size() * 2 + outposts.size());

	constexpr int tooClose = 500;
	std::vector<MySpirit*> defenders;
	for (auto& t : enemies) {
		if (t.db > 200 + tooClose + (t.shape == Shape::Square ? 300 : 0))
			continue;
		

		float attackStrength = std::max(1.f, t.strength());


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
							cost += 20 * (t.energy - s->energy + 1);
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
						cost += 20 * (t.energy - s->energy + 1);
					if (cost < minCost) {
						minCost = cost;
						minDist = travelDist;
						best = it;
					}
				}
			}

			if (minCost != std::numeric_limits<float>::infinity()) {
				auto* s = *best;
				available.erase(best);
				defenders.push_back(s);

				attackStrength -= s->strength();

				auto interceptPos = isStill ? t : t + velDir * minDist;

				if (t.db > 300)
					attackOrCharge(*s, interceptPos);
				else
					s->move(interceptPos);				
			} else
				break; // cannot find any more defenders
		}
	}

	for (auto*& d : defenders) {
		d->shout("⚔️");
		for (auto*& s : available) {
			if (d->energy >= d->energyCapacity)
				break;
			if (!s->usedEnergize && s->energy > 0 && dist(*s, *d) < 200)
				s->energize(*d);
		}
	}
}



void attack() {
	for (auto*& s : available) {
		ChargeTarget* closestTarget = nullptr;
		float closestTargetDistance = std::numeric_limits<float>::infinity();
		for (auto& base : bases) {
			if (base.controlledBy == myPlayerId || base.controlledBy == -1)
				continue;
			float d = dist(base, *s);
			if (d < closestTargetDistance) {
				closestTargetDistance = d;
				closestTarget = &base;
			}
		}
		if (!closestTarget)
			for (auto& outpost : outposts) {
				if (outpost.controlledBy == myPlayerId || outpost.controlledBy == -1)
					continue;
				float d = dist(outpost, *s);
				if (d < closestTargetDistance) {
					closestTargetDistance = d;
					closestTarget = &outpost;
				}
			}

		if (closestTarget)
			attackOrCharge(*s, *closestTarget);
	}
}






template<bool isFriendly>
void addSweepPoints(float A, float B, float strength) {
	float angle1 = A - B;
	float angle2 = isFriendly ? A + B : A + B - 2 * M_PI;
	angles.emplace_back(SweepPoint{
		.angle = angle1 + (isFriendly ? F_EPS : -F_EPS),
		.strength = strength,
		.type = isFriendly ? SweepPointType::Add : SweepPointType::Subtract,
	});
	angles.emplace_back(SweepPoint{
		.angle = angle2 + (isFriendly ? -F_EPS : F_EPS),
		.strength = strength,
		.type = isFriendly ? SweepPointType::Subtract : SweepPointType::Add,
	});
}


void moveCombatAdvantage(MySpirit& s, const Position& targetPosition) {

	static constexpr float MAX_MOVE_DIST = 20;
	static constexpr float TARGET_WEIGHT = -0.1f / (2*MAX_MOVE_DIST);

	float targetAngle = atan2(targetPosition - s);

	angles.clear();
	angles.emplace_back(SweepPoint{
		.angle = targetAngle,
		.type = SweepPointType::Target,
	});
	
	float currentStrength = s.strength();

	for (auto it = available.begin(); it != available.end(); it++) {
		auto*& other = *it;
		float d = dist(s, *other);

		if (&s != other && d < 2*2*MAX_MOVE_DIST) {
			float strength = other->strength();
			if (d == 0) {
				currentStrength += strength;
				continue;
			}
			float B = acosf(d / (2*2*MAX_MOVE_DIST));
			float A = atan2(*other - s);
			addSweepPoints<true>(A, B, strength);
			// angles.emplace_back(SweepPoint{
			// 	.angle = A,
			// 	.type = SweepPointType::Interest,
			// });
		}
	}
	for (auto& other : enemies) {
		constexpr float attackDist = 200.1f + MAX_MOVE_DIST;
		float d = dist(s, other);
		float strength = other.strength();
		if (d >= attackDist + MAX_MOVE_DIST)
			continue; // never in range
			
		currentStrength -= strength;

		if (d <= attackDist - MAX_MOVE_DIST) 
			continue; // always in range

		float B = acosf((d - attackDist - MAX_MOVE_DIST) / (2*MAX_MOVE_DIST));
		float A = atan2(other - s);
		addSweepPoints<false>(A, B, strength);
	}

	for (auto& outpost : outposts) {
		if (outpost.energy <= 0)
			continue;
		float attackDist = outpost.range + .1f;
		float d = dist(s, outpost);

		if (d >= attackDist + MAX_MOVE_DIST)
			continue; // never in range
		
		float strength = outpost.strength();
		if (!outpost.isFriendly())
			currentStrength -= strength;
			
		if (d <= attackDist - MAX_MOVE_DIST) {
			if (outpost.isFriendly())
				currentStrength += strength;
			continue; // always in range
		}

		
		float B = acosf((d - attackDist - MAX_MOVE_DIST) / (2*MAX_MOVE_DIST));
		float A = atan2(outpost - s);
		if (outpost.isFriendly())
			addSweepPoints<true>(A, B, strength);
		else
			addSweepPoints<false>(A, B, strength);
	}

	std::sort(angles.begin(), angles.end(), [](auto& a, auto& b){
		return a.angle < b.angle;
	});

	float bestScore = -std::numeric_limits<float>::infinity();
	std::vector<SweepPoint>::iterator bestSweepPoint;
	for (auto it = angles.begin(); it != angles.end(); it++) {

		if (it->type == SweepPointType::Add)
			currentStrength += it->strength;

		auto nextPos = s + MAX_MOVE_DIST * fromAngle(it->angle);
		// print("%f,%f ", it->angle, currentStrength);	
		float score = std::min(0.f, currentStrength) + TARGET_WEIGHT * dist(nextPos, targetPosition);
		if (score > bestScore) {
			bestScore = score;
			bestSweepPoint = it;
		}

		if (it->type == SweepPointType::Subtract)
			currentStrength -= it->strength;
	}
	// println("d %f", bestAngle);

	if (bestSweepPoint->type == SweepPointType::Target)
		s.move(targetPosition);
	else {
		float moveDist;
		if (std::abs(bestSweepPoint->angle - targetAngle) < M_PI_2 || std::abs(bestSweepPoint->angle - targetAngle) > M_PI + M_PI_2)
			moveDist = MAX_MOVE_DIST;
		else {
			moveDist = MAX_MOVE_DIST; // TODO

		}
		s.move(s + (moveDist + 1) * fromAngle(bestSweepPoint->angle));
	}
}