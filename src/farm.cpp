#include "farm.h"

#include "position.h"
#include "interface.h"
#include "position.h"

#include <algorithm>
#include <cmath>

#include "printf.h"





int tippingPoint(int numChains, int travelTime, Star& star) {
	float perfectEfficiency = 10.f / (10 * numChains + 2 * (10 + travelTime));
	int starEnergyProduction = Star::energyGenFlat + (int)std::roundf(star.energyCapacity * Star::energyGenScaling);
	float e = (starEnergyProduction - Star::energyGenFlat) * 100 - 50;
	int f = starEnergyProduction * numChains + (int)ceilf((float)starEnergyProduction / 10 * 2 * (10 + travelTime));
	float baseE = 0;

	while (e > star.energy) {
		float farmConsumption = perfectEfficiency * f;
		starEnergyProduction = Star::energyGenFlat + (int)std::roundf((e - starEnergyProduction + farmConsumption) * Star::energyGenScaling);
		e -= starEnergyProduction;
		e += farmConsumption;
		baseE -= farmConsumption;
		if (baseE <= 0) {
			baseE += 50;
			f--;
		}
		// println("%i %f %f %i", f, e, f * perfectEfficiency, starEnergyProduction);
	}

	println("%i farmers allowed", f);
	return f;
}



