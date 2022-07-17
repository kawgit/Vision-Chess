#pragma once

#include "search.h"
#include <vector>
#include <map>
#include <string>

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

void process_command(string cmd);

void print_uci_info(Pos& pos, SearchInfo& si, vector<ThreadInfo>& tis);

void uci();