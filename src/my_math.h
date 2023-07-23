#ifndef MY_MATH_H_
#define MY_MATH_H_

#include <math.h> // only for sqrtf @todo ?!

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

#define PI 3.14159265359f
#define TABLE_SIZE 257
#define STEP_SIZE 0.25f * 2 * PI / (TABLE_SIZE - 1)

float table[TABLE_SIZE] = {
    0.0000000f, 0.0061359f, 0.0122715f, 0.0184067f, 0.0245412f, 0.0306748f, 
    0.0368072f, 0.0429383f, 0.0490677f, 0.0551952f, 0.0613207f, 0.0674439f, 
    0.0735646f, 0.0796824f, 0.0857973f, 0.0919090f, 0.0980171f, 0.1041216f, 
    0.1102222f, 0.1163186f, 0.1224107f, 0.1284981f, 0.1345807f, 0.1406582f, 
    0.1467305f, 0.1527972f, 0.1588582f, 0.1649131f, 0.1709619f, 0.1770042f, 
    0.1830399f, 0.1890687f, 0.1950903f, 0.2011046f, 0.2071114f, 0.2131103f, 
    0.2191012f, 0.2250839f, 0.2310581f, 0.2370236f, 0.2429802f, 0.2489276f, 
    0.2548656f, 0.2607941f, 0.2667128f, 0.2726214f, 0.2785197f, 0.2844076f, 
    0.2902847f, 0.2961509f, 0.3020059f, 0.3078496f, 0.3136818f, 0.3195020f, 
    0.3253103f, 0.3311063f, 0.3368898f, 0.3426607f, 0.3484187f, 0.3541635f, 
    0.3598951f, 0.3656130f, 0.3713172f, 0.3770074f, 0.3826835f, 0.3883450f, 
    0.3939920f, 0.3996242f, 0.4052413f, 0.4108432f, 0.4164295f, 0.4220003f, 
    0.4275551f, 0.4330938f, 0.4386162f, 0.4441221f, 0.4496113f, 0.4550836f, 
    0.4605387f, 0.4659765f, 0.4713967f, 0.4767992f, 0.4821838f, 0.4875502f, 
    0.4928982f, 0.4982277f, 0.5035384f, 0.5088302f, 0.5141027f, 0.5193560f, 
    0.5245897f, 0.5298036f, 0.5349976f, 0.5401715f, 0.5453250f, 0.5504580f, 
    0.5555702f, 0.5606616f, 0.5657318f, 0.5707808f, 0.5758082f, 0.5808140f, 
    0.5857978f, 0.5907597f, 0.5956993f, 0.6006165f, 0.6055110f, 0.6103828f, 
    0.6152316f, 0.6200572f, 0.6248595f, 0.6296383f, 0.6343933f, 0.6391245f, 
    0.6438316f, 0.6485144f, 0.6531729f, 0.6578067f, 0.6624158f, 0.6669999f, 
    0.6715590f, 0.6760927f, 0.6806010f, 0.6850837f, 0.6895406f, 0.6939715f, 
    0.6983762f, 0.7027547f, 0.7071068f, 0.7114322f, 0.7157308f, 0.7200025f, 
    0.7242470f, 0.7284644f, 0.7326543f, 0.7368166f, 0.7409511f, 0.7450578f, 
    0.7491364f, 0.7531868f, 0.7572088f, 0.7612024f, 0.7651673f, 0.7691033f, 
    0.7730104f, 0.7768885f, 0.7807372f, 0.7845566f, 0.7883464f, 0.7921066f, 
    0.7958369f, 0.7995373f, 0.8032075f, 0.8068475f, 0.8104572f, 0.8140363f, 
    0.8175848f, 0.8211025f, 0.8245893f, 0.8280451f, 0.8314696f, 0.8348629f, 
    0.8382247f, 0.8415549f, 0.8448536f, 0.8481203f, 0.8513552f, 0.8545580f, 
    0.8577286f, 0.8608669f, 0.8639728f, 0.8670462f, 0.8700870f, 0.8730950f, 
    0.8760701f, 0.8790122f, 0.8819212f, 0.8847971f, 0.8876396f, 0.8904487f, 
    0.8932243f, 0.8959663f, 0.8986744f, 0.9013488f, 0.9039893f, 0.9065957f, 
    0.9091680f, 0.9117060f, 0.9142098f, 0.9166790f, 0.9191138f, 0.9215140f, 
    0.9238795f, 0.9262102f, 0.9285061f, 0.9307670f, 0.9329928f, 0.9351835f, 
    0.9373390f, 0.9394592f, 0.9415441f, 0.9435934f, 0.9456074f, 0.9475856f, 
    0.9495282f, 0.9514350f, 0.9533060f, 0.9551412f, 0.9569404f, 0.9587035f, 
    0.9604305f, 0.9621214f, 0.9637761f, 0.9653944f, 0.9669765f, 0.9685221f, 
    0.9700313f, 0.9715039f, 0.9729400f, 0.9743394f, 0.9757021f, 0.9770281f, 
    0.9783174f, 0.9795697f, 0.9807853f, 0.9819639f, 0.9831055f, 0.9842101f, 
    0.9852777f, 0.9863081f, 0.9873014f, 0.9882576f, 0.9891765f, 0.9900582f, 
    0.9909027f, 0.9917098f, 0.9924796f, 0.9932119f, 0.9939070f, 0.9945646f, 
    0.9951847f, 0.9957674f, 0.9963126f, 0.9968203f, 0.9972904f, 0.9977230f, 
    0.9981181f, 0.9984756f, 0.9987954f, 0.9990777f, 0.9993224f, 0.9995294f, 
    0.9996988f, 0.9998306f, 0.9999247f, 0.9999812f, 1.0000000f
};

