
#include <stdlib.h>
#include "readstat.h"
#include "readstat_reader.h"
#include "readstat_io_unistd.h"

readstat_parser_t *readstat_parser_init(void) {
    readstat_parser_t *parser = calloc(1, sizeof(readstat_parser_t));
    parser->io = calloc(1, sizeof(readstat_io_t));
    if (unistd_io_init(parser) != READSTAT_OK) {
        readstat_parser_free(parser);
        return NULL;
    }
    parser->output_encoding = "UTF-8";
    return parser;
}

void readstat_parser_free(readstat_parser_t *parser) {
    if (parser) {
        if (parser->io) {
            readstat_set_io_ctx(parser, NULL);
            free(parser->io);
        }
        free(parser);
    }
}

readstat_error_t readstat_set_metadata_handler(readstat_parser_t *parser, readstat_metadata_handler metadata_handler) {
    parser->handlers.metadata = metadata_handler;
    return READSTAT_OK;
}

readstat_error_t readstat_set_note_handler(readstat_parser_t *parser, readstat_note_handler note_handler) {
    parser->handlers.note = note_handler;
    return READSTAT_OK;
}

readstat_error_t readstat_set_variable_handler(readstat_parser_t *parser, readstat_variable_handler variable_handler) {
    parser->handlers.variable = variable_handler;
    return READSTAT_OK;
}

readstat_error_t readstat_set_value_handler(readstat_parser_t *parser, readstat_value_handler value_handler) {
    parser->handlers.value = value_handler;
    return READSTAT_OK;
}

readstat_error_t readstat_set_value_label_handler(readstat_parser_t *parser, readstat_value_label_handler label_handler) {
    parser->handlers.value_label = label_handler;
    return READSTAT_OK;
}

readstat_error_t readstat_set_error_handler(readstat_parser_t *parser, readstat_error_handler error_handler) {
    parser->handlers.error = error_handler;
    return READSTAT_OK;
}

readstat_error_t readstat_set_progress_handler(readstat_parser_t *parser, readstat_progress_handler progress_handler) {
    parser->handlers.progress = progress_handler;
    return READSTAT_OK;
}

readstat_error_t readstat_set_fweight_handler(readstat_parser_t *parser, readstat_fweight_handler fweight_handler) {
    parser->handlers.fweight = fweight_handler;
    return READSTAT_OK;
}

readstat_error_t readstat_set_open_handler(readstat_parser_t *parser, readstat_open_handler open_handler) {
    parser->io->open = open_handler;
    return READSTAT_OK;
}

readstat_error_t readstat_set_close_handler(readstat_parser_t *parser, readstat_close_handler close_handler) {
    parser->io->close = close_handler;
    return READSTAT_OK;
}

readstat_error_t readstat_set_seek_handler(readstat_parser_t *parser, readstat_seek_handler seek_handler) {
    parser->io->seek = seek_handler;
    return READSTAT_OK;
}

readstat_error_t readstat_set_read_handler(readstat_parser_t *parser, readstat_read_handler read_handler) {
    parser->io->read = read_handler;
    return READSTAT_OK;
}

readstat_error_t readstat_set_update_handler(readstat_parser_t *parser, readstat_update_handler update_handler) {
    parser->io->update = update_handler;
    return READSTAT_OK;
}

readstat_error_t readstat_set_io_ctx(readstat_parser_t *parser, void *io_ctx) {
    if (parser->io->io_ctx_needs_free) {
        free(parser->io->io_ctx);
    }

    parser->io->io_ctx = io_ctx;
    parser->io->io_ctx_needs_free = 0;

    return READSTAT_OK;
}

readstat_error_t readstat_set_file_character_encoding(readstat_parser_t *parser, const char *encoding) {
    parser->input_encoding = encoding;
    return READSTAT_OK;
}

readstat_error_t readstat_set_handler_character_encoding(readstat_parser_t *parser, const char *encoding) {
    parser->output_encoding = encoding;
    return READSTAT_OK;
}

readstat_error_t readstat_set_row_limit(readstat_parser_t *parser, long row_limit) {
    parser->row_limit = row_limit;
    return READSTAT_OK;
}

readstat_error_t readstat_set_row_offset(readstat_parser_t *parser, long row_offset) {
    parser->row_offset = row_offset;
    return READSTAT_OK;
}

readstat_reader_t *readstat_reader_alloc(readstat_parser_t *parser,
        readstat_file_format_t format, void *user_ctx) {
    readstat_reader_t *reader = calloc(1, sizeof(readstat_reader_t));
    if (reader == NULL)
        return NULL;

    reader->parser = parser;
    reader->format = format;
    reader->user_ctx = user_ctx;
    return reader;
}

void readstat_reader_free(readstat_reader_t *reader) {
    if (reader == NULL)
        return;

    if (!reader->closed && reader->close)
        reader->close(reader);
    free(reader);
}

readstat_reader_t *readstat_reader_open(readstat_parser_t *parser,
        readstat_file_format_t format, const char *path, void *user_ctx,
        readstat_error_t *out_error) {
    readstat_reader_t *reader = NULL;
    readstat_error_t error = READSTAT_ERROR_PARSE;

    if (parser == NULL) {
        if (out_error)
            *out_error = READSTAT_ERROR_PARSE;
        return NULL;
    }

    switch (format) {
        case READSTAT_FILE_FORMAT_DTA:
            reader = readstat_dta_reader_open(parser, path, user_ctx, &error);
            break;
        case READSTAT_FILE_FORMAT_SAV:
            reader = readstat_sav_reader_open(parser, path, user_ctx, &error);
            break;
        case READSTAT_FILE_FORMAT_POR:
            reader = readstat_por_reader_open(parser, path, user_ctx, &error);
            break;
        case READSTAT_FILE_FORMAT_SAS7BDAT:
            reader = readstat_sas7bdat_reader_open(parser, path, user_ctx, &error);
            break;
        case READSTAT_FILE_FORMAT_XPORT:
            reader = readstat_xport_reader_open(parser, path, user_ctx, &error);
            break;
        default:
            error = READSTAT_ERROR_PARSE;
            break;
    }

    if (reader == NULL && out_error)
        *out_error = error;

    return reader;
}

readstat_error_t readstat_reader_read_rows(readstat_reader_t *reader, size_t max_rows) {
    readstat_error_t retval;

    if (reader == NULL)
        return READSTAT_ERROR_PARSE;
    if (reader->error != READSTAT_OK)
        return reader->error;
    if (reader->eof || max_rows == 0)
        return READSTAT_OK;

    retval = reader->read_rows(reader, max_rows);
    if (retval != READSTAT_OK)
        reader->error = retval;
    return retval;
}

int readstat_reader_is_eof(const readstat_reader_t *reader) {
    return reader == NULL || reader->eof;
}

void readstat_reader_close(readstat_reader_t *reader) {
    readstat_reader_free(reader);
}
