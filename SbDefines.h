/*
 * SbDefines.h
 *
 *  Created on: Aug 28, 2016
 *      Author: gpollep
 */

#ifndef SBDEFINES_H_
#define SBDEFINES_H_

struct configStruct {
	char pattr[50];
	int read;
	int write;
	int execute;
};

typedef struct configStruct ConfigStruct;

#define TRUE 1
#define FALSE 0

#define READ 4
#define WRITE 2
#define EXEC 1

//#define GET_REG(pid, regAddr) ptrace(PTRACE_PEEKUSER, pid, (regAddr*8), NULL)
#define GET_REG(pid, regAddr, offset) ptrace(PTRACE_PEEKUSER, pid, (offset + 8*regAddr), NULL)
#define SET_REG(pid, regAddr, offset, val) ptrace(PTRACE_POKEUSER, pid, (offset + 8*regAddr), val)

#endif /* SBDEFINES_H_ */
