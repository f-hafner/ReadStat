#ifndef INCLUDE_READSTAT_READER_H
#define INCLUDE_READSTAT_READER_H

#include "readstat.h"

struct readstat_reader_s {
    readstat_parser_t *parser;
    readstat_file_format_t format;
    void *user_ctx;
    void *ctx;
    int eof;
    int closed;
    readstat_error_t error;
    readstat_error_t (*read_rows)(readstat_reader_t *reader, size_t max_rows);
    void (*close)(readstat_reader_t *reader);
};

readstat_reader_t *readstat_reader_alloc(readstat_parser_t *parser,
        readstat_file_format_t format, void *user_ctx);
void readstat_reader_free(readstat_reader_t *reader);

readstat_reader_t *readstat_dta_reader_open(readstat_parser_t *parser,
        const char *path, void *user_ctx, readstat_error_t *out_error);
readstat_reader_t *readstat_sav_reader_open(readstat_parser_t *parser,
        const char *path, void *user_ctx, readstat_error_t *out_error);
readstat_reader_t *readstat_por_reader_open(readstat_parser_t *parser,
        const char *path, void *user_ctx, readstat_error_t *out_error);
readstat_reader_t *readstat_sas7bdat_reader_open(readstat_parser_t *parser,
        const char *path, void *user_ctx, readstat_error_t *out_error);
readstat_reader_t *readstat_xport_reader_open(readstat_parser_t *parser,
        const char *path, void *user_ctx, readstat_error_t *out_error);

#endif
