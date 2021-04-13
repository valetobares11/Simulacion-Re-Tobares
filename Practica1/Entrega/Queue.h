#include <stdbool.h> 
#include <stdlib.h>
#include <stdio.h>


//ESTRUCTURA DE COLA
typedef struct nodo {
    float info;
    struct nodo *sig;
}nodoQ;

int cant_element;
nodoQ* raiz;
nodoQ* fondo;

bool isEmpty(){
  return (cant_element == 0);
}
void initialization() {
cant_element = 0; // cantidad de elementos de la cola;
raiz=NULL;
fondo=NULL;
}


void add(float x) {
    nodoQ* new_cte = malloc(sizeof(nodoQ));
    new_cte -> info = x;
    new_cte->sig=NULL;
    if (isEmpty()) {
        raiz = new_cte;
        fondo = new_cte;
    } else {
        fondo->sig = new_cte;
        fondo = new_cte;
    }
    cant_element++;
}

float pop() {
    float time_cte;
    if (!isEmpty()) {
        time_cte = raiz->info;
        nodoQ* removed = raiz;
        if (raiz == fondo) {
            raiz = NULL;
            fondo = NULL;
        } else {
            raiz = raiz->sig;
        }
        free(removed);
        cant_element--;
        return time_cte;
    }
    else
        return time_cte;
}


int size(){
    return cant_element;
}