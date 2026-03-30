
/**
 *  psse.c
 *
 *  Post Scarcity Software Environment: entry point.
 *
 *  Start up and initialise the environement - just enough to get working
 *  and (ultimately) hand off to the executive.
 *
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

#include "psse.h"
#include "memory/node.h"
#include "ops/stack_ops.h"

void print_banner(  ) {
    fwprintf( stdout, L"Post-Scarcity Software Environment version %s\n\n",
              VERSION );
}

/**
 * Print command line options to this `stream`.
 * 
 * @stream the stream to print to.
 */
void print_options( FILE *stream ) {
    fwprintf( stream, L"Expected options are:\n" );
    fwprintf( stream,
              L"\t-d\tDump memory to standard out at end of run (copious!);\n" );
    fwprintf( stream, L"\t-h\tPrint this message and exit;\n" );
    fwprintf( stream, L"\t-p\tShow a prompt (default is no prompt);\n" );
    fwprintf( stream,
              L"\t-s LIMIT\n\t\tSet the maximum stack depth to this LIMIT (int)\n" );
#ifdef DEBUG
    fwprintf( stream,
              L"\t-v LEVEL\n\t\tSet verbosity to the specified level (0...512)\n" );
    fwprintf( stream, L"\t\tWhere bits are interpreted as follows:\n" );
    fwprintf( stream, L"\t\t1\tALLOC;\n" );
    fwprintf( stream, L"\t\t2\tARITH;\n" );
    fwprintf( stream, L"\t\t4\tBIND;\n" );
    fwprintf( stream, L"\t\t8\tBOOTSTRAP;\n" );
    fwprintf( stream, L"\t\t16\tEVAL;\n" );
    fwprintf( stream, L"\t\t32\tINPUT/OUTPUT;\n" );
    fwprintf( stream, L"\t\t64\tLAMBDA;\n" );
    fwprintf( stream, L"\t\t128\tREPL;\n" );
    fwprintf( stream, L"\t\t256\tSTACK;\n" );
    fwprintf( stream, L"\t\t512\tEQUAL.\n" );
#endif
}


 /**
 * main entry point; parse command line arguments, initialise the environment,
 * and enter the read-eval-print loop.
 */
int main( int argc, char *argv[] ) {
    int option;
    bool dump_at_end = false;
    bool show_prompt = false;
    char *infilename = NULL;

    setlocale( LC_ALL, "" );
    // if ( io_init(  ) != 0 ) {
    //     fputs( "Failed to initialise I/O subsystem\n", stderr );
    //     exit( 1 );
    // }

    while ( ( option = getopt( argc, argv, "dhi:ps:v:" ) ) != -1 ) {
        switch ( option ) {
            case 'd':
                dump_at_end = true;
                break;
            case 'h':
                print_banner(  );
                print_options( stdout );
                exit( 0 );
                break;
            case 'i':
                infilename = optarg;
                break;
            case 'p':
                show_prompt = true;
                break;
            case 's':
                stack_limit = atoi( optarg );
                break;
            case 'v':
                verbosity = atoi( optarg );
                break;
            default:
                fwprintf( stderr, L"Unexpected option %c\n", option );
                print_options( stderr );
                exit( 1 );
                break;
        }
    }

    initialise_node( 0 );

    // repl(  );

    exit( 0 );
}
