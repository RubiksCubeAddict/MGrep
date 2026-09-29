#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <ctype.h>
#include <dirent.h>
#include <regex.h>

#define COLOR_GREEN "\033[1;32m"
#define COLOR_RED "\033[1;31m"
#define COLOR_RESET "\033[0m"

int is_valid_word_match(const char *buffer, const char *match, size_t query_len) {
	if (match > buffer) {
		char prev = *(match - 1);
		if (isalnum((unsigned char)prev) || prev == '_') {
			return 0;
		}
	}

	char next = *(match + query_len);
	if (isalnum((unsigned char)next) || next == '_') {
		return 0;
	}
	return 1;
}

int search_file(const char *filename, const char *query, int case_insensitive, int inverted, int count_only, int word_match, int quiet, int files_with_matches, int context, int extended_regex) {
	FILE *fp = fopen(filename, "r");
	if (fp == NULL) {
		printf("Could not open file %s\n", filename);
		return -1;
	}

	regex_t re;
	int re_compiled = 0;
	if (extended_regex) {
		int regflags = REG_EXTENDED;
		if (case_insensitive) regflags |= REG_ICASE;
		if(regcomp(&re, query, regflags) != 0) {
			if (!quiet) printf("mgrep: invalid regular expression: %s\n", query);
			if (re_compiled) regfree(&re);
			fclose(fp);
			return -1;
		}
		re_compiled = 1;
	}

	size_t buf_size = 128;
	char *buffer = malloc(buf_size);
	if (buffer == NULL) {
		if (re_compiled) regfree(&re);
		fclose(fp);
		return -1;
	}

	char **history_lines = NULL;
	int *history_linenums = NULL;
	if (context > 0) {
		history_lines = calloc(context, sizeof(char *));
		history_linenums = calloc(context, sizeof(int));
	}
	int history_head = 0;
	int history_count = 0;
	int post_context_left = 0;

	int linenum = 0;
	int match_count = 0;
	int c;
	size_t len = 0;

	while ((c = fgetc(fp)) != EOF) {
		if (len + 1 >= buf_size) {
			buf_size *= 2;
			char *new_buffer = realloc(buffer, buf_size);
			if (new_buffer == NULL) {
				if (context > 0) {
					for (int i = 0; i < context; i++) free(history_lines[i]);
					free(history_lines);
					free(history_linenums);
				}
				if (re_compiled) regfree(&re);
				fclose(fp);
				return -1;
			}
			buffer = new_buffer;
		}

		buffer[len++] = (char)c;

		if (c == '\n') {
			buffer[len] = '\0';
			linenum++;

			char *match = NULL;
			size_t query_len = 0;
			int matched = 0;

			if (extended_regex) {
				regmatch_t pmatch;
				if (regexec(&re, buffer, 1, &pmatch, 0) == 0) {
					matched = 1;
					match = buffer + pmatch.rm_so;
					query_len = pmatch.rm_eo - pmatch.rm_so;
				}
			} else {
				char *search_ptr = buffer;
				query_len = strlen(query);

				while (1) {
					if (case_insensitive) {
						match = strcasestr(search_ptr, query);
					} else {
						match = strstr(search_ptr, query);
					}

					if (match == NULL) {
						break;
					}

					if (!word_match || is_valid_word_match(buffer, match, query_len)) {
						break;
					}
					search_ptr = match + 1;
				}
			}

			int is_match = inverted ? !matched : matched;

			if (is_match) {
				match_count++;
				if(files_with_matches) {
					if (!quiet) printf("%s\n", filename);
					fclose(fp);
					free(buffer);
					if (context > 0) {
						for (int i = 0; i < context; i++) free(history_lines[i]);
						free(history_lines);
						free(history_linenums);
					}
					if (re_compiled) regfree(&re);
					return 1;
				}
				if (!quiet && !count_only) {
					if (context > 0 && history_count > 0) {
						for (int i = 0; i < history_count; i++) {
							int idx = (history_head + i) % context;
							if (history_lines[idx]) {
								printf("%s-%d-%s", filename, history_linenums[idx], history_lines[idx]);
							}
						}
						history_count = 0;
					}
					if (inverted) {
						printf("%s:%d\t%s", filename, linenum, buffer);
					} else {
						size_t prefix_len = match - buffer;
						fwrite(buffer, 1, prefix_len, stdout);

						printf(COLOR_RED);
						fwrite(match, 1, query_len, stdout);
						printf(COLOR_RESET);

						printf("%s", match + query_len);
					}
					post_context_left = context;
				}
			} else {
				if (!quiet && !count_only && post_context_left > 0) {
					printf("%s-%d-%s", filename, linenum, buffer);
					post_context_left--;
				} else if (context > 0) {
					int idx = (history_head + history_count) % context;
					if (history_lines[idx]) free(history_lines[idx]);
					history_lines[idx] = strdup(buffer);
					history_linenums[idx] = linenum;

					if (history_count < context) {
						history_count++;
					} else {
						history_head = (history_head + 1) % context;
					}
				}
			}
			len = 0;
		}
	}

	if (len > 0) {
		buffer[len] = '\0';
		linenum++;
		char *match = NULL;
		size_t query_len = 0;
		int matched = 0;

		if (extended_regex) {
			regmatch_t pmatch;
			if (regexec(&re, buffer, 1, &pmatch, 0) == 0) {
				matched = 1;
				match = buffer + pmatch.rm_so;
				query_len = pmatch.rm_eo - pmatch.rm_so;
			}
		} else {
			char *search_ptr = buffer;
			query_len = strlen(query);
			while (1) {
				if (case_insensitive) {
					match = strcasestr(search_ptr, query);
				} else {
					match = strstr(search_ptr, query);
				}
				if (match == NULL) {
					break;
				}

				if (!word_match || is_valid_word_match(buffer, match, query_len)) {
					matched = 1;
					break;
				}
				search_ptr = match + 1;
			}
		}
		int is_match = inverted ? !matched : matched;

		if (is_match) {
			match_count++;
			if(files_with_matches) {
				if (!quiet) {
					printf("%s\n", filename);
				}
				if (re_compiled) regfree(&re);
				fclose(fp);
				free(buffer);
				return 1;
			}
			if (!quiet && !count_only) {
				if (inverted) {
					printf("%s:%d\t%s\n", filename, linenum, buffer);
				} else {
					size_t prefix_len = match - buffer;
					fwrite(buffer, 1, prefix_len, stdout);

					printf(COLOR_RED);
					fwrite(match, 1, query_len, stdout);
					printf(COLOR_RESET);

					printf("%s", match + query_len);
				}
			}
		}
	}

	if (re_compiled) regfree(&re);

	if (count_only) {
		printf("%s:\t%s%d%s\n", filename, COLOR_GREEN, match_count, COLOR_RESET);
	}

	fclose(fp);
	free(buffer);
	return match_count;
}

