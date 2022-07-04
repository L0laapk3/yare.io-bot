
#include "objects.h"
#include <vector>
#include <math.h>

#include "safeMove.h"
#include "printf.h"


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
void addAngleSweepPoints(float centerAngle, float offsetAngle, float strength, float& startStrength) {
	// centerAngle: -PI to PI
	// offsetAngle: 0 to PI
	float angle1 = centerAngle - offsetAngle + (isFriendly ? F_EPS : -F_EPS);
	float angle2 = centerAngle + offsetAngle + (isFriendly ? -F_EPS : F_EPS);

	if (angle1 < -M_PI || angle2 > M_PI) {
		startStrength += (isFriendly ? strength : -strength);
		if (angle1 < -M_PI) // start is before finish
			angle1 += 2 * M_PI;
		if (angle2 > M_PI) // finish is before start
			angle2 -= 2 * M_PI;
	}
	anglePoints.emplace_back(AngleSweepPoint{
		.angle = angle1,
		.strength = strength,
		.type = isFriendly ? SweepPointType::Add : SweepPointType::Subtract,
	});
	anglePoints.emplace_back(AngleSweepPoint{
		.angle = angle2,
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

	float targetAngle = atan2(targetPosition - *this);
	
	auto lineStart = *this - moveSpeed * fromAngle(targetAngle);
	auto lineEnd = *this + moveSpeed * fromAngle(targetAngle);

	anglePoints.clear();
	anglePoints.emplace_back(AngleSweepPoint{
		.angle = targetAngle,
		.type = SweepPointType::Target,
	});
	linePoints.clear();
	linePoints.emplace_back(LineSweepPoint{
		.distance = std::min<float>(2 * moveSpeed + F_EPS, dist(lineStart, targetPosition)),
		.type = SweepPointType::Target,
	});
	
	// add all circle-circle and circle-line intersections to list
	float currentStrength = strength();
	for (auto it = available.begin(); it != available.end(); it++) {
		auto*& other = *it;
		float d = dist(*this, *other);

		if (this != other && d < 2*(moveSpeed + other->moveSpeed)) {
			float strength = other->strength();
			if (d == 0) {
				currentStrength += strength;
				continue;
			}
			// angular sweep point
			float offsetAngle = acosf(d / (2*2*moveSpeed));
			float centerAngle = atan2(*other - *this);
			addAngleSweepPoints<true>(centerAngle, offsetAngle, strength, currentStrength);

			// linear sweep point
			auto projPoint = proj(*other - *this, lineEnd - lineStart);
			float projDist = dist(projPoint, lineEnd);
			if (projDist < 2*(moveSpeed + other->moveSpeed)) {
				float lineDist = dist(*this, projPoint);
				float lineOffset = sqrt((2*(moveSpeed + other->moveSpeed))*(2*(moveSpeed + other->moveSpeed)) - projDist*projDist);
				addLineSweepPoints<true>(lineDist, lineOffset, strength);
			}
		}
	}
	for (auto& other : enemies) {
		const float attackRange = other.range + (other.maxRange - other.range) / (other.rangeGrowth - moveSpeed);
		const float attackDist = attackRange + other.moveSpeed + F_EPS;
		float d = dist(*this, other);
		float strength = other.strength();
		if (d >= attackDist + moveSpeed)
			continue; // never in range
			
		currentStrength -= strength;

		if (d <= attackDist - moveSpeed) 
			continue; // always in range

		// angular sweep point
		float offsetAngle = acosf((d - attackDist - moveSpeed) / (2*moveSpeed));
		float centerAngle = atan2(other - *this);
		addAngleSweepPoints<false>(centerAngle, offsetAngle, strength, currentStrength);

		// linear sweep point
		auto projPoint = proj(other - *this, lineEnd - lineStart);
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
		float attackDist = outpost.range - (outpost.isFriendly() ? range : 0) + F_EPS;
		float d = dist(outpost, *this);

		if (d >= attackDist + moveSpeed)
			continue; // never in range
		
		float strength = outpost.strength();
		if (!outpost.isFriendly())
			currentStrength -= strength;
			
		if (d <= attackDist - moveSpeed) {
			if (outpost.isFriendly())
				currentStrength += strength;
			continue; // always in range
		}

		// angular sweep point
		float offsetAngle = acosf((d - attackDist - moveSpeed) / (2*moveSpeed));
		float centerAngle = atan2(outpost - *this);
		if (outpost.isFriendly())
			addAngleSweepPoints<true>(centerAngle, offsetAngle, strength, currentStrength);
		else
			addAngleSweepPoints<false>(centerAngle, offsetAngle, strength, currentStrength);

		// linear sweep point
		auto projPoint = proj(outpost - *this, lineEnd - lineStart);
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

	static constexpr float TARGET_WEIGHT = -0.1f / (2*moveSpeed);
	auto scoreFn = [&](float strength, Position& pos) {
		return std::min(0.f, strength) + TARGET_WEIGHT * dist(pos, targetPosition);
	};

	// sweep over all angle intersections
	float strengthTowardsTarget;
	float bestScore = -std::numeric_limits<float>::infinity();
	std::vector<AngleSweepPoint>::iterator bestSweepPoint;
	for (auto it = anglePoints.begin(); it != anglePoints.end(); it++) {

		if (it->type == SweepPointType::Add)
			currentStrength += it->strength;

		auto nextPos = *this + moveSpeed * fromAngle(it->angle);
		float score = scoreFn(currentStrength, nextPos);
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
	Position bestPosition = *this + (moveSpeed + F_EPS) * fromAngle(bestSweepPoint->angle);

	// sweep over all line intersections
	currentStrength = strengthTowardsTarget;
	for (auto it = linePoints.begin(); it != linePoints.end(); it++) {

		if (it->type == SweepPointType::Add)
			currentStrength += it->strength;

		auto nextPos = inDirection(lineStart, lineEnd, it->distance);
		float score = scoreFn(currentStrength, nextPos);

		if (score > bestScore) {
			bestScore = score;
			bestPosition = nextPos;
		}
		
		if (it->type == SweepPointType::Subtract)
			currentStrength -= it->strength;
	}
	
	move(bestPosition);
}