#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <wctype.h>

int main( int argc, char *argv[] ) {
    fwide( stdin, 1 );
    fwide( stdout, 1 );

  for (wchar_t c = fgetwc( stdin ); !feof( stdin); c = fgetwc( stdin )) {
    if (c != '\n') {
      fwprintf( stdout, L"Read character %d, %C\t", (int)c, c);
      fputwc( c, stdout);
      fputws(L"\n", stdout);
    }
  }
}
