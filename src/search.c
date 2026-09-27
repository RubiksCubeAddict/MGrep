#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int search_file(const char *query, const char *filename) {
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

			char* match = strstr(buffer, query);
			if (match != NULL) {
				printf("%d\t%s", linenum, buffer);
			}

			len = 0;
		}
	}

	if (len > 0) {
		buffer[len] = '\0';
		linenum++;
		char *match = strstr(buffer, query);
		if (match != NULL) {
			printf("%d\t%s\n", linenum, buffer);
		}
	}

	fclose(fp);
	free(buffer);
	return 0;
}