void StripNewLine(char * line);
void convert13to10(char * text);
int strcpy_13to10(char *dest,char *src);
void StripChar(char * text,char ch);
void StripTrail(char * text,char ch);
BOOL equals(const char *str1,const char *str2);
BOOL iequals(const char *str1,const char *str2);
void wordFirst(char * dest,const char * buffer);
int wordFirstAndLen(char * dest,const char * buffer);
void wordFirstUpper(char * dest,const char * buffer);
int wordFirstAndLenUpper(char * dest,char * buffer);
char *wordLast(char *buffer);
void word(char *word,char *str,int n);
void word(char *word,char *str,int n,char ws);
char * wordStart(char *str,int n);

enum GetEmailType {
	GETEMAIL_NAME      = 0,
	GETEMAIL_NAMEEMAIL = 1,
	GETEMAIL_EMAIL     = 2
};
void get_email(char * out,const char * email,GetEmailType type);

void toUpper(char *text);
char * stristr(const char * src,const char * sub);
char * stristr_q(const char * src,const char * sub);
int stricmpe(const char *txt,const char *cmp);
void stripEscapes(char * text);
void stripWhitespace(char * text);
void translateIso(char *dest,const char *src);
