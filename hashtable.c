#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hashtable.h"

/**
 * @brief hash function
 * 
 * sum of all ASCII values of characters in a string ajusted to the size of h
 * integer ajusted to the size of h
 * 
 * @param h hashtable for which the hash value is necessary 
 * @param key_void key to be hashed
 * @return int
 */
int hash(HT* h, void* key_void) {

    int ret = 0; //default hash

    //key type check
    if(h->key_type == STRING) {

        char* key = (char*)key_void;
        

        for(int i =0; key[i] != '\0'; i++){
           ret += key[i];
        }

    }else if(h->key_type == INT) {

        ret = *((int*)key_void);

    }else {
        return -1;//error condition if key type is not defined
    }
    
    return ret%(h->size);
    
}

/**
 * @brief checks if given position in hashtable is free to write on
 * 
 * returns false if h->key_type is not defined
 * 
 * @param h hashtable on whitch to check
 * @param p position to check
 * @return int 
 */
int isfreeHT(HT* h, int p) {
    if(h->key_type == STRING) {
        return strcmp((char*)h->tbl[p].key, EMPTY_STRING) == 0 || strcmp((char*)h->tbl[p].key, DELETED_STRING) == 0;
    }
    if(h->key_type == INT) {
        return *((int*)h->tbl[p].key) == EMPTY_INT || *((int*)h->tbl[p].key) == DELETED_INT;
    }
    return 0;
}

/**
 * @brief frees allocated memory pointed to by h->tbl[p].value
 * 
 * @param h hashtable on whitch memory will be freed
 * @param p position on whitch memory should be freed
 * @return int 
 */
int freeValueHT(HT* h, int p) {
    //value type check
    if(h->value_type == PEDIDO) {
        deepFreePedido(h->tbl[p].value);//deep free of pedido
    }else if (h->value_type == INT) {
        free(h->tbl[p].value);// simple free for integer
    }else {
        return -1;//error condition if typr not defined
    }
    return 0;
}

/**
 * @brief compares two given keys of the same type
 * 
 * returns 0 if keys are equal
 * returns <0 if key1 < key2
 * returns >0 if key1 > key2
 * 
 * @param h hashtable on whitch keys will be compared
 * @param key1 
 * @param key2 
 * @return int 
 */
int keycmp(HT* h, void* key1, void* key2) {
    //type check
    if(h->key_type == STRING) {
        return strcmp((char*)key1, (char*)key2);
    }
    if(h->key_type == INT) {
        return *((int*)key1) - *((int*)key2);
    }
    return -1;//error condition if type not defined
}

/**
 * @brief inicializa um dicionario
 * 
 * aux_array is alocated as simulated matrix
 * 
 * @param h pointer to hashtable to be created
 * @param size size of new hashtable
 * @param aux_array_flag flag indicating if an auxiliary array is to be created
 * @param key_type type of key
 * @param value_type type of value
 * 
 * @return int
 */
int initHT(HT *h, int size, int aux_array_flag, int key_type, int value_type) {

    //aux_array_flag check
    if (aux_array_flag) {
        if((h->aux_array.array = malloc(2*size*sizeof(int))) == NULL) { //aux_array_flag initialization
            return -1;//error condition if malloc fails
        };
        h->aux_array.last = -1; //initial last position is set initialy as -1
    }else {
        h->aux_array.array = NULL;// if aux_array is not requested aux_array pointer is set to NULL
    }

    

    if((h->tbl = calloc(size, sizeof(struct pair))) == NULL){//memory allocation for hashtable
        return -1;//error condition if calloc fails
    }

    //setting initial parameters 
    h->value_type = value_type;
    h->key_type = key_type;
    h->aux_array.aux_array_flag = aux_array_flag;
    h->size = size;
    h->used = 0;

    //key type check
    if(h->key_type == STRING) {
        //if key is set to STRING, MAX_TRANSF_SIZE*sizeof(char) bytes of memory are allocated
        //for eatch position in hashtable

        for (int i=0; i<size; i++) {
            if((h->tbl[i].key = (void*)malloc(MAX_TRANSF_SIZE*sizeof(char))) == NULL){
                return -1;
            }
            strcpy(h->tbl[i].key, EMPTY_STRING);
            h->tbl[i].value = NULL;
        }

    }else if(h->key_type == INT) {
        //if key is set to INT sizeof(pid_t) bytes of memory are allocated
        //for eatch position in hashtable
        for (int i=0; i<size; i++) {
            pid_t* tmp = malloc(sizeof(pid_t));
            *tmp = EMPTY_INT;
            h->tbl[i].key = (void*)tmp;
            h->tbl[i].value = NULL;
        }

    }else {
        return -1;//error condition if key_type is not defined
    }
    return 0;
}

