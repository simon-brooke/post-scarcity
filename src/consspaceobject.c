/**
 *  consspaceobject.c
 *
 *  Structures common to all cons space objects.
 *
 *
 *  (c) 2017 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "conspage.h"
#include "consspaceobject.h"


/**
 * increment the reference count of the object at this cons pointer.
 *
 * You can't roll over the reference count. Once it hits the maximum
 * value you cannot increment further.
 */
void incref( struct cons_pointer pointer) {
  struct cons_space_object cell = pointer2cell( pointer);

  if (cell.count < MAXREFERENCE) {
    cell.count ++;
  }
}


/**
 * Decrement the reference count of the object at this cons pointer.
 *
 * If a count has reached MAXREFERENCE it cannot be decremented.
 * If a count is decremented to zero the cell should be freed.
 */
void decref( struct cons_pointer pointer) {
  struct cons_space_object cell = pointer2cell( pointer);

  if (cell.count < MAXREFERENCE) {
    cell.count --;

    if (cell.count == 0) {
      free_cell( pointer);
    }
  }
}


/**
 * dump the object at this cons_pointer to this output stream.
 */
void dump_object( FILE* output, struct cons_pointer pointer) {
    struct cons_space_object cell = conspages[pointer.page]->cell[pointer.offset];
    char * tag = malloc( TAGLENGTH + 1);
    memset( tag, 0, TAGLENGTH + 1);
    strncpy( tag, cell.tag, TAGLENGTH);

    fprintf( output, "\tDumping object at page %d, offset %d with tag %s, count %d\n",
	     pointer.page,
	     pointer.offset,
	     tag,
	     cell.count);

    if ( strncmp( tag, CONSTAG, TAGLENGTH) == 0) {
      fprintf( output, "\tCons cell: car at page %d offset %d, cdr at page %d offset %d\n",
	       cell.payload.cons.car.page, cell.payload.cons.car.offset, cell.payload.cons.cdr.page, cell.payload.cons.cdr.offset);
    } else if ( strncmp( tag, INTEGERTAG, TAGLENGTH) == 0) {
      fprintf( output, "\t\tInteger cell: value %ld\n", cell.payload.integer.value);
    } else if ( strncmp( tag, FREETAG, TAGLENGTH) == 0) {
      fprintf( output, "\t\tFree cell: next at page %d offset %d\n",
	       cell.payload.cons.cdr.page, cell.payload.cons.cdr.offset);
    } else if ( strncmp( tag, REALTAG, TAGLENGTH) == 0) {
      fprintf( output, "\t\tReal cell: value %Lf\n", cell.payload.real.value);
    } else if ( strncmp( tag, STRINGTAG, TAGLENGTH) == 0) {
      fprintf( output, "\t\tString cell: character '%c' next at page %d offset %d\n",
	       cell.payload.string.character, cell.payload.string.cdr.page,
	       cell.payload.string.cdr.offset);
    };
}
