#ifndef Table_H
#define Table_H

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
  Bucket nodes[HASH_TABLE_CAP];
} HashTable;

HashTable hash_table_new(void);
void hash_table_insert(HashTable *ht, const char *key_str, void *const data);
void *hash_table_get(HashTable *ht, const char *key_str);

#endif
