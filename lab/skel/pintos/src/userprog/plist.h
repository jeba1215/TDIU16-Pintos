#ifndef _PLIST_H_
#define _PLIST_H_

#include "userprog/map.h"

/* Place functions to handle a running process here (process list).

   plist.h : Your function declarations and documentation.
   plist.c : Your implementation.

   The following is strongly recommended:

   - A function that given process inforamtion (up to you to create)
   inserts this in a list of running processes and return an integer
   that can be used to find the information later on.

   - A function that given an integer (obtained from above function)
   FIND the process information in the list. Should return some
   failure code if no process matching the integer is in the list.
   Or, optionally, several functions to access any information of a
   particular process that you currently need.

   - A function that given an integer REMOVE the process information
   from the list. Should only remove the information when no process
   or thread need it anymore, but must guarantee it is always
   removed EVENTUALLY.

   - A function that print the entire content of the list in a nice,
   clean, readable format.

*/


struct process
{
  char name[16];
  int parent;
  int exit_status;
  int parent_dead;
  int is_alive;
  struct semaphore* sema;
};

void plist_init (void);

key_t plist_add_process(struct map*, int, char*, struct semaphore*);
struct process* plist_find_process(struct map*, key_t);
void plist_remove_process(struct map*, key_t, bool);

void plist_for_each(struct map*, void(*exec)(key_t, struct process*, int), int);
void plist_remove_if(struct map*, bool(*exec)(key_t, struct process*, int), int);

void plist_set_exit_status(struct map*, key_t, int);

void plist_print_all(struct map* m);

#endif
