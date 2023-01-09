#pragma once

#include <iostream>
#include <chrono>
#include <string>

using namespace std;

typedef uint64_t Timestamp;

Timestamp get_current_ms();

Timestamp get_time_diff(Timestamp start);

void print_time_diff(Timestamp start);

void pop_time_diff(Timestamp &start);
