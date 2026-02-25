/*
 * equal.c
 *
 * Checks for shallow and deep equality
 *
 * (c) 2017 Simon Brooke <simon@journeyman.cc>
 * Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include <math.h>
#include <stdbool.h>

#include "arith/integer.h"
#include "arith/peano.h"
#include "arith/ratio.h"
#include "debug.h"
#include "memory/conspage.h"
#include "memory/consspaceobject.h"
#include "memory/vectorspace.h"
#include "ops/equal.h"
#include "ops/intern.h"

/**
 * Shallow, and thus cheap, equality: true if these two objects are
 * the same object, else false.
 */
bool eq( struct cons_pointer a, struct cons_pointer b ) {
    return ( ( a.page == b.page ) && ( a.offset == b.offset ) );
}

/**
 * True if the objects at these two cons pointers have the same tag, else false.
 * @param a a pointer to a cons-space object;
 * @param b another pointer to a cons-space object.
 * @return true if the objects at these two cons pointers have the same tag,
 * else false.
 */
bool same_type( struct cons_pointer a, struct cons_pointer b ) {
    struct cons_space_object *cell_a = &pointer2cell( a );
    struct cons_space_object *cell_b = &pointer2cell( b );

    return cell_a->tag.value == cell_b->tag.value;
}

/**
 * Some strings will be null terminated and some will be NIL terminated... ooops!
 * @param string the string to test
 * @return true if it's the end of a string.
 */
bool end_of_string( struct cons_pointer string ) {
    return nilp( string ) ||
        pointer2cell( string ).payload.string.character == '\0';
}

/**
 * @brief compare two long doubles and returns true if they are the same to
 * within a tolerance of one part in a billion.
 * 
 * @param a 
 * @param b 
 * @return true if `a` and `b` are equal to within one part in a billion.
 * @return false otherwise.
 */
bool equal_ld_ld( long double a, long double b ) {
    long double fa = fabsl( a );
    long double fb = fabsl( b );
    /* difference of magnitudes */
    long double diff = fabsl( fa - fb );
    /* average magnitude of the two */
    long double av = ( fa > fb ) ? ( fa - diff ) : ( fb - diff );
    /* amount of difference we will tolerate for equality */
    long double tolerance = av * 0.000000001;

    bool result = ( fabsl( a - b ) < tolerance );

    debug_printf( DEBUG_ARITH, L"\nequal_ld_ld returning %d\n", result );

    return result;
}

/**
 * @brief Private function, don't use. It depends on its arguments being 
 * numbers and doesn't sanity check them.
 * 
 * @param a a lisp integer -- if it isn't an integer, things will break.
 * @param b a lisp real -- if it isn't a real, things will break.
 * @return true if the two numbers have equal value.
 * @return false if they don't.
 */
bool equal_integer_real( struct cons_pointer a, struct cons_pointer b ) {
    debug_print( L"\nequal_integer_real: ", DEBUG_ARITH );
    debug_print_object( a, DEBUG_ARITH );
    debug_print( L" = ", DEBUG_ARITH );
    debug_print_object( b, DEBUG_ARITH );
    bool result = false;
    struct cons_space_object *cell_a = &pointer2cell( a );
    struct cons_space_object *cell_b = &pointer2cell( b );

    if ( nilp( cell_a->payload.integer.more ) ) {
        result =
            equal_ld_ld( ( long double ) cell_a->payload.integer.value,
                         cell_b->payload.real.value );
    } else {
        fwprintf( stderr,
                  L"\nequality is not yet implemented for bignums compared to reals." );
    }

    debug_printf( DEBUG_ARITH, L"\nequal_integer_real returning %d\n",
                  result );

    return result;
}

/**
 * @brief Private function, don't use. It depends on its arguments being 
 * numbers and doesn't sanity check them.
 * 
 * @param a a lisp integer -- if it isn't an integer, things will break.
 * @param b a lisp number.
 * @return true if the two numbers have equal value.
 * @return false if they don't.
 */
bool equal_integer_number( struct cons_pointer a, struct cons_pointer b ) {
    debug_print( L"\nequal_integer_number: ", DEBUG_ARITH );
    debug_print_object( a, DEBUG_ARITH );
    debug_print( L" = ", DEBUG_ARITH );
    debug_print_object( b, DEBUG_ARITH );
    bool result = false;
    struct cons_space_object *cell_b = &pointer2cell( b );

    switch ( cell_b->tag.value ) {
        case INTEGERTV:
            result = equal_integer_integer( a, b );
            break;
        case REALTV:
            result = equal_integer_real( a, b );
            break;
        case RATIOTV:
            result = false;
            break;
    }

    debug_printf( DEBUG_ARITH, L"\nequal_integer_number returning %d\n",
                  result );

    return result;
}

