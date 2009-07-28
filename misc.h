//#include <mui/Toolbar_mcc.h>
#include <mui/TheBar_mcc.h>

/* Contains information about a uuencoded attachment.
 */
const int UUENCODEDATA_FILENAME_LEN = 299;
const int UUENCODEDATA_EXT_LEN = 63;
struct uuEncodeData {
	char filename[UUENCODEDATA_FILENAME_LEN+1];
	char ext[UUENCODEDATA_EXT_LEN+1]; // extension, not MIME Type!
};

// Modes for translateCharset()
#define CHRMODE_FROM 0
#define CHRMODE_TO 256

// A macro to simplify getting a string from catalog
/*
In the beginning i've defined it as:

#define CatalogStr(x) GetCatalogStr(nc_Catalog,x,x##_STR)

But StormC seems to process x##_STR part in a wrong way. I've tried GNU cpp
preprocessor, everything went fine with it. Unfortunately i can't figure out
what's actually wrong in StormC because i can't see preprocessor's output there.
Big thanks for that to H&P! :-((((((
*/
#define CatalogStr(x,s) ((CONST STRPTR)GetCatalogStr(nc_Catalog,x,(const char *)s))
// #defines for CatComp
#define CATCOMP_NUMBERS
#define CATCOMP_STRINGS

void Readln(BPTR file,char *buffer);
int decode_qprint(const char *in,char *out,int length,BOOL nl);
void parse_html(char *in,char *out,int length,BOOL reset);
int decode_base64(const char *in,char *out,int length);
int encode_base64(char *in,char *out,int length);
int ydecode(BOOL *corrupt,char *in,char *out,int length,uuEncodeData *uudata);
int uudecode(char *in,char *out,int length,uuEncodeData *uudata);
void getMIMEType(char *type,const char *ext);
char *translateCharset(unsigned char *text,char *charset);

void tests(int major,int minor);

void InitMenu(struct NewMenu *Menu,ULONG StartID);
void InitToolbar(MUIS_TheBar_Button *Toolbar,ULONG StartID);
void InitArray(const char **Array,ULONG StartID);

BOOL FileCopy(const char *FileIn, const char *FileOut);

/* A #define so we only call the logging function if logging is enabled.
 * Defining with an 'if' would make more sense, but it seemed to cause
 * weird problems, hence it is done with the '&&'.
 */
extern BOOL logbool;
#define nLog logbool = ((account.flags & Account::LOGGING)!=0) && LogFunc
BOOL LogFunc(const char * text,...);

/* The log file handle. */
extern BPTR logFile;
