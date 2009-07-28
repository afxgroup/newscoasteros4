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

#ifndef __amigaos4__
#include <clib/amigaguide_protos.h>
#include <clib/asl_protos.h>
#include <clib/dos_protos.h>
#endif
#include <proto/amigaguide.h>
#include <proto/asl.h>
#include <proto/dos.h>

#include <stdio.h>
#include <string.h>

#include "vector.h"
#include "general.h"

/* Opens the AmigaGuide documentation at a specific node and line.
 */

/*for os 4 only */
/* I don't think this is auto opened */
#ifdef __amigaos4__
struct AmigaGuideIFace *IAmigaGuide = NULL;
#endif

BOOL ShowGuide(STRPTR node,LONG line) {
	struct NewAmigaGuide nag = {NULL};
	AMIGAGUIDECONTEXT handle;
	nag.nag_Name = (CONST STRPTR)"PROGDIR:NewsCoaster.Guide";
	nag.nag_Node = node;
	nag.nag_Line = line;


	handle = OpenAmigaGuide(&nag,NULL);
	if (handle)
	{
		CloseAmigaGuide(handle);
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/* Opens an ASL requester.
 */
BOOL LoadASL(char *filename,const char *title,const char *ifile,const char *pattern,BOOL folders) {
	static char current_dir[300] = "";
	BOOL ok=FALSE;
	struct FileRequester * freq=NULL;
	freq = (struct FileRequester *)AllocAslRequest(ASL_FileRequest,NULL);
	if (freq) {
		if(AslRequestTags(freq,
								ASLFR_TitleText,			(ULONG)title,
								ASLFR_Window,				0,
								ASLFR_Flags1,				FRF_DOPATTERNS,
								ASLFR_InitialFile,		(ULONG)ifile,
								ASLFR_InitialDrawer,		(ULONG)current_dir,
								ASLFR_InitialPattern,	(ULONG)pattern,
								ASLFR_DrawersOnly,		folders,
								/*ASLFR_UserData,			app,
								ASLFR_IntuiMsgFunc,		&IntuiMsgHook,*/
								TAG_DONE)) {
			strcpy(current_dir,freq->fr_Drawer);

			strcpy(filename,freq->fr_Drawer);
			if(AddPart(filename,freq->fr_File,MAXFILENAME))
				ok=TRUE;
		}
		FreeAslRequest(freq);
	}
	return ok;
}
