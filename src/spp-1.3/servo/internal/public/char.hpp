#ifndef SERVO_INTERNAL_PUBLIC_CHAR_HPP
#define SERVO_INTERNAL_PUBLIC_CHAR_HPP

#include <string>
#include <map>
#include <any>

namespace servo {

class Parser;

class Char {
public:
    std::string string_val; // 'string' is a keyword/type
    int index;
    Parser* parser;
    std::map<std::string, std::any> data;

    Char(std::string s, int i, Parser* p) : string_val(s), index(i), parser(p) {}
};

}

#endif
