/* Copyright 2021 <> */
#include <stdlib.h>
#include <string.h>

#include "hash_ring.h"
#include "utils.h"

struct hash_ring {
    //  buffer ce stocheaza etichete/replici de servere
    //  in ordinea crescatoare a hash-urilor lor
    int* buff;

    //  dimensiunea maxima a hash ring-ului
    unsigned int capacity;

    //  dimensiunea hash ring-ului
    unsigned int load;
};

unsigned int hash_function_servers(void *a) {
    unsigned int uint_a = *((unsigned int *)a);

    uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
    uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
    uint_a = (uint_a >> 16u) ^ uint_a;
    return uint_a;
}

//  initializare si alocare hash ring
hash_ring* init_hash_ring()
{
    hash_ring* hr;

    hr = calloc(1, sizeof(hash_ring));
    DIE(!hr, "malloc hr");

    //  presupunem o dimensiune maxima
    hr->capacity = 100;

    hr->load = 0;

    //  initializare si alocare buffer
    hr->buff = calloc(hr->capacity, sizeof(int));

    return hr;
}

unsigned int search_hash_ring(hash_ring* hr, int left, int right,
        unsigned int hash)
{
    unsigned int mid;

    //  determin index-ul aflat la mijloc
    mid = (left + right) / 2;

    //  simulez circularitatea
    //  hash-ul dat este mai mare decat hash-ul ultimei replici din hash ring
    if (hash > hash_function_servers(&hr->buff[right])) {
        return 0;
    }

    //  hash-ul dat este mai mic decat hash-ul primei replici din hash ring
    if (hash <= hash_function_servers(&hr->buff[0])) {
        return 0;
    }

    //  am gasit primul hash mai mare decat hash-ul dat
    if (hash <= hash_function_servers(&hr->buff[mid]) &&
                hash > hash_function_servers(&hr->buff[mid-1])) {
        return mid;
    }

    //  caut in jumatatea superioara
    if (hash > hash_function_servers(&hr->buff[mid])) {
        return search_hash_ring(hr, mid + 1, right, hash);
    }

    //  caut in jumatatea inferioara
    return search_hash_ring(hr, left, mid - 1, hash);
}

unsigned int insert_hash_ring(hash_ring* hr, unsigned int hash, int data)
{
    int i;

    //  verific daca numarul de replici este egal cu capacitatea hashring-ului
    //  resize buffer
    if (hr->load == hr->capacity) {
        int* new;
        unsigned int new_size;

        hr->capacity = 2 * hr->capacity;
        new_size = hr->capacity * sizeof(int);

        new = realloc(hr->buff, new_size);
        DIE(!new, "realloc new");

        hr->buff = new;
    }

    //  adaug prima replica in hash ring
    if (hr->load == 0) {
        hr->buff[0] = data;
        hr->load++;

        return 0;
    }

    //  shiftez elementele cu o pozitie cand gasesc replica cu hash mai mare
    int n;
    n = hr->load - 1;
    for (i = n; (i >= 0 && hash_function_servers(&hr->buff[i]) > hash); --i) {
        hr->buff[i + 1] = hr->buff[i];
    }

    //  adaug replica in hashring
    hr->buff[i + 1] = data;

    //  actualizez numarul de replici
    hr->load++;

    //  returnez pozitia pe care am inserat replica hash ring
    return i + 1;
}

unsigned int delete_hash_ring(hash_ring* hr, unsigned int hash)
{
    unsigned int index, i;

    //  determin pozitia pe care se afla replica cu hash-ul dat
    index = search_hash_ring(hr, 0, hr->load - 1, hash);

    //  shiftez elementele la stanga incepand cu cel de pe pozitia index + 1
    for (i = index; i < hr->load - 1; ++i) {
        hr->buff[i] = hr->buff[i + 1];
    }

    //  actualizez numarul de replici din hash ring
    hr->load--;

    //  returnez pozitia pe care se aflat replica stearsa
    return index;
}

void free_hash_ring(hash_ring* hr)
{
    //  hash ring-ul nu exista
    if (!hr) {
        return;
    }

    //  eliberez buffer-ul care contine replicile
    free(hr->buff);

    //  eliberez hash ring
    free(hr);
}

unsigned int find_server_id(hash_ring* hr, unsigned int position)
{
    //  simulez circularitatea
    if (position ==  hr->load) {
        position = 0;
    }

    //  replica id = 2
    if (hr->buff[position] - 200000 >= 0) {
        return hr->buff[position] - 200000;
    }

    //  replica id = 1
    if (hr->buff[position] - 100000 >= 0) {
        return hr->buff[position] - 100000;
    }

    //  replica id = 0
    return hr->buff[position];
}
