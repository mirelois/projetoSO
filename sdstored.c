#include "sdstored.h"

#define StringToBuffer(r, w, string, buffer) \
    for (; string[r] != '\0' && string[r] != ' ' && string[r] != '\n'; w++, r++) {\
       buffer[w] = string[r];\
    }\
    buffer[w] = '\0';\
    if (string[r] != '\0')\
        r++;\

#define TestMaxPipe(r, bytes_read_pipe, fd_leitura, pipeRead)\
    if (r == bytes_read_pipe) {\
        r = 0;\
        bytes_read_pipe = read(fd_leitura, pipeRead, MAX_BUFF);\
    }\

int flag_term = 1;

void term_handler(int signum) {
    flag_term = 0;
}

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
    int *max, *curr, c = 1;
    
    if (readHT(maxs, transf, (void**) &max) == -1) {
        write(2, "Transformation not in config.\n", 31);
        return 1;
    }
    if (readHT(h, transf, (void**) &curr) == -1) {
        c = 1;
    } else {
        c = (*curr);
    }
    if (c > *max) {
        //rejeitar o pedido
        return 1;
    } else {
        char *s = strdup(transf);
        writeHT(h, s, &c);
    }
    return 0;
}

/**
 * @brief Criar um objeto do tipo Pedido
 * 
 * Recebe uma string do servidor para ser colocado num Pedido.
 * Salta o indicador "proc-file".
 * Coloca no prio a string com a prioridade; no in a string com o nome do ficheiro de input; no out a string com o nome do ficheiro de output.
 * Cria o dicionário a ser usada dentro do Pedido para guardar quantas transformações de cada tipo tem.
 * Para cada pedido na string, acrescenta 1 à sua entrada do dicionário, recusando o pedido se exceder o máximo.
 * 
 * @param string Input vindo do servidor, para ser dividido para as parcelas do Pedido.
 * @param dest Local da memória onde alocar o Pedido.
 * @param maxs Dicionário dos valores máximos das Transformações.
 * @param n_pedido Número do pedido.
 * @return int Mensagens de erro.
 */
int createPedido(char *string, Pedido **dest, HT *maxs, int n_pedido, int fd) {
    *dest = malloc(sizeof(Pedido));
    (*dest)->id = n_pedido;
    char buffer[33];
    //supor que tem a prioridade, in e out
    int r = 0, w, i; //saltar o proc-file à frente
    //segunda parte da string é o número de argumentos
    //eu sei que recebi pelo menos 5 coisas, com o proc-file
    (*dest)->fd = fd;
    w = 0;
    StringToBuffer(r, w, string, buffer)
    (*dest)->prio = strdup(buffer);
    w = 0;
    StringToBuffer(r, w, string, buffer)
    (*dest)->in = strdup(buffer);
    w = 0;
    StringToBuffer(r, w, string, buffer)
    (*dest)->out = strdup(buffer);
    (*dest)->pedido = strdup(string+r);
    (*dest)->hashtable = malloc(sizeof(HT));
    initHT((*dest)->hashtable, INIT_DICT_SIZE, 0, STRING, INT);
    for(i = 0; string[r] != '\0'; i++) {
        w = 0;
        StringToBuffer(r, w, string, buffer)
        //temos de ver os espaços, ir adicionando ao HT
        if ((w = addTransfHT(buffer, (*dest)->hashtable, maxs)) == 1) {
            //pedido rejeitado
            return 1;
        } else if (w == -1) {
            //erro de execução
            return -1;
        }
    }
    (*dest)->n_transfs = i;
    return 0;
}

void escolheEntradaSaidaOneTransf(char *in, char *out){
    int fd_i, fd_o;
    if((fd_i = open(in, O_RDONLY)) == -1){
        write(2,"Failed to open file in\n", 24);
        _exit(-1);
    }
    if((fd_o = open(out, O_CREAT | O_TRUNC | O_WRONLY, 0666)) == -1){
        write(2, "Failed to open file out\n", 25);
        _exit(-1);
    } //pôr if's à volta dos opens (já pus -- carlos)
    if((dup2(fd_i, 0)) == -1){
        write(2, "Failed to dup the input\n", 25);
        _exit(-1);
    }
    if((dup2(fd_o, 1)) == -1){
        write(2, "Failed to dup the output\n", 26);
        _exit(-1);
    } //pôr if's à volta dos dups (já pus -- carlos)
    close(fd_i);
    close(fd_o);
}

