#define MAX_BUFF 1024
#define DICT_SIZE 13 //possivel ter que mudar
#define MAX_TRANSF_SIZE 32

#include "hashtable.h"

int strArrayToString(int n, const char *array[], char **dest, int mode);
int readConfigNew(int fd, HT *hash_table);