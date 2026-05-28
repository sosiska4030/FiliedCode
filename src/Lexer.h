//
// Created by Пользователь on 17.04.2026.
//

#ifndef FCC_LEXER_H
#define FCC_LEXER_H

#include <fstream>
#include <iostream>
#include <cctype>
#include <vector>
#include <map>
#include <unordered_map>


enum class Tokens {
    IDENTIFIER,
    NUMBER,
    FLOAT,
    BOOL,
    STRING,
    KEYWORD,
    OPERATOR,
    PUNCTUATOR,
    UNKNOWN,
    END_OF_FILE,
    TYPE

};

struct Token {
    Tokens tokentype;
    std::string value;
    Token(Tokens tokentype, std::string value) : tokentype(tokentype), value(value) {

    }
};

class Lexer {

private:
    std::vector<Token> tokens;
    std::unordered_map<std::string, Tokens> keywords;



public:
    Lexer();
    ~Lexer();
    void scan(const std::string& source);
    void addToken(Tokens type, std::string value) {
        tokens.push_back(Token(type, value));
    }
    const std::vector<Token>& getTokens() const { return tokens; }


};




#endif //FCC_LEXER_H