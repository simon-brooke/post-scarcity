#include <stdint.h>
#include <stdio.h>

int main( int argc, char *argv[] ) {

  for (int i = 1; i < argc; i++) {
    printf( "%4.4s:\t%u\n", argv[i], (uint32_t)*argv[i]);
  }
}
