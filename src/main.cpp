#include <iostream>
#include <fstream>
#include <cctype>
#include <cstdio>
#include <map>
#include "Lexer.h"
#include "Parser.h"
#include "Codegen.h"
#include "Debug.h"


struct BuildResult {
    std::string objFile;
    std::string exeFile;
    bool success;
};

struct BuildOptions {
    bool link = true;
    bool run = false;
    bool deleteAfter = false;
    bool checkOnly = false;
};

BuildResult buildFile(const std::string& filepath);
bool checkFile(const std::string& filepath);

int main(int argc, char** argv) {

    if (argc >= 3 && std::string(argv[1]) == "build")
    {
        int fileArgIdx = 2;
        if (std::string(argv[2]) == "--debug") {
            debugMode = true;
            fileArgIdx = 3;
            if (argc < 4) {
                std::cerr << "ERROR: Expected file name after debug\n";
                return -1;
            }
        }
        buildFile(argv[fileArgIdx]);
    }
    else if (argc == 2 && std::string(argv[1]) == "--version") {
        std::cout << "FiliedCode Compiler, version: 0.5\n";
    }
    else if (argc >= 3 && std::string(argv[1]) == "run_without_file") {
        int fileArgIdx = 2;
        if (std::string(argv[2]) == "--debug") {
            debugMode = true;
            fileArgIdx = 3;
            if (argc < 4) {
                std::cerr << "ERROR: Expected file name after debug\n";
                return -1;
            }
        }
        auto result = buildFile(argv[fileArgIdx]);
        if (result.success) {
            std::cout << "Running: " << result.exeFile << "\n---\n";
            std::system(("\"" + result.exeFile + "\"").c_str());
            std::remove(result.objFile.c_str());
            std::remove(result.exeFile.c_str());
            std::cout << "\n---\nTemporary files deleted\n";
        }
    }
    else if (argc >= 3 && std::string(argv[1]) == "build_and_run")
    {
        int fileArgIdx = 2;
        if (std::string(argv[2]) == "--debug") {
            debugMode = true;
            fileArgIdx = 3;
            if (argc < 4) {
                std::cerr << "ERROR: Expected file name after debug\n";
                return -1;
            }
        }
        auto result = buildFile(argv[fileArgIdx]);
        if (result.success) {
            std::cout << "Running: " << result.exeFile<< "\n---\n";
            std::system(("\"" + result.exeFile + "\"").c_str());
        }

    }
    else if (argc >= 3 && std::string(argv[1]) == "check") {
        checkFile(argv[2]);
    }

    return 0;
}

BuildResult buildFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cout << "ERROR: Can't open file\n";
        return {"", "", false};
    }

    std::string content, line;
    while (std::getline(file, line)) content += line + '\n';
    file.close();

    size_t start = content.find_first_not_of(" \t\n\r\xEF\xBB\xBF");
    if (start != std::string::npos) content = content.substr(start);

    Lexer lexer;
    lexer.scan(content);
    Parser parser(lexer.getTokens());
    auto ast = parser.parseTopLevel();
    Codegen codegen;
    codegen.generateProgram(ast.get());

    std::string outfile = filepath.substr(0, filepath.find_last_of('.')) + ".o";
    codegen.emitObjectFile(outfile);

    std::string exefile = filepath.substr(0, filepath.find_last_of('.')) + ".exe";
    std::string linkCmd = "gcc \"" + outfile + "\" -o \"" + exefile + "\"";

    if (std::system(linkCmd.c_str()) != 0) {
        std::cerr << "Linker error!\n";
        return {"", "", false};
    }

    std::cout << "Build complete: " << exefile << "\n";
    return {outfile, exefile, true};
}

bool checkFile(const std::string &filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cout << "ERROR: Can't open file\n";
        return false;
    }
    std::string content, line;
    while (std::getline(file, line)) content += line + '\n';
    file.close();

    size_t start = content.find_first_not_of(" \t\n\r\xEF\xBB\xBF");
    if (start != std::string::npos) content = content.substr(start);

    Lexer lexer;
    lexer.scan(content);
    Parser parser(lexer.getTokens());
    auto ast = parser.parseTopLevel();
    if (!ast) {
        std::cerr << "Check failed: Parsing errors\n";
        return false;
    }
    Codegen codegen;
    codegen.generateProgram(ast.get());
    std::cout << "Check passed: no errors\n";
    return true;
}
