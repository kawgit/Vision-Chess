#include <functional>
#include <string>



struct Test {
    std::string name;
    function<void()> func;

    Test(std::string n, function<void()> f) {
        name = n;
        func = f;
    }
};