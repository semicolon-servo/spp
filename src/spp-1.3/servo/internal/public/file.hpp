#ifndef SERVO_INTERNAL_PUBLIC_FILE_HPP
#define SERVO_INTERNAL_PUBLIC_FILE_HPP

#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include "safe.hpp"

namespace servo {

class File {
public:
    std::string path;
    std::string content;
    bool content_loaded = false;

    File(std::string path, bool no_read = false);
    File(std::string path, std::string content);

    std::string read();
    void write(std::string content, std::string mode = "w");
    std::string getContent();
    std::string getPath();
    std::string getExtension();
    std::string getBaseName();
    std::vector<std::string> getParts();
    std::string getParent();
    std::string getChild(std::vector<std::string> tree);
    std::string getType();
    bool getExists();
    bool deleteFile();
    bool createDirectory();
};

}

#endif
