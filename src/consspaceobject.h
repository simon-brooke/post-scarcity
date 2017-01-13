/**
 *  consspaceobject.h
 *
 *  Declarations common to all cons space objects.
 *
 *
 *  (c) 2017 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
/* wide characters */
#include <wchar.h>
#include <wctype.h>

#ifndef __consspaceobject_h
#define __consspaceobject_h

/**
 * The length of a tag, in bytes.
 */
#define TAGLENGTH 4

/**
 * tag values, all of which must be 4 bytes. Must not collide with vector space tag values
 */
#define CONSTAG     "CONS"
#define FREETAG     "FREE"
#define FUNCTIONTAG "FUNC"
#define INTEGERTAG  "INTR"
#define NILTAG      "NIL "
#define READTAG     "READ"
#define REALTAG     "REAL"
#define STRINGTAG   "STRG"
#define TRUETAG     "TRUE"
#define VECTORPOINTTAG  "VECP"
#define WRITETAG    "WRIT"

/**
 * a cons pointer which points to the special NIL cell
 */
#define NIL (struct cons_pointer){ 0, 0}

/**
 * the maximum possible value of a reference count
 */
#define MAXREFERENCE 4294967295

/**
 * a macro to convert a tag into a number
 */
#define tag2uint(tag) ((uint32_t)*tag)

#define pointer2cell(pointer) ((conspages[pointer.page]->cell[pointer.offset]))

/**
 * true if conspointer points to the special cell NIL, else false 
 * (there should only be one of these so it's slightly redundant).
 */
#define nilp(conspoint) (check_tag(conspoint,NILTAG))

/**
 * true if conspointer points to a cons cell, else false 
 */
#define consp(conspoint) (check_tag(conspoint,CONSTAG))

/**
 * true if conspointer points to a function cell, else false 
 */
#define functionp(conspoint) (check_tag(conspoint,FUNCTIONTAG))

/**
 * true if conspointer points to a string cell, else false 
 */
#define stringp(conspoint) (check_tag(conspoint,STRINGTAG))

/**
 * true if conspointer points to an integer cell, else false 
 */
#define integerp(conspoint) (check_tag(conspoint,INTEGERTAG))

/**
 * true if conspointer points to a real number cell, else false 
 */
#define realp(conspoint) (check_tag(conspoint,REALTAG))

/**
 * true if conspointer points to some sort of a number cell, 
 * else false
 */
#define numberp(conspoint) (check_tag(conspoint,INTEGERTAG)||check_tag(conspoint,REALTAG))

/**
 * true if conspointer points to a true cell, else false 
 * (there should only be one of these so it's slightly redundant).
 * Also note that anything that is not NIL is truthy.
 */
#define tp(conspoint) (checktag(conspoint,TRUETAG))

/**
 * true if conspoint points to something that is truthy, i.e.
 * anything but NIL.
 */
#define truep(conspoint) (!checktag(conspoint,NILTAG))

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

struct function_payload {
  struct cons_pointer source;
  struct cons_pointer (*executable)(struct cons_pointer, struct cons_pointer);
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
  wint_t character;          /* the actual character stored in this cell */
  uint32_t padding;            /* unused padding to word-align the cdr */
  struct cons_pointer cdr;
};

/**
 * an object in cons space.
 */
struct cons_space_object {
  union {
    char bytes[TAGLENGTH];     /* the tag (type) of this cell, considered as bytes */
    uint32_t value;            /* the tag considered as a number */
  } tag;
  uint32_t count;              /* the count of the number of references to this cell */
  struct cons_pointer access;  /* cons pointer to the access control list of this cell */
  union {
    /* if tag == CONSTAG */
    struct cons_payload cons;
    /* if tag == FREETAG */
    struct free_payload free;
    /* if tag == FUNCTIONTAG */
    struct function_payload function;
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
 * Check that the tag on the cell at this pointer is this tag
 */
int check_tag( struct cons_pointer pointer, char* tag);


/**
 * increment the reference count of the object at this cons pointer
 */
void inc_ref( struct cons_pointer pointer);


/**
 * decrement the reference count of the object at this cons pointer
 */
void dec_ref( struct cons_pointer pointer);


/**
 * dump the object at this cons_pointer to this output stream.
 */
void dump_object( FILE* output, struct cons_pointer pointer);

struct cons_pointer make_cons( struct cons_pointer car, struct cons_pointer cdr);

/**
 * Construct a string from this character (which later will be UTF) and
 * this tail. A string is implemented as a flat list of cells each of which
 * has one character and a pointer to the next; in the last cell the 
 * pointer to next is NIL.
 */
struct cons_pointer make_string( char c, struct cons_pointer tail);

#endif
