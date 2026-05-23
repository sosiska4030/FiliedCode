//
// Created by Пользователь on 21.04.2026.
//

#include "Parser.h"
#include <iostream>
#include <map>

std::unique_ptr<ASTNode> Parser::parseTopLevel() {

    auto program = std::make_unique<BlockAST>();
    while (!isAtEnd()) {


        if (check(Tokens::KEYWORD) && curTok().value == "func") {

            program->push_into_statements(parseFunction());
            continue;
        }
        if (check(Tokens::KEYWORD) && curTok().value == "use") {
            while (!isAtEnd() && curTok().value != ";") consume();
            consume();
            continue;
        }
        consume();
    }
    return program;
}

std::unique_ptr<FunctionAST> Parser::parseFunction() {
    consume();

    std::string name = consume().value;

    if (consume().value != "(") std::cerr << "ERROR: Expected: (\n";
    if (consume().value != ")") std::cerr << "ERROR: Expected: )\n";

    if (check(Tokens::OPERATOR) && curTok().value == "->") {
        consume();
        if (check(Tokens::TYPE)) {
            std::string returnType = consume().value;
        }
        else {
            std::cerr << "ERROR: Expected type after arrow\n";
        }
    }

    if (!check(Tokens::PUNCTUATOR) || curTok().value != "{") {
        std::cerr << "ERROR: Expected { but found" << curTok().value << "\n";
        return nullptr;
    }
    consume();
    auto Body = std::make_unique<BlockAST>();

    while (!isAtEnd() && curTok().value != "}") {
        auto stmt = parseStatement();
        if (stmt) {
            Body->push_into_statements(std::move(stmt));
        }
        else {
            consume();
        }
    }

    if (!isAtEnd() && curTok().value == "}") {
        consume();
    }

    return std::make_unique<FunctionAST>(name, std::move(Body));
}

std::unique_ptr<ASTNode> Parser::parseStatement() {
    if ((check(Tokens::IDENTIFIER) && curTok().value == "printf")
        || (check(Tokens::IDENTIFIER) && curTok().value == "println")) {
        return parseCallExpr();
    }

    if (check(Tokens::IDENTIFIER))
    {
        return parseCallExpr();
    }

    if (check(Tokens::KEYWORD) && curTok().value == "variable")
    {
        return parseVariableDeclaration();
    }
    return nullptr;
}

std::unique_ptr<CallExprAST> Parser::parseCallExpr() {
    std::string name = consume().value;
    if (!check(Tokens::PUNCTUATOR) || curTok().value != "(") {
        std::cerr << "ERROR: Expected (";
    }
    consume();
    std::vector<std::unique_ptr<ASTNode>> args;

    while (curTok().value != ")") {
        if (check(Tokens::STRING)) {
            args.push_back(std::make_unique<stringExprAST>(consume().value));
        }
        else if (check(Tokens::NUMBER)) {
            args.push_back(std::make_unique<numberExprAST>(std::stoi(consume().value)));
        }
        else if (check(Tokens::IDENTIFIER)) {
            args.push_back(std::make_unique<VariableRefAST>(consume().value));
        }
        else {
            consume();
        }
    }



    if (curTok().value == ")")
    {
        consume();
    }
    if (check(Tokens::PUNCTUATOR) && curTok().value == ";")
    {
        consume();
    }
    return std::make_unique<CallExprAST>(name, std::move(args));
}

std::unique_ptr<VarDeclareAST> Parser::parseVariableDeclaration() {

     if (check(Tokens::KEYWORD) && curTok().value == "variable")
    {
        consume();
        if (!(check(Tokens::TYPE)))
        {
            std::cerr << "ERROR: Expected type\n";
        }
        std::string type = consume().value;
        if (!check(Tokens::IDENTIFIER)) std::cerr << "ERROR: Expected identifier\n";
        std::string name = curTok().value;
        consume();

        std::unique_ptr<ASTNode> initValue = nullptr;
        if (check(Tokens::OPERATOR) && curTok().value == "==")
        {
            consume();

            if (check(Tokens::NUMBER)) {
                // Я тут задумывал вместо std::unique_ptr<numberExprAST> value объявить, как auto value, но решил что так будет лучше
                std::unique_ptr<numberExprAST> value = std::make_unique<numberExprAST>(std::stoi(consume().value));
                initValue = std::move(value);
            }
            else if (check(Tokens::STRING)) {
                // Здесь аналогично
                std::unique_ptr<stringExprAST> value = std::make_unique<stringExprAST>(consume().value);
                initValue = std::move(value);
            }

        }

        if (curTok().value == ";") consume();

        return std::make_unique<VarDeclareAST>(name, type, std::move(initValue));

    }
     return nullptr;
}

std::unique_ptr<numberExprAST> Parser::parseNumberExpr() {
    consume();
    if (check(Tokens::NUMBER))
    {
        int value = std::stoi(consume().value);

        return std::make_unique<numberExprAST>(value);
    }
	return nullptr;
}


