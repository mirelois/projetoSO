
#define MAX_PRIORITY 5
#define MAX_BUFF 1024

#include "hashtable.h"
#include "includes.h"

int readConfig(int fd, HT *hash_table);
int strArrayToString(int n, char *array[], char **dest, pid_t pid);


