/* Copyright 2021 <> */
#include <stdlib.h>
#include <string.h>

#include "load_balancer.h"
#include "utils.h"

struct load_balancer {
    //  hash ring-ul folosit pt distribuirea serverelor si a cheilor
	hash_ring* ring;

    //  memoriile serverelor (hashtables)
    server_memory** servers;

    //  numarul maxim de servere
    unsigned int max_size;

    //  numarul de replici, adica 3 * numarul de servere
    unsigned int r_num;
};

load_balancer* init_load_balancer() {
	load_balancer* ld;

    // initializare load balancer
    ld = calloc(1, sizeof(load_balancer));
    DIE(!ld, "malloc ld");

    //  initializare numar maxim de servere
    ld->max_size = 99999;

    //  initializare numar de replici existente
    ld->r_num = 0;

    //  initializare hash ring
    ld->ring =  init_hash_ring();

    //  initializare array ce contine memoriile de servere
    ld->servers = calloc(ld->max_size, sizeof(server_memory*));
    DIE(!ld->servers, "malloc ld->servers");

    for (unsigned int i = 0; i < ld->max_size; ++i) {
	    ld->servers[i] = NULL;
	}

    return ld;
}


void loader_store(load_balancer* main, char* key, char* value, int* server_id) {
	server_memory* memory;
    unsigned int hash_index;
    int id, position;

    //  daca load balancer-ul nu exista sau nu exista servere
    if (!main || main->r_num == 0) {
        return;
    }

    //  determin hash-ul cheii pe care vreau sa o adaug
    hash_index = hash_function_key(key);

    //  determin pozitia de pe hash ring a replicii catre care distribui cheia
    //  adica gasesc prima replica cu hash-ul mai mare decat hash-ul cheii
    position = search_hash_ring(main->ring, 0, main->r_num - 1, hash_index);

    //  determin server ul pe care voi stoca perechea cheie-valoare
    id = find_server_id(main->ring, position);
    *server_id = id;

    //  determin memoria serverului
    memory = main->servers[id];

    //  stochez perechea cheie-valoare
    server_store(memory, key, value);
}

char* loader_retrieve(load_balancer* main, char* key, int* server_id) {
	char* value;
    unsigned int hash_index;
    int id, position;
    server_memory* memory;

    //  daca load balancer-ul nu exista sau nu exista servere
	if (!main || main->r_num == 0) {
        return NULL;
    }

    //  determin hash-ul cheii date
    hash_index = hash_function_key(key);

    //  determin pozitia replicii catre care a fost distribuita cheia
    position = search_hash_ring(main->ring, 0, main->r_num - 1, hash_index);

    //  determin server-ul pe care este stocata cheia
    id = find_server_id(main->ring, position);
    *server_id = id;

    //  determin memoria server-ului
    memory = main->servers[id];

    //  determin valoarea asociata cheii
    value = server_retrieve(memory, key);

    //  returnez valoare
    return value;
}

//  functie de remapare a elementelor din memory1 in memory2
//  old_id este id-ul server-ului asociat memory1
void rehash_server(load_balancer* main, server_memory* memory1,
        server_memory* memory2, unsigned int old_id)
{
    struct info** server_info;
    unsigned int rehash;
    unsigned int id, position;
    char key;

    //  determin toate elementele unui server (perechi cheie-valoare)
    //  pentru a afla ce elemente pot fi remapate
    server_info = server_retrieve_all(memory1);

    //  pentru fiecare perche din memoria server-ului
    for (unsigned int i = 0; i < memory1->size; ++i) {
        //  determin hash-ul cheii
        rehash = hash_function_key(server_info[i]->key);

        //  determin pozitia replicii catre care pot distribui cheia
        position = search_hash_ring(main->ring, 0, main->r_num - 1, rehash);

        //  determin id-ul server-ului pe care pot stoca perechea cheie-valoare
        id = find_server_id(main->ring, position);

        //  daca id-urile difera
        //  inseamna ca putem remapa cheia pe server-ul vecin
        if (id != old_id) {
            //  stochez perechea cheie-valoare in memoria server-ul vecin
            server_store(memory2, server_info[i]->key, server_info[i]->value);
            key = *server_info[i]->key;

            //  sterg cheia din memoria server-ului vechi
            server_remove(memory1, &key);
        }
    }
    //  eliberez memoria alocata pt array-ul de perechii cheie valoare
    free(server_info);
}

