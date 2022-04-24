#define INIT_DICT_SIZE 13
#define MAX_CHARGE 0.75
#define MAX_TRANSF_SIZE 32
#define FREE(h,p) strcmp((h->tbl)[p].key, EMPTY) == 0 || strcmp((h->tbl)[p].key, DELETED) == 0

#define EMPTY       "-"
#define DELETED     "+"


struct pair {
    char key[MAX_TRANSF_SIZE];
    int value;
};
    
typedef struct {
    int   size;
    int   used;
    struct pair *tbl;
} HT;


int hash(char key[], int size);

void freeHT(HT *h);

int initHT (HT *h, int size);

int plusOneHT(HT *h, char key[], int* value);

int writeHT (HT *h, char key[], int value);

int readHT (HT *h, char key[], int* value);

int deleteHT (HT *h, char key[]);

int printHT(HT *h, int size);