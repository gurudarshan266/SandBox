/*
 * GlobHelper.h
 *
 *  Created on: Aug 28, 2016
 *      Author: gpollep
 */

#ifndef GLOBUTILS_H_
#define GLOBUTILS_H_

#include "SbDefines.h"

int GetFilesList(const char* pattr, char*** fileList, int* count);
int Find(const char* fileName, char** fileList, int count, int);
int CheckAccess(const char* file, ConfigStruct* cs, int configCount, int permission);
int GlobMatch(const char* file, ConfigStruct* cs);
void DumpFileList(char** FileList, int count);
void GetParentDirectory(char* fn, char* result);
int CheckAncestorPermissions(char* filenm, char* parent_dir,int flags_to_check,ConfigStruct* cs, int configCount);

#endif /* GLOBUTILS_H_ */
