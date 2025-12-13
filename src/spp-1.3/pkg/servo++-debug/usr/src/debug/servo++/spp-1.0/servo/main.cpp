#include "internal/private/parser.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    // Mimic servo/__main__.py logic roughly
    if (argc < 2) {
        std::cerr << "\033[1m[servo@spp]\033[0;91m please provide a servo file as argument 1.\033[0m" << std::endl;
        return 1;
    }
    // simple arg handling
    std::string path = argv[1];
    servo::File f(path, true); // no_read=True initially?
    // python code: Parser(File(..., no_read=True))
    // then check file type
    if (f.getType() != "file") {
         std::cerr << "\033[1m[servo@spp]\033[0;91m tried to run servo file that is a directory or does not exist:\n        - " << f.getPath() << "\033[0m" << std::endl;
         return 1;
    }
    f.read();
    
    servo::Parser p(f);
    try {
        p.parse().execute();
    } catch (const std::exception& e) {
        // Safe wrapper usually handles printing, but main might catch top level
        return 1; 
    }
    return 0;
}
