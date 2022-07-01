
#include "objects.h"
#include <vector>
#include <math.h>

#include "safeMove.h"


// considers all friendly and foe spirits and structures, their attack ranges and movement ranges.
// performs 2 sweeps:
// angular sweep over all circle intersection points of interest: https://www.geeksforgeeks.org/angular-sweep-maximum-points-can-enclosed-circle-given-radius/
// linear sweep in the direction of target, with all line-circle intersection points of interest


constexpr float F_EPS = std::numeric_limits<float>::epsilon() * 100;

enum SweepPointType {
	Add,
	Subtract,
	Target,
};
struct AngleSweepPoint {
	float angle;
	float strength;
	SweepPointType type;
};
struct LineSweepPoint {
	float distance; // 40 to 0, 40 being furthest away from target (lineStart)
	float strength;
	SweepPointType type;
};
std::vector<AngleSweepPoint> anglePoints;
std::vector<LineSweepPoint> linePoints;

template<bool isFriendly>
void addAngleSweepPoints(float centerAngle, float offsetAngle, float strength) {
	float angle1 = centerAngle - offsetAngle;
	float angle2 = isFriendly ? centerAngle + offsetAngle : centerAngle + offsetAngle - 2 * M_PI;
	anglePoints.emplace_back(AngleSweepPoint{
		.angle = angle1 + (isFriendly ? F_EPS : -F_EPS),
		.strength = strength,
		.type = isFriendly ? SweepPointType::Add : SweepPointType::Subtract,
	});
	anglePoints.emplace_back(AngleSweepPoint{
		.angle = angle2 + (isFriendly ? -F_EPS : F_EPS),
		.strength = strength,
		.type = isFriendly ? SweepPointType::Subtract : SweepPointType::Add,
	});
}

template<bool isFriendly>
void addLineSweepPoints(float centerDist, float offsetDist, float strength) {
	float dist1 = centerDist - offsetDist;
	float dist2 = centerDist + offsetDist;
	if (dist1 > 0 && dist1 < 40)
		linePoints.emplace_back(LineSweepPoint{
			.distance = dist1 + (isFriendly ? F_EPS : -F_EPS),
			.strength = strength,
			.type = isFriendly ? SweepPointType::Add : SweepPointType::Subtract,
		});
	if (dist2 > 0 && dist2 < 40)
		linePoints.emplace_back(LineSweepPoint{
			.distance = dist2 + (isFriendly ? -F_EPS : F_EPS),
			.strength = strength,
			.type = isFriendly ? SweepPointType::Subtract : SweepPointType::Add,
		});
}

void safeMoveReserve(int mySpiritCount, int enemySpiritCount, int outpostCount) {
	anglePoints.reserve(1 + mySpiritCount * 2 + enemySpiritCount * 2 + outpostCount);
	linePoints.reserve(1 + mySpiritCount * 2 + enemySpiritCount * 2 + outpostCount);
}

