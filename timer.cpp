#include "timer.h"
#include <iostream>
#include <chrono>
#include <string>

Timestamp get_current_ms() {
    return chrono::duration_cast< chrono::milliseconds >(chrono::system_clock::now().time_since_epoch()).count();
}

void print_time_diff(Timestamp start) {
    cout<<"Stopwatch: "<<to_string(get_current_ms()-start)<<" ms"<<endl;
}

void pop_time_diff(Timestamp &start) {
    cout<<"Stopwatch: "<<to_string(get_current_ms()-start)<<" ms"<<endl; 
    start = get_current_ms(); 
}
