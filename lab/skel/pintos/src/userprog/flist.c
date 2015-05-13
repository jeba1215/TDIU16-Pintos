#include <stddef.h>
#include "flist.h"
#include "threads/thread.h"
#include "userprog/map.h"
#include <string.h>

//---------------------------------

void flist_init(struct map* m){
  map_init(m);
}

key_t flist_add_file(struct file* f, struct thread* t){
  return map_insert(&(t->open_files), f);
}

value_t flist_find_file(int fd, struct thread* t){
  return map_find(&(t->open_files), fd);
}

value_t flist_remove_file(int fd, struct thread* t){
  return map_remove(&(t->open_files), fd);
}

bool always_true(key_t t, value_t v, int i)
{
  return true;
}

void flist_remove_process(struct thread* t)
{
  if (t == NULL) return;

  map_remove_if(&(t->open_files), always_true, 0);
}
