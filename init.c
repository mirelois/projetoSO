#include "init.h"
#include "includes.h"

/**
 * @brief Função torna um array de string em uma string.
 * 
 * @param n indices.
 * @param array array de strings.
 * @param dest string
 * @param pid pid
 * @return int código de sucesso
 */
int strArrayToString(int n, char *array[], char **dest, pid_t pid) {
    if (n>0) {
        char *sep = " ", buffer[33];
        int i,j,c,s;
        for (i = 0, c = 0, s = 0; i<n; i++, s++) {
            c += strlen(array[i]);
        }
        
        pid_t tamanho = pid;
        for(j=0; tamanho!=0; j++){ // contar quantas casas
            tamanho = tamanho/10;
        }
        (*dest) = malloc(c+s+i+1); 
        char *target = (*dest);
        //if(tamanho > 4) return -1 -> possibilidade ? xD
        //int j;
        //for(j=0; j < 4-i; j++)
        //    target[j] = '0';
        //target += j;
        sprintf(target, "%d %s", pid, array[0]);
        target += strlen(array[0]) + j;
        for(i = 1; i<n; i++) {
            strcat(target, sep);
            target++;
            strcat(target, array[i]);
            target += strlen(array[i]);
        }
        target++;
        *target = '\0';
        return c+s+j+1;
    }
    return 0;
}


/**
 * @brief Função que lê o config
 * 
 * @param fd fd.
 * @param hash_table hashtable para ler. 
 * @return int código de sucesso.
 */
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

