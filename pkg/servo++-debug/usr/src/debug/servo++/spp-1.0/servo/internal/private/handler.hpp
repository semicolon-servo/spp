#ifndef SERVO_INTERNAL_PRIVATE_HANDLER_HPP
#define SERVO_INTERNAL_PRIVATE_HANDLER_HPP

#include <vector>
#include <string>
#include <any>
#include "../public/safe.hpp"

namespace servo {

class Handler {
public:
    std::vector<std::string> args;

    Handler(std::vector<std::string> args);
    
    std::string get(std::any index_or_option, std::string else_value = "");
};

}

#endif
