/*
    NewsCoaster
    Copyright (C) 1999-2004 NewsCoaster development team

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    NewsCoaster Homepage :
        http://newscoaster.tripod.com/

    Mailing List to hear about announcements of new releases etc at:
        http://groups.yahoo.com/group/newscoaster-announce/
*/

#include <dos/dos.h>
#include <libraries/gadtools.h>
#include <libraries/mui.h>

#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>

#include <proto/dos.h>
#include <proto/locale.h>
#include <proto/muimaster.h>
#include <proto/utility.h>
#include <proto/codesets.h>

#include "vector.h"
#include "various.h"
#include "misc.h"
#include "datehandler.h"
#include "strings.h"
#include "main.h"

extern struct codeset *sysCodeset;

/* Variable for the nLog #define. */
BOOL logbool = FALSE;

/* The log file handle. */
BPTR logFile=NULL;

/* Reads a line (either CR or LF terminated) from 'file' into 'buffer'.
 */
void Readln(BPTR file,char * buffer) {
	LONG ch = 0;
	for(;;) {
		ch=FGetC(file);
		if(ch==-1 || ch==10 || ch==13) {
			*buffer=0;
			break;
		}
		*buffer=(char)ch;
		buffer++;
	}
}

/* Decodes quoted printable plain text from 'in' and copies the result to
 * 'out'.
 * 'length' is the length of the encoded text.
 * 'nl' should be TRUE iff newline characters are to be skipped (this will
 * normally be the case).
 */
int decode_qprint(const char * in,char * out,int length,BOOL nl) {
	// leave as %d not %s below!
	nLog("decode_qprint((char *)%d,(char *)%d,(int)%d,(BOOL)%d) called\n",in,out,length,nl);
	char t[4]="";
	int asc=0;
	int d=0;
	const char * end=&in[length];
	do {
		if(*in=='=') {
			in++;
			if(*in==10) {
				if(nl) {
					in++;
					if(*in==13)
						in++;
				}
			}
			else {
				strncpy(t,in,2);
				t[2]=0;
				in+=2;
				sscanf(t,"%x",&asc);
				*out++=(unsigned char)asc;
				d++;
			}
		}
		else {
			*out++=*in++;
			d++;
		}
	} while(in<end);
	*out= '\0';
	return d;
}

/* Output a newline character to 'out', followed by 'quoteLevel' '>'
 * characters.
 */
void newLineHtml(char **out,int quoteLevel) {
	*(*out)++ = 10;
	for(int i=0;i<quoteLevel;i++) {
		*(*out)++ = '>';
		*(*out)++ = ' ';
	}
}

/* Converts HTML from 'in' into plain text and outputs the result to 'out'.
 * 'length' is the length of the HTML text.
 * 'reset' is whether to reset the quote level.
 */
