#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TAGLENGTH 4

struct dummy {
  union {
    char bytes[TAGLENGTH];  /* the tag (type) of this cell,
                                   * considered as bytes */
    uint32_t value;         /* the tag considered as a number */
  } tag;
};

int main( int argc, char *argv[] ) {
  struct dummy *b = malloc( sizeof( struct dummy));
  struct dummy buffer = *b;

  for (int i = 1; i < argc; i++) {

    strncpy( &buffer.tag.bytes[0], argv[i], TAGLENGTH );

    printf( "%4.4s:\t%d\n", argv[i], buffer.tag.value);
  }
}
