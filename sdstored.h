#include "unistd.h"
#include "errno.h"
#include "string.h"
//#include "hashtable.h"
#include "stdio.h"
#include <fcntl.h>
#include "init.h"
#include "limits.h"
#include "malloc.h"

typedef struct nodo {
    Pedido *pedido;
    struct nodo *next;
}LList;

typedef struct cabeca {
    LList *start;
    LList *end;
}PendingQueue;
