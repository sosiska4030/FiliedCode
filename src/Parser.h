//
// Created by Пользователь on 21.04.2026.
//
#pragma once

#ifndef FCC_PARSER_H
#define FCC_PARSER_H

#include <vector>
#include <string>
#include <memory>
#include <map>
#include "llvm/IR/Value.h"
#include "Lexer.h"

class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual llvm::Value* codegen() = 0;
};

class CallExprAST : public ASTNode {
    std::string Callee;
    std::vector<std::unique_ptr<ASTNode>> Args;
public:
    CallExprAST(const std::string& Callee, std::vector<std::unique_ptr<ASTNode>> Args) :
        Callee(Callee), Args(std::move(Args)) {

    }

    const std::string& getCallee() const { return Callee; }
    const std::vector<std::unique_ptr<ASTNode>>& getArgs() const { return Args; }

    llvm::Value* codegen() override { return nullptr; }
};


class FunctionAST : public ASTNode {
    std::string Name;
    std::unique_ptr<ASTNode> Body;
public:
    FunctionAST(const std::string& Name, std::unique_ptr<ASTNode> Body) :
        Name(Name), Body(std::move(Body)) {

    }

    const std::string& getName() const { return Name; }
    ASTNode* getBody() const { return Body.get(); }
    llvm::Value* codegen() override { return nullptr; }
};

class VarDeclareAST : public ASTNode {
    std::string Name;
    std::string Type;
    std::unique_ptr<ASTNode> InitValue;

public:
    VarDeclareAST(const std::string& Name, const std::string& Type, std::unique_ptr<ASTNode> InitValue) :
    Name(Name), Type(Type), InitValue(std::move(InitValue))
    {

    }
    const std::string& getName() const { return Name; }
    const std::string& getType() const { return Type; }
    ASTNode* getInitValue() const { return InitValue.get(); }
    llvm::Value* codegen() override { return nullptr; }
};

class BlockAST : public ASTNode {
    std::vector<std::unique_ptr<ASTNode>> statements;

public:

    void push_into_statements(std::unique_ptr<ASTNode> pushed) {
        if (pushed) {
            statements.push_back(std::move(pushed));
        }

    }
    const std::vector<std::unique_ptr<ASTNode>>& getStatements() const { return statements; }
    llvm::Value* codegen() override { return nullptr; }
};

class stringExprAST : public ASTNode {

    std::string value;

public:
    stringExprAST(std::string value) : value(value)
    {

    }

    const std::string& getValue() const { return value; }
    llvm::Value* codegen() override { return nullptr; }
};

class numberExprAST : public ASTNode
{
    int value;

public:

    numberExprAST(int value) : value(value)
    {

    }
    int getValue() const { return value; }
    llvm::Value* codegen() override { return nullptr; }
};

class floatExprAST : public ASTNode {

    double value;
public:
    floatExprAST(double value) : value(value) {}
    double getValue() const { return value; }
    llvm::Value* codegen() override { return nullptr; }
};

class boolExprAST : public ASTNode {
    bool value;
public:
    boolExprAST(bool value) : value(value) {}
    bool getValue() const { return value; }
    llvm::Value* codegen() override { return nullptr; }
};

class Parser {

    std::vector<Token> tokens;
    size_t pos = 0;

    Token curTok() {return tokens[pos]; }
    Token consume() {return tokens[pos++]; }

    bool check(Tokens type) {
        if (isAtEnd()) {
            return false;
        }
        return curTok().tokentype == type;
    }

    bool isAtEnd() {return pos >= tokens.size() || curTok().tokentype == Tokens::END_OF_FILE; }




public:
    Parser(const std::vector<Token>& tokens) : tokens(tokens) { }

    std::unique_ptr<ASTNode> parseTopLevel();
    std::unique_ptr<FunctionAST> parseFunction();
    std::unique_ptr<ASTNode> parseStatement();
    std::unique_ptr<CallExprAST> parseCallExpr();
    std::unique_ptr<VarDeclareAST> parseVariableDeclaration();
    std::unique_ptr<numberExprAST> parseNumberExpr();
    std::unique_ptr<ASTNode> parseExpression();
    std::unique_ptr<ASTNode> parsePrimary();
    std::unique_ptr<ASTNode> parseIf();
    std::unique_ptr<ASTNode> parseCondition();


};





class BinaryExprAST : public ASTNode
{
    char Op;
    std::unique_ptr<ASTNode> LHS;
    std::unique_ptr<ASTNode> RHS;

public:
    BinaryExprAST(char Op, std::unique_ptr<ASTNode> LHS, std::unique_ptr<ASTNode> RHS)
            : Op(Op), LHS(std::move(LHS)), RHS(std::move(RHS)) {}

    char getOp() const { return Op; }
    ASTNode* getLHS() const { return LHS.get(); }
    ASTNode* getRHS() const { return RHS.get(); }
    llvm::Value* codegen() override { return nullptr; }


};

class VariableRefAST : public ASTNode {
    std::string Name;

public:
    VariableRefAST(const std::string& Name) : Name(Name) {

    }
    const std::string& getName() { return Name; }
    llvm::Value* codegen() override { return nullptr; }
};


class IfAST : public ASTNode{
private:
    std::unique_ptr<ASTNode> condition;
    std::unique_ptr<ASTNode> then;
    std::unique_ptr<ASTNode> else_;



public:
    IfAST(std::unique_ptr<ASTNode> condition, std::unique_ptr<ASTNode> then, std::unique_ptr<ASTNode> else_) : condition(std::move(condition)), then(std::move(then)),
    else_(std::move(else_)) {

    }

    ASTNode* getCondition() { return condition.get(); }
    ASTNode* getThen() { return then.get(); }
    ASTNode* getElse() { return else_.get(); }
    llvm::Value* codegen() override { return nullptr; }
};

class ComparisonAST : public ASTNode {
    std::string Op;
    std::unique_ptr<ASTNode> LHS;
    std::unique_ptr<ASTNode> RHS;

public:
    ComparisonAST(const std::string& Op, std::unique_ptr<ASTNode> LHS, std::unique_ptr<ASTNode> RHS) : Op(Op), LHS(std::move(LHS)), RHS(std::move(RHS)) {

    }

    const std::string& getOp() const { return Op; }
    ASTNode* getLHS() const { return LHS.get(); }
    ASTNode* getRHS() const { return RHS.get(); }
    llvm::Value* codegen() override { return nullptr; }
};

#endif //FCC_PARSER_H