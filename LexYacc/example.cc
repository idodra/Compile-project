#include <iostream>
#include "lex_yacc.h"//include头文件以使用parser
using namespace std;
void printTree(TreeNode *node) {
	if (node == NULL) {
		return;
	}
	cout << node->type << " ";
	printTree(node->left);
	printTree(node->right);
}
int main() {
	//调用parse_file()解析json文件
	parse_file("case6.json");
	//解析完成后,可以通过根节点TreeRoot(TreeNode*类型)访问树,比如:
	printTree(TreeRoot);
	//不需要使用树之后调用free_tree()删除树
	//原因:1.建树过程调用了malloc,需要free防止内存泄漏
	//    2.避免后续解析其他json文件出错
	free_tree(TreeRoot);
	return 0;
}