void parse_html(char * in,char * out,int length,BOOL reset) {
	// leave as %d not %s below!
	nLog("parse_html((char *)%s,(char *)%d,(int)%d,(BOOL)%d) called\n",in,out,length,reset);
	static BOOL inTag=FALSE;
	static int quoteLevel = 0;
	if(reset) {
		inTag=FALSE;
		quoteLevel = 0;
	}
	char t[16] = "";
	char *end=&in[length];
	do {
		if(*in==10 || *in==13) {
			*out++ = ' ';
			in++;
		}
		else if(*in == '&') {
			in++;
			int i=0;
			while(*in != ';' && i<15)
				t[i++] = *in++;
			in++;
			t[i] = 0;
			if(iequals(t,"nbsp"))
				*out++ = ' ';
			else if(iequals(t,"amp"))
				*out++ = '&';
			else if(iequals(t,"copy"))
				*out++ = '©';
			else if(iequals(t,"lt"))
				*out++ = '<';
			else if(iequals(t,"gt"))
				*out++ = '>';
			else if(*t == '#' && i>1) {
				int val = atoi(&t[1]);
				*out++ = (char)val;
			}
		}
		else {
			if(inTag) {
				if(*in == '>')
					inTag=FALSE;
				in++;
			}
			else {
				if(*in == '<') {
					inTag=TRUE;
					in++;
					strncpy(t,in,15);
					t[15]=0;
					if(STRNICMP(in,"B>",2)==0)
						*out++ = '*';
					else  if(STRNICMP(in,"/B>",3)==0)
						*out++ = '*';
					else  if(STRNICMP(in,"I>",2)==0)
						*out++ = '/';
					else  if(STRNICMP(in,"/I>",3)==0)
						*out++ = '/';
					else  if(STRNICMP(in,"U>",2)==0)
						*out++ = '_';
					else  if(STRNICMP(in,"/O>",3)==0)
						*out++ = '_';
					else  if(STRNICMP(in,"BR>",3)==0)
						newLineHtml(&out,quoteLevel);
					else  if(STRNICMP(in,"P>",2)==0) {
						newLineHtml(&out,quoteLevel);
						newLineHtml(&out,quoteLevel);
					}
					else if(STRNICMP(in,"BLOCKQUOTE>",11)==0 || STRNICMP(in,"BLOCKQUOTE ",11)==0) {
						quoteLevel++;
						newLineHtml(&out,quoteLevel);
					}
					else if(STRNICMP(in,"/BLOCKQUOTE>",12)==0) {
						if(quoteLevel>0) {
							quoteLevel--;
							newLineHtml(&out,quoteLevel);
						}
					}
				}
				else
					*out++ = *in++;
			}
		}
	} while(in<end);
	*out=0;
}

/* Decodes base64 'in' and copies the result to 'out'.
 * 'length' is the length of the encoded text.
 */
int decode_base64(const char *in,char *out,int length) {
	// leave as %d not %s below!
	nLog("decode_base64((char *)%d,(char *)%d,(int)%d) called\n",in,out,length);
	char *buffer = new char[length + 1024];
	char *buffero = buffer;
	int d=0,d2=0,c=0;
	const char *end = &in[length];
	do {
		if(*in>='A' && *in<='Z') {
			*buffer++=*in - 'A';
			d++;
		}
		else if(*in>='a' && *in<='z') {
			*buffer++=*in - 'a' + 26;
			d++;
		}
		else if(*in>='0' && *in<='9') {
			*buffer++=*in - '0' + 52;
			d++;
		}
		else if (*in=='+') {
			*buffer++=62;
			d++;
		}
		else if (*in=='/') {
			*buffer++=63;
			d++;
		}
		else if (*in=='=') {
			//end of data?
			break;
		}
		in++;
	} while(in<end);
	*buffer = '\0';
	d2=0;
	//printf("d = %d\n",d);
	for(c=0;c<d;c+=4) {
		if( c + 3 >= d ) {
			// needs special handling!
			*out++ = buffero[c]*4 + (buffero[c+1] / 16);
			d2++;

			if( c + 2 >= d ) {
				//printf("!\n");
				break;
			}

			*out++ = (buffero[c+1] % 16)*16 + (buffero[c+2] / 4);
			d2++;
			break;
		}
		*out++ = buffero[c]*4 + (buffero[c+1] / 16);
		*out++ = (buffero[c+1] % 16)*16 + (buffero[c+2] / 4);
		*out++ = (buffero[c+2] % 4)*64 + buffero[c+3];
		d2+=3;
	}
	delete [] buffero;
	//printf("d2 = %d\n",d2);
	return d2;
}

