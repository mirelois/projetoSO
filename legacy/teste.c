#define MAX_BUFF 9
#define DICT_SIZE 13 //possivel ter que mudar
#include "includes.h"
#include "hashtable.h"

int readConfigNew(int fd, HT *hash_table) {
    initHT(hash_table, DICT_SIZE);
    int bytes_read, i = 0, word_size;
    char buff[MAX_BUFF], dict_key[20], dict_value[5];
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

ssize_t readConfig(int fd, HT *hash_table) {

    initHT(hash_table, DICT_SIZE);

    int bytes_read;
    char buff[MAX_BUFF];

    int flag_continue = 0;
    
    while((bytes_read = read(fd, buff, MAX_BUFF)) > 0) {
        if(flag_continue){
            flag_continue = 0;
            continue;
        }
        printf("\n\nbuffer(%s)\n\n", buff);
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
                printf("\ndist-1-(%d)\n",dist);
                lseek(fd, -10, SEEK_CUR);

                flag_continue = 1;
                break;
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
                printf("\ndist-2-(%d)\n",dist);
                lseek(fd, dist, SEEK_CUR);
                flag_continue = 1;
                break;
            }


            char dict_value[word_size+1];
    
            strncpy(dict_value, buff+i-word_size, word_size);
            dict_value[word_size] = '\0';

            

            word_size = 0;

            

            dist = 0;

            writeHT(hash_table, dict_key, atoi(dict_value));
        }   

    }
}

int printHT(HT *h, int size) {
    for(int i = 0; i < size; i++) {
        printf("%d -> (%s,%d)\n",i ,(h->tbl)[i].key, (h->tbl)[i].value);
    }
    return 0;
}



int main() {

    int fd = open("sdstored.conf", O_RDONLY, 444);

    HT *h = malloc(sizeof(HT));

    //printf("a\n");
    initHT(h, 13);
    //printf("b\n");
    readConfig(fd, h);
    /*
    writeHT(h, "key1", 1);
    writeHT(h, "key2", 2);
    writeHT(h, "key3", 3);
    writeHT(h, "key4", 4);
    writeHT(h, "key5", 5);
    writeHT(h, "key12", 12);
     writeHT(h, "key13", 13);
     */
    printHT(h, 13);
    //printf("c\n");
    return 0;
}





