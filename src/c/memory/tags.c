/**
 *  memory/tags.h
 *
 *  It would be nice if I could get the macros for tsg operations to work,
 *  but at present they don't and they're costing me time. So I'm going to
 *  redo them as functions.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "memory/pointer.h"
#include "memory/pso2.h"

uint32_t get_tag_value (struct pso_pointer p) {
	struct pso2* object = pointer_to_object( p);

	return object->header.tag.value & 0xffffff;
}

/**
 * @brief check that the tag of the object indicated by this poiner has this
 * value.
 *
 * @param p must be a struct pso_pointer, indicating the appropriate object.
 * @param v should be an integer, ideally uint32_t, the expected value of a tag.
 *
 * @return true if the tag at p matches v, else false.
 */
bool check_tag( struct pso_pointer p, uint32_t v) {
	return get_tag_value(p) == v;
}

/**
 * @brief Like check_tag, q.v., but comparing with the string value of the tag
 * rather than the integer value. Only the first TAGLENGTH characters of `s` 
 * are considered.
 * 
 * @param p a pointer to an object;
 * @param s a string, in C conventions;
 * @return true if the first TAGLENGTH characters of `s` are equal to the tag 
 *			of the object.
 * @return false otherwise.
 */
bool check_type( struct pso_pointer p, char* s) {
	return (strncmp( 
		&(pointer_to_object(p)->header.tag.bytes.mnemonic[0]), s, TAGLENGTH)
		 == 0);
}