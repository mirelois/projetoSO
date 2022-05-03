#define INIT_DICT_SIZE 13
#define MAX_CHARGE 0.75
#define MAX_TRANSF_SIZE 32
#define FREE(h,p) (keycmp(h, (h->tbl)[p].key, (void*)EMPTY) == 0 || keycmp(h, (h->tbl)[p].key, (void*)DELETED) == 0)
#define STRING 1
#define INT 0

#define EMPTY       "-"
#define DELETED     "+"


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
    int type;
    struct aux_array aux_array;
    int size;
    int used;
    struct pair *tbl;
} HT;


int hash(HT* h, void* key_void);

int keycmp(HT* h, void* key1, void* key2);

void freeHT(HT *h);

int initHT (HT *h, int size, int aux_array_flag, int type);

int plusOneHT(HT *h, char key[], int* value);

int writeHT (HT *h, void* key, int value);

int readHT (HT *h, void* key, int* value);

int deleteHT (HT *h, char key[]);

int printHT(HT *h, int size);