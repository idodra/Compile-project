/*
 * 建立符号表，确定index范围
 */

const int MAX_INT = 0x7fffffff;
const int MIN_INT = 0x80000000;

// 记录index的取值范围
struct index_info {
    int begin;
    int end;
};
std::map<std::string, index_info > indexDom; // index范围
std::map<std::string, std::vector<size_t> > varTable; // var表, 记录shape
std::queue<int> sizeQueue; // 辅助, 记录Clist

// 根据节点类型分别处理
void parse_expr(TreeNode *node, int begin, int end);

// idExpr —— Add
void Index_Add(TreeNode *node, int begin, int end) {
    if (node->right->type == Intv) {
        int i = node->right->val.Int;
        int _begin = (begin == MAX_INT) ? begin : begin - i;
        int _end = (end == MIN_INT) ? end : end - i;
        parse_expr(node->left, _begin, _end);
        return;
    }
    parse_expr(node->left, begin, end);
    parse_expr(node->right, begin, end);
}

// idExpr —— Sub
void Index_Sub(TreeNode *node, int begin, int end) {
    if (node->right->type == Intv) {
        int i = node->right->val.Int;
        int _begin = (begin == MAX_INT) ? begin : begin + i;
        int _end = (end == MIN_INT) ? end : end + i;
        parse_expr(node->left, _begin, _end);
        return;
    }
    parse_expr(node->left, begin, end);
    parse_expr(node->right, begin, end);
}

// idExpr —— Mul
void Index_Mul(TreeNode *node, int begin, int end) {
    if (node->right->type == Intv) {
        int i = node->right->val.Int;
        int _begin = (begin == MAX_INT) ? begin : begin / i;
        int _end = (end == MIN_INT) ? end : end / i;
        parse_expr(node->left, _begin, _end);
        return;
    }
    parse_expr(node->left, begin, end);
    parse_expr(node->right, begin, end);
}

// idExpr —— Div
void Index_Div(TreeNode *node, int begin, int end) {
    if (node->right->type == Intv) {
        int i = node->right->val.Int;
        int _begin = (begin == MAX_INT) ? begin : begin * i;
        int _end = (end == MIN_INT) ? end : end * i;
        parse_expr(node->left, _begin, _end);
        return;
    }
    parse_expr(node->left, begin, end);
    parse_expr(node->right, begin, end);
}

// idExpr —— Mod
void Index_Mod(TreeNode *node, int begin, int end) {
    if (node->right->type == Intv) {
        int i = node->right->val.Int;
        parse_expr(node->left, MAX_INT, MIN_INT);
        return;
    }
    parse_expr(node->left, MAX_INT, MIN_INT);
    parse_expr(node->right, 1, end > 1 ? end : 1);
}

// idExpr —— Id
void Index_Id(TreeNode *node, int begin, int end) {
    if (indexDom.find(node->val.String) != indexDom.end()) {
        index_info info = indexDom[node->val.String];
        info.begin = info.begin > begin ? info.begin : begin;
        info.end = info.end < end ? info.end : end;
        indexDom[node->val.String] = info;
        return;
    }
    index_info info;
    info.begin = begin; info.end = end;
    indexDom[node->val.String] = info;
}

void parse_expr(TreeNode *node, int begin, int end) {
    if (node == NULL) {
        return;
    }
    switch (node->type) {
        case Add:
            Index_Add(node, begin, end);
            break;
        case Sub:
            Index_Sub(node, begin, end);
            break;
        case Mul:
            Index_Mul(node, begin, end);
            break;
        case Div:
            Index_Div(node, begin, end);
            break;
        case Mod:
            Index_Mod(node, begin, end);
            break;
        default:
            Index_Id(node, begin, end);
            break;
    }    
}

void insert_args(TreeNode *node) {
    if (node == NULL) {
        return;
    }
    switch (node->type) {
        case Add:
            Index_Add(node, 0, sizeQueue.front());
            sizeQueue.pop();
            break;
        case Sub:
            Index_Sub(node, 0, sizeQueue.front());
            sizeQueue.pop();
            break;
        case Mul:
            Index_Mul(node, 0, sizeQueue.front());
            sizeQueue.pop();
            break;
        case Div:
            Index_Div(node, 0, sizeQueue.front());
            sizeQueue.pop();
            break;
        case Mod:
            Index_Mod(node, 0, sizeQueue.front());
            sizeQueue.pop();
            break;
        case Index:
            Index_Id(node, 0, sizeQueue.front());
            sizeQueue.pop();
            break;
        default:
            insert_args(node->left);
            insert_args(node->right);
            break;
    }
}

void insert_size(TreeNode *node, std::vector<int> &_shape) {
    if (node == NULL) {
        return;
    }
    if (node->type == Intv) {
        _shape.push_back(node->val.Int);
    }
    insert_size(node->left, _shape);
    insert_size(node->right, _shape);
}

void buildTable(TreeNode *node) {
    if (node == NULL) {
        return;
    }
    // 符号表中添加张量变量shape信息
    if (node->type == Tref) {
        std::vector<int> sizeList;
        insert_size(node->left, sizeList);

        while (!sizeQueue.empty()) sizeQueue.pop();
        std::vector<size_t> shape;
        for (auto i: sizeList) { sizeQueue.push(i); shape.push_back(i); }
        insert_args(node->right);
        if (varTable.find(node->val.String) != varTable.end()) {
            varTable[node->val.String] = shape;
        }
        return;
    }
    // 符号表中添加常量变量shape信息
    if (node->type == Sref) {
        std::vector<int> sizeList;
        insert_size(node->left, sizeList);
        std::vector<size_t> shape;
        for (auto i: sizeList) shape.push_back(i);
        if (varTable.find(node->val.String) != varTable.end()) {
            varTable[node->val.String] = shape;
        }
        return;
    }

    buildTable(node->left);
    buildTable(node->right);
}