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

#include "conspage.h"
#include "consspaceobject.h"
#include "debug.h"
#include "print.h"
#include "stack.h"
#include "vectorspace.h"


void dump_string_cell( FILE * output, wchar_t *prefix,
                       struct cons_pointer pointer ) {
    struct cons_space_object cell = pointer2cell( pointer );
    if ( cell.payload.string.character == 0 ) {
        fwprintf( output,
                  L"\t\t%ls cell: termination; next at page %d offset %d, count %u\n",
                  prefix,
                  cell.payload.string.cdr.page, cell.payload.string.cdr.offset,
                  cell.count );
    } else {
        fwprintf( output,
                  L"\t\t%ls cell: character '%lc' (%d) next at page %d offset %d, count %u\n",
                  prefix,
                  ( wint_t ) cell.payload.string.character,
                  cell.payload.string.character,
                  cell.payload.string.cdr.page,
                  cell.payload.string.cdr.offset, cell.count );
        fwprintf( output, L"\t\t value: " );
        print( output, pointer );
        fwprintf( output, L"\n" );
    }
}

/**
 * dump the object at this cons_pointer to this output stream.
 */
void dump_object( FILE * output, struct cons_pointer pointer ) {
    struct cons_space_object cell = pointer2cell( pointer );
    fwprintf( output,
              L"\t%4.4s (%d) at page %d, offset %d count %u\n",
              cell.tag.bytes,
              cell.tag.value, pointer.page, pointer.offset, cell.count );

    switch ( cell.tag.value ) {
        case CONSTV:
            fwprintf( output,
                      L"\t\tCons cell: car at page %d offset %d, cdr at page %d offset %d, count %u :",
                      cell.payload.cons.car.page,
                      cell.payload.cons.car.offset,
                      cell.payload.cons.cdr.page,
                      cell.payload.cons.cdr.offset, cell.count );
            print( output, pointer );
            fputws( L"\n", output );
            break;
        case EXCEPTIONTV:
            fwprintf( output, L"\t\tException cell: " );
            dump_stack_trace( output, pointer );
            break;
        case FREETV:
            fwprintf( output, L"\t\tFree cell: next at page %d offset %d\n",
                      cell.payload.cons.cdr.page,
                      cell.payload.cons.cdr.offset );
            break;
        case INTEGERTV:
            fwprintf( output,
                      L"\t\tInteger cell: value %ld, count %u\n",
                      cell.payload.integer.value, cell.count );
            if (!nilp(cell.payload.integer.more)) {
                fputws( L"\t\tBIGNUM! More at\n:", output);
                dump_object(output, cell.payload.integer.more);
            }
            break;
        case LAMBDATV:
            fwprintf( output, L"\t\tLambda cell;\n\t\t args: " );
            print( output, cell.payload.lambda.args );
            fwprintf( output, L";\n\t\t\tbody: " );
            print( output, cell.payload.lambda.body );
            fputws( L"\n", output );
            break;
        case NILTV:
            break;
        case NLAMBDATV:
            fwprintf( output, L"\t\tNlambda cell; \n\t\targs: " );
            print( output, cell.payload.lambda.args );
            fwprintf( output, L";\n\t\t\tbody: " );
            print( output, cell.payload.lambda.body );
            fputws( L"\n", output );
            break;
        case RATIOTV:
            fwprintf( output,
                      L"\t\tRational cell: value %ld/%ld, count %u\n",
                      pointer2cell( cell.payload.ratio.dividend ).
                      payload.integer.value,
                      pointer2cell( cell.payload.ratio.divisor ).
                      payload.integer.value, cell.count );
            break;
        case READTV:
            fwprintf( output, L"\t\tInput stream\n" );
            break;
        case REALTV:
            fwprintf( output, L"\t\tReal cell: value %Lf, count %u\n",
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
                fwprintf( output,
                          L"\t\tPointer to vector-space object at %p\n",
                          cell.payload.vectorp.address );
                struct vector_space_object *vso = cell.payload.vectorp.address;
                fwprintf( output,
                          L"\t\tVector space object of type %4.4s (%d), payload size %d bytes\n",
                          &vso->header.tag.bytes, vso->header.tag.value,
                          vso->header.size );
                if ( stackframep( vso ) ) {
                    dump_frame( output, pointer );
                }
                switch ( vso->header.tag.value ) {
                    case STACKFRAMETV:
                        dump_frame( output, pointer );
                        break;
                }
            }
            break;
        case WRITETV:
            fwprintf( output, L"\t\tOutput stream\n" );
            break;
    }
}
