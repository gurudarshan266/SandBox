/*
 * GlobUtils.c
 *
 *  Created on: Aug 28, 2016
 *      Author: gpollep
 */

#include "GlobUtils.h"
#include "SbDefines.h"

#include <glob.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/limits.h>
#include <libgen.h>

int GetFilesList(const char* pattr, char*** fileList, int* count)
{
	glob_t glob_result;

	int result = glob(pattr, GLOB_TILDE, NULL, &glob_result);

	if(result != 0)//Error has occurred
	{
#ifdef DEBUG
		switch(result)
		{
			case GLOB_NOSPACE:
				printf("\nError occurred due to low memory ");
				break;
			case GLOB_ABORTED:
				printf("\nError occurred while reading");
				break;
			case GLOB_NOMATCH:
				printf("\nNo matches were found for the pattern %s",pattr);
				break;
			default:
				printf("\nError occurred while fetching files list with error code = %d",result);
				break;
		}
#endif
		return result;
	}

	//Copies the string array to a new array
	*count = glob_result.gl_pathc;
	*fileList = (char**) malloc(sizeof(char*)*glob_result.gl_pathc);

	int k;
	for(k=0; k<*count;k++)
	{
		(*fileList)[k] = (char*) malloc( sizeof(char) * (strlen(glob_result.gl_pathv[k])+1) );
		strcpy((*fileList)[k],glob_result.gl_pathv[k]);
	}


	globfree(&glob_result);

	return result;
}

//TODO: Use lfind() in stdlib.h
int Find(const char* fileName, char** fileList, int count, int useAbsPath)
{
	int i;

	int res = FALSE;

	char *absPathFn = (char*)calloc(PATH_MAX,sizeof(char));
	char *absPathFileList = (char*)calloc(PATH_MAX,sizeof(char));

	for(i=0; i<count; i++)
	{
		if(useAbsPath)//Use absolute path for matching
		{
			realpath(fileName,absPathFn);
			realpath(fileList[i],absPathFileList);

			if(strcmp(absPathFn,absPathFileList) == 0)
			{
				res = TRUE;
				break;
			}
		}
		else
		{
			if(strcmp(fileName,fileList[i]) == 0)
			{
				res = TRUE;
				break;
			}
		}
	}

	free(absPathFn);
	free(absPathFileList);

	return res;
}

int GlobMatch(const char* file, ConfigStruct* cs)
{
	char** fileList;
	int count = 0;

	int err = GetFilesList(cs->pattr, &fileList, &count);

	if(err == 0) //Glob call was successful
	{
		int res = Find(file, fileList, count, TRUE);

		//Clean up FileList memory
		if(fileList)
		{
//			int i;
//			DumpFileList(fileList, count);
		}

		return res;
	}

	return FALSE;
}
/* Checks User Permission set by OS and the permission set in config file for final result
 * If a file matches more than one pattern, the last pattern will be considered
 * No default access is allowed
 */
int CheckAccess(const char* file, ConfigStruct* cs, int configCount, int requested_permission)
{
	int read = (requested_permission & (READ))>>2;
	int write = (requested_permission & (WRITE))>>1;
	int execute = (requested_permission & (EXEC))>>0;

	int i, result = TRUE;

	for(i = 0; i<configCount;i++)
	{
		int isMatch = GlobMatch(file, &cs[i] );

		if(isMatch)
		{
			int config_permission = (cs[i].read<<2) | (cs[i].write<<1) | (cs[i].execute<<0);
			result = (config_permission & requested_permission) == requested_permission;
#ifdef DEBUG
			printf("\nFile = %s Pattern = %s Access = %d",file, cs[i].pattr, result);
#endif
		}
	}

	return result;
}

void DumpFileList(char** FileList, int count)
{
	printf("\n\nCount = %d",count);
	int i;
	for(i=0;i<(int)count;i++)
		printf("\n%s",FileList[i]);
}

void GetParentDirectory(char* fn, char* result)
{
	char* tmp = (char*)malloc(sizeof(char)*PATH_MAX);
	realpath(fn,tmp);

	strcpy(result,dirname(tmp));

	free(tmp);

}

int CheckAncestorPermissions(char* filenm, char* parent_dir,int flags_to_check,ConfigStruct* cs, int configCount)
{
	int result = TRUE;

	GetParentDirectory(filenm,parent_dir);

	printf("\n\nFilename = %s  flags = %d", filenm,
			flags_to_check);

	while(1)
	{
		printf("\nChecking permissions for directory \"%s\"",parent_dir);

		result &= CheckAccess(parent_dir, cs, configCount,
				flags_to_check);

		if(strcmp(parent_dir, "/")==0 || result==0)
			break;

		GetParentDirectory(parent_dir,parent_dir);
	}

	return result;
}
//int main() {}

