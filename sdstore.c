#include "sdstore.h"
#include "init.h"
/**
 *
 * @param argc
 * @param argv
 * @return
 */
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
    } else if (argc == 2 && strcmp(argv[1], "status") == 0) {
        //do status, pedir ao servidor o status das tasks em execução e dos limites
        //o server que imprima as suas cenas, I'm done here.
    } else if (argc >= 5 && strcmp(argv[1], "proc-file") == 0) {
        //do procfile, pode ou não ter prioridade, importa? não basta mandar ao servidor e ele depois manda de volta para cá
        //nada de forks e execs, o servidor já está aberto
        //esperar pelas informações do servidor? para imprimir pending, processing e concluded. Ler do pipe com nome???
        //open pipe com nome

        //escrever primeiro um int que é o n_transfs
        //como escrever o que recebeu?
            //literalmente só escrever no pipe...oq?
                //array argv pimba para o pipe (de uma vez?)
                //o client dá parse para um Pedido e escreve em bytes lá
        char *string;
        int i, n;
        n = atoi(argv[2]);
        if (n < 1 || n > MAX_PRIORITY) {
            write(2, "Missing priority.",18);
            return -1;
        }
        n = strArrayToString(argc-1, argv+1, &string, 1); //testar erro?
        //write(pipe, string, strlen(string));
        free(string);
        
        //vai ler e escrever 3 vezes
        int bytes_read;
        char buffer[32];
        for (i = 0; i<2;i++) {
            //read pipe com nome NÃO LÊ NADA QUE ISTO DÁ ASNEIRA
            write(1, buffer, bytes_read);
        }
        //read pipe da conclusão
        
    }
    return 0;
}
