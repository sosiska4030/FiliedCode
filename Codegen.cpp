//
// Created by Пользователь on 17.04.2026.
//

#include "Codegen.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/TargetParser/Host.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/IR/LegacyPassManager.h"


Codegen::Codegen() {
    Context = std::make_unique<llvm::LLVMContext>();
    Module = std::make_unique<llvm::Module>("FiliedCode", *Context);
    Builder = std::make_unique<llvm::IRBuilder<>>(*Context);
}

void Codegen::declareExternalFunctions()
{
    llvm::Type* charPtr = llvm::PointerType::get(*Context, 0);

    llvm::FunctionType* printfType = llvm::FunctionType::get(
        llvm::Type::getInt32Ty(*Context),
        { charPtr },
        true
    );

    llvm::Function::Create(
        printfType,
        llvm::Function::ExternalLinkage,
        "printf",
        *Module
    );
}

void Codegen::generateProgram(ASTNode* root) {
    BlockAST* block = dynamic_cast<BlockAST*>(root);
    if(!block) return;

    declareExternalFunctions();

    for (const auto& stmt : block->getStatements()) {
        if (auto* func = dynamic_cast<FunctionAST*>(stmt.get())) {
            codegenFunction(func);
        }
    }
    Module->print(llvm::outs(), nullptr);
}

llvm::Value *Codegen::codegenFunction(FunctionAST *node) {
    llvm::FunctionType* funcType = llvm::FunctionType::get(
        llvm::Type::getVoidTy(*Context),
        false
    );

    llvm::Function* func = llvm::Function::Create(
        funcType,
        llvm::Function::ExternalLinkage,
        node->getName(),
        *Module
        );

    llvm::BasicBlock* entry = llvm::BasicBlock::Create(
        *Context,
        "entry",
        func
    );

    Builder->SetInsertPoint(entry);

    codegenBlock(dynamic_cast<BlockAST*>(node->getBody()));
    Builder->CreateRetVoid();

    return func;
}

llvm::Value* Codegen::codegenBlock(BlockAST *node) {
    if (!node) return nullptr;

    llvm::Value* last = nullptr;

    for (const auto& stmt : node->getStatements()) {
        if (auto* call = dynamic_cast<CallExprAST*>(stmt.get())) {
            last = codegenCall(call);
        }
        else if (auto* var = dynamic_cast<VarDeclareAST*>(stmt.get())) {
            last = codegenVarDecl(var);
        }
    }
    return last;
}

llvm::Value* Codegen::codegenString(stringExprAST *node) {
    return Builder->CreateGlobalString(node->getValue());
}

llvm::Value* Codegen::codegenCall(CallExprAST *node) {

    llvm::Function* callee = Module->getFunction(node->getCallee());

    if (!callee) {
        std::cerr << "ERROR: Unknown function" << node->getCallee() << "\n";
        return nullptr;
    }

    std::vector<llvm::Value*> args;

    for (const auto& arg : node->getArgs()) {
        if (auto* strArg = dynamic_cast<stringExprAST*>(arg.get())) {
            args.push_back(codegenString(strArg));
        }
    }

    return Builder->CreateCall(callee, args, "calltmp");

}

llvm::Value* Codegen::codegenVarDecl(VarDeclareAST *node) {

    llvm::AllocaInst* alloca = Builder->CreateAlloca(
        llvm::Type::getInt32Ty(*Context),
        nullptr,
        node->getName()
    );

    if (ASTNode* initVal = node->getInitValue()) {

    }

    NamedValues[node->getName()] = alloca;

    return alloca;
}

void Codegen::emitObjectFile(const std::string &filename) {
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();

    llvm::Triple targetTriple(llvm::sys::getDefaultTargetTriple());
    Module->setTargetTriple(targetTriple);

    std::string error;

    auto* target = llvm::TargetRegistry::lookupTarget(targetTriple, error);
    if (!target) {
        llvm::errs() << "ERROR: " << error << "\n";
    }

    auto* targetMachine = target->createTargetMachine(
        targetTriple, "generic", "",
        llvm::TargetOptions{},
        llvm::Reloc::PIC_
    );

    Module->setDataLayout(targetMachine->createDataLayout());

    std::error_code ec;
    llvm::raw_fd_ostream dest(filename, ec, llvm::sys::fs::OF_None);
    if (ec) {
        llvm::errs() << "ERROR: Couldn't open file" << ec.message() << "\n";
        return;
    }

    llvm::legacy::PassManager pass;
    targetMachine->addPassesToEmitFile(
        pass, dest, nullptr, llvm::CodeGenFileType::ObjectFile
    );

    pass.run(*Module);
    dest.flush();

    llvm::outs() << "Object file written to: " << filename << "\n";

}