void escolheEntradaSaida(Pedido *pedido, int i, int *p[2]){
    if(i == pedido->n_transfs-1){
        if((dup2(p[i-1][0], 0)) == -1){
            write(2, "Failed to dup the input\n", 25);
            _exit(-1);
        }
        close(p[i-1][0]);
        int fd_o;
        if((fd_o = open(pedido->out, O_CREAT | O_TRUNC | O_WRONLY, 0666)) == -1){
            write(2, "Failed to open file out\n", 25);
            _exit(-1);
        }
        if((dup2(fd_o, 1)) == -1){
            write(2, "Failed to dup the output\n", 26);
            _exit(-1);
        }
        close(fd_o);
    }else if(i == 0){
        int fd_i;
        if((fd_i = open(pedido->in, O_RDONLY)) == -1){
            write(2, "Failed to open file in\n", 24);
            _exit(-1);
        }
        if((dup2(fd_i, 1)) == -1){
            write(2, "Failed to dup the input\n", 25);
            _exit(-1);
        }
        close(fd_i);
        close(p[i][0]);
        if((dup2(p[i][1], 1)) == -1){
            write(2, "Failed to dup the output\n", 26);
            _exit(-1);
        }
        close(p[i][1]);
    }else{
        close(p[i][0]);
        if((dup2(p[i-1][0], 0)) == -1){
            write(2, "Failed to dup the input\n", 25);
            _exit(-1);
        }
        close(p[i-1][0]);
        if((dup2(p[i][1], 1)) == -1){
            write(2, "Failed to dup the output\n", 26);
            _exit(-1);
        }
        close(p[i][1]);
    }
}

pid_t executaPedido(Pedido *pedido, char *pasta, int fd_escrita) {
    //fazer isto num manager para não mandar o server abaixo
    //fazer com que o manager seja uma função auxiliar
        //why? código. constantemente copiar o dicionário para cada manager SE COPIAR METE FORK NO MAIN
        //importa a função? vai copiar o processo inteiro?
    pid_t manager;
    if ((manager = fork()) == -1) {
        //se não conseguires dar fork ao manager?
        write(2,"Failed Fork to Manager\n", 24);
    } else if (manager == 0) {
        write(pedido->fd, "processing\n", 12);
        int r=0, w, tamanhoinicial = strlen(pasta);
        char buffer[tamanhoinicial + 33];
        for(w=0; pasta[w]!='\0'; w++)
            buffer[w] = pasta[w];
        buffer[w++] = '/';
        //o manager fala com o client? pode dizer-lhe diretamente que acabou sem passar pelo servidor
        if (pedido->n_transfs == 1) {
            //char buffer[strlen(pasta) + w + 1];
            switch(fork()){
                case -1:
                    write(2, "Failed Fork Manager to Child\n", 30);
                    _exit(-1);
                case 0:
                    escolheEntradaSaidaOneTransf(pedido->in, pedido->out);
                    //sprintf(buffer, "%s/%s", pasta, pedido->transfs[4]);
                    StringToBuffer(r, w, pedido->pedido, buffer);
                    int ret = execl(buffer, buffer, (char *) NULL);
                    write(2, "Failed Exec Manager Child\n", 27);
                    _exit(ret);
                default: ;
                    int status;
                    wait(&status);
                    if (!WIFEXITED(status) || WEXITSTATUS(status) == 255) {
                        write(2, "Failed Exec or Transf\n", 23);
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
                        write(2, "Failed pipe\n", 13);
                        _exit(-1);
                    }
                // char buffer[strlen(pasta) + strlen(pedido->transfs[i+4]) + 1];
                switch(fork()){
                    case -1:
                        write(2, "Failed Fork Manager to Child\n", 30);
                        _exit(-1);
                    case 0:
                        w = tamanhoinicial;
                        StringToBuffer(r, w, pedido->pedido, buffer);
                        escolheEntradaSaida(pedido, i, p);
                        int ret = execl(buffer, buffer);
                        write(2, "Failed Exec Manager Child\n", 27);
                        _exit(ret);
                    default:
                        if(i != pedido->n_transfs-1)
                            close(p[i][1]);
                        if(i != 0)
                            close(p[i-1][0]);
                        int status;
                        wait(&status);
                        if (!WIFEXITED(status) || WEXITSTATUS(status) == 255) {
                            write(2, "Failed Exec or Transf\n", 23);
                            _exit(-1);
                        }
                    }
                }
            //n_transf-1 fork->dup especial COM CONTA DO ALTERNANTE (i%2) de out->exec
            //realizaTransf(pedido, pasta, i, p1, p2);
            //o manager avisa o cliente ou avisa o servidor que avisa o cliente
                //prob avisa o servidor que avisa o cliente
        }
        //fazer forks while houver transformações
        //fazemos 1º fork se houver transformação
        //E agora? Fazemos primeiro 1) e depois vê-se como os stores querem a cena avançada
            //1) fazemos dup do ficheiro para o stdin
                //poupa imensas syscalls por não usar tralha
            //2) fazemos write para um pipe (saber o tamanho) e fazer dup do pipe para o stdin
                //para quê? não vale mais a pena 1) e depois sacar o tamanho?
                //mais constante
        int tamanho = getpid();
        //int j;
        //for(j=0; tamanho!=0; j++){ // contar quantas casas
        //    tamanho = tamanho/10;
        //}
        //cuidado com o tamanho do buffer
        sprintf(buffer, "%d", tamanho);
        write(fd_escrita, buffer, strlen(buffer)+1);
        write(pedido->fd, "concluded\n", 11); // Falta o avançado
        _exit(0);
    }
    return manager;
}

