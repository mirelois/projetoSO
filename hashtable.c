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
int hash(HT* h, void* key_void) {

    int ret = 0;

    if(h->type == STRING) {

        char* key = (char*)key_void;
        

        for(int i =0; key[i] != '\0'; i++){
           ret += key[i];
        }

    }else if(h->type == INT) {

        ret = *((int*)key_void);

    }else {
        return -1;//condiçao de erro se o user for alta palhaço
    }
    
    return ret%(h->size);
    
}

int isfreeHT(HT* h, int p) {
    if(h->type == STRING) {
        return strcmp((char*)h->tbl[p].key, EMPTY_STRING) == 0 || strcmp((char*)h->tbl[p].key, DELETED_STRING) == 0;
    }
    if(h->type == INT) {
        return *((int*)h->tbl[p].key) == EMPTY_INT || *((int*)h->tbl[p].key) == DELETED_INT;
    }

}

int keycmp(HT* h, void* key1, void* key2) {
    if(h->type == STRING) {
        return strcmp((char*)key1, (char*)key2);
    }
    if(h->type == INT) {
        return *((int*)key1) - *((int*)key2);
    }
}

/**
 * @brief inicializa um dicionario
 * 
 * 
 * @param h pointer de um dicionario a ser inicializado
 * @param size tamanho do dicionario
 * @return 0 se o dicionario for inicializado corretamente e -1 caso contrario
 */
int initHT(HT *h, int size, int aux_array_flag, int type) {

    if (aux_array_flag) {
        if((h->aux_array.array = malloc(2*size*sizeof(int))) == NULL) {
            return -1;
        };
        h->aux_array.last = -1;
    }else {
        h->aux_array.array = NULL;
    }

    h->tbl = calloc(size, sizeof(struct pair));

    if(h->tbl == NULL){
        return -1;
    }

    h->type = type;
    h->aux_array.aux_array_flag = aux_array_flag;
    h->size = size;
    h->used = 0;

    if(h->type == STRING) {

        for (int i=0; i<size; i++) {
            h->tbl[i].key = (void*)malloc(MAX_TRANSF_SIZE*sizeof(char));
            strcpy(h->tbl[i].key, EMPTY_STRING);
            h->tbl[i].value = NULL;
        }

    }else if(h->type == INT) {
        
        for (int i=0; i<size; i++) {
            pid_t* tmp = malloc(sizeof(pid_t));
            *tmp = EMPTY_INT;
            h->tbl[i].key = (void*)tmp;
            h->tbl[i].value = NULL;
        }

    }else {
        return -1;//condiçao de erro se o user for alta palhaço
    }
    return 0;
}

void AuxFree(HT *h) {
    if(h->aux_array.aux_array_flag) {
        free(h->aux_array.array);
    }
    for(int i = 0; i < h->size; i++) {

        free(h->tbl[i].key);
    }
    free(h->tbl);
}

/**
 * @brief liberta a memoria de um dicionario 
 * 
 * 
 * @param h pointer de um dicionario a libertar
 * @return 0 se p nao e primo 1 caso contrario
 */
void freeHT(HT *h) {
    if(h->aux_array.aux_array_flag) {
        free(h->aux_array.array);
    }
    for(int i = 0; i < h->size; i++) {
        
        if(h->tbl[i].value != NULL){
            free(h->tbl[i].value);
        }

        free(h->tbl[i].key);
    }
    free(h->tbl);
    //free(h);
}

/**
 * @brief verifica se um numero é primo 
 * 
 * 
 * @param p valor a testar
 * @return 0 se p nao e primo 1 caso contrario
 */
int isprime(int p){
    int i;
    for(i = 2 ; i <= p/2 ; i++){
        if(p % i == 0) {
            return 0;
        }
    }
    return 1;
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
int writeHTaux (HT *h, void* key, void* value) {

    int p = hash(h, key);
    int flag = 1;

    for(p; !isfreeHT(h, p) && (flag = keycmp(h, key,(h->tbl)[p].key)); p = (p+1)%(h->size));
    
    free(h->tbl[p].value);

    if (!flag) {
        (h->tbl)[p].value = value;
    }else {

    if (h->aux_array.aux_array_flag) {
        int last = h->aux_array.last;
        if (last != -1) {
            h->aux_array.array[POS(last,1)] = p;
        }
        h->aux_array.array[POS(p,0)] = last;
        h->aux_array.last = p;
    }

    if (h->type == STRING) {
        strcpy(h->tbl[p].key, (char*)key);
    }else if (h->type == INT) {
        *(pid_t*)(h->tbl[p].key) = *(pid_t*)(key);
    }
    (h->tbl)[p].value = value;
    h->used++;

    }

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
int writeHT (HT *h, void* key, void* value) {

    float charge = (h->used + 1.0)/(h->size);

    if(charge >= MAX_CHARGE) {

        HT *new_h = malloc(sizeof(HT));

        if(new_h == NULL){
            return -1;
        }

        int new_size;

        for(new_size = (h->size)*2; !isprime(new_size); new_size++);


        if(initHT(new_h, new_size, h->aux_array.aux_array_flag, h->type) == -1){
            return -1;
        }

        for(int i = 0; i < h->size; i++) {
            if (!isfreeHT(h, i)){
                writeHTaux(new_h, h->tbl[i].key, h->tbl[i].value);
            }
        }

        if (h->aux_array.aux_array_flag) {
            h->aux_array.array = new_h->aux_array.array;
            h->aux_array.last = new_h->aux_array.last;
        }
        AuxFree(h);
        h->tbl = new_h->tbl;
        h->size = new_h->size;
        h->used = new_h->used;
        
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
int readHT(HT *h, void* key, void** value){
    int p , r = -1 , c = h->size, flg = 1;
    void* empty;
    if (h->type == STRING) {
        empty = (void*)EMPTY_STRING;
    }else if (h->type == INT) {
        int tmp = EMPTY_INT;
        empty = (void*)&tmp;
    }
    for(p = hash(h, key); keycmp(h, key,(h->tbl)[p].key) && (c > 0) && (flg = keycmp(h, empty,(h->tbl)[p].key)); c--){
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
int deleteHT (HT *h, void* key) {

    void* x;
    int p = readHT(h, key, &x);

    if(p != -1) {

        if (h->type == STRING) {
            strcpy(h->tbl[p].key, DELETED_STRING);
        }else if (h->type == INT) {
            *(pid_t*)(h->tbl[p].key) = DELETED_INT;
        }

        free(h->tbl[p].value);
        h->tbl[p].value = NULL;

        if (h->aux_array.aux_array_flag) {

            int last = h->aux_array.last;

            if (p == last) {
                h->aux_array.last = h->aux_array.array[POS(p,0)];
            }
            
            int i1 = h->aux_array.array[POS(p,0)];
            int i2 = h->aux_array.array[POS(p,1)];
            h->aux_array.array[POS(i1,1)] = h->aux_array.array[POS(p,1)];
            h->aux_array.array[POS(i2,0)] = h->aux_array.array[POS(p,0)];

        }

    }
    return p;
}
