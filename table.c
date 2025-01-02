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

inline static void hash_table_keys_insert(HashTable *ht, HashKey *key) {
  ht->keys[ht->keys_count++] = key;
}

void hash_table_insert(HashTable *ht, const char *key_str, void *const data) {
  HashKey key = hash_table_key_hash(key_str);
  HashKey key_mod = key % HASH_TABLE_CAP;

  Bucket new_bucket = {.key = key, .data = data};

  Bucket *bucket = &ht->nodes[key_mod];
  if (bucket->data == NULL) {
    *bucket = new_bucket;

    hash_table_keys_insert(ht, &key);
    return;
  }

  while (1) {
    if (bucket->next == NULL) {
      bucket->next = &new_bucket;

      hash_table_keys_insert(ht, &key);
      return;
    }

    bucket = bucket->next;
  }
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
static void hash_table_bucket_dump(Bucket *bucket) {
  printf("Bucket: \n");
  printf("\tkey: %lld\n", bucket->key);
  printf("\tdata: %s\n", (char *)bucket->data);
}

static void hash_table_keys_dump(HashTable *ht) {
  printf("Keys: \n");
  for (size_t i = 0; i < ht->keys_count; i++) {
    printf("\t%lld\n", *ht->keys[i]);
  }
}
#pragma clang diagnostic pop

void **hash_table_get(HashTable *ht, const char *key_str) {
  HashKey key = hash_table_key_hash(key_str);
  HashKey key_mod = key % HASH_TABLE_CAP;

  Bucket *bucket = &ht->nodes[key_mod];
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

int hash_table_keys_contains(HashTable *ht, char *key_str) {
  HashKey key = hash_table_key_hash(key_str);

  for (size_t i = 0; i < ht->keys_count; i++) {
    if (*ht->keys[i] == key) {
      printf("contains key at index %zu", i);
      return 1;
    }
  }

  return 0;
}

inline static void hash_table_keys_delete(HashTable *ht, HashKey *key) {
  size_t i = 0;
  for (i = 0; i < ht->keys_count; i++) {
    if (ht->keys[i] == key) {
      // delete
      ht->keys[i] = 0;
      goto merge;
    }
  }

merge:
  for (size_t j = i; j < ht->keys_count - 1; j++) {
    ht->keys[j] = ht->keys[j + 1];
  }
  ht->keys_count--;
}

void hash_table_delete(HashTable *ht, const char *key_str) {
  HashKey key = hash_table_key_hash(key_str);
  HashKey key_mod = key % HASH_TABLE_CAP;

  Bucket *bucket = &ht->nodes[key_mod];

  if (bucket->key == key) {
    if (bucket->next != NULL) {
      *bucket = *bucket->next;
    } else {
      bucket->key = 0;
      bucket->data = NULL;
    }

    hash_table_keys_delete(ht, &key);
    return;
  }

  while (1) {
    if (bucket->next == NULL) {
      fprintf(stderr, "HT: Value not found for key %s\n", key_str);
      exit(2);
    }

    if (bucket->next->key != key_mod) {
      bucket = bucket->next;
      continue;
    }

    if (bucket->next->next != NULL) {
      *bucket->next = *bucket->next->next;
    } else {
      bucket->next->data = 0;
      bucket->next->key = 0;
    }

    hash_table_keys_delete(ht, &key);
    return;
  }
}

/*int main(void) {*/
/*  HashTable table = hash_table_new();*/
/*  hash_table_insert(&table, "Apple", (void *)"Hi");*/
/*  void **raw_apple_price = hash_table_get(&table, "Apple");*/
/*  char *apple_price = (char *)(*raw_apple_price);*/
/*  printf("Fetched %s\n", apple_price);*/
/*  *raw_apple_price = (void *)"Yo";*/
/**/
/*  raw_apple_price = hash_table_get(&table, "Apple");*/
/*  apple_price = (char *)(*raw_apple_price);*/
/*  printf("New Fetched %s\n", apple_price);*/
/**/
/*  hash_table_keys_dump(&table);*/
/*  hash_table_delete(&table, "Apple");*/
/*  hash_table_keys_dump(&table);*/
/*}*/