/**
 * @brief Adiciona um pedido à PendingQueue
 * 
 * @param pedido Pedido a adicionar
 * @param queue Queue onde adicionar
 * @return int Mensagem de erro
 */
int addPendingQueue(Pedido *pedido, PendingQueue *queue) {
    LList *new;
    if ((new = malloc(sizeof(LList))) == NULL) {
        write(2, "Failed to create LList", 23);
        return -1;
    }
    int p = atoi(pedido->prio);
    new->next = NULL;
    new->pedido = pedido;
    if (queue[p].end != NULL) {
        queue[p].end->next = new;
    }
    queue[p].end = new;
    if (queue[p].start == NULL) {
        queue[p].start = new;
    }
    return 0;
}

/**
 * @brief Função para determinar se um pedido é executável
 * 
 * Verifica o Dicionário dos máximos e o dos atuais para saber se se pode executar o dado Pedido.
 * 
 * @param pedido Pedido a ver se se pode executar.
 * @param maxs Dicionário dos máximos.
 * @param curr Dicionário dos atuais.
 * @return int Inteiro tratado como booleano.
 */
int isPedidoExec(Pedido *pedido, HT *maxs, HT *curr) {
    char transf[MAX_TRANSF_SIZE];
    int i, s, *new, *c, *max;
    for (i = 0, s = maxs->aux_array.last; s != -1 && i<pedido->hashtable->used; i++, s = maxs->aux_array.array[POS(s, 0)]) {
        strcpy(transf, (char *) maxs->tbl[i].key);
        if (readHT(pedido->hashtable, (void *) transf, (void **) &new) != -1) {
            readHT(maxs, transf, &max);
            if (readHT(curr, transf, &c) == -1) {
                char *wr = strdup(transf);
                c = malloc(sizeof(int));
                *c = 0;
                writeHT(curr, wr, c);
            }
            if (max - c < new) {
                return 0;
            }
        }
    }
    return 1;
}

Pedido *choosePendingQueue(PendingQueue queue[], HT *maxs, HT *curr) {
    int i = MAX_PRIORITY;
    LList *nodo;
    Pedido *pedido;
    while (i >= 0 && queue[i].start == NULL) i--;
    //if (i >= 0) {
    //    for (nodo = &(queue[i].start); (*nodo) != NULL; nodo = &((*nodo)->next)) {
    //        pedido = (*nodo)->pedido;
    //        int l;
    //        if (l = isPedidoExec(pedido, maxs, curr)) {
    //            (*nodo) = (*nodo)->next;
    //            return pedido;
    //        }
    //    }
    //}
    //tenta executar o primeiro, se deu deu
    if (i >= 0) {
        nodo = queue[i].start;
        pedido = nodo->pedido;
        if (isPedidoExec(pedido, maxs, curr)) {
            queue[i].start = nodo->next;
            return pedido;
        }
    }
    return NULL;
}

