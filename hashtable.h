#define INIT_DICT_SIZE 13
#define MAX_CHARGE 0.75
#define MAX_TRANSF_SIZE 32

enum type{PID_T, STRING, PEDIDO, INT};

#define POS(i,j) 2*i +j

#define EMPTY_STRING       "-"
#define DELETED_STRING     "+"
#define EMPTY_PID_T         -1
#define DELETED_PID_T       -2

struct pair {
    void* key;
    void* value;
};
    
struct aux_array {
    int *array;
    int last;
    int aux_array_flag;
};
    
typedef struct {
    int value_type;
    int key_type;
    struct aux_array aux_array;
    int size;
    int used;
    struct pair *tbl;
} HT;

typedef struct pedido {
    //guardar strings para não sobrecarregar fd's do servidor
    int id;
    int n_transfs;
    int fd;
    //apontador para hashtable para guardar os valores dos pedidos
    HT *hashtable;
    char *prio;
    char *pedido;
    char *in;
    char *out;
}Pedido;

int hash(HT* h, void* key_void);

int isfreeHT(HT* h, int p);

int keycmp(HT* h, void* key1, void* key2);

void freeHT(HT *h);

int initHT(HT *h, int size, int aux_array_flag, int key_type, int value_type);

int writeHT (HT *h, void* key, void* value);

int readHT (HT *h, void* key, void** value);

int deleteHT (HT *h, void* key);

//to be eliminated
int printHT(HT *h);

void deepFreePedido(Pedido *dest);