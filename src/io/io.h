
/*
 * io.h
 *
 * Communication between PSSE and the outside world, via libcurl.
 *
 * (c) 2019 Simon Brooke <simon@journeyman.cc>
 * Licensed under GPL version 2.0, or, at your option, any later version.
 */

#ifndef __psse_io_h
#define __psse_io_h
#include <curl/curl.h>
#include "consspaceobject.h"

extern CURLSH *io_share;

int io_init(  );

URL_FILE *file_to_url_file( FILE * f );
wint_t url_fgetwc( URL_FILE * input );
wint_t url_ungetwc( wint_t wc, URL_FILE * input );

struct cons_pointer get_default_stream( bool inputp, struct cons_pointer env );

struct cons_pointer
lisp_close( struct stack_frame *frame, struct cons_pointer frame_pointer,
            struct cons_pointer env );
struct cons_pointer
lisp_open( struct stack_frame *frame, struct cons_pointer frame_pointer,
           struct cons_pointer env );
struct cons_pointer
lisp_read_char( struct stack_frame *frame, struct cons_pointer frame_pointer,
                struct cons_pointer env );
struct cons_pointer
lisp_slurp( struct stack_frame *frame, struct cons_pointer frame_pointer,
            struct cons_pointer env );


#endif
