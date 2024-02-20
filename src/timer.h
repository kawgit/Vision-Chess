#pragma once

#include <iostream>
#include <chrono>
#include <string>



typedef uint64_t Timestamp;

const Timestamp TIME_MAX = 0xFFFFFFFFFFFFFFFF;

Timestamp get_current_ms();

Timestamp get_time_diff(Timestamp start);

void print_time_diff(Timestamp start);

void pop_time_diff(Timestamp &start);
