#ifndef SEARCH_H
#define SEARCH_H

int search_file(const char *query, const char *filename, int case_insensitive, int inverted);

int search_path(const char *path, const char *query, int case_insensitive, int recursive, int inverted);

#endif