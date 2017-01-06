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
#include <stdio.h>

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
 * the maximum possible value of a reference count
 */
#define MAXREFERENCE ((2 ^ 32) - 1)

/**
 * a macro to convert a tag into a number
 */
#define tag2uint(tag) ((uint32_t)*tag)

#define pointer2cell(pointer) ((conspages[pointer.page]->cell[pointer.offset]))


/**
 * true if conspointer points to the special cell NIL, else false 
 */
#define nilp(conspoint) (strncmp(pointer2cell(conspoint).tag, NILTAG, TAGLENGTH)==0)

/**
 * true if conspointer points to a cons cell, else false 
 */
#define consp(conspoint) (strncmp(pointer2cell(conspoint).tag, CONSTAG, TAGLENGTH)==0)

/**
 * true if conspointer points to a string cell, else false 
 */
#define stringp(conspoint) (strncmp(pointer2cell(conspoint).tag, STRINGTAG, TAGLENGTH)==0)

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
  long int value;
};


/**
 * payload for a real number cell. Internals of this liable to change to give 128 bits
 * precision, but I'm not sure of the detail.
 */
struct real_payload {
  long double value;
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
  char tag[TAGLENGTH];         /* the tag (type) of this cell */
  uint32_t count;              /* the count of the number of references to this cell */
  struct cons_pointer access;  /* cons pointer to the access control list of this cell */
  union {
    /* if tag == CONSTAG */
    struct cons_payload cons;
    /* if tag == FREETAG */
    struct free_payload free;
    /* if tag == INTEGERTAG */
    struct integer_payload integer;
    /* if tag == NILTAG; we'll treat the special cell NIL as just a cons */
    struct cons_payload nil;
    /* if tag == REALTAG */
    struct real_payload real;
    /* if tag == STRINGTAG */
    struct string_payload string;
    /* if tag == TRUETAG; we'll treat the special cell T as just a cons */
    struct cons_payload t;
  } payload;
};


/**
 * increment the reference count of the object at this cons pointer
 */
void incref( struct cons_pointer pointer);


/**
 * decrement the reference count of the object at this cons pointer
 */
void decref( struct cons_pointer pointer);


/**
 * dump the object at this cons_pointer to this output stream.
 */
void dump_object( FILE* output, struct cons_pointer pointer);

#endif
