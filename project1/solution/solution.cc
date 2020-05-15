#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <queue>

#include "IR.h"
#include "IRMutator.h"
#include "IRVisitor.h"
#include "IRPrinter.h"
#include "type.h"

#include "lex_yacc.h"//include头文件以使用parser
#include "buildTable.h"

using namespace Boost::Internal;

/*
 * 建立IRTree
 */
Json caseInfo;                          // case信息
Type index_type = Type::int_scalar(32);     
Type data_type;
std::vector<Stmt> mainStmt;             // 最外层Stmt
std::vector<Expr> inVars;               // 输入变量
std::vector<Expr> outVars;              // 输出变量
std::vector<std::string> leftindex;     // 辅助, 记录等号左边出现的index
std::vector<Expr> argsList;             // 一个Tref的下标列表

// 获取临时变量
int tmpNum = 0;
std::string getTemp() {
    std::string tmp = "tmp" + std::to_string(tmpNum);
    tmpNum += 1;
    return tmp;
}

// 查找一个vector<string>中是否有某个字符串, 合并vector<string>
bool inVector(std::vector<std::string> &list, std::string s) {
    for (std::string i: list) {
        if (strcmp(i.c_str(), s.c_str()) == 0) return true;
    }
    return false;
}

// 根据节点类型, 选择处理函数
Expr deal_expr(TreeNode *node, Type type, std::vector<std::string>& indexList);

// Binary —— Add/Sub/Mul/Div/Mod
Expr deal_Binary(TreeNode *node, Type type, std::vector<std::string>& indexList) {
    //std::cout << "deal Binary begin!" << std::endl;

    Expr a = deal_expr(node->left, type, indexList);
    Expr b = deal_expr(node->right, type, indexList);

    BinaryOpType opType;
    switch (node->type) {
        case Add:
	        opType = BinaryOpType::Add;
            break;
        case Sub:
            opType = BinaryOpType::Sub;
            break;
        case Mul:
            opType = BinaryOpType::Mul;
            break;
        case Fdiv:
        case Idiv:
            opType = BinaryOpType::Div;
            break;
        default:
            opType = BinaryOpType::Mod;
            break;
    }

    //std::cout << "deal Binary end!" << std::endl;
    return Binary::make(type, opType, a, b);
}

// Var —— Sref
Expr deal_Sref(TreeNode *node, Type type) {
    //std::cout << "deal Sref" << std::endl;
    return Var::make(type, node->val.String, {}, varTable[node->val.String]);;
}

// Index
Expr deal_Index(TreeNode *node, Type type, std::vector<std::string>& indexList) {
    //std::cout << "deal Index begin!" << std::endl;

    if (!inVector(indexList, node->val.String)) {
        indexList.push_back(node->val.String);
    }

    Expr dom = Dom::make(index_type, indexDom[node->val.String].begin, indexDom[node->val.String].end);

    if (!inVector(leftindex, node->val.String)) {
        return Index::make(index_type, node->val.String, dom, IndexType::Reduce);
    }

    //std::cout << "deal Index end!" << std::endl;
    return Index::make(index_type, node->val.String, dom, IndexType::Spatial);
}

// Const —— IntImm/FloatImm
Expr deal_Const(TreeNode *node, Type type) {
    //std::cout << "deal Const begin!" << std::endl;

    if (type == Type::int_scalar(32)) {
        //std::cout << "deal Const end!" << std::endl;
        return IntImm::make(Type::int_scalar(32), node->val.Int);
    }

    //std::cout << "deal Const end!" << std::endl;
    return FloatImm::make(Type::float_scalar(32), node->val.Int);
}

// Tref的args处理
void deal_args(TreeNode *node, std::vector<Expr> &_args, std::vector<std::string> &_list) {
    //std::cout << "deal args begin!" << std::endl;
    
    if (node == NULL) {
        return;
    }

    if (node->type == Com) {
        deal_args(node->left, _args, _list);
        deal_args(node->right, _args, _list);
        //std::cout << "deal args begin!" << std::endl;
        return;
    }

    _args.push_back(deal_expr(node, index_type, _list));
    //std::cout << "deal args end!" << std::endl;
}

// Var —— Tref
Expr deal_Tref(TreeNode *node, Type type, std::vector<std::string>& indexList) {
    //std::cout << "deal Tref begin!" << std::endl;
    argsList.clear();
    deal_args(node->right, argsList, indexList);

    // 建立返回结构
    //std::cout << "deal Tref end!" << std::endl;
    return Var::make(data_type, node->val.String, argsList, varTable[node->val.String]);
}

Expr deal_expr(TreeNode *node, Type type, std::vector<std::string>& indexList) {
    //std::cout << "deal Expr begin!" << std::endl;

    switch (node->type) {
        case Add:
        case Sub:
        case Mul:
        case Fdiv:
        case Idiv:
        case Mod:
            return deal_Binary(node, type, indexList);
        case Tref:
            return deal_Tref(node, type, indexList);
        case Sref:
            return deal_Sref(node, type);
        case Intv:
        case Floatv:
            return deal_Const(node, type);
        case Par:
            return deal_expr(node->left, type, indexList);
        default:
            break;
    }
    //std::cout << "deal Expr end!" << std::endl;
    return deal_Index(node, type, indexList);
}

