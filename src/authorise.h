/*
 * authorise.h
 *
 * Basic implementation of a authorisation.
 *
 * (c) 2021 Simon Brooke <simon@journeyman.cc>
 * Licensed under GPL version 2.0, or, at your option, any later version.
 */

#ifndef __psse_authorise_h
#define __psse_authorise_h

struct cons_pointer authorised(struct cons_pointer target, struct cons_pointer acl);

#endif