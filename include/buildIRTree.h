#include "IR.h"
#include "IRMutator.h"
#include "IRVisitor.h"
#include "IRPrinter.h"
#include "type.h"

using namespace Boost::Internal;

export std::map<std::string, index_info > indexDom;        // index范围
export std::map<std::string, std::vector<size_t> > varTable;    // var表, 记录shape

Json caseInfo;                          // case信息
Type index_type = Type::int_scalar(32);     
Type data_type;
std::map<std::string, Expr> indexTable; //  index表
std::vector<Stmt> mainStmt;             // 最外层Stmt
std::vector<Expr> inVars;               // 输入变量
std::vector<Expr> outVars;              // 输出变量
std::vector<std::string> leftindex;     // 辅助, 记录等号左边出现的index

// 获取临时变量
int tmpNum = 0;
std::tring getTemp() {
    std::string tmp = "tmp" + to_string(tmpNum);
    tmpNum += 1;
    return tmp;
}

// Expr及相关信息的结构
struct Expr_info {
    Expr expr;
    std::vector<std::string> indexList;
    bool isLocal;
};

// 根据节点类型, 选择处理函数
Expr_info deal_expr(TreeNode *node, Type type);

// Binary —— Add/Sub/Mul/Div/Mod
Expr_info deal_Binary(TreeNode *node, Type type) {
    Expr_info a = deal_expr(node->left, type);
    Expr_info b = deal_expr(node->right, type);
    Expr_info ret;

    switch (node->type) {
        case Add:
            ret.expr = Binary::make(type, BinaryOpType::Add, a.expr, b.expr);
            break;
        case Sub:
            ret.expr = Binary::make(type, BinaryOpType::Sub, a.expr, b.expr);
            break;
        case Mul:
            ret.expr = Binary::make(type, BinaryOpType::Mul, a.expr, b.expr);
            break;
        case Div:
            ret.expr = Binary::make(type, BinaryOpType::Div, a.expr, b.expr);
            break;
        default:
            ret.expr = Binary::make(type, BinaryOpType::Mod, a.expr, b.expr);
            break;
    }

    for (std::string index: a.indexList) ret.indexList.push_back(index);
    for (std::string index: b.indexList) {
        if (find(a.indexList.begin(), a.indexList.end(), index) == a.indexList.end()) {
            ret.indexList.push_back(index);
        }    
    }

    ret.isLocal = a.isLocal && b.isLocal;
    return ret;
}

// Var —— Sref
Expr_info deal_Sref(TreeNode *node, Type type) {
    Expr expr = Var::make(type, node->val.String, NULL, varTable[node->val.String]);

    Expr_info ret;
    ret.expr = expr;
    ret.isLocal = true;
    return ret;
}

// Index
Expr_info deal_Index(TreeNode *node, Type type) {
    Expr_info ret;
    ret.indexList.push_back(node->val.String);
    ret.isLocal = true;

    if (indexTable.find(node->val.String) != indexTable.end()) {
        ret.expr = indexTable[node->val.String];
        return ret;
    }

    Expr dom = Dom::make(index_type, indexDom[node->val.String].begin, indexDom[node->val.String].end);
    IndexType iType;
    if (find(leftindex.begin(), leftindex.end(), node->val.String) == leftindex.end()) {
        iType = IndexType::Reduce;
    }
    else {
        iType = IndexType::Spatial;
    }
    Expr expr = Index::make(index_type, node->val.String, dom, iType);
    indexTable[node->val.String] = expr;
    ret.expr = expr;
    return ret;
}

// Const —— IntImm/FloatImm
Expr_info deal_Const(TreeNode *node, Type type) {
    Expr ret;
    Expr expr;
    
    if (type == Type::int_scalar(32)) {
        expr = IntImm::make(Type::int_scalar(32), node->val.Int);
    }
    else {
        expr = FloatImm::make(Type::float_scalar(32), node->val.Int);
    }

    ret.expr = expr;
    ret.isLocal = true;

    return ret;
}

// Tref的args处理
void deal_args(TreeNode *node, std::vector<Expr> &_args, std::vector<std::string> &_list) {
    if (node == NULL) {
        return;
    }
    if (node->type == Com) {
        deal_args(node->left, _args, _list);
        deal_args(node->right, _args, _list);
        return;
    }
    Expr_info info = deal_expr(node, index_type);
    _args.push_back(info.expr);
    for (std::string index: info.indexList) {
        if (find(_list.begin(), _list.end(), index) == _list.end()) _list.push_back(index);
    }
}

// Var —— Tref
Expr_info deal_Tref(TreeNode *node, Type type) {
    std::vector<Expr> argsList;
    std::vector<std::string> indexList; 
    deal_args(node->right, argsList, indexList);
    // 创建结点
    Expr expr = Var::make(data_type, node->val.String, argsList, varTable[node->val.String]);
    // 建立返回结构
    Expr_info ret;
    ret.expr = expr;
    ret.indexList = indexList;
    ret.isLocal = false;
    return ret;
}