int encode_base64(char *in,char *out,int length) {
	// leave as %d not %s below!
	nLog("encode_base64((char *)%d,(char *)%d,(int)%d) called\n",in,out,length);
	char *buffer = new char[(length*5)/3+8192];
	char *buffero = buffer;
	int d=0,d2=0,c=0;
	//printf("length = %d\n",length);
	for(c=0;c<length;c+=3) {
		if( c + 2 >= length ) {
			// needs special handling!
			*buffer++ = (in[c] & 252)/4;
			if( c + 1 >= length ) {
				*buffer++ = (in[c] & 3)*16;
				*buffer++=255;
				*buffer++=255;
				d+=4;
				break;
			}
			*buffer++ = (in[c] & 3)*16 + (in[c+1] & 240)/16;
			*buffer++ = (in[c+1] & 15)*4;
			*buffer++=255;
			d+=4;
			break;
		}
		*buffer++ = (in[c] & 252)/4;
		*buffer++ = (in[c] & 3)*16 + (in[c+1] & 240)/16;
		*buffer++ = (in[c+1] & 15)*4 + (in[c+2] & 192)/64;
		*buffer++ = (in[c+2] & 63);
		d+=4;
	}
	for(c=0;c<d;c++) {
		unsigned char ch = (unsigned char)buffero[c];
		if(ch < 26) {
			*out++=buffero[c] + 'A';
			d2++;
		}
		else if(ch >= 26 && ch < 52) {
			*out++=buffero[c] - 26 + 'a';
			d2++;
		}
		else if(ch >= 52 && ch < 62) {
			*out++=buffero[c] - 52 + '0';
			d2++;
		}
		else if(ch == 62) {
			*out++='+';
			d2++;
		}
		else if(ch == 63) {
			*out++='/';
			d2++;
		}
		else if(ch == 255) {
			*out++='=';
			d2++;
		}
		if((d2+1) % 73 == 0) {
			*out++=10;
			d2++;
		}
	}
	*out = '\0';
	delete [] buffero;
	return d2;
}

// todo: better handling of multifile messages
int ydecode(BOOL *corrupt,char *in,char *out,int length,uuEncodeData *uudata) {
	nLog("ydecode((BOOL *)%d,(char *)%d,(char *)%d,(int)%d,(uuEncodeData *)%d) called\n",corrupt,in,out,length,uudata);
	*corrupt = FALSE;
	char *end=&in[length];
	char *in2=NULL,*begin=NULL;
	char filename[1024] = "unnamed";
	//char bbuf[1024] = "";
	//strcpy(filename_g,"unnamed");
	if(STRNICMP(in,"=ybegin ",8)==0) {
		in2 = in;
		begin = in;
	}
	else {
		in2 = stristr_q(in,"\n=YBEGIN ");
		begin = in2+1;
	}
	in2 = strchr((char *)&in2[1],'\n');
	in2++;

	int size1 = -1;
	{
		int size = (int)(in2-begin);
		char *temp = new char[size + 1];
		strncpy(temp,(char *)begin,size);
		temp[size]=0;
		// name
		char *ptr = stristr_q(temp,"NAME=");
		if(ptr != NULL) {
			ptr += 5;
			char *eptr = NULL;
			if(*ptr == '\"') {
				ptr++;
				eptr = strchr(ptr,'\"');
			}
			else {
				eptr = strchr(ptr,' ');
				char *nl = strchr(ptr,'\n');
				if( (nl != NULL && nl < eptr) || eptr == NULL )
					eptr = nl;
			}
			if(eptr == NULL)
				strcpy(filename,ptr);
			else {
				strncpy(filename,ptr,(int)(eptr - ptr));
				filename[(int)(eptr - ptr)] = '\0';
			}
			StripTrail(filename,'\r');
		}
		// size
		ptr = stristr_q(temp,"SIZE=");
		if(ptr != NULL) {
			size1 = atoi( &ptr[5] );
			nLog("  found size1 = %d\n",size1);
		}
		delete [] temp;
	}

	strncpy(uudata->filename,filename,UUENCODEDATA_FILENAME_LEN);
	uudata->filename[UUENCODEDATA_FILENAME_LEN] = '\0';
	char *dot = NULL,*ndot = NULL;
	dot = strchr(uudata->filename,'.');
	while(dot != NULL) {
		ndot = strchr(&dot[1],'.');
		if(ndot == NULL)
			break;
		dot = ndot;
	}
	if(dot)
		strncpy(uudata->ext,dot,UUENCODEDATA_EXT_LEN);
	else
		strncpy(uudata->ext,".txt",UUENCODEDATA_EXT_LEN);
	uudata->ext[UUENCODEDATA_EXT_LEN] = '\0';

	int d2 = 0;
	//int size2 = -1;
	do {
		unsigned char c = *in2++;
		if(c == '\0' || c == 10 || c == 13) {
		//if(c == '\0' || c == 9 || c == 10 || c == 13) {
			// skip
		}
		else if(c == '=') {
			char c2 = *in2++;
			if(c2 == 'y' || c2 == 'Y') {
				//printf("%c %c %c\n",in2[0],in2[1],in2[2]);
				/*if(strincmp_q(in2,"END ",4)==0) {
					char *ptr = stristr_q(in2,"SIZE=");
					if(ptr != NULL) {
						size2 = atoi( &ptr[5] );
						nLog("  found size2 = %d\n",size2);
					}
				}*/
				char *nl = strchr(in2,'\n');
				if(nl != NULL)
					in2 = &nl[1];
				//printf(":%d %d\n",in2,end);
			}
			else {
				c2 = (unsigned char)(c2 - 64);
				c2 = (unsigned char)(c2 - 42);
				*out++ = c2;
				d2++;
			}
		}
		else {
			c = (unsigned char)(c - 42);
			*out++ = c;
			d2++;
		}
	} while(in2<end);
	*out = '\0';

	nLog("  actual length is = %d\n",d2);
	//if(d2 != size1 || d2 != size2) {
	if(d2 != size1) {
		*corrupt = TRUE;
	}

	return d2;
}

