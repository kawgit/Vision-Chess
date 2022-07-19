#include <functional>
#include <string>

using namespace std;

struct Test {
    string name;
    function<void()> func;

    Test(string n, function<void()> f) {
        name = n;
        func = f;
    }
};