#define MAX_BUFF 1024
#define DICT_SIZE 13 //possivel ter que mudar
#include "includes.h"
#include "hashtable.h"


//n sei se funciona

ssize_t readConfig(int fd, size_t size, HT *hash_table) {

    initHT(hash_table, DICT_SIZE);

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
            dict_key[word_size+1] = '\0';

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
            dict_value[word_size+1] = '\0';

            

            word_size = 0;

            

            dist = 0;

            writeHT(hash_table, dict_key, atoi(dict_value));
    }   

}



