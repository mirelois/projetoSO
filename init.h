#define MAX_BUFF 1024
#define MAX_PRIORITY 5

#include "hashtable.h"

int strArrayToString(int n, const char *array[], char **dest, int mode);
int readConfigNew(int fd, HT *hash_table);