#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define NODE_MAX 2000

enum TreeNodeType {Add, Sub, Mul, Fdiv, Idiv, Mod, Eq, Com, Par, Tref, Sref, Index, Intv, Floatv};

union TreeNodeVal {
	int Int;
	float Float;
	char *String;
};

struct TreeNode {
	TreeNodeVal val;
	TreeNodeType type;
	TreeNode *left, *right;
};

int node_num = 0;
TreeNode *TreeRoot = NULL;
TreeNode nodes[NODE_MAX];

// void ValCopy(TreeNodeVal &dest, const TreeNodeVal &src, const TreeNodeType &t) {
// 	if (t == Intv) {
// 		dest.Int = src.Int;
// 	}
// 	else if (t == Floatv) {
// 		dest.Float = src.Float;
// 	}
// 	else {
// 		dest.String = strdup(src.String);
// 	}
// }

bool ValEqual(const TreeNodeVal &val1, const TreeNodeVal &val2, const TreeNodeType &t) {
	if (t == Intv) {
		return (val1.Int == val2.Int);
	}
	else if (t == Floatv) {
		return (val1.Float == val2.Float);
	}
	else {
		return (strcmp(val1.String, val2.String) == 0);
	}
}

TreeNode* find_valnode(const TreeNodeType &t, const TreeNode *left, const TreeNode *right, const TreeNodeVal &val) {
	for (int i = 0; i < node_num; ++i) {
		if (nodes[i].type == t && nodes[i].left == left && nodes[i].right == right && ValEqual(nodes[i].val, val, t)) {
			return nodes+i;
		}
	}
	return NULL;
}

TreeNode* new_valnode(const TreeNodeType &t, TreeNode *left, TreeNode *right, const TreeNodeVal &val) {
	if (t != Intv && t != Floatv && t != Index && t != Tref && t != Sref) {
		return NULL;
	}
	TreeNode* node = find_valnode(t, left, right, val);
	if (node == NULL) {
		nodes[node_num].type = t;
		nodes[node_num].left = left;
		nodes[node_num].right = right;
		if (t == Intv) {
			nodes[node_num].val.Int = val.Int;
		}
		else if (t == Floatv) {
			nodes[node_num].val.Float = val.Float;
		}
		else {
			nodes[node_num].val.String = strdup(val.String);
		}
		node = nodes+node_num;
		node_num++;
	}
	return node;
}

TreeNode* find_node(const TreeNodeType &t, const TreeNode *left, const TreeNode *right) {
	for (int i = 0; i < node_num; ++i) {
		if (nodes[i].type == t && nodes[i].left == left && nodes[i].right == right) {
			return nodes+i;
		}
	}
	return NULL;
}

TreeNode* new_node(const TreeNodeType &t, TreeNode *left, TreeNode *right) {
	if (t == Intv || t == Floatv || t == Index || t == Tref || t == Sref) {
		return NULL;
	}
	TreeNode* node = find_node(t, left, right);
	if (node == NULL) {
		nodes[node_num].type = t;
		nodes[node_num].left = left;
		nodes[node_num].right = right;
		node = nodes+node_num;
		node_num++;
	}
	return node;
}

void free_node(TreeNode *node) {
	if (node->type == Index || node->type == Tref || node->type == Sref) {
		if (node->val.String != NULL) {
			free(node->val.String);
			node->val.String = NULL;
		}
	}
	if (node->left != NULL) {
		free_node(node->left);
	}
	if (node->right != NULL) {
		free_node(node->right);
	}
}

void free_tree(TreeNode *root) {
	if (root != NULL) {
		free_node(root);
	}
	node_num = 0;
	TreeRoot = NULL;
}
