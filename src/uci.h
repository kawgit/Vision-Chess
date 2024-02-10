#pragma once

#include "search.h"
#include <vector>
#include <map>
#include <string>

namespace uci {

struct Option {
    std::string name;
    std::string type;
    std::string default_setting;
    std::string value;
    std::string init;
    Option(std::string name_, std::string type_, std::string default_, std::string init_) {
        name = name_;
        type = type_;
        default_setting = default_;
        value = default_setting;
        init = init_;
    }
};

// void print_uci_info(SearchInfo& si);

void mainloop();

} // namespace uci