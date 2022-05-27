#include "sdstored.h"

#define StringToBuffer(r, w, string, buffer) \
    for (; string[r] != '\0' && string[r] != ' ' && string[r] != '\n'; w++, r++) {\
       buffer[w] = string[r];\
    }\
    buffer[w] = '\0';\
    if (string[r] != '\0')\
        r++;\

#define TestMaxPipe(r, bytes_read_pipe, fd_leitura, pipeRead)\
    if (r >= bytes_read_pipe) {\
        r = 0;\
        bytes_read_pipe = read(fd_leitura, pipeRead, MAX_BUFF);\
    }\


int fd_leitura, fd_escrita;
int flag_term = 1;

/**
 * @brief Rotina de término gracioso.
 * 
 * Altera a flag de termino e fecha o descritor de escrita do fifo principal que mantém o servidor bloqueado em leitura.
 * 
 * @param signum 
 */
void term_handler(int signum) {
    write(1, "Received SIGTERM (SIGINT)\n", 27);
    flag_term = 0;
    close(fd_escrita);
}

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
        c++;
    }
    if (c > *max) {
        //rejeitar o pedido
        return 1;
    } else {
        writeHT(h, transf, &c);
    }
    return 0;
}

/**
 * @brief Criar um objeto do tipo Pedido
 * 
 * Recebe uma string do servidor para ser colocado num Pedido.
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
    (*dest)->fd = fd;
    //supor que tem a prioridade, in e out
    char buffer[33];
    int r = 0, w, i;
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

/**
 * @brief Função que executa um pedido, criando a pipeline através de um filho ("manager")
 * 
 * @param pedido Pedido a ser executado
 * @param pasta Pasta onde se encontram os executáveis
 * @param fd_escrita Pid do pipe com nome do servidor onde escrever
 * @return pid_t Pid do manager que é devolvido para o servidor
 */
