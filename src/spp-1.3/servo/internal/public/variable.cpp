#include "../public/variable.hpp"
#include "../private/parser.hpp"
#include <iostream>
#include <vector>

namespace servo {

std::any Variable::call(std::vector<std::any> args) {
    if (this->value.type() == typeid(std::function<std::any(std::vector<std::any>)>)) {
        auto func = std::any_cast<std::function<std::any(std::vector<std::any>)>>(this->value);
        return func(args);
    } else if (this->value.type() == typeid(std::function<void(std::string)>)) {
         // Handle system calls mapping?
         // Builtins.system takes string.
         // If args is vector, convert to string?
         // This is getting tricky due to type erasure.
         // Assuming specialized handling in parser or uniform wrapper.
         throw std::runtime_error("Variable is not callable (type mismatch)");
    }
    throw std::runtime_error("Variable '" + this->name + "' is not callable");
}

}
