#include <stddef.h>

#include "plist.h"
#include "threads/synch.h"
#include "threads/malloc.h"
#include "lib/string.h"
#include "lib/stdio.h"

struct lock plist_lock;

void
plist_init (void)
{
  lock_init(&plist_lock);
}

void plist_print_process(key_t, struct process*, int);
bool flag_child(key_t, struct process*, int);

key_t plist_add_process(struct map* m, int parent_id, char* name, struct semaphore* sema)
{
  struct process* p = malloc(sizeof(struct process));
  if(p == NULL)
    return -1;
  strlcpy(p->name, name, 16);
  p->parent = parent_id;
  p->sema = sema;
  p->exit_status = 0;
  p->parent_dead = 0;
  p->is_alive = 1;
  lock_acquire(&plist_lock);
  key_t k = map_insert(m, p);
  lock_release(&plist_lock);
  return k;

}

struct process* plist_find_process(struct map* m, key_t k)
{
  lock_acquire(&plist_lock);
  struct process* p = map_find(m, k);
  lock_release(&plist_lock);
  return p;
}

void plist_remove_process(struct map* m , key_t k, bool force_remove)
{
  lock_acquire(&plist_lock);
  struct process* p = map_find(m, k);
  if(p != NULL)
  {
    p->is_alive = 0;
    p->parent_dead = force_remove;
    map_remove_if(m, (void*) &flag_child, k);
  }
  lock_release(&plist_lock);
}

void plist_print_all(struct map* m)
{
  printf("# %-3s %-16s %-11s %-6s %-6s %-13s\n", "PID", "NAME", "EXIT_STATUS", "ALIVE?", "PARENT", "PARENT_ALIVE?");
  lock_acquire(&plist_lock);
  map_for_each(m, (void*) &plist_print_process, 0);
  lock_release(&plist_lock);
}

void plist_set_exit_status(struct map* m, key_t k, int status)
{
  lock_acquire(&plist_lock);
  struct process* p = map_find(m, k);
  p->exit_status = status;
  lock_release(&plist_lock);
}

void plist_print_process(key_t k, struct process* p, int aux)
{
  if(p != NULL)
  {
    printf("# %-3i %-16s %-11i %-6s %-6i %-13s\n",
        k, p->name, p->exit_status, p->is_alive ? "true":"false", p->parent,
        p->parent_dead ? "false":"true");
  }
}

bool flag_child(key_t k, struct process* p, int parent)
{
  if(p != NULL)
  {
    if(p->parent == parent)
    {
      p->parent_dead = 1;
    }
    if(p->parent_dead && !p->is_alive)
    {
      free(p->sema);
      free(p);
      return 1;
    }
  }
  return 0;
}

