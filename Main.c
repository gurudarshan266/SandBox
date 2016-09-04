/*
 * Main.c
 *
 *  Created on: Aug 28, 2016
 *      Author: gpollep
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/reg.h>
#include <sys/syscall.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/limits.h>

#include "GlobUtils.h"
#include "SbDefines.h"
#include "ConfigParser.h"
#include "PtraceUtils.h"

int main(int argc, char** argv)
{
	if(argc<2)
	{
		printf("Missing Arguments\n");
		printf("fend [-c config] <command [args ...]>");
		return -1;
	}

	char configFile[256];
	int configFileFound;

	if(strcmp(argv[1],"-c") == 0)
	{
		strcpy(configFile, argv[2]);
		configFileFound = TRUE;
	}
	else
	{
		FILE* fp = fopen(".fendrc","r");
		if(fp == NULL)//.fendrc not found in current directory. Look for it in home
		{
			fp = fopen("~/.fendrc","r");
			if(fp == NULL)
			{
				printf("Must provide a config file.");
				return -1;
			}
		}
		fclose(fp);//Will be opened again later

		strcpy(configFile, ".fendrc");
		configFileFound = FALSE;
	}



	int nextArgPos = (configFileFound==TRUE)? 3 : 1;

	int configCount = 0;
	ConfigStruct* cs = GenerateConfigStructs(configFile,&configCount);

	/*Fork a child*/
	pid_t child = fork();

	if (child == 0)
	{
		ptrace(PTRACE_TRACEME, 0, NULL, NULL);
		execl("/bin/sh", "sh", "test.sh", NULL);
		return -1;
	}
	else
	{
		long sys_call_num;
		int status;
		int isEntry = FALSE;//Start with true to not account execve sys call

		int isOpenAllowed = FALSE;

		while (1)
		{
			wait(&status);
			if (WIFEXITED(status))
				break;

			sys_call_num = ptrace(PTRACE_PEEKUSER, child, 8 * ORIG_RAX,	NULL);


			switch(sys_call_num)
			{
				case SYS_open:
				{
					if (isEntry == FALSE) {
						isEntry = TRUE;

						char *filenm = (char*)calloc(PATH_MAX,sizeof(char));
						long rdi = GET_REG(child, RDI, 0);
						GetString(child, rdi, filenm);
						printf("\nSys Open: filename = %s", filenm);

						int flags = GET_REG(child, RSI, 0);

						int read = ((flags&0x3) ==  O_RDONLY) || ((flags&0x3) == O_RDWR);

						int write= ((flags&0x3) ==  O_WRONLY) || ((flags&0x3) == O_RDWR);

						int flags_to_check = 0;
						flags_to_check |= ((read)?1<<READ:0) | ((write)?1<<WRITE:0);

						isOpenAllowed = CheckAccess(filenm, cs, configCount, flags_to_check);
					}

					else
					{
							long rax = GET_REG(child, RAX, 0);
							printf("\nreturn val = %ld", rax);

							if(isOpenAllowed!=1 && rax>2)
							{
								//TODO: Clean child's file descriptor
								SET_REG(child, RAX, 0, EACCES);
							}

						isEntry = FALSE;
					}
				}
				break;

				default : break;
			}

			ptrace(PTRACE_SYSCALL, child, NULL, NULL);
		}
	}



#ifdef DEBUG2
	int err = GetFilesList(pattr, &filelist, &count);
	if(err == 0)
	{
		int exists = Find(fileName,filelist, count);
		printf("\n%s Exists = %d\n", fileName, exists);
	}


	if(err == 0)
	{
		printf("\nCount = %d ",count);
		int i;
		for(i=0;i<(int)count;i++)
			printf("\n%s",filelist[i]);
	}
#endif
}
