#ifndef Table_H
#define Table_H

#include <stddef.h>
#include <stdint.h>

#define HASH_TABLE_CAP 256

typedef struct Bucket Bucket;
typedef uint64_t HashKey;

struct Bucket {
  Bucket *next;
  HashKey key;
  void *data;
};

typedef struct {
  Bucket *nodes[HASH_TABLE_CAP];

  HashKey *keys[HASH_TABLE_CAP];
  uint8_t keys_count;
} HashTable;

HashTable hash_table_new(void);
void hash_table_insert(HashTable *ht, const char *key_str, void *const data);
void **hash_table_get(HashTable *ht, const char *key_str);
void hash_table_delete(HashTable *ht, const char *key_str);
int hash_table_keys_contains(HashTable *ht, char *key_str);

#endif
