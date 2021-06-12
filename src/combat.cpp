#include "combat.h"

#include "objects.h"

#include <algorithm>
#include <map>


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
		auto* s = *it;
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






std::map<int, int> pastDefenders;
void defend() {
	constexpr int tooClose = 500;
	// std::vector<Spirit*> defenders;
	for (auto& t : enemies) {
		if (t.db > 200 + tooClose + (t.shape == Shape::square ? 300 : 0))
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
					auto* s = *it;
					if (s->energy <= 0)
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
					auto* s = *it;
					if (s->energy <= 0)
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
				auto* s = *best;
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
}



void attack() {
	for (auto*& s : available)
		s->move(bases[1]);
}