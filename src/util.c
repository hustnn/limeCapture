#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>

void print_debug_message(char* fmt,va_list ap)
{
   fprintf(stdout,fmt,ap);
   fprintf(stdout,"\n");
   fflush(stdout);
}

void debug_real(char* fmt,...)
{
   va_list ap;
   va_start(ap,fmt);
   print_debug_message(fmt,ap);
   va_end(ap);
}

void die(char* fmt,...)
{
   va_list ap;
   va_start(ap,fmt);
   print_debug_message(fmt,ap);
   va_end(ap);
   exit(1);
}

