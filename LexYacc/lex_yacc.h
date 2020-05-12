#include "json.h"
#include "y.tab.c"

void parse_file(const std::string &filename) {
	Json json;
	read_json(filename, json);
	FILE *file = fopen("json.txt", "w+");
	if (file == NULL) {
		printf("cannot open \"json.txt\"\n");
		exit(1);
	}
	fputs(json.kernel.c_str(), file);
	fseek(file, 0, SEEK_SET);
	extern FILE *yyin;
	yyin = file;
	printf("parse begin\n");
	yyparse();
	printf("parse end\n");
	fclose(file);
}