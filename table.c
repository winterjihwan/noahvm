#include "table.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

inline Hash_Table hash_table_new(void) {
  Hash_Table ht = {0};

  return ht;
}

inline static HashKey hash_table_key_hash(const char *raw_key) {
  HashKey key = 5381;

  for (size_t i = 0; raw_key[i] != '\0'; i++) {
    key = ((key << 5) + key) + raw_key[i];
  }

  return key;
}

inline static void hash_table_keys_insert(Hash_Table *ht, HashKey *key) {
  ht->keys[ht->keys_count++] = key;
}

void hash_table_insert(Hash_Table *ht, const char *key_str, void *const data) {
  HashKey key = hash_table_key_hash(key_str);
  HashKey key_mod = key % HASH_TABLE_CAP;

  Bucket *new_bucket = (Bucket *)malloc(sizeof(Bucket));
  new_bucket->key = key;
  new_bucket->data = data;

  Bucket *bucket = ht->nodes[key_mod];
  if (bucket == NULL) {
    ht->nodes[key_mod] = new_bucket;

    hash_table_keys_insert(ht, &key);
    return;
  }

  while (1) {
    if (bucket->next == NULL) {
      bucket->next = new_bucket;

      hash_table_keys_insert(ht, &key);
      return;
    }

    bucket = bucket->next;
  }
}

void **hash_table_get(Hash_Table *ht, const char *key_str) {
  HashKey key = hash_table_key_hash(key_str);
  HashKey key_mod = key % HASH_TABLE_CAP;

  Bucket *bucket = ht->nodes[key_mod];
  if (bucket == NULL) {
    fprintf(stderr, "HT: Value not found for key %s\n", key_str);
    exit(1);
  }

  if (bucket->key == key) {
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

int hash_table_keys_contains(Hash_Table *ht, char *key_str) {
  HashKey key = hash_table_key_hash(key_str);

  for (size_t i = 0; i < ht->keys_count; i++) {
    if (*ht->keys[i] == key) {
      return 1;
    }
  }

  return 0;
}

inline static void hash_table_keys_delete(Hash_Table *ht, HashKey *key) {
  size_t i = 0;
  for (i = 0; i < ht->keys_count; i++) {
    if (ht->keys[i] == key) {
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

void hash_table_delete(Hash_Table *ht, const char *key_str) {
  HashKey key = hash_table_key_hash(key_str);
  HashKey key_mod = key % HASH_TABLE_CAP;

  Bucket *bucket = ht->nodes[key_mod];

  if (bucket == NULL) {
    fprintf(stderr, "HT: Value not found for key %s\n", key_str);
    exit(2);
  }

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
      Bucket *tmp = bucket->next;
      bucket->next = bucket->next->next;
      free(tmp);
    } else {
      free(bucket->next);
    }

    hash_table_keys_delete(ht, &key);
    return;
  }
}

void hash_table_destruct(Hash_Table *ht) {
  for (size_t i = 0; i < HASH_TABLE_CAP; i++) {
    Bucket *bucket = ht->nodes[i];

    while (bucket != NULL) {
      Bucket *next = bucket->next;
      free(bucket);
      bucket = next;
    }
  }
}
