/**
 *  memory/tags.h
 *
 *  Tags for all page space and vector objects known to the bootstrap layer.
 *
 *  All macros!
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#ifndef __psse_memory_tags_h
#define __psse_memory_tags_h

#define TAGLENGTH 3

#define CONSTAG     "CNS"
#define EXCEPTIONTAG "EXP"
#define FREETAG     "FRE"
#define FUNCTIONTAG "FUN"
#define HASHTAG     "HTB"
#define INTEGERTAG  "INT"
#define KEYTAG	    "KEY"
#define LAMBDATAG   "LMD"
#define LOOPTAG     "LOP"
#define LAZYCONSTAG "LZY"
#define LAZYSTRTAG  "LZS"
#define LAZYWRKRTAG "WRK"
#define MUTEXTAG    "MTX"
#define NAMESPACETAG "NSP"
#define NILTAG      "NIL"
#define NLAMBDATAG  "NLM"
#define RATIOTAG    "RAT"
#define READTAG     "RED"
#define REALTAG     "REA"
#define SPECIALTAG  "SFM"
#define STACKTAG    "STK"
#define STRINGTAG   "STR"
#define SYMBOLTAG   "SYM"
#define TIMETAG     "TIM"
#define TRUETAG     "TRU"
#define VECTORTAG   "VEC"
#define VECTORPOINTTAG "VSP"
#define WRITETAG    "WRT"

// TODO: all these tag values are WRONG, recalculate!

#define CONSTV      5459523
#define EXCEPTIONTV 5265477
#define FREETV      4543046
#define FUNCTIONTV  5133638
#define HASHTV      4346952
#define INTEGERTV   5525065
#define KEYTV       5850443
#define LAMBDATV    4345164
#define LOOPTV      5263180
#define MUTEXTV     5788749
#define NAMESPACETV 5264206
#define NILTV       4999502
#define NLAMBDATV   5065806
#define RATIOTV     5521746
#define READTV      4474194
#define REALTV      4277586
#define SPECIALTV   5064275
#define STACKTV     4936787
#define STRINGTV    5395539
#define SYMBOLTV    5069139
#define TIMETV      5065044
#define TRUETV      5591636
#define VECTORTV    4408662
#define VECTORPOINTTV 5264214
#define WRITETV     5264214

#define consp(p)    (check_tag(p,CONSTV))
#define exceptionp(p) (check_tag(p,EXCEPTIONTV))
#define freep(p)    (check_tag(p,FREETV))
#define functionp(p) (check_tag(p,FUNCTIONTV))
#define integerp(p) (check_tag(p,INTEGERTV))
#define keywordp(p) (check_tag(p,KEYTV))
#define lambdap(p)  (check_tag(p,LAMBDATV))
#define loopp(p)    (check_tag(p,LOOPTV))
#define namespacep(p)(check_tag(p,NAMESPACETV))
// the version of nilp in ops/truth.c is better than this, because it does not
// require a fetch, and will see nils curated by other nodes as nil.
// #define nilp(p)     (check_tag(p,NILTV))
#define numberp(p)  (check_tag(p,INTEGERTV)||check_tag(p,RATIOTV)||check_tag(p,REALTV))
#define ratiop(p)   (check_tag(p,RATIOTV))
#define readp(p)    (check_tag(p,READTV))
#define realp(p)    (check_tag(p,REALTV))
#define sequencep(p) (check_tag(p,CONSTV)||check_tag(p,STRINGTV)||check_tag(p,SYMBOLTV))
#define specialp(p) (check_tag(p,SPECIALTV))
#define streamp(p)  (check_tag(p,READTV)||check_tag(p,WRITETV))
#define stringp(p)  (check_tag(p,STRINGTV))
#define symbolp(p)  (check_tag(p,SYMBOLTV))
#define timep(p)    (check_tag(p,TIMETV))
// the version of truep in ops/truth.c is better than this, because it does not
// require a fetch, and will see ntsils curated by other nodes as t.
// #define tp(p)       (check_tag(p,TRUETV))
// #define truep(p) (  !check_tag(p,NILTV))
#define vectorpointp(p) (check_tag(p,VECTORPOINTTV))
#define vectorp(p) (check_tag(p,VECTORTV))
#define writep(p)   (check_tag(p,WRITETV))

/**
 * @brief return the numerical value of the tag of the object indicated by 
 * pointer `p`.
 * 
 * @param p must be a struct pso_pointer, indicating the appropriate object.
 *
 * @return the numerical value of the tag, as a uint32_t.
 */
#define get_tag_value(p)((pointer_to_object(p)->header.tag.value) & 0xffffff)

/**
 * @brief check that the tag of the object indicated by this poiner has this 
 * value.
 *
 * @param p must be a struct pso_pointer, indicating the appropriate object.
 * @param v should be an integer, ideally uint32_t, the expected value of a tag.
 * 
 * @return true if the tag at p matches v, else false.
 */
#define check_tag(p,v) (get_tag_value(p) == v)

#endif