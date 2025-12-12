#ifndef SERVO_INTERNAL_PUBLIC_VARIABLE_HPP
#define SERVO_INTERNAL_PUBLIC_VARIABLE_HPP

#include <string>
#include <any>
#include <map>
#include <memory>
#include <vector>
#include "safe.hpp"

namespace servo {

class Parser;

class Variable {
public:
    std::string name;
    std::any value;
    std::string value_type;
    std::map<std::string, std::shared_ptr<Variable>> children;
    Parser* parser;

    Variable(std::string n, std::any v, std::string t, std::map<std::string, std::shared_ptr<Variable>> c, Parser* p)
        : name(n), value(v), value_type(t), children(c), parser(p) {}
    
    Variable() = default;

    std::any call(std::vector<std::any> args = {}); // Adjusted signature for C++
};

}

#endif
