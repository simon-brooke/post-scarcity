/**
 * print.c
 *
 * First pass at a printer, for bootstrapping.
 *
 *
 * (c) 2017 Simon Brooke <simon@journeyman.cc>
 * Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include <ctype.h>
#include <stdio.h>
#include <string.h>
/* wide characters */
#include <wchar.h>
#include <wctype.h>

#include "conspage.h"
#include "consspaceobject.h"
#include "integer.h"
#include "print.h"

void print_string_contents( FILE* output, struct cons_pointer pointer) {
  if ( check_tag( pointer, STRINGTAG)) {
    struct cons_space_object* cell = &pointer2cell(pointer);
    wint_t c = cell->payload.string.character;

    if ( c != '\0') {
      fputwc( c, output);
    }
    print_string_contents( output, cell->payload.string.cdr);
  }
}


void print_string( FILE* output, struct cons_pointer pointer) {
  fputc( '"', output);
  print_string_contents( output, pointer);
  fputc( '"', output);
}


void print_list_contents( FILE* output, struct cons_pointer pointer) {
  if ( check_tag( pointer, CONSTAG)) {
    struct cons_space_object* cell = &pointer2cell(pointer);

    print( output, cell->payload.cons.car);

    if ( !nilp( cell->payload.cons.cdr)) {
      fputc( ' ', output);
    }
    print_list_contents( output, cell->payload.cons.cdr);
  }
}


void print_list( FILE* output, struct cons_pointer pointer) {
  fputc( '(', output);
  print_list_contents( output, pointer);
  fputc( ')', output);
}

void print( FILE* output, struct cons_pointer pointer) {
  struct cons_space_object cell = pointer2cell( pointer);

  /* Because tags have values as well as bytes, this if ... else if
   * statement can ultimately be replaced by a switch, which will
   * be neater. */
  if ( check_tag( pointer, CONSTAG)) {
    print_list( output, pointer);
  } else if ( check_tag( pointer, INTEGERTAG)) {
    fprintf( output, "%ld", cell.payload.integer.value);
  } else if ( check_tag( pointer, NILTAG)) {
    fprintf( output, "NIL");
  } else if ( check_tag( pointer, REALTAG)) {
    fprintf( output, "%Lf", cell.payload.real.value);
  } else if ( check_tag( pointer, STRINGTAG)) {
    print_string( output, pointer);
  } else if ( check_tag( pointer, TRUETAG)) {
    fprintf( output, "T");
  }
}
