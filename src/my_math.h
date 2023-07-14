#ifndef MY_MATH_H_
#define MY_MATH_H_

// @todo: get rid of math.h
#include <math.h>

typedef struct Tag_Vec2 {
	float x;
	float y;
} Vec2;

typedef struct Tag_Vec3 {
	float x;
	float y;
	float z;
} Vec3;

typedef struct Tag_Vec4 {
	float x;
	float y;
	float z;
	float w;
} Vec4;

typedef struct Tag_Mat4 {
	float e[4][4];
} Mat4;

inline Vec2 vec2_add(Vec2 v1, Vec2 v2) {
	Vec2 result;
	result.x = v1.x + v2.x;
	result.y = v1.y + v2.y;
	return result;
}

inline Vec2 vec2_sub(Vec2 v1, Vec2 v2) {
	Vec2 result;
	result.x = v1.x - v2.x;
	result.y = v1.y - v2.y;
	return result;
}

inline Vec2 vec2_negate(Vec2 v) {
	Vec2 result;
	result.x = -v.x;
	result.y = -v.y;
	return result;
}

inline Vec2 vec2_scale(float s, Vec2 v) {
	Vec2 result;
	result.x = s * v.x;
	result.y = s * v.y;
	return result;
}

inline Vec2 vec2_make(float x, float y) {
	Vec2 result;
	result.x = x;
	result.y = y;
	return result;
}

Vec2 vec2_normalize(Vec2 v) {
	Vec2 result = {0};
	if (v.x != 0 || v.y != 0) {
		float length = sqrtf(v.x * v.x + v.y * v.y);
		result.x = v.x / length;
		result.y = v.y / length;
	}
	return result;
}

inline Vec3 vec3_add(Vec3 v1, Vec3 v2) {
	Vec3 result;
	result.x = v1.x + v2.x;
	result.y = v1.y + v2.y;
	result.z = v1.z + v2.z;
	return result;
}

inline Vec3 vec3_sub(Vec3 v1, Vec3 v2) {
	Vec3 result;
	result.x = v1.x - v2.x;
	result.y = v1.y - v2.y;
	result.z = v1.z - v2.z;
	return result;
}

inline Vec3 vec3_negate(Vec3 v) {
	Vec3 result;
	result.x = -v.x;
	result.y = -v.y;
	result.z = -v.z;
	return result;
}

inline Vec3 vec3_scale(float s, Vec3 v) {
	Vec3 result;
	result.x = s * v.x;
	result.y = s * v.y;
	result.z = s * v.z;
	return result;
}

inline Vec3 vec3_make(float x, float y, float z) {
	Vec3 result;
	result.x = x;
	result.y = y;
	result.z = z;
	return result;
}

Vec3 vec3_normalize(Vec3 v) {
	Vec3 result = {0};
	if (v.x != 0 || v.y != 0 || v.z != 0) {
		float length = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
		result.x = v.x / length;
		result.y = v.y / length;
		result.z = v.z / length;
	}
	return result;
}

inline Mat4	mat4_mul(Mat4 a, Mat4 b) {
	Mat4 result;
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			for (int k = 0; k < 4; ++k) {
				result.e[i][j] += a.e[i][k] * b.e[k][j];
			}
		}
	}
	return result;
}

#endif 
