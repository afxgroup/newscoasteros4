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

#include <proto/locale.h>

#include "mui_headers.h"

#include "vector.h"
#include "various.h"
#include "main.h"
#include "misc.h"
#include "statuswindow.h"
#include "subthreads.h"

#include "newscoaster_catalog.h"

/* A safe way to delete the class from a Hook called with an object
 * belonging to the window. We can't do a 'delete statusWindow' or whatever from
 * within the Hook, since the destructor calls MUI_DisposeObject on the MUI
 * objects associated with the class. Instead, use:
 *     DoMethod(app, MUIM_Application_PushMethod, app, 3, MUIM_CallHook,
 *         &disposeHook, statusWindow );
 */
void StatusWindow::disposeFunc(StatusWindow **stsW)
{
	nLog("StatusWindow::disposeFunc() called\n");
	StatusWindow * statusWindow = *stsW;
	delete statusWindow;
}

void StatusWindow::abortFunc(StatusWindow **stsW)
{
	StatusWindow * statusWindow = *stsW;
	statusWindow->setAborted(TRUE);
#ifndef DONT_USE_THREADS
	if( statusWindow->is_subthread )
	{
		printf("about to abort..\n");
		thread_abort(NULL);
		printf("aborted!\n");
	}
#endif
}

StatusWindow::StatusWindow(Object *app,const char * title)
{
	init(title, FALSE);
}

StatusWindow::StatusWindow(Object *app,const char * title,BOOL quiet)
{
	init(title, quiet);
}

void StatusWindow::init(const char * title,BOOL quiet)
{
	this->aborted = FALSE;
	this->is_subthread = FALSE;

	wnd = WindowObject,
		MUIA_Window_Title,			title,
		MUIA_Window_ID,				MAKE_ID('N','E','W','S'),
		MUIA_Window_CloseGadget,	FALSE,
		MUIA_Window_Activate,		!quiet,

		WindowContents, GROUP = VGroup,
			Child, TXT_INFO=TextObject,
				GroupFrame,
				MUIA_Group_Horiz, TRUE,
				MUIA_Text_Contents, "",
				MUIA_Text_PreParse, "\33c",
				MUIA_Text_SetMin, TRUE,
			End,
			Child, BT_ABORT=SimpleButton(CatalogStr(MSG_ABORT,MSG_ABORT_STR)),
			End,
		End;
	DoMethod(app,OM_ADDMEMBER,wnd);
	DoMethod(BT_ABORT,MUIM_Notify,MUIA_Pressed,FALSE,
		app,4,MUIM_CallHook,&hook_standard,abortFunc,this);
	set(wnd,MUIA_Window_Open,TRUE);
}

StatusWindow::~StatusWindow()
{
	set(wnd,MUIA_Window_Open,FALSE);
	DoMethod(app,OM_REMMEMBER,wnd);
	MUI_DisposeObject(wnd);
}

void StatusWindow::setTitle(const char * t)
{
	nLog("StatusWindow::setTitle((char *)%s) called\n",t);
	set(wnd,MUIA_Window_Title,t);
}

void StatusWindow::setText(const char * t)
{
	nLog("StatusWindow::setText((char *)%s) called\n",t);
	strncpy(text,t,TXTLEN);
	text[TXTLEN] = '\0';
#ifdef DONT_USE_THREADS
	set(TXT_INFO,MUIA_Text_Contents,text);
#else
	/* We can only access MUI from the main task, not from the subthread. */
	if( this->is_subthread )
	{
		nLog("AAA\n");
		DoMethod(app, MUIM_Application_PushMethod, TXT_INFO, 3, MUIM_Set, MUIA_Text_Contents, text );
		nLog("BBB\n");
	}
	else
		set(TXT_INFO,MUIA_Text_Contents,text);
#endif
	/*if(DoMethod(GROUP,MUIM_Group_InitChange))
		DoMethod(GROUP,MUIM_Group_ExitChange);*/
	nLog("    done\n");
}

void StatusWindow::resize()
{
	if(DoMethod(GROUP,MUIM_Group_InitChange))
		DoMethod(GROUP,MUIM_Group_ExitChange);
}

void StatusWindow::setVisible(BOOL v)
{
	set(wnd,MUIA_Window_Open,v);
}

void StatusWindow::setAborted(BOOL aborted)
{
	this->aborted=aborted;
}

BOOL StatusWindow::isAborted()
{
	return aborted;
}
