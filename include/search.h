#ifndef SEARCH_H
#define SEARCH_H

int search_file(const char *query, const char *filename, int case_insensitive, int inverted, int count_only, int word_match, int quiet, int files_with_matches, int context, int extended_regex);

int search_path(const char *path, const char *query, int case_insensitive, int recursive, int inverted, int count_only, int word_match, int quiet, int files_with_matches, int context, int extended_regex);

int is_valid_word_match(const char *buffer, const char *match, size_t query_len);

#endif