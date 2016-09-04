/*
 * PtraceUtils.c
 *
 *  Created on: Aug 31, 2016
 *      Author: gpollep
 */
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/reg.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "SbDefines.h"

void GetData(pid_t child, long offset,
             char *str, int len)
{
	int long_size = sizeof(long);

	char *laddr;
    int i, j;
    union u {
            long val;
            char chars[long_size];
    }data;
    i = 0;
    j = len / long_size;
    laddr = str;
    while(i < j) {
//    	data.val = GET_REG(child,i, offset);
        data.val = ptrace(PTRACE_PEEKDATA,
                          child, offset + i * 8,
                          NULL);
        memcpy(laddr, data.chars, long_size);
        ++i;
        laddr += long_size;
    }
    j = len % long_size;
    if(j != 0) {
        data.val = ptrace(PTRACE_PEEKDATA,
                          child, offset + i * 8,
                          NULL);
        memcpy(laddr, data.chars, j);
    }
    str[len] = '\0';
}

void GetString(pid_t child, long offset,
             char *str)
{
	int long_size = sizeof(long);

	char *laddr;
    int i, len = 0;
    union u {
            long val;
            char chars[long_size];
    }data;
    i = 0;
    laddr = str;
    while(1) {
//    	data.val = GET_REG(child,i, offset);
        data.val = ptrace(PTRACE_PEEKDATA,
                          child, offset + i * 8,
                          NULL);
        if(strlen(data.chars) < 8)
        {
        	memcpy(laddr, data.chars, strlen(data.chars));
        	len += strlen(data.chars);
        	break;
        }

        memcpy(laddr, data.chars, long_size);
        ++i;
        laddr += long_size;
        len += long_size;

    }
    str[len] = '\0';
}

void AttcachProcess(const char* exec, int argc, char**argv)
{

}
