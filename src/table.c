#include "table.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

inline Hash_Table hash_table_new(void) {
  Hash_Table ht = {0};

  return ht;
}

inline static HashKey hash_table_key_hash(const Sv raw_key) {
  HashKey key = 5381;

  for (int i = 0; i < raw_key.len; i++) {
    key = ((key << 5) + key) + raw_key.str[i];
  }

  return key;
}

inline static void hash_table_keys_insert(Hash_Table *ht, HashKey key) {
  ht->keys[ht->keys_count++] = key;
}

void hash_table_insert(Hash_Table *ht, const Sv key_str, const Word data) {
  HashKey key = hash_table_key_hash(key_str);
  HashKey key_mod = key % HASH_TABLE_CAP;

  Bucket *new_bucket = (Bucket *)malloc(sizeof(Bucket));
  new_bucket->key = key;
  new_bucket->data = data;

  Bucket *bucket = ht->nodes[key_mod];

  ht->nodes[key_mod] = new_bucket;
  if (bucket != NULL)
    new_bucket->prev = bucket;

  hash_table_keys_insert(ht, key);
  return;
}

Word *hash_table_get(Hash_Table *ht, const Sv key_str) {
  HashKey key = hash_table_key_hash(key_str);
  HashKey key_mod = key % HASH_TABLE_CAP;

  Bucket *bucket = ht->nodes[key_mod];
  if (bucket == NULL)
    goto not_found;

  if (bucket->key == key) {
    return &bucket->data;
  }

  while (1) {
    if (bucket->prev == NULL)
      goto not_found;

    if (bucket->prev->key != key) {
      bucket = bucket->prev;
      continue;
    }

    return &bucket->prev->data;
  }

not_found:
  fprintf(stderr, "HT: Value not found for key %.*s\n", key_str.len,
          key_str.str);
  exit(1);
}

int hash_table_keys_contains(Hash_Table *ht, Sv key_str) {
  HashKey key = hash_table_key_hash(key_str);

  for (size_t i = 0; i < ht->keys_count; i++)
    if (ht->keys[i] == key)
      return 1;

  return 0;
}

inline static void hash_table_keys_delete(Hash_Table *ht, HashKey key) {
  size_t i = 0;
  for (i = 0; i < ht->keys_count; i++) {
    if (ht->keys[i] == key) {
      ht->keys[i] = 0;
      goto merge;
    }
  }

  return;

merge:
  for (size_t j = i; j < ht->keys_count - 1; j++) {
    ht->keys[j] = ht->keys[j + 1];
  }
  ht->keys_count--;
}

void hash_table_delete(Hash_Table *ht, const Sv key_str) {
  HashKey key = hash_table_key_hash(key_str);
  HashKey key_mod = key % HASH_TABLE_CAP;

  Bucket *bucket = ht->nodes[key_mod];

  if (bucket == NULL)
    goto not_found;

  if (bucket->key == key) {
    if (bucket->prev != NULL) {
      Bucket *t = bucket;
      *bucket = *bucket->prev;
      free(t);
    } else {
      free(bucket);
    }

    hash_table_keys_delete(ht, key);
    return;
  }

  while (1) {
    if (bucket->prev == NULL)
      goto not_found;

    if (bucket->prev->key != key) {
      bucket = bucket->prev;
      continue;
    }

    if (bucket->prev->prev != NULL) {
      Bucket *t = bucket->prev;
      bucket->prev = bucket->prev->prev;
      free(t);
    } else {
      free(bucket->prev);
    }

    hash_table_keys_delete(ht, key);
    return;
  }

not_found:
  fprintf(stderr, "HT: Value not found for key %.*s\n", key_str.len,
          key_str.str);
  exit(2);
}

void hash_table_destruct(Hash_Table *ht) {
  for (size_t i = 0; i < HASH_TABLE_CAP; i++) {
    Bucket *bucket = ht->nodes[i];

    while (bucket != NULL) {
      Bucket *prev = bucket->prev;
      free(bucket);
      bucket = prev;
    }
  }
}
