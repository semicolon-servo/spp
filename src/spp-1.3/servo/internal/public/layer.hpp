#ifndef SERVO_INTERNAL_PUBLIC_LAYER_HPP
#define SERVO_INTERNAL_PUBLIC_LAYER_HPP

#include <string>

namespace servo {

class Parser; // Forward declaration

class Layer {
public:
    std::string name;
    std::string type;
    Parser* parser;
    int index;

    Layer(std::string name, std::string layer_type, Parser* parser);
    
    Layer getAbove();
    Layer getBelow();
};

}

#endif
