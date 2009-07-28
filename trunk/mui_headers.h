/* Includes and other common stuff for MUI.
 * This is modified from the file "demo.h" that comes with the example MUI
 * programs.
 */

#define DONT_USE_THREADS

#ifdef __GNUC__
#define NO_INLINE_STDARG
#endif

/* MUI */
#include <libraries/mui.h>
// set() and get() are not defined by the mui.h in C++ mode
#define get(obj,attr,store) GetAttr(attr,obj,(ULONG *)store)
#define set(obj,attr,value) SetAttrs(obj,attr,value,TAG_DONE)
// Standard HOOKFUNC definition does not work with gcc on AmigaOS
struct CPPHook
{
    struct MinNode h_MinNode;
    ULONG	   (*h_Entry)(...);	/* assembler entry point */
    ULONG	   (*h_SubEntry)(...);	/* often HLL entry point */
    APTR	   h_Data;		/* owner specific	 */
};
typedef unsigned long (*CPPHOOKFUNC)(...);

/* Prototypes */
#include <clib/alib_protos.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/icon.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <proto/gadtools.h>
#include <proto/utility.h>
#include <proto/asl.h>

#include <proto/muimaster.h>
#include <proto/codesets.h>

/* ANSI C */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Compiler specific stuff */

#ifdef _DCC

#define REG(x) __ ## x
#define ASM
#define SAVEDS __geta4

#else

#ifndef __amigaos4__
#define REG(x) register __## x
#endif
#define ASM
#define SAVEDS

#ifdef __SASC
#include <pragmas/exec_sysbase_pragmas.h>
#endif /* ifdef SASC      */

#endif /* ifdef _DCC */

/*************************/
/* Init & Fail Functions */
/*************************/

static VOID fail(Object *app,const char *str) {
	if( (0 != str) && (0 != *str) )
		printf("FAIL! : %s",str);
	if (app)
		MUI_DisposeObject(app);
#ifdef __amigaos4__
	if(IMUIMaster) {
		DropInterface((struct Interface *)IMUIMaster);
	}
#endif

	if (MUIMasterBase) {
		CloseLibrary(MUIMasterBase);
		MUIMasterBase=0;
	}
}


static VOID init(VOID) {
	if (!(MUIMasterBase = OpenLibrary(MUIMASTER_NAME,MUIMASTER_VMIN)))
		fail(NULL,"Failed to open "MUIMASTER_NAME".");
#ifdef __amigaos4__
	if(!(IMUIMaster=(struct MUIMasterIFace *)GetInterface(MUIMasterBase,"main", 1, NULL)))
		fail(NULL,"Failed to open interface to "MUIMASTER_NAME".");
#endif
}

#ifndef MAKE_ID
#define MAKE_ID(a,b,c,d) ((ULONG) (a)<<24 | (ULONG) (b)<<16 | (ULONG) (c)<<8 | (ULONG) (d))
#endif

#define BetterString(contents,maxlen)\
   BetterStringObject,\
      StringFrame,\
      MUIA_String_MaxLen  , maxlen,\
      MUIA_String_Contents, contents,\
      MUIA_CycleChain, 1, \
      End
