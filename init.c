#include "init.h"
#include "includes.h"


int strArrayToString(int n, char *array[], char **dest, pid_t pid) {
    if (n>0) {
        char *sep = " ", buffer[33];
        int i,c,s;
        for (i = 0, c = 0, s = -1; i<n; i++, s++) {
            c += strlen(array[i]);
        }
        (*dest) = malloc(c+s+5); // inicial: c + s + 1, como queremos 4 chars para tamanho do pedido fica c + s + 5
        char *target = (*dest);
        pid_t tamanho = pid;
        for(i=0; tamanho!=0; i++){ // contar quantas casas
            tamanho = tamanho/10;
        }
        //if(tamanho > 4) return -1 -> possibilidade ? xD
        //int j;
        //for(j=0; j < 4-i; j++)
        //    target[j] = '0';
        //target += j;
        sprintf(target, "%d %s", pid, array[0]);
        target += strlen(array[0]) + i;
        for(i = 1; i<n; i++) {
            strcat(target, sep);
            target++;
            strcat(target, array[i]);
            target += strlen(array[i]);
        }
        return c+s+5;
    }
    return 0;
}

int readConfig(int fd, HT *hash_table) {
    //ver os casos em que pode falhar
    int bytes_read, i = 0, word_size, tmp;
    char buff[MAX_BUFF], dict_key[MAX_TRANSF_SIZE], dict_value[5];
    bytes_read = read(fd, buff, MAX_BUFF);
    while(bytes_read > 0) {
        word_size = 0;
        while(i<bytes_read && buff[i] != ' ') {
            dict_key[word_size++] = buff[i];
            if (i == bytes_read - 1) {
                bytes_read = read(fd, buff, MAX_BUFF);
                i = 0;
            } else {
                i++;
            }
        }
        dict_key[word_size] = '\0';
        i++; //ew tenho de fazer read aqui again
        if (i == bytes_read) {
            bytes_read = read(fd, buff, MAX_BUFF);
            i = 0;
        }

        word_size = 0;
        while(i<bytes_read && buff[i] != '\n') {
            dict_value[word_size++] = buff[i];
            if (i == bytes_read - 1) {
                bytes_read = read(fd, buff, MAX_BUFF);
                i = 0;
            } else {
                i++;
            }
        }
        dict_value[word_size] = '\0';
        i++;
        tmp = atoi(dict_value);
        writeHT(hash_table, dict_key, &tmp);
        if (i == bytes_read) {
            bytes_read = read(fd, buff, MAX_BUFF);
            i = 0;
        }
    }
    return 0;
}

//esta está testada e funciona
//ver os casos em que pode falhar em -1

