/*

#include <cassert>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include "types.h"
#include "distribution.h"

Distribution::Distribution(float u_, float o_, float lb_, float ub_) : u(u_), o(o_), lb(lb_), ub(ub_) {

    const float pi = 3.14159265358979;
    
    const float denominator = sqrt(2 * pi) * o;

    for (size_t i = 0; i < Distribution::resolution; i++) {

        const float x = i_to_x(i);
        const float y = exp(-.5 * pow((x - u) / o, 2)) / denominator;

        values[i] = y;
    }
}

void print(const Distribution& distribution) {
    const size_t horizontal_resolution = 32;
    const size_t vertical_resolution   = 20;
    
    float col_values[horizontal_resolution];
    float max_value = 0;

    for (size_t col = 0; col < horizontal_resolution; col++) {
        
        size_t lb = size_t(float(col    ) / horizontal_resolution * Distribution::resolution);
        size_t ub = size_t(float(col + 1) / horizontal_resolution * Distribution::resolution);

        assert(lb != ub);
        
        col_values[col] = 0;
        for (size_t i = lb; i < ub; i++) {
            col_values[col] += distribution.values[i];
        }
        col_values[col] /= ub - lb;

        if (col_values[col] > max_value)
            max_value = col_values[col];

    }

    std::cout << "            ";
    for (size_t col = 0; col < 3 * horizontal_resolution + 4; col++)
        std::cout << "-";

    std::cout << "\n";

    for (size_t row = 0; row <= vertical_resolution; row++) {

        int height = vertical_resolution - row;
        float y = float(vertical_resolution - row) / vertical_resolution * max_value;
        
        std::cout << std::setw(11) << y << " | ";

        for (size_t col = 0; col < horizontal_resolution; col++) {
            
            bool under = height <= int(col_values[col] / max_value * vertical_resolution + .5);
            std::cout << (under ? " # " : " - ");

        }

        std::cout << " |" << std::endl;

    }

    std::cout << "            ";
    for (size_t col = 0; col < 3 * horizontal_resolution + 4; col++)
        std::cout << "-";
    
    std::cout << std::endl << "  " << std::setw(11) << distribution.lb;

    for (size_t col = 0; col < 3 * horizontal_resolution - 7; col++)
        std::cout << " ";

    std::cout << std::setw(10) << distribution.ub << std::endl;



    // for (size_t i = 0; i < Distribution::resolution; i++)
	// 	std::cout << int(i) << " " << distribution.values[i] << std::endl;
}

void differentiate(const Distribution* src, Distribution* dst) {

    assert(src != dst);

    const float reciprocal_stepsize = 1.0f / src->stepsize();

    const __m256 v_reciprocal_stepsize = _mm256_set1_ps(reciprocal_stepsize);

    constexpr size_t vwidth = (sizeof(src->values) * 8) / 256;
    for (size_t i = 0; i < vwidth; i++) {

        __m256 vsrc = _mm256_load_ps(src->values + i * 8);

        __m256 shifted = reinterpret_cast<__m256>(_mm256_srli_si256(reinterpret_cast<__m256i>(vsrc), 4));
        __m256  result = _mm256_mul_ps(_mm256_sub_ps(shifted, vsrc), v_reciprocal_stepsize);

        _mm256_store_ps(dst->values + i * 8, result);

    }

    for (size_t i = 0; i < vwidth * 2 - 1; i++)
        dst->values[i * 4 + 3] = (src->values[i * 4 + 4] - src->values[i * 4 + 3]) * reciprocal_stepsize;

    dst->values[Distribution::resolution - 1] = 2 * (dst->values[Distribution::resolution - 2]) - dst->values[Distribution::resolution - 3];

}

void integrate(const Distribution* src, Distribution* dst) {

    float cum_total = 0;
    float stepsize = src->stepsize();

    for (size_t i = 0; i < Distribution::resolution; i++) {
        
        dst->values[i] = cum_total;
        cum_total += src->values[i] * stepsize;

    }

    // dst->values[Distribution::resolution - 1] = 1;

}

void multiply(const Distribution* src1, const Distribution* src2, Distribution* dst) {

    constexpr size_t vwidth = (sizeof(src1->values) * 8) / 256;
    
    for (size_t i = 0; i < vwidth; i++) {

        __m256 vsrc1 = _mm256_load_ps(src1->values + i * 8);
        __m256 vsrc2 = _mm256_load_ps(src2->values + i * 8);

        __m256 vdst = _mm256_mul_ps(vsrc1, vsrc2);
        
        _mm256_store_ps(dst->values + i * 8, vdst);

    }

}

constexpr std::array<float, Distribution::resolution> ASCENDING_NUMS = []() constexpr {

    std::array<float, Distribution::resolution> array {};

    for (size_t i = 0; i < Distribution::resolution; i++)

		array[i] = float(i);

    return array;

}();

float mean_of(const Distribution* dist) {
    __m256* van  = (__m256*) ASCENDING_NUMS.data();
    __m256* vsrc = (__m256*) dist->values;

    float acc[8];

    __m256 vacc = _mm256_mul_ps(*vsrc, *van);

    size_t vwidth = (sizeof(dist->values) * 8) / 256;
    
    for (size_t i = 0; i < vwidth - 1; i++)
        vacc = _mm256_fmadd_ps(*(vsrc + i), *(van + i), vacc);

    _mm256_store_ps(acc, vacc);

    float mean = 0;

    for (size_t i = 0; i < 8; i++)
        mean += acc[i];

    mean *= dist->stepsize();
    mean /= dist->resolution;
    mean += dist->lb;

    return mean;

}


*/