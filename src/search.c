#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <ctype.h>
#include <dirent.h>

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

int search_file(const char *filename, const char *query, int case_insensitive, int inverted, int count_only, int word_match) {
	FILE *fp = fopen(filename, "r");
	if (fp == NULL) {
		printf("Could not open file %s\n", filename);
		return -1;
	}

	size_t buf_size = 128;
	char *buffer = malloc(buf_size);
	if (buffer == NULL) {
		fclose(fp);
		return -1;
	}

	int linenum = 0;
	int match_count = 0;
	int c;
	size_t len = 0;

	while ((c = fgetc(fp)) != EOF) {
		if (len + 1 >= buf_size) {
			buf_size *= 2;
			char *new_buffer = realloc(buffer, buf_size);
			if (new_buffer == NULL) {
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
			char *search_ptr = buffer;
			size_t query_len = strlen(query);

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

			int is_match = inverted ? (match == NULL) : (match != NULL);

			if (is_match) {
				if (count_only) {
					match_count++;
				} else {
					if (inverted) {
						printf("%s:%d\t%s", filename, linenum, buffer);
					} else {
						size_t prefix_len = match - buffer;
						fwrite(buffer, 1, prefix_len, stdout);

						size_t query_len = strlen(query);
						printf(COLOR_RED);
						fwrite(match, 1, query_len, stdout);
						printf(COLOR_RESET);

						printf("%s", match + query_len);
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
		char *search_ptr = buffer;
		size_t query_len = strlen(query);

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
		int is_match = inverted ? (match == NULL) : (match != NULL);

		if (is_match) {
			if (count_only) {
				match_count++;
			} else {
				if (inverted) {
					printf("%s:%d\t%s\n", filename, linenum, buffer);
				} else {
					size_t prefix_len = match - buffer;
					fwrite(buffer, 1, prefix_len, stdout);

					size_t query_len = strlen(query);
					printf(COLOR_RED);
					fwrite(match, 1, query_len, stdout);
					printf(COLOR_RESET);

					printf("%s", match + query_len);
				}
			}
		}
	}

	if (count_only) {
		printf("%s:\t%s%d%s\n", filename, COLOR_GREEN, match_count, COLOR_RESET);
	}

	fclose(fp);
	free(buffer);
	return 0;
}

int search_path(const char *path, const char *query, int case_insensitive, int recursive, int inverted, int count_only, int word_match) {
	struct stat path_stat;
	if (stat(path, &path_stat) != 0) {
		printf("Could not access path %s\n", path);
		return -1;
	}

	if (S_ISREG(path_stat.st_mode)) {
		return search_file(path, query, case_insensitive, inverted, count_only, word_match);
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

			search_path(full_path, query, case_insensitive, recursive, inverted, count_only, word_match);

			free(full_path);
		}
		closedir(dir);
	}
	return 0;
}