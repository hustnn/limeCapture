#ifndef __UTIL_H
#define __UTIL_H

#include "lime_capture.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>

void print_debug_message(char* fmt,va_list ap);

void debug_real(char* fmt,...);

void die(char* fmt,...);

#endif