int search_path(const char *path, const char *query, int case_insensitive, int recursive, int inverted, int count_only, int word_match, int quiet, int files_with_matches, int context, int extended_regex) {
	struct stat path_stat;
	if (stat(path, &path_stat) != 0) {
		if (!quiet) {
			printf("Could not access path %s\n", path);
		}
		return -1;
	}

	if (S_ISREG(path_stat.st_mode)) {
		return search_file(path, query, case_insensitive, inverted, count_only, word_match, quiet, files_with_matches, context, extended_regex);
	}

	else if (S_ISDIR(path_stat.st_mode)) {
		if (!recursive) {
			printf("mgrep: %s is a directory (use -r to search recursively)\n", path);
			return -1;
		}

		DIR *dir = opendir(path);
		if (dir == NULL) {
			printf("Could not open directory %s\n", path);
			return -1;
		}

		int total_matches = 0;
		struct dirent *entry;
		while ((entry = readdir(dir)) != NULL) {
			if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
				continue;
			}

			if (entry->d_name[0] == '.' ||
				strcmp(entry->d_name, "node_modules") == 0 ||
				strcmp(entry->d_name, "build") == 0 ||
				strcmp(entry->d_name, "bin") == 0) {
				continue;
			}

			size_t path_len = strlen(path);
			size_t name_len = strlen(entry->d_name);
			size_t full_len = path_len + name_len + 2;

			char *full_path = malloc(full_len);
			if (full_path == NULL) {
				continue;
			}
			snprintf(full_path, full_len, "%s/%s", path, entry->d_name);

			int res =search_path(full_path, query, case_insensitive, recursive, inverted, count_only, word_match, quiet, files_with_matches, context, extended_regex);

			if (res > 0) {
				total_matches += res;
			}

			free(full_path);
		}
		closedir(dir);
		return total_matches;
	}
	return 0;
}