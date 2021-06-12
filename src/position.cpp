
#include "position.h"

#include <math.h>



Position operator+(const Position& u, const Position& v) {
	return { u.x + v.x, u.y + v.y };
}
Position operator-(const Position& u, const Position& v) {
	return { u.x - v.x, u.y - v.y };
}
Position operator*(const Position& u, float a) {
	return { u.x * a, u.y * a };
}
Position operator/(const Position& u, float a) {
	return { u.x / a, u.y / a };
}
float dot(const Position& u, const Position& v) {
	return u.x * v.x + u.y * v.y;
}
float norm(const Position& u) {
	return sqrtf(dot(u, u));
}
Position normalize(const Position& u) {
	return u / norm(u);
}
float dist(const Position& u, const Position& v) {
	return norm(u - v);
}
Position inDirection(const Position& u, const Position& v, float d) {
	return u + normalize(v - u) * d;
}