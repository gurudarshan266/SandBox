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

#endif /* GLOBUTILS_H_ */
