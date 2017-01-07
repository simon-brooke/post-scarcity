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
 * Check that the tag on the cell at this pointer is this tag
 */
int check_tag( struct cons_pointer pointer, char* tag) {
  struct cons_space_object cell = pointer2cell(pointer);
  return strncmp( &cell.tag.bytes[0], tag, TAGLENGTH) == 0;
}


/**
 * increment the reference count of the object at this cons pointer.
 *
 * You can't roll over the reference count. Once it hits the maximum
 * value you cannot increment further.
 */
void inc_ref( struct cons_pointer pointer) {
  struct cons_space_object* cell = &pointer2cell( pointer);

  if (cell->count < MAXREFERENCE) {
    cell->count ++;
  }
}


/**
 * Decrement the reference count of the object at this cons pointer.
 *
 * If a count has reached MAXREFERENCE it cannot be decremented.
 * If a count is decremented to zero the cell should be freed.
 */
void dec_ref( struct cons_pointer pointer) {
  struct cons_space_object* cell = &pointer2cell( pointer);

  if (cell->count <= MAXREFERENCE) {
    cell->count --;

    if (cell->count == 0) {
      free_cell( pointer);
    }
  }
}


/**
 * dump the object at this cons_pointer to this output stream.
 */
void dump_object( FILE* output, struct cons_pointer pointer) {
  struct cons_space_object cell = pointer2cell(pointer);
  fprintf( output,
	   "\tDumping object at page %d, offset %d with tag %c%c%c%c (%d), count %u\n",
	   pointer.page,
	   pointer.offset,
	   cell.tag.bytes[0],
	   cell.tag.bytes[1],
	   cell.tag.bytes[2],
	   cell.tag.bytes[3],
	   cell.tag.value,
	   cell.count);

  if ( check_tag(pointer, CONSTAG)) {
    fprintf( output,
	     "\tCons cell: car at page %d offset %d, cdr at page %d offset %d\n",
	     cell.payload.cons.car.page, cell.payload.cons.car.offset,
	     cell.payload.cons.cdr.page, cell.payload.cons.cdr.offset);
  } else if ( check_tag(pointer, INTEGERTAG)) {
    fprintf( output, "\t\tInteger cell: value %ld\n", cell.payload.integer.value);
  } else if ( check_tag( pointer, FREETAG)) {
    fprintf( output, "\t\tFree cell: next at page %d offset %d\n",
	     cell.payload.cons.cdr.page, cell.payload.cons.cdr.offset);
  } else if ( check_tag(pointer, REALTAG)) {
    fprintf( output, "\t\tReal cell: value %Lf\n", cell.payload.real.value);
  } else if ( check_tag( pointer, STRINGTAG)) {
    fprintf( output, "\t\tString cell: character '%c' next at page %d offset %d\n",
	     cell.payload.string.character, cell.payload.string.cdr.page,
	     cell.payload.string.cdr.offset);
  };
}


/**
 * Construct a cons cell from this pair of pointers.
 */
struct cons_pointer make_cons( struct cons_pointer car, struct cons_pointer cdr) {
  struct cons_pointer pointer = allocate_cell( CONSTAG);

  struct cons_space_object* cell = &conspages[pointer.page]->cell[pointer.offset];

  inc_ref(car);
  inc_ref(cdr);
  cell->payload.cons.car = car;
  cell->payload.cons.cdr = cdr;

  return pointer;
}
  
/**
 * Construct a string from this character (which later will be UTF) and
 * this tail. A string is implemented as a flat list of cells each of which
 * has one character and a pointer to the next; in the last cell the 
 * pointer to next is NIL.
 */
struct cons_pointer make_string( char c, struct cons_pointer tail) {
  struct cons_pointer pointer = NIL;
  
  if ( check_tag( tail, STRINGTAG) || check_tag( tail, NILTAG)) {
    pointer = allocate_cell( STRINGTAG);
    struct cons_space_object* cell = &conspages[pointer.page]->cell[pointer.offset];

    inc_ref(tail);
    cell->payload.string.character = (uint32_t) c;
    cell->payload.string.cdr = tail;
  } else {
    fprintf( stderr, "Warning: only NIL and STRING can be appended to STRING\n");
  }
  
  return pointer;
}
