#include "sdstore.h"

int main(int argc, char const *argv[])
{
    //quantos argumentos são precisos?
    //0 -> help
    //1 -> status
    //1+ -> procfile
    if (argc == 1) {
        write(1, "./sdstore status\n", 18);
        write(1, "./sdstore proc-file priority input-filename output-filename transformation-id-1 transformation-id-2 ...\n", 105);
        return 0;
    } else if (strcmp(argv[1], "status") == 0) {
        //do status
    } else if (strcmp(argv[1], "proc-file") == 0) {
        //do procfile
    }
    return 0;
}
