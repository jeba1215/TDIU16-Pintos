#ifndef SIMPLE_MAP_STRUCT
#define SIMPLE_MAP_STRUCT

//#define PANIC() exit(1)
#include <stdbool.h>

typedef void* value_t;
typedef int key_t;

//typedef enum {false, true} bool;

/* symbolisk konstant för att lätt kunna ändra storlekeni 
fortsättningen används denna då storleken behövs */
#define MAP_SIZE 500

struct map{
  value_t content[MAP_SIZE];
};

void map_init(struct map* m);
key_t map_insert(struct map* m, value_t v);
value_t map_find(struct map* m, key_t k);
value_t map_remove(struct map* m, key_t k);
void map_for_each(struct map* m,  void (*exec)(key_t k, value_t v, int aux), int aux);
void map_remove_if(struct map* m,  bool (*cond)(key_t k, value_t v, int aux), int aux);

#endif
