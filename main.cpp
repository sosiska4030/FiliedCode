#include <iostream>
#include <fstream>
#include <cctype>
#include <map>
#include "Lexer.h"
#include "Parser.h"
#include "Codegen.h"


int main(int argc, char** argv) {

    if (argc >= 3 && std::string(argv[1]) == "build")
    {

        std::ifstream file(argv[2]);

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
        size_t start = content.find_first_not_of(" \t\n\r\xEF\xBB\xBF");
        if (start != std::string::npos) {
            content = content.substr(start);
        }
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
        std::string exefile = outfile.substr(0, outfile.find_last_of('.')) + ".exe";
        std::string linkCmd = "gcc \"" + outfile + "\" -o \"" + exefile + "\"";
        int result = std::system(linkCmd.c_str());
        if (result == 0) {
            std::cout << "Linked: " << exefile << std::endl;
        } else {
            std::cerr << "Linker error!\n";
        }
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
    else if (argc >= 3 && std::string(argv[1]) == "build_and_run")
    {

    }
    else if (argc >= 3 && std::string(argv[1]) == "check")
    {

    }
*/

    
    return 0;
}