/*
 * deal_PosTerm(TreeNode *node, Expr tmp), deal_NegTerm(TreeNode *node, Expr tmp)
 * 处理等式右侧表达式的一个项，Pos为加，Neg为减。node表示该项的AST节点，tmp为该等式临时变量。该项如果有Reduce的index，
 * 需要用循环求和。返回类型为Stmt，返回值是处理该项的求值语句
 */
Stmt deal_PosTerm(TreeNode *node, Expr tmp) {
    //std::cout << "deal PosTerm begin!" << std::endl;

    // 获取表达式
    std::vector<std::string> indexList;
    Expr expr = deal_expr(node, data_type, indexList);

    // 判断是否有Reduce的index
    std::vector<Expr> loopindex;
    for (std::string index : indexList) {
        if (!inVector(leftindex, index)) {
            Expr dom = Dom::make(index_type, indexDom[index].begin, indexDom[index].end);
            loopindex.push_back(Index::make(index_type, index, dom, IndexType::Reduce));
        }
    }
    
    // 根据是否有Reduce的index确定返回语句
    MoveType mType;
    if (indexList.empty())
        mType = MoveType::LocalToMem;
    else
        mType = MoveType::MemToMem;
    
    if (loopindex.empty()) {
    //std::cout << "deal PosTerm end!" << std::endl;
        return Move::make(tmp,
            Binary::make(data_type, BinaryOpType::Add, tmp, expr), 
            mType);
    }
    Stmt move = Move::make(tmp,
        Binary::make(data_type, BinaryOpType::Add, tmp, expr), 
        mType);
    //std::cout << "deal PosTerm end!" << std::endl;
    return LoopNest::make(loopindex, {move});
}

Stmt deal_NegTerm(TreeNode *node, Expr tmp) {
    //std::cout << "deal NegTerm begin!" << std::endl;
    Stmt ret;

    // 获取表达式
    std::vector<std::string> indexList;
    Expr expr = deal_expr(node, data_type, indexList);

    // 判断是否有Reduce的index
    std::vector<Expr> loopindex;
    for (std::string index : indexList) {
        if (!inVector(leftindex, index)) {
            Expr dom = Dom::make(index_type, indexDom[index].begin, indexDom[index].end);
            loopindex.push_back(Index::make(index_type, index, dom, IndexType::Reduce));
        }
    }
    
    // 根据是否有Reduce的index确定返回语句
    MoveType mType;
    if (indexList.empty())
        mType = MoveType::LocalToMem;
    else
        mType = MoveType::MemToMem;
    
    if (loopindex.empty()) {
        //std::cout << "deal NegTerm end!" << std::endl;
        return Move::make(tmp,
            Binary::make(data_type, BinaryOpType::Sub, tmp, expr), 
            mType);
    }

    Stmt move = Move::make(tmp,
        Binary::make(data_type, BinaryOpType::Sub, tmp, expr), 
        mType);

    //std::cout << "deal NegTerm end!" << std::endl;
    return LoopNest::make(loopindex, {move});;
}

/*
 * deal_Pos(Neg)RHS(TreeNode *node, std::vector<Stmt> &stmtList, Expr tmp)
 * 处理等式右侧的表达式，Pos(Neg)表示该项前为加号(减号)，node表示右侧表达式的AST节点，stmtList为主循环体的语句列表，tmp为临时变量。
 * 将等式右侧划分为term。此处创建的Stmt都在主循环体中，所以要放入stmtList。
 * 返回值是Expr类型，是将term的计算结果联合起来的表达式
 */
void deal_NegRHS(TreeNode * node, std::vector<Stmt> &stmtList, Expr tmp);
void deal_PosRHS(TreeNode * node, std::vector<Stmt> &stmtList, Expr tmp) {
    //std::cout << "deal PosRHS begin!" << std::endl;
    if (node->type == Add) {
        deal_PosRHS(node->left, stmtList, tmp);
        deal_PosRHS(node->right, stmtList, tmp);
        //std::cout << "deal PosRHS end!" << std::endl;
        return;
    }

    if (node->type == Sub) {
        deal_PosRHS(node->left, stmtList, tmp);
        deal_NegRHS(node->right, stmtList, tmp);
//std::cout << "deal PosRHS end!" << std::endl;
        return;
    }

    // 处理项
    stmtList.push_back(deal_PosTerm(node, tmp));
//std::cout << "deal PosRHS end!" << std::endl;
    return;
}

void deal_NegRHS(TreeNode * node, std::vector<Stmt> &stmtList, Expr tmp) {
    //std::cout << "deal NegRHS begin!" << std::endl;
    if (node->type == Add) {
        deal_NegRHS(node->left, stmtList, tmp);
        deal_PosRHS(node->right, stmtList, tmp);
        //std::cout << "deal NegRHS end!" << std::endl;
        return;
    }

    if (node->type == Sub) {
        deal_NegRHS(node->left, stmtList, tmp);
        deal_NegRHS(node->right, stmtList, tmp);
//std::cout << "deal NegRHS end!" << std::endl;
        return;
    }

    // 处理项
    stmtList.push_back(deal_NegTerm(node, tmp));
//std::cout << "deal NegRHS end!" << std::endl;
    return;
}

