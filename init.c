#include "init.h"
#include "includes.h"


int strArrayToString(int n, char *array[], char **dest, int mode) {
    if (n>0) {
        char *sep = " ", buffer[33];
        int i,c,s;
        for (i = 0, c = 0, s = -1; i<n; i++, s++) {
            c += strlen(array[i]);
        }
        if (mode == 1) {
            snprintf(buffer, 33, "%d", n);
            c += strlen(buffer);
            s++;
        }
        (*dest) = malloc(c+s+1);
        char *target = (*dest);
        *target = '\0';
        strcat(target, array[0]);
        target += strlen(array[0]);
        if (mode == 1) {
            strcat(target, sep);
            target++;
            strcat(target, buffer);
            target += strlen(buffer);
        }
        for(i = 1; i<n; i++) {
            strcat(target, sep);
            target++;
            strcat(target, array[i]);
            target += strlen(array[i]);
        }
        return c+s+1;
    }
    return 0;
}

int readConfig(int fd, HT *hash_table) {
    //ver os casos em que pode falhar
    initHT(hash_table, INIT_DICT_SIZE);
    int bytes_read, i = 0, word_size;
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
        writeHT(hash_table, dict_key, atoi(dict_value));
        if (i == bytes_read) {
            bytes_read = read(fd, buff, MAX_BUFF);
            i = 0;
        }
    }
    return 0;
}

//esta está testada e funciona
//ver os casos em que pode falhar em -1


