#include "../public/layer.hpp"
#include "../private/parser.hpp"

namespace servo {

Layer::Layer(std::string name, std::string layer_type, Parser* parser) 
    : name(name), type(layer_type), parser(parser) {
    // There was no explicit usage of index in __init__ in python for setting it, 
    // but the python code had `self.index = len(parser.stack) - 1`.
    // Assuming stack here refers to sys_stack? 
    // In parser.py: `self.sys_stack: list[Layer] = []`
    // If Layer is added to stack, it would be there.
    // If we assume standard usage, we'll check valid stack size.
    if (!parser->sys_stack.empty()) {
        this->index = parser->sys_stack.size() - 1;
    } else {
        this->index = -1;
    }
}

Layer Layer::getAbove() {
    return Safe::call([this]() -> Layer {
        if (this->index - 1 >= 0 && this->index - 1 < this->parser->sys_stack.size()) {
             return this->parser->sys_stack[this->index - 1];
        }
        throw std::out_of_range("Layer index out of range");
    }, "servo.internal.public.layer");
}

Layer Layer::getBelow() {
    return Safe::call([this]() -> Layer {
        if (this->index + 1 < this->parser->sys_stack.size()) {
             return this->parser->sys_stack[this->index + 1];
        }
        throw std::out_of_range("Layer index out of range");
    }, "servo.internal.public.layer");
}

}
