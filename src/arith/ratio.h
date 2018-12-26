/**
 * ratio.h
 *
 * functions for rational number cells.
 *
 *
 * (c) 2017 Simon Brooke <simon@journeyman.cc>
 * Licensed under GPL version 2.0, or, at your option, any later version.
 */

#ifndef __ratio_h
#define __ratio_h

struct cons_pointer simplify_ratio( struct cons_pointer frame_pointer,
                                    struct cons_pointer arg );

struct cons_pointer add_ratio_ratio( struct cons_pointer frame_pointer,
                                     struct cons_pointer arg1,
                                     struct cons_pointer arg2 );

struct cons_pointer add_integer_ratio( struct cons_pointer frame_pointer,
                                       struct cons_pointer intarg,
                                       struct cons_pointer ratarg );

struct cons_pointer divide_ratio_ratio( struct cons_pointer frame_pointer,
                                        struct cons_pointer arg1,
                                        struct cons_pointer arg2 );

struct cons_pointer multiply_ratio_ratio( struct cons_pointer frame_pointer, struct
                                          cons_pointer arg1, struct
                                          cons_pointer arg2 );

struct cons_pointer multiply_integer_ratio( struct cons_pointer frame_pointer,
                                            struct cons_pointer intarg,
                                            struct cons_pointer ratarg );

struct cons_pointer subtract_ratio_ratio( struct cons_pointer frame_pointer,
                                          struct cons_pointer arg1,
                                          struct cons_pointer arg2 );

struct cons_pointer make_ratio( struct cons_pointer frame_pointer,
                                struct cons_pointer dividend,
                                struct cons_pointer divisor );


#endif
