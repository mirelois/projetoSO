#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include "hashtable.h"

#define StringToBuffer(r, string, buffer) \
    for (w = 0; string[r] != '\0' && string[r] != ' '; w++, r++) {\
       buffer[w] = string[r];\
    }\
    buffer[w] = '\0';\
    r++;\

typedef struct pedido {
    //guardar strings para não sobrecarregar fd's do servidor
    char **transfs;
    int n_transfs;
    //apontador para hashtable para guardar os valores dos pedidos
    HT *hashtable;
}Pedido;

int createPedido(char *string, Pedido **dest, HT *maxs) {
    *dest = malloc(sizeof(Pedido));
    HT *h = malloc(sizeof(HT));
    initHT(h, 13);
    char buffer[32];
    //supor que tem a prioridade, in e out
    int r = 0, w, n, i = 0;
    StringToBuffer(r, string, buffer)
    n = atoi(buffer);
    (*dest)->transfs = malloc(n*sizeof(char*));
    for(; string[r] != '\0' && i<3; n--) {
        StringToBuffer(r, string, buffer)
        //escrever o buffer para os transfs
        if(((*dest)->transfs[i++] = strdup(buffer)) == NULL) {
            write(2, "Problem with memory.", 21);
            return -1;
        }
    }
    if (n < 0) {
        //já sabemos que tem prioridade porque o cliente já viu
        write(2, "Problem finding input/output files.", 36);
        return -1;
    }
    for(; string[r] != '\0' && i < n+3; r++) {
        StringToBuffer(r, string, buffer)
        //escrever o buffer para os transfs
        (*dest)->transfs[i++] = strdup(buffer);
        //temos de ver os espaços, ir adicionando ao HT
        int max, curr;
        
        if (readHT(maxs, buffer, &max) == -1) {
            printf("%s %d\n", buffer, max);
            write(2, "Transformation not in config.", 31);
            return -1;
        }
        if (plusOneHT(h, buffer, &curr) == -1) {
            writeHT(h, buffer, 1);
            curr = 1;
        }
        if (curr > max) {
            //rejeitar o pedido
        }
    }
    (*dest)->hashtable = h;
    (*dest)->n_transfs = n;
    return 0;
}

int strArrayToString(int n, char *array[], char **dest, int mode) {
    //mode: 0 or 1
    char *sep = " ", buffer[33];
    int i,c,s;
    itoa(n, buffer, 10);
    
    for (i = 0, c = 0, s = -1; i<n; i++, s++) {
        c += strlen(array[i]);
    }
    if (mode == 1) {
        c += strlen(buffer);
        s++;
    }
    (*dest) = malloc(c+s+1);
    char *target = (*dest);
    *target = '\0';
    if (mode == 1) {
        strcat(target, buffer);
        target += strlen(buffer);
        strcat(target, sep);
        target++;
    }
    for(i = 0; i<n; i++) {
        if (i > 0) {
            strcat(target, sep);
            target++;
        }
        strcat(target, array[i]);
        target += strlen(array[i]);
    }
    return 0;
}

int printHT(HT *h, int size) {
    for(int i = 0; i < size; i++) {
        printf("%d -> (%s,%d)\n",i ,(h->tbl)[i].key, (h->tbl)[i].value);
    }
    return 0;
}

int main(int argc, char *argv[])
{
    Pedido *pedido;
    HT maxs;
    initHT(&maxs, 11);
    char* s = "da";
    writeHT(&maxs, "da", 3);
    printHT(&maxs, maxs.size);
    char *pot;
    int r;
    printf("%d %d\n", readHT(&maxs, "da", &r), r);
    strArrayToString(argc-1, argv+1, &pot, 1);
    printf("%s\n", pot);
    createPedido(pot, &pedido, &maxs);
    printf("%d\n", pedido->n_transfs);
    for (int i = 0; i<pedido->n_transfs+3; i++) {
        printf("%s\n", pedido->transfs[i]);
    }
    printHT(pedido->hashtable, pedido->hashtable->size);
    //free(pot);
    //free(pedido->hashtable);
    //free(pedido->transfs);
    //free(pedido);
    //char *pot;
    //strArrayToString(argc-1, argv+1, &pot, 1);
    //printf("%s\n", pot);
    //printf("Escrito\n");
    
    //char *lido[argc-1];
    return 0;
}