void loader_add_server(load_balancer* main, int server_id)
{
	unsigned int hash_index;
    unsigned int position;
    unsigned int stamp = 0;
    int succ_server_id;

    server_memory* memory;
    server_memory* succ_memory;

    //  initializez memoria server-ului
    memory = init_server_memory();

    //  o introduc in array-ul de memorii de server pe pozitia id-ului dat
    main->servers[server_id] = memory;

    for (int i = 0; i < 3; ++i) {
        //  determin eticheta replicii
        stamp = i*100000 + server_id;

        //  determin hash ul replicii
        hash_index = hash_function_servers(&stamp);

        //  determin pozitia pe care am inserat o replica in hash ring
        position = insert_hash_ring(main->ring, hash_index, stamp);

        //  actualizez numarul de replici din hash ring
        main->r_num = main->r_num + 1;

        //  verifica daca exista mai mult de un server
        if (main->r_num > 3) {
            //  determin id ul serverului succesor
            succ_server_id = find_server_id(main->ring, position + 1);

            //  verific daca am doua servere cu acelasi id unul dupa altul
            if (server_id != succ_server_id) {
                //  determin memoria serverului succesor
                succ_memory = main->servers[succ_server_id];

                //  verific daca serverul succesor are elemente stocate
                if (succ_memory->size >= 1) {
                    //  determin ce elemente pot fi remapate pe serverul nou
                    rehash_server(main, succ_memory, memory, succ_server_id);
                }
            }
        }
    }
}

void loader_remove_server(load_balancer* main, int server_id) {
	unsigned int stamp = 0;
    unsigned int hash_index, position;
    int succ_server_id;

    server_memory* memory;
    server_memory* succ_memory;

    //  load balancer ul nu exista sau nu are servere stocate
    if (!main || main->r_num == 0) {
        return;
    }

    //  determin memoria server ului dat
    memory = main->servers[server_id];

    for (unsigned int i = 0; i < 3; ++i) {
        //  determin eticheta replicii
        stamp = i*100000 + server_id;

        //  determin hash ul replicii
        hash_index = hash_function_servers(&stamp);

        //  sterg replica si aflu pozitia pe care se afla in hash ring
        position = delete_hash_ring(main->ring, hash_index);

        //  actualizez numarul de replici din hash ring
        main->r_num = main->r_num - 1;

        //  determin id-ul server-ului succesor
        //  se afla pe hash ring unde se afla replica stearsa anterior
        succ_server_id = find_server_id(main->ring, position);

        //  determin memoria serverului succesor
        succ_memory = main->servers[succ_server_id];

        //  daca replica stearsa se refera la un server cu elemente
        if (memory->size >= 1 && main->r_num > 3) {
            //  remapez elementele
            rehash_server(main, memory, succ_memory, server_id);
        }
    }

    //  eliberez memoria serverului dat
    free_server_memory(memory);
    main->servers[server_id] = NULL;
}

void free_load_balancer(load_balancer* main) {
    //  eliberez memoria alocata pentru hash ring
    free_hash_ring(main->ring);

    // eliberez memoria fiecarui server adaugat
    for (unsigned int i = 0; i < main->max_size; ++i) {
        if (main->servers[i] != NULL) {
            free_server_memory(main->servers[i]);
        }
    }
    //  eliberez memoria alocata pt array-ul de servere
    free(main->servers);

    //  eliberez memoria alocata pentru load balancer
    free(main);
}
