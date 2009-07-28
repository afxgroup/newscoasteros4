#ifndef __CODESETS_H__
#define __CODESETS_H__
// FROM YAM
#include <proto/codesets.h>

#define SIZE_CTYPE      40

static char LocalCharset[SIZE_CTYPE+1];

static struct codesetList 	*codesetsList = NULL; 	//for codesets - GLOBAL variable
static struct codeset 		*sysCodeset   = NULL;	//for codesets - GLOBAL variable

/// TrimStart
//  Strips leading spaces
char *TrimStart(char *s);
char *TrimEnd(char *s);
char *Trim(char *s);

// codesets.library helper functions
char *strippedCharsetName(const struct codeset* codeset);

#endif //__CODESETS_H__
