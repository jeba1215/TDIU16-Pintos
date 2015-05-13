#include <debug.h>
#include <stdio.h>
#include <string.h>

#include "userprog/gdt.h"
#include "userprog/process.h"
#include "userprog/load.h"
#include "userprog/pagedir.h"
#include "userprog/tss.h"
#include "filesys/file.h"
#include "threads/flags.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "threads/interrupt.h"


#include "threads/synch.h"
#include "threads/malloc.h"
#include "userprog/flist.h"
#include "userprog/plist.h"
#include "userprog/map.h"

struct map process_list;

struct main_args
{

  void (*ret)(void);
  int argc;
  char** argv;
};

#define STACK_DEBUG(...) //printf(__VA_ARGS__)

void* setup_main_stack(const char* command_line, void* stack_top)
{
  struct main_args* esp;
  int argc;
  int total_size;
  int line_size;
  char* ptr_save;

  char* cmd_line_on_stack;


  /* CALCULate the bytes needed to store the command_line */
  line_size = strlen(command_line) + 1;
  STACK_DEBUG("# line_size = %d\n", line_size);

  /* round up to make it even divisible by 4 */
  line_size += 3 - (line_size - 1) % 4;
  STACK_DEBUG("# line_size (aligned) = %d\n", line_size);

  /* calculate how many words the command_line contain */
  //argc = count_args(command_line, " ");
  argc = 0;
  char* new_command_line = malloc(line_size);
  if(new_command_line == NULL)
  {
    return NULL;
  }
  strlcpy(new_command_line, command_line, line_size);
  char* token;
  for (token = strtok_r (new_command_line, " ", &ptr_save); token != NULL; token = strtok_r (NULL, " ", &ptr_save))
    {
      argc++;
    }
  
  STACK_DEBUG("# argc = %d\n", argc);


  /* calculate the size needed on our simulated stack */
  total_size = (4 + argc)*4 + line_size;
  STACK_DEBUG("# total_size = %d\n", total_size);

  /* calculate where the final stack top for the program will be located */
  esp = stack_top - total_size;

  /* setup return address and argument count */
  esp->ret = NULL;
  esp->argc = argc;

  /* calculate where in the memory the argv array starts */
  esp->argv = (char**) (esp + 1);
  esp->argv = (char**)((int)esp + 12);

  /* calculate where in the memory the words is stored */
  cmd_line_on_stack = stack_top - line_size;

  /* copy the command_line to where it should be in the stack */
  strlcpy(cmd_line_on_stack, command_line, line_size);

  /* build argv array and insert null-characters after each word */
  //char* token;
  char* cmd_copy = (char*)command_line;
  int i = 0;

  for (token = strtok_r (cmd_copy, " ", &ptr_save); token != NULL;
       token = strtok_r (NULL, " ", &ptr_save))
    {
      strlcpy(cmd_line_on_stack, token, strlen(token)+1);
      
      esp->argv[i] = cmd_line_on_stack;

      cmd_line_on_stack += strlen(token);
      strlcpy(cmd_line_on_stack,"\0",1);
      ++cmd_line_on_stack;
      ++i;
    }

  esp->argv[argc] = NULL;
  return esp; /* the new stack top */
}

void process_init(void)
{
  map_init(&process_list);
  plist_init();
}

/* This function is currently never called. As thread_exit does not
 * have an exit status parameter, this could be used to handle that
 * instead. Note however that all cleanup after a process must be done
 * in process_cleanup, and that process_cleanup are already called
 * from thread_exit - do not call cleanup twice! */
void process_exit(int status)
{
  plist_set_exit_status(&process_list, thread_current()->pid, status);
  thread_exit();
}


/* Print a list of all running processes. The list shall include all
 * relevant debug information in a clean, readable format. */
void process_print_list()
{
  plist_print_all(&process_list);
}


struct parameters_to_start_process
{
  struct semaphore* sema;
  bool success;
  int pid;
  char* command_line;
};

static void
start_process(struct parameters_to_start_process* parameters) NO_RETURN;

/* Starts a new proccess by creating a new thread to run it. The
   process is loaded from the file specified in the COMMAND_LINE and
   started with the arguments on the COMMAND_LINE. The new thread may
   be scheduled (and may even exit) before process_execute() returns.
   Returns the new process's thread id, or TID_ERROR if the thread
   cannot be created.
   */

  int
