#include "sdstore.h"

int main(int argc, char const *argv[])
{
    //quantos argumentos são precisos?
    //0 -> help
    //1 -> status
    //2+ -> procfile
    switch (argc)
    {
    case 1:
        write(1, "./sdstore status\n", 18);
        write(1, "./sdstore proc-file priority input-filename output-filename transformation-id-1 transformation-id-2 ...\n", 105);
        return 0;
    
    default:
        break;
    }
    return 0;
}
