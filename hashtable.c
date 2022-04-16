#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hashtable.h"

int hash(char key[], int size) {
    int ret = 0;
    for(int i =0; key[i] != '\0'; i++){
        ret += key[i];
    }
    return ret%size;
}


void initHT(HT *h, int size) {
    h->tbl = calloc(size, sizeof(struct pair));
    h->size = size;
    h->used = 0;
    for (int i=0; i<size; i++) {
        strcpy(h->tbl[i].key, EMPTY);
        h->tbl[i].value = -1;
    }
}


int freeHT(HT *h, int k) {
    
    return freeHT_sol(h, k);
}

int isprime(int p){
    int i , r = 0;
    for(i = 2 ; i < p/2 ; i++){
        if(p%i == 0){r = 1;}
    }
    return r;
}


//assume que ha espaço
//nao é dinamico
int writeHT (HT *h, char key[], int value) {

    int p= hash(key, h->size);

    for(p; !FREE(p); p = (p+1)%(h->size)){printf("%d\n", p);};

    strcpy((h->tbl)[p].key, key);
    (h->tbl)[p].value = value;
    h->used++;

    return p;
}

/*
int writeHT(HT *h, char key[], int value){
    HT new_h;
    if(((h->used)/(h->size)) <= 0.80){

    }
}
*/

int readHT(HT *h, char key[], int* value){
    int p , r = -1 , c = h->size;
    for(p = hash(key,h->size); strcmp(key,(h->tbl)[p].key) && (c > 0);c--){
        p = (p+1)%(h->size);
    }
    if(c != 0){
        r = p;
        *value = (h->tbl)[p].value;
    }
    return r;
}



int deleteHT (HT *h, char key[]) {
    int x;
    int p = readHT(h, key, &x);
    if(p != -1) {
        strcpy((h->tbl)[p].key, DELETED);
    }
    return p;
}
