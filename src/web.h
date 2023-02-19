#pragma once
#include <emscripten.h>
#include <string>
#include "types.h"

using namespace std;

struct UCI_Option {
    string name;
    string type;
    string default_setting;
    string value;
    string init;
    UCI_Option(string name_, string type_, string default_, string init_) {
        name = name_;
        type = type_;
        default_setting = default_;
        value = default_setting;
        init = init_;
    }
};

// wasm only functions
void launch();

void wasm_println(string line);

void wasm_print_uci_info();

// functions to wrap:
extern "C" {
    int main();

    // void process_uci_line(const char* line);

    // void web_worker(int ti_index);

    // void web_stop();

    // const char* probe_move();

    // int probe_eval();

    // int probe_depth();

    // const char* probe_pv();
}