#ifndef MISC_H
#define MISC_H

#include "stdint.h"

//
// useful macros
//
#define SIZE(x) sizeof(x) / sizeof(x[0])

//
// types
// 
typedef int8_t b8;

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float  f32;
typedef double f64;

//
// other defines
//
#define M_TRUE  1
#define M_FALSE 0
#define SUCCESS 0
#define FAILURE 1

#define TAU 6.28318530718f

#endif