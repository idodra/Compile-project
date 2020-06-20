#include <string>
#include "lex_yacc.h"
using namespace std;

std::string grad_name = "";
TreeNode *grad_node = NULL;
TreeNode *one_node = NULL;
TreeNode *zero_node = NULL;

// void find_grad_node(TreeNode *node) {
// 	if (node == NULL) {
// 		return;
// 	}
// 	if (node->type == Tref && strcmp(node->val.String, grad_name.c_str()) == 0) {
// 		if (grad_node == NULL) {
// 			grad_node = node;
// 		}
// 		else {
// 			//
// 		}
// 	}
// 	else {
// 		find_grad_node(node->left);
// 		find_grad_node(node->right);
// 	}
// }

void init(std::string &name) {
	TreeNodeVal val;
	grad_name = name;
	grad_node = NULL;//find_grad_node(root);
	val.Int = 1;
	one_node = new_valnode(Intv, NULL, NULL, val);
	val.Int = 0;
	zero_node = new_valnode(Intv, NULL, NULL, val);
}

TreeNode* rename_node(TreeNode *node) {
	char temp[50];
	TreeNodeVal val;
	TreeNode *node_ret;
	strcpy(temp, "d");
	val.String = strdup(strcat(temp, node->val.String));
	node_ret = new_valnode(node->type, node->left, node->right, val);
	free(val.String);
	return node_ret;
}

bool isONE(TreeNode *node) {
	return (node->type == Intv && node->val.Int == 1);
}

bool isZERO(TreeNode *node) {
	return (node->type == Intv && node->val.Int == 0);
}

bool isEXPR(TreeNode *node) {
	TreeNodeType type = node->type;
	return (type == Add || type == Sub || type == Mul || type == Fdiv || type == Idiv || type == Mod);
}

TreeNode* sim_add(TreeNode *left, TreeNode *right) {
	bool lis0 = isZERO(left);
	bool ris0 = isZERO(right);
	if (lis0 && ris0) {
		return zero_node;
	}
	else if (!lis0 && ris0) {
		return left;
	}
	else if (lis0 && !ris0) {
		return right;
	}
	else {
		return new_node(Add, left, right);
	}
}

TreeNode* sim_sub(TreeNode *left, TreeNode *right) {
	bool lis0 = isZERO(left);
	bool ris0 = isZERO(right);
	if (lis0 && ris0) {
		return zero_node;
	}
	else if (!lis0 && ris0) {
		return left;
	}
	else if (lis0 && !ris0) {
		return new_node(Sub, left, right);
	}
	else {
		return new_node(Sub, left, right);
	}
}

TreeNode* sim_mul(TreeNode *left, TreeNode *right) {
	bool lis0 = isZERO(left);
	bool ris0 = isZERO(right);
	bool lis1 = isONE(left);
	bool ris1 = isONE(right);
	if (lis0 || ris0) {
		return zero_node;
	}
	else if (lis1 && ris1) {
		return one_node;
	}
	else if (!lis1 && ris1) {
		return left;
	}
	else if (lis1 && !ris1) {
		return right;
	}
	else {
		return new_node(Mul, left, right);
	}
}

TreeNode* sim_div(TreeNode *left, TreeNode *right) {
	bool lis0 = isZERO(left);
	bool ris0 = isZERO(right);
	bool lis1 = isONE(left);
	bool ris1 = isONE(right);
	if (ris0) {
		exit(1);
	}
	else if (lis0) {
		return zero_node;
	}
	else if (ris1) {
		return left;
	}
	else {
		return new_node(Fdiv, left, right);
	}
}

