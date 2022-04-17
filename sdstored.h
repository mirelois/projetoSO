#include "unistd.h"
#include "errno.h"
#include "string.h"
#include "hashtable.h"
#include "stdio.h"
#include <fcntl.h>

typedef struct pedido {
    //guardar strings para não sobrecarregar fd's do servidor
    char *file_in;
    char *file_out;
    char *transfs[];
    int n_transfs;
    int priority;
    //apontador para hashtable para guardar os valores dos pedidos
    HT *hashtable;
}Pedido;