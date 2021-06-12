#pragma once

struct Position {
	float x, y;
};


Position operator+(const Position& u, const Position& v);
Position operator-(const Position& u, const Position& v);
Position operator*(const Position& u, float a);
Position operator/(const Position& u, float a);
float dot(const Position& u, const Position& v);
float norm(const Position& u);
Position normalize(const Position& u);
float dist(const Position& u, const Position& v);
Position inDirection(const Position& u, const Position& v, float d);
