/**
 *  payloads/character.c
 *
 *  A character object.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

/*
 * wide characters
 */
#include <wchar.h>
#include <wctype.h>

#include "memory/node.h"
#include "memory/pointer.h"
#include "memory/pso.h"
#include "memory/pso2.h"
#include "memory/tags.h"

#include "ops/truth.h"

#include "payloads/character.h"

struct pso_pointer make_character( wint_t c) {
	struct pso_pointer result = allocate( CHARACTERTAG, 2 );

	if (!nilp(result)) {
		pointer_to_object(result)->payload.character.character = (wchar_t) c;
	}

	return result;
}
