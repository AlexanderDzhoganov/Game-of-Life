#include <cmath>
#include "vec2.h"

// vec2 member functions

vec2::vec2() {
	xy[0] = 0.f;
	xy[1] = 0.f;
	lengthCached = false;
}

vec2::vec2(float x, float y) {
	xy[0] = x;
	xy[1] = y;
	lengthCached = false;
}

void vec2::set(float x, float y) {
	xy[0] = x;
	xy[1] = y;
	lengthCached = false;
}

float& vec2::operator[](const int index) {
	lengthCached = false;
	return xy[index];
}

const float& vec2::operator[](const int index) const {
	return xy[index];
}

void vec2::negate() {
	lengthCached = false;
	xy[0] *= -1.f;
	xy[1] *= -1.f;
}

vec2& vec2::operator=(const vec2& v) {
	if(this == &v) return *this;
	lengthCached = false;
	xy[0] = v[0];
	xy[1] = v[1];
	return *this;
}
bool vec2::operator==(const vec2& v) {
	if(xy[0] == v[0] && xy[1] == v[1]) return true;
	return false;
}

bool vec2::operator!=(const vec2& v) {
	return !(*this == v);
}

vec2& vec2::operator+=(const vec2& v) {
	lengthCached = false;
	xy[0] += v[0];
	xy[1] += v[1];
	return *this;
}

vec2& vec2::operator-=(const vec2& v) {
	lengthCached = false;
	vec2 tmp(v);
	tmp.negate();
	*this += tmp;
	return *this;
}

vec2& vec2::operator*=(const float& x) {
	lengthCached = false;
	xy[0] *= x;
	xy[1] *= x;
	return *this;
}

vec2 vec2::operator+(const vec2& v) {
	return vec2(*this) += v;
}

vec2 vec2::operator-(const vec2& v) {
	return vec2(*this) -= v;
}

vec2 vec2::operator*(const float& x) {
	return vec2(*this) *= x;
}

float vec2::dot_product(const vec2& v) {
	return (xy[0] * v[0] + xy[1] * v[1]);
}

float vec2::length() {
	if(!lengthCached) {
		C_length2 = xy[0] * xy[0] + xy[1] * xy[1];
		C_length = std::sqrt(C_length2);
		lengthCached = true;
	}
	return C_length;
}

float vec2::length2() {
	if(!lengthCached) {
		C_length2 = xy[0] * xy[0] + xy[1] * xy[1];
	}
	return C_length2;
}

void vec2::normalize() {
	float len = this->length();
	xy[0] /= len;
	xy[1] /= len;
	C_length = C_length2 = 1.f;
}
