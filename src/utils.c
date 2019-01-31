/*
 * utils.c
 *
 * little generally useful functions which aren't in any way special to PSSE.
 *
 * (c) 2019 Simon Brooke <simon@journeyman.cc>
 * Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include <ctype.h>
#include <stdlib.h>
#include <string.h>


int index_of( char c, char *s ) {
    int i;

    for ( i = 0; s[i] != c && s[i] != 0; i++ );

    return s[i] == c ? i : -1;
}

char *trim( char *s ) {
    int i;

    for ( i = strlen( s ); ( isblank( s[i] ) || iscntrl( s[i] ) ) && i >= 0;
          i-- ) {
        s[i] = '\0';
    }
    for ( i = 0; ( isblank( s[i] ) || iscntrl( s[i] ) ) && s[i] != '\0'; i++ );

    return ( char * ) &s[i];
}
