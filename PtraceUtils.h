/*
 * PtraceUtils.h
 *
 *  Created on: Aug 31, 2016
 *      Author: gpollep
 */

#ifndef PTRACEUTILS_H_
#define PTRACEUTILS_H_

#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void GetData(pid_t child, long offset, char *str, int len);
void GetString(pid_t child, long offset, char *str);

#endif /* PTRACEUTILS_H_ */
