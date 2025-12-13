#ifndef SERVO_INTERNAL_PRIVATE_BUILTINS_HPP
#define SERVO_INTERNAL_PRIVATE_BUILTINS_HPP

#include <string>
#include <functional>
#include "../public/safe.hpp"

namespace servo {

class Builtins {
public:
    static void system(std::string args);
    static std::string systemreturn(std::string args);
    static void if_(bool condition, std::function<void()> true_branch);
};

}

#endif
