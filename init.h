#define MAX_BUFF 1024
#define DICT_SIZE 13 //possivel ter que mudar

#include "hashtable.h"

int strArrayToString(int n, const char *array[], char **dest);
int readConfigNew(int fd, HT *hash_table);