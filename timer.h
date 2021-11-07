#pragma once

#include <iostream>
#include <chrono>
#include <string>

using namespace std;

typedef uint64_t Timestamp;

Timestamp get_current_ms();

void print_time_diff(Timestamp start);

void pop_time_diff(Timestamp &start);
