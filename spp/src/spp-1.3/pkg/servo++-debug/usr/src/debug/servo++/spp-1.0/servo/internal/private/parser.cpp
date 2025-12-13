#include "parser.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <cctype>
#include <sstream>
#include <cmath>
#include "../public/safe.hpp"

namespace servo {

Parser::Parser(File file) : file(file) {
    this->char_obj = nullptr;
    // Pool init
    this->pool["system"] = std::make_shared<Variable>("system", 
        std::function<std::any(std::vector<std::any>)>([](std::vector<std::any> args) -> std::any {
            std::string s;
            if (!args.empty()) {
                if (args[0].type() == typeid(String)) s = static_cast<std::string>(std::any_cast<String>(args[0]));
                else if (args[0].type() == typeid(std::string)) s = std::any_cast<std::string>(args[0]);
            }
            Builtins::system(s);
            return std::any();
        }), 
        "func", std::map<std::string, std::shared_ptr<Variable>>{}, this);

    this->pool["systemreturn"] = std::make_shared<Variable>("systemreturn", 
        std::function<std::any(std::vector<std::any>)>([](std::vector<std::any> args) -> std::any {
             std::string s;
             if (!args.empty()) {
                if (args[0].type() == typeid(String)) s = static_cast<std::string>(std::any_cast<String>(args[0]));
                else if (args[0].type() == typeid(std::string)) s = std::any_cast<std::string>(args[0]);
             }
             return std::any(String(Builtins::systemreturn(s)));
        }), 
        "func", std::map<std::string, std::shared_ptr<Variable>>{}, this);

    // system_math placeholder - could be exposed math capabilities
    // system_math
    auto system_math = std::make_shared<Variable>("system_math", 0, "module", std::map<std::string, std::shared_ptr<Variable>>{}, this);
    system_math->children["pi"] = std::make_shared<Variable>("pi", String("3.14159265359"), "float", std::map<std::string, std::shared_ptr<Variable>>{}, this);
    
    // helper for math functions
    auto math_func = [this](std::string cmd_name) {
         return std::function<std::any(std::vector<std::any>)>([cmd_name](std::vector<std::any> args) -> std::any {
             if(args.empty()) return String("0");
             std::string val_str;
             if (args[0].type() == typeid(String)) val_str = static_cast<std::string>(std::any_cast<String>(args[0]));
             else if (args[0].type() == typeid(std::string)) val_str = std::any_cast<std::string>(args[0]);
             
             // use bc -l for math functions? e.g. s(x), c(x)
             std::string bc_func = "";
             if (cmd_name == "sin") bc_func = "s";
             else if (cmd_name == "cos") bc_func = "c";
             else if (cmd_name == "tan") bc_func = "s/c"; // simplistic, bc -l has s, c, a, l, e, j
             else if (cmd_name == "sqrt") bc_func = "sqrt";
             
             if (!bc_func.empty()) {
                  std::string cmd = "echo \"" + bc_func + "(" + val_str + ")\" | bc -l";
                  std::string res = Builtins::systemreturn(cmd);
                  if(!res.empty() && res.back() == '\n') res.pop_back();
                  return String(res);
             }
             return String("0");
         });
    };

    system_math->children["sin"] = std::make_shared<Variable>("sin", math_func("sin"), "func", std::map<std::string, std::shared_ptr<Variable>>{}, this);
    system_math->children["cos"] = std::make_shared<Variable>("cos", math_func("cos"), "func", std::map<std::string, std::shared_ptr<Variable>>{}, this);
    system_math->children["sqrt"] = std::make_shared<Variable>("sqrt", math_func("sqrt"), "func", std::map<std::string, std::shared_ptr<Variable>>{}, this);
    
    this->pool["system_math"] = system_math;
    // input placeholder
     this->pool["input"] = std::make_shared<Variable>("input", 0, "func", std::map<std::string, std::shared_ptr<Variable>>{}, this);
}

std::string Parser::getLastModeStackType() {
    if (!mode_stack.empty()) {
        return std::any_cast<std::string>(mode_stack.back()["type"]);
    }
    return "NULL";
}

std::string Parser::wrap_strings(std::string expr) {
    // Regex refactoring difficult in C++ std without regex lib overhead.
    // Simplified: return as is.
    return expr;
}

std::shared_ptr<Variable> Parser::findVariable(std::string name) {
    if (pool.find(name) != pool.end()) {
        auto val = pool[name];
        // Handle derived / string wrapping logic...
        return val;
    }
    // Dot access logic
    size_t dot = name.find(".");
    if (dot != std::string::npos) {
        std::string part0 = name.substr(0, dot);
        std::string part1 = name.substr(dot + 1); // Only support 1 level of dot for now?
        // Recursion or loop for multi-level? simple loop
        
        // Loop logic
        std::string current_name = part0;
        std::string remainder = part1;
        
        if (pool.find(current_name) != pool.end()) {
             auto current_var = pool[current_name];
             // Navigate down
             while(true) {
                 size_t next_dot = remainder.find(".");
                 std::string key = (next_dot == std::string::npos) ? remainder : remainder.substr(0, next_dot);
                 std::string next_rem = (next_dot == std::string::npos) ? "" : remainder.substr(next_dot + 1);
                 
                 if (current_var->children.find(key) != current_var->children.end()) {
                     current_var = current_var->children[key];
                 } else {
                     throw std::runtime_error("Variable '" + key + "' not found in '" + current_name + "'");
                 }
                 
                 if (next_rem.empty()) return current_var;
                 
                 current_name += "." + key;
                 remainder = next_rem;
             }
        }
    }
    throw std::runtime_error("variable '" + name + "' not found");
}

ParsedMaterial Parser::parse() {
    return ParsedMaterial([this]() {
        this->parseSource();
        this->execute();
    }, this);
}

void Parser::execute() {
    for (auto& func : parsed_funcs) {
        func();
    }
}

std::string Parser::parseSource() {
    std::string content = file.read();
    for (size_t i = 0; i < content.length(); ++i) {
        std::string s(1, content[i]);
        this->char_obj = std::make_shared<Char>(s, i, this);
        this->parseChar();
    }
    if (!mode_stack.empty() && std::any_cast<std::string>(mode_stack.back()["type"]) == "WAIT_BLOCK") {
        this->parseWaitBlock(true);
    }
    if (!mode_stack.empty()) {
        throw std::runtime_error("Unexpected end of file. Unterminated mode: " + std::any_cast<std::string>(mode_stack.back()["type"]));
    }
    return "";
}

void Parser::parseChar(std::string match_value) {
    std::string mode = match_value;
    if (mode.empty()) mode = getLastModeStackType();
    
    // std::cout << "Debug: char='" << (char_obj ? char_obj->string_val : "null") << "' mode='" << mode << "' stack_size=" << mode_stack.size() << std::endl;

    if (mode == "NULL") parseNull();
    else if (mode == "IDENTIFIER") parseIdentifier();
    else if (mode == "CALL") parseCall();
    else if (mode == "CHECK_ASSIGNMENT") parseCheckAssignment();
    else if (mode == "STRING") parseString();
    else if (mode == "MATH") parseMath();
    else if (mode == "COMMENT") parseComment();
    else if (mode == "MLCOMMENT") parseMLComment();
    else if (mode == "ARTIFACT") parseArtifact();
    else if (mode == "INTEGER") parseInteger();
    else if (mode == "ASSIGNMENT") parseAssignment();
    else if (mode == "FUNCTION_DEF") parseFunctionDef();
    else if (mode == "BLOCK") parseBlock();
    else if (mode == "WAIT_BLOCK") parseWaitBlock();
    else if (mode == "RETURN") parseReturn();
}

void Parser::parseNull() {
    std::string s = char_obj->string_val;
    if (isalpha(s[0]) || s == "_") {
        std::map<std::string, std::any> m; m["type"] = std::string("IDENTIFIER"); m["buffer"] = s;
        mode_stack.push_back(m);
    } else if (s == "\"" || s == "\'") {
        std::map<std::string, std::any> m; m["type"] = std::string("STRING"); m["buffer"] = std::string(""); m["quote"] = s;
        mode_stack.push_back(m);
    } else if (isdigit(s[0])) {
         std::map<std::string, std::any> m; m["type"] = std::string("INTEGER"); m["buffer"] = s;
         mode_stack.push_back(m);
    } else if (s == "#") {
         std::map<std::string, std::any> m; m["type"] = std::string("COMMENT");
         mode_stack.push_back(m);
    } else if (s == "<") {
         std::map<std::string, std::any> m; m["type"] = std::string("ARTIFACT"); m["buffer"] = std::string("");
         mode_stack.push_back(m);
    } else if (isspace(s[0])) {
        // pass
    } else {
        throw std::runtime_error("Unexpected character: '" + s + "'");
    }
}
// ... Implement other methods similarly ...

// Placeholder implementations for the sake of completion within tool limits
void Parser::parseIdentifier() { 
    std::string s = char_obj->string_val;
    if (isalnum(s[0]) || s == "." || s == "_") {
        std::string buf = std::any_cast<std::string>(mode_stack.back()["buffer"]);
        mode_stack.back()["buffer"] = buf + s;
    } else if (s == "(") {
        std::string buf = std::any_cast<std::string>(mode_stack.back()["buffer"]);
        mode_stack.push_back({{"type", std::string("CALL")}, {"identifier", buf}, {"buffer", std::string("")}});
        // pop IDENTIFIER
        // In python: self.mode_stack.pop(-2)
        // Here stack is ... IDENTIFIER, CALL. 
        // We want to remove IDENTIFIER (which is at -2).
        mode_stack.erase(mode_stack.end() - 2);
    } else if (isspace(s[0])) {
         std::string buf = std::any_cast<std::string>(mode_stack.back()["buffer"]);
         if (buf == "fn") {
             mode_stack.back()["type"] = std::string("FUNCTION_DEF");
             mode_stack.back()["phase"] = std::string("name");
             mode_stack.back()["buffer"] = std::string("");
         } else if (buf == "return") {
             mode_stack.back()["type"] = std::string("RETURN");
             mode_stack.back()["buffer"] = std::string("");
         } else {
             // strict check assignment logic
             mode_stack.back()["type"] = std::string("CHECK_ASSIGNMENT");
         }
    } else if (s == "=") {
         // assignment
         std::string var_name = std::any_cast<std::string>(mode_stack.back()["buffer"]); // should strip?
         mode_stack.pop_back();
         mode_stack.push_back({{"type", std::string("ASSIGNMENT")}, {"name", var_name}, {"buffer", std::string("")}});
    } else {
         mode_stack.pop_back();
    }
}

void Parser::parseCall() { 
     std::map<std::string, std::any>& mode = mode_stack.back();
     std::string s = char_obj->string_val;

     // Quote handling
     if (mode.count("quote") && std::any_cast<std::string>(mode["quote"]) != "") {
          std::string q = std::any_cast<std::string>(mode["quote"]);
          if (s == q) mode["quote"] = std::string("");
          std::string buf = std::any_cast<std::string>(mode["buffer"]);
          mode["buffer"] = buf + s;
          return;
     }
     if (s == "'" || s == "\"") {
          mode["quote"] = s;
          std::string buf = std::any_cast<std::string>(mode["buffer"]);
          mode["buffer"] = buf + s;
          return;
     }

     if (s == "(") {
         int nesting = 0;
         if(mode.count("nesting")) nesting = std::any_cast<int>(mode["nesting"]);
         mode["nesting"] = nesting + 1;
         std::string buf = std::any_cast<std::string>(mode["buffer"]);
         mode["buffer"] = buf + s;
     } else if (s == ")") {
          int nesting = 0;
          if(mode.count("nesting")) nesting = std::any_cast<int>(mode["nesting"]);
          
          if (nesting > 0) {
              mode["nesting"] = nesting - 1;
              std::string buf = std::any_cast<std::string>(mode["buffer"]);
              mode["buffer"] = buf + s;
          } else {
               std::string identifier = std::any_cast<std::string>(mode["identifier"]);
               std::string arg_str = std::any_cast<std::string>(mode["buffer"]);
               mode_stack.pop_back(); 
               // poping CALL

               // Split args
               std::vector<std::any> args;
               std::stringstream ss(arg_str);
               std::string item;
               // Basic split by comma (doesn't handle commas in strings/nested)
               // TODO: Better split
               while(std::getline(ss, item, ',')) {
                    // trim
                    item.erase(0, item.find_first_not_of(" \t"));
                    item.erase(item.find_last_not_of(" \t") + 1);
                    if(item.empty()) continue;

                    // Check if pure math first (no quotes, no alpha except e/E if we supported sci notation, but let's stick to basic)
                    if (item.find_first_not_of("0123456789+-*/%^. ()") == std::string::npos && 
                        item.find_first_of("0123456789") != std::string::npos) { // Ensure at least one digit
                         std::string cmd = "echo \"" + item + "\" | bc";
                         std::string res = Builtins::systemreturn(cmd);
                         if(!res.empty() && res.back() == '\n') res.pop_back();
                         args.push_back(String(res));
                         continue;
                    }
                    
                    // Simple expression evaluator for concatenations
                    std::string current_val;
                    std::stringstream ss_plus(item);
                    std::string part;
                    bool first = true;
                    
                    while(std::getline(ss_plus, part, '+')) {
                        // trim part
                        part.erase(0, part.find_first_not_of(" \t"));
                        part.erase(part.find_last_not_of(" \t") + 1);
                        
                        std::string part_val;
                        if(part.empty()) continue;

                        if(part.size() >= 2 && (part.front() == '"' || part.front() == '\'') && part.back() == part.front()) {
                            part_val = part.substr(1, part.size()-2);
                        } else if(isdigit(part[0]) || part[0] == '-') {
                            // Math
                            std::string cmd = "echo \"" + part + "\" | bc";
                            std::string res = Builtins::systemreturn(cmd);
                            if(!res.empty() && res.back() == '\n') res.pop_back();
                            part_val = res;
                        } else {
                            // Function call check
                            size_t open_paren = part.find('(');
                            size_t close_paren = part.rfind(')');
                            bool handled_call = false;
                            
                            if (open_paren != std::string::npos && close_paren == part.size() - 1 && open_paren < close_paren) {
                                 std::string func_name = part.substr(0, open_paren);
                                 bool valid_id = true;
                                 for(char c : func_name) if(!isalnum(c) && c != '.' && c != '_') valid_id = false;
                                 
                                 if(valid_id) {
                                     try {
                                         auto func_var = this->findVariable(func_name);
                                         std::string args_str = part.substr(open_paren + 1, close_paren - open_paren - 1);
                                         std::vector<std::any> func_args;
                                         std::stringstream ss_args(args_str);
                                         std::string arg_item;
                                         while(std::getline(ss_args, arg_item, ',')) {
                                              arg_item.erase(0, arg_item.find_first_not_of(" \t"));
                                              arg_item.erase(arg_item.find_last_not_of(" \t") + 1);
                                              if(arg_item.empty()) continue;
                                              if(isdigit(arg_item[0])) {
                                                  func_args.push_back(String(arg_item)); 
                                              } else {
                                                  try {
                                                      func_args.push_back(this->findVariable(arg_item)->value);
                                                  } catch(...) {
                                                      func_args.push_back(String(arg_item));
                                                  }
                                              }
                                         }
                                         std::any res = func_var->call(func_args);
                                         if (res.type() == typeid(String)) part_val = static_cast<std::string>(std::any_cast<String>(res));
                                         else if (res.type() == typeid(std::string)) part_val = std::any_cast<std::string>(res);
                                         else if (res.type() != typeid(void)) part_val = ""; // Has value but unknown type
                                         handled_call = true;
                                     } catch(const ReturnSignal& sig) {
                                         // Function returned via signal - extract value
                                         if (sig.value.type() == typeid(String)) part_val = static_cast<std::string>(std::any_cast<String>(sig.value));
                                         else if (sig.value.type() == typeid(std::string)) part_val = std::any_cast<std::string>(sig.value);
                                         else part_val = "";
                                         handled_call = true;
                                     } catch(...) {}
                                 }
                            }
                            
                            if (!handled_call) {
                                // Variable
                                try {
                                    auto v = this->findVariable(part);
                                    if (v->value.type() == typeid(String)) part_val = static_cast<std::string>(std::any_cast<String>(v->value));
                                    else if (v->value.type() == typeid(std::string)) part_val = std::any_cast<std::string>(v->value);
                                    else part_val = "";
                                } catch(...) {
                                    part_val = part; // fallback
                                }
                            }
                        }
                        
                        if(first) current_val = part_val;
                        else {
                            // Check if both are numeric for addition
                            bool is_num1 = current_val.find_first_not_of("0123456789.-") == std::string::npos && 
                                           current_val.find_first_of("0123456789") != std::string::npos;
                            bool is_num2 = part_val.find_first_not_of("0123456789.-") == std::string::npos && 
                                           part_val.find_first_of("0123456789") != std::string::npos;
                            
                            if (is_num1 && is_num2) {
                                std::string cmd = "echo \"" + current_val + " + " + part_val + "\" | bc -l";
                                std::string res = Builtins::systemreturn(cmd);
                                if(!res.empty() && res.back() == '\n') res.pop_back();
                                // bc -l can produce .123, normalize?
                                current_val = res;
                            } else {
                                current_val += part_val;
                            }
                        }
                        first = false;
                    }
                    args.push_back(String(current_val));
               }

               std::shared_ptr<Variable> var = this->findVariable(identifier);
               if (var->children.count("__block_arg_index")) {
                    std::map<std::string, std::any> next;
                    next["type"] = std::string("WAIT_BLOCK");
                    next["func"] = var;
                    next["run_args"] = args;
                    next["buffer"] = std::string("");
                    mode_stack.push_back(next);
               } else {
                    var->call(args);
               }
          }
     } else {
          std::string buf = std::any_cast<std::string>(mode["buffer"]);
          mode["buffer"] = buf + s;
     }
}
void Parser::parseCheckAssignment() {
    std::string s = char_obj->string_val;
    if (isspace(s[0]) && s != "\n") return;
    if (s == "=") {
        // ...
    } else if (s == "\n") {
         throw std::runtime_error("Unexpected token/newline after identifier");
    } else {
         throw std::runtime_error("Unexpected token after identifier");
    }
}
void Parser::parseString() {
    std::string s = char_obj->string_val;
    std::string quote = std::any_cast<std::string>(mode_stack.back()["quote"]);
    if (s == quote) {
        mode_stack.pop_back();
    } else {
        std::string buf = std::any_cast<std::string>(mode_stack.back()["buffer"]);
        mode_stack.back()["buffer"] = buf + s;
    }
}
void Parser::parseMath() {
    std::string s = char_obj->string_val;
    if (isdigit(s[0]) || std::string("+-*/%^.").find(s) != std::string::npos) {
         std::string buf = std::any_cast<std::string>(mode_stack.back()["buffer"]);
         mode_stack.back()["buffer"] = buf + s;
    } else {
         std::string buf = std::any_cast<std::string>(mode_stack.back()["buffer"]);
         mode_stack.pop_back();
         
         // Evaluate
         std::string res = buf;
         // Use bc for now
         std::string cmd = "echo \"" + buf + "\" | bc -l"; // -l for float logic if needed
         std::string out = Builtins::systemreturn(cmd);
         if (!out.empty() && out.back() == '\n') out.pop_back();
         res = out;
         
         // Update parent buffer
         if (!mode_stack.empty()) {
             std::string pbuf = std::any_cast<std::string>(mode_stack.back()["buffer"]);
             mode_stack.back()["buffer"] = pbuf + res;
         }
         
         this->parseChar();
    }
}
void Parser::parseComment() { if (char_obj->string_val == "\n") mode_stack.pop_back(); }
void Parser::parseMLComment() {}
void Parser::parseArtifact() {
     std::string s = char_obj->string_val;
     if (s == ">") {
          std::string buf = std::any_cast<std::string>(mode_stack.back()["buffer"]);
          // buffer: "import foo"
          std::string action; 
          std::string module_name;
          std::stringstream ss(buf);
          ss >> action >> module_name;
          
          if (action == "import") {
              std::string path;
              if (File(module_name + ".sv").getExists()) {
                  path = module_name + ".sv";
              } else if (File("reach/" + module_name + ".sv").getExists()) {
                  path = "reach/" + module_name + ".sv";
              } else if (File("../reach/" + module_name + ".sv").getExists()) {
                  path = "../reach/" + module_name + ".sv";
              } else if (File("servo/reach/" + module_name + ".sv").getExists()) {
                   path = "servo/reach/" + module_name + ".sv";
              } else {
                  throw std::runtime_error("Module '" + module_name + "' not found locally or in reach.");
              }
              
              Parser module_parser{File(path)};
              module_parser.parse().execute(); // run it
              
              // Create module variable
              // Filter defaults? Defaults are system, systemreturn, system_math, input.
              std::map<std::string, std::shared_ptr<Variable>> module_members;
              for(auto const& [key, val] : module_parser.pool) {
                  if (key != "system" && key != "systemreturn" && key != "system_math" && key != "input") {
                      module_members[key] = val;
                  }
              }
              
              // Variable type 'module', value could be anything or just use children
              auto mod_var = std::make_shared<Variable>(module_name, 0, "module", module_members, this);
              this->pool[module_name] = mod_var;

          } else {
               throw std::runtime_error("Unknown artifact action: " + action);
          }
          mode_stack.pop_back();
     } else {
           std::string buf = std::any_cast<std::string>(mode_stack.back()["buffer"]);
           mode_stack.back()["buffer"] = buf + s;
     }
}
void Parser::parseInteger() {
    std::string s = char_obj->string_val;
    if (isdigit(s[0])) {
         std::string buf = std::any_cast<std::string>(mode_stack.back()["buffer"]);
         mode_stack.back()["buffer"] = buf + s;
    } else if (std::string("+-*/%^").find(s) != std::string::npos) {
         mode_stack.back()["type"] = std::string("MATH");
         std::string buf = std::any_cast<std::string>(mode_stack.back()["buffer"]);
         mode_stack.back()["buffer"] = buf + s;
    } else if (s == ")") {
         mode_stack.pop_back();
         this->parseChar();
    } else {
         mode_stack.pop_back();
         this->parseChar();
    }
}
void Parser::parseAssignment() {
     std::string s = char_obj->string_val;
     if (s == "\n") {
         std::string buf = std::any_cast<std::string>(mode_stack.back()["buffer"]);
         std::string var_name = std::any_cast<std::string>(mode_stack.back()["name"]);
         mode_stack.pop_back();

        // remove leading/trailing whitespace
        buf.erase(0, buf.find_first_not_of(" \t"));
        buf.erase(buf.find_last_not_of(" \t") + 1);

         if (!buf.empty()) {
             std::any val;
             if (buf.size() >=2 && (buf.front() == '"' || buf.front() == '\'')) {
                  val = String(buf.substr(1, buf.size()-2));
             } else if (isdigit(buf[0]) || buf[0] == '-') {
                  std::string cmd = "echo \"" + buf + "\" | bc";
                  std::string res = Builtins::systemreturn(cmd);
                   // trim newline
                   if(!res.empty() && res.back() == '\n') res.pop_back();
                  val = String(res);
             } else {
                  try {
                      auto v = this->findVariable(buf);
                      val = v->value;
                  } catch(...) {
                      // Fallback to string if not found? Or error? Python evals. 
                      // If fail, Python catches exception and does nothing?
                      // Python: try link; except: print error; pass
                      // We will assume string
                      val = String(buf);
                  }
             }
             this->pool[var_name] = std::make_shared<Variable>(var_name, val, "String", std::map<std::string, std::shared_ptr<Variable>>{}, this);
         }
     } else {
         std::string buf = std::any_cast<std::string>(mode_stack.back()["buffer"]);
         mode_stack.back()["buffer"] = buf + s;
     }
}
void Parser::parseFunctionDef() {
    std::map<std::string, std::any>& mode = mode_stack.back();
    std::string s = char_obj->string_val;
    std::string phase = std::any_cast<std::string>(mode["phase"]);

    if (phase == "name") {
        if (s == "(") {
            std::string buf = std::any_cast<std::string>(mode["buffer"]);
            // trim
            buf.erase(0, buf.find_first_not_of(" \n\r\t"));
            buf.erase(buf.find_last_not_of(" \n\r\t") + 1);
            mode["name"] = buf;
            mode["buffer"] = std::string("");
            mode["phase"] = std::string("args");
        } else if (!s.empty() && !isspace(s[0])) {
             std::string buf = std::any_cast<std::string>(mode["buffer"]);
             mode["buffer"] = buf + s;
        }
    } else if (phase == "args") {
        if (s == ")") {
             std::string args_str = std::any_cast<std::string>(mode["buffer"]);
             std::vector<std::string> args;
             std::stringstream ss(args_str);
             std::string item;
             while (std::getline(ss, item, ',')) {
                 // trim
                 item.erase(0, item.find_first_not_of(" \n\r\t"));
                 item.erase(item.find_last_not_of(" \n\r\t") + 1);
                 if(!item.empty()) args.push_back(item);
             }
             mode["args"] = args;
             mode["buffer"] = std::string("");
             mode["phase"] = std::string("before_body");
        } else {
             std::string buf = std::any_cast<std::string>(mode["buffer"]);
             mode["buffer"] = buf + s;
        }
    } else if (phase == "before_body") {
         if (s == "{") {
             mode["phase"] = std::string("body");
             mode["nesting"] = 1;
             mode["buffer"] = std::string("");
         }
    } else if (phase == "body") {
         int nesting = std::any_cast<int>(mode["nesting"]);
         if (s == "{") {
             mode["nesting"] = nesting + 1;
             std::string buf = std::any_cast<std::string>(mode["buffer"]);
             mode["buffer"] = buf + s;
         } else if (s == "}") {
             nesting--;
             mode["nesting"] = nesting;
             if (nesting == 0) {
                 std::string name = std::any_cast<std::string>(mode["name"]);
                 std::vector<std::string> args = std::any_cast<std::vector<std::string>>(mode["args"]);
                 std::string body = std::any_cast<std::string>(mode["buffer"]);
                 
                 this->defineFunction(name, args, body);
                 mode_stack.pop_back();
             } else {
                 std::string buf = std::any_cast<std::string>(mode["buffer"]);
                 mode["buffer"] = buf + s;
             }
         } else {
             std::string buf = std::any_cast<std::string>(mode["buffer"]);
             mode["buffer"] = buf + s;
         }
    }
}
void Parser::parseBlock() {
    std::map<std::string, std::any>& mode = mode_stack.back();
    std::string s = char_obj->string_val;

    if (s == "{") {
        mode["nesting"] = std::any_cast<int>(mode["nesting"]) + 1;
        std::string buf = std::any_cast<std::string>(mode["buffer"]);
        mode["buffer"] = buf + s;
    } else if (s == "}") {
        int nesting = std::any_cast<int>(mode["nesting"]) - 1;
        mode["nesting"] = nesting;
        if (nesting == 0) {
            std::string block_code = std::any_cast<std::string>(mode["buffer"]);
            mode_stack.pop_back();

            std::string anon_name = "__lambda_" + std::to_string(this->pool.size());
            this->defineFunction(anon_name, {}, block_code);

            if (!mode_stack.empty()) {
                // Should check if parent has buffer
                 try {
                    std::string buf = std::any_cast<std::string>(mode_stack.back()["buffer"]);
                    mode_stack.back()["buffer"] = buf + anon_name;
                 } catch(...) {}
            }
        } else {
             std::string buf = std::any_cast<std::string>(mode["buffer"]);
             mode["buffer"] = buf + s;
        }
    } else {
         std::string buf = std::any_cast<std::string>(mode["buffer"]);
         mode["buffer"] = buf + s;
    }
}
void Parser::parseWaitBlock(bool eof) {
    std::map<std::string, std::any>& mode = mode_stack.back();
    std::string buffer = std::any_cast<std::string>(mode["buffer"]);
    
    // trim buffer
    buffer.erase(0, buffer.find_first_not_of(" \t\n\r"));
    buffer.erase(buffer.find_last_not_of(" \t\n\r") + 1);

    if (!buffer.empty()) {
        std::vector<std::any> final_args;
        if (mode.count("run_args")) {
             std::any ra = mode["run_args"];
             if (ra.type() == typeid(std::vector<std::any>)) final_args = std::any_cast<std::vector<std::any>>(ra);
             else final_args.push_back(ra);
        }

        std::shared_ptr<Variable> func_var = std::any_cast<std::shared_ptr<Variable>>(mode["func"]);
        
        int block_idx = -1;
        if(func_var->children.count("__block_arg_index")) {
            block_idx = std::any_cast<int>(func_var->children["__block_arg_index"]->value);
        }
        
        if (block_idx != -1) {
            try {
                auto lambda_var = this->findVariable(buffer);
                // Insert lambda into args at block_idx
                if(static_cast<size_t>(block_idx) >= final_args.size()) {
                     final_args.resize(block_idx + 1);
                     final_args[block_idx] = lambda_var->value; // pass the function/any
                } else {
                     final_args.insert(final_args.begin() + block_idx, lambda_var->value);
                }
            } catch(...) {}
        }
        
        mode_stack.pop_back();
        func_var->call(final_args);

        if (!eof) this->parseChar();
        return;
    }

    if (!eof && isspace(char_obj->string_val[0])) return;

    if (!eof && char_obj->string_val == "{") {
        std::map<std::string, std::any> next_mode;
        next_mode["type"] = std::string("BLOCK");
        next_mode["buffer"] = std::string("");
        next_mode["nesting"] = 1;
        mode_stack.push_back(next_mode);
        return;
    }

    mode_stack.pop_back();
    std::shared_ptr<Variable> func_var = std::any_cast<std::shared_ptr<Variable>>(mode["func"]);
    std::vector<std::any> final_args;
    if (mode.count("run_args")) {
         std::any ra = mode["run_args"];
         if (ra.type() == typeid(std::vector<std::any>)) final_args = std::any_cast<std::vector<std::any>>(ra);
         else final_args.push_back(ra);
    }
    func_var->call(final_args);

    if (!eof) this->parseChar();
}
void Parser::parseReturn() {
    std::string s = char_obj->string_val;
    if (s == "\n") {
         std::string buf = std::any_cast<std::string>(mode_stack.back()["buffer"]);
         mode_stack.pop_back();
         
         // trim
         buf.erase(0, buf.find_first_not_of(" \t"));
         buf.erase(buf.find_last_not_of(" \t") + 1);

         std::any val;
         if (!buf.empty()) {
             if (buf.size() >=2 && (buf.front() == '"' || buf.front() == '\'')) {
                  val = String(buf.substr(1, buf.size()-2));
             } else if (isdigit(buf[0]) || buf[0] == '-') {
                  std::string cmd = "echo \"" + buf + "\" | bc";
                  std::string res = Builtins::systemreturn(cmd);
                  if(!res.empty() && res.back() == '\n') res.pop_back();
                  val = String(res);
             } else {
                  try {
                       val = this->findVariable(buf)->value;
                  } catch(...) {
                       val = String(buf);
                  }
             }
         }
         throw ReturnSignal(val);
    } else {
         std::string buf = std::any_cast<std::string>(mode_stack.back()["buffer"]);
         mode_stack.back()["buffer"] = buf + s;
    }
}
void Parser::defineFunction(std::string name, std::vector<std::string> args, std::string body) {
    std::vector<std::string> clean_args;
    int block_arg_idx = -1;
    for(size_t i=0; i<args.size(); ++i) {
        if(args[i].size() > 1 && args[i].front() == '{' && args[i].back() == '}') {
             if(block_arg_idx != -1) throw std::runtime_error("Multiple block args not supported");
             clean_args.push_back(args[i].substr(1, args[i].size()-2));
             block_arg_idx = static_cast<int>(i);
        } else {
             clean_args.push_back(args[i]);
        }
    }

    auto func_impl = [this, body, clean_args](std::vector<std::any> call_args) -> std::any {
         // Flatten args if single tuple/vector passed? 
         // Python impl checks for tuple.
         // C++ call convention is vector<any>.
         
         Parser func_parser(File("virtual", body));
         // func_parser.pool = this->pool;
         
         for(size_t i=0; i<clean_args.size(); ++i) {
             if(i < call_args.size()) {
                 func_parser.pool[clean_args[i]] = std::make_shared<Variable>(clean_args[i], call_args[i], "arg", std::map<std::string, std::shared_ptr<Variable>>{}, &func_parser);
             } else {
                 // Default to empty string
                 func_parser.pool[clean_args[i]] = std::make_shared<Variable>(clean_args[i], String(""), "arg", std::map<std::string, std::shared_ptr<Variable>>{}, &func_parser);
             }
         }
         
         try {
             ParsedMaterial mat = func_parser.parse();
             mat.run(); // or execute()? Header says execute()
         } catch (const ReturnSignal& sig) {
             return sig.value;
         }
         return std::any();
    };
    
    auto var = std::make_shared<Variable>(name, std::function<std::any(std::vector<std::any>)>(func_impl), "func", std::map<std::string, std::shared_ptr<Variable>>{}, this);
    if(block_arg_idx != -1) {
        var->children["__block_arg_index"] = std::make_shared<Variable>("__block_arg_index", block_arg_idx, "int", std::map<std::string, std::shared_ptr<Variable>>{}, nullptr);
    }
    this->pool[name] = var;
}

}
