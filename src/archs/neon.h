#pragma once
#include <immintrin.h>

typedef simd_t __m128i;

#define simd_width_epi8  16
#define simd_width_epi16 8
#define simd_width_epi32 4

#define simd_add_epi16 _mm256_add_epi16
#define simd_sub_epi16 _mm256_sub_epi16