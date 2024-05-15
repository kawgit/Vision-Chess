/*

#pragma once

#include <immintrin.h>
#include <vector>
#include <optional>
#include <memory>

#include "types.h"

#define ALIGN64 alignas(64)

struct Distribution {

    static const size_t resolution = 1024;

    float u;
    float o;
    float lb;
    float ub;

    ALIGN64 float values[resolution];

    Distribution (float u_, float o_, float lb_, float ub_);

    inline float range() const {
        return ub - lb;
    }

    inline float stepsize() const {
        return range() / resolution;
    }

    inline float i_to_x(const size_t i) const {
        return (float(i) + .5) / Distribution::resolution * range() + lb;
    }

    inline size_t x_to_i(const float x) const {
        return size_t((x - lb) / range() * Distribution::resolution);
    }

};

void print(const Distribution& distribution);

void differentiate(const Distribution* src, Distribution* dst);

void integrate(const Distribution* src, Distribution* dst);

void multiply(const Distribution* src1, const Distribution* src2, Distribution* dst);

float mean_of(const Distribution* dist);
*/