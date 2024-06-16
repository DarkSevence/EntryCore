#include <stdio.h>

void WriteVersion()
{
#ifndef __WIN32__
	FILE* fp = fopen("version.txt", "w");

	if (fp)
	{
		fclose(fp);
	}
#endif
}

