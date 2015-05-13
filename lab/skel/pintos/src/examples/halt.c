/* halt.c

   Simple program to test whether running a user program works.
 	
   Just invokes a system call that shuts down the OS. */

#include <random.h>
#include <stdio.h>
#include <string.h>
#include <syscall.h>
#include "tests/filesys/base/syn-write.h"
#include "tests/lib.h"
#include "tests/main.h"

char buf1[BUF_SIZE];
char buf2[BUF_SIZE];

int
main (void)
{
  debug ("jesper - exec(\"corrupt-elf\"): %d\n", exec ("corrupt-elf"));

  return 0;
}
