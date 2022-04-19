#include "unistd.h"
#include "errno.h"
#include "string.h"
#include "hashtable.h"
#include "stdio.h"
#include <fcntl.h>

typedef struct pedido {
    //guardar strings para não sobrecarregar fd's do servidor
    char *pedido[];
    int n_transfs;
    //apontador para hashtable para guardar os valores dos pedidos
    HT *hashtable;
}Pedido;