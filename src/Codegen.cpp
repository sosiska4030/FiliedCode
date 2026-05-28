//
// Created by Пользователь on 17.04.2026.
//

#include "Codegen.h"
#include "Debug.h"
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

    llvm::FunctionType* scanfType = llvm::FunctionType::get(
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

    llvm::Function::Create(
        scanfType,
        llvm::Function::ExternalLinkage,
        "scanf",
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
    if (debugMode) Module->print(llvm::outs(), nullptr);
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
        else if (auto* bin = dynamic_cast<BinaryExprAST*>(stmt.get())) {
            last = codegenBinaryExpr(bin);
        }
        else if (auto* ifNode = dynamic_cast<IfAST*>(stmt.get())) {
            last = codegenIf(ifNode);
        }
    }
    return last;
}

llvm::Value* Codegen::codegenString(stringExprAST *node) {
    return Builder->CreateGlobalString(node->getValue());
}

llvm::Value* Codegen::codegenCall(CallExprAST *node) {

    if (node->getCallee() == "printf" || node->getCallee() == "println") {
        return codegenPrint(node, node->getCallee() == "println");
    }

    if (node->getCallee() == "input") {
        return codegenInput(node);
    }

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


    llvm::Type* varType = getLLVMType(node->getType());
    llvm::AllocaInst* alloca = Builder->CreateAlloca(
       varType,
       nullptr,
       node->getName()
   );

    if (ASTNode* initVal = node->getInitValue()) {
        llvm::Value* val = codegenValue(initVal);

        if (val) {
            llvm::Type* targetType = varType;
            llvm::Type* valType = val->getType();

            if (valType->isDoubleTy() && targetType->isFloatTy()) {
                val = Builder->CreateFPTrunc(val, targetType, "trunc");
            }
            else if (valType->isFloatingPointTy() && targetType->isIntegerTy()) {
                val = Builder->CreateFPToSI(val, targetType, "fptoint");
            }
            else if (valType->isIntegerTy() && targetType->isFloatingPointTy()) {
                val = Builder->CreateSIToFP(val, targetType, "inttofp");
            }
            else if (valType->isIntegerTy() && targetType->isIntegerTy() && valType != targetType) {
                val = Builder->CreateIntCast(val, targetType, true, "intcast");
            }

            Builder->CreateStore(val, alloca);
        }
    }
   /* if (node->getType() == "int") {
        varType = llvm::Type::getInt32Ty(*Context);
    }
    else if (node->getType() == "string") {
        varType = llvm::PointerType::get(*Context, 0);
    }
    else {
        varType = llvm::Type::getInt32Ty(*Context);
    }



    if (ASTNode* initVal = node->getInitValue()) {
        llvm::Value* val = nullptr;

        if (auto* num = dynamic_cast<numberExprAST*>(initVal)) {
            val = llvm::ConstantInt::get(llvm::Type::getInt32Ty(*Context), num->getValue());
        }
        else if (auto* fl = dynamic_cast<floatExprAST*>(initVal)) {
            val = llvm::ConstantFP::get(getLLVMType(node->getType()), fl->getValue());
        }
        else if (auto* str = dynamic_cast<stringExprAST*>(initVal)) {
            val = codegenString(str);
        }
        else if (auto* bin = dynamic_cast<BinaryExprAST*>(initVal)) {
            val = codegenBinaryExpr(bin);
        }

        if (val) {
            Builder->CreateStore(val, alloca);
        }
    }
*/
    NamedValues[node->getName()] = alloca;
    NamedTypes[node->getName()] = node->getType();

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

llvm::Value* Codegen::codegenVarRef(VariableRefAST *node) {
    auto it = NamedValues.find(node->getName());
    if (it == NamedValues.end()) {
        std::cerr << "ERROR: Unknown variable: " << node->getName() << "\n";
        return nullptr;
    }


    auto typeIt = NamedTypes.find(node->getName());
    std::string type = (typeIt != NamedTypes.end()) ? typeIt->second : "int";
    llvm::Type* loadType = getLLVMType(type);
  /*  if (typeIt != NamedTypes.end() && typeIt->second == "string") {
        loadType = llvm::PointerType::get(*Context, 0);
    }
    else {
        loadType = llvm::Type::getInt32Ty(*Context);
    } */

    return Builder->CreateLoad(
        loadType,
        it->second,
        node->getName()
    );
}

llvm::Value* Codegen::codegenPrint(CallExprAST *node, bool newLine) {
    llvm::Function* printfFN = Module->getFunction("printf");

    for (const auto& arg : node->getArgs()) {
        llvm::Value* val = nullptr;
        std::string fmt;

        if (auto* strArg = dynamic_cast<stringExprAST*>(arg.get())) {
            fmt = newLine ? "%s\n" : "%s";
            val = codegenString(strArg);
        }
        else if (auto* numArg = dynamic_cast<numberExprAST*>(arg.get())) {
            fmt = newLine ? "%d\n" : "%d";
            val = llvm::ConstantInt::get(llvm::Type::getInt32Ty(*Context), numArg->getValue());
        }
        else if (auto* refArg = dynamic_cast<VariableRefAST*>(arg.get())) {
            auto typeIt = NamedTypes.find(refArg->getName());
            std::string type = (typeIt != NamedTypes.end()) ? typeIt->second : "int";
            if (type == "string") {
                fmt = newLine ? "%s\n" : "%s";
            }
            else if (type == "float" || type == "double") {
                fmt = newLine ? "%f\n" : "%f";
            }
            else if (type == "char") {
                fmt = newLine ? "%c\n" : "%c";
            }
            else {
                fmt = newLine ? "%d\n" : "%d";
            }
            val = codegenVarRef(refArg);

            if (val && val->getType()->isFloatTy()) {
                val = Builder->CreateFPExt(val, llvm::Type::getDoubleTy(*Context), "fpext");
            }
        }
        else if (auto* binArg = dynamic_cast<BinaryExprAST*>(arg.get())) {
            fmt = newLine ? "%d\n" : "%d";
            val = codegenBinaryExpr(binArg);
        }
        else if (auto* flArg = dynamic_cast<floatExprAST*>(arg.get())) {
            fmt = newLine ? "%f\n" : "%f";
            val = llvm::ConstantFP::get(llvm::Type::getDoubleTy(*Context), flArg->getValue());
        }

        if (val) {
            llvm::Value* fmtStr = Builder->CreateGlobalString(fmt, "fmt");
            Builder->CreateCall(printfFN, {fmtStr, val}, "printtpm");
        }

    }
    return nullptr;
}

llvm::Value* Codegen::codegenBinaryExpr(BinaryExprAST* node) {
    llvm::Value* left = nullptr;
    llvm::Value* right = nullptr;

    if (auto* num = dynamic_cast<numberExprAST*>(node->getLHS())) {
        left = llvm::ConstantInt::get(llvm::Type::getInt32Ty(*Context), num->getValue());
    }
    else if (auto* ref = dynamic_cast<VariableRefAST*>(node->getLHS())) {
        left = codegenVarRef(ref);
    }
    else if (auto* bin = dynamic_cast<BinaryExprAST*>(node->getLHS())) {
        left = codegenBinaryExpr(bin);
    }

    if (auto* num = dynamic_cast<numberExprAST*>(node->getRHS())) {
        right = llvm::ConstantInt::get(llvm::Type::getInt32Ty(*Context), num->getValue());
    }
    else if (auto* ref = dynamic_cast<VariableRefAST*>(node->getRHS())) {
        right = codegenVarRef(ref);
    }
    else if (auto* bin = dynamic_cast<BinaryExprAST*>(node->getRHS())) {
        right = codegenBinaryExpr(bin);
    }

    if (!left || !right) {
        return nullptr;
    }

    switch (node->getOp()) {
        case '+': return Builder->CreateAdd(left, right, "addtmp");
        case '-': return Builder->CreateSub(left, right, "subtmp");
        case '*': return Builder->CreateMul(left, right, "multmp");
        case '/': return Builder->CreateSDiv(left, right, "divtmp");
        default:
            std::cerr << "ERROR: Unknown operator!\n";
            return nullptr;

    }
}

llvm::Value* Codegen::codegenComparison(ComparisonAST* node) {
    llvm::Value* left = codegenValue(node->getLHS());
    llvm::Value* right = codegenValue(node->getRHS());

    if (!left || !right) return nullptr;

    const std::string op = node->getOp();

    if (op == "<")  return Builder->CreateICmpSLT(left, right, "cmptmp");
    if (op == ">")  return Builder->CreateICmpSGT(left, right, "cmptmp");
    if (op == "<=") return Builder->CreateICmpSLE(left, right, "cmptmp");
    if (op == ">=") return Builder->CreateICmpSGE(left, right, "cmptmp");
    if (op == "===") return Builder->CreateICmpEQ(left, right, "cmptmp");
    if (op == "!=") return Builder->CreateICmpNE(left, right, "cmptmp");

    std::cerr << "ERROR: Unknown comparison operator: " << op << "\n";
    return nullptr;
}


llvm::Value* Codegen::codegenValue(ASTNode *node) {
    if (auto* num = dynamic_cast<numberExprAST*>(node)) {
        return llvm::ConstantInt::get(llvm::Type::getInt32Ty(*Context), num->getValue());
    }
    if (auto* str = dynamic_cast<stringExprAST*>(node)) {
        return codegenString(str);
    }
    if (auto* ref = dynamic_cast<VariableRefAST*>(node)) {
        return codegenVarRef(ref);
    }
    if (auto* bin = dynamic_cast<BinaryExprAST*>(node)) {
        return codegenBinaryExpr(bin);
    }
    if (auto* cmp = dynamic_cast<ComparisonAST*>(node)) {
        return codegenComparison(cmp);
    }
    if (auto* fl = dynamic_cast<floatExprAST*>(node)) {
        return llvm::ConstantFP::get(llvm::Type::getDoubleTy(*Context), fl->getValue());
    }
    return nullptr;
}

llvm::Value* Codegen::codegenIf(IfAST* node) {
    llvm::Value* condValue = codegenValue(node->getCondition());

    if (!condValue) return nullptr;

    llvm::Function* function = Builder->GetInsertBlock()->getParent();

    llvm::BasicBlock* thenBB = llvm::BasicBlock::Create(*Context, "then", function);
    llvm::BasicBlock* elseBB = llvm::BasicBlock::Create(*Context, "else", function);
    llvm::BasicBlock* mergeBB = llvm::BasicBlock::Create(*Context, "merge", function);

    Builder->CreateCondBr(condValue, thenBB, elseBB);

    Builder->SetInsertPoint(thenBB);
    codegenBlock(dynamic_cast<BlockAST*>(node->getThen()));
    Builder->CreateBr(mergeBB);

    Builder->SetInsertPoint(elseBB);
    if (node->getElse()) {
        codegenBlock(dynamic_cast<BlockAST*>(node->getElse()));
    }
    Builder->CreateBr(mergeBB);

    Builder->SetInsertPoint(mergeBB);
    return nullptr;
}

llvm::Value* Codegen::codegenInput(CallExprAST* node) {
    llvm::Function* scanfFn = Module->getFunction("scanf");

    for (const auto& arg : node->getArgs()) {
        auto* ref = dynamic_cast<VariableRefAST*>(arg.get());
        if (!ref) {
            std::cerr << "ERROR: input() expects a variable\n";
            continue;
        }

        auto it = NamedValues.find(ref->getName());
        if (it == NamedValues.end()) {
            std::cerr << "ERROR: Unknown variable in input: " << ref->getName() << "\n";
            continue;
        }

        std::string fmt;
        auto typeIt = NamedTypes.find(ref->getName());
        std::string type = (typeIt != NamedTypes.end()) ? typeIt->second : "int";

        if (type == "string") {
            std::cerr << "ERROR: Sorry, strings isn't supported for input yet =( \n";
            continue;
        }

        if (type == "int") fmt = "%d";
        else if (type == "double") fmt = "%lf";
        else if (type == "float") fmt = "%f";
        else if (type == "char") fmt = "%c";
        else fmt = "%d";

        llvm::Value* fmtStr = Builder->CreateGlobalString(fmt, "scanf_fmt");
        Builder->CreateCall(scanfFn, {fmtStr, it->second}, "scantmp");
    }
    return nullptr;
}

llvm::Type* Codegen::getLLVMType(const std::string& type) {
    if (type == "int")    return llvm::Type::getInt32Ty(*Context);
    if (type == "float")  return llvm::Type::getFloatTy(*Context);
    if (type == "double") return llvm::Type::getDoubleTy(*Context);
    if (type == "char")   return llvm::Type::getInt8Ty(*Context);
    if (type == "string") return llvm::PointerType::get(*Context, 0);
    return llvm::Type::getInt32Ty(*Context);
}




