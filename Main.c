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
	pid_t child_pid = fork();

	if (child_pid == 0)
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

		int isOpenAllowed = TRUE;
		int isRenameAllowed = TRUE;
		int isStatAllowed = TRUE;

		while (1)
		{
			child_pid = wait(&status);
			if (WIFEXITED(status))
				break;

			ptrace(PTRACE_SETOPTIONS, child_pid, 0,PTRACE_O_TRACECLONE|PTRACE_O_TRACEFORK);

			sys_call_num = ptrace(PTRACE_PEEKUSER, child_pid, 8 * ORIG_RAX,	NULL);



			switch(sys_call_num)
			{
				/* OPEN */
				case SYS_open:
				{
					printf("\n\nPID = %d System Call = %ld",child_pid,sys_call_num);

					if (isEntry == FALSE) {
						isEntry = TRUE;

						char *filenm = (char*)calloc(PATH_MAX,sizeof(char));
						char *parent_dir = (char*) calloc(PATH_MAX, sizeof(char));

						long rdi = GET_REG(child_pid, RDI, 0);
						GetString(child_pid, rdi, filenm);

						int flags = GET_REG(child_pid, RSI, 0);

						int read = ((flags&1) ==  O_RDONLY) || ((flags&0x2) == O_RDWR);

						int write = ((flags&1) ==  O_WRONLY) || ((flags&0x2) == O_RDWR);

						int exec =  ((flags&__O_CLOEXEC) != 0);

						int flags_to_check = 0;
						flags_to_check |= ((read)?READ:0) | ((write)?WRITE:0) | ((exec)?EXEC:0);

						//Check if the file's ancestor directories have permission
						GetParentDirectory(filenm,parent_dir);
						isOpenAllowed = CheckAncestorPermissions(filenm, parent_dir,EXEC,cs, configCount);

						printf("\n\nSys Open: filename = %s flags = %d", filenm, flags_to_check);

						if(isOpenAllowed)
							isOpenAllowed &= CheckAccess(filenm, cs, configCount, flags_to_check);

						free(filenm);
						free(parent_dir);
					}

					else
					{
							long rax = GET_REG(child_pid, RAX, 0);
							printf("\nSys Open: Return val = %ld", rax);

							if((isOpenAllowed == 0) && rax>2)
							{
								//TODO: Clean child's file descriptor
								SET_REG(child_pid, RAX, 0, -EACCES);
								printf("\nDisallowing access to file");
							}

						isEntry = FALSE;
					}
				}
				break;



				/* RENAME */
				case SYS_rename: /*Allow rename only when parent directory has execute permission*/
				{
					printf("\n\nPID = %d System Call = %ld",child_pid,sys_call_num);

				if (isEntry == FALSE) {
					isEntry = TRUE;

					char *filenm = (char*) calloc(PATH_MAX, sizeof(char));
					char *parent_dir = (char*) calloc(PATH_MAX, sizeof(char));
					int flags_to_check = EXEC;

					/* Source File */
					{
						long rdi = GET_REG(child_pid, RDI, 0);
						GetString(child_pid, rdi, filenm);

						GetParentDirectory(filenm,parent_dir);

						printf("\n\nSys Rename: Source Filename = %s  flags = %d", filenm,
								flags_to_check);

						isRenameAllowed = CheckAncestorPermissions(filenm, parent_dir,flags_to_check,cs, configCount);

						if(isRenameAllowed)
							isRenameAllowed &= CheckAccess(filenm, cs, configCount, READ);
					}

					/* Destination File */
					{
						long rsi = GET_REG(child_pid, RSI, 0);
						GetString(child_pid, rsi, filenm);

						GetParentDirectory(filenm,parent_dir);

						printf("\n\nSys Rename: Dst Filename = %s  flags = %d", filenm,
								flags_to_check);


						isRenameAllowed &= CheckAncestorPermissions(filenm, parent_dir,flags_to_check,cs, configCount);

						if(isRenameAllowed)
								isRenameAllowed &= CheckAccess(filenm, cs, configCount, WRITE);
					}

					//Rename permission is not allowed then send NULL as the source file name
					if(!isRenameAllowed)
					{
						SET_REG(child_pid, RDI, 0, NULL);
						printf("\nSys Rename: Not allowing to rename the file");
					}

					free(filenm);
					free(parent_dir);
				}

				else {
					long rax = GET_REG(child_pid, RAX, 0);
					printf("\nSys Rename: Return val = %ld", rax);

					if(isRenameAllowed == 0)
					{
						SET_REG(child_pid, RAX, 0, -EACCES);
					}
					isEntry = FALSE;
				}
			}
				break;





				/*ACCESS, STAT and LSTAT */
				case SYS_stat:
				case SYS_lstat:
				{
					printf("\n\nPID = %d System Call = %ld",child_pid,sys_call_num);

				if (isEntry == FALSE) {
					isEntry = TRUE;

					char *filenm = (char*) calloc(PATH_MAX, sizeof(char));
					char *parent_dir = (char*) calloc(PATH_MAX, sizeof(char));
					int flags_to_check = EXEC;

					long rdi = GET_REG(child_pid, RDI, 0);
					GetString(child_pid, rdi, filenm);

					GetParentDirectory(filenm,parent_dir);

					printf("\nSys (l)stat: Source Filename = %s  flags = %d", filenm,
							flags_to_check);

					isStatAllowed = CheckAncestorPermissions(filenm, parent_dir,flags_to_check,cs, configCount);

					if(!isStatAllowed)
					{
						SET_REG(child_pid, RDI, 0, NULL);
						printf("\nSys (l)stat: Not allowing access to the file");
					}

					free(filenm);
					free(parent_dir);
				}

				else {
					long rax = GET_REG(child_pid, RAX, 0);
					printf("\nSys (l)stat: Return val = %ld", rax);

					if(isStatAllowed == 0)
					{
						SET_REG(child_pid, RAX, 0, -EACCES);
					}
					isEntry = FALSE;
				}
			}
				break;

				default : break;
			}

			ptrace(PTRACE_SYSCALL, child_pid, NULL, NULL);
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
