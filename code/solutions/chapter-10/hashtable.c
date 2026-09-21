// hashtable.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Entry { char *key; int value; struct Entry *next; } Entry;
typedef struct { Entry **buckets; size_t nbuckets; } HashTable;

static unsigned long hash_str(const char *s) {                 // djb2
    unsigned long h = 5381;
    for (; *s; s++) h = h * 33 + (unsigned char)*s;
    return h;
}

static HashTable *ht_new(size_t n) {
    HashTable *t = malloc(sizeof *t);
    if (!t) return NULL;
    t->buckets = calloc(n, sizeof *t->buckets);
    if (!t->buckets) { free(t); return NULL; }
    t->nbuckets = n;
    return t;
}

static int ht_put(HashTable *t, const char *key, int value) {
    size_t i = hash_str(key) % t->nbuckets;
    for (Entry *e = t->buckets[i]; e; e = e->next)
        if (strcmp(e->key, key) == 0) { e->value = value; return 0; }   // cập nhật nếu đã có
    Entry *e = malloc(sizeof *e);
    if (!e) return -1;
    size_t len = strlen(key) + 1;
    e->key = malloc(len);
    if (!e->key) { free(e); return -1; }
    memcpy(e->key, key, len);
    e->value = value;
    e->next = t->buckets[i];
    t->buckets[i] = e;
    return 0;
}

static int ht_get(const HashTable *t, const char *key, int *out) {
    for (Entry *e = t->buckets[hash_str(key) % t->nbuckets]; e; e = e->next)
        if (strcmp(e->key, key) == 0) { *out = e->value; return 0; }
    return -1;
}

static int ht_remove(HashTable *t, const char *key) {
    for (Entry **pp = &t->buckets[hash_str(key) % t->nbuckets]; *pp; pp = &(*pp)->next) {
        if (strcmp((*pp)->key, key) == 0) {
            Entry *dead = *pp;
            *pp = dead->next;
            free(dead->key);
            free(dead);
            return 0;
        }
    }
    return -1;
}

static void ht_free(HashTable *t) {
    if (!t) return;
    for (size_t i = 0; i < t->nbuckets; i++)
        for (Entry *e = t->buckets[i], *n; e; e = n) { n = e->next; free(e->key); free(e); }
    free(t->buckets);
    free(t);
}

int main(void) {
    HashTable *t = ht_new(16);
    if (!t) return 1;
    ht_put(t, "an", 10); ht_put(t, "binh", 20); ht_put(t, "an", 11);   // cập nhật "an"
    int v;
    if (ht_get(t, "an", &v) == 0) printf("an = %d\n", v);              // 11
    ht_remove(t, "binh");
    printf("binh %s\n", ht_get(t, "binh", &v) == 0 ? "con" : "da xoa");
    ht_free(t);
    return 0;
}
