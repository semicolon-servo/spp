#include "builtins.hpp"
#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <array>
#include <memory>

namespace servo {

void Builtins::system(std::string args) {
    Safe::call([args]() {
        // Simple system call, not capturing output properly for print unless we simulate check=True
        int ret = std::system(args.c_str());
        if (ret != 0) {
            throw std::runtime_error("Command failed with return code " + std::to_string(ret));
        }
    }, "servo.internal.private.builtins");
}

std::string Builtins::systemreturn(std::string args) {
    return Safe::call([args]() -> std::string {
        std::array<char, 128> buffer;
        std::string result;
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(args.c_str(), "r"), pclose);
        if (!pipe) {
            throw std::runtime_error("popen() failed!");
        }
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result += buffer.data();
        }
        return result;
    }, "servo.internal.private.builtins");
}

void Builtins::if_(bool condition, std::function<void()> true_branch) {
    Safe::call([condition, true_branch]() {
        if (condition) true_branch();
    }, "servo.internal.private.builtins");
}

}
