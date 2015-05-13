#include <stdio.h>
#include <string.h>
#include "map.h"

bool valid_key(int k){
  if(k > 1 && k < MAP_SIZE)
    return true;
  else
    return false;
}

void map_init(struct map* m){
  int i;
  for(i=2; i<MAP_SIZE; ++i){
    m->content[i] = NULL;
  }
}

key_t map_insert(struct map* m, value_t v){
  int i;
  for(i=2; i<MAP_SIZE; ++i){
    if(!m->content[i]){
      m->content[i] = v;
      return i;
    }
  }
  return -1;
}

value_t map_find(struct map* m, key_t k){
  if(valid_key(k) && m->content[k]){
    return m->content[k];
  }
  else{
    debug("map_find returns null\n");
    return NULL;
  }
}

value_t map_remove(struct map* m, key_t k){
  if(valid_key(k) && m->content[k]){
    value_t ret = m->content[k];
    m->content[k] = NULL;
    return ret;
  }else
    return NULL;
}


void map_for_each(struct map* m,  void (*exec)(key_t k, value_t v, int aux), int aux){
  int i;
  for(i=0; i<MAP_SIZE; ++i){
    exec(i, m->content[i], aux);
  }
}

void map_remove_if(struct map* m,  bool (*cond)(key_t k, value_t v, int aux), int aux){
  int i;
  for(i=0; i<MAP_SIZE; ++i){
    if( cond(i, m->content[i], aux) ){
      map_remove(m, i);     
    }
  }
}
