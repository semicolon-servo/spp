#include "../public/parsedmaterial.hpp"
#include "../private/parser.hpp"
#include "../public/safe.hpp"
#include <iostream>

namespace servo {

void ParsedMaterial::execute() {
    if (!this->parser->file.path.empty()) {
        auto func = this->raw_execute;
        Safe::call([func, this]() {
            // "servo.base" for main? Or file name.
            // Python: self.parser.file.getParts()[-2] + "." + self.parser.file.getBaseName()...
            // We'll mimic this naming for Safe call
            std::string name = "workspace.main"; // Placeholder logic 
            // In C++ reusing Safe::call properly with dynamic name might be tricky due to static file_name arg.
            // But Safe::call takes string.
            try {
                func();
            } catch (const ReturnSignal&) {
                // Only ignore top-level return signals for non-virtual files
                // Virtual files (function bodies) should propagate ReturnSignal
                if (this->parser->file.path != "virtual") {
                    // Ignore for real files
                } else {
                    throw; // Re-throw for virtual files (function bodies)
                }
            }
        }, "parsed_execution"); 
    } else {
        this->raw_execute();
    }
}

}
