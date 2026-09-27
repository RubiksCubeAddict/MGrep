#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>

int search_file(const char *filename, const char *query, int case_insensitive) {
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

			char* match = NULL;
			if (case_insensitive) {
				match = strcasestr(buffer, query);
			} else {
				match = strstr(buffer, query);
			}
			if (match != NULL) {
				printf("%s:%d\t%s", filename, linenum, buffer);
			}

			len = 0;
		}
	}

	if (len > 0) {
		buffer[len] = '\0';
		linenum++;
		char *match = case_insensitive ? strcasestr(buffer, query) : strstr(buffer, query);
		if (match != NULL) {
			printf("%s:%d\t%s\n", filename, linenum, buffer);
		}
	}

	fclose(fp);
	free(buffer);
	return 0;
}

int search_path(const char *path, const char *query, int case_insensitive, int recursive) {
	struct stat path_stat;
	if (stat(path, &path_stat) != 0) {
		printf("Could not access path %s\n", path);
		return -1;
	}

	if (S_ISREG(path_stat.st_mode)) {
		return search_file(path, query, case_insensitive);
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

			size_t path_len = strlen(path);
			size_t name_len = strlen(entry->d_name);
			size_t full_len = path_len + name_len + 2;

			char *full_path = malloc(full_len);
			if (full_path == NULL) {
				continue;
			}
			snprintf(full_path, full_len, "%s/%s", path, entry->d_name);

			search_path(full_path, query, case_insensitive, recursive);
		}
		closedir(dir);
	}
	return 0;
}