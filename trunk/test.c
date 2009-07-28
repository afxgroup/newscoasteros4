#include <stdio.h>
#include <time.h>
#include <proto/dos.h>

BOOL FileCopy(const char *FileIn, const char *FileOut)
{
	int64 inlen,outlen;
	int8 buffer[ 131072 ];
	BPTR in  = Open(FileIn,MODE_OLDFILE);  //check
	BPTR out = Open(FileOut,MODE_NEWFILE);  //check
	BOOL success = TRUE;

	if (!in || !out)
	{
		if (in) Close(in);
		if (out) Close(out);
		success = FALSE;
	}

	if (success)
	{
		do
		{
			inlen = Read(in, buffer, sizeof(buffer) );

			if( inlen >0 )
			{
				outlen = Write(out, buffer, inlen);

				if( outlen != inlen )
				{
					fprintf(stderr, "Error during copying file from %s to %s\n",FileIn,FileOut);
					success = FALSE;
					break;
				}
			}
		}
		while( inlen > 0 );
	}
	if (in)	Close(in);
	if (out) Close(out);

	return success;
}


int main()
{
	char cps[32];
	clock_t bytes_downloaded = 1306;
	clock_t time2 = 725927;
	clock_t testvar = 50 * bytes_downloaded;
	printf("1: %lld\n",testvar);
	testvar = testvar / time2;
	printf("2: %lld\n",testvar);
	sprintf(cps,"3: %lld", (50 * bytes_downloaded * 100) / time2 );
	printf("%s\n",cps);

	FileCopy("Ram:echo.txt","RAM:echo2.txt");
	FileCopy("Ram:echo.txt","DH1:echo2.txt");
	FileCopy("Ram:echo.txt","DH1:Dev/echo2.txt");
	return 0;
}
