#ifndef SERVO_INTERNAL_PUBLIC_SAFE_HPP
#define SERVO_INTERNAL_PUBLIC_SAFE_HPP

#include <iostream>
#include <string>
#include <exception>
#include <cstdlib>
#include <typeinfo>
#include <cxxabi.h>

namespace servo {

class Safe {
public:
    template<typename Func>
    static auto call(Func f, const std::string& file_name = "") -> decltype(f()) {
        try {
            return f();
        } catch (const std::exception& error) {
            std::string error_name = typeid(error).name();
            int status;
            char* demangled = abi::__cxa_demangle(error_name.c_str(), 0, 0, &status);
            if(status == 0) {
                error_name = demangled;
                free(demangled);
            }
            
            // Check if this is a ReturnSignal - if so, silently re-throw
            // std::cerr << "DEBUG: error_name = " << error_name << std::endl;
            if (error_name.find("ReturnSignal") != std::string::npos) {
                throw;
            }
            
            // Basic formatting to match Python output style (approximated)
            std::string pretty_name = "";
            for (char c : error_name) {
                if (std::isupper(c)) pretty_name += " ";
                pretty_name += std::toupper(c);
            }
            // Strip "std::" etc if needed? simpler to just show what we have.
            
            // replace ERROR with FATAL
            size_t pos = pretty_name.find("ERROR");
            if (pos != std::string::npos) pretty_name.replace(pos, 5, "FATAL");

            std::cout << "\033[1m[servo@spp]\033[0;91m got '" << pretty_name << "' from function in '" 
                      << (file_name.empty() ? "<unknown>" : file_name) << "':\n      - " 
                      << error.what() << "\033[0m" << std::endl;

            if (file_name == "servo.base") {
                 std::cout << "\033[91m      - exit with code 1\033[0m" << std::endl;
                 std::exit(1);
            }
            throw;
        }
    }
};

}

#endif
