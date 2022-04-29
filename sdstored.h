#include "unistd.h"
#include "errno.h"
#include "string.h"
//#include "hashtable.h"
#include "stdio.h"
#include <fcntl.h>
#include "init.h"
#include "malloc.h"


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

typedef struct nodo {
    Pedido *pedido;
    struct nodo *next;
}LList;

typedef struct cabeca {
    LList *start;
    LList *end;
}PendingQueue;