pid_t executaPedido(Pedido *pedido, char *pasta) {
    //fazer isto num manager para não mandar o server abaixo
    //fazer com que o manager seja uma função auxiliar
        //why? código. constantemente copiar o dicionário para cada manager SE COPIAR METE FORK NO MAIN
        //importa a função? vai copiar o processo inteiro?
    int ret = 0, bytes_input, bytes_output, fd_i, fd_o;
    if((fd_i = open(pedido->in, O_RDONLY)) == -1){
        write(2,"Failed to open file in\n", 24);
        return -1;
    }
    bytes_input = lseek(fd_i, 0, SEEK_END);
    lseek(fd_i, 0, SEEK_SET);
    if((fd_o = open(pedido->out, O_CREAT | O_TRUNC | O_WRONLY, 0666)) == -1){
        write(2, "Failed to open file out\n", 25);
        return -1;
    }
    pid_t manager;
    if ((manager = fork()) == -1) {
        //se não conseguires dar fork ao manager?
        write(2,"Failed Fork to Manager\n", 24);
    } else if (manager == 0) {
        int r=0, w, tamanhoinicial = strlen(pasta);
        char buffer[tamanhoinicial + 33];
        sprintf(buffer, "processing %s\n", pedido->pedido);
        write(pedido->fd, buffer, strlen(buffer));
        for(w=0; pasta[w]!='\0'; w++)
            buffer[w] = pasta[w];
        //buffer[w++] = '/';
        //o manager fala com o client? pode dizer-lhe diretamente que acabou sem passar pelo servidor
        if (pedido->n_transfs == 1) {
            //char buffer[strlen(pasta) + w + 1];
            switch(fork()){
                case -1:
                    write(2, "Failed Fork Manager to Child\n", 30);
                    _exit(-1);
                case 0:
                    //escolheEntradaSaidaOneTransf(pedido->in, pedido->out);
                    //sprintf(buffer, "%s/%s", pasta, pedido->transfs[4]);
                    if((dup2(fd_i, 0)) == -1){
                        write(2, "Failed to dup the input\n", 25);
                        _exit(-1);
                    }
                    close(fd_i);
                    if((dup2(fd_o, 1)) == -1){
                        write(2, "Failed to dup the output\n", 26);
                        _exit(-1);
                    }
                    close(fd_o);
                    StringToBuffer(r, w, pedido->pedido, buffer);
                    int ex = execlp(buffer, buffer, (char *) NULL);
                    write(2, "Failed Exec Manager Child\n", 27);
                    _exit(ex);
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
            //int fd_i, fd_o;
            //if((fd_i = open(pedido->in, O_RDONLY)) == -1){
            //    write(2, "Failed to open file in\n", 24);
            //    _exit(-1);
            //}
            if((dup2(fd_i, 0)) == -1){
                write(2, "Failed to dup the input\n", 25);
                _exit(-1);
            }
            close(fd_i);
            int p[pedido->n_transfs-1][2], i;
            for (i = 0; i < pedido->n_transfs; i++) { //fork->dups alternantes (i%2)->exec
                if(i != pedido->n_transfs-1)
                    if(pipe(p[i]) == -1){
                        write(2, "Failed pipe\n", 13);
                        _exit(-1);
                    }else{
                        //if((fd_o = open(pedido->out, O_CREAT | O_TRUNC | O_WRONLY, 0666)) == -1){
                        //    write(2, "Failed to open file out\n", 25);
                        //    _exit(-1);
                        //}
                        if((dup2(fd_o, 1)) == -1){
                            write(2, "Failed to dup the output\n", 26);
                            _exit(-1);
                        }
                    }
                w = tamanhoinicial;
                StringToBuffer(r, w, pedido->pedido, buffer);
                // char buffer[strlen(pasta) + strlen(pedido->transfs[i+4]) + 1];
                switch(fork()){
                    case -1:
                        write(2, "Failed Fork Manager to Child\n", 30);
                        _exit(-1);
                    case 0:
                        //w = tamanhoinicial;
                        //StringToBuffer(r, w, pedido->pedido, buffer);
                        if(i==0){
                            close(p[i][0]);
                            if((dup2(p[i][1], 1)) == -1){
                                write(2, "Failed to dup the output\n", 26);
                                _exit(-1);
                            }
                            close(p[i][1]);
                        }else if(i == pedido->n_transfs-1){
                            if((dup2(p[i-1][0], 0)) == -1){
                                write(2, "Failed to dup the input\n", 25);
                                _exit(-1);
                            }
                            close(p[i-1][0]);
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
                        int ex = execl(buffer, buffer, (char *) NULL);
                        write(2, "Failed Exec Manager Child\n", 27);
                        _exit(ex);
                    default:
                        if(i!=0)
                            close(p[i-1][0]);
                        if(i!=pedido->n_transfs-1)
                            close(p[i][1]);
                }
            }
            

            int status;
            for(i = 0; i<pedido->n_transfs; i++) {
                wait(&status);
                //printf("help: %d\n", WIFEXITED(status));fflush(stdout);
                if (!WIFEXITED(status) || WEXITSTATUS(status) == 255) {
                    //perror("Failed Exec or Transf");
                    write(pedido->fd, "Failed Exec or Transformation.\n", 32);
                    ret = -1;
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
        bytes_output = lseek(fd_o, 0, SEEK_END);
        close(fd_i);
        close(fd_o);
        int tamanho = getpid();
        //int j;
        //for(j=0; tamanho!=0; j++){ // contar quantas casas
        //    tamanho = tamanho/10;
        //}
        //cuidado com o tamanho do buffer
        sprintf(buffer, "%d", tamanho);
        write(fd_escrita, buffer, strlen(buffer)+1);
        char concluded[100]; // mudar tamanho
        sprintf(concluded, "concluded (bytes-input: %d, bytes-output: %d)\n", bytes_input, bytes_output);
        write(pedido->fd, concluded, strlen(concluded));
        _exit(ret);
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
int addPendingQueue(Pedido *pedido, PendingQueue *queue, int *n_trasnfs_pending) {
    LList *new = malloc(sizeof(LList));
    if (new == NULL) {
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
    (*n_trasnfs_pending)++;
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
    int i, s, *new, *c, *max, c_int;
    for (i = 0, s = maxs->aux_array.last; s != -1 && i<maxs->entries; i++, s = maxs->aux_array.array[POS(s, 0)]) {
        if (readHT(pedido->hashtable, (maxs->tbl[s].key), (void **) &new) != -1 && readHT(maxs, (maxs->tbl[s].key), (void**) &max) != -1) {
            if (*new > 0) {
                if (readHT(curr, (maxs->tbl[s].key), (void **) &c) == -1) {
                //char *wr = strdup((char*)(maxs->tbl[s].key));
                c_int = 0;
                writeHT(curr, (char*)(maxs->tbl[s].key), (void *) &c_int);
                } else {
                    c_int = *c;
                }
                if (*max - c_int < *new) {
                    return 0;
                }
            }
            //printf("%d\n", *new);
        }
    }
    return 1;
}

/**
 * @brief Função que escolhe um pedido da Pending Queue
 * 
 * Escolhe o pedido com maior prioridade e tenta executar o com menor id, isto é, o que chegou primeiro à queue
 * 
 * @param queue PendingQueue onde se encontram os pedidos
 * @param maxs Hashtable dos máximos
 * @param curr Hashtable dos pedidos em processamento
 * @return Pedido* 
 */
Pedido *choosePendingQueue(PendingQueue queue[], HT *maxs, HT *curr, int *n_transfs_pendingQ) {
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
            free(nodo);
            if (queue[i].start == NULL) {
                queue[i].end = NULL;
            }
            (*n_transfs_pendingQ)--;
            return pedido;
        }
    }
    return NULL;
}

int removeCurr(Pedido *pedido, HT *curr, HT *maxs) {
    int i, *read, *sub;
    for (i = maxs->aux_array.last; i != -1; i = maxs->aux_array.array[POS(i,0)]) {
        if (readHT(curr, maxs->tbl[i].key, &read) != -1 && readHT(pedido->hashtable, maxs->tbl[i].key, &sub) != -1) {
            *read -= *sub;
            //escrita muito manhosa porque este é literalmente o apontador da hashtable
        }
    }
    return 0;
}

/**
 * @brief Função auxiliar que através de um pedido constrói a string que deve ser impressa no comando status
 * 
 * @param pedido Pedido de onde retirar as informações
 * @param dest String destino onde escrever
 * @return int Número de bytes escritos
 */
char *pedidoToString(Pedido *pedido, int *n) {
    (*n) = -1;
    int tmp = pedido->id;
    //pedido->id sempre >0
    while (tmp != 0) {
        tmp /= 10;
        (*n)++;
    }
    (*n) += strlen(pedido->out) + strlen(pedido->pedido) + strlen(pedido->in) + strlen(pedido->prio) + 22; //task + proc-file + 6 espaços + \n + \0 + ':'
    char *string = malloc((*n) + 1);
    snprintf(string, (*n)+1,"Task %d: proc-file %s %s %s %s\n", pedido->id, pedido->prio, pedido->in, pedido->out, pedido->pedido); //como só é usado para imprimir mais vale por o \n
    return string;
}

int addCurr(Pedido *pedido, HT *curr, HT *maxs) {
    int i, *add, *c, count = 0;
    for (i = maxs->aux_array.last; i!=-1; i = maxs->aux_array.array[POS(i,0)]) {
        //transf = strdup((char *) (maxs->tbl[i].key));
        //preciso deste strdup? se a chave já existe a hashtable faz clone ou apenas deita ao lixo este apontador?
        if (readHT(pedido->hashtable, maxs->tbl[i].key, &add) != -1) {
            if (readHT(curr, maxs->tbl[i].key, &c) == -1) {
                count = 0;
            } else {
                count = *c;
            }
            //dos inteiros a HT faz cópia, não esquecer
            count += *add;
            writeHT(curr, (char *) (maxs->tbl[i].key), &count);
        }
    }
    return 0;
}

/**
 * @brief Função que executa o ciclo principal do servidor
 * 
 * @param pasta String com a pasta onde se encontram os executáveis
 * @param maxs Hashtable dos valores máximos para as transformações
 * @param curr Hashtable dos valores atuais para as transformações
 * @param proc Hashtable dos pedidos em processamento
 * @return int Valor de erro
 */
int run(char const *pasta, HT *maxs, HT *curr, HT *proc) {
    //o servidor é executado com o config e com a pasta
    //todo teste para ver se não nos estão a tentar executar o server maliciosamente

    int r, w, n_transfs_pendingQ = 0;
    PendingQueue pendingQ[MAX_PRIORITY+1];
    for (r = 0; r<MAX_PRIORITY+1; r++) {
        pendingQ[r].end = NULL;
        pendingQ[r].start = NULL;
    }

    int status, n_pedido = 1, bytes_read_pipe = 0, pid_read;
    char pipeRead[MAX_BUFF], pipeParse[MAX_BUFF];
    Pedido *pedido_read;
    r = 0;
    //a flag_term indica que o servidor ainda não recebeu SIGTERM ou SIGINT
    //n_transfs_pendingQ é o número de transformações 
    while(flag_term || n_transfs_pendingQ > 0 || proc->entries) {
        TestMaxPipe(r, bytes_read_pipe, fd_leitura, pipeRead)
        w = 0;
        while (bytes_read_pipe > 0 && pipeRead[r] != ' ' && pipeRead[r] != '\0') {
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
                        deepFreePedido(pedido);
                        return -1;
                    } else if (w == 1) {
                        //pedido rejeitado
                        write(pedido->fd, "Failed Request\n", 16);
                        deepFreePedido(pedido);
                        //avisar o cliente que deu asneira se tiver fd aberto lmao
                    } else if (addPendingQueue(pedido, pendingQ, &n_transfs_pendingQ)==-1) { // Fazer write(pedido->fd) com pending
                        deepFreePedido(pedido);
                        return -1;
                    } else {
                        //executaPedido(pedido, pasta);
                        //avisar o cliente que foi posto em pending
                        sprintf(pipeParse, "pending Task: #%d\n", pedido->id);
                        write(pedido->fd, pipeParse, strlen(pipeParse));
                    }
                    for (   pedido = choosePendingQueue(pendingQ, maxs, curr, &n_transfs_pendingQ); pedido != NULL; 
                            pedido = choosePendingQueue(pendingQ, maxs, curr, &n_transfs_pendingQ)) {
                        
                        //adicionar aos em processamento
                        if (addCurr(pedido, curr, maxs) == -1) {
                            write(2, "Failed to write to curr\n", 25);
                        }
                        //avisar o cliente que foi adicionado aos em processamento
                        int pid_manager;
                        if ((pid_manager = executaPedido(pedido, pasta)) == -1) {
                            write(pedido->fd, "Failed request\n", 16);
                            deepFreePedido(pedido);
                        } else {
                            writeHT(proc, (void *) &pid_manager, pedido);
                        }
                    }
                } else if (strcmp(pipeParse, "status") == 0) {
                    char buffer[1024];
                    buffer[0] = '\0';
                    //possível a partir dum array construir uma string com strcat, os bytes_read é o número de bytes necessários
                    char **string = malloc(sizeof(char*) * proc->entries);
                    int i, bytes_read = 0, j, count, total = 0, *c;
                    for (i = proc->aux_array.last, j = 0; i != -1 && j<proc->entries; i = proc->aux_array.array[POS(i, 0)], j++) {
                        //o pedidoToString dá malloc às strings e depois atira apontador
                        string[j] = pedidoToString((Pedido *) proc->tbl[i].value, &(bytes_read));
                        total += bytes_read;
                    }
                    for(i = 0; i<j; i++) {
                        strcat(buffer, string[i]);
                        free(string[i]);
                    }
                    //write(fd_pedido, buffer, total);
                    //escrever num só write
                    
                    for (i = maxs->aux_array.last; i!=-1;i = maxs->aux_array.array[POS(i,0)]) {
                        if (readHT(curr, (char*) (maxs->tbl[i].key), (void**) &c) == -1) {
                            count = 0;
                        } else {
                            count = *c;
                        }
                        //concatenar strings com sprintf
                        bytes_read = snprintf(buffer + total, 1024-total, "%s: %d/%d\n", (char*) (maxs->tbl[i].key), count, *((int*)(maxs->tbl[i].value)));
                        if (total + bytes_read > 1024) {
                            write(fd_pedido, buffer, total);
                            total = 0;
                            i = maxs->aux_array.array[POS(i,1)];
                        } else {
                            total += bytes_read;
                        }
                    }
                    write(fd_pedido, buffer, total); //só um write com todas as linhas prontas, só espero que elas não passem de 1024
                    free(string);
                    close(fd_pedido);
                } else {
                    int fd_pedido;
                    if ((fd_pedido = open(pipeParse, O_WRONLY)) == -1) {
                        write(2, "Failed to open pipe to client\n", 31);
                    }
                    write(fd_pedido, "Pedido não reconhecido.\n", 26);
                    close(fd_pedido);
                    TestMaxPipe(r, bytes_read_pipe, fd_leitura, pipeRead)
                    while (pipeRead[r] != '\0') {
                        r++;
                        TestMaxPipe(r, bytes_read_pipe, fd_leitura, pipeRead)
                    }
                    r++;
                }
            }
        } else if (bytes_read_pipe > 0 && proc_pid_pos == -1) {
            
            int fd_pedido;
            if ((fd_pedido = open(pipeParse, O_WRONLY)) == -1) {
                write(2, "Failed to open pipe to client\n", 31);
            }
            write(fd_pedido, "Servidor em processo de terminação\n", 38);
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
            removeCurr(pedido_read, curr, maxs);
            //retirar aos proc
            waitpid(pid_read, &status, 0);
            //testar os erros do status
            deleteHT(proc, (void *) &pid_read, 1);

            //só está a executar um de cada vez
            Pedido *pedido;
            for (   pedido = choosePendingQueue(pendingQ, maxs, curr, &n_transfs_pendingQ); pedido != NULL; 
                    pedido = choosePendingQueue(pendingQ, maxs, curr, &n_transfs_pendingQ)) {
    
                //adicionar aos em processamento
                if (addCurr(pedido, curr, maxs) == -1) {
                    write(2, "Failed to write to curr\n", 25);
                }
                //avisar o cliente que foi adicionado aos em processamento
                int pid_manager;
                if ((pid_manager = executaPedido(pedido, pasta)) == -1) {
                    write(pedido->fd, "Failed request\n", 16);
                    deepFreePedido(pedido);
                } else {
                    writeHT(proc, (void *) &pid_manager, pedido);
                }
            }
        }
    }
    close(fd_leitura);
}

/**
 * @brief Função que executa o servidor
 * 
 * @param argc Número de argumentos
 * @param argv Argumentos
 * @return código de saída 
 */
int main(int argc, char const *argv[]) {
    int r = 0;
    signal(SIGTERM, term_handler);
    signal(SIGINT, term_handler);
    
    int pid_server = getpid();
    int tamanho = 0, tmp = pid_server;
    while (tmp != 0) {
        tamanho++;
        tmp /= 10;
    }
    char buffer[tamanho+1];
    sprintf(buffer, "%d\n", pid_server);
    write(1, buffer, tamanho+1);

    if((mkfifo("entrada", 0666)) == -1){
        //auxílio para não estar constantemente a remover o ficheiro "entrada"
        unlink("entrada");
        if((mkfifo("entrada", 0666)) == -1){
            write(2, "Failed to create Named pipe entrada\n", 37);
            return -1;
        }
    }

    if ((fd_leitura = open("entrada", O_RDONLY)) == -1) {
        write(2, "Failed to open the named pipe\n", 31);
        return -1;
    }

    if (flag_term) {
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
            freeHT(maxs);
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
        if (initHT(proc, INIT_DICT_SIZE, 1, PID_T, PEDIDO) == -1) {
            write(2, "No space for Hashtable\n", 24);
            freeHT(maxs);
            freeHT(curr);
            freeHT(proc);
            return -1;
        }
        r = run(argv[2], maxs, curr, proc);
        
        freeHT(maxs);
        freeHT(curr);
        freeHT(proc);
    }
    unlink("entrada");
    write(1, "Sepukku gracioso!\n", 19);
    return r;
}
