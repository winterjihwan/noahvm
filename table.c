#include "table.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

inline HashTable hash_table_new(void) {
  HashTable ht = {0};

  return ht;
}

inline static HashKey hash_table_key_hash(const char *raw_key) {
  HashKey key = 5381;

  for (size_t i = 0; raw_key[i] != '\0'; i++) {
    key = ((key << 5) + key) + raw_key[i];
  }

  return key;
}

void hash_table_insert(HashTable *ht, const char *key_str, void *const data) {
  HashKey key = hash_table_key_hash(key_str);
  key %= HASH_TABLE_CAP;

  Bucket new_bucket = {.key = key, .data = data};

  Bucket *bucket = &ht->nodes[key];
  if (bucket->data == NULL) {
    *bucket = new_bucket;
    return;
  }

  while (1) {
    if (bucket->next == NULL) {
      bucket->next = &new_bucket;
      return;
    }

    bucket = bucket->next;
  }
}

static void bucket_dump(Bucket *bucket) {
  printf("Bucket: \n");
  printf("\tkey: %lld\n", bucket->key);
  printf("\tdata: %s\n", (char *)bucket->data);
}

void **hash_table_get(HashTable *ht, const char *key_str) {
  HashKey key = hash_table_key_hash(key_str);
  key %= HASH_TABLE_CAP;

  Bucket *bucket = &ht->nodes[key];
  if (bucket->data != NULL) {
    return &bucket->data;
  }

  while (1) {
    if (bucket->next == NULL) {
      fprintf(stderr, "HT: Value not found for key %s\n", key_str);
      exit(1);
    }

    if (bucket->next->key != key) {
      bucket = bucket->next;
      continue;
    }

    return &bucket->next->data;
  }
}

void hash_table_delete(HashTable *ht, const char *key_str) {
  HashKey key = hash_table_key_hash(key_str);
  key %= HASH_TABLE_CAP;

  Bucket *bucket = &ht->nodes[key];

  if (bucket->key == key) {
    if (bucket->next != NULL) {
      *bucket = *bucket->next;
    } else {
      bucket->key = 0;
      bucket->data = NULL;
    }

    return;
  }

  while (1) {
    if (bucket->next == NULL) {
      fprintf(stderr, "HT: Value not found for key %s\n", key_str);
      exit(2);
    }

    if (bucket->next->key != key) {
      bucket = bucket->next;
      continue;
    }

    if (bucket->next->next != NULL) {
      *bucket->next = *bucket->next->next;
    } else {
      bucket->next->data = 0;
      bucket->next->key = 0;
    }
    return;
  }
}
