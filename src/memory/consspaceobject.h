/*
 *  consspaceobject.h
 *
 *  Declarations common to all cons space objects.
 *
 *
 *  (c) 2017 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#ifndef __psse_consspaceobject_h
#define __psse_consspaceobject_h

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
/*
 * wide characters
 */
#include <wchar.h>
#include <wctype.h>

#include "io/fopen.h"
// #include "memory/conspage.h"


/**
 * The length of a tag, in bytes.
 */
#define TAGLENGTH 4

/*
 * tag values, all of which must be 4 bytes. Must not collide with vector space
 * tag values
 */

/**
 * An ordinary cons cell:
 */
#define CONSTAG     "CONS"

/**
 * The string `CONS`, considered as an `unsigned int`.
 * @todo tag values should be collected into an enum.
 */
#define CONSTV      1397641027

/**
 * An exception. TODO: we need a means of dealing with different classes of
 * exception, and we don't have one yet.
 */
#define EXCEPTIONTAG "EXEP"

/**
 * The string `EXEP`, considered as an `unsigned int`.
 */
#define EXCEPTIONTV 1346721861

/**
 * An unallocated cell on the free list - should never be encountered by a Lisp
 * function.
 */
#define FREETAG     "FREE"

/**
 * The string `FREE`, considered as an `unsigned int`.
 */
#define FREETV      1162170950

/**
 * An ordinary Lisp function - one whose arguments are pre-evaluated.
 * \see LAMBDATAG for interpretable functions.
 * \see SPECIALTAG for functions whose arguments are not pre-evaluated.
 */
#define FUNCTIONTAG "FUNC"

/**
 * The string `FUNC`, considered as an `unsigned int`.
 */
#define FUNCTIONTV  1129207110

/**
 * An integer number (bignums are integers).
 */
#define INTEGERTAG  "INTR"

/**
 * The string `INTR`, considered as an `unsigned int`.
 */
#define INTEGERTV   1381256777

/**
 * A keyword - an interned, self-evaluating string.
 */
#define KEYTAG	    "KEYW"

/**
 * The string `KEYW`, considered as an `unsigned int`.
 */
#define KEYTV       1465468235

/**
 * A lambda cell. Lambdas are the interpretable (source) versions of functions.
 * \see FUNCTIONTAG.
 */
#define LAMBDATAG  "LMDA"

/**
 * The string `LMDA`, considered as an `unsigned int`.
 */
#define LAMBDATV   1094995276

/**
 * A loop exit is a special kind of exception which has exactly the same
 * payload as an exception.
 */
#define LOOPXTAG    "LOOX"

/**
 * The string `LOOX`, considered as an `unsigned int`.
 */
#define LOOPXTV     1481592652

/**
 * The special cons cell at address {0,0} whose car and cdr both point to
 * itself.
 */
#define NILTAG      "NIL "

/**
 * The string `NIL `, considered as an `unsigned int`.
 */
#define NILTV       541870414

/**
 * An nlambda cell. NLambdas are the interpretable (source) versions of special
 * forms. \see SPECIALTAG.
 */
#define NLAMBDATAG  "NLMD"

/**
 * The string `NLMD`, considered as an `unsigned int`.
 */
#define NLAMBDATV   1145916494

/**
 * A rational number, stored as pointers two integers representing dividend
 * and divisor respectively.
 */
#define RATIOTAG    "RTIO"

/**
 * The string `RTIO`, considered as an `unsigned int`.
 */
#define RATIOTV     1330205778

/**
 * An open read stream.
 */
#define READTAG     "READ"

/**
 * The string `READ`, considered as an `unsigned int`.
 */
#define READTV      1145128274

/**
 * A real number, represented internally as an IEEE 754-2008 `binary64`.
 */
#define REALTAG     "REAL"

/**
 * The string `REAL`, considered as an `unsigned int`.
 */
#define REALTV      1279346002

/**
 * A special form - one whose arguments are not pre-evaluated but passed as
 * provided.
 * \see NLAMBDATAG.
 */
#define SPECIALTAG  "SPFM"

/**
 * The string `SPFM`, considered as an `unsigned int`.
 */
#define SPECIALTV   1296453715

/**
 * A string of characters, organised as a linked list.
 */
#define STRINGTAG   "STRG"

/**
 * The string `STRG`, considered as an `unsigned int`.
 */
#define STRINGTV    1196577875

/**
 * A symbol is just like a string except not self-evaluating.
 */
#define SYMBOLTAG   "SYMB"

/**
 * The string `SYMB`, considered as an `unsigned int`.
 */
#define SYMBOLTV    1112365395

/**
 * A time stamp.
 */
#define TIMETAG     "TIME"

