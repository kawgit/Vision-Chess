#pragma once

#include "bits.h"
#include "types.h"
#include <vector>
#include <string>
#include <map>

using namespace std;

void add_tunable(float* target, float mn, float mx, string id);

void add_tunables(vector<float>& targets, float mn, float mx, string section_id, int from = 0, int to = -1);

struct Tunable {
    Tunable(float* _target, float _lb, float _ub);
    void apply();
    inline float value() { return weight * range + lb; };

    float* target;
    float weight;
    float lb;
    float range;
};

class Configuration {
    public:

    Configuration();
    Configuration(Configuration* parent);

    void apply();
    void mutate(float radius, float mutation_chance);
    void save(string path);
    void load(string path);
    void print();

    
    private:
    vector<Tunable> tunables;
};
