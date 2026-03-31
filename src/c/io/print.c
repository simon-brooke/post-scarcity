/**
 *  io/print.c
 *
 *  Post Scarcity Software Environment: print.
 *
 *  Print basic Lisp objects..This is :bootstrap layer print; it needs to be 
 *  able to print characters, symbols, integers, lists and dotted pairs. I 
 *  don't think it needs to be able to print anything else.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*
 * wide characters
 */
#include <wchar.h>
#include <wctype.h>
/* libcurl, used for io */
#include <curl/curl.h>

#include "io/fopen.h"
#include "io/io.h"

#include "memory/node.h"
#include "memory/pointer.h"
#include "memory/pso.h"
#include "memory/pso2.h"
#include "memory/tags.h"

#include "payloads/character.h"
#include "payloads/cons.h"
#include "payloads/integer.h"

struct pso_pointer in_print( struct pso_pointer p, URL_FILE * output);

struct pso_pointer print_list_content( struct pso_pointer p, URL_FILE * output) {
   struct pso_pointer result = nil;

    if (consp(p)) {
        for (; consp( p); p = cdr(p)) {
            struct pso2* object = pointer_to_object(p);
 
            result = in_print( object->payload.cons.car, output);

            if (exceptionp(result)) break;

            switch (get_tag_value(object->payload.cons.cdr)) {
                case NILTV :
                    break;
                case CONSTV :
                    url_fputwc( L' ', output );
                    break;
                default :
                    url_fputws( L" . ", output);
                    result = in_print( object->payload.cons.cdr, output);
            }

        }
    } else {
        // TODO: return exception
    }

    return result;
}

struct pso_pointer in_print( struct pso_pointer p, URL_FILE * output) {
    struct pso2* object = pointer_to_object(p);
    struct pso_pointer result = nil;

    if (object != NULL) {
        switch (get_tag_value( p)) {
            case CHARACTERTV :
                url_fputwc( object->payload.character.character, output);
                break;
            case CONSTV :
                url_fputwc( L'(', output );
                result = print_list_content( p, output);
                url_fputwc( L')', output );
                break;
            case INTEGERTV :
                url_fwprintf( output, L"%d", (int64_t)(object->payload.integer.value));
                break;
            case TRUETV :
                url_fputwc( L't', output );
                break;
            case NILTV :
                url_fputws( L"nil", output );
            default :
                // TODO: return exception
        }
    } else {
                // TODO: return exception
    }

    return result;
}

/**
 * @brief Simple print for bootstrap layer.
 * 
 * @param p pointer to the object to print.
 * @param stream if a pointer to an open write stream, print to there.
 * @return struct pso_pointer `nil`, or an exception if some erroe occurred.
 */
struct pso_pointer print( struct pso_pointer p, struct pso_pointer stream) {
    URL_FILE *output = writep( stream) ? 
        pointer_to_object( stream )->payload.stream.stream : 
        file_to_url_file(stdout);

    if ( writep( stream)) { inc_ref( stream); }

    struct pso_pointer result = in_print(p, output);

    if ( writep( stream)) { dec_ref( stream); }

    return result;
}
