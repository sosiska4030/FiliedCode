#include <iostream>
#include <fstream>
#include <cctype>
#include <map>
#include "Lexer.h"
#include "Parser.h"
#include "Codegen.h"


struct BuildResult {
    std::string objFile;
    std::string exeFile;
    bool success;
};

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

int main(int argc, char** argv) {

    if (argc >= 3 && std::string(argv[1]) == "build")
    {
        buildFile(argv[2]);
    }
    else if (argc == 2 && std::string(argv[1]) == "--version") {
        std::cout << "FiliedCode Compiler, version: 0.3\n";
    }
 /*   else if (argc >= 4 && std::string(argv[1]) == "build -o")
    {
        std::ifstream file(argv[3]);

        if (!file.is_open()) {
            std::cout << "ERROR: Can't open file" << std::endl;
            return -1;
        }

        std::string content;
        std::string line;

        while (std::getline(file, line)) {
            content += line + '\n';
        }

        file.close();

        Lexer lexer;
        lexer.scan(content);

        Parser parser(lexer.getTokens());
        auto ast = parser.parseTopLevel();

        Codegen codegen;
        codegen.generateProgram(ast.get());

        std::string outfile = std::string(argv[2]);
        outfile = outfile.substr(0, outfile.find_last_of('.')) + ".o";
        codegen.emitObjectFile(outfile);

        std::cout <<"Build complete: " << outfile << "\n";
    }
    else if (argc >= 3 && std::string(argv[1]) == "run")
    {

    }
    */
    else if (argc >= 3 && std::string(argv[1]) == "build_and_run")
    {
        auto result = buildFile(argv[2]);
        if (result.success) {
            std::cout << "Running: " << result.exeFile<< "\n---\n";
            std::system(("\"" + result.exeFile + "\"").c_str());
        }

    }
    /*
    else if (argc >= 3 && std::string(argv[1]) == "check")
    {

    }
*/

    
    return 0;
}