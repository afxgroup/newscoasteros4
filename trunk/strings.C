/*
    NewsCoaster
    Copyright (C) 1999-2003 Mark Harman and Pavel Fedin

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

#include "mui_headers.h"
#include <exec/types.h>
#include <string.h>
#include <ctype.h>
#include <string.h>

#include "vector.h"
#include "various.h"
#include "misc.h"
#include "strings.h"

/* Strips a newline from the end of 'line'.
 */
void StripNewLine(char * line) {
	for(;;) {
		if(*line==13 || *line==10) {
			*line=0;
			return;
		}
		if(*line==0)
			return;
		line++;
	}
}

/* Replaces a string ending in CRLF with a string that just ends in LF.
 */
void convert13to10(char * text) {
	for(;;) {
		if(*text==13) {
			text++;
			if(*text==10) {
				*(text-1)=10;
				*text=0;
				return;
			}
		}
		if(*text==0)
			break;
		text++;
	}
}

/* Copys 'src' to 'dest', but replaces the CRLF with just a LF.
 * Returns the length of the copied string.
 */
int strcpy_13to10(char *dest,char *src) {
	int p = (int)dest;
	while(*src!= 0) {
		if(*src == 13) {
			src++;
			if(*src == 10) {
				*dest++ = 10;
				break;
			}
			src--;
		}
		*dest = *src;
		dest++;
		src++;
	}
	*dest = '\0';
	return (int)dest - p;
}

/* Terminates 'text' at the first occurance of 'ch'.
 */
void StripChar(char * text,char ch) {
	char *str=strchr(text,ch);
	if(str != 0)
		*str= '\0';
}

/* Removes trailing characters from 'text' so that last character is not
 * 'ch'.
 */
void StripTrail(char * text,char ch) {
	char *ptr = &text[strlen(text)-1];
	while(ptr>=text) {
		if(*ptr == ch)
			*ptr=0;
		else
			break;
		ptr--;
	}
}

/* Returns TRUE iff the strings 'str1' and 'str2' are equal (case
 * sensitive).
 */
BOOL equals(const char *str1,const char *str2) {
	if(*str1 ==0 || *str2 ==0)
		return FALSE;
	while(*str1 == *str2) {
		if(*str1==0)
			return TRUE;
		str1++;
		str2++;
	}
	return FALSE;
}

/* Returns TRUE iff the strings 'str1' and 'str2' are equal (case
 * insensitive).
 */
BOOL iequals(const char *str1,const char *str2) {
	if(*str1 ==0 || *str2 ==0)
		return FALSE;
	while(toupper(*str1) == toupper(*str2)) {
		if(*str1 == '\0')
			return TRUE;
		str1++;
		str2++;
	}
	return FALSE;
}

/* Unless otherwise specified, the following functions use 'word' to mean a
 * sequence of non-whitespace characters seperated by one or more
 * whitespace (as defined by isspace()) characters.
 */

/* Copies the first word of 'buffer' to 'dest'.
 */
void wordFirst(char * dest,const char * buffer) {
	while(*buffer!= 0 && isspace(*buffer))
		buffer++;
	if(*buffer==0) {
		*dest = '\0';
		return;
	}
	while(*buffer!= 0 && !isspace(*buffer)) {
		*dest = *buffer;
		dest++;
		buffer++;
	}
	*dest= '\0';
}

/* Copies the first word of 'buffer' to 'dest'.
 * Returns the length of 'dest'.
 */
int wordFirstAndLen(char * dest,const char * buffer) {
	//nLog("wordFirstAndLen((char *)%d,(char *)%s) called\n",dest,buffer);
	char * ptr = dest;
	while(*buffer!= 0 && isspace(*buffer))
		buffer++;
	if(*buffer==0) {
		*dest = '\0';
		return 0;
	}
	while(*buffer!= 0 && !isspace(*buffer)) {
		*dest = *buffer;
		dest++;
		buffer++;
	}
	*dest = '\0';
	return ((int)dest - (int)ptr);
}

/* Copies the first word of 'buffer' to 'dest', converting it to upper
 * case.
 */
void wordFirstUpper(char * dest,const char * buffer) {
	while(*buffer!= 0 && isspace(*buffer))
		buffer++;
	if(*buffer==0) {
		*dest = '\0';
		return;
	}
	while(*buffer!= 0 && !isspace(*buffer)) {
		*dest = toupper(*buffer);
		dest++;
		buffer++;
	}
	*dest = '\0';
}

