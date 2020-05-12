#include <iostream>
#include "lex_yacc.h"//include头文件以使用parser
using namespace std;

void deal_Add(TreeNode *node) {
    
}

void deal_Sub(TreeNode *node) {
    
}

void deal_Mul(TreeNode *node) {
    
}

void deal_Div(TreeNode *node) {
    
}

void deal_Mod(TreeNode *node) {
    
}

void deal_Eq(TreeNode *node) {
    
}

void deal_Com(TreeNode *node) {
    
}

void deal_Tref(TreeNode *node) {
    
}

void deal_Sref(TreeNode *node) {
    
}

void deal_Tref(TreeNode *node) {
    
}

void deal_Index(TreeNode *node) {
    
}

void deal_IntV(TreeNode *node) {
    
}

void deal_FloatV(TreeNode *node) {
    
}

void printTree(TreeNode *node) {
    if (node == NULL) {
        return;
    }
    
    switch (node->type) {
        case Add:
            deal_Add(node);
            break;
        case Sub:
            deal_Sub(node);
            break;
        case Mul:
            deal_Mul(node);
            break;
        case Div:
            deal_Div(node);
            break;
        case Mod:
            deal_Mod(node);
            break;
        case Eq:
            deal_Eq(node);
            break;
        case Mul:
        deal_Mul(node);
        break;
        case Mul:
        deal_Mul(node);
        break;
        case Mul:
        deal_Mul(node);
        break;
        case Mul:
        deal_Mul(node);
        break;
        case Mul:
        deal_Mul(node);
        break;
        case Mul:
        deal_Mul(node);
        break;
        case Mul:
        deal_Mul(node);
        break;
        case Mul:
        deal_Mul(node);
        break;
            
        default:
            break;
    }
    
    if (node->type == Intv) {
        cout << node->type << " " << node->val.Int << endl;
    }
    else if (node->type == Floatv) {
        cout << node->type << " " << node->val.Float << endl;
    }
    else {
        cout << node->type << " ";
        printf("%s\n", (node->val).String);
    }
    printTree(node->left);
    printTree(node->right);
}
int main() {
    //调用parse_file()解析json文件
    parse_file("../project1/cases/case1.json");
    //解析完成后,可以通过根节点TreeRoot(TreeNode*类型)访问树,比如:
    printTree(TreeRoot);
    //不需要使用树之后调用free_tree()删除树
    //原因:1.建树过程调用了malloc,需要free防止内存泄漏
    //    2.避免后续解析其他json文件出错
    free_tree(TreeRoot);
    return 0;
}

