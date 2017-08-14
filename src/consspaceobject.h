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
/*
 * wide characters 
 */
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
/**
 * An ordinary cons cell: 1397641027
 */
#define CONSTAG     "CONS"
#define CONSTV      1397641027

/**
 * An unallocated cell on the free list - should never be encountered by a Lisp
 * function. 1162170950
 */
#define FREETAG     "FREE"
#define FREETV      1162170950

/**
 * An ordinary Lisp function - one whose arguments are pre-evaluated and passed as
 * a stack frame. 1129207110
 */
#define FUNCTIONTAG "FUNC"
#define FUNCTIONTV  1129207110
/**
 * An integer number. 1381256777
 */
#define INTEGERTAG  "INTR"
#define INTEGERTV   1381256777

/**
 * The special cons cell at address {0,0} whose car and cdr both point to itself.
 * 541870414
 */
#define NILTAG      "NIL "
#define NILTV       541870414

/**
 * An open read stream.
 */
#define READTAG     "READ"

/**
 * A real number.
 */
#define REALTAG     "REAL"
#define REALTV      1279346002

/**
 * A special form - one whose arguments are not pre-evaluated but passed as a
 * s-expression. 1296453715
 */
#define SPECIALTAG  "SPFM"
#define SPECIALTV   1296453715

/**
 * A string of characters, organised as a linked list. 1196577875
 */
#define STRINGTAG   "STRG"
#define STRINGTV    1196577875

/**
 * A symbol is just like a string except not self-evaluating. 1112365395 
 */
#define SYMBOLTAG   "SYMB"
#define SYMBOLTV    1112365395

/**
 * The special cons cell at address {0,1} which is canonically different from NIL.
 * 1163219540
 */
#define TRUETAG     "TRUE"
#define TRUETV      1163219540

/**
 * A pointer to an object in vector space.
 */
#define VECTORPOINTTAG  "VECP"

/**
 * An open write stream.
 */
#define WRITETAG    "WRIT"

/**
 * a cons pointer which points to the special NIL cell
 */
#define NIL (struct cons_pointer){ 0, 0}

/**
 * a cons pointer which points to the special T cell
 */
#define TRUE (struct cons_pointer){ 0, 1}

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
 * true if conspointer points to a special form cell, else false 
 */
#define specialp(conspoint) (check_tag(conspoint,SPECIALTAG))

/**
 * true if conspointer points to a string cell, else false 
 */
#define stringp(conspoint) (check_tag(conspoint,STRINGTAG))

/**
 * true if conspointer points to a string cell, else false 
 */
#define symbolp(conspoint) (check_tag(conspoint,SYMBOLTAG))

/**
 * true if conspointer points to an integer cell, else false 
 */
#define integerp(conspoint) (check_tag(conspoint,INTEGERTAG))

/**
 * true if conspointer points to a read stream cell, else false 
 */
#define readp(conspoint) (check_tag(conspoint,READTAG))

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
 * true if conspointer points to a write stream cell, else false.
 */
#define writep(conspoint) (check_tag(conspoint,WRITETAG))

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
    uint32_t page;              /* the index of the page on which this cell
                                 * resides */
    uint32_t offset;            /* the index of the cell within the page */
};

/*
 * number of arguments stored in a stack frame 
 */
#define args_in_frame 8

/**
 * A stack frame. Yes, I know it isn't a cons-space object, but it's defined
 * here to avoid circularity. TODO: refactor.
 */
struct stack_frame {
    struct stack_frame *previous; /* the previous frame */
    struct cons_pointer arg[args_in_frame];
    /*
     * first 8 arument bindings 
     */
    struct cons_pointer more;   /* list of any further argument bindings */
    struct cons_pointer function; /* the function to be called */
};

/**
 * payload of a cons cell.
 */
struct cons_payload {
    struct cons_pointer car;
    struct cons_pointer cdr;
};

/**
 * Payload of a function cell. 
 * source points to the source from which the function was compiled, or NIL 
 * if it is a primitive.
 * executable points to a function which takes a pointer to a stack frame
 * (representing its stack frame) and a cons pointer (representing its 
 * environment) as arguments and returns a cons pointer (representing its
 * result).
 */
