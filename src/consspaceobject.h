/**
 *  consspaceobject.h
 *
 *  Declarations common to all cons space objects.
 *
 *
 *  (c) 2017 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */


#include <stdint.h>

#ifndef __consspaceobject_h
#define __consspaceobject_h

/**
 * The length of a tag, in bytes.
 */
#define TAGLENGTH 4

/**
 * tag values, all of which must be 4 bytes. Must not collide with vector space tag values
 */
#define CONSTAG  "CONS"
#define FREETAG  "FREE"
#define INTEGERTAG  "INTR"
#define NILTAG  "NIL "
#define READTAG  "READ"
#define REALTAG  "REAL"
#define STRINGTAG  "STRG"
#define TRUETAG  "TRUE"
#define VECTORPOINTTAG  "VECP"
#define WRITETAG  "WRIT"

/**
 * a cons pointer which points to the special NIL cell
 */
#define NIL (struct cons_pointer){ 0, 0}


/**
 * An indirect pointer to a cons cell
 */
struct cons_pointer {
  uint32_t page;               /* the index of the page on which this cell resides */
  uint32_t offset;             /* the index of the cell within the page */
};


/**
 * payload of a cons cell.
 */
struct cons_payload {
  struct cons_pointer car;
  struct cons_pointer cdr;
};

/**
 * payload of a free cell. For the time being identical to a cons cell,
 * but it may not be so in future.
 */
struct free_payload {
  struct cons_pointer car;
  struct cons_pointer cdr;
};

/**
 * payload of an integer cell. For the time being just a signed integer;
 * later might be a signed 128 bit integer, or might have some flag to point to an
 * optional bignum object.
 */
struct integer_payload {
  long int integer;
};


/**
 * payload for a real number cell. Internals of this liable to change to give 128 bits
 * precision, but I'm not sure of the detail.
 */
struct real_payload {
  long double real;
};

/**
 * payload of a string cell. At least at first, only one UTF character will be stored in each cell.
 */
struct string_payload {
  uint32_t character;          /* the actual character stored in this cell */
  uint32_t padding;            /* unused padding to word-align the cdr */
  struct cons_pointer cdr;
};

/**
 * an object in cons space.
 */
struct cons_space_object {
  char tag[TAGLENGTH];         /* the tag (type) of this cons cell */
  uint32_t count;              /* the count of the number of references to this cell */
  struct cons_pointer access;  /* cons pointer to the access control list of this cell */
  union {
    /* if tag == CONSTAG */
    struct cons_payload cons;
    /* if tag == FREETAG */
    struct free_payload free;
    struct integer_payload integer;
    struct cons_payload nil;
    struct real_payload real;
    struct string_payload string;
    struct cons_payload t;
  } payload;
};


#endif
