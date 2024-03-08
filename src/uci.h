#pragma once

#include <optional>
#include <string>
#include <vector>

#include "search.h"

namespace uci {

void print(std::string message);

struct Option {
    std::string name;
    std::string type;
    std::string def;
    
    std::optional<int> min;
    std::optional<int> max;
    
    std::string value;
    
};

void mainloop();

} // namespace uci