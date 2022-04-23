#include "init.h"
#include "includes.h"


int strArrayToString(int n, char *array[], char **dest, int mode) {
    char *sep = " ", buffer[33];
    int i,c,s;
    itoa(n, buffer, 10);
    
    for (i = 0, c = 0, s = -1; i<n; i++, s++) {
        c += strlen(array[i]);
    }
    if (mode == 1) {
        c += strlen(buffer);
        s++;
    }
    (*dest) = malloc(c+s+1);
    char *target = (*dest);
    *target = '\0';
    if (mode == 1) {
        strcat(target, buffer);
        target += strlen(buffer);
        strcat(target, sep);
        target++;
    }
    for(i = 0; i<n; i++) {
        if (i > 0) {
            strcat(target, sep);
            target++;
        }
        strcat(target, array[i]);
        target += strlen(array[i]);
    }
    return c+s+1;
}

int readConfigNew(int fd, HT *hash_table) {
    //ver os casos em que pode falhar
    initHT(hash_table, DICT_SIZE);
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
        dict_key[word_size+1] = '\0';
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
        dict_value[word_size+1] = '\0';
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

ssize_t readConfig(int fd, HT *hash_table) {

    initHT(hash_table, INIT_DICT_SIZE);

    int bytes_read;
    char buff[MAX_BUFF];

    int flag_continue = 0;
    
    while((bytes_read = read(fd, buff, MAX_BUFF)) > 0) {
        if(flag_continue){
            flag_continue = 0;
            continue;
        }

        int i = 0;
        int word_size = 0;

        int dist = 0;

        int flag_bytes_read;
        for(i = 0; i < bytes_read; i++) {

            for(i; buff[i] != ' ' && (flag_bytes_read = (i < bytes_read)); i++) {
                word_size++;
            }// passa para o prox ' '

            dist = word_size;

            if (!flag_bytes_read){
                lseek(fd, -dist, SEEK_CUR);

                flag_continue = 1;
            }
            char dict_key[word_size+1]; // declara key com max size = bytes_read. E se istruçao 

            strncpy(dict_key, buff+i-word_size, word_size);
            dict_key[word_size] = '\0';

            word_size = 0;

            i++;



            for(i; buff[i] != '\n' && (flag_bytes_read = (i < bytes_read)); i++) {
                word_size++;
            }

            dist += word_size;

            if (!flag_bytes_read){
                lseek(fd, -dist, SEEK_CUR);
                flag_continue = 1;
            }


            char dict_value[word_size+1];
    
            strncpy(dict_value, buff+i-word_size, word_size);
            dict_value[word_size] = '\0';

            

            word_size = 0;

            

            dist = 0;

            writeHT(hash_table, dict_key, atoi(dict_value));
    }   

}