/* Copies the first word of 'buffer' to 'dest', converting it to upper
 * case.
 * Returns the length of 'dest'.
 */
int wordFirstAndLenUpper(char * dest,char * buffer) {
	//nLog("wordFirstAndLenUpper((char *)%d,(char *)%d:%s) called\n",dest,buffer,buffer);
	char *ptr = dest;
	while(*buffer != '\0' && isspace(*buffer))
		buffer++;
	/*if(*buffer == '\0') {
		*dest = '\0';
		return 0;
	}*/
	while(*buffer != '\0' && !isspace(*buffer)) {
		*dest = toupper(*buffer);
		dest++;
		buffer++;
	}
	*dest = '\0';
	return ((int)dest - (int)ptr);
}

/* Returns a pointer to the last word.
 */
char *wordLast(char *buffer) {
	if( *buffer == '\0' )
		return buffer;
	char *last = buffer;
	char *ptr = buffer;
	ptr++;
	while( *ptr != '\0' ) {
		if( isspace(ptr[-1]) && !isspace(ptr[0]) )
			last = ptr;
		ptr++;
	}
	return last;
}

/* Copies the 'n'th word (ie, starting from n==1) in 'str' to 'word'.
 */
void word(char *word,char *str,int n) {
	for(int i=1;i<n;i++) {
		while( *str!= 0 && isspace(*str) ) // spaces
			str++;
		while( *str!= 0 && !isspace(*str) ) // non-spaces
			str++;
		if( *str ==0 ) {
			*word = '\0';
			return;
		}
	}
	while( *str!= 0 && isspace(*str) ) // spaces
		str++;
	while( *str!= 0 && !isspace(*str) ) { // copy word
		*word = *str;
		word++;
		str++;
	}
	*word = '\0';
}

/* Copies the 'n'th "word" (ie, starting from n==1) in 'str' to 'word',
 * where words are considered to be strings seperated by characters 'ws'.
 */
void word(char *word,char *str,int n,char ws) {
	nLog("%d::%s\n",n,str);
	char *tptr = word;
	for(int i=1;i<n;i++) {
		while( *str!= 0 && *str==ws ) // spaces
			str++;
		while( *str!= 0 && *str!=ws ) // non-spaces
			str++;
		if( *str ==0 ) {
			*word = '\0';
			return;
		}
	}
	while( *str!= 0 && *str==ws ) // spaces
		str++;
	while( *str!= 0 && *str!=ws ) { // copy word
		*word = *str;
		word++;
		str++;
	}
	*word = '\0';
	nLog("%s\n",tptr);
}

/* Returns a pointer to the start of the 'n'th word (ie, starting from
 * n==1) in 'str'.
 * Returns NULL if there are less than 'n' words.
 */
char * wordStart(char *str,int n) {
	for(int i=1;i<n;i++) {
		while( *str!= 0 && isspace(*str) ) // spaces
			str++;
		while( *str!= 0 && !isspace(*str) ) // non-spaces
			str++;
		if( *str ==0 ) {
			return NULL;
		}
	}
	while( *str!= 0 && isspace(*str) ) // spaces
		str++;
	return str;
}

/* Extract either the name or email from 'email' and copy to 'out'. 'type'
 * can be:
 * GETEMAIL_NAME      - return the name
 * GETEMAIL_NAMEEMAIL - return the name, or if there is no name, return the email
 * GETEMAIL_EMAIL     - return the email
 */
void get_email(char * out,const char * email,GetEmailType type) {
	STRPTR t1 = NULL;
	char word1[256] = "";
	int t2 = 0;
	if(type==GETEMAIL_NAME || type==GETEMAIL_NAMEEMAIL) {
		if(email[0]=='<' || email[1]=='<') {
			if(email[strlen(email)-1]=='>') {
				// no name
				if(type==GETEMAIL_NAME) {
					out[0] = '\0';
					return;
				}
			}
			else {
				// email is first
				t1=strchr(email,'>');
				if(t1==0) {
					out[0]= '\0';
					return;
				}
				t2=strlen(email)+(int)email-(int)t1+3;
				strncpy(out,t1+2,t2);
				out[t2]= '\0';
				return;
			}
		}
		else {
			t1=strchr(email,'<');
			if(t1==0)
				t2=strlen(email)+1;
			else
				t2=(int)t1-(int)email;
			strncpy(out,email,t2-1);
			out[t2-1]= '\0';
			return;
		}
	}
	// get email
	t1=strchr(email,'<');
	if(t1==0) {
		wordFirst(word1,email);
		strcpy(out,word1);
		return;
	}
	t2=(int)t1-(int)email;
	int t3 = strlen(email) - t2 - 2;
	if(t3 > 0) {
		strncpy(out,t1+1,t3);
		out[t3]= '\0';
	}
	else
		*out = '\0';
	return;
}

