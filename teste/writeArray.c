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

int createPedido(char *string, Pedido **dest, HT *maxs, int n_pedido) {
    *dest = malloc(sizeof(Pedido));
    (*dest)->id = n_pedido;
    char buffer[32];
    //supor que tem a prioridade, in e out
    int r = 0, w, n, i = 0, c = 0; //saltar o proc-file à frente
    //segunda parte da string é o número de argumentos
    //eu sei que recebi pelo menos 5 coisas, com o proc-file
    
    for(; string[r] != '\0' && i<4; i++) {
        for (; string[r] != ' '; r++);
        r++;
    }

    (*dest)->hashtable = malloc(sizeof(HT));
    initHT((*dest)->hashtable, 13);
    for(; string[r] != '\0'; i++) {
        StringToBuffer(r, string, buffer)
        //temos de ver os espaços, ir adicionando ao HT
        if ((w = addTransfHT(buffer, (*dest)->hashtable, maxs)) == 1) {
            //pedido rejeitado
            return 1;
        } else if (w == -1) {
            //erro de execução
            return -1;
        }
    }

    (*dest)->pedido = strdup(string);
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
    
    return 0;
}