/**
 * The string `TIME`, considered as an `unsigned int`.
 */
#define TIMETV      1162692948

/**
 * The special cons cell at address {0,1} which is canonically different
 * from NIL.
 */
#define TRUETAG     "TRUE"

/**
 * The string `TRUE`, considered as an `unsigned int`.
 */
#define TRUETV      1163219540

/**
 * A pointer to an object in vector space.
 */
#define VECTORPOINTTAG  "VECP"

/**
 * The string `VECP`, considered as an `unsigned int`.
 */
#define VECTORPOINTTV 1346585942

/**
 * An open write stream.
 */
#define WRITETAG    "WRIT"

/**
 * The string `WRIT`, considered as an `unsigned int`.
 */
#define WRITETV 1414091351

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

/**
 * given a cons_pointer as argument, return the cell.
 */
#define pointer2cell(pointer) ((conspages[pointer.page]->cell[pointer.offset]))

/**
 * true if `conspoint` points to the special cell NIL, else false
 * (there should only be one of these so it's slightly redundant).
 */
#define nilp(conspoint) (check_tag(conspoint,NILTAG))

/**
 * true if `conspoint` points to a cons cell, else false
 */
#define consp(conspoint) (check_tag(conspoint,CONSTAG))

/**
 * true if `conspoint` points to an exception, else false
 */
#define exceptionp(conspoint) (check_tag(conspoint,EXCEPTIONTAG))

/**
 * true if `conspoint` points to a function cell, else false
 */
#define functionp(conspoint) (check_tag(conspoint,FUNCTIONTAG))

/**
 * true if `conspoint` points to a keyword, else false
 */
#define keywordp(conspoint) (check_tag(conspoint,KEYTAG))

/**
 * true if `conspoint` points to a Lambda binding cell, else false
 */
#define lambdap(conspoint) (check_tag(conspoint,LAMBDATAG))

/**
 * true if `conspoint` points to a loop exit exception, else false.
 */
#define loopexitp(conspoint) (check_tag(conspoint,LOOPXTAG))

/**
 * true if `conspoint` points to a special form cell, else false
 */
#define specialp(conspoint) (check_tag(conspoint,SPECIALTAG))

/**
 * true if `conspoint` points to a string cell, else false
 */
#define stringp(conspoint) (check_tag(conspoint,STRINGTAG))

/**
 * true if `conspoint` points to a symbol cell, else false
 */
#define symbolp(conspoint) (check_tag(conspoint,SYMBOLTAG))

/**
 * true if `conspoint` points to an integer cell, else false
 */
#define integerp(conspoint) (check_tag(conspoint,INTEGERTAG))

/**
 * true if `conspoint` points to a rational number cell, else false
 */
#define ratiop(conspoint) (check_tag(conspoint,RATIOTAG))

/**
 * true if `conspoint` points to a read stream cell, else false
 */
#define readp(conspoint) (check_tag(conspoint,READTAG))

/**
 * true if `conspoint` points to a real number cell, else false
 */
#define realp(conspoint) (check_tag(conspoint,REALTAG))

/**
 * true if `conspoint` points to some sort of a number cell,
 * else false
 */
#define numberp(conspoint) (check_tag(conspoint,INTEGERTAG)||check_tag(conspoint,RATIOTAG)||check_tag(conspoint,REALTAG))

/**
 * true if `conspoint` points to a sequence (list, string or, later, vector),
 * else false.
 */
#define sequencep(conspoint) (check_tag(conspoint,CONSTAG)||check_tag(conspoint,STRINGTAG)||check_tag(conspoint,SYMBOLTAG))

/**
 * true if `conspoint` points to a vector pointer, else false.
 */
#define vectorpointp(conspoint) (check_tag(conspoint,VECTORPOINTTAG))

/**
 * true if `conspoint` points to a write stream cell, else false.
 */
#define writep(conspoint) (check_tag(conspoint,WRITETAG))

#define streamp(conspoint) (check_tag(conspoint,READTAG)||check_tag(conspoint,WRITETAG))

/**
 * true if `conspoint` points to a true cell, else false
 * (there should only be one of these so it's slightly redundant).
 * Also note that anything that is not NIL is truthy.
 */
#define tp(conspoint) (check_tag(conspoint,TRUETAG))

/**
 * true if `conspoint` points to a time cell, else false.
 */
#define timep(conspoint) (check_tag(conspoint,TIMETAG))

/**
 * true if `conspoint` points to something that is truthy, i.e.
 * anything but NIL.
 */
#define truep(conspoint) (!check_tag(conspoint,NILTAG))

/**
 * An indirect pointer to a cons cell
 */
