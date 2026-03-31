/**
 *  memory/page.h
 *
 *  Page for paged space psoects.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#ifndef __psse_memory_page_h
#define __psse_memory_page_h

#include "memory/pointer.h"
#include "memory/pso2.h"
#include "memory/pso3.h"
#include "memory/pso4.h"
#include "memory/pso5.h"
#include "memory/pso6.h"
#include "memory/pso7.h"
#include "memory/pso8.h"
#include "memory/pso9.h"
#include "memory/psoa.h"
#include "memory/psob.h"
#include "memory/psoc.h"
#include "memory/psod.h"
#include "memory/psoe.h"
#include "memory/psof.h"

/** 
 * the size of a page, **in bytes**.
 */
#define PAGE_BYTES 1048576

/**
 * the number of pages we will initially allow for. For
 * convenience we'll set up an array of cons pages this big; however,
 * TODO: later we will want a mechanism for this to be able to grow
 * dynamically to the maximum we can allow.
 */
#define NPAGES 64

extern union page *pages[NPAGES];

/**
 * @brief A page is a megabyte of memory which contains objects all of which 
 * are of the same size class. 
 *
 * No page will contain both pso2s and pso4s, for example. We know what size 
 * objects are in a page by looking at the size tag of the first object, which 
 * will always be the fourth byte in the page (i.e page.bytes[3]). However, we 
 * will not normally have to worry about what size class the objects on a page 
 * are, since on creation all objects will be linked onto the freelist for 
 * their size class, they will be allocated from that free list, and on garbage
 * collection they will be returned to that freelist. 
 */
union page {
    uint8_t bytes[PAGE_BYTES];
    uint64_t words[PAGE_BYTES / 8];
    struct pso2 pso2s[PAGE_BYTES / 32];
    struct pso3 pso3s[PAGE_BYTES / 64];
    struct pso4 pso4s[PAGE_BYTES / 128];
    struct pso5 pso5s[PAGE_BYTES / 256];
    struct pso6 pso6s[PAGE_BYTES / 512];
    struct pso7 pso7s[PAGE_BYTES / 1024];
    struct pso8 pso8s[PAGE_BYTES / 2048];
    struct pso9 pso9s[PAGE_BYTES / 4096];
    struct psoa psoas[PAGE_BYTES / 8192];
    struct psob psobs[PAGE_BYTES / 16384];
    struct psoc psocs[PAGE_BYTES / 32768];
    struct psod psods[PAGE_BYTES / 65536];
    struct psoe psoes[PAGE_BYTES / 131072];
    struct psof psofs[PAGE_BYTES / 262144];
};

struct pso_pointer allocate_page( uint8_t size_class );

uint32_t get_pages_allocated();

#endif
