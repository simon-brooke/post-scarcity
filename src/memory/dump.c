/*
 *  dump.c
 *
 *  Dump representations of both cons space and vector space objects.
 *
 *
 *  (c) 2018 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
/*
 * wide characters
 */
#include <wchar.h>
#include <wctype.h>

#include "memory/conspage.h"
#include "memory/consspaceobject.h"
#include "debug.h"
#include "memory/hashmap.h"
#include "ops/intern.h"
#include "io/io.h"
#include "io/print.h"
#include "memory/stack.h"
#include "memory/vectorspace.h"


void dump_string_cell( URL_FILE * output, wchar_t *prefix,
                       struct cons_pointer pointer ) {
    struct cons_space_object cell = pointer2cell( pointer );
    if ( cell.payload.string.character == 0 ) {
        url_fwprintf( output,
                      L"\t\t%ls cell: termination; next at page %d offset %d, count %u\n",
                      prefix,
                      cell.payload.string.cdr.page,
                      cell.payload.string.cdr.offset, cell.count );
    } else {
        url_fwprintf( output,
                      L"\t\t%ls cell: character '%lc' (%d) with hash %d; next at page %d offset %d, count %u\n",
                      prefix,
                      ( wint_t ) cell.payload.string.character,
                      cell.payload.string.character,
                      cell.payload.string.hash,
                      cell.payload.string.cdr.page,
                      cell.payload.string.cdr.offset, cell.count );
        url_fwprintf( output, L"\t\t value: " );
        print( output, pointer );
        url_fwprintf( output, L"\n" );
    }
}

/**
 * dump the object at this cons_pointer to this output stream.
 */
void dump_object( URL_FILE * output, struct cons_pointer pointer ) {
    struct cons_space_object cell = pointer2cell( pointer );
    url_fwprintf( output, L"\t%4.4s (%d) at page %d, offset %d count %u\n",
                  cell.tag.bytes, cell.tag.value, pointer.page, pointer.offset,
                  cell.count );

    switch ( cell.tag.value ) {
        case CONSTV:
            url_fwprintf( output,
                          L"\t\tCons cell: car at page %d offset %d, cdr at page %d "
                          L"offset %d, count %u :",
                          cell.payload.cons.car.page,
                          cell.payload.cons.car.offset,
                          cell.payload.cons.cdr.page,
                          cell.payload.cons.cdr.offset, cell.count );
            print( output, pointer );
            url_fputws( L"\n", output );
            break;
        case EXCEPTIONTV:
            url_fwprintf( output, L"\t\tException cell: " );
            dump_stack_trace( output, pointer );
            break;
        case FREETV:
            url_fwprintf( output,
                          L"\t\tFree cell: next at page %d offset %d\n",
                          cell.payload.cons.cdr.page,
                          cell.payload.cons.cdr.offset );
            break;
        case INTEGERTV:
            url_fwprintf( output, L"\t\tInteger cell: value %ld, count %u\n",
                          cell.payload.integer.value, cell.count );
            if ( !nilp( cell.payload.integer.more ) ) {
                url_fputws( L"\t\tBIGNUM! More at:\n", output );
                dump_object( output, cell.payload.integer.more );
            }
            break;
        case KEYTV:
            dump_string_cell( output, L"Keyword", pointer );
            break;
        case LAMBDATV:
            url_fwprintf( output, L"\t\t\u03bb cell;\n\t\t args: " );
            print( output, cell.payload.lambda.args );
            url_fwprintf( output, L";\n\t\t\tbody: " );
            print( output, cell.payload.lambda.body );
            url_fputws( L"\n", output );
            break;
        case NILTV:
            break;
        case NLAMBDATV:
            url_fwprintf( output, L"\t\tn\u03bb cell; \n\t\targs: " );
            print( output, cell.payload.lambda.args );
            url_fwprintf( output, L";\n\t\t\tbody: " );
            print( output, cell.payload.lambda.body );
            url_fputws( L"\n", output );
            break;
        case RATIOTV:
            url_fwprintf( output,
                          L"\t\tRational cell: value %ld/%ld, count %u\n",
                          pointer2cell( cell.payload.ratio.dividend ).
                          payload.integer.value,
                          pointer2cell( cell.payload.ratio.divisor ).
                          payload.integer.value, cell.count );
            break;
        case READTV:
            url_fputws( L"\t\tInput stream; metadata: ", output );
            print( output, cell.payload.stream.meta );
            url_fputws( L"\n", output );
            break;
        case REALTV:
            url_fwprintf( output, L"\t\tReal cell: value %Lf, count %u\n",
                          cell.payload.real.value, cell.count );
            break;
        case STRINGTV:
            dump_string_cell( output, L"String", pointer );
            break;
        case SYMBOLTV:
            dump_string_cell( output, L"Symbol", pointer );
            break;
        case TRUETV:
            break;
        case VECTORPOINTTV:{
                url_fwprintf( output,
                              L"\t\tPointer to vector-space object at %p\n",
                              cell.payload.vectorp.address );
                struct vector_space_object *vso = cell.payload.vectorp.address;
                url_fwprintf( output,
                              L"\t\tVector space object of type %4.4s (%d), payload size "
                              L"%d bytes\n",
                              &vso->header.tag.bytes, vso->header.tag.value,
                              vso->header.size );

                switch ( vso->header.tag.value ) {
                    case STACKFRAMETV:
                        dump_frame( output, pointer );
                        break;
                    case HASHTV:
                        dump_map( output, pointer );
                        break;
                }
            }
            break;
        case WRITETV:
            url_fputws( L"\t\tOutput stream; metadata: ", output );
            print( output, cell.payload.stream.meta );
            url_fputws( L"\n", output );
            break;
    }
}
