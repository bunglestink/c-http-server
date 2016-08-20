#include <string.h>
#include "dictionary.h"
#include "lib.h"

static DictionaryEntry* get_entry(Dictionary* dict, const char* key);
static DictionaryNode* get_node(Dictionary* dict, const char* key);


Dictionary* Dictionary_new() {
  Dictionary* dict = (Dictionary*) x_malloc(sizeof(Dictionary));
  dict->head = NULL;
  dict->size = 0;
  return dict;
}


void Dictionary_delete(Dictionary* dict) {
  DictionaryNode* node = dict->head;
  DictionaryNode* next;
  while (node != NULL) {
    next = node->next;
    x_free(node);
    node = next;
  }
  x_free(dict);
}


void Dictionary_set(Dictionary* dict, char* key, char* value) {
  DictionaryEntry* entry = get_entry(dict, key);
  if (entry != NULL) {
    entry->value = value;
    return;
  }
  DictionaryNode* node = (DictionaryNode*) x_malloc(sizeof(DictionaryNode));
  node->entry.key = key;
  node->entry.value = value;
  node->prev = NULL;
  node->next = dict->head;
  if (dict->head != NULL) {
    ((DictionaryNode*)dict->head)->prev = node;
  }
  dict->head = node;
  dict->size += 1;
}


char* Dictionary_get(Dictionary* dict, const char* key) {
  DictionaryEntry* entry = get_entry(dict, key);
  if (entry == NULL) {
    return NULL;
  }
  return entry->value;
}


int Dictionary_has(Dictionary* dict, const char* key) {
  DictionaryEntry* entry = get_entry(dict, key);
  return entry == NULL ? 0 : 1;
}


void Dictionary_remove(Dictionary* dict, const char* key) {
  DictionaryNode* node = get_node(dict, key);
  if (node == NULL) {
    return;
  }
  if (node->next) {
    node->next->prev = node->prev;
  }
  if (node->prev) {
    node->prev->next = node->next;
  }
  dict->size -= 1;
  x_free(node);
}


DictionaryNode* get_node(Dictionary* dict, const char* key) {
  DictionaryNode* node = dict->head;
  while (node != NULL) {
    if (strcmp(key, node->entry.key) == 0) {
      return node;
    }
    node = node->next;
  }
  return NULL;
}

DictionaryEntry* get_entry(Dictionary* dict, const char* key) {
  DictionaryNode* node = get_node(dict, key);
  return node == NULL ? NULL : &node->entry;
}


DictionaryEntry* Dictionary_get_entries(Dictionary* dict) {
  DictionaryEntry* entries = (DictionaryEntry*) x_malloc(dict->size * sizeof(DictionaryEntry));
  DictionaryNode* node = dict->head;
  int i = 0;
  while (node != NULL) {
    entries[i] = node->entry;
    node = node->next;
    i += 1;
  }
  return entries;
}
