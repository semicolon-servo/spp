#include "handler.hpp"
#include <iostream>
#include <stdexcept>

namespace servo {

Handler::Handler(std::vector<std::string> args) : args(args) {}

std::string Handler::get(std::any index_or_option, std::string else_value) {
    return Safe::call([this, index_or_option, else_value]() -> std::string {
        if (index_or_option.type() == typeid(int)) {
            int index = std::any_cast<int>(index_or_option);
            if (index >= 0 && index < this->args.size()) {
                return this->args[index];
            }
            return else_value;
        } else if (index_or_option.type() == typeid(std::string) || index_or_option.type() == typeid(const char*)) {
             std::string option;
             if (index_or_option.type() == typeid(std::string)) option = std::any_cast<std::string>(index_or_option);
             else option = std::string(std::any_cast<const char*>(index_or_option));
             
             for (size_t i = 0; i < this->args.size(); ++i) {
                 if (this->args[i] == option) {
                     if (i + 1 < this->args.size()) return this->args[i+1];
                 }
             }
             return else_value;
        }
        throw std::invalid_argument("index_or_option must be int or str");
    }, "servo.internal.private.handler");
}

}
