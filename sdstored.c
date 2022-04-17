#include "sdstored.h"

//estrutura de dados para implementar o dicionário dos limites
//como determinar o número? começar com hardcode a 13

int main(int argc, char const *argv[])
{
    //o servidor é executado com o config e com a pasta
    //todo teste para ver se não nos estão a tentar executar o server maliciosamente
    int fdConfig;
    if (argc < 3 || (fdConfig = open(argv[1], O_RDONLY)) == -1) {
        //como validar um path (pasta)?
        perror("Failed to execute server");
        return -1;
    }

    char pasta[] = argv[2];
    //lembrar da pasta em algum sítio para os executáveis
    //eventualmente fazer sprintf("%s/%s", nome da pasta, nome da transforamação)

    //todo preencher o dicionário com 1º argumento
    //parse desse ficheiro .config: readln c/ sequencial até ' '

    //depois do servidor ser executado, fica à espera de ler do pipe com nome a instrução

    //não esquecer de fazer o status
        //tem diferente input
        //apenas imprime o que está a ser processado (não o que está à espera)
        //muito provavelmente vamos ter de tirar da lista/queue/heap ready e pôr numa lista "in proccessing"
            //fica muito mais fácil de saber quais os que contam contra os maxs e depois tirar quando acabarem

    //suponhamos que o servidor sabe prioridade, init file, transfs e file final
    //1ª coisa que ele faz:
    //ler todas as transfs e guardar num dicionário quantas são de cada
    
    //ver se nenhuma delas excede o valor max == nunca vai ser executado => atirar para o lixo oops (não encher a queue)
        //nesse caso deu barraco senhor cliente

    //ponderar executar da queue: ver cada valor de cada transf e ver se "cabe"
        //se "cabe", executa.
        //se não conseguir, vai para o fim que se lixa tentasse noutra altura c:

    //sem prioridades, queue para os processos
    //para fazer prioridades é preciso alguma coisa parecida com maxheap/array com valor máximo tipo orla do djikstra

    //prioridade extra: quantidade de transformações/quantidade de transformações iguais == n_transf que passamos para o executaPedido
    //tentar escolher a que tem menos porque deve demorar menos

    //finalmente executar alguma coisa yey como?
    //as transformações lêem do stdin e mandam para o stdout
    //como começar/acabar?
        //fazer dup do stdin direto para o ficheiro OU fazer dup para um pipe e escrever nesse pipe
        //quando fazer open do ficheiro inicial/final? antes ou depois do fork?
    
    //fazer o executa pedido está feito, mas precisa de conseguir fazer concorrente na mesma
    //o executaPedido vai devolver para não parar o servidor mas isso não quer dizer que acabou...
    //esperar pelo sinal de término para poder decrementar do dicionário dos maxs
    return 0;
}

int executaPedido(Pedido *pedido, char *pasta) {
    //fazer isto num manager para não mandar o server abaixo
    //fazer com que o manager seja uma função auxiliar
        //why? código. constantemente copiar o dicionário para cada manager SE COPIAR METE FORK NO MAIN
        //importa a função? vai copiar o processo inteiro?
    int manager;
    if ((manager = fork()) == -1) {
        //se não conseguires dar fork ao manager?
        perror("Failed Fork to Manager");
    } else if (manager == 0) {
        //o manager fala com o client? pode dizer-lhe diretamente que acabou sem passar pelo servidor
        if (pedido->n_transfs == 1) {
            switch (fork())
            {
            case 0:
                char buffer[strlen(pasta) + strlen(transfs[0]) + 1];
                //responsabilidade do manager abrir e fichar os fds -> não sobrecarregar os fd's do server
                int ret, fd_i, fd_o;
                fd_i = open(pedido->file_in, O_RDONLY);
                fd_o = open(pedido->file_out, O_CREAT | O_TRUNC | O_WRONLY, 0666); //pôr if's à volta dos opens
                dup2(fd_i, 0);
                dup2(fd_o, 1); //pôr if's à volta dos dups
                close(fd_i);
                close(fd_o);
                sprintf(buffer, "%s/%s", pasta, transfs[0]);
                ret = execl(buffer, buffer);
                perror("Failed Exec Manager Child");
                _exit(ret);
            case -1:
                perror("Failed Fork Manager to Child");
                _exit(-1);
            default:
                int status;
                wait(&status);
                if (!WIFEXITED(status) || WEXITSTATUS(status) == 255) {
                    perror("Failed Exec or Transf");
                    _exit(-1);
                }
                _exit(0);
            }
        } else {
            int p1[2];
            int p2[2];
            if (pipe(p1)==-1) {
                perror("Failed pipe 1");
                _exit(-1);
            }
            if (pipe(p2)==-1) {
                perror("Failed pipe 2");
                _exit(-1);
            }
            int i;
            //0 fork->dups especiais de in->exec
            for (i = 1; i< pedido->n_transfs - 1; i++) {
                //fork->dups alternantes (i%2)->exec
            }
            //n_transf-1 fork->dup especial COM CONTA DO ALTERNANTE (i%2) de out->exec
            for (i = 0; i < 2; i++) {
                close(p1[i]);
                close(p2[i]);
            }
            close(p1[0]);
            close(p1[1]);
            close(p2[0]);
            close(p2[1]);
            _exit(0);
        }

        //fazer forks while houver transformações
        //fazemos 1º fork se houver transformação
        //E agora? Fazemos primeiro 1) e depois vê-se como os stores querem a cena avançada
            //1) fazemos dup do ficheiro para o stdin
                //poupa imensas syscalls por não usar tralha
            //2) fazemos write para um pipe (saber o tamanho) e fazer dup do pipe para o stdin
                //para quê? não vale mais a pena 1) e depois sacar o tamanho?
                //mais constante
    } else {
        //não fazer nada de jeito ou um wait não bloqueante
        //sinais! quando o manager der SIGTRAP o servidor vai ver quem acabou
        //o servidor só quer saber para limpar do dicionário as transformações a serem usadas
    }
}
