/**
 *  payloads/character.h
 *
 *  A character object.
 *
 *  Note that, instead of instantiating actual character objects, it would be
 *  possible to reserve one special page index, outside the normal page range,
 *  possibly even page 0, such that a character would be represented by a
 *  pso_pointer {node, special_page, character_code}. The special page wouldn't
 *  actually have to exist. This wouldn't prevent `nil` being 'the object at
 *  {n, 0, 0}, since the UTF character with the index 0 is NULL, which feels
 *  entirely compatible. UTF 1 is 'Start of heading', which is not used by
 *  anything I'm aware of these days, and is canonically not NULL, so I don't
 *  see why we should not continue to treat {n, 0, 1} as `t`.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#ifndef __psse_payloads_character_h
#define __psse_payloads_character_h
/*
 * wide characters
 */
#include <wchar.h>
#include <wctype.h>


#define CHARTAG "CHR"
#define CHARTV  5392451

/**
 * @brief a single character, as returned by the reader.
 */
struct character_payload {
    wchar_t character;
};

struct pso_pointer make_character( wint_t c );
#endif