int uudecode(char *in,char *out,int length,uuEncodeData *uudata) {
	// leave as %d not %s below!
	nLog("uudecode((char *)%d,(char *)%d,(int)%d,(uuEncodeData *)%d) called\n",in,out,length,uudata);
	char *buffer = new char[length + 1024];
	char *buffero = buffer;
	int d=0,d2=0,c=0;
	char *end=&in[length];
	char *in2=NULL,*begin=NULL;
	char filename[1024] = "";
	char *out_stored = out;
	//char bbuf[1024] = "";
	//if(strincmp(in,"begin ",6)==0 && isdigit(in[6]) ) {
	// we should be at the start anyway
	if(STRNICMP(in,"begin ",6)==0) {
		in2=in;
		begin=in;
	}
	else {
		in2=stristr_q(in,"\nBEGIN ");
		begin=in2+1;
	}
	in2=strchr(&in2[1],'\n');
	in2++;
	{
		int size = (int)(in2-begin);
		char *temp = new char[size + 1];
		strncpy(temp,(char *)begin,size);
		temp[size] = '\0';
		word(filename,temp,3);
		delete [] temp;
	}
	strncpy(uudata->filename,filename,UUENCODEDATA_FILENAME_LEN);
	uudata->filename[UUENCODEDATA_FILENAME_LEN] = '\0';
	char *dot = NULL,*ndot = NULL;
	dot = strchr(uudata->filename,'.');
	while(dot != NULL) {
		ndot = strchr(&dot[1],'.');
		if(ndot == NULL)
			break;
		dot = ndot;
	}
	if(dot)
		strncpy(uudata->ext,dot,UUENCODEDATA_EXT_LEN);
	else
		strncpy(uudata->ext,".txt",UUENCODEDATA_EXT_LEN);
	uudata->ext[UUENCODEDATA_EXT_LEN] = '\0';
	int count = 0;
	int k = 0;
	do {
		if(*in2=='\n' || *in2=='\0')
			break;
		if(STRNICMP(in2,"end",3)==0 && isspace(in2[4]))
			break;
		count = (int)*in2++ - 32;
		if(count % 3 != 0) {
			count+=3;
			if(count % 3 == 1)
				d-=2;
			if(count % 3 == 2)
				d--;
		}
		count = (count / 3) * 4;
		for(k=0;k<count;k++) {
			*buffer++ = (*in2++ - 32) % 64;
			d++;
		}
		while(*in2!='\n' && *in2!='\0')
			in2++;
		if(*in2=='\0')
			break; // just to be safe
		in2++;
	} while(in2<end);
	*buffer = '\0';
	d2=0;
	for(c=0;c<d;c+=4) {
		if( d-c<4) {
			// needs special handling!
			*out++ = buffero[c]*4 + (buffero[c+1] / 16);
			d2++;
			if(c+2>=d)
				break;
			*out++ = (buffero[c+1] % 16)*16 + (buffero[c+2] / 4);
			d2++;
			break;
 		}
		*out++ = buffero[c]*4 + (buffero[c+1] / 16);
		*out++ = (buffero[c+1] % 16)*16 + (buffero[c+2] / 4);
		*out++ = (buffero[c+2] % 4)*64 + buffero[c+3];
		d2+=3;
	}
	*out = '\0';
	delete [] buffero;
	return d2;
}

