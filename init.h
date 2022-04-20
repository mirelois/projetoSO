#define MAX_BUFF 1024
#define DICT_SIZE 13 //possivel ter que mudar

char *strArrayToString(int n, char *array[], char **dest);
int readConfigNew(int fd, HT *hash_table);