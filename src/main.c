#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <unistd.h>
#include "../include/search.h"

void print_usage(void) {
	printf("Usage: mgrep [-i] [-r] <search_term> <filename/dir>\n");
}

int main(int argc, char *argv[]) {
	int opt;
	int case_insensitive = 0;
	int recursive = 0;
	int inverted = 0;
	int count_only = 0;
	int word_match = 0;
	int quiet = 0;
	int files_with_matches = 0;

	while ((opt = getopt(argc, argv, "irvcwql")) != -1) {
		switch (opt) {
			case 'i':
				case_insensitive = 1;
				break;
			case 'r':
				recursive = 1;
				break;
			case 'v':
				inverted = 1;
				break;
			case 'c':
				count_only = 1;
				break;
			case 'w':
				word_match = 1;
				break;
			case 'q':
				quiet = 1;
				break;
			case 'l':
				files_with_matches = 1;
				break;
			default:
				print_usage();
				return -1;
		}
	}

	if (optind + 2 > argc) {
		print_usage();
		return -1;
	}

	const char *path = argv[optind];
	const char *query = argv[optind + 1];

	int total_matches = search_path(path, query, case_insensitive, recursive, inverted, count_only, word_match, quiet, files_with_matches);

	if (total_matches < 0) {
		return 2;
	}

	if (quiet) {
		return (total_matches > 0) ? 0 : 1;
	}

}