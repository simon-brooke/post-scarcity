/**
 *  payloads/read_stream.h
 *
 *  A read stream.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include <stdio.h>

#include <curl/curl.h>

#include "io/fopen.h"
#include "memory/pointer.h"
#include "memory/pso.h"
#include "memory/pso2.h"
#include "memory/tags.h"


/**
 * Construct a cell which points to a stream open for reading.
 * @param input the C stream to wrap.
 * @param metadata a pointer to an associaton containing metadata on the stream.
 * @return a pointer to the new read stream.
 */
struct pso_pointer make_read_stream( URL_FILE *input,
                                      struct pso_pointer metadata ) {
    struct pso_pointer pointer = allocate( READTAG, 2);
    struct pso2 *cell = pointer_to_object( pointer );

    cell->payload.stream.stream = input;
    cell->payload.stream.meta = metadata;

    return pointer;
}