TreeNode* get_grad(TreeNode *node) {
	TreeNode *left, *right;
	switch (node->type) {
		case Add:
			left = get_grad(node->left);
			right = get_grad(node->right);
			return sim_add(left, right);
		case Sub:
			left = get_grad(node->left);
			right = get_grad(node->right);
			return sim_sub(left, right);
		case Mul:
			//left = new_node(Mul, get_grad(node->left), node->right);
			//right = new_node(Mul, node->left, get_grad(node->right));
			//return new_node(Add, left, right);
			TreeNode *ret;
			left = sim_mul(get_grad(node->left), node->right);
			right = sim_mul(node->left, get_grad(node->right));
			ret = sim_add(left, right);
			if (isEXPR(ret)) {
				return new_node(Par, ret, NULL);
			}
			else {
				return ret;
			}
		case Fdiv:
		case Idiv:
			// left = new_node(Mul, get_grad(node->left), node->right);
			// right = new_node(Mul, node->left, get_grad(node->right));
			// left = new_node(Sub, left, right);
			// right = new_node(Mul, node->right, node->right);
			// return new_node(Fdiv, left, right);
			left = sim_mul(get_grad(node->left), node->right);
			right = sim_mul(node->left, get_grad(node->right));
			left = sim_sub(left, right);
			if (isEXPR(left)) {
				left = new_node(Par, left, NULL);
			}
			right = sim_mul(node->right, node->right);
			if (isEXPR(right)) {
				right = new_node(Par, right, NULL);
			}
			return sim_div(left, right);
		case Eq:
			TreeNode *Dout, *Din;
			Dout = rename_node(node->left);
			right = get_grad(node->right);
			Din = rename_node(grad_node);
			return new_node(Eq, Din, sim_mul(Dout, right));
		case Com:
			left = get_grad(node->left);
			right = get_grad(node->right);
			return new_node(Com, left, right);
		case Par:
			left = get_grad(node->left);
			return new_node(Par, left, NULL);
		case Tref:
			if (strcmp(node->val.String, grad_name.c_str()) == 0) {
				if (grad_node == NULL) {
					grad_node = node;
				}
				return one_node;
			}
			return zero_node;
		case Mod:
		case Sref:
		case Index:
		case Intv:
		case Floatv:
		default:
			return zero_node;
	}
}

TreeNode* change_tree(TreeNode *root, std::string &name) {
	init(name);
	return get_grad(root);
}

void free_all() {
	for (int i = 0; i < node_num; ++i) {
		if (nodes[i].type == Index || nodes[i].type == Tref || nodes[i].type == Sref) {
			if (nodes[i].val.String != NULL) {
				free(nodes[i].val.String);
				nodes[i].val.String = NULL;
			}
		}
	}
	node_num = 0;
	TreeRoot = NULL;
	grad_name = "";
	grad_node = NULL;
	one_node = NULL;
	zero_node = NULL;
}

void pnode(TreeNode *node) {
	switch (node->type) {
		case Add:
			pnode(node->left);
			cout << "+";
			pnode(node->right);
			break;
		case Sub:
			pnode(node->left);
			cout << "-";
			pnode(node->right);
			break;
		case Mul:
			pnode(node->left);
			cout << "*";
			pnode(node->right);
			break;
		case Fdiv:
		case Idiv:
			pnode(node->left);
			cout << "/";
			pnode(node->right);
			break;
		case Eq:
			pnode(node->left);
			cout << "=";
			pnode(node->right);
			break;
		case Com:
			pnode(node->left);
			cout << ",";
			pnode(node->right);
			break;
		case Par:
			cout << "(";
			pnode(node->left);
			cout << ")";
			break;
		case Tref:
			cout << node->val.String << "<";
			pnode(node->left);
			cout << ">[";
			pnode(node->right);
			cout << "]";
			break;
		case Mod:
			pnode(node->left);
			cout << "%";
			pnode(node->right);
			break;
		case Sref:
			cout << node->val.String << "<";
			pnode(node->left);
			cout << ">";
			break;
		case Index:
			cout << node->val.String;
			break;
		case Intv:
			cout << node->val.Int;
			break;
		case Floatv:
			cout << node->val.Float;
		default:
			break;
	}
}