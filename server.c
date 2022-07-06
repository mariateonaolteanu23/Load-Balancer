/* Copyright 2021 <> */
#include <stdlib.h>
#include <string.h>

#include "server.h"
#include "utils.h"

//  Functie de generare hash pt chei
unsigned int hash_function_key(void *a) {
    unsigned char *puchar_a = (unsigned char *) a;
    unsigned int hash = 5381;
    int c;

    while ((c = *puchar_a++))
        hash = ((hash << 5u) + hash) + c;

    return hash;
}

//  aloc si initializez memeoria server-ului
//  server_memory este implementata ca un hasserverable
server_memory* init_server_memory() {
	server_memory* server;

	server = malloc(sizeof(server_memory));
	DIE(!server, "malloc server");

	server->size = 0;
	server->hmax = N_BUCKETS;

	//  initializare si alocare array de liste inlantuite
	server->buckets = malloc(server->hmax * sizeof(linked_list_t *));
	DIE(!server->buckets, "server->buckets malloc");

	for (unsigned int i = 0; i < server->hmax; i++) {
		server->buckets[i] = ll_create(sizeof(struct info));
	}

	return server;
}

//  pe baza perechii cheie-valoare date creez structura info
struct info create_new_info(char* key, char* value)
{
	struct info new_info;
	char* actual_key;
    char* actual_value;

	actual_key = malloc(strlen(key) + 1);
    actual_value = malloc(strlen(value) + 1);

    memcpy(actual_key, key, strlen(key) + 1);
    memcpy(actual_value, value, strlen(value) + 1);

	new_info.key = actual_key;
	new_info.value = actual_value;

	return new_info;
}

void server_store(server_memory* server, char* key, char* value) {
	unsigned int index;
	linked_list_t* bucket;
	ll_node_t* current;

	struct info new_info;
	struct info* current_info;

	if (!server) {
		return;
	}

	//  determin index-ul bucket-ului in care voi stoca info
	index = hash_function_key(key) % server->hmax;

	//  determin bucket-ul
	bucket = server->buckets[index];

	//  verific daca exista deja in server cheia pe care o adaug
	if (bucket->size) {
		current = bucket->head;

		//  parcurg lista asociata bucket-ului
		while (current) {
			current_info = current->data;
			if(strcmp(key, current_info->key) == 0) {
				if (strcmp(current_info->value, value) != 0) {
					//  actualizez valoarea cheii deja existente
					memcpy(current_info->value, value, strlen(value) + 1);
				}
				return;
			}
			current = current->next;
		}
	}
	//  initializez structura info
	new_info = create_new_info(key, value);

	//  creez un nou nod in lista bucket-ului pe baza datelor din new_info
	ll_add_nth_node(server->buckets[index], bucket->size, &new_info);

	//  actulizez dimensiunea server-ululi
	server->size++;
}

void server_remove(server_memory* server, char* key) {
	unsigned int index;
	struct info* current_info;
	linked_list_t* bucket;
	ll_node_t* current;
	ll_node_t* delete;

	//  server-ul nu exista
	if(!server) {
		return;
	}

	//  determin index-ul bucket-ului
	index = hash_function_key(key) % server->hmax;

	//  determin bucket-ul
	bucket = server->buckets[index];

	if(!bucket || !bucket->head) {
		//  cheia data nu a fost gasita in server
		return;
	}

	//  parcurg lista asociata bucket-ului
	current = bucket->head;
	for(unsigned int i = 0; i < bucket->size; ++i) {
		current_info = current->data;

		if (strcmp(key, current_info->key) == 0) {
			//  elimin nodul din lista
			delete = ll_remove_nth_node(bucket, i);

			//  eliberez memoria alocata informatiilor din nod
			free(current_info->key);
			free(current_info->value);
			free(delete->data);

			//  eliberez memoria alocata nodului
			free(delete);

			//  actualizez dimensiunea server-ului
			server->size--;

			//  am eliminat nodul si perechea cheie-valoare asociata lui
			break;
		}
		current = current->next;
	}
}

char* server_retrieve(server_memory* server, char* key) {
	unsigned int index;
	struct info* current_info;
	linked_list_t* bucket;
	ll_node_t* current;

	//  server-ul nu exista
	if (!server) {
		return NULL;
	}

	//  determin index-ul bucket-ului
	index = hash_function_key(key) % server->hmax;

	//  determin bucket-ul
	bucket = server->buckets[index];

	//  bucket-ul e gol sau nu exista
	if(!bucket || !bucket->head) {
		//  cheia data nu a fost gasita in server
		return NULL;
	}

	//  parcurg lista asociata bucket-ului
	current = bucket->head;
	while (current) {
		current_info = current->data;

		//  verific daca exista cheia in server
		if(strcmp(key, current_info->key) == 0) {
			//  returnez valoarea asociata cheii
			return current_info->value;
		}
		current = current->next;
	}
	//  cheia data nu a fost gasita in server
	return NULL;
}

void free_server_memory(server_memory* server) {
	ll_node_t* temp;
	ll_node_t* current;
	linked_list_t* bucket;

	//  server-ul nu exista
	if (!server) {
		return;
	}

	//  parcurg fiecare bucket
	for (unsigned int i = 0; i < server->hmax; ++i) {
		bucket = server->buckets[i];

		temp = bucket->head;
		//  parcurg lista asociata bucket-ului
		while(temp != NULL) {
			current = temp;
			temp = temp->next;

			//  eliberez memoria alocata informatiilor din nod
			free(((struct info*)(current->data))->value);
			free(((struct info*)(current->data))->key);
			free(current->data);

			//  eliberez memoria alocata nodului
			free(current);
		}
		//  eliberez memoria alocata bucket-ul
		free(bucket);
	}
	//  eliberez memoria alocata array-ului de bucket-uri
	free(server->buckets);

	//  eliberez memoria server-ului
	free(server);
}

//  functia returneaza toate perechile cheie-valoare din server
struct info** server_retrieve_all(server_memory* server)
{
    ll_node_t* curr;
    struct info** info;
	struct info* current_info;

	unsigned int i;
	int j = 0;

	//  aloc un array de pointeri la structuri info
	info = malloc(sizeof(struct info*) * server->size);

	//  parcurg tot server-ul
    for (i = 0; i < server->hmax; ++i) {
        curr = ((linked_list_t*)server->buckets[i])->head;

        while (curr) {
			current_info = curr->data;

			//  adaug informatia in array
            info[j] = current_info;

			curr = curr->next;
			j++;
        }
    }
	return info;
}
