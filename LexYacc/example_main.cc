#include <iostream>
#include "grad.h"
using namespace std;

int main() {
	for (int i = 1; i < 11; ++i) {
		cout << "case" << i << endl;
		Json json;
		std::string s = "cases/case";
		s += to_string(i)+".json";
		read_json(s, json);
		//cout << json.kernel << endl;
		parse_string(json.kernel);
		cout << "origin AST: ";
		pnode(TreeRoot);
		cout << endl;
		for (int j = 0; j < json.grad_to.size(); ++j) {
			TreeNode *root = change_tree(TreeRoot, json.grad_to[j]);
			cout << "new AST   : ";
			pnode(root);
			cout << endl;
		}
		free_all();
		cout << endl;
	}
	return 0;
}