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

  if ( check_tag( pointer, CONSTAG)) {
    fputc( '(', output);
    for (struct cons_pointer p = pointer; consp( p);
	 p = pointer2cell( p).payload.cons.cdr) {
      print( output, p);
      fputc( ' ', output);
    }
    fputc( ')', output);
  } else if ( check_tag( pointer, INTEGERTAG)) {
    fprintf( output, " %ld", cell.payload.integer.value);
  } else if ( check_tag( pointer, NILTAG)) {
    fprintf( output, "NIL");
  } else if ( check_tag( pointer, REALTAG)) {
    fprintf( output, "%Lf", cell.payload.real.value);
  } else if ( check_tag( pointer, STRINGTAG)) {
    fputc( '"', output);
    for (struct cons_pointer p = pointer; stringp( p);
	 p = pointer2cell( p).payload.string.cdr) {
      // TODO: That's potentially a UTF character, needs more handling.
      char c = (char)pointer2cell( p).payload.string.character;

      if ( c != '\0') {
	fprintf( output, "%c", c);
      }
    }
    fputc( '"', output);
  } else if ( check_tag( pointer, TRUETAG)) {
    fprintf( output, "T");
  }
}
