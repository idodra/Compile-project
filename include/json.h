#include <fstream>
#include <string>
#include <vector>

struct Json {
	std::string name;
	std::vector<std::string> ins;
	std::vector<std::string> outs;
	std::string data_type;
	std::string kernel;
	std::vector<std::string> grad_to;
};

std::string get_val(const std::string &s, const bool &flag) {
	char c;
	int begin, end;
	c = flag ? '\"' : '[';
	begin = s.find_first_of(c, s.find_first_of(':'))+1;
	c = flag ? '\"' : ']';
	end = s.find_last_of(c);
	return s.substr(begin, end-begin);
}

void split_save(const std::string &s, std::vector<std::string> &v) {
	int begin = -1, len = s.length();
	for (int i = 0; i < len; ++i) {
		if (s[i] == '\"') {
			if (begin == -1) {
				begin = i+1;
			}
			else {
				v.push_back(s.substr(begin, i-begin));
				begin = -1;
			}
		}
	}
}

std::string del_ws(const std::string &s) {
	int len = s.length();
	std::string ret = "";
	for (int i = 0; i < len; ++i) {
		if (s[i] != ' ') {
			ret.push_back(s[i]);
		}
	}
	return ret;
}

void read_json(const std::string &filename, Json &json) {
	std::string s;
	std::ifstream ifile(filename, std::ios::in);
	getline(ifile, s);
	getline(ifile, s);
	json.name = get_val(s, true);
	getline(ifile, s);
	split_save(get_val(s, false), json.ins);
	getline(ifile, s);
	split_save(get_val(s, false), json.outs);
	getline(ifile, s);
	json.data_type = get_val(s, true);
	getline(ifile, s);
	json.kernel = del_ws(get_val(s, true));
	getline(ifile, s);
	split_save(get_val(s, false), json.grad_to);
	ifile.close();
}