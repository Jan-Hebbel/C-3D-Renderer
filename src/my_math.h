#ifndef MY_MATH_H_
#define MY_MATH_H_

// @todo: get rid of math.h
#include <math.h>

typedef struct Tag_Vec2 {
	float x;
	float y;
} Vec2;

typedef struct {
    int x;
    int y;
} Vec2I;

typedef struct {
    int x;
    int y;
    int z;
} Vec3I;

typedef struct Tag_Vec3 {
	float x;
	float y;
	float z;
} Vec3;

typedef union Tag_Vec4 {
    struct {
        float x;
        float y;
        float z;
        float w;
    } value;
    float e[4];
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

inline Vec2I vec2i_sub(Vec2I v1, Vec2I v2) {
    Vec2I result;
    result.x = v1.x - v2.x;
    result.y = v1.y - v2.y;
    return result;
}

inline Vec3I vec3i_sub(Vec3I v1, Vec3I v2) {
    Vec3I result;
    result.x = v1.x - v2.x;
    result.y = v1.y - v2.y;
    result.z = v1.z - v2.z;
    return result;
}

inline Vec3I vec3i_make(int x, int y, int z) {
    Vec3I result = { x, y, z };
    return result;
}

Vec3I icross(Vec3I v1, Vec3I v2) {
    Vec3I result;
    result.x = v1.y * v2.z - v1.z * v2.y;
    result.y = v1.z * v2.x - v1.x * v2.z;
    result.z = v1.x * v2.y - v1.y * v2.x;
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

inline Vec4 vec4_make(float x, float y, float z, float w) {
	Vec4 result;
	result.value.x = x;
	result.value.y = y;
	result.value.z = z;
	result.value.w = w;
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

inline Mat4 mat4_identity() {
	Mat4 result = {0.0f};
	result.e[0][0] = 1.0f;
	result.e[1][1] = 1.0f;
	result.e[2][2] = 1.0f;
	result.e[3][3] = 1.0f;
	return result;
}

inline Mat4 ortho_projection(float left, float right, float bottom, float top, float near, float far) {
	Mat4 orth = {0};
	orth.e[0][0] = 2.0f / (right - left);
	orth.e[1][1] = 2.0f / (top - bottom);
	orth.e[2][2] = 1.0f / (far - near);
	orth.e[3][0] = -(right + left) / (right - left);
	orth.e[3][1] = -(top + bottom) / (top - bottom);
	orth.e[3][2] = -near / (far - near);
	orth.e[3][3] = 1.0f;
	return orth;
}

inline Vec4 mat4_vec4_mul(Mat4 m, Vec4 v) {
	Vec4 result = {0};
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			result.e[j] += m.e[i][j] * v.e[j];
		}
	}
	return result;
}

Mat4 translate(Vec3 pos) {
    Mat4 result = mat4_identity();
    result.e[0][3] = pos.x;
    result.e[1][3] = pos.y;
    result.e[2][3] = pos.z;
    return result;
}

#endif 