/* Converts a file extension 'ext' into a MIMEType 'type'.
 */
void getMIMEType(char * type,const char * ext) {
	// leave as %d not %s below!
	nLog("getMIMEType((char *)%s,(char *)ext) called\n",type,ext);
	if(iequals(ext,".txt"))
		strcpy(type,"text/plain");
	else if(iequals(ext,".html"))
		strcpy(type,"text/html");
	else if(iequals(ext,".htm"))
		strcpy(type,"text/html");
	else if(iequals(ext,".shtml"))
		strcpy(type,"text/html");
	else if(iequals(ext,".guide"))
		strcpy(type,"text/x-aguide");
	else if(iequals(ext,".exe"))
		strcpy(type,"application/octet-stream");
	else if(iequals(ext,".ps"))
		strcpy(type,"application/postscript");
	else if(iequals(ext,".lha"))
		strcpy(type,"application/x-lha");
	else if(iequals(ext,".lzx"))
		strcpy(type,"application/x-lzx");
	else if(iequals(ext,".zip"))
		strcpy(type,"application/x-zip");
	else if(iequals(ext,".jpeg"))
		strcpy(type,"image/jpeg");
	else if(iequals(ext,".jpg"))
		strcpy(type,"image/jpeg");
	else if(iequals(ext,".png"))
		strcpy(type,"image/png");
	else if(iequals(ext,".gif"))
		strcpy(type,"image/gif");
	else if(iequals(ext,".iff"))
		strcpy(type,"image/x-ilbm");
	else if(iequals(ext,".wav"))
		strcpy(type,"audio/x-wav");
	else if(iequals(ext,".mpeg"))
		strcpy(type,"video/mpeg");
	else if(iequals(ext,".mpg"))
		strcpy(type,"video/mpeg");
	else if(iequals(ext,".mov"))
		strcpy(type,"video/quicktime");
	else if(iequals(ext,".anim"))
		strcpy(type,"video/x-anim");
}

static char charset_table[512];
static char current_charset[100]="";

/*
BOOL translateCharset(unsigned char * text,char * charset, int mode) {
	BPTR fptr;
	int i;


	if (strstr(charset,"UTF-8")>0)
	{
		if (text)
		{
			// now convert this prossible UTF8 string to a normal string
			char *dest1 = CodesetsUTF8ToStr(CSA_Source, text,
											CSA_DestCodeset, sysCodeset->name,
											TAG_DONE);
			printf("%s\n",dest1);
			realloc(text,strlen(dest1)+1);
			strcpy((char*)text,dest1);
			text[strlen(dest1)] = 0;
		}
		return FALSE;
	}
	else
	{
		if (stricmp(current_charset,charset))
		{
			strcpy(current_charset,charset);
			sprintf(charset_table,"PROGDIR:Charsets/%s.charset",charset);
			fptr=Open(charset_table,MODE_OLDFILE);
			if (fptr)
			{
				Read(fptr,charset_table,512);
				Close(fptr);
			}
			else
				for (i=0;i<256;i++)
				{
					charset_table[i]=i;
					charset_table[i+CHRMODE_TO]=i;
				}
		}
		BOOL bit7=TRUE;
		while(0!=*text) {
			*text=charset_table[*text+mode];
			if(*text++ > 127)
				bit7=FALSE;
		}
		return bit7;
	}
}
*/
BOOL IsValidUTF8(char *buffer2)
{
	char *utf8_buffer;

	utf8_buffer = (char *)malloc(strlen(buffer2));
	memset(utf8_buffer,0,strlen(buffer2));
    memcpy(utf8_buffer,buffer2,strlen(buffer2));

    for(unsigned long count=0; count<strlen(buffer2); count++)
    {
        if(utf8_buffer[count] >= 194 && utf8_buffer[count]<223) return TRUE;
    }


	if (utf8_buffer) free(utf8_buffer);

    return FALSE;

}


