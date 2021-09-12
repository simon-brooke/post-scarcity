/**
 * lookup3.h
 *
 * Minimal header file wrapping Bob Jenkins' lookup3.c
 *
 *
 * (c) 2019 Simon Brooke <simon@journeyman.cc>
 * Public domain.
 */

#ifndef __lookup3_h
#define __lookup3_h

uint32_t hashword( const uint32_t * k, size_t length, uint32_t initval );

#endif
