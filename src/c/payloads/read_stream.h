/**
 *  payloads/read_stream.h
 *
 *  A read stream.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#ifndef __psse_payloads_read_stream_h
#define __psse_payloads_read_stream_h

#include <stdio.h>

#include "memory/pointer.h"

/**
 * An open read stream.
 */
#define READTAG     "REA"

/**
 * payload of a read or write stream cell.
 */
struct stream_payload {
    /** the stream to read from or write to. */
    URL_FILE *stream;
    /** metadata on the stream (e.g. its file attributes if a file, its HTTP
     * headers if a URL, etc). Expected to be an association, or nil. Not yet
     * implemented. */
    struct cons_pointer meta;
};

#endif