int createInputChild(int pipe_input[2], int *pid_input_child, int fd_leitura) {
    *pid_input_child = fork();
    switch (*pid_input_child)
    {
    case 0: ;
        int bytes_read,r;
        char pipeRead[4]; // Colocar 512 em vez de Max_Buff ? 
        //read(0, pipeRead, MAX_BUFF);
        //lembrar de tirar
        //for (r = 0; r<MAX_BUFF; r++) {
        //    if (pipeRead[r] == EOF) {
        //        pipeRead[r] = '\0';
        //    }
        //}
        bytes_read = read(fd_leitura, pipeRead, 4); // falta tratar erros
        //loop de read do pipe com nome para buffer
        close(pipe_input[0]);
        write(pipe_input[1], pipeRead, bytes_read);
        close(pipe_input[1]);
        _exit(0);

        default: 
            return 0;
    }
}

int pedidoToString(Pedido *pedido, char **dest) {
    int n = strlen(pedido->out) + strlen(pedido->pedido) + strlen(pedido->in) + strlen(pedido->prio) + 14;
    (*dest) = malloc(n);
    sprintf((*dest), "%s %s %s %s %s", "proc-file", pedido->prio, pedido->in, pedido->out, pedido->pedido);
    return n;
}

int main(int argc, char const *argv[]) {
    //o servidor é executado com o config e com a pasta
    //todo teste para ver se não nos estão a tentar executar o server maliciosamente
    signal(SIGTERM, term_handler);

    //if((mkfifo("entrada", 0666)) == -1){
    //    write(2, "Failed to create Named pipe entrada\n", 37);
    //    return -1;
    //}

    int fd_leitura, fd_escrita;

    if ((fd_leitura = open("entrada", O_RDONLY)) == -1) {
        write(2, "Failed to open the named pipe\n", 31);
        return -1;
    }
    if ((fd_escrita = open("entrada", O_WRONLY)) == -1) {
        write(2, "Failed to open the named pipe\n", 31);
        return -1;
    }

    int fdConfig;
    if ((fdConfig = open(argv[1], O_RDONLY)) == -1) {
        //como validar um path (pasta)?
        write(2, "Failed to open config file\n", 28);
        return -1;
    }
    
    HT *maxs = malloc(sizeof(HT));
    if (initHT(maxs, INIT_DICT_SIZE, 1, STRING, INT) == -1) {
        write(2, "No space for Hashtable", 23);
        freeHT(maxs);
        return -1;
    }
    //incializar a hashtables dos máximos
    if (readConfig(fdConfig, maxs) == -1) {
        write(2, "Failed to read config\n", 23);
        return -1;
    }

    //fix manhoso
    HT *curr = malloc(sizeof(HT));
    if (initHT(curr, INIT_DICT_SIZE, 0, STRING, INT) == -1) {
        write(2, "No space for Hashtable\n", 24);
        freeHT(maxs);
        freeHT(curr);
        return -1;
    }

    HT *proc = malloc(sizeof(HT));
    if (initHT(proc, INIT_DICT_SIZE, 1, INT, PEDIDO) == -1) {
        write(2, "No space for Hashtable\n", 24);
        freeHT(maxs);
        freeHT(curr);
        freeHT(proc);
        return -1;
    }

    int r, w;
    PendingQueue pendingQ[MAX_PRIORITY+1];
    for (r = 0; r<MAX_PRIORITY+1; r++) {
        pendingQ[r].end = NULL;
        pendingQ[r].start = NULL;
    }

    int status, n_pedido = 1, bytes_read_pipe = 0, pid_read;
    char pipeRead[MAX_BUFF], pipeParse[MAX_BUFF];
    Pedido *pedido_read;
    r = 0;
    while(flag_term && 1) {
        TestMaxPipe(r, bytes_read_pipe, fd_leitura, pipeRead)
        w = 0;
        while (pipeRead[r] != ' ' && pipeRead[r] != '\0') {
            pipeParse[w++] = pipeRead[r++];
            TestMaxPipe(r, bytes_read_pipe, fd_leitura, pipeRead)
        }
        r++;
        pipeParse[w] = '\0';
        pid_read = atoi(pipeParse);
        int proc_pid_pos = readHT(proc, (void *) &pid_read, (void **) &pedido_read);
        if (proc_pid_pos == -1 && flag_term) {
            //deve de ser input
            int fd_pedido;
            if ((fd_pedido = open(pipeParse, O_WRONLY)) == -1) {
                write(2, "Failed to open pipe to client\n", 31);
                //rejeita pedido
            } else {
                w = 0;
                TestMaxPipe(r, bytes_read_pipe, fd_leitura, pipeRead)
                while (pipeRead[r] != ' ' && pipeRead[r] != '\0') {
                    pipeParse[w++] = pipeRead[r++];
                    TestMaxPipe(r, bytes_read_pipe, fd_leitura, pipeRead)
                }
                pipeParse[w] = '\0';
                r++;
                if (strcmp(pipeParse, "proc-file") == 0) {
                    w = 0;
                    TestMaxPipe(r, bytes_read_pipe, fd_leitura, pipeRead)
                    while (pipeRead[r] != '\0') {
                        pipeParse[w++] = pipeRead[r++];
                        TestMaxPipe(r, bytes_read_pipe, fd_leitura, pipeRead)
                    }
                    r++;
                    pipeParse[w] = '\0';
                    //Leitura do pedido
                    Pedido *pedido;
                    if ((w = createPedido(pipeParse, &pedido, maxs, n_pedido++, fd_pedido)) == -1) {
                        //erro de execução
                        freeHT(maxs);
                        freeHT(curr);
                        freeHT(proc);
                        deepFreePedido(pedido);
                        return -1;
                    } else if (w == 1) {
                        //pedido rejeitado
                        deepFreePedido(pedido);
                        //avisar o cliente que deu asneira se tiver fd aberto lmao
                    } else if (addPendingQueue(pedido, pendingQ)==-1) { // Fazer write(pedido->fd) com pending
                        freeHT(maxs);
                        freeHT(curr);
                        freeHT(proc);
                        deepFreePedido(pedido);
                        return -1;
                    }
                    //executaPedido(pedido, pasta);
                    //avisar o cliente que foi posto em pending
                    write(pedido->fd, "pending\n", 9);
                    pedido = choosePendingQueue(pendingQ, maxs, curr); //já remove da pending queue
                    if (pedido != NULL) {
                        //adicionar aos em processamento
                        //avisar o cliente que foi adicionado aos em processamento
                        int *pid_manager = malloc(sizeof(int));
                        *pid_manager = executaPedido(pedido, argv[2], fd_escrita);
                        if (writeHT(proc, (void *) pid_manager, pedido) == -1) {
                            write(2, "Failed to write to proc\n", 5);
                        }
                    }
                } else if (strcmp(pipeParse, "status") == 0) {
                    char *string;
                    int i, bytes_read;
                    for (i = proc->aux_array.last; i != -1; w = proc->aux_array.array[POS(i, 0)]) {
                        bytes_read = pedidoToString((Pedido *) proc->tbl[i].value, &string);
                        write(((Pedido *)proc->tbl[i].value)->fd, string, bytes_read);
                        free(string);
                    }
                } else {
                    //borradovski
                }
            }
        } else if (proc_pid_pos == -1) {
            
            int fd_pedido;
            if ((fd_pedido = open(pipeParse, O_WRONLY)) == -1) {
                write(2, "Failed to open pipe to client\n", 31);
                //rejeita pedido
            }
            write(fd_pedido, "Mano seu trouxa\n", 17);
            close(fd_pedido);
            TestMaxPipe(r, bytes_read_pipe, fd_leitura, pipeRead)
            while (pipeRead[r] != '\0') {
                r++;
                TestMaxPipe(r, bytes_read_pipe, fd_leitura, pipeRead)
            }
            r++;
        } else if (proc_pid_pos >= 0) {
            //deve de ser termino do manager
            //int key = atoi(pipeParse);
            //retirar aos proc
            waitpid(pid_read, &status, 0);
            //testar os erros do status
            deleteHT(proc, &pid_read);

            Pedido *pedido;
            pedido = choosePendingQueue(pendingQ, maxs, curr); //já remove da pending queue
            if (pedido != NULL) {
                //adicionar aos em processamento
                //avisar o cliente que foi adicionado aos em processamento
                int *pid_manager = malloc(sizeof(int));
                *pid_manager = executaPedido(pedido, argv[2], fd_escrita);
                writeHT(proc, (void *) pid_manager, pedido);
            }
        }
    }
    close(fd_escrita);
    close(fd_leitura);
    freeHT(maxs);
    freeHT(curr);
    freeHT(proc);
    unlink("entrada");
    return 0;
}
