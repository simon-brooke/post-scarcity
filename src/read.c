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
#include <stdbool.h>
#include <stdio.h>

#include "consspaceobject.h"
#include "integer.h"
#include "read.h"

/* for the time being things which may be read are:
   strings
   numbers - either integer or real, but not yet including ratios or bignums
   lists
   Can't read atoms because I don't yet know what an atom is or how it's stored. */

struct cons_pointer read_number( FILE* input, char initial);
struct cons_pointer read_list( FILE* input, char initial);
struct cons_pointer read_string( FILE* input, char initial);


/**
 * Read the next object on this input stream and return a cons_pointer to it,
 * treating this initial character as the first character of the object
 * representation.
 */
struct cons_pointer read_continuation( FILE* input, char initial) {
  struct cons_pointer result = NIL;

  char c;

  for (c = initial; c == '\0' || isblank( c); c = fgetc( input));
  
  switch( c) {
  case '(' :
  case ')':
    result = read_list(input, fgetc( input));
    break;
  case '"': result = read_string(input, fgetc( input));
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
    result = read_number( input, c);
    break;
  default:
    fprintf( stderr, "Unrecognised start of input character %c\n", c);
  }

  return result;
}


/**
 * read a number from this input stream, given this initial character.
 */
struct cons_pointer read_number( FILE* input, char initial) {
  int accumulator = 0;
  int places_of_decimals = 0;
  bool seen_period = false;
  char c;

  fprintf( stderr, "read_number starting '%c' (%d)\n", initial, initial);
  
  for (c = initial; isdigit( c); c = fgetc( input)) {
    if ( c == '.') {
      seen_period = true;
    } else {
      accumulator = accumulator * 10 + ((int)c - (int)'0');

      if ( seen_period) {
	places_of_decimals ++;
      }
    }
  }

  /* push back the character read which was not a digit */
  fputc( c, input);

  return make_integer( accumulator);
}


/**
 * Read a list from this input stream, which no longer contains the opening
 * left parenthesis.
 */
struct cons_pointer read_list( FILE* input, char initial) {
  struct cons_pointer cdr = NIL;
  struct cons_pointer result= NIL;

  fprintf( stderr, "read_list starting '%c' (%d)\n", initial, initial);
  
  if ( initial != ')' ) {
    struct cons_pointer car = read_continuation( input, initial);
    cdr = read_list( input, fgetc( input));
    result = make_cons( car, cdr);
  }

  return result;
}


/** 
 * Read a string from this input stream, which no longer contains the opening
 * double quote. Note that there is (for now) a problem with the list
 * representation of a string, which is that there's no obvious representation of
 * an empty string.
 */
struct cons_pointer read_string( FILE* input, char initial) {
  struct cons_pointer cdr = NIL;
  struct cons_pointer result;

  fprintf( stderr, "read_string starting '%c' (%d)\n", initial, initial);

  switch ( initial) {
  case '\0':
    result = make_string( initial, NIL);
    break;
  case '"':
    result = make_string( '\0', NIL);
    break;
  default:
    result = make_string( initial, read_string( input, fgetc( input)));
    break;
  }

  return result;
}


/**
 * Read the next object on this input stream and return a cons_pointer to it.
 */
struct cons_pointer read( FILE* input) {
  return read_continuation( input, '\0');
}
    


    
