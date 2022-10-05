#include "tuner.h"
#include "types.h"
#include "eval.h"
#include "search.h"
#include "pos.h"
#include "movegen.h"
#include <vector>
#include <cassert>
#include <cmath>
#include <fstream>
#include <iomanip>

using namespace std;

vector<string> list_of_tunable_ids;
vector<Tunable> list_of_tunables;

void add_tunable(float* target, float mn, float mx, string id) {
    list_of_tunables.emplace_back(target, mn, mx);
    list_of_tunable_ids.push_back(id);
}

void add_tunables(vector<float>& targets, float mn, float mx, string section_id, int from, int to) {
    for (int i = from; i < min(targets.size(), to == -1 ? targets.size() : to); i++) {
        add_tunable(&targets[i], mn, mx, section_id);
    }
}

Tunable::Tunable(float* _target, float _lb, float _ub) {
    target = _target;
    lb = _lb;
    range = _ub-_lb;
    weight = (*target - lb) / range;
}

void Tunable::apply() {
    *target = value();
}

Configuration::Configuration() {
    for (Tunable& tunable : list_of_tunables) {
        tunables.push_back(tunable);
    }
}

Configuration::Configuration(Configuration* parent) {
    assert(parent);

    for (Tunable& tunable : parent->tunables) {
        tunables.push_back(tunable);
    }
}
    
void Configuration::apply() {
    for (Tunable& tunable : tunables) {
        tunable.apply();
    }
}

void Configuration::mutate(float radius, float mutation_chance) {
    if (mutation_chance == -1) {
        Tunable& tunable = tunables[floor(randf(0, tunables.size()))];
        tunable.weight += randf(-radius, radius);
        tunable.weight = max((float)0, min(tunable.weight, (float)1));
    } 
    else {
        for (Tunable& tunable : tunables) {
            if (randf(0, 1) < mutation_chance) {
                tunable.weight += randf(-radius, radius);
                tunable.weight = max((float)0, min(tunable.weight, (float)1));
            }
        }
    }
}

void Configuration::save(string path) {
    ofstream fs(path, ios::binary);
    if (!fs) {
        cout << "Cannot open file " << path << endl;
        return;
    }

    for (int i = 0; i < list_of_tunables.size(); i++) {
        fs.write((char*)&tunables[i].weight, sizeof(float));
    }

    fs.close();
}

void Configuration::load(string path) {
    ifstream fs(path, ios::binary);
    if (!fs) {
        cout << "Cannot open file " << path << endl;
        return;
    }
    
    for (int i = 0; i < list_of_tunables.size(); i++) {
        assert(fs.read(reinterpret_cast<char*>(&tunables[i].weight), sizeof(float)));
    }

    fs.close();
}

void Configuration::print() {
    assert(tunables.size() == list_of_tunables.size());
    cout << endl;
    for (int i = 0; i < list_of_tunables.size(); i++) {
        cout << setw(80) << list_of_tunable_ids[i] << " " << to_string(tunables[i].value()) << endl;
    }
}

int main() {
    initHash(3232);
    initMoveGen();
    srand(get_current_ms());

    add_tunables(piece_eval, 100, 1000, "piece_evals_starting_from_knight", KNIGHT - PAWN, KING - PAWN);
    
    Configuration champ;
    champ.load("champ.nnue");
    Color champ_color = WHITE;
    for (int i = 0; i < 1000; i++) {
        Configuration chall(champ);
        chall.mutate(.9, -1);
        champ.print();
        chall.print();
        Pos pos;
        while (!pos.is_over()) {
            if (pos.turn == champ_color) {
                champ.apply();
            }
            else {
                chall.apply();
            }
            pos.do_move(get_best_move(pos, 50));
        }

        pos.save("games/" + to_string(i) + ".pgn");

        if (pos.is_draw()) {
            cout << i << " result: draw " << endl;
        }
        else if (pos.turn == champ_color) {
            cout << i << " result: champ loss " << endl;
            champ = chall;
            champ.save("nnues/" + to_string(i) + ".nnue");
        }
        else {
            cout << i << " result: champ win " << endl;
        }

        champ_color = opp(champ_color);
    }

    return 0;
}