/**
 * @brief Private function, don't use. It depends on its arguments being 
 * numbers and doesn't sanity check them.
 * 
 * @param a a lisp real -- if it isn't an real, things will break.
 * @param b a lisp number.
 * @return true if the two numbers have equal value.
 * @return false if they don't.
 */
bool equal_real_number( struct cons_pointer a, struct cons_pointer b ) {
    debug_print( L"\nequal_real_number: ", DEBUG_ARITH );
    debug_print_object( a, DEBUG_ARITH );
    debug_print( L" = ", DEBUG_ARITH );
    debug_print_object( b, DEBUG_ARITH );
    bool result = false;
    struct cons_space_object *cell_b = &pointer2cell( b );

    switch ( cell_b->tag.value ) {
        case INTEGERTV:
            result = equal_integer_real( b, a );
            break;
        case REALTV:{
                struct cons_space_object *cell_a = &pointer2cell( a );
                result =
                    equal_ld_ld( cell_a->payload.real.value,
                                 cell_b->payload.real.value );
            }
            break;
        case RATIOTV:
            struct cons_space_object *cell_a = &pointer2cell( a );
            result =
                equal_ld_ld( c_ratio_to_ld( b ), cell_a->payload.real.value );
            break;
    }

    debug_printf( DEBUG_ARITH, L"\nequal_real_number returning %d\n", result );

    return result;
}

/**
 * @brief Private function, don't use. It depends on its arguments being 
 * numbers and doesn't sanity check them.
 * 
 * @param a a number
 * @param b a number
 * @return true if the two numbers have equal value.
 * @return false if they don't.
 */
bool equal_number_number( struct cons_pointer a, struct cons_pointer b ) {
    bool result = eq( a, b );

    debug_print( L"\nequal_number_number: ", DEBUG_ARITH );
    debug_print_object( a, DEBUG_ARITH );
    debug_print( L" = ", DEBUG_ARITH );
    debug_print_object( b, DEBUG_ARITH );

    if ( !result ) {
        struct cons_space_object *cell_a = &pointer2cell( a );
        struct cons_space_object *cell_b = &pointer2cell( b );

        switch ( cell_a->tag.value ) {
            case INTEGERTV:
                result = equal_integer_number( a, b );
                break;
            case REALTV:
                result = equal_real_number( a, b );
                break;
            case RATIOTV:
                switch ( cell_b->tag.value ) {
                    case INTEGERTV:
                        /* as ratios are simplified by make_ratio, any
                         * ratio that would simplify to an integer is an
                         * integer, TODO: no longer always true. */
                        result = false;
                        break;
                    case REALTV:
                        result = equal_real_number( b, a );
                        break;
                    case RATIOTV:
                        result = equal_ratio_ratio( a, b );
                        break;
                        /* can't throw an exception from here, but non-numbers
                         * shouldn't have been passed in anyway, so no default. */
                }
                break;
                /* can't throw an exception from here, but non-numbers
                 * shouldn't have been passed in anyway, so no default. */
        }
    }

    debug_printf( DEBUG_ARITH, L"\nequal_number_number returning %d\n",
                  result );

    return result;
}

/**
 * @brief equality of two map-like things. 
 *
 * The list returned by `keys` on a map-like thing is not sorted, and is not 
 * guaranteed always to come out in the same order. So equality is established
 * if:
 * 1. the length of the keys list is the same; and 
 * 2. the value of each key in the keys list for map `a` is the same in map `a` 
 *    and in map `b`.
 *
 * Private function, do not use outside this file, **WILL NOT** work 
 * unless both arguments are VECPs.
 * 
 * @param a a pointer to a vector space object.
 * @param b another pointer to a vector space object.
 * @return true if the two objects have the same logical structure.
 * @return false otherwise.
 */
bool equal_map_map( struct cons_pointer a, struct cons_pointer b ) {
    bool result=false;

    struct cons_pointer keys_a = hashmap_keys( a);
    
    if ( c_length( keys_a) == c_length( hashmap_keys( b))) {
        result = true;

        for ( struct cons_pointer i = keys_a; !nilp( i); i = c_cdr( i)) {
            struct cons_pointer key = c_car( i);
            if ( !equal( hashmap_get( a, key),hashmap_get( b, key))) {
                result = false; break;
            }
        }
    }

    return result;
}

