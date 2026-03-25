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

#define PAGE_SIZE 1048576

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
    uint8_t[PAGE_SIZE] bytes;
    uint64_t[PAGE_SIZE / 8] words;
    struct pso2[PAGE_SIZE / 32] pso2s;
    struct pso3[PAGE_SIZE / 64] pso3s;
    struct pso4[PAGE_SIZE / 128] pso4s;
    struct pso5[PAGE_SIZE / 256] pso5s;
    struct pso6[PAGE_SIZE / 512] pso6s;
    struct pso7[PAGE_SIZE / 1024] pso7s;
    struct pso8[PAGE_SIZE / 2048] pso8s;
    struct pso9[PAGE_SIZE / 4096] pso9s;
    struct psoa[PAGE_SIZE / 8192] psoas;
    struct psob[PAGE_SIZE / 16384] psobs;
    struct psoc[PAGE_SIZE / 32768] psocs;
    struct psod[PAGE_SIZE / 65536] psods;
    struct psoe[PAGE_SIZE / 131072] psoes;
    struct psof[PAGE_SIZE / 262144] psofs;
};

#endif
