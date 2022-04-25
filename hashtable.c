#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hashtable.h"

/**
 * @brief funcao de hash
 * 
 * soma o valor de ascii de todos os caracteres de uma string
 * 
 * @param key string a qual a funçao sera aplicada
 * @param size tamanho do dicionario
 * @return 
 */
int hash(char key[], int size) {
    int ret = 0;
    for(int i =0; key[i] != '\0'; i++){
        ret += key[i];
    }
    return ret%size;
}

/**
 * @brief inicializa um dicionario
 * 
 * 
 * @param h pointer de um dicionario a ser inicializado
 * @param size tamanho do dicionario
 * @return 0 se o dicionario for inicializado corretamente e -1 caso contrario
 */
int initHT(HT *h, int size) {
    h->tbl = calloc(size, sizeof(struct pair));

    if(h->tbl == NULL){
        return -1;
    }

    h->size = size;
    h->used = 0;
    for (int i=0; i<size; i++) {
        strcpy(h->tbl[i].key, EMPTY);
        h->tbl[i].value = -1;
    }
    return 0;
}

/**
 * @brief incrementa o valor associado a uma chave por um 
 * 
 * 
 * @param h pointer de um dicionario onde atualizar o valor
 * @param key chave cujo valor associado sera incrementado
 * @param value pointer de um valor onde guardar o valor incrementado
 * @return posicao onde o valor foi incrementado ou -1 de falhar
 */
int plusOneHT(HT *h, char key[], int *value) {

    int x;
    int p = readHT(h, key, &x);
    if(p != -1) {
        (h->tbl)[p].value++;
        *value = x+1;
    }
    return p;
}

/**
 * @brief liberta a memoria de um dicionario 
 * 
 * 
 * @param h pointer de um dicionario a libertar
 * @return 0 se p nao e primo 1 caso contrario
 */
void freeHT(HT *h) {
    free(h->tbl);
    free(h);
}

/**
 * @brief verifica se um numero é primo 
 * 
 * 
 * @param p valor a testar
 * @return 0 se p nao e primo 1 caso contrario
 */
int isprime(int p){
    int i , r = 0;
    for(i = 2 ; i < p/2 ; i++){
        if(p%i == 0) {
            r = 1;
        }
    }
    return r;
}


/**
 * @brief funcao auxiliar do writeHT 
 * 
 * 
 * @param h pointer para o dicionario onde escrever
 * @param key chave a colocar no dicionario
 * @param value valor a associar com a chave no dicionario
 * @return posicao onde escreveu
 */
int writeHTaux (HT *h, char key[], int value) {

    int p= hash(key, h->size);

    for(p; !FREE(h,p); p = (p+1)%(h->size));

    strcpy((h->tbl)[p].key, key);
    (h->tbl)[p].value = value;
    h->used++;

    return p;
}


/**
 * @brief escreve um par chave valor no dicionario 
 * 
 * quando a carga passa de MAX_CHARGE o tamanho do 
 * dicionario é aumentado para o menor numero primo maior do que o dobro do tamanho atual
 * 
 * assume que chave nao ocorre no dicionario
 * 
 * @param h pointer para o dicionario onde escrever
 * @param key chave a colocar no dicionario
 * @param value valor a associar com a chave no dicionario
 * @return posiçao onde colocou o par ou -1 se falhar
 */
int writeHT (HT *h, char key[], int value) {

    float charge = ((float)(h->used))/(h->size);

    if(charge >= MAX_CHARGE) {

        HT *new_h = malloc(sizeof(HT));

        if(new_h == NULL){
            return -1;
        }

        int new_size;

        for(new_size = (h->size)*2; !isprime(new_size); new_size++);

        if(initHT(new_h, new_size+1) == -1){
            return -1;
        }

        for(int i = 0; i < h->size; i++) {
            
            writeHTaux(new_h, h->tbl[i].key, h->tbl[i].value);
            
        }

        *h = *new_h;

        freeHT(new_h);
        
    }

    return writeHTaux(h, key, value);
}

/**
 * @brief lê o valor associado a uma chave
 * 
 * assume que chave so aparece uma ves no dicionario
 * 
 * @param h pointer para o dicionario de onde ler
 * @param key chave
 * @param value pointer onde é colocado o valor associado a chave
 * @return posicao na tabela de onde foi lido o valor ou -1 caso a chave nao exista no dicionario
 */
int readHT(HT *h, char key[], int* value){
    int p , r = -1 , c = h->size, flg = 1;
    for(p = hash(key,h->size); strcmp(key,(h->tbl)[p].key) && (c > 0) && (flg = strcmp(EMPTY,(h->tbl)[p].key)); c--){
        p = (p+1)%(h->size);
    }
    if(c != 0 && flg){
        r = p;
        *value = (h->tbl)[p].value;
    }
    return r;
}

/**
 * @brief subtitui a chave por DELETED e coloca o valor guardado a -1
 * 
 * assume que chave so aparece uma ves no dicionario
 * 
 * @param h pointer para o dicionario em que sera eliminado o valor
 * @param key chave a eliminar
 * @return posicao na tabela de onde foi eliminada a chave ou -1 caso a chave nao exista no dicionario
 */
int deleteHT (HT *h, char key[]) {
    int x;
    int p = readHT(h, key, &x);
    if(p != -1) {
        strcpy((h->tbl)[p].key, DELETED);
    }
    return p;
}

int printHT(HT *h, int size) {
    for(int i = 0; i < size; i++) {
        printf("%d -> (%s,%d)\n",i ,(h->tbl)[i].key, (h->tbl)[i].value);
    }
    return 0;
}