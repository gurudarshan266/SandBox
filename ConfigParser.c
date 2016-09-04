/*
 * ConfigParser.c
 *
 *  Created on: Aug 28, 2016
 *      Author: gpollep
 */

#include <stdio.h>
#include <stdlib.h>

#include "SbDefines.h"

ConfigStruct* GenerateConfigStructs(const char* configFile, int* count)
{
	FILE* fp = fopen(configFile,"r");

	fscanf(fp, "%d", count);

	ConfigStruct* cs = (ConfigStruct*) calloc(*count, sizeof(ConfigStruct));

	int i;

	for(i=0; i<*count; i++)
	{
		int permissions;
		fscanf(fp, "%d %s",&permissions, cs[i].pattr);
		cs[i].read = permissions/100;
		cs[i].write = (permissions%100)/10;
		cs[i].execute = (permissions%10);

#ifdef DEBUG
		printf("\nPermission = %d%d%d Pattern = %s",cs[i].read,cs[i].write,cs[i].execute, cs[i].pattr);
#endif
	}

	fclose(fp);

	return cs;
}