process_execute (const char *command_line)
{
  char debug_name[64];
  int command_line_size = strlen(command_line) + 1;
  tid_t thread_id = -1;
  int  process_id = -1;

  /* LOCAL variable will cease existence when function return! */
  struct parameters_to_start_process arguments;

  debug("%s#%d: process_execute(\"%s\") ENTERED\n",
      thread_current()->name,
      thread_current()->tid,
      command_line);

  /* COPY command line out of parent process memory */
  arguments.command_line = malloc(command_line_size);
  if (arguments.command_line == NULL){
    debug("Command line was invalid\n");
    return process_id;
  }
  debug("Command line is valid\n");
  strlcpy(arguments.command_line, command_line, command_line_size);

  strlcpy_first_word (debug_name, command_line, 64);

  struct semaphore s;
  sema_init(&s, 0);
  arguments.sema = &s;
  arguments.success = false;
  arguments.pid = thread_current()->pid;
  debug(" parent is: %d\n",arguments.pid);

  /* SCHEDULES function `start_process' to run (LATER) */
  thread_id = thread_create (debug_name, PRI_DEFAULT,
      (thread_func*)start_process, &arguments);
  debug("after create_thread 1, thread_id: %i\n", thread_id);

  if (thread_id != -1)
    sema_down(&s);
  debug("after create_thread 2\n");

  if (arguments.success)
    process_id = arguments.pid;
  debug("after create_thread 3\n");

  debug("Command Line: %s\n", arguments.command_line);

  /* WHICH thread may still be using this right now? */
  /* This causes panic in multi-recurse test */
  free(arguments.command_line);

  debug("after create_thread 4\n");

  debug("%s#%d: process_execute(\"%s\") RETURNS %d\n",
      thread_current()->name,
      thread_current()->tid,
      command_line, process_id);

  /* MUST be -1 if `load' in `start_process' return false */
  return process_id;
}

/* A thread function that loads a user process and starts it
   running. */
  static void
start_process (struct parameters_to_start_process* parameters)
{
  /* The last argument passed to thread_create is received here... */
  struct intr_frame if_;

  char file_name[64];
  strlcpy_first_word (file_name, parameters->command_line, 64);

  debug("%s#%d: start_process(\"%s\") ENTERED\n",
      thread_current()->name,
      thread_current()->tid,
      parameters->command_line);

  /* Initialize interrupt frame and load executable. */
  memset (&if_, 0, sizeof if_);
  if_.gs = if_.fs = if_.es = if_.ds = if_.ss = SEL_UDSEG;
  if_.cs = SEL_UCSEG;
  if_.eflags = FLAG_IF | FLAG_MBS;

  debug("Loading file: %s\n", file_name);
  parameters->success = load (file_name, &if_.eip, &if_.esp);
  debug("Loading done\n");
  

  debug("%s#%d: start_process(...): load returned %d\n",
      thread_current()->name,
      thread_current()->tid,
      parameters->success);

  struct semaphore* sema = malloc(sizeof(struct semaphore));
  if(sema == NULL)
    parameters->success = false;

  if ( parameters->success )
  { 
    sema_init(sema, 0);
    parameters->pid = plist_add_process(&process_list, parameters->pid, thread_current()->name, sema);
    if (parameters->pid == -1)
      parameters->success = false;
    thread_current()->pid = parameters->pid;

    // process_print_list();
    /* We managed to load the new program to a process, and have
       allocated memory for a process stack. The stack top is in
       if_.esp, now we must prepare and place the arguments to main on
       the stack. */

    /* A temporary solution is to modify the stack pointer to
       "pretend" the arguments are present on the stack. A normal
       C-function expects the stack to contain, in order, the return
       address, the first argument, the second argument etc. */

    //HACK if_.esp -= 12; /* Unacceptable solution. */
    if_.esp = setup_main_stack(parameters->command_line, if_.esp);
    if( if_.esp == NULL )
    {
      parameters->success = false;
    }

    /* The stack and stack pointer should be setup correct just before
       the process start, so this is the place to dump stack content
       for debug purposes. Disable the dump when it works. */

    //dump_stack ( PHYS_BASE + 15, PHYS_BASE - if_.esp + 16 );

  }

  debug("%s#%d: start_process(\"%s\") DONE\n",
      thread_current()->name,
      thread_current()->tid,
      parameters->command_line);

  sema_up( parameters->sema );
  /* If load fail, quit. Load may fail for several reasons.
     Some simple examples:
     - File does not exist
     - File do not contain a valid program
     - Not enough memory
     */
  if ( ! parameters->success )
  {
    debug("Load failed, exiting thread\n");
    thread_exit ();
  }

  /* Start the user process by simulating a return from an interrupt,
     implemented by intr_exit (in threads/intr-stubs.S). Because
     intr_exit takes all of its arguments on the stack in the form of
     a `struct intr_frame', we just point the stack pointer (%esp) to
     our stack frame and jump to it. */
  asm volatile ("movl %0, %%esp; jmp intr_exit" : : "g" (&if_) : "memory");
  NOT_REACHED ();
}