void MySpirit::safeMove(const Position& targetPosition) {

	static constexpr float MAX_MOVE_DIST = 20;
	static constexpr float TARGET_WEIGHT = -0.1f / (2*MAX_MOVE_DIST);

	float targetAngle = atan2(targetPosition - *this);
	
	auto lineStart = inDirection(*this, targetPosition, -MAX_MOVE_DIST);
	auto lineEnd = inDirection(*this, targetPosition, MAX_MOVE_DIST);

	anglePoints.clear();
	anglePoints.emplace_back(AngleSweepPoint{
		.angle = targetAngle,
		.type = SweepPointType::Target,
	});
	linePoints.clear();
	linePoints.emplace_back(LineSweepPoint{
		.distance = std::max<float>(2 * MAX_MOVE_DIST + F_EPS, dist(lineStart, targetPosition)),
		.type = SweepPointType::Target,
	});
	
	// add all circle-circle and circle-line intersections to list
	float currentStrength = strength();
	for (auto it = available.begin(); it != available.end(); it++) {
		auto*& other = *it;
		float d = dist(*this, *other);

		if (this != other && d < 2*2*MAX_MOVE_DIST) {
			float strength = other->strength();
			if (d == 0) {
				currentStrength += strength;
				continue;
			}
			// angular sweep point
			float offsetAngle = acosf(d / (2*2*MAX_MOVE_DIST));
			float centerAngle = atan2(*other - *this);
			addAngleSweepPoints<true>(centerAngle, offsetAngle, strength);

			// linear sweep point
			auto projPoint = proj(*other - *this, targetPosition - *this);
			float projDist = dist(projPoint, lineEnd);
			if (projDist < 2*2*MAX_MOVE_DIST) {
				float lineDist = dist(*this, projPoint);
				float lineOffset = sqrt((2*2*MAX_MOVE_DIST)*(2*2*MAX_MOVE_DIST) - projDist*projDist);
				addLineSweepPoints<true>(lineDist, lineOffset, strength);
			}
		}
	}
	for (auto& other : enemies) {
		constexpr float attackDist = 200.1f + MAX_MOVE_DIST;
		float d = dist(*this, other);
		float strength = other.strength();
		if (d >= attackDist + MAX_MOVE_DIST)
			continue; // never in range
			
		currentStrength -= strength;

		if (d <= attackDist - MAX_MOVE_DIST) 
			continue; // always in range

		// angular sweep point
		float offsetAngle = acosf((d - attackDist - MAX_MOVE_DIST) / (2*MAX_MOVE_DIST));
		float centerAngle = atan2(other - *this);
		addAngleSweepPoints<false>(centerAngle, offsetAngle, strength);

		// linear sweep point
		auto projPoint = proj(other - *this, targetPosition - *this);
		float projDist = dist(projPoint, lineEnd);
		if (projDist < attackDist) {
			float lineDist = dist(*this, projPoint);
			float lineOffset = sqrt(attackDist*attackDist - projDist*projDist);
			addLineSweepPoints<false>(lineDist, lineOffset, strength);
		}
	}

	for (auto& outpost : outposts) {
		if (outpost.energy <= 0)
			continue;
		float attackDist = outpost.range + F_EPS;
		float d = dist(outpost, *this);

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

		// angular sweep point
		float offsetAngle = acosf((d - attackDist - MAX_MOVE_DIST) / (2*MAX_MOVE_DIST));
		float centerAngle = atan2(outpost - *this);
		if (outpost.isFriendly())
			addAngleSweepPoints<true>(centerAngle, offsetAngle, strength);
		else
			addAngleSweepPoints<false>(centerAngle, offsetAngle, strength);

		// linear sweep point
		auto projPoint = proj(outpost - *this, targetPosition - *this);
		float projDist = dist(projPoint, lineEnd);
		if (projDist < attackDist) {
			float lineDist = dist(*this, projPoint);
			float lineOffset = sqrt(attackDist*attackDist - projDist*projDist);
			if (outpost.isFriendly())
				addLineSweepPoints<true>(lineDist, lineOffset, strength);
			else
				addLineSweepPoints<false>(lineDist, lineOffset, strength);
		}
	}

	// sort lists
	std::sort(anglePoints.begin(), anglePoints.end(), [](auto& a, auto& b){
		return a.angle < b.angle;
	});
	std::sort(linePoints.begin(), linePoints.end(), [](auto& a, auto& b){
		return a.distance < b.distance;
	});

	// sweep over all angle intersections
	float strengthTowardsTarget;
	float bestScore = -std::numeric_limits<float>::infinity();
	std::vector<AngleSweepPoint>::iterator bestSweepPoint;
	for (auto it = anglePoints.begin(); it != anglePoints.end(); it++) {

		if (it->type == SweepPointType::Add)
			currentStrength += it->strength;

		auto nextPos = *this + MAX_MOVE_DIST * fromAngle(it->angle);
		// print("%f,%f ", it->angle, currentStrength);	
		float score = std::min(0.f, currentStrength) + TARGET_WEIGHT * dist(nextPos, targetPosition);
		if (score > bestScore) {
			bestScore = score;
			bestSweepPoint = it;
		}

		if (it->type == SweepPointType::Subtract)
			currentStrength -= it->strength;
		if (it->type == SweepPointType::Target) {
			strengthTowardsTarget = currentStrength;
		}
	}
	Position bestPosition = *this + (MAX_MOVE_DIST + F_EPS) * fromAngle(bestSweepPoint->angle);

	// sweep over all line intersections
	currentStrength = strengthTowardsTarget;
	for (auto it = linePoints.begin(); it != linePoints.end(); it++) {

		if (it->type == SweepPointType::Add)
			currentStrength += it->strength;

		auto nextPos = inDirection(lineStart, lineEnd, it->distance);
		float score = std::min(0.f, currentStrength) + TARGET_WEIGHT * dist(nextPos, targetPosition);
		if (score > bestScore) {
			bestScore = score;
			bestPosition = nextPos;
		}
		
		if (it->type == SweepPointType::Subtract)
			currentStrength -= it->strength;
	}
	
	move(bestPosition);
}