/* Converts 'text' to upper case.
 */
void toUpper(char *text) {
	while( (*text = toupper(*text)) != '\0')
		text++;
}

/* Similar to strstr, but the search is case insensitive.
 */
char * stristr(const char * src,const char * sub) {
	if(*sub==0)
		return (char *)src;

	char * pos = NULL;
	const char * subptr = sub;
	for(char *p=(char *)src; *p != '\0' ;) {
		if(toupper(*p) == toupper(*subptr)) {
			p++;
			subptr++;
			if(*subptr == '\0') {
				pos = &p[ (int)sub - (int)subptr ];
				break;
			}
		}
		else if(subptr != sub)
			subptr = sub;
		else
			p++;
	}
	return pos;
}

/* Similar to stristr, but it assumes that 'sub' is in upper case.
 */
char * stristr_q(const char * src,const char * sub) {
	if(*sub==0)
		return (char *)src;

	char * pos = NULL;
	const char * subptr = sub;
	for(char *p=(char *)src; *p != '\0' ;) {
		if(toupper(*p) == *subptr) {
			p++;
			subptr++;
			if(*subptr == '\0') {
				pos = &p[ (int)sub - (int)subptr ];
				break;
			}
		}
		else if(subptr != sub)
			subptr = sub;
		else
			p++;
	}
	return pos;
}

/* Like stricmp, but cmp must be at the end of txt.
 */
int stricmpe(const char *txt,const char *cmp) {
	int txt_len = strlen(txt);
	int cmp_len = strlen(cmp);
	return stricmp(&txt[txt_len - cmp_len],cmp);
}

/* Replaces all Escape characters in the string with a space.
 * This is due to a security flaw when devices such as APIPE: are mounted,
 * allowing code to be executed by displaying malicious text in an MUI Text
 * object (or subclass).
 * See http://www.abraxis.com/SA-2001-11-08.html for more details.
 */
void stripEscapes(char * text) {
	while(*text != '\0') {
		if(*text == 27)
			*text = ' ';
		text++;
	}
	/*for(int i=0;i<length;i++) {
		if(*text == 27 || *text == '\0')
			*text = ' ';
		text++;
	}*/
}

void stripWhitespace(char * text) {
	while(*text != '\0') {
		if(isspace(*text))
			*text = ' ';
		text++;
	}
}

void translateIso(char *dest,const char *src) {
	char *dest_save;
	char charset[100];
	char *translated_string = NULL;

	dest_save=dest;
	strcpy (charset,account.charset_write);
	while(*src != '\0') {
		if(src[0] == '=' && src[1] == '?') {
			char *qu_1 = strchr(&src[2],'?');
			char *qu_2 = NULL;
			char *end = NULL;
			if(qu_1 != 0)
				qu_2 = strchr(&qu_1[1],'?');
			if(qu_2 != 0)
				end = strstr(&qu_2[1],"?=");
			if(end != 0) {
				strncpy(charset,&src[2],(int)(qu_1-src-2));
				charset[(int)(qu_1-src-2)]=0;
				char enc = qu_1[1];
				src = &qu_2[1];
				/*while(src < end) {
					*dest++ = *src++;
				}*/

				int len = (int)end - (int)src;
				if(len > 0) {
					if(enc == 'q' || enc == 'Q') {
						int dlen = decode_qprint(src, dest, len, TRUE);
						dest += dlen;
					}
					else if(enc == 'b' || enc == 'B') {
						int dlen = decode_base64(src, dest, len);
						dest += dlen;
					}
					else {
						while(src < end) {
							*dest++ = *src++;
						}
					}
				}
				src = &end[2];
			}
			else {
				*dest++ = *src++;
				*dest++ = *src++;
			}
		}
		else
			*dest++ = *src++;
	}
	*dest = '\0';
	for (unsigned int i=0;i<strlen(charset);i++)
		charset[i]=toupper(charset[i]);

	charset[strlen(charset)]='\0';
	translated_string=translateCharset((unsigned char *)dest_save,charset);
	if (translated_string)
		dest_save = translated_string;
}

