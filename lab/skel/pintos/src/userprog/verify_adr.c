#include "userprog/verify_adr.h"
#include "userprog/pagedir.h"
#include "threads/vaddr.h"
#include "lib/stddef.h"
#include "threads/thread.h"

/* Kontrollera alla adresser från och med start till och inte med
 * (start+length). */
bool verify_fix_length(void* start, uint32_t length)
{
  void* adr;
  void* end = start+length;
  uint32_t *pd = thread_current()->pagedir;
  if(start == NULL || end >= PHYS_BASE)
    return false;

  for (adr = pg_round_down(start); adr < end; adr += PGSIZE)
  {
    if(pagedir_get_page(pd, adr) == NULL)
      return false;
  }
  return true;
}

/* Kontrollera alla adresser från och med start till och med den
 * adress som först inehåller ett noll-tecken, `\0'. (C-strängar
 * lagras på detta sätt.) */
bool verify_variable_length(char* start)
{
  if(start == NULL)
    return false;
  char* adr = start;
  void* page_adr = pg_round_down(adr);
  uint32_t *pd = thread_current()->pagedir;

  if (adr >= PHYS_BASE || pagedir_get_page(pd, page_adr) == NULL)
    return false;

  if (*adr == '\0')
    return true;

  page_adr += PGSIZE;

  do {
    ++adr;
    if (adr >= PHYS_BASE)
      return false;
    if (adr == page_adr)
    {
      if (pagedir_get_page(pd, page_adr) == NULL)
      {
        return false;
      }
      page_adr += PGSIZE;
    }
  } while (*adr != '\0');

  return true;
}