void deal_Eq(TreeNode *node) {
//std::cout << "deal Eq begin!" << std::endl;
    // 处理等式左侧
//std::cout << "0 I'm here!" << std::endl;
    std::vector<std::string> indexList;
    Expr tref = deal_Tref(node->left, index_type, indexList);
//std::cout << "1 I'm here!" << std::endl;
    leftindex = indexList;
//std::cout << "2 I'm here!" << std::endl;
    // 主循环index，即等式左侧Tref的args
    std::vector<Expr> loopindex;
    for (std::string index : indexList) {
        Expr dom = Dom::make(index_type, indexDom[index].begin, indexDom[index].end);
        loopindex.push_back(Index::make(index_type, index, dom, IndexType::Spatial));
    }
//std::cout << "4 I'm here!" << std::endl;
    // 处理等式右侧
    std::vector<Stmt> stmtList;
    // 临时变量tmp记录右侧结果，它的结构与左侧Tref相同，初始化
    Expr tmp = Var::make(data_type, getTemp(), loopindex, varTable[node->left->val.String]);
    mainStmt.push_back(Def::make(tmp));
    if (data_type == Type::int_scalar(32)) {
        stmtList.push_back(Move::make(tmp, IntImm::make(data_type, 0), MoveType::LocalToMem));
    }
    else {
        stmtList.push_back(Move::make(tmp, FloatImm::make(data_type, 0), MoveType::LocalToMem));
    }

    deal_PosRHS(node->right, stmtList, tmp);
    mainStmt.push_back(LoopNest::make(loopindex, stmtList));
    mainStmt.push_back(LoopNest::make(loopindex, {Move::make(tref, tmp, MoveType::MemToMem)}));
//std::cout << "deal Eq end!" << std::endl;
}

void parseTree(TreeNode *node) {
//std::cout << "parse Tree begin!" << std::endl;
    if (node == NULL) {
        return;
    }
    switch (node->type) {
        case Eq:
            deal_Eq(node);
            break;
        default:
            parseTree(node->left);
            parseTree(node->right);
            break;
    }
//std::cout << "parse Tree end!" << std::endl;
}

void parseVar() {
//std::cout << "parse Var begin!" << std::endl;
    for (auto var_name: caseInfo.ins) {
        inVars.push_back(Var::make(data_type, var_name, {}, varTable[var_name]));
    }
    for (auto var_name: caseInfo.outs) {
        outVars.push_back(Var::make(data_type, var_name, {}, varTable[var_name]));
    }
//std::cout << "parse Var begin!" << std::endl;
}

std::string buildTree(TreeNode *node, std::string filename) {
    //调用read_json解析json文件, 记录case信息
    read_json(filename, caseInfo);
    if (strcmp(caseInfo.data_type.c_str(), "float") == 0) {
        data_type = Type::float_scalar(32);
    } 
    else {
        data_type = Type::int_scalar(32);
    }

    //初始化
    tmpNum = 0;
    inVars.clear();
    outVars.clear();
    mainStmt.clear();
    //给出ins/outs的变量结构
    parseVar();
    //开始建立IR树
    parseTree(node);
    // kernel
    Group kernel = Kernel::make(caseInfo.name, inVars, outVars, mainStmt, KernelType::CPU);
    // visitor
    IRVisitor visitor;
    kernel.visit_group(&visitor);
    // mutator
    IRMutator mutator;
    kernel = mutator.mutate(kernel);
    // printer
    IRPrinter printer;
    std::string code = printer.print(kernel);

    std::cout << "Success!\n";
    return code;
}

void pass(std::string inFile, std::string outFile) {
    //调用parse_file()解析json文件
    parse_file(inFile);
    //记录index和它的取值范围, 张量的shape信息，构造符号表
    indexDom.clear();
    varTable.clear();
    buildTable(TreeRoot);
    //遍历, 构造IR树
    std::string code = buildTree(TreeRoot, inFile);
    //不需要使用树之后调用free_tree()删除树
    //原因:1.建树过程调用了malloc,需要free防止内存泄漏
    //    2.避免后续解析其他json文件出错
    free_tree(TreeRoot);

    std::ofstream ofile(outFile, std::ios::out);
    ofile << code;
    std::cout << code << std::endl;
    ofile.close();
}

int main() {
    pass("./cases/case1.json", "./kernels/kernel_case1.cc");
    pass("./cases/case4.json", "./kernels/kernel_case4.cc");
    pass("./cases/case5.json", "./kernels/kernel_case5.cc");
    pass("./cases/case6.json", "./kernels/kernel_case6.cc");
    pass("./cases/case7.json", "./kernels/kernel_case7.cc");
    pass("./cases/case10.json", "./kernels/kernel_case10.cc");
    pass("./cases/example.json", "./kernels/kernel_example.cc");
    
    return 0;
}