// @todo
/* float lerp(float v0, float v1, float x) { */
/*     return 0.0f; */
/* } */

float m_sin(float turn) {
    float normalized_turn = turn - (int)turn;
    if (normalized_turn < 0.0f) normalized_turn = 1.0f + normalized_turn;

    int mirror = 0;
    int flip = 0;
    
    float index;
    if (normalized_turn >= 0.0f && normalized_turn < 0.25f) {
        index = normalized_turn * 4.0f * (TABLE_SIZE - 1);
    }
    else if (normalized_turn >= 0.25f && normalized_turn < 0.5f) {
        index = (normalized_turn - 0.25f) * 4.0f * (TABLE_SIZE - 1);
        mirror = 1;
    }
    else if (normalized_turn >= 0.5f && normalized_turn < 0.75f) {
        index = (normalized_turn - 0.5f) * 4.0f * (TABLE_SIZE - 1);
        flip = 1;
    }
    else {
        index = (normalized_turn - 0.75f) * 4.0f * (TABLE_SIZE - 1);
        mirror = 1;
        flip = 1;
    }
    if (mirror) {
        index = (TABLE_SIZE - 1) - index;
    }
    int index0 = (int)index;
    int index1 = index0 + 1;
    
    float lerp = table[index0] + (((table[index1] - table[index0]) / STEP_SIZE) *
                                  ((index - index0) * STEP_SIZE));
    
    if (flip) {
        return -lerp;
    }
    else {
        return lerp;
    }
}

float m_cos(float turn) {
    float normalized_turn = turn - (int)turn;
    if (normalized_turn < 0.0f) normalized_turn = 1.0f + normalized_turn;

    int mirror = 0;
    int flip = 0;
    
    float index;
    if (normalized_turn >= 0.0f && normalized_turn < 0.25f) {
        index = normalized_turn * 4.0f * (TABLE_SIZE - 1);
        mirror = 1;
    }
    else if (normalized_turn >= 0.25f && normalized_turn < 0.5f) {
        index = (normalized_turn - 0.25f) * 4.0f * (TABLE_SIZE - 1);
        flip = 1;
    }
    else if (normalized_turn >= 0.5f && normalized_turn < 0.75f) {
        index = (normalized_turn - 0.5f) * 4.0f * (TABLE_SIZE - 1);
        mirror = 1;
        flip = 1;
    }
    else {
        index = (normalized_turn - 0.75f) * 4.0f * (TABLE_SIZE - 1);
    }
    if (mirror) {
        index = (TABLE_SIZE - 1) - index;
    }
    int index0 = (int)index;
    int index1 = index0 + 1;
    
    float lerp = table[index0] + (((table[index1] - table[index0]) / STEP_SIZE) *
                                  ((index - index0) * STEP_SIZE));
    
    if (flip) {
        return -lerp;
    }
    else {
        return lerp;
    }    
}

float m_tan(float turn) {
    return m_sin(turn) / m_cos(turn);
}

float m_cotan(float turn) {
    return m_cos(turn) / m_sin(turn);
}

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

