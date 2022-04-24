#include "sdstored.h"

#define StringToBuffer(r, string, buffer) \
    for (w = 0; string[r] != '\0' && string[r] != ' '; w++, r++) {\
       buffer[w] = string[r];\
    }\
    buffer[w] = '\0';\
    r++;\

//estrutura de dados para implementar o dicionário dos limites
//como determinar o número? começar com hardcode a 13

/**
 * @brief Adicionar uma transformação ao dicionário de um Pedido
 * 
 * Assume que os máximos já estão definidos.
 * Verifica se a transformação existe no servidor, se não avisa que o pedido deve ser rejeitado
 * 
 * @param transf Transformação a adicionar
 * @param h Dicionário onde adicionar
 * @param maxs Dicionário dos máximos
 * @return int Valor de erro se o pedido for válido ou rejeitado
 */
int addTransfHT(char *transf, HT *h, HT *maxs) {
    int max, curr;
    if (readHT(maxs, transf, &max) == -1) {
        write(2, "Transformation not in config.", 31);
        return 1;
    }
    if (plusOneHT(h, transf, &curr) == -1) {
        writeHT(h, transf, 1);
        curr = 1;
    }
    if (curr > max) {
        //rejeitar o pedido
        return -1;
    }
    return 0;
}

void deepFree(Pedido *dest) {
    if (dest->hashtable)
        freeHT(dest->hashtable);
    if (dest->transfs) {
        int i;
        for(i = 0; i<dest->n_transfs+4; i++) {
            free(dest->transfs[i]);
        }
        free(dest->transfs);
    }
    free(dest->string);
    free(dest);
}

int createPedido(char *string, Pedido **dest, HT *maxs, int n_pedido) {
    *dest = malloc(sizeof(Pedido));
    (*dest)->id = n_pedido;
    char buffer[32];
    //supor que tem a prioridade, in e out
    int r = 0, w, n, i = 0;
    StringToBuffer(r, string, buffer)
    //primeira parte da string é o número de argumentos
    n = atoi(buffer);
    (*dest)->transfs = malloc(n*sizeof(char*));
    for(; string[r] != '\0' && i<4; i++) {
        StringToBuffer(r, string, buffer)
        //escrever o buffer para os transfs
        if(((*dest)->transfs[i] = strdup(buffer)) == NULL) {
            write(2, "Problem with memory.", 21);
            return -1;
        }
    }

    (*dest)->hashtable = malloc(sizeof(HT));
    initHT(h, 13);
    for(; string[r] != '\0' && i < n; r++, i++) {
        StringToBuffer(r, string, buffer)
        //escrever o buffer para os transfs
        if(((*dest)->transfs[i] = strdup(buffer)) == NULL) {
            write(2, "Problem with memory.", 21);
            return -1;
        }
        //temos de ver os espaços, ir adicionando ao HT
        if ((w = addTransfHT(buffer, (*dest)->hashtable, maxs)) == 1) {
            //pedido rejeitado
            return 1;
        } else if (w == -1) {
            //erro de execução
            return -1;
        }
    }
    (*dest)->n_transfs = n-4;
    return 0;
}

void escolheEntradaSaidaOneTransf(Pedido *pedido){
    int fd_i, fd_o;
    if((fd_i = open(pedido->transfs[2], O_RDONLY)) == -1){
        write(2,"Failed to open file in", 23);
        _exit(-1);
    }
    if((fd_o = open(pedido->transfs[3], O_CREAT | O_TRUNC | O_WRONLY, 0666)) == -1){
        write(2, "Failed to open file out", 24);
        _exit(-1);
    } //pôr if's à volta dos opens (já pus -- carlos)
    if((dup2(fd_i, 0)) == -1){
        write(2, "Failed to dup the input", 24);
        _exit(-1);
    }
    if((dup2(fd_o, 1)) == -1){
        write(2, "Failed to dup the output", 25);
        _exit(-1);
    } //pôr if's à volta dos dups (já pus -- carlos)
    close(fd_i);
    close(fd_o);
}