char *translateCharset(unsigned char *text,char * charset)
{
	char *result = NULL;
	char *dest   = NULL;


	if (text)
	{
		if (NULL == charset)
		{
			charset = (char *)malloc(255);
			memset(charset,0,255);

			if (IsValidUTF8((char*)text)==TRUE)
			{
				strcpy(charset,"UTF-8");
			}
			else
			{
				strcpy(charset,sysCodeset->name);
			}
		}

		struct codeset *srcCodeset = CodesetsFindA(charset, NULL);

		struct TagItem CharsetTagList[] =
		{
			{CSA_Source, (ULONG)text },
			{CSA_SourceCodeset, (ULONG)srcCodeset},
			{CSA_DestCodeset, (ULONG)sysCodeset},
			{TAG_DONE, 0}
		};

		dest = CodesetsConvertStrA(CharsetTagList);
		if (dest)
		{
			result = strdup(dest);
			CodesetsFree(dest);
		}
		else
			result = strdup((char*)text);
	}
	return result;
}
/* Logging function.
 */
/*void nLog(char * text,...) {
	if((account.flags & Account::LOGGING)==0)
		return;*/
BOOL LogFunc(const char * text,...) {
	struct DateStamp ds;
	DateTime dt;
	char dateLog[32] = "";
	char timeLog[32] = "";
	const int LEN = 8192;
	static char message[LEN+1] = "";

	va_list vlist;
	if((0 != logFile) && (0 != text)) {
		va_start(vlist,text);
		message[LEN] = '\0';
		vsprintf(message,text,vlist);
		if(message[LEN] != '\0') {
			// buffer overrun!
			char err[] = "Logging buffer overflow!\n";
			printf("%s",err);
			Write(logFile,(void *)err,strlen(err));
			message[LEN] = '\0';
		}
		va_end(vlist);
		DateStamp(&ds);
		dt.dat_Stamp=ds;
		dt.dat_Format=FORMAT_CDN;
		dt.dat_Flags=NULL;
		dt.dat_StrDay=NULL;
		dt.dat_StrDate=dateLog; // puts format dd-mm-yy
		dt.dat_StrTime=timeLog;
		DateToStr(&dt);
		Write(logFile,dateLog,strlen(dateLog));
		Write(logFile,(void *)" - ",3);
		Write(logFile,timeLog,strlen(timeLog));
		Write(logFile,(void *)" : ",3);
		Write(logFile,message,strlen(message));
		FFlush(logFile); // make sure it gets to disk.
		return TRUE;
	}
	return FALSE;
}

void InitMenu(struct NewMenu *Menu,ULONG StartID)
{
	int i;

	for (i=0;Menu[i].nm_Type;i++)
		if (Menu[i].nm_Label != NM_BARLABEL)
			Menu[i].nm_Label=CatalogStr(StartID++,Menu[i].nm_Label);

}

void InitToolbar(MUIS_TheBar_Button *Toolbar,ULONG StartID)
{
	int i;

	for (i=0;Toolbar[i].img != MUIV_TheBar_End;i++)
		if (Toolbar[i].img != MUIV_TheBar_BarSpacer)
		{
			Toolbar[i].text=CatalogStr(StartID++,Toolbar[i].text);
			Toolbar[i].help=CatalogStr(StartID++,Toolbar[i].help);
		}

}
void InitArray(const char **Array,ULONG StartID)
{
	int i;

	for (i=0;Array[i];i++)
		Array[i]=CatalogStr(StartID+i,Array[i]);
}

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