struct cons_pointer {
   /** the index of the page on which this cell resides */
    uint32_t page;
    /** the index of the cell within the page */
    uint32_t offset;
};

/*
 * number of arguments stored in a stack frame
 */
#define args_in_frame 8

/**
 * A stack frame. Yes, I know it isn't a cons-space object, but it's defined
 * here to avoid circularity. \todo refactor.
 */
struct stack_frame {
    /** the previous frame. */
    struct cons_pointer previous;
    /** first 8 arument bindings. */
    struct cons_pointer arg[args_in_frame];
    /** list of any further argument bindings. */
    struct cons_pointer more;
    /** the function to be called. */
    struct cons_pointer function;
    /** the number of arguments provided. */
    int args;
};

/**
 * payload of a cons cell.
 */
struct cons_payload {
  /** Contents of the Address Register, naturally. */
    struct cons_pointer car;
  /** Contents of the Decrement Register, naturally. */
    struct cons_pointer cdr;
};

/**
 * Payload of an exception.
 * Message should be a Lisp string; frame should be a pointer to an (unfreed) stack frame.
 */
struct exception_payload {
  /** The payload: usually a Lisp string but in practice anything printable will do. */
    struct cons_pointer payload;
  /** pointer to the (unfreed) stack frame in which the exception was thrown. */
    struct cons_pointer frame;
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
    /**
     * pointer to metadata (e.g. the source from which the function was compiled).
     */
    struct cons_pointer meta;
    /**  pointer to a function which takes a cons pointer (representing
     * its argument list) and a cons pointer (representing its environment) and a
     * stack frame (representing the previous stack frame) as arguments and returns
     * a cons pointer (representing its result).
     * \todo check this documentation is current!
     */
    struct cons_pointer ( *executable ) ( struct stack_frame *,
                                          struct cons_pointer,
                                          struct cons_pointer );
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
 * payload of an integer cell. An integer is in principle a sequence of cells;
 * only 60 bits (+ sign bit) are actually used in each cell. If the value
 * exceeds 60 bits, the least significant 60 bits are stored in the first cell
 * in the chain, the next 60 in the next cell, and so on. Only the value of the
 * first cell in any chain should be negative.
 */
struct integer_payload {
    /** the value of the payload (i.e. 60 bits) of this cell. */
    int64_t value;
    /** the next (more significant) cell in the chain, ir `NIL` if there are no
     * more. */
    struct cons_pointer more;
};

/**
 * payload for lambda and nlambda cells.
 */
struct lambda_payload {
    /** the arument list */
    struct cons_pointer args;
    /** the body of the function to be applied to the arguments. */
    struct cons_pointer body;
};

/**
 * payload for ratio cells. Both `dividend` and `divisor` must point to integer cells.
 */
struct ratio_payload {
    /** a pointer to an integer representing the dividend */
    struct cons_pointer dividend;
    /** a pointer to an integer representing the divisor. */
    struct cons_pointer divisor;
};

/**
 * payload for a real number cell. Internals of this liable to change to give 128 bits
 * precision, but I'm not sure of the detail.
 */
struct real_payload {
    /** the value of the number */
    long double value;
};

/**
 * Payload of a special form cell. Currently identical to the payload of a
 * function cell.
 * \see function_payload
 */
struct special_payload {
    /**
     * pointer to the source from which the special form was compiled, or NIL
     * if it is a primitive.
     */
    struct cons_pointer meta;
    /**  pointer to a function which takes a cons pointer (representing
     * its argument list) and a cons pointer (representing its environment) and a
     * stack frame (representing the previous stack frame) as arguments and returns
     * a cons pointer (representing its result). */
    struct cons_pointer ( *executable ) ( struct stack_frame *,
                                          struct cons_pointer,
                                          struct cons_pointer );
};

/**
 * payload of a read or write stream cell.
 */
struct stream_payload {
    /** the stream to read from or write to. */
    URL_FILE *stream;
    /** metadata on the stream (e.g. its file attributes if a file, its HTTP
     * headers if a URL, etc). Expected to be an association, or nil. Not yet
     * implemented. */
    struct cons_pointer meta;
};

/**
 * payload of a string cell. At least at first, only one UTF character will
 * be stored in each cell. The doctrine that 'a symbol is just a string'
 * didn't work; however, the payload of a symbol or keyword cell is identical
 * to the payload of a string cell, except that a keyword may store a hash
 * of its own value in the padding.
 */
struct string_payload {
    /** the actual character stored in this cell */
    wint_t character;
    /** a hash of the string value, computed at store time. */
    uint32_t hash;
    /** the remainder of the string following this character. */
    struct cons_pointer cdr;
};

/**
 * The payload of a time cell: an unsigned 128 bit value representing micro-
 * seconds since the estimated date of the Big Bang (actually, for
 * convenience, 14Bn years before 1st Jan 1970 (the UNIX epoch))
 */
struct time_payload {
    unsigned __int128 value;
};

/**
 * payload of a vector pointer cell.
 */
struct vectorp_payload {
    /** the tag of the vector-space object. NOTE that the vector space object
     * should itself have the identical tag. */
    union {
        /** the tag (type) of the vector-space object this cell
         * points to, considered as bytes. */
        char bytes[TAGLENGTH];
        /** the tag considered as a number */
        uint32_t value;
    } tag;
    /** unused padding to word-align the address */
    uint32_t padding;
    /** the address of the actual vector space
     * object (\todo will change when I actually
     * implement vector space) */
    void *address;
};

/**
 * an object in cons space.
 */
struct cons_space_object {
    union {
        /** the tag (type) of this cell,
         * considered as bytes */
        char bytes[TAGLENGTH];
        /** the tag considered as a number */
        uint32_t value;
    } tag;
    /** the count of the number of references to this cell */
    uint32_t count;
    /** cons pointer to the access control list of this cell */
    struct cons_pointer access;
    union {
        /**
         * if tag == CONSTAG
         */
        struct cons_payload cons;
        /**
         * if tag == EXCEPTIONTAG || tag == LOOPXTAG
         */
        struct exception_payload exception;
        /**
         * if tag == FREETAG
         */
        struct free_payload free;
        /**
         * if tag == FUNCTIONTAG
         */
        struct function_payload function;
        /**
         * if tag == INTEGERTAG
         */
        struct integer_payload integer;
        /**
         * if tag == LAMBDATAG or NLAMBDATAG
         */
        struct lambda_payload lambda;
        /**
         * if tag == NILTAG; we'll treat the special cell NIL as just a cons
         */
        struct cons_payload nil;
        /**
         * if tag == RATIOTAG
         */
        struct ratio_payload ratio;
        /**
         * if tag == READTAG || tag == WRITETAG
         */
        struct stream_payload stream;
        /**
         * if tag == REALTAG
         */
        struct real_payload real;
        /**
         * if tag == SPECIALTAG
         */
        struct special_payload special;
        /**
         * if tag == STRINGTAG || tag == SYMBOLTAG
         */
        struct string_payload string;
        /**
         * if tag == TIMETAG
         */
        struct time_payload time;
        /**
         * if tag == TRUETAG; we'll treat the special cell T as just a cons
         */
        struct cons_payload t;
        /**
         * if tag == VECTORPTAG
         */
        struct vectorp_payload vectorp;
    } payload;
};

bool check_tag( struct cons_pointer pointer, char *tag );

void inc_ref( struct cons_pointer pointer );

void dec_ref( struct cons_pointer pointer );

struct cons_pointer c_type( struct cons_pointer pointer );

struct cons_pointer c_car( struct cons_pointer arg );

struct cons_pointer c_cdr( struct cons_pointer arg );

int c_length( struct cons_pointer arg);

struct cons_pointer make_cons( struct cons_pointer car,
                               struct cons_pointer cdr );

struct cons_pointer make_exception( struct cons_pointer message,
                                    struct cons_pointer frame_pointer );

struct cons_pointer make_function( struct cons_pointer src,
                                   struct cons_pointer ( *executable )
                                    ( struct stack_frame *,
                                      struct cons_pointer,
                                      struct cons_pointer ) );

struct cons_pointer c_string_to_lisp_keyword( wchar_t *symbol );

struct cons_pointer make_lambda( struct cons_pointer args,
                                 struct cons_pointer body );

struct cons_pointer make_nlambda( struct cons_pointer args,
                                  struct cons_pointer body );

struct cons_pointer make_special( struct cons_pointer src,
                                  struct cons_pointer ( *executable )
                                   ( struct stack_frame *,
                                     struct cons_pointer,
                                     struct cons_pointer ) );

struct cons_pointer make_string( wint_t c, struct cons_pointer tail );

struct cons_pointer make_symbol_or_key( wint_t c, struct cons_pointer tail,
                                        char *tag );

#define make_symbol(c, t) (make_symbol_or_key( c, t, SYMBOLTAG))

#define make_keyword(c, t) (make_symbol_or_key( c, t, KEYTAG))

struct cons_pointer make_read_stream( URL_FILE * input,
                                      struct cons_pointer metadata );

struct cons_pointer make_write_stream( URL_FILE * output,
                                       struct cons_pointer metadata );

struct cons_pointer c_string_to_lisp_string( wchar_t *string );

struct cons_pointer c_string_to_lisp_symbol( wchar_t *symbol );

#endif