void escolheEntradaSaida(Pedido *pedido, int i, int **p){
    if(i == pedido->n_transfs-1){
        if((dup2(p[i-1][0], 0)) == -1){
            write(2, "Failed to dup the input", 24);
            _exit(-1);
        }
        close(p[i-1][0]);
        int fd_o;
        if((fd_o = open(pedido->transfs[3], O_CREAT | O_TRUNC | O_WRONLY, 0666)) == -1){
            write(2, "Failed to open file out", 24);
            _exit(-1);
        }
        if((dup2(fd_o, 1)) == -1){
            write(2, "Failed to dup the output", 25);
            _exit(-1);
        }
        close(fd_o);
    }else if(i == 0){
        close(p[i][0]);
        if((dup2(p[i][1], 1)) == -1){
            write(2, "Failed to dup the output", 25);
            _exit(-1);
        }
        close(p[i][1]);
    }else{
        close(p[i][0]);
        if((dup2(p[i-1][0], 0)) == -1){
            write(2, "Failed to dup the input", 24);
            _exit(-1);
        }
        close(p[i-1][0]);
        if((dup2(p[i][1], 1)) == -1){
            write(2, "Failed to dup the output", 25);
            _exit(-1);
        }
        close(p[i][1]);
    }
}

int executaPedido(Pedido *pedido, char *pasta) {
    //fazer isto num manager para não mandar o server abaixo
    //fazer com que o manager seja uma função auxiliar
        //why? código. constantemente copiar o dicionário para cada manager SE COPIAR METE FORK NO MAIN
        //importa a função? vai copiar o processo inteiro?
    int manager;
    if ((manager = fork()) == -1) {
        //se não conseguires dar fork ao manager?
        write(2,"Failed Fork to Manager", 23);
    } else if (manager == 0) {
        //o manager fala com o client? pode dizer-lhe diretamente que acabou sem passar pelo servidor
        if (pedido->n_transfs == 1) {
            char buffer[strlen(pasta) + strlen(pedido->transfs[4]) + 1];
            switch(fork()){
                case -1:
                    write(2, "Failed Fork Manager to Child", 29);
                    _exit(-1);
                case 0:
                    escolheEntradaSaidaOneTransf(pedido);
                    sprintf(buffer, "%s/%s", pasta, pedido->transfs[4]);
                    int ret = execl(buffer, buffer);
                    write(2, "Failed Exec Manager Child", 26);
                    _exit(ret);
                default:
                    int status;
                    wait(&status);
                    if (!WIFEXITED(status) || WEXITSTATUS(status) == 255) {
                        write(2, "Failed Exec or Transf", 22);
                        _exit(-1);
                    }
                }
        } else {
            //0 fork->dups especiais de in->exec
            //realizaTransf(pedido, pasta, 0, p1, p2);
            //não funciona c:
            int p[pedido->n_transfs-1][2], i;
            for (i = 0; i < pedido->n_transfs; i++) { //fork->dups alternantes (i%2)->exec
                if(i != pedido->n_transfs-1)
                    if(pipe(p[i]) == -1){
                        write(2, "Failed pipe", 12);
                        _exit(-1);
                    }
                char buffer[strlen(pasta) + strlen(pedido->transfs[i+4]) + 1];
                switch(fork()){
                    case -1:
                        write(2, "Failed Fork Manager to Child", 29);
                        _exit(-1);
                    case 0:
                        escolheEntradaSaida(pedido, i, **p);
                        sprintf(buffer, "%s/%s", pasta, pedido->transfs[i+4]);
                        int ret = execl(buffer, buffer);
                        write(2, "Failed Exec Manager Child", 26);
                        _exit(ret);
                    default:
                        if(i != pedido->n_transfs-1)
                            close(p[i][1]);
                        if(i != 0)
                            close(p[i-1][0]);
                        int status;
                        wait(&status);
                        if (!WIFEXITED(status) || WEXITSTATUS(status) == 255) {
                            write(2, "Failed Exec or Transf", 22);
                            _exit(-1);
                        }
                    }
                }
            //n_transf-1 fork->dup especial COM CONTA DO ALTERNANTE (i%2) de out->exec
            //realizaTransf(pedido, pasta, i, p1, p2);
            //o manager avisa o cliente ou avisa o servidor que avisa o cliente
                //prob avisa o servidor que avisa o cliente
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
        //o manager também pode logo mandar o aviso ao cliente de alguma maneira
    }
}

int addPendingQueue(Pedido *pedido, PendingQueue queue[]) {
    LList *new;
    if ((new = malloc(sizeof(LList))) == NULL) {
        write(2, "Failed to create LList", 23);
        return -1;
    }
    new->next = NULL;
    new->pedido = pedido;
    queue[(pedido->transfs[1] - 1)].end->next = new;
    queue[(pedido->transfs[1] - 1)].end = new;
    return 0;
}

int isPedidoExec(Pedido *pedido, HT *maxs, HT *curr) {
    char transf[MAX_TRANSF_SIZE];
    int i, s, new, c, max;
    for (i = 0, s = 0; i<pedido->hashtable->size && s<pedido->hashtable->used; i++) {
        
        if (!(FREE(pedido->hashtable, i))) {
            s++;
            readHT(maxs, pedido->hashtable->tbl[i].key, &max);
            readHT(pedido->hashtable, pedido->hashtable->tbl[i].key, &new);
            if (readHT(curr, pedido->hashtable->tbl[i].key, &c) == -1) {
                c = 0;
            }
            if (max - c < new)
                return 0;
        }
    }
    return 1;
}

Pedido *choosePendingQueue(PendingQueue queue[], HT *maxs, HT *curr) {
    int i;
    LList **nodo;
    Pedido *pedido;
    for (i = MAX_PRIORITY - 1; i>=0; i--) {
        for (nodo = &(queue[i].start); (*nodo) != NULL; nodo = &((*nodo)->next)) {
            pedido = (*nodo)->pedido;
            if (isPedidoExec(pedido, maxs, curr)) {
                (*nodo) = (*nodo)->next;
                return pedido;
            }
        }
    }
    return NULL;
}

int main(int argc, char const *argv[]) {
    //o servidor é executado com o config e com a pasta
    //todo teste para ver se não nos estão a tentar executar o server maliciosamente
    int fdConfig;
    if ((fdConfig = open(argv[1], O_RDONLY)) == -1) {
        //como validar um path (pasta)?
        write(2, "Failed to open config file", 25);
        return -1;
    }

    char pasta[] = argv[2];
    //lembrar da pasta em algum sítio para os executáveis
    //eventualmente fazer sprintf("%s/%s", nome da pasta, nome da transforamação)

    HT maxs;
    if (initHT(&maxs, INIT_DICT_SIZE) == -1) {
        write(2, "No space for Hashtable", 23);
    }
    if (readConfigNew(fdConfig, &maxs) == -1) {
        write(2, "Failed to read config", 22);
    }
    //fix manhoso
    HT curr;
    if (initHT(&curr, INIT_DICT_SIZE) == -1) {
        write(2, "No space for Hashtable", 23);
    }

    //todo preencher o dicionário com 1º argumento
    //parse desse ficheiro .config: readln c/ sequencial até ' '
    int i;
    PendingQueue pendingQ[MAX_PRIORITY];
    for (i = 0; i<MAX_PRIORITY; i++) {
        pendingQ[i].end = NULL;
        pendingQ[i].start = NULL;
    }

    //depois do servidor ser executado, fica à espera de ler do pipe com nome a instrução
    char pipeRead[MAX_BUFF];
    //loop de read do pipe com nome para buffer

    char pipeParse[32];
    int w, n_pedido = 0;
    i = 0;
    StringToBuffer(i, pipeRead, pipeParse)
    if (strcmp(pipeParse, "status") == 0) {
        //não esquecer de fazer o status
        //tem diferente input
        //apenas imprime o que está a ser processado (não o que está à espera)
        //muito provavelmente vamos ter de tirar da lista/queue/heap ready e pôr numa lista "in proccessing"
            //fica muito mais fácil de saber quais os que contam contra os maxs e depois tirar quando acabarem
        /*
        int i, n;
        char *string;
        for (i = 0; i < n_pedidos_processamento; i++) {
            n = strArrayToString(pedidos_processamento[i]->n_transfs + 4, pedidos_processamento[i]->transfs, &string, 0);
            write(1, string, n); ou se calhar tem de escrever o cliente?
        }
        o pedidos processamento pode não ser array, pode ter de ser com apontadores, depende da implementação
        */
    } else if (strcmp(pipeParse, "proc-file") == 0) {
        //Leitura do pedido
        Pedido *pedido;
        if ((w = createPedido(pipeRead, &pedido, &maxs, n_pedido++)) == -1) {
            //erro de execução
            return -1;
        } else if (w == 1) {
            //pedido rejeitado
            deepFree(pedido);
            //escrever de volta ao cliente que deu asneira
        }
        if (addPendingQueue(pedido, pendingQ)==-1) {
            
            return -1;
        }
        //avisar o cliente que foi posto em pending
        
        pedido = choosePendingQueue(pendingQ, maxs, curr); //já remove da pending queue
        if (pedido != NULL) {
            //adicionar aos em processamento
            
            //avisar o cliente que foi adicionado aos em processamento
            executaPedido(pedido, pasta);
        }
        //tentar executar logo?, se não colocar à espera
    } else {
        //erro de input do cliente, rejeitar o pedido
    }

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
    freeHT(maxs);
    return 0;
}
