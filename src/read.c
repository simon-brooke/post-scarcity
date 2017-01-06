/**
 * read.c
 *
 * First pass at a reader, for bootstrapping.
 *
 *
 * (c) 2017 Simon Brooke <simon@journeyman.cc>
 * Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include <ctype.h>
#include <stdio.h>

#include "consspaceobject.h"
#include "integer.h"
#include "read.h"

/* for the time being things which may be read are:
   strings
   numbers - either integer or real, but not yet including ratios or bignums
   lists
   Can't read atoms because I don't yet know what an atom is or how it's stored. */

/**
 * read a number from this input stream, given this initial character.
 */
struct cons_pointer readnumber( FILE* input, char initial) {
  int accumulator = 0;
  char c;
  
  for (c = initial; isdigit( c); c = fgetc( input)) {
    int digitvalue = (int)c - (int)'0';
    accumulator = accumulator * 10 + digitvalue;
  }

  /* push back the character read which was not a digit */
  fputc( c, input);

  return makeinteger( accumulator);
}


struct cons_pointer readlist( FILE* input) {
  return NIL;
}


struct cons_pointer readstring( FILE* input) {
  return NIL;
}


/**
 * read the next object on this input stream and return a cons_pointer to it.
 */
struct cons_pointer read( FILE* input) {
  struct cons_pointer result = NIL;

  char c = fgetc( input);

  while ( isblank( c)) {
    c = fgetc( input);
  }
  
  switch( c) {
  case '(' : result = readlist(input);
    break;
  case '"': result = readstring(input);
    break;
  case '0':
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9':
    // case '.':
    result = readnumber( input, c);
    break;
  default:
    fprintf( stderr, "Unrecognised start of input character %c\n", c);
  }

  return result;
}
    


    
