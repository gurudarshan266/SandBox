/*
 * ConfigParser.c
 *
 *  Created on: Aug 28, 2016
 *      Author: gpollep
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SbDefines.h"
#include "ConfigParser.h"

ConfigStruct* GenerateConfigStructs(const char* configFile, int* count)
{
	FILE* fp = fopen(configFile,"r");

	fscanf(fp, "%d", count);

	ConfigStruct* cs = (ConfigStruct*) calloc(*count, sizeof(ConfigStruct));

	int i;

	char* line = (char*)calloc(1024, sizeof(char));

	fgets(line, 1024, fp);

	for(i=0; i<*count;)
	{
		int permissions;

		fgets(line, 1024, fp);

		int n= strlen(line);

		if(n>0 && line[n-1]=='\n')
			line[n-1]='\0';
//		fscanf(fp,"%d %s",&permissions, cs[i].pattr);

		int err = Tokenize(line, cs[i].pattr, &permissions);

		if(err<0) continue;//Empty line

		cs[i].read = permissions/100;
		cs[i].write = (permissions%100)/10;
		cs[i].execute = (permissions%10);

#ifdef DEBUG
		printf("\nPermission = %d%d%d Pattern = %s",cs[i].read,cs[i].write,cs[i].execute, cs[i].pattr);
#endif
		i++;
	}

	fclose(fp);

	free(line);

	return cs;
}

int Tokenize(char* str, char* pattr, int* permission)
{
	const char* delimiter = " \t";
	char *tok = strtok(str, delimiter);

	int count = 0;

	while(tok != NULL && count<=2)
	{
		count++;

		if(count == 1)
			*permission = atoi(tok);

		else if(count == 2)
			strcpy(pattr, tok);

		tok = strtok(NULL, delimiter);
	}

	if(count < 2) return -1; //If the number of strings are less than 2, return invalid code

	return 0;


}

