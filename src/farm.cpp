#include "farm.h"

#include "objects.h"
#include "position.h"
#include "interface.h"

#include <algorithm>
#include <cmath>

#include "printf.h"


int loss1 = 0, loss2 = 0;
void farm() {
	if (available.size() <= 0)
		return;

	float bsDist = dist(bases[0], stars[0]);

	auto bsNorm = normalize(stars[0] - bases[0]);
	auto P1 = bases[0] + bsNorm * 198;
	auto P2A = bases[0] + bsNorm * 396;
	auto P2B = bases[0] + bsNorm * (bsDist - 198);

	float travelTime = std::ceil((bsDist - 598.5f) / 20) - 1;

	float farmer2CycleTime = 2 * (10 + travelTime);
	float perfectRatio = 1 - 10 / (10 + farmer2CycleTime);


	std::sort(available.begin(), available.end(), [](auto* a, auto* b){
		return a->db - a->ds < b->db - b->ds;
	});
	std::vector<MySpirit*>::iterator farmers1End = available.begin();
	Position haulA, targetA;
	if (available.size() < 3 || (available.size() == 3 && bases[0].energy <= 1.8 * available[0]->energyCapacity)) {
		haulA = P1;
		targetA = bases[0];
	} else {
		haulA = P2A;
		targetA = P1;

		int count1;
		if (currentTick < 14)
			count1 = available.size() / 2;
		else {
			count1 = std::max(1.f, std::roundf(available.size() * (1 - perfectRatio - .0)));

			int oldCount1 = 0;
			for (; oldCount1 < available.size(); oldCount1++)
				if (available[oldCount1]->db - available[oldCount1]->ds > -bsDist + 400)
					break;
			int oldCount2 = available.size() - oldCount1;
			if (oldCount2 > available.size() - count1)
				count1--;
			count1 = std::max(1, count1);
		}

		farmers1End += count1;

		for (auto it = available.begin(); it < farmers1End; it++) {
			auto* s = *it;
			if (s->db <= 200)
				s->move(P1);
			else {
				s->move(bases[0]);
				// move it to start of farmers2 and fix sizes
				std::rotate(it, it + 1, farmers1End--);
			}
			if (!s->energy)
				loss1++;
			if (!s->usedEnergize && s->db <= 200 && bases[0].energy < bases[0].energyCapacity)
				s->energizeBase(bases[0]);
		}

		std::sort(available.begin(), farmers1End, [](auto*& a, auto*& b){
			return a->energy < b->energy;
		});
		for (auto it = available.end(); it-- != farmers1End; ) {
			auto* s = *it;
			float d2a = dist(*s, haulA);
			float d2b = dist(*s, P2B);

			if (!s->usedEnergize) {
				bool hasTarget = false;
				if (s->energy > 0) {

					if (dist(*s, bases[0]) <= 200 && bases[0].energy <= bases[0].energyCapacity) {
						s->energizeBase(bases[0]);
						hasTarget = true;
					} else
						for (auto j = available.begin(); j < farmers1End; j++) {
							auto* f = *j;
							if (f->energy + std::min(s->size, s->energy) >= f->energyCapacity)
								break; // all remaining targets are too full
							if (dist(*s, *f) > 200)
								continue;

							s->energize(*f);
							hasTarget = true;

							auto k = j + 2;
							for (; k < farmers1End; k++)
								if ((*k)->energy >= f->energy)
									break;
							if ((*(k - 1))->energy < f->energy)
								std::iter_swap(j, k-1);
							
							break;
						}
				}
				// no energy or no targets
				if (!hasTarget) {
					if (s->ds <= 200 && s->energy < s->energyCapacity) {
						s->charge();
					} else if (s->energy > 0) {
						for (auto j = farmers1End; j < available.end(); j++) {
							auto* t = *j;
							if (t->energy + std::min(s->energy, s->size) <= t->energyCapacity && (t->db <= 375 || d2a > s->energy * 20) && t->db - 20 < s->db) {
								s->energize(*t);
								hasTarget = true;
								break;
							}
						}
						if (!hasTarget && s->db < 400)
							loss2++;
					}
				}
			}

			if (!s->usedMove) {
				if (s->energy == 0 || (s->energy < s->energyCapacity && (d2b < 18.5 || d2a > s->energy * 20)))
					s->move(s->ds > 201.5 ? stars[0] : P2B);
				else {
					bool noMove = false;
					if (s->energy != 0 && s->db <= 201.5 && farmers1End == available.begin()) {
						noMove = true;
						for (auto j = available.begin(); j < farmers1End; j++)
							if (dist(*s, **j) <= 200) {
								noMove = false;
								break;
							}
					}
					if (!noMove)
						s->move(dist(*s, targetA) > 201.5 ? targetA : haulA);
				}
			}
		}
	}
	
	println("loss1: %i loss2: %i", loss1, loss2);
}