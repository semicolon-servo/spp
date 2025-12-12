#include "file.hpp"
#include <iostream>
#include <sstream>

namespace servo {

namespace fs = std::filesystem;

File::File(std::string path, bool no_read) {
    // resolve absolute path
    if (!path.empty()) {
        try {
            this->path = fs::absolute(path).string();
        } catch (...) {
            this->path = path;
        }
    } else {
        this->path = "";
    }
    
    if (!no_read && !this->path.empty()) {
        read();
    }
}

File::File(std::string path, std::string content) : path(path), content(content), content_loaded(true) {}

std::string File::read() {
    return Safe::call([this]() -> std::string {
        if (this->content_loaded) return this->content;
        if (this->path.empty()) {
            throw std::runtime_error("read() while path still not provided to File object.");
        }
        std::ifstream f(this->path);
        if (!f.is_open()) return ""; // Or throw?
        std::stringstream buffer;
        buffer << f.rdbuf();
        this->content = buffer.str() + " "; // Add space as per python
        this->content_loaded = true;
        return this->content;
    }, "servo.internal.public.file");
}

void File::write(std::string content, std::string mode) {
    Safe::call([this, content, mode]() {
        if (this->path.empty()) {
            throw std::runtime_error("write() while path still not provided to File object.");
        }
        std::ofstream f;
        if (mode == "a") f.open(this->path, std::ios::app);
        else f.open(this->path);
        
        f << content;
        this->content = content;
        this->content_loaded = true;
    }, "servo.internal.public.file");
}

std::string File::getContent() {
    return Safe::call([this]() { return this->content; }, "servo.internal.public.file");
}

std::string File::getPath() {
    return Safe::call([this]() { return this->path; }, "servo.internal.public.file");
}

std::string File::getExtension() {
    return Safe::call([this]() {
         size_t dot = this->path.find_last_of(".");
         if (dot != std::string::npos) return this->path.substr(dot + 1);
         return std::string("");
    }, "servo.internal.public.file");
}

std::string File::getBaseName() {
    return Safe::call([this]() {
        return fs::path(this->path).filename().string();
    }, "servo.internal.public.file");
}

std::vector<std::string> File::getParts() {
    return Safe::call([this]() {
        std::vector<std::string> parts;
        for (const auto& part : fs::path(this->path)) {
            parts.push_back(part.string());
        }
        return parts;
    }, "servo.internal.public.file");
}

std::string File::getParent() {
    return Safe::call([this]() {
        return fs::path(this->path).parent_path().string();
    }, "servo.internal.public.file");
}

std::string File::getChild(std::vector<std::string> tree) {
    return Safe::call([this, tree]() {
        fs::path p = this->path;
        for (const auto& part : tree) p /= part;
        return p.string();
    }, "servo.internal.public.file");
}

std::string File::getType() {
    return Safe::call([this]() -> std::string {
        if (fs::is_directory(this->path)) return "dir";
        if (fs::is_regular_file(this->path)) return "file";
        return "";
    }, "servo.internal.public.file");
}

bool File::getExists() {
    return Safe::call([this]() {
        return !getType().empty();
    }, "servo.internal.public.file");
}

bool File::deleteFile() {
    return Safe::call([this]() {
        std::string t = getType();
        if (t == "file") {
            fs::remove(this->path);
            return true;
        } else if (t == "dir") {
            fs::remove_all(this->path);
            return true;
        }
        return false;
    }, "servo.internal.public.file");
}

bool File::createDirectory() {
    return Safe::call([this]() {
        if (!getExists()) {
            fs::create_directories(this->path);
            return true;
        }
        return false;
    }, "servo.internal.public.file");
}

}
