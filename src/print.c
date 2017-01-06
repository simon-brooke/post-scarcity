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

#include "conspage.h"
#include "consspaceobject.h"
#include "integer.h"
#include "print.h"

void print( FILE* output, struct cons_pointer pointer) {
  struct cons_space_object cell = pointer2cell( pointer);

  if ( strncmp( cell.tag, CONSTAG, TAGLENGTH) == 0) {
    fputc( '(', output);
    for (struct cons_pointer p = pointer; consp( p);
	 p = pointer2cell( p).payload.cons.cdr) {
      print( output, p);
      fputc( ' ', output);
    }
    fputc( ')', output);
  } else if ( strncmp( cell.tag, INTEGERTAG, TAGLENGTH) == 0) {
    fprintf( output, " %ld", cell.payload.integer.value);
  } else if ( strncmp( cell.tag, NILTAG, TAGLENGTH) == 0) {
    fprintf( output, "NIL");
  } else if ( strncmp( cell.tag, REALTAG, TAGLENGTH) == 0) {
    fprintf( output, "%Lf", cell.payload.real.value);
  } else if ( strncmp( cell.tag, STRINGTAG, TAGLENGTH) == 0) {
    fputc( '"', output);
    for (struct cons_pointer p = pointer; stringp( p);
	 p = pointer2cell( p).payload.string.cdr) {
      fprintf( output, '%c', (char)pointer2cell( p).payload.string.character);
    }
    fputc( '"', output);
  } else if ( strncmp( cell.tag, TRUETAG, TAGLENGTH) == 0) {
    fprintf( output, "T");
  }
}
