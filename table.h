#ifndef Table_H
#define Table_H

#include <stddef.h>
#include <stdint.h>

#define HASH_TABLE_CAP 255

typedef struct Bucket Bucket;
typedef uint64_t HashKey;

typedef struct {
  char *str;
  int len;
} Sv;

struct Bucket {
  Bucket *prev;
  HashKey key;
  void *data;
};

typedef struct {
  Bucket *nodes[HASH_TABLE_CAP];

  HashKey keys[HASH_TABLE_CAP];
  uint8_t keys_count;
} Hash_Table;

Hash_Table hash_table_new(void);
void hash_table_destruct(Hash_Table *ht);

void hash_table_insert(Hash_Table *ht, const Sv key_str, void *const data);
void **hash_table_get(Hash_Table *ht, const Sv key_str);
void hash_table_delete(Hash_Table *ht, const Sv key_str);

int hash_table_keys_contains(Hash_Table *ht, Sv key_str);

#endif
