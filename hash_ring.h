/* Copyright 2021 <> */
#ifndef HASH_RING_H_
#define HASH_RING_H_

struct hash_ring;
typedef struct hash_ring hash_ring;

unsigned int hash_function_servers(void *a);

hash_ring* init_hash_ring();

void free_hash_ring(hash_ring* hr);

/**
 * search_hash_ring() - Gets the position of given hash on the hash ring.
 * @arg1: Hash ring which distributes the replica TAGs.
 * @arg2: Lower half of the hash ring.
 * @arg3: Upper half of the hash ring.
 * @arg4: Hash represented as an unsigned integer.
 */
unsigned int search_hash_ring(hash_ring* hr, int left, int right,
        unsigned int hash);

/**
 * insert_hash_ring() - Inserts replica TAG and gets its position on the hash ring.
 * @arg1: Hash ring which distributes the replica TAGs.
 * @arg2: Hash of the inserted replica TAG.
 * @arg3: Inserted replica TAG.
 */
unsigned int insert_hash_ring(hash_ring* hr, unsigned int hash, int data);

/**
 * delete_hash_ring() - Deletes replica TAG from the hash ring and gets its position.
 * @arg1: Hash ring which distributes the replica TAGs.
 * @arg2: Hash of the deleted replica TAG.
 */
unsigned int delete_hash_ring(hash_ring* hr, unsigned int hash);

/**
 * find_server_id() - Gets the server id by its replica TAG's position on the hash ring.
 * @arg1: Hash ring which distributes the replica TAGs.
 * @arg2: Position of replica TAG.
 */
unsigned int find_server_id(hash_ring* hr, unsigned int position);

#endif  /* HASH_RING_H_ */
