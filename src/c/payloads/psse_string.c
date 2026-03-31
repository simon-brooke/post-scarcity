/**
 *  payloads/string.c
 *
 *  A string cell.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */


#include <stdint.h>

 /*
 * wide characters
 */
#include <wchar.h>
#include <wctype.h>

#include "memory/node.h"
#include "memory/pointer.h"
#include "memory/pso2.h"
#include "memory/tags.h"

#include "ops/string_ops.h"
#include "ops/truth.h"
