#ifndef SERVO_INTERNAL_PUBLIC_STRING_HPP
#define SERVO_INTERNAL_PUBLIC_STRING_HPP

#include <string>
#include <iostream>

namespace servo {

class String : public std::string {
public:
    using std::string::string;
    String(const std::string& s) : std::string(s) {}
    String(const char* s) : std::string(s) {}
    String(int i) : std::string(std::to_string(i)) {}
    String(double d) : std::string(std::to_string(d)) {}

    template<typename T>
    String operator+(const T& other) const {
        return String(static_cast<std::string>(*this) + std::to_string(other)); // Simplified
    }
    
    // Concatenation with string
    String operator+(const std::string& other) const {
        return String(static_cast<std::string>(*this) + other);
    }
    String operator+(const char* other) const {
        return String(static_cast<std::string>(*this) + other);
    }

    friend std::ostream& operator<<(std::ostream& os, const String& s) {
        os << static_cast<std::string>(s);
        return os;
    }
};

}

#endif
