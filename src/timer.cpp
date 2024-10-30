#include <chrono>
#include <iostream>
#include <string>

#include "timer.h"

Timestamp get_current_ms() {
    return std::chrono::duration_cast< std::chrono::milliseconds >(std::chrono::system_clock::now().time_since_epoch()).count();
}

Timestamp get_time_diff(Timestamp start) {
    return get_current_ms() - start;
}

void print_time_diff(Timestamp start) {
    std::cout << "Stopwatch: " << std::to_string(get_current_ms() - start) << " ms" << std::endl;
}

void pop_time_diff(Timestamp &start) {
    std::cout << "Stopwatch: " << std::to_string(get_current_ms() - start) << " ms" << std::endl; 
    start = get_current_ms(); 
}