Expr_info deal_expr(TreeNode *node, Type type) {
    if (node == NULL) {
        return;
    }
    Expr_info ret;
    switch (node->type) {
        case Add:
        case Sub:
        case Mul:
        case Div:
        case Mod:
            ret = deal_Binary(node, type);
            break;
        case Tref：
            ret = deal_Tref(node, type);
            break;
        case Sref：
            ret = deal_Sref(node, type);
            break;
        case Index:
            ret = deal_Index(node, type);
            break;
        case Intv:
        case Floatv:
            ret = deal_Const(node, type);
            break;
        case Par:
            ret = deal_expr(node->left, type)
            break;
        default:
            break;
    }
    return ret;
}

/*
 * deal_Term(TreeNode *node, Expr tmp)
 * 处理等式右侧表达式的一个项。node表示该项的AST节点，tmp为记录该项值的临时变量。该项如果有Reduce的index，需要用循环求和。
 * 返回类型为Stmt，返回值是处理该项的求值语句
 */
Stmt deal_Term(TreeNode *node, Expr tmp) {
    if (node == NULL) {
        return;
    }
    Stmt ret;

    // 获取表达式
    Expr_info info = deal_expr(node, data_type);

    // 判断是否有Reduce的index
    std::vector<Expr> loopindex;
    for (std::string index : info.indexList) {
        if (find(leftindex.begin(), leftindex.end(), index) == leftindex.end()) {
            loopindex.push_back(indexTable[index]);
        }
    }
    
    // 根据是否有Reduce的index确定返回语句
    MoveType mType;
    if (info.isLocal) 
        mType = MoveType::LocalToLocal;
    else
        mType = MoveType::MemToLocal;
    
    if (loopindex.empty()) {
        ret = Move::make(tmp,
            Binary::make(data_type, BinaryOpType::Add, tmp, info.expr), 
            mType);
    }
    else {
        Stmt move = Move::make(tmp,
            Binary::make(data_type, BinaryOpType::Add, tmp, info.expr), 
            mType);
        ret = LoopNest::make(loopindex, {move});
    }
    
    return ret;
}

/*
 * deal_RHS(TreeNode *node, std::vector<Stmt> &stmtList)
 * 处理等式右侧的表达式，node表示右侧表达式的AST节点，stmtList为主循环体的语句列表。
 * 将等式右侧划分为term。此处创建的Stmt都在主循环体中，所以要放入stmtList。
 * 返回值是Expr类型，是将term的计算结果联合起来的表达式
 */
Expr deal_RHS(TreeNode * node, std::vector<Stmt> &stmtList) {
    Expr ret;

    switch (node->type) {
        case Add:
            Expr a = deal_RHS(node->left);
            Expr b = deal_RHS(node->right);
            ret = Binary::make(data_type, BinaryOpType::Add, a, b);
            break;
        case Sub:
            Expr a = deal_RHS(node->left);
            Expr b = deal_RHS(node->right);
            ret = Binary::make(data_type, BinaryOpType::Sub, a, b);
            break;
        default:
            // 创建新的临时变量
            std::string tmpName = getTemp();
            Expr tmp = Var::make(data_type, tmpName, NULL, {1});
            // 临时变量初始化
            if (data_type == Type::int_scalar(32)) {
                stmtList.push_back(Move::make(tmp, IntImm::make(data_type, 0), MoveType::LocalToLocal));
            }
            else {
                stmtList.push_back(Move::make(tmp, FloatImm::make(data_type, 0), MoveType::LocalToLocal));
            }
            // 处理项
            stmtList.push_back(deal_Term(node, tmp));
            break;
    }
    return ret;
}

void deal_Eq(TreeNode *node) {
    // 处理等式左侧
    Expr_info info = deal_Tref(node->left, IndexType);
    leftindex = info.indexList;
    // 主循环index，即等式左侧Tref的args
    std::vector<Expr> loopindex;
    for (std::string index : info.indexList) {
        loopindex.push_back(indexTable[index]);
    }

    // 处理等式右侧
    std::vector<Stmt> stmtList;
    // 临时变量tmp记录右侧结果，它的结构与左侧Tref相同，初始化
    Expr tmp = Var::make(data_type, "tmp", loopindex, varTable[node->left->val.String]);
    if (data_type == Type::int_scalar(32)) {
        stmtList.push_back(Move::make(tmp, IntImm::make(data_type, 0), MoveType::LocalToMem));
    }
    else {
        stmtList.push_back(Move::make(tmp, FloatImm::make(data_type, 0), MoveType::LocalToMem));
    }
    // tmp = tmp + deal_RHS表达式
    Expr rhs = deal_RHS(node->right, stmtList);
    stmtList.push_back(Move::make(tmp,
        Binary::make(data_type, BinaryOpType::Add, tmp, rhs), 
        LocalToMem);
    mainStmt.push_back(LoopNest::make(loopindex, stmtList));
    mainStmt.push_back(LoopNest::make(loopindex, {Move::make(info.expr, tmp, MoveType::MemToMem)}));
}

void parseTree(TreeNode *node) {
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
}

void parseVar() {
    for (auto var_name: caseInfo.ins) {
        inVars.push_back(Var::make(data_type, var_name, NULL, varTable[var_name]));
    }
    for (auto var_name: caseInfo.outs) {
        outVars.push_back(Var::make(data_type, var_name, NULL, varTable[var_name]));
    }
}

void buildTree(TreeNode *node, std::string filename) {
    //调用read_json解析json文件, 记录case信息
    read_json(filename, caseInfo);
    if (strcmp(caseInfo.data_type, "float") == 0) {
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

    std::cout << code;
    std::cout << "Success!\n";
}