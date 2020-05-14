#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <queue>

#include "lex_yacc.h"//include头文件以使用parser
#include "buildTable.h"
#include "buildIRTree.h"

void pass(std::string filename) {
    //调用parse_file()解析json文件
    parse_file(filename);
    //记录index和它的取值范围, 张量使用的index信息，构造符号表
    buildTable(TreeRoot);
    //遍历, 构造IR树
    buildTree(TreeRoot, filename);
    //不需要使用树之后调用free_tree()删除树
    //原因:1.建树过程调用了malloc,需要free防止内存泄漏
    //    2.避免后续解析其他json文件出错
    free_tree(TreeRoot);
}

int main() {
    pass("../cases/case1.json");
    pass("../cases/case4.json");
    pass("../cases/case5.json");
    pass("../cases/case6.json");
    pass("../cases/case7.json");
    pass("../cases/case10.json");
    pass("../cases/example.json");
    
    return 0;
}

