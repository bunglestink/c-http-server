#ifndef _DICTIONARY_H_
#define _DICTIONARY_H_


typedef struct DictionaryEntryStruct {
  char* key;
  void* value;
} DictionaryEntry;


/** @internal */
typedef struct DictionaryNodeStruct {
  DictionaryEntry entry;
  struct DictionaryNodeStruct* prev;
  struct DictionaryNodeStruct* next;
} DictionaryNode;


// TODO: Replace with hashmap if performance ever matters.
typedef struct DictionaryStruct {
  DictionaryNode* head;
  int size;
} Dictionary;


Dictionary* Dictionary_new();
void Dictionary_delete(Dictionary* dict);
void Dictionary_set(Dictionary* dict, char* key, char* value);
char* Dictionary_get(Dictionary* dict, const char* key);
DictionaryEntry* Dictionary_get_entries(Dictionary* dict);
int Dictionary_has(Dictionary* dict, const char* key);
void Dictionary_remove(Dictionary* dict, const char* key);


#endif

