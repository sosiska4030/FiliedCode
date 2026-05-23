//
// Created by Пользователь on 17.04.2026.
//
#pragma once

#ifndef FCC_CODEGEN_H
#define FCC_CODEGEN_H


#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "Parser.h"
#include "Lexer.h"
#include <memory>


class Codegen {

public:
    std::unique_ptr<llvm::LLVMContext> Context;
    std::unique_ptr<llvm::Module> Module;
    std::unique_ptr<llvm::IRBuilder<>> Builder;

    std::map<std::string, llvm::Value*> NamedValues;
    std::map<std::string, std::string> NamedTypes;

    Codegen();

    void generateProgram(ASTNode* root);

    llvm::Value* codegenFunction(FunctionAST* node);
    llvm::Value* codegenBlock(BlockAST* node);
    llvm::Value* codegenCall(CallExprAST* node);
    llvm::Value* codegenString(stringExprAST* node);
    llvm::Value* codegenVarDecl(VarDeclareAST* node);
    llvm::Value* codegenVarRef(VariableRefAST* node);
    llvm::Value* codegenPrint(CallExprAST* node, bool newLine);

    void declareExternalFunctions();
    void emitObjectFile(const std::string& filename);

};


#endif //FCC_CODEGEN_H