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
        //do status, pedir ao servidor o status das tasks em execução e dos limites
    } else if (strcmp(argv[1], "proc-file") == 0) {
        //do procfile, pode ou não ter prioridade, importa? não basta mandar ao servidor e ele depois manda de volta para cá
        //nada de forks e execs, o servidor já está aberto
        //esperar pelas informações do servidor? para imprimir pending, processing e concluded
    }
    return 0;
}
