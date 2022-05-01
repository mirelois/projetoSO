#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include "hashtable.h"

#define StringToBuffer(r, w, string, buffer) \
    for (; string[r] != '\0' && string[r] != ' ' && string[r] != '\n'; w++, r++) {\
       buffer[w] = string[r];\
    }\
    buffer[w] = '\0';\
    if (string[r] != '\0')\
        r++;\

typedef struct pedido {
    //guardar strings para não sobrecarregar fd's do servidor
    int id;
    int n_transfs;
    //apontador para hashtable para guardar os valores dos pedidos
    HT *hashtable;
    char *prio;
    char *pedido;
    char *in;
    char *out;
}Pedido;

int strArrayToString(int n, char *array[], char **dest, int mode) {
    if (n>0) {
        char *sep = " ", buffer[33];
        int i,c,s;
        for (i = 0, c = 0, s = -1; i<n; i++, s++) {
            c += strlen(array[i]);
        }
        if (mode == 1) {
            snprintf(buffer, 33, "%d", n);
            c += strlen(buffer);
            s++;
        }
        (*dest) = malloc(c+s+1);
        char *target = (*dest);
        *target = '\0';
        strcat(target, array[0]);
        target += strlen(array[0]);
        if (mode == 1) {
            strcat(target, sep);
            target++;
            strcat(target, buffer);
            target += strlen(buffer);
        }
        for(i = 1; i<n; i++) {
            strcat(target, sep);
            target++;
            strcat(target, array[i]);
            target += strlen(array[i]);
        }
        return c+s+1;
    }
    return 0;
}

int addTransfHT(char *transf, HT *h, HT *maxs) {
    int max, curr;
    if (readHT(maxs, transf, &max) == -1) {
        write(1, transf, strlen(transf));
        printf(" %s ", h->tbl[9].key);
        printf("%d", strcmp(h->tbl[9].key, transf));
        write(2, "Transformation not in config.\n", 31);
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

int createPedido(char *string, Pedido **dest, HT *maxs, int n_pedido) {
    *dest = malloc(sizeof(Pedido));
    (*dest)->id = n_pedido;
    char buffer[32];
    //supor que tem a prioridade, in e out
    int r = 10, w, i; //saltar o proc-file à frente
    //segunda parte da string é o número de argumentos
    //eu sei que recebi pelo menos 5 coisas, com o proc-file   
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
    initHT((*dest)->hashtable, 13, 0);
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

int main(int argc, char *argv[])
{
    Pedido *pedido;
    HT maxs;
    char *string;
    int decoy;
    initHT(&maxs, 13, 0);
    writeHT(&maxs, "lmao", 4);
    printHT(&maxs,maxs.size);
    strArrayToString(argc-1, argv+1, &string, 0);
    if (createPedido(string, &pedido, &maxs, 0) == 0) {
        printf("%s %s %s\n", pedido->prio, pedido->in, pedido->out);
        printf("%s\n", pedido->pedido);
        printf("%d %d", pedido->id, pedido->n_transfs);
    }
    
    
    return 0;
}
