#ifndef SERVO_INTERNAL_PUBLIC_PARSEDMATERIAL_HPP
#define SERVO_INTERNAL_PUBLIC_PARSEDMATERIAL_HPP

#include <functional>
#include "safe.hpp"

namespace servo {

class Parser;

class ParsedMaterial {
public:
    std::function<void()> raw_execute;
    Parser* parser;

    ParsedMaterial(std::function<void()> raw_stmt, Parser* p) : raw_execute(raw_stmt), parser(p) {}
    
    void execute();
    void run() { execute(); } // Alias if needed, or stick to execute
};

}

#endif
