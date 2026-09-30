#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../include/search.h"

void print_usage(void) {
	printf("Usage: mgrep [OPTIONS] <search_term> <filename/dir>\n");
	printf("Options:\n");
	printf("	-i			Case-insensitive search\n");
	printf("	-r			Recursive search\n");
	printf("	-v			Inverted match\n");
	printf("	-c			Count matching lines\n");
	printf("	-w			Whole-word match\n");
	printf("	-q			Quiet mode (exit codes only)\n");
	printf("	-l			Files with matches only\n");
	printf("	-C <num> 		Print <num> lines of context\n");
	printf("	-E			Extended regular expression (POSIX regex)\n");
	printf("	-n			Print line number with output lines\n");
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
	int context = 0;
	int extended_regex = 0;
	int show_line_numbers = 0;

	while ((opt = getopt(argc, argv, "irvcwqlC:En")) != -1) {
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
			case 'C':
				context = atoi(optarg);
				break;
			case 'E':
				extended_regex = 1;
				break;
			case 'n':
				show_line_numbers = 1;
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

	const char *query = argv[optind];
	const char *path = argv[optind + 1];

	int total_matches = search_path(path, query, case_insensitive, recursive, inverted, count_only, word_match, quiet, files_with_matches, context, extended_regex, show_line_numbers);

	if (total_matches < 0) {
		return 2;
	}

	if (quiet) {
		return (total_matches > 0) ? 0 : 1;
	}

}