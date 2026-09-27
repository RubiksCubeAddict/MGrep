#include <stdio.h>
#include "../include/search.h"

int main(int argc, char *argv[]) {
	if (argc < 3) {
		printf("Usage: mgrep <search_term> <filename>\n");
		return 1;
	}

	if (search_file(argv[1], argv[2]) == 0) {
		return 0;
	}

	return -1;
}