/**
 * equal.c
 *
 * Checks for shallow and deep equality
 *
 * (c) 2017 Simon Brooke <simon@journeyman.cc>
 * Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include <math.h>
#include <stdbool.h>

#include "conspage.h"
#include "consspaceobject.h"
#include "integer.h"

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
 * Deep, and thus expensive, equality: true if these two objects have
 * identical structure, else false.
 */
bool equal( struct cons_pointer a, struct cons_pointer b ) {
    bool result = eq( a, b );

    if ( !result && same_type( a, b ) ) {
        struct cons_space_object *cell_a = &pointer2cell( a );
        struct cons_space_object *cell_b = &pointer2cell( b );

        switch ( cell_a->tag.value ) {
        case CONSTV:
            result =
                equal( cell_a->payload.cons.car, cell_b->payload.cons.car )
                && equal( cell_a->payload.cons.cdr, cell_b->payload.cons.cdr );
            break;
        case STRINGTV:
        case SYMBOLTV:
            /*
             * slightly complex because a string may or may not have a '\0'
             * cell at the end, but I'll ignore that for now. I think in
             * practice only the empty string will. 
             */
            result =
                cell_a->payload.string.character ==
                cell_b->payload.string.character
                && equal( cell_a->payload.string.cdr,
                          cell_b->payload.string.cdr );
            break;
        case INTEGERTV:
        case REALTV:
            {
                double num_a = numeric_value( a );
                double num_b = numeric_value( b );
                double max =
                    fabs( num_a ) >
                    fabs( num_b ) ? fabs( num_a ) : fabs( num_b );

                /*
                 * not more different than one part in a million - close enough 
                 */
                result = fabs( num_a - num_b ) < ( max / 1000000.0 );
            }
            break;
        default:
            result = false;
            break;
        }

        /*
         * there's only supposed ever to be one T and one NIL cell, so each
         * should be caught by eq; equality of vector-space objects is a whole 
         * other ball game so we won't deal with it now (and indeedmay never). 
         * I'm not certain what equality means for read and write streams, so
         * I'll ignore them, too, for now.
         */
    }

    return result;
}