/**
 * @brief auxiliary function to use in writeHT
 * 
 * @param h 
 */
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
 * @brief frees all memory allocated for a given hashtable 
 * 
 * @param h hashtable to free
 */
void freeHT(HT *h) {
    //aux_array check
    if(h->aux_array.aux_array_flag) {//if aux array has ben created it must be freed
        free(h->aux_array.array);
    }

    for(int i = 0; i < h->size; i++) {
        
        if(h->tbl[i].value != NULL){//free all allocated values
            freeValueHT(h, i);
        }

        free(h->tbl[i].key);//free all keys
    }
    free(h->tbl);//free hashtable
    free(h);//free pointer to h
}

/**
 * @brief checks if number is prime 
 * 
 * 
 * @param p value to test
 * @return int
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
 * @param h hashtable on whitch to write
 * @param key key to add to hashtable
 * @param value value to associate with key
 * @return int
 */
int writeHTaux (HT *h, void* key, void* value) {

    int p = hash(h, key);
    int flag = 1;

    //increments p until free position is found
    for(p; !isfreeHT(h, p) && (flag = keycmp(h, key,(h->tbl)[p].key)); p = (p+1)%(h->size));

    //if value is already allocated free it
    if(h->tbl[p].value != NULL){
        freeValueHT(h, p);
    }

    //copy value from value to hashtable
    if (h->value_type == PEDIDO) {
        h->tbl[p].value = value;
    }else if (h->value_type == INT) {
        int* tmp = malloc(sizeof(int));
        *tmp = *(int*)value;
        h->tbl[p].value = (void*)tmp;
    }else {
        return -1;
    }
    
    //if key is new to hashtable write it on to hashtable
    if (flag) {
        
        //update aux_array
        if (h->aux_array.aux_array_flag) {
            int last = h->aux_array.last;
            if (last != -1) {
                h->aux_array.array[POS(last,1)] = p;
            }
            h->aux_array.array[POS(p,0)] = last;
            h->aux_array.last = p;
        }

        //copy key from key to hashtable
        if (h->key_type == STRING) {
            strcpy(h->tbl[p].key, (char*)key);
        }else if (h->key_type == INT) {
            *(pid_t*)(h->tbl[p].key) = *(pid_t*)(key);
        }

        h->used++;//increment used

    }

    return p;
}

/**
 * @brief writes pair key,value in hashtable 
 * 
 * if charge passes MAX_CHARGE
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


        if(initHT(new_h, new_size, h->aux_array.aux_array_flag, h->key_type, h->value_type) == -1){
            return -1;
        }

        for(int i = 0; i < h->size; i++) {
            if (!isfreeHT(h, i)){
                writeHTaux(new_h, h->tbl[i].key, h->tbl[i].value);
            }
        }

        AuxFree(h);

        if (h->aux_array.aux_array_flag) {
            h->aux_array.array = new_h->aux_array.array;
            h->aux_array.last = new_h->aux_array.last;
        }
        
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
    if (h->key_type == STRING) {
        empty = (void*)EMPTY_STRING;
    }else if (h->key_type == INT) {
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

        if (h->key_type == STRING) {
            strcpy(h->tbl[p].key, DELETED_STRING);
        }else if (h->key_type == INT) {
            *(pid_t*)(h->tbl[p].key) = DELETED_INT;
        }

        freeValueHT(h, p);
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

int printHT(HT *h) {
    if(h->key_type == INT){
        for(int i = 0; i < h->size; i++) {
            if((h->tbl)[i].value != NULL){
                printf("%d -> (%d,%d)\n",i ,*((int*)((h->tbl)[i].key)), *(int*)((h->tbl)[i].value));
            }else{
                printf("%d -> (%d,NULL)\n",i ,*((int*)((h->tbl)[i].key)));
            }
            
        }
    }else if(h->key_type == STRING) {
        for(int i = 0; i < h->size; i++) {
            printf("%d -> (%s,%d)\n",i ,(char*)((h->tbl)[i].key), *((int*)(h->tbl)[i].value));
        }
    }
    putchar('\n');
    return 0;
    
}

void deepFreePedido(Pedido *dest) {
    if (dest->hashtable) {
        freeHT(dest->hashtable);
    }
    free(dest->in);
    free(dest->out);
    free(dest->prio);
    free(dest->pedido);
    close(dest->fd);
}

int printPedido(Pedido *p) {
    printf("id->%d\nn_tranfs->%d\nprio->%s\npedido->%s\nin->%s\nout->%s\n", p->id, p->n_transfs, p->prio,p->pedido, p->in, p->out);
    printf("hashtable:\n");
    printHT(p->hashtable);
}