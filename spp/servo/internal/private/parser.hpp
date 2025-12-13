#ifndef SERVO_INTERNAL_PRIVATE_PARSER_HPP
#define SERVO_INTERNAL_PRIVATE_PARSER_HPP

#include <vector>
#include <map>
#include <string>
#include <any>
#include <functional>
#include <memory> 
// Include dependencies
#include "../public/file.hpp"
#include "../public/char.hpp"
#include "../public/layer.hpp"
#include "../public/variable.hpp"
#include "../public/parsedmaterial.hpp"
#include "../public/string.hpp"
#include "../public/safe.hpp"
#include "builtins.hpp"

namespace servo {

struct ReturnSignal : public std::exception {
    std::any value;
    ReturnSignal(std::any v) : value(v) {}
};


class Parser {
public:
    File file;
    // Char* char_obj; // using pointer or optional
    std::shared_ptr<Char> char_obj;
    std::vector<std::map<std::string, std::any>> mode_stack;
    std::vector<Layer> sys_stack;
    std::vector<std::function<void()>> parsed_funcs;
    std::map<std::string, std::shared_ptr<Variable>> pool;

    Parser(File file);

    std::string getLastModeStackType();
    std::string wrap_strings(std::string expr);
    std::shared_ptr<Variable> findVariable(std::string name);
    ParsedMaterial parse();
    void execute();
    std::string parseSource();
    void parseChar(std::string match_value = "");
    
    // Parsing methods
    void parseNull();
    void parseIdentifier();
    void parseCall();
    void parseCheckAssignment();
    void parseString();
    void parseMath();
    void parseComment();
    void parseMLComment();
    void parseArtifact();
    void parseInteger();
    void parseAssignment();
    void parseFunctionDef();
    void parseBlock();
    void parseWaitBlock(bool eof=false);
    void parseReturn();

    void defineFunction(std::string name, std::vector<std::string> args, std::string body);
    
    // Helper for eval
    std::any evaluate_expression(std::string expr);
};

}

#endif