Vec3I vec3i_cross(Vec3I v1, Vec3I v2) {
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

Vec3 vec3_cross(Vec3 v1, Vec3 v2) {
    Vec3 result;
    result.x = v1.y * v2.z - v1.z * v2.y;
    result.y = v1.z * v2.x - v1.x * v2.z;
    result.z = v1.x * v2.y - v1.y * v2.x;
    return result;
}

float vec3_dot(Vec3 v1, Vec3 v2) {
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
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
            result.e[i][j] = 0.0f;
			for (int k = 0; k < 4; ++k) {
				result.e[i][j] += a.e[i][k] * b.e[k][j];
			}
		}
	}
	return result;
}

Mat4 mat4_mul3(Mat4 a, Mat4 b, Mat4 c) {
    return mat4_mul(mat4_mul(a, b), c);
}

inline Mat4 mat4_identity(void) {
	Mat4 result = {0.0f};
	result.e[0][0] = 1.0f;
	result.e[1][1] = 1.0f;
	result.e[2][2] = 1.0f;
	result.e[3][3] = 1.0f;
	return result;
}

Vec4 mat4_vec4_mul(Mat4 m, Vec4 v) {
	Vec4 result = {0};
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			result.e[i] += m.e[i][j] * v.e[j];
		}
	}
	return result;
}

Mat4 transpose(Mat4 m) {
    Mat4 result;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            result.e[i][j] = m.e[j][i];
        }
    }
    return result;
}

Mat4 translate(float x, float y, float z) {
    Mat4 result = mat4_identity();
    result.e[0][3] = x;
    result.e[1][3] = y;
    result.e[2][3] = z;
    return result;
}

Mat4 rotate_x(float turn) {
    Mat4 result = mat4_identity();
    result.e[1][1] =  m_cos(turn);
    result.e[1][2] = -m_sin(turn);
    result.e[2][1] =  m_sin(turn);
    result.e[2][2] =  m_cos(turn);
    return result;
}

Mat4 rotate_y(float turn) {
    Mat4 result = mat4_identity();
    result.e[0][0] =  m_cos(turn);
    result.e[0][2] =  m_sin(turn);
    result.e[2][0] = -m_sin(turn);
    result.e[2][2] =  m_cos(turn);
    return result;
}

Mat4 rotate_z(float turn) {
    Mat4 result = mat4_identity();
    result.e[0][0] =  m_cos(turn);
    result.e[0][1] = -m_sin(turn);
    result.e[1][0] =  m_sin(turn);
    result.e[1][1] =  m_cos(turn);
    return result;
}

Mat4 LookAt(float position_x, float position_y, float position_z,
            float target_x, float target_y, float target_z,
            float fake_up_x, float fake_up_y, float fake_up_z) {
    Vec3 position = { position_x, position_y, position_z };
    Vec3 target = { target_x, target_y, target_z };
    Vec3 fake_up = { fake_up_x, fake_up_y, fake_up_z };

    // calculating forward direction of camera and negating it, so that the
    // camera points toward negative z
    Vec3 backward = vec3_normalize(vec3_sub(position, target));
    Vec3 right = vec3_normalize(vec3_cross(fake_up, backward));
    Vec3 up = vec3_cross(backward, right);

    Mat4 result = { right.x,    right.y,    right.z,    -vec3_dot(right, position),
                    up.x,       up.y,       up.z,       -vec3_dot(up, position),
                    backward.x, backward.y, backward.z, -vec3_dot(backward, position),
                    0.0f,       0.0f,       0.0f,       1.0f };
    return result;
}

Mat4 ortho_projection(float left, float right,
                      float bottom, float top,
                      float near, float far) {
    float width = right - left;
    float height = top - bottom;
    float depth = far - near;

    Mat4 orth = { 2.0f / width, 0.0f,          0.0f,          -(right + left) / width,
                  0.0f,         2.0f / height, 0.0f,          -(top + bottom) / height,
                  0.0f,         0.0f,          -2.0f / depth, -(far + near) / depth,
                  0.0f,         0.0f,          0.0f,          1.0f };
	return orth;
}

Mat4 perspective_projection(float fov, float aspect, float near, float far) {
    /* float f = m_tan(0.5f * fov); */
    /* float inverse_range = 1.0f / (near - far); */
    float f = m_cotan(0.5f * fov);
    float inv_depth = 1.0f / (near - far);
    
    Mat4 result = { f / aspect, 0.0f, 0.0f,                      0.0f,
                    0.0f,       f,    0.0f,                      0.0f,
                    0.0f,       0.0f, (far + near) * inv_depth,  (2.0f * far * near) * inv_depth,
                    0.0f,       0.0f, -1.0f,                     0.0f };
    return result;
}

#endif 