struct function_payload {
    struct cons_pointer source;
    struct cons_pointer (*executable) (struct stack_frame *,
                                       struct cons_pointer);
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
 * Payload of a special form cell. 
 * source points to the source from which the function was compiled, or NIL 
 * if it is a primitive.
 * executable points to a function which takes a cons pointer (representing
 * its argument list) and a cons pointer (representing its environment) and a
 * stack frame (representing the previous stack frame) as arguments and returns
 * a cons pointer (representing its result).
 *
 * NOTE that this means that special forms do not appear on the lisp stack,
 * which may be confusing. TODO: think about this.
 */
struct special_payload {
    struct cons_pointer source;
    struct cons_pointer (*executable) (struct cons_pointer s_expr,
                                       struct cons_pointer env,
                                       struct stack_frame * frame);
};

/**
 * payload of a read or write stream cell.
 */
struct stream_payload {
    FILE *stream;
};

/**
 * payload of a string cell. At least at first, only one UTF character will
 * be stored in each cell. The doctrine that 'a symbol is just a string'
 * didn't work; however, the payload of a symbol cell is identical to the
 * payload of a string cell.
 */
struct string_payload {
    wint_t character;           /* the actual character stored in this cell */
    uint32_t padding;           /* unused padding to word-align the cdr */
    struct cons_pointer cdr;
};

struct vectorp_payload {
    union {
        char bytes[TAGLENGTH];  /* the tag (type) of the
                                 * vector-space object this cell
                                 * points to, considered as bytes.
                                 * NOTE that the vector space object 
                                 * should itself have the identical
                                 * tag. */
        uint32_t value;         /* the tag considered as a number */
    } tag;
    uint64_t address;           /* the address of the actual vector space
                                 * object (TODO: will change when I actually
                                 * implement vector space) */
};

/**
 * an object in cons space.
 */
struct cons_space_object {
    union {
        char bytes[TAGLENGTH];  /* the tag (type) of this cell,
                                 * considered as bytes */
        uint32_t value;         /* the tag considered as a number */
    } tag;
    uint32_t count;             /* the count of the number of references to
                                 * this cell */
    struct cons_pointer access; /* cons pointer to the access control list of
                                 * this cell */
    union {
        /*
         * if tag == CONSTAG 
         */
        struct cons_payload cons;
        /*
         * if tag == FREETAG 
         */
        struct free_payload free;
        /*
         * if tag == FUNCTIONTAG 
         */
        struct function_payload function;
        /*
         * if tag == INTEGERTAG 
         */
        struct integer_payload integer;
        /*
         * if tag == NILTAG; we'll treat the special cell NIL as just a cons 
         */
        struct cons_payload nil;
        /*
         * if tag == READTAG || tag == WRITETAG 
         */
        struct stream_payload stream;
        /*
         * if tag == REALTAG 
         */
        struct real_payload real;
        /*
         * if tag == SPECIALTAG 
         */
        struct special_payload special;
        /*
         * if tag == STRINGTAG || tag == SYMBOLTAG 
         */
        struct string_payload string;
        /*
         * if tag == TRUETAG; we'll treat the special cell T as just a cons 
         */
        struct cons_payload t;
        /*
         * if tag == VECTORPTAG 
         */
        struct vectorp_payload vectorp;
    } payload;
};

/**
 * Check that the tag on the cell at this pointer is this tag
 */
int check_tag(struct cons_pointer pointer, char *tag);

/**
 * increment the reference count of the object at this cons pointer
 */
void inc_ref(struct cons_pointer pointer);

/**
 * decrement the reference count of the object at this cons pointer
 */
void dec_ref(struct cons_pointer pointer);

/**
 * dump the object at this cons_pointer to this output stream.
 */
void dump_object(FILE * output, struct cons_pointer pointer);

struct cons_pointer make_cons(struct cons_pointer car,
                              struct cons_pointer cdr);

/**
 * Construct a cell which points to an executable Lisp special form.
 */
struct cons_pointer make_function(struct cons_pointer src,
                                  struct cons_pointer (*executable)








                                   (struct stack_frame *,
                                    struct cons_pointer));

/**
 * Construct a cell which points to an executable Lisp special form.
 */
struct cons_pointer make_special(struct cons_pointer src,
                                 struct cons_pointer (*executable)








                                  (struct cons_pointer s_expr,
                                   struct cons_pointer env,
                                   struct stack_frame * frame));

/**
 * Construct a string from this character and this tail. A string is
 * implemented as a flat list of cells each of which has one character and a 
 * pointer to the next; in the last cell the pointer to next is NIL.
 */
struct cons_pointer make_string(wint_t c, struct cons_pointer tail);

/**
 * Construct a symbol from this character and this tail. A symbol is identical
 * to a string except for having a different tag.
 */
struct cons_pointer make_symbol(wint_t c, struct cons_pointer tail);

/**
 * Return a lisp string representation of this old skool ASCII string.
 */
struct cons_pointer c_string_to_lisp_string(char *string);

/**
 * Return a lisp symbol representation of this old skool ASCII string.
 */
struct cons_pointer c_string_to_lisp_symbol(char *symbol);

#endif
