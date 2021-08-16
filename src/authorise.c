/*
 * authorised.c
 *
 * For now, a dummy authorising everything.
 * 
 * (c) 2021 Simon Brooke <simon@journeyman.cc>
 * Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include "memory/conspage.h"
#include "memory/consspaceobject.h"


/**
 * TODO: does nothing, yet. What it should do is access a magic value in the
 * runtime environment and check that it is identical to something on this `acl`
 */
struct cons_pointer authorised(struct cons_pointer target, struct cons_pointer acl) {
    if (nilp(acl)) {
        acl = pointer2cell(target).access;
    }
    return TRUE;
}

