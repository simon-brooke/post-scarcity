/**
 * conspage.c
 *
 * Setup and tear down cons pages, and (FOR NOW) do primitive 
 * allocation/deallocation of cells.
 * NOTE THAT before we go multi-threaded, these functions must be
 * aggressively
 * thread safe.
 *
 * (c) 2017 Simon Brooke <simon@journeyman.cc>
 * Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "consspaceobject.h"
#include "conspage.h"



/**
 * Flag indicating whether conspage initialisation has been done.
 */
bool conspageinitihasbeencalled = false;

/**
 * the number of cons pages which have thus far been initialised.
 */
int initialisedconspages = 0;

/**
 * The (global) pointer to the (global) freelist. Not sure whether this ultimately 
 * belongs in this file.
 */
struct cons_pointer freelist = NIL;

/**
 * An array of pointers to cons pages.
 */
struct cons_page* conspages[NCONSPAGES];


/**
 * Make a cons page whose serial number (i.e. index in the conspages directory) is pageno.
 * Initialise all cells and prepend each to the freelist; if pageno is zero, do not prepend
 * cells 0 and 1 to the freelist but initialise them as NIL and T respectively.
 */
void makeconspage() {
  struct cons_page* result = malloc( sizeof( struct cons_page));

  if ( result != NULL) {
    for (int i = 0; i < CONSPAGESIZE; i++) {
      if ( initialisedconspages == 0 && i < 2) {
        if ( i == 0) {
          /* initialise cell as NIL */
          strncpy( result->cell[i].tag, NILTAG, TAGLENGTH);
          result->cell[i].count = MAXREFERENCE;
          result->cell[i].payload.free.car = NIL;
          result->cell[i].payload.free.cdr = NIL;
        } else if ( i == 1) {
          /* initialise cell as T */
          strncpy( result->cell[i].tag, TRUETAG, TAGLENGTH);
          result->cell[i].count = MAXREFERENCE;
          result->cell[i].payload.free.car = (struct cons_pointer){ 0, 1};
          result->cell[i].payload.free.cdr = (struct cons_pointer){ 0, 1};
        }
      }

      /* otherwise, standard initialisation */
      strncpy( result->cell[i].tag, FREETAG, TAGLENGTH);
      result->cell[i].payload.free.car = NIL;
      result->cell[i].payload.free.cdr = freelist;
      freelist.page = initialisedconspages;
      freelist.offset = i;
    }
  } else {
    fprintf( stderr, "FATAL: Failed to allocate memory for cons page %d\n", initialisedconspages);
    exit(1);
  }

  conspages[initialisedconspages] = result;
  initialisedconspages++;
}


/**
 * dump the allocated pages to this output stream.
 */
void dumppages( FILE* output) {
  for ( int i = 0; i < initialisedconspages; i++) {
    fprintf( output, "\nDUMPING PAGE %d\n", i);

    for ( int j = 0; j < CONSPAGESIZE; j++) {
      dump_object( output, (struct cons_pointer){i, j});
    }
  }
}


/**
 * Frees the cell at the specified pointer. Dangerous, primitive, low
 * level.
 *
 * @pointer the cell to free
 */
void free_cell(struct cons_pointer pointer) {
    struct cons_space_object cell = conspages[pointer.page]->cell[pointer.offset];

    if ( strncmp( cell.tag, FREETAG, 4) != 0) {
      if ( cell.count == 0) {
        strncpy( cell.tag, FREETAG, 4);
        cell.payload.free.car = NIL;
        cell.payload.free.cdr = freelist;
        freelist = pointer;
      } else {
        fprintf( stderr, "Attempt to free cell with %d dangling references at page %d, offset %d\n",
                 cell.count, pointer.page, pointer.offset);
      }
    } else {
      fprintf( stderr, "Attempt to free cell which is already FREE at page %d, offset %d\n",
               pointer.page, pointer.offset);
    }      
}


/**
 * Allocates a cell with the specified tag. Dangerous, primitive, low
 * level.
 *
 * @param tag the tag of the cell to allocate - must be a valid cons space tag.
 * @return the cons pointer which refers to the cell allocated.
 */
struct cons_pointer allocatecell( char* tag) {
  struct cons_pointer result = freelist;

  if ( result.page == NIL.page && result.offset == NIL.offset) {
    makeconspage();
    result = allocatecell( tag);
  } else {
    struct cons_space_object cell = conspages[result.page]->cell[result.offset];

    freelist = cell.payload.free.cdr;

    fprintf( stderr, "Before: %c\n", cell.tag[0]);
    strncpy( cell.tag, tag, 4);
    fprintf( stderr, "After: %c\n", cell.tag[0]);
    
    cell.count = 1;
    cell.payload.cons.car = NIL;
    cell.payload.cons.cdr = NIL;

    fprintf( stderr, "Allocated cell of type '%s' at %d, %d \n",
	     tag, result.page, result.offset);
    dump_object( stderr, result);
  }

  return result;
}


/**
 * initialise the cons page system; to be called exactly once during startup.
 */
void conspagesinit() {
  if ( conspageinitihasbeencalled == false) {
    for (int i = 0; i < NCONSPAGES; i++) {
      conspages[i] = (struct cons_page *) NULL;
    }

    makeconspage();
    conspageinitihasbeencalled = true;
  } else {
    fprintf( stderr, "WARNING: conspageinit() called a second or subsequent time\n");
  }
}
