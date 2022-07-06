/* Copyright 2021 <> */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "linked_list.h"
#include "utils.h"

//  functia de initializare si alocare lista inlantuita
linked_list_t* ll_create(unsigned int data_size)
{
    linked_list_t* list;

    list = malloc(sizeof(linked_list_t));
    DIE(!list, "malloc list");

    //  initial lista este goala
    list->head = NULL;
    list->size = 0;

    //  dimensiunea informatiilor stocate in nod
    list->data_size = data_size;

    return list;
}

//  functia creeaza un nou nod pe baza datelor trimise prin pointerul new_data
ll_node_t* ll_create_new_node(const void* new_data, unsigned int data_size)
{
    ll_node_t* new_node;

    new_node = malloc(sizeof(ll_node_t));
    DIE(!new_node, "malloc new_node");

    new_node->data = malloc(data_size);
    DIE(!new_node->data, "malloc new_node->data");

    memcpy(new_node->data, new_data, data_size);

    return new_node;
}

//  functia adauga un nod pe pozitia n
void ll_add_nth_node(linked_list_t* list, unsigned int n, const void* new_data)
{
    //  lista nu exista
    if (!list) {
        return;
    }

    ll_node_t* new_node;
    ll_node_t* current;

    //  creez noul nod
    new_node = ll_create_new_node(new_data, list->data_size);

    //  adaug nod pe prima pozitie
    if (n == 0 || list->size == 0) {
        new_node->next = list->head;
        list->head = new_node;

        //  actualizez dimensiunea listei
        list->size++;

        return;
    }

    //  daca n >= nr_noduri, noul nod se adauga la finalul listei
    if (n > list->size) {
        n = list->size;
    }

    //  adaug nod pe pozitia n
    current = list->head;

    n--;
    while (n != 0) {
        current = current->next;
        n--;
    }

    new_node->next = current->next;
    current->next = new_node;

    //  actualizez dimensiunea listei
    list->size++;
}

//  functia elimina nodul de pe pozitia n din lista
//  functia retunreza un pointer la nodul eliminat anterior
ll_node_t* ll_remove_nth_node(linked_list_t* list, unsigned int n)
{
    struct ll_node_t* rem_node;

    //  lista nu exista sau este goala
    if (!list || !list->head) {
        return NULL;
    }

    // elimin nodul de pe prima pozitie
    if (list->size == 1 || n == 0) {
        rem_node = list->head;
        list->head = rem_node->next;

        //  actualizez dimensiunea listei
        list->size--;

        return rem_node;
    }

    //  daca n >= nr_noduri - 1, se elimina nodul de la finalul listei
    if (n > list->size - 1) {
        n = list->size - 1;
    }

    //  elimin nodul de pe pozitia n
    ll_node_t* last;
    ll_node_t* prev;

    prev = list->head;
    last = list->head->next;

    n--;
    while (n != 0) {
        prev = last;
        last = last->next;

        n--;
    }
    rem_node = last;
    prev->next = last->next;

    //  actualizez dimensiunea listei
    list->size--;

    return rem_node;
}

//  functia elibereaza memoria alocata listei inlantuite
void ll_free(linked_list_t** pp_list)
{
    linked_list_t* list;
    ll_node_t* current;

    list = *pp_list;

    //  lista nu exista
    if (!list) {
        return;
    }

    //  parcurg lista
    while (list->head) {
        current = list->head;
        list->head = list->head->next;

        //  eliberez memoria alocata info stocata in nod
        free(current->data);
        //  eliberez memoria alocata nodului
        free(current);
    }
    free(list);
    list = NULL;
}