/* Wait for process `child_id' to die and then return its exit
   status. If it was terminated by the kernel (i.e. killed due to an
   exception), return -1. If `child_id' is invalid or if it was not a
   child of the calling process, or if process_wait() has already been
   successfully called for the given `child_id', return -1
   immediately, without waiting.

   This function will be implemented last, after a communication
   mechanism between parent and child is established. */
  int
process_wait (int child_id)
{
  int status = -1;
  struct thread *cur = thread_current ();

  debug("%s#%d: process_wait(%d) ENTERED\n",
      cur->name, cur->pid, child_id);
  /* Yes! You need to do something good here ! */
  struct process* p = plist_find_process(&process_list, child_id);
  if (p != NULL && p->parent == cur->pid)
  {
    struct semaphore* sema = p->sema;
    sema_down(sema);
    status = p->exit_status;
    plist_remove_process(&process_list, child_id, true);
  }
  debug("%s#%d: process_wait(%d) RETURNS %d\n",
      cur->name, cur->tid, child_id, status);

  return status;
}

/* Free the current process's resources. This function is called
   automatically from thread_exit() to make sure cleanup of any
   process resources is always done. That is correct behaviour. But
   know that thread_exit() is called at many places inside the kernel,
   mostly in case of some unrecoverable error in a thread.

   In such case it may happen that some data is not yet available, or
   initialized. You must make sure that nay data needed IS available
   or initialized to something sane, or else that any such situation
   is detected.
   */

  void
process_cleanup (void)
{
  struct thread  *cur = thread_current ();
  uint32_t       *pd  = cur->pagedir;
  int status = -1;

  debug("%s#%d: process_cleanup() ENTERED\n", cur->name, cur->tid);

  flist_remove_process(cur);
  struct process* process = plist_find_process(&process_list, cur->pid);
  if(process != NULL)
  {
    status = process->exit_status;
  }

  /* Later tests DEPEND on this output to work correct. You will have
   * to find the actual exit status in your process list. It is
   * important to do this printf BEFORE you tell the parent process
   * that you exit.  (Since the parent may be the main() function,
   * that may sometimes poweroff as soon as process_wait() returns,
   * possibly before the printf is completed.)
   */
  printf("%s: exit(%d)\n", thread_name(), status);

  if(process != NULL)
  {
    struct semaphore* sema = process->sema;
    sema_up(sema);
    plist_remove_process(&process_list, cur->pid, false);
  }

  /* Destroy the current process's page directory and switch back
     to the kernel-only page directory. */
  if (pd != NULL)
  {
    /* Correct ordering here is crucial.  We must set
       cur->pagedir to NULL before switching page directories,
       so that a timer interrupt can't switch back to the
       process page directory.  We must activate the base page
       directory before destroying the process's page
       directory, or our active page directory will be one
       that's been freed (and cleared). */
    cur->pagedir = NULL;
    pagedir_activate (NULL);
    pagedir_destroy (pd);
  }
  debug("%s#%d: process_cleanup() DONE with status %d\n",
      cur->name, cur->tid, status);
}

/* Sets up the CPU for running user code in the current
   thread.
   This function is called on every context switch. */
  void
process_activate (void)
{
  struct thread *t = thread_current ();

  /* Activate thread's page tables. */
  pagedir_activate (t->pagedir);

  /* Set thread's kernel stack for use in processing
     interrupts. */
  tss_update ();
}

