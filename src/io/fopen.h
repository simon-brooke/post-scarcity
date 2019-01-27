/*
 * fopen.h
 *
 * adapted from https://curl.haxx.se/libcurl/c/fopen.html.
 *
 *
 * Modifications to read/write wide character streams by
 * Simon Brooke.
 *
 * NOTE THAT: for my purposes, I'm only interested in wide characters,
 * and I always read them one character at a time.
 *
 * Copyright (c) 2003, 2017 Simtec Electronics
 * Some portions (c) 2019 Simon Brooke <simon@journeyman.cc>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This example requires libcurl 7.9.7 or later.
 */

#ifndef __fopen_h
#define __fopen_h
#include <curl/curl.h>
/*
 * wide characters
 */
#include <wchar.h>
#include <wctype.h>

#define url_fwprintf(f, ...) ((f->type = CFTYPE_FILE) ? fwprintf( f->handle.file, __VA_ARGS__) : -1)
#define url_fputws(ws, f) ((f->type = CFTYPE_FILE) ?  fputws(ws, f->handle.file) : 0)
#define url_fputwc(wc, f) ((f->type = CFTYPE_FILE) ?  fputwc(wc, f->handle.file) : 0)

enum fcurl_type_e {
    CFTYPE_NONE = 0,
    CFTYPE_FILE = 1,
    CFTYPE_CURL = 2
};

struct fcurl_data {
    enum fcurl_type_e type;     /* type of handle */
    union {
        CURL *curl;
        FILE *file;
    } handle;                   /* handle */

    char *buffer;               /* buffer to store cached data */
    size_t buffer_len;          /* currently allocated buffer's length */
    size_t buffer_pos;          /* cursor into in buffer */
    int still_running;          /* Is background url fetch still in progress */
};

typedef struct fcurl_data URL_FILE;

/* exported functions */
URL_FILE *url_fopen( const char *url, const char *operation );
int url_fclose( URL_FILE * file );
int url_feof( URL_FILE * file );
size_t url_fread( void *ptr, size_t size, size_t nmemb, URL_FILE * file );
char *url_fgets( char *ptr, size_t size, URL_FILE * file );
void url_rewind( URL_FILE * file );

wint_t url_fgetwc( URL_FILE * file );
wint_t url_ungetwc( wint_t wc, URL_FILE * input );
URL_FILE *file_to_url_file( FILE * f );

#endif