/**
 * @brief equality of two vector-space things. 
 *
 * Expensive, but we need to be able to check for equality of at least hashmaps
 * and namespaces.
 *
 * Private function, do not use outside this file, not guaranteed to work 
 * unless both arguments are VECPs pointing to map like things.
 * 
 * @param a a pointer to a vector space object.
 * @param b another pointer to a vector space object.
 * @return true if the two objects have the same logical structure.
 * @return false otherwise.
 */
bool equal_vector_vector( struct cons_pointer a, struct cons_pointer b ) {
    bool result = false;

    if ( eq( a, b)) {
        result = true; // same 
        /* there shouldn't ever be two separate VECP cells which point to the
         * same address in vector space, so I don't believe it's worth checking
         * for this.
         */
    } else if ( vectorp( a) && vectorp( b)) {
        struct vector_space_object * va = pointer_to_vso( a);
        struct vector_space_object * vb = pointer_to_vso( b);

        /* what we're saying here is that a namespace is not equal to a map,
         * even if they have identical logical structure. Is this right? */
        if ( va->header.tag.value == vb->header.tag.value) {
            switch ( va->header.tag.value) {
                case HASHTV:
                case NAMESPACETV:
                    result = equal_map_map( a, b);
                    break;
            }
        }
    }
    // else can't throw an exception from here but TODO: should log.

    return result;
}

/**
 * Deep, and thus expensive, equality: true if these two objects have
 * identical structure, else false.
 */
bool equal( struct cons_pointer a, struct cons_pointer b ) {
    debug_print( L"\nequal: ", DEBUG_ARITH );
    debug_print_object( a, DEBUG_ARITH );
    debug_print( L" = ", DEBUG_ARITH );
    debug_print_object( b, DEBUG_ARITH );

    bool result = false; 
    
    if ( eq( a, b )) {
        result = true;
    } else if ( !numberp( a ) && same_type( a, b ) ) {
        struct cons_space_object *cell_a = &pointer2cell( a );
        struct cons_space_object *cell_b = &pointer2cell( b );

        switch ( cell_a->tag.value ) {
            case CONSTV:
            case LAMBDATV:
            case NLAMBDATV:
                /* TODO: it is not OK to do this on the stack since list-like 
                 * structures can be of indefinite extent. It *must* be done by 
                 * iteration (and even that is problematic) */
                result =
                    equal( cell_a->payload.cons.car, cell_b->payload.cons.car )
                    && equal( cell_a->payload.cons.cdr,
                              cell_b->payload.cons.cdr );
                break;
            case KEYTV:
            case STRINGTV:
            case SYMBOLTV:
                /* slightly complex because a string may or may not have a '\0'
                 * cell at the end, but I'll ignore that for now. I think in
                 * practice only the empty string will.
                 */
                /* TODO: it is not OK to do this on the stack since list-like 
                 * structures can be of indefinite extent. It *must* be done by 
                 * iteration (and even that is problematic) */
#ifdef DEBUG
                 debug_print( L"Comparing '", DEBUG_ARITH);
                 debug_print_object( a, DEBUG_ARITH);
                 debug_print( L"' to '", DEBUG_ARITH);
                 debug_print_object( b, DEBUG_ARITH);
#endif
                result =
                    cell_a->payload.string.hash == cell_b->payload.string.hash
                    && cell_a->payload.string.character ==
                    cell_b->payload.string.character
                    &&
                    ( equal
                      ( cell_a->payload.string.cdr,
                        cell_b->payload.string.cdr )
                      || ( end_of_string( cell_a->payload.string.cdr )
                           && end_of_string( cell_b->payload.string.cdr ) ) );
                break;
            case VECTORPOINTTV:
                if ( cell_b->tag.value == VECTORPOINTTV) {
                    result = equal_vector_vector( a, b);
                } else {
                    result = false;
                }
                break;
            default:
                result = false;
                break;
        }
    } else if ( numberp( a ) && numberp( b ) ) {
        result = equal_number_number( a, b );
    }

    /*
     * there's only supposed ever to be one T and one NIL cell, so each
     * should be caught by eq.
     *
     * I'm not certain what equality means for read and write streams, so
     * I'll ignore them, too, for now.
     */

    debug_printf( DEBUG_ARITH, L"\nequal returning %d\n", result );

    return result;
}