int loss1 = 0, loss2 = 0;
void farmStar(std::vector<MySpirit*>& farmers, Star& star, bool preserve) {
	if (available.size() <= 0)
		return;

	Position P2B;
	Outpost& outpost = outposts[0];
	// println("%i %i %i", dist(outpost, star) <= outpost.range, outpost.energy, outpost.isFriendly());
	float oRange = outpost.range + .1f;
	if (dist(outpost, star) <= oRange && outpost.energy > 0 && !outpost.isFriendly()) {
		auto os = star - outpost;
		float d = norm(os);
		if (d + 199.9f <= oRange)
			return;
		auto proj = os * (oRange*oRange - 199.9f*199.f - dot(os, os)) / (2 * dot(os, os));
		auto orth = sqrtf(199.9f*199.9f - dot(proj, proj)) * rot90(normalize(os), bases[0] - star);
		P2B = star + proj + orth;
	} else
		P2B = star + normalize(bases[0] - star) * 199.9f;

	float P2BsDist = dist(bases[0], P2B);

	int numChains = ceilf((P2BsDist - 19.9f * 6) / 199.9f - 1);

	int travelTime = std::max((int)ceilf((P2BsDist - 199.9f * (1 + numChains)) / 19.9f) - 1, 0);


	auto bP2BNorm = normalize(bases[0] - P2B);
	auto P2A = P2B + bP2BNorm * 19.9f * (1 + travelTime);
	auto P1 = P2A + bP2BNorm * 199.9f;

	float farmer2CycleTime = 2.f * (10 + travelTime);
	float perfectEfficiency = 10.f / (10 * numChains + farmer2CycleTime);

	int starEnergyProduction = 3 + (int)std::roundf(star.energy / 100.f);
	int maxFarmers = starEnergyProduction * numChains + (int)ceilf((float)starEnergyProduction / 10 * 2 * (10 + travelTime));
	if (preserve)
		maxFarmers = std::min(maxFarmers, tippingPoint(numChains, travelTime, star));
	int farmerCount = std::min<int>(maxFarmers, available.size());
	auto farmersEnd = available.begin() + farmerCount;
	// println("dist: %f, numChains: %i, travelTime: %i, farmers: %i/%i%s", P2BsDist + 199.9f, numChains, travelTime, farmerCount, maxFarmers, preserve ? "p" : "");
	if (farmerCount <= 0)
		return;

	std::sort(available.begin(), farmersEnd, [&](auto*& a, auto*& b){
		return a->db - dist(star, *a) < b->db - dist(star, *b);
	});

	for (auto it = available.begin(); it < farmersEnd; it++)
		(*it)->shout("🚜");


	auto farmers1End = available.begin();
	int count1;
	Position haulA, targetA;
	if (available.size() < 3 || (farmerCount == 3 && bases[0].energy <= 1.8 * farmers[0]->energyCapacity)) {
		haulA = P1;
		targetA = bases[0];
		count1 = 0;
	} else {
		haulA = P2A;
		targetA = P1;

		if (currentTick < 14)
			count1 = farmerCount / 2 / numChains;
		else {
			count1 = std::max(1, (int)std::roundf(farmerCount * perfectEfficiency)) * numChains;

			int oldCount1 = 0;
			for (; oldCount1 < farmerCount; oldCount1++)
				if (farmers[oldCount1]->db - dist(star, *farmers[oldCount1]) > -P2BsDist + 200)
					break;
			int oldCount2 = farmerCount - oldCount1;
			if (oldCount2 > farmerCount - count1 && farmerCount != maxFarmers)
				count1 -= numChains;
			count1 = std::max(numChains, count1);
		}

		farmers1End += count1;
	}
	

	// sort groups that have the same distance to base by energy
	for (auto tIt = available.begin(); tIt < farmersEnd; tIt++) {
		auto sIt = tIt;
		auto*& s = *sIt;
		while (tIt < farmersEnd) {
			auto *& t = *tIt;
			if (s->db + 5 <= t->db)
				break;
			tIt++;
		}
		std::sort(sIt, tIt, [&](auto* a, auto* b){
			return a->energy < b->energy;
		});
	}

	auto tStartIt = available.begin();
	for (auto sIt = available.begin(); sIt < farmersEnd; sIt++) {
		auto*& s = *sIt;
		// if (s->db < 220)
		// 	println("%i %f", s->usedEnergize, s->db);
		if (s->usedEnergize)
			continue;
		if (s->energy > 0 && s->db <= 200) {
			s->energizeBase(bases[0]);
		} else if (s->energy + s->size <= s->energyCapacity && dist(star, *s) <= 200)
			s->charge(star);
		else {
			while (tStartIt < sIt) {
				if ((*tStartIt)->db + 200 >= s->db)
					break;
				tStartIt++;
			}
			for (auto tIt = tStartIt; tIt < sIt; tIt++) {
				auto*& t = *tIt;
				float d = dist(*s, *t);
				if (d > 200 || d < 5)
					continue;
				if (t->energy + std::min(s->energy, s->size) > t->energyCapacity)
					continue;
				if (dist(P2A, *t) + dist(P2B, *t) <= 20 * (1 + travelTime + 1 + (t->energy + t->size - 1) / t->size)) // todo: think about this
					continue;
				s->energize(*t);

				// find swap to maintain energy order
				auto kIt = tIt + 2;
				for (; kIt < farmersEnd; kIt++)
					if ((*kIt)->energy >= t->energy || t->db + 5 <= (*kIt)->db)
						break;
				auto*& k = *--kIt;
				if (k->energy < t->energy && t->db + 5 > k->db)
					std::iter_swap(tIt, kIt);
				break;
			}
		}
	}

	if (count1 != 0) {
		auto f1it = available.begin();
		for (int i = 0; i < numChains; i++) {
			auto transferTo = i == 0 ? bases[0] : P1 + (numChains - i) * 199.9f * bP2BNorm;
			auto afkPoint = P1 + (numChains - i - 1) * 199.9f * bP2BNorm;
			for (; f1it < farmers1End - count1 * (numChains - i - 1) / numChains; f1it++) {
				auto* s = *f1it;
				
				if (!s->usedMove)
					s->move(dist(transferTo, *s) > 200 ? inDirection(transferTo, *s, 199.9f) : afkPoint);
			}
		}
	}

	for (auto it = farmersEnd; it-- != farmers1End; ) {
		auto* s = *it;
		float d2a = dist(*s, haulA);
		float d2b = dist(*s, P2B);

		if (!s->usedMove) {
			if (s->energy == 0 || (s->energy < s->energyCapacity && (dist(star, *s) <= 200 || d2a > s->energy * 20))) {
				s->move(dist(star, *s) > 200 && s->energy > 0 ? inDirection(star, *s, 199.9f) : P2B);
			} else {
				bool tempf1 = false;
				if (s->energy != 0 && s->db <= 200 && farmers1End == available.begin()) {
					tempf1 = true;
					for (auto j = available.begin(); j < farmers1End; j++)
						if (dist(*s, **j) <= 200) {
							tempf1 = false;
							break;
						}
				}
				if (tempf1)
					s->move(inDirection(bases[0], star, 199.9f));
				else
					s->move(dist(*s, targetA) > 200 && s->energy > 0 ? inDirection(targetA, *s, 199.9f) : haulA);
			}
		}
	}
	
	available.erase(available.begin(), farmersEnd);
	// println("loss1: %i loss2: %i", loss1, loss2);
}




void farm() {
	std::sort(available.begin(), available.end(), [&](auto*& a, auto*& b) {
		return dist(stars[0], *a) - dist(stars[1], *a) - a->db < dist(stars[0], *b) - dist(stars[1], *b) - b->db;
	});
	farmStar(available, stars[0], stars[1].activatesIn <= 0);
	
	std::sort(available.begin(), available.end(), [&](auto*& a, auto*& b) {
		return dist(stars[1], *a) - a->db < dist(stars[1], *b) - b->db;
	});
	farmStar(available, stars[1], false);

	// farmStar(available, stars[2], false);
}