#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define DEBUG_ALLOC 1
#define DEBUG_STACK 2
#define DEBUG_ARITH 4
#define DEBUG_EVAL 8
#define DEBUG_LAMBDA 16
#define DEBUG_BOOTSTRAP 32
#define DEBUG_IO 64
#define DEBUG_REPL 128

int check_level( int v, int level, char * name) {
  int result = 0;
  if (v & level) {
    printf("\t\t%s (%d) matches;\n", name, level);
    result = 1;
  }

  return result;
}

int main( int argc, char *argv[] ) {

  for (int i = 1; i < argc; i++) {
    int v = atoi(argv[i]);

    printf("Level %d:\n", v);
    int matches = check_level(v, DEBUG_ALLOC, "DEBUG_ALLOC") +
    check_level(v, DEBUG_STACK, "DEBUG_STACK") +
    check_level(v, DEBUG_ARITH, "DEBUG_ARITH") +
    check_level(v, DEBUG_EVAL, "DEBUG_EVAL") +
    check_level(v, DEBUG_LAMBDA, "DEBUG_LAMBDA") +
    check_level(v, DEBUG_BOOTSTRAP, "DEBUG_BOOTSTRAP") +
    check_level(v, DEBUG_IO, "DEBUG_IO") +
    check_level(v, DEBUG_REPL, "DEBUG_REPL");
    printf("\t%d matches\n", matches);
  }
}
