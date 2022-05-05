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
        write(1, "./sdstore proc-file <-p priority> input-filename output-filename transformation-id-1 transformation-id-2 ...\n", 105);
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
        char *string, prio[2] = "0";
        int i = 2, n;
        if (strcmp(argv[2], "-p") == 0) {
            n = atoi(argv[3]);
            if (n >= 0 && n < 5)
                prio[0] = '0'+n;
            i = 4;
        }

        argv[i-2] = "proc-file";
        argv[i-1] = prio;
        n = strArrayToString(argc-i+2, argv+i-2, &string); //testar erro?
        int fd_escrita;
        if((fd_escrita = open("entrada", O_WRONLY)) == -1){
            write(2, "Failed to open the named pipe\n", 31);
            exit(-1);
        }
        write(fd_escrita, string, strlen(string));
        close(fd_escrita); // fechar por agora
        free(string);
        //vai ler e escrever 3 vezes
        int bytes_read;
        char buffer[32];
        for (i = 0; i<2;i++) {
            //read pipe com nome NÃO LÊ NADA QUE ISTO DÁ ASNEIRA tem de ler do pipe com nome criado aqui
            write(1, buffer, bytes_read);
        }
        //read pipe da conclusão
        
    }
    return 0;
}
