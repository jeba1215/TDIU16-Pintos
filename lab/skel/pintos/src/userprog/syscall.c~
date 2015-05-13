#include <stdio.h>
#include <syscall-nr.h>
#include "userprog/syscall.h"
#include "threads/interrupt.h"
#include "threads/thread.h"

/* header files you probably need, they are not used yet */
#include <string.h>
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "threads/vaddr.h"
#include "threads/init.h"
#include "userprog/pagedir.h"
#include "userprog/process.h"
#include "devices/input.h"
#include "flist.h"
#include "devices/timer.h"
#include "userprog/verify_adr.h"

#define DBG(format, ...) printf(format "\n", ##__VA_ARGS__)

static void syscall_handler (struct intr_frame *);

void syscall_init (void)
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}


/* This array defined the number of arguments each syscall expects.
   For example, if you want to find out the number of arguments for
   the read system call you shall write:

   int sys_read_arg_count = argc[ SYS_READ ];

   All system calls have a name such as SYS_READ defined as an enum
   type, see `lib/syscall-nr.h'. Use them instead of numbers.
   */
const int argc[] = {
  /* basic calls */
  0, 1, 1, 1, 2, 1, 1, 1, 3, 3, 2, 1, 1,
  /* not implemented */
  2, 1,    1, 1, 2, 1, 1,
  /* extended */
  0
};


  static int32_t
sys_open(char* fname)
{
  struct file* ofile = filesys_open(fname);
  if(ofile != NULL)
  {
    return flist_add_file(ofile, thread_current());
  } else {
    return -1;
  }
}

  static bool
sys_remove(char* filename)
{
  return filesys_remove(filename);
}

  static int32_t
sys_filesize(int32_t fd)
{
  struct file *f = flist_find_file(fd, thread_current());
  if (f == NULL)
    return -1;
  return file_length(f);
}

  static int32_t
sys_read(const int32_t fd, char* buf, const int32_t len)
{
  if (fd == STDIN_FILENO)
  {
    char c;
    int i;
    for (i = 0; i < len; ++i)
    {
      c = input_getc();
      buf[i] = c == '\r'? '\n' : c;
      putchar(c);
    }
    return i;
  }
  else
  {
    struct file *f = flist_find_file(fd, thread_current());
    if (f != NULL) {
      return file_read(f, buf, len);
    }
  }
  return -1;
}

  static int32_t
sys_write(const int32_t fd, char* buf, const int32_t len)
{
  if (fd == STDOUT_FILENO)
  {
    putbuf(buf, len);
    return len;
  }
  else
  {
    struct file *f = flist_find_file(fd, thread_current());
    if (f != NULL) {
      return file_write(f, buf, len);
    }
  }
  return -1;
}

  static void
sys_file_seek(int32_t fd, off_t pos)
{
  if (pos < 0)
  {
    return;
  }

  struct file *f = flist_find_file(fd, thread_current());
  if (f != NULL || pos <= file_length(f))
  {
    file_seek(f, pos);
  }
}

  static off_t
sys_file_tell(int32_t fd)
{
  struct file *f = flist_find_file(fd, thread_current());
  if (f != NULL)
    return file_tell(f);
  return -1;
}

  static void
sys_close(int32_t fd)
{
  filesys_close(flist_remove_file(fd, thread_current()));
}

  static void
sys_verify_variable_length(char* ptr)
{
  if(!verify_variable_length(ptr))
    process_exit(-1);
}
  static void
sys_verify_fix_length(void* ptr, uint32_t length)
{
  if(!verify_fix_length(ptr, length))
    process_exit(-1);
}


  static void
syscall_handler (struct intr_frame *f)
{
  int32_t* esp = (int32_t*)f->esp;

  sys_verify_fix_length(esp, 4);
  sys_verify_fix_length(esp+1, argc[*esp]*4);
  
  switch ( esp[0] )
  {
    case SYS_HALT:
      power_off();
      break;

    case SYS_EXIT:
      process_exit(esp[1]);
      break;

    case SYS_EXEC:
      sys_verify_variable_length((char*)esp[1]);
      f->eax = process_execute((char*)esp[1]);
      break;

    case SYS_WAIT:
      f->eax = process_wait(esp[1]);
      break;

    case SYS_CREATE:
      sys_verify_variable_length((char*)esp[1]);
      f->eax = filesys_create((const char*)esp[1], esp[2]);
      break;

    case SYS_REMOVE:
      sys_verify_variable_length((char*)esp[1]);
      f->eax = sys_remove((char*)esp[1]);
      break;

    case SYS_OPEN:
      sys_verify_variable_length((char*)esp[1]);
      f->eax = sys_open((char*)esp[1]);
      break;

    case SYS_FILESIZE:
      f->eax = sys_filesize(esp[1]);
      break;

    case SYS_READ:
      sys_verify_fix_length((char*)esp[2], esp[3]);
      f->eax = sys_read(esp[1], (char*)esp[2], esp[3]);
      break;

    case SYS_WRITE:
      sys_verify_fix_length((char*)esp[2], esp[3]);
      f->eax = sys_write(esp[1], (char*)esp[2], esp[3]);
      break;

    case SYS_SEEK:
      sys_file_seek(esp[1], esp[2]);
      break;

    case SYS_TELL:
      f->eax = sys_file_tell(esp[1]);
      break;

    case SYS_CLOSE:
      sys_close(esp[1]);
      break;

    case SYS_SLEEP:
      timer_msleep(esp[1]);
      break;

    case SYS_PLIST:
      process_print_list();
      break;

    default:
      {
        printf ("# Executed an unknown system call!\n");

        printf ("# Stack top + 0: %d\n", esp[0]);
        printf ("# Stack top + 1: %d\n", esp[1]);

        process_exit(-1);
      }
  }
}
