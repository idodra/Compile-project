#include "json.h"
#include "y.tab.c"

void parse_string(const std::string &s) {
	printf("parse string \"%s\"\n", s.c_str());
	FILE *file = fopen("json.txt", "w+");
	if (file == NULL) {
		printf("cannot open \"json.txt\"\n");
		exit(1);
	}
	fputs(s.c_str(), file);
	fseek(file, 0, SEEK_SET);
	extern FILE *yyin;
	yyin = file;
	printf("parse begin\n");
	yyparse();
	printf("parse end\n");
	fclose(file);
}

void parse_file(const std::string &filename) {
	printf("parse file \"%s\"\n", filename.c_str());
	Json json;
	read_json(filename, json);
	parse_string(json.kernel);
}