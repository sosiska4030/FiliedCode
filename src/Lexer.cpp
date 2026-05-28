//
// Created by Пользователь on 17.04.2026.
//

#include "Lexer.h"
#include "Codegen.h"
#include <fstream>
#include <cctype>
#include <string>
#include <map>

#include "Debug.h"

Lexer::Lexer() {
    keywords["func"] = Tokens::KEYWORD;
    keywords["variable"] = Tokens::KEYWORD;
    keywords["array"] = Tokens::KEYWORD;
    keywords["dynamic_array"] = Tokens::KEYWORD;
    keywords["const"] = Tokens::KEYWORD;
    keywords["pointer"] = Tokens::KEYWORD;
    keywords["referral"] = Tokens::KEYWORD;
    keywords["object"] = Tokens::KEYWORD;
    keywords["class"] = Tokens::KEYWORD;
    keywords["use"] = Tokens::KEYWORD;
    keywords["namespace"] = Tokens::KEYWORD;
    keywords["use_namespace"] = Tokens::KEYWORD;
    keywords["switch"] = Tokens::KEYWORD;
    keywords["case"] = Tokens::KEYWORD;
    keywords["break"] = Tokens::KEYWORD;
    keywords["if"] = Tokens::KEYWORD;
    keywords["else_if"] = Tokens::KEYWORD;
    keywords["else"] = Tokens::KEYWORD;
    keywords["for"] = Tokens::KEYWORD;
    keywords["foreach"] = Tokens::KEYWORD;
    keywords["in"] = Tokens::KEYWORD;
    keywords["while"] = Tokens::KEYWORD;
    keywords["extends"] = Tokens::KEYWORD;
    keywords["any"] = Tokens::KEYWORD;
    keywords["create"] = Tokens::KEYWORD;
    keywords["destroy"] = Tokens::KEYWORD;
    keywords["public"] = Tokens::KEYWORD;
    keywords["private"] = Tokens::KEYWORD;
    keywords["protected"] = Tokens::KEYWORD;
    keywords["virtual"] = Tokens::KEYWORD;
    keywords["int"] = Tokens::TYPE;
    keywords["string"] = Tokens::TYPE;
    keywords["double"] = Tokens::TYPE;
    keywords["float"] = Tokens::TYPE;
    keywords["char"] = Tokens::TYPE;
    keywords["void"] = Tokens::TYPE;
    keywords["true"] = Tokens::KEYWORD;
    keywords["false"] = Tokens::KEYWORD;

}

Lexer::~Lexer() {

}



void Lexer::scan(const std::string& source) {
    size_t cursor = 0;
    while (cursor < source.length()) {
        char current = source[cursor];


        if (std::isspace(current)) {
            cursor++;
            continue;
        }

        if (std::isalpha(current)) {
            std::string identifer;
            while (isalnum(source[cursor])) {
                identifer += source[cursor++];
            }


            auto it = keywords.find(identifer);

            if (it != keywords.end()) {
                addToken(it->second, identifer);
                if (debugMode) std::cout << "Keyword found: " << identifer << std::endl;
            }
            else {
                addToken(Tokens::IDENTIFIER, identifer);
                if (debugMode) std::cout << "Identifer found: " << identifer << std::endl;
            }

            continue;
        }


        if (std::isdigit(current)) {
            std::string number;
            bool is_float = false;
            while (isdigit(source[cursor]) || source[cursor] == '.') {
                if (source[cursor] == '.') {
                    is_float = true;
                }
                number.push_back(source[cursor]);
                cursor++;
            }

            if (is_float) {
                addToken(Tokens::FLOAT, number);
                if (debugMode) std::cout << "Float found: " << number << std::endl;

            } else {
                if (debugMode) std::cout << "Number found: " << number << std::endl;
                addToken(Tokens::NUMBER, number);
            }
            continue;
        }

        if (current == '-' && cursor + 1 < source.length() && source[cursor + 1] == '>') {
            addToken(Tokens::OPERATOR, "->");
            if (debugMode) std::cout << "Operator found: ->" << std::endl;
            cursor += 2;
            continue;
        }

        if (current == '/' && cursor + 1 < source.length() && source[cursor + 1] == '/') {
            while (cursor < source.length() && source[cursor] != '\n') {
                cursor++;
            }
            continue;
        }


        if (current == '+' || current == '-')
        {

            std::string fc_operator;
            while (source[cursor] == '+' || source[cursor] == '-') {
                fc_operator += (source[cursor]);
                cursor++;
            }
            addToken(Tokens::OPERATOR, fc_operator);
            if (debugMode) std::cout << "Operator found: " << fc_operator << std::endl;
            continue;

        }

        if (current == '!') {
            std::string no;
            if (cursor + 1 < source.length() && source[cursor + 1] == '=') {
                no = "!=";
            }
            addToken(Tokens::OPERATOR, no);
            if (debugMode) std::cout << "Operator found: " << no << std::endl;
            continue;
        }

        if (current == '&') {
            std::string op = "&";
            if (cursor + 1 < source.length() && source[cursor + 1] == '&') {
                op = "&&";
                cursor++;
            }
            addToken(Tokens::OPERATOR, op);
            if (debugMode) std::cout << "Operator found: " << op << std::endl;
            cursor++;
            continue;
        }

        if (current == '|') {
            std::string fc_or;
            if (cursor + 1 < source.length() && source[cursor + 1] == '|') {
                addToken(Tokens::OPERATOR, "||");
                if (debugMode) std::cout << "Operator found: ||" << std::endl;
                cursor += 2;
            }
            else {
                addToken(Tokens::UNKNOWN, "|");
                cursor++;
            }
            continue;
        }

        if (current == '=') {
            std::string op = "=";

            if (cursor + 1 < source.length() && source[cursor + 1] == '=') {
                op = "==";
                cursor++;

                if (cursor + 1 < source.length() && source[cursor + 1] == '=') {
                    op = "===";
                    cursor++;
                }
            }
            if (debugMode) std::cout << "Found: " << op << std::endl;
            addToken(Tokens::OPERATOR, op);
            cursor++;
            continue;
        }

        if (current == '<') {
            std::string op = "<";
            if (cursor + 1 < source.length() && source[cursor + 1] == '=') {
                op = "<=";
                cursor++;
            }
            addToken(Tokens::OPERATOR, op);
            if (debugMode) std::cout << "Operator found: " << op << std::endl;
            cursor++;
            continue;
        }

        if (current == '>') {
            std::string op = ">";
            if (cursor + 1 < source.length() && source[cursor + 1] == '=') {
                op = ">=";
                cursor++;
            }
            addToken(Tokens::OPERATOR, op);
            if (debugMode) std::cout << "Operator found: " << op << std::endl;
            cursor++;
            continue;
        }


        if (current == '"') {
            std::string str;
            cursor++;
            do {
                if (cursor + 1 < source.length() && source[cursor] == '"' && source[cursor - 1] == '\\') {
                    str += source[cursor + 1];
                    cursor++;
                    continue;
                }
                str += source[cursor];
                cursor++;
            } while (cursor < source.length() && source[cursor] != '"');
            if (debugMode) std::cout << "String found: " << str << std::endl;
            addToken(Tokens::STRING, str);
            cursor++;
            continue;
        }

        if (current == '(' || current == ')' || current == '{' || current == '}'
            || current == '[' || current == ']' || current == ';' || current == ',' || current == '.'
            || current == ':') {

            std::string pun(1, current);
            addToken(Tokens::PUNCTUATOR, pun);
            if (debugMode) std::cout << "Punctuator found: " << pun << std::endl;
            cursor++;
            continue;

        }



        cursor++;
    }




}


