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

#include "mui_headers.h"
#include <mui/NFloattext_mcc.h>
#include <mui/NListview_mcc.h>
#include <mui/TextEditor_mcc.h>
#include <mui/BetterString_mcc.h>
#include <proto/locale.h>

#include <ctype.h>

#include "vector.h"
#include "various.h"
#include "writewindow.h"
#include "main.h"
#include "misc.h"
#include "indices.h"
#include "datehandler.h"
#include "general.h"
#include "strings.h"
#include "netstuff.h"
#include "lists.h"
#include "newscoaster_catalog.h"

//static LONG w_cmap[9];

static const char *Pages_write[]   = {"Message","Attachments","Options",NULL };

BOOL WriteWindow::save(BOOL force) {
	//printf("save!\n");
	WriteWindow *ww = this;
	GroupData *gdata = NULL;
	get_gdata(&gdata,-1);
	MessageListData *mdata = write_message(TRUE,FALSE,force);
	if(mdata) {
		if(ww->newmessage.edit==FALSE)
			ww->newmessage.sent=FALSE;
		ww->newmessage.edit=TRUE;
		ww->newmessage.reply=FALSE;
		//get_refs(&(ww->newmessage),gdata,mdata);
		get_refs(&(ww->newmessage),gdata,mdata,GETREFS_ALL);
		//printf("last ref: %s\n",ww->newmessage.references[ww->newmessage.nrefs-1]);

		strncpy(ww->newmessage.messageID,mdata->messageID,NEWMESS_short);
		ww->newmessage.messageID[NEWMESS_short] = '\0';
		//delete mdata;
		return TRUE;
	}
	return FALSE;
}

HOOKCL3( ULONG, WriteWindow::TextEditor_Dispatcher_write, IClass *, cl, a0, Object *, obj, a2, struct MUIP_TextEditor_HandleError *, msg, a1 )

	switch(msg->MethodID) {
		/*case MUIM_Show:
			break;
		case MUIM_Hide:
			break;*/
		case MUIM_DragQuery:
			return FALSE;
			break;
		/*case MUIM_DragDrop:
			drop_msg = (MUIP_DragDrop *)msg;
			ULONG active;
			if(GetAttr(MUIA_List_Active, drop_msg->obj, &active))
				DoMethod(obj, MUIM_TextEditor_InsertText, StdEntries[active]);
			break;*/
		case MUIM_TextEditor_HandleError:
			char *errortxt = NULL;
			switch(msg->errorcode) {
				case Error_ClipboardIsEmpty:
					errortxt = CatalogStr(MSG_CLIPBOARD_EMPTY,MSG_CLIPBOARD_EMPTY_STR);
					break;
				case Error_ClipboardIsNotFTXT:
					errortxt = CatalogStr(MSG_NO_TEXT_IN_CLIP,MSG_NO_TEXT_IN_CLIP_STR);
					break;
				case Error_MacroBufferIsFull:
					break;
				case Error_MemoryAllocationFailed:
					break;
				case Error_NoAreaMarked:
					errortxt = CatalogStr(MSG_NO_AREA_MARKED,MSG_NO_AREA_MARKED_STR);
					break;
				case Error_NoMacroDefined:
					break;
				case Error_NothingToRedo:
					errortxt = CatalogStr(MSG_NOTHING_TO_REDO,MSG_NOTHING_TO_REDO_STR);
					break;
				case Error_NothingToUndo:
					errortxt = CatalogStr(MSG_NOTHING_TO_UNDO,MSG_NOTHING_TO_UNDO_STR);
					break;
				case Error_NotEnoughUndoMem:
					errortxt = CatalogStr(MSG_NO_MEMORY_FOR_UNDO,MSG_NO_MEMORY_FOR_UNDO_STR);
					break;
				case Error_StringNotFound:
					break;
				case Error_NoBookmarkInstalled:
					errortxt = CatalogStr(MSG_NO_BOOKMARK_INSTALLED,MSG_NO_BOOKMARK_INSTALLED_STR);
					break;
				case Error_BookmarkHasBeenLost:
					errortxt = CatalogStr(MSG_BOOKMARK_LOST,MSG_BOOKMARK_LOST_STR);
					break;
			}
			if(errortxt) {
				MUI_Request(app, NULL, 0L, NULL, CatalogStr(MSG_CONTINUE,MSG_CONTINUE_STR), errortxt);
			}
			break;
	}
	return DoSuperMethodA(cl,obj,(Msg)msg);
}

void WriteWindow::updateMimeType(const char *mimetype)
{
	nLog("WriteWindow::updateMimeType((char *)%s) called\n",mimetype);
	ULONG entries = 0;
	MIMEType * mime = NULL;
	get(NLIST_write_ATT,MUIA_NList_Entries,&entries);
	if(entries>0)
	{
		ULONG val = 0;
		get(NLIST_write_ATT,MUIA_NList_Active,&val);
		if(val!=MUIV_NList_Active_Off)
		{
			DoMethod(NLIST_write_ATT,MUIM_NList_GetEntry,MUIV_NList_GetEntry_Active,&mime);
			strncpy(mime->type,mimetype,MIMETYPE_TYPE_LEN);
			mime->type[MIMETYPE_TYPE_LEN] = '\0';
			DoMethod(NLIST_write_ATT,MUIM_NList_ReplaceSingle,mime,MUIV_NList_Insert_Active,NOWRAP,ALIGN_LEFT);
			DoMethod(NLIST_write_ATT,MUIM_NList_Redraw,MUIV_NList_Redraw_Active);
		}
	}
}

//HOOKCL3( LONG, WriteWindow::mimeTypeFunc, CPPHook *, hook, a0, Object *, object, a2, WriteWindow **, W, a1 )
void WriteWindow::mimeTypeFunc(WriteWindow **W) {
	char *s = NULL;
	WriteWindow * ww = *W;
	//get(object,MUIA_String_Contents,&s);
	get(ww->STR_write_MIME,MUIA_String_Contents,&s);
	ww->updateMimeType(s);
	//return(TRUE);
}

HOOKCL2( VOID, WriteWindow::attObjStrFunc, Object *, pop, a2, Object *, str, a1 )

	char *x = NULL;
	DoMethod(pop,MUIM_List_GetEntry,MUIV_List_GetEntry_Active,&x);
	set(str,MUIA_String_Contents,x);

	WriteWindow *ww = NULL;
	int size=ptrs->getSize();
	WriteWindow ** data = (WriteWindow **)ptrs->getData();
	for(int i=0;i<size;i++) {
		if(data[i]->LIST_write_MIME == pop) {
			ww = data[i];
			break;
		}
	}
	if(ww == NULL) {
		nLog("Error - can't find WriteWindow for PopObject!\n");
		return;
	}
	ww->updateMimeType(x);
	/*int entries = 0;
	MIMEType * mime = NULL;
	get(ww->NLIST_write_ATT,MUIA_NList_Entries,&entries);
	if(entries>0) {
		int val = 0;
		get(ww->NLIST_write_ATT,MUIA_NList_Active,&val);
		if(val!=MUIV_NList_Active_Off) {
			DoMethod(ww->NLIST_write_ATT,MUIM_NList_GetEntry,MUIV_NList_GetEntry_Active,&mime);
			strncpy(mime->type,x,MIMETYPE_TYPE_LEN);
			mime->type[MIMETYPE_TYPE_LEN] = '\0';
			DoMethod(ww->NLIST_write_ATT,MUIM_NList_ReplaceSingle,mime,MUIV_NList_Insert_Active,NOWRAP,ALIGN_LEFT);
			DoMethod(ww->NLIST_write_ATT,MUIM_NList_Redraw,MUIV_NList_Redraw_Active);
		}
	}*/
}

//HOOKCL3( VOID, WriteWindow::addNGFunc, CPPHook *, hook, a0, Object *, object, a2, WriteWindow **, W, a1 )
void WriteWindow::addNGFunc(void **args) {
	//WriteWindow * ww = *W;
	WriteWindow * ww = (WriteWindow *)args[0];
	Object *object = (Object *)args[1];
	if(object == ww->BT_write_NG)
		getNGChoice(CatalogStr(MSG_PLEASE_SELECT_NEWSGROUP,MSG_PLEASE_SELECT_NEWSGROUP_STR),CatalogStr(MSG_ADD_REMOVE_GROUP,MSG_ADD_REMOVE_GROUP_STR),ww->STR_write_NG,FALSE);
	else if(object == ww->BT_write_FU)
		getNGChoice(CatalogStr(MSG_PLEASE_SELECT_NEWSGROUP,MSG_PLEASE_SELECT_NEWSGROUP_STR),CatalogStr(MSG_ADD_REMOVE_GROUP,MSG_ADD_REMOVE_GROUP_STR),ww->STR_write_FOLLOWUP,TRUE);
	//return 0;
}

/* A safe way to delete the class from a Hook called with an object
 * belonging to the window. We can't do a 'delete ww' or whatever from
 * within the Hook, since the destructor calls MUI_DisposeObject on the MUI
 * objects associated with the class. Instead, use:
 *     DoMethod(app, MUIM_Application_PushMethod, app, 3, MUIM_CallHook,
 *         &disposeHook, ww );
 */
//HOOKCL3( LONG, WriteWindow::disposeFunc, CPPHook *, hook, a0, Object *, object, a2, WriteWindow **, W, a1 )
void WriteWindow::disposeFunc(WriteWindow **W) {
	nLog("WriteWindow::disposeFunc() called\n");
	WriteWindow * ww = *W;
	delete ww;
}

HOOKCL3( LONG, WriteWindow::completeFunc, CPPHook *, hook, a0, Object *, object, a2, WriteWindow **, W, a1 )
/*void WriteWindow::completeFunc(void **args) {
	WriteWindow * ww = (WriteWindow *)args[0];
	Object *object = (Object *)args[1];*/
	/*WriteWindow * ww = (WriteWindow *)p[0];
	int ID = *((int *)p[1]);
	printf("%d\n",ID);*/
	WriteWindow * ww = *W;
	int result;
	BOOL close = FALSE;
	if(object == ww->BT_write_P) {
		/*if(*getstr(ww->STR_write_SUBJECT)==0) {
			MUI_RequestA(app,0,0,"Write Message","_Okay","You must specify a subject\nfor your message!",0);
			set(ww->wnd,MUIA_Window_ActiveObject,ww->STR_write_SUBJECT);
		}
		else*/
		{
			MessageListData *mdata = ww->write_message(FALSE,TRUE,FALSE);
			if(mdata) {
				//delete mdata;
				set(ww->wnd,MUIA_Window_Open,FALSE);
				close = TRUE;
				DoMethod(ww->ED_write_MESS,MUIM_TextEditor_ClearText);
				postnews(1);
			}
		}
	}
	else if(object == ww->BT_write_PL) {
		/*if(*getstr(ww->STR_write_SUBJECT)==0)
			MUI_RequestA(app,0,0,"Write Message","_Okay","You must specify a subject\nfor your message!",0);
		else*/
		{
			MessageListData *mdata = ww->write_message(FALSE,FALSE,FALSE);
			if(mdata) {
				//delete mdata;
				set(ww->wnd,MUIA_Window_Open,FALSE);
				close = TRUE;
				DoMethod(ww->ED_write_MESS,MUIM_TextEditor_ClearText);
			}
		}
	}
	else if(object == ww->BT_write_H) {
		/*if(*getstr(ww->STR_write_SUBJECT)==0)
			MUI_RequestA(app,0,0,"Write Message","_Okay","You must specify a subject\nfor your message!",0);
		else*/
		{
			MessageListData *mdata = ww->write_message(TRUE,FALSE,FALSE);
			if(mdata) {
				//delete mdata;
				set(ww->wnd,MUIA_Window_Open,FALSE);
				close = TRUE;
				DoMethod(ww->ED_write_MESS,MUIM_TextEditor_ClearText);
			}
		}
	}
	else if(object == ww->BT_write_C || object == ww->wnd) {
		result=MUI_RequestA(app,0,0,CatalogStr(MSG_CANCEL,MSG_CANCEL_STR),CatalogStr(MSG_DISCARD_OR_CANCEL,MSG_DISCARD_OR_CANCEL_STR),CatalogStr(MSG_DISCARD_CHANGES_ARE_YOU_SURE,MSG_DISCARD_CHANGES_ARE_YOU_SURE_STR),0);
		if(result==1) {
			set(ww->wnd,MUIA_Window_Open,FALSE);
			close = TRUE;
			DoMethod(ww->ED_write_MESS,MUIM_TextEditor_ClearText);
			set(ww->STR_write_SUBJECT,MUIA_String_Contents,"");
			for(int k=0;k<ww->newmessage.nrefs;k++) {
				if(ww->newmessage.references[k]) {
					delete ww->newmessage.references[k];
					ww->newmessage.references[k]=NULL;
				}
				ww->newmessage.nrefs=0;
			}
		}
	}
	if(close) {
		DoMethod(app, MUIM_Application_PushMethod, app, 4, MUIM_CallHook, &hook_standard, disposeFunc, ww );
	}
	return close ? 1 : 0;
}

//HOOKCL3( LONG, WriteWindow::rot13Func, CPPHook *, hook, a0, Object *, object, a2, WriteWindow **, W, a1 )
void WriteWindow::rot13Func(WriteWindow **W) {
	WriteWindow * ww = *W;
	rot13(ww->ED_write_MESS);
	//return 0;
}

//HOOKCL3( LONG, WriteWindow::readFunc, CPPHook *, hook, a0, Object *, object, a2, WriteWindow **, W, a1 )
void WriteWindow::readFunc(WriteWindow **W) {
	static char buffer[1024] = "";
	WriteWindow * ww = *W;
	if(LoadASL(buffer,CatalogStr(MSG_SELECT_FILE_TO_READ,MSG_SELECT_FILE_TO_READ_STR),(const char *)"",(const char *)"#?",FALSE))
		readInText(ww->ED_write_MESS,buffer);
	//return 0;
}

//HOOKCL3( LONG, WriteWindow::cysigFunc, CPPHook *, hook, a0, Object *, object, a2, WriteWindow **, W, a1 )
void WriteWindow::cysigFunc(WriteWindow **W) {
	//printf("ARSE\n");
	BOOL ed_quiet;
	WriteWindow * ww = *W;
	ULONG val = 0;
	char *temp=NULL,*temp2=NULL;
	get(ww->CY_write_SIG,MUIA_Cycle_Active,&val);
	val--;
	set(ww->ED_write_MESS,MUIA_TextEditor_ExportWrap,0);
	temp=(char *)DoMethod(ww->ED_write_MESS,MUIM_TextEditor_ExportText);
	if(temp)
	{
		temp2=temp;
		for(;;) {
			temp2=strstr(temp2,"-- ");
			if(temp2==NULL)
				break;
			if(temp2==temp)
				break;
			if(temp2[-1]=='\n' && temp2[3]=='\n')
				break;
			temp2++;
		}
		if(temp2) {
			if(temp2[-1]=='\n')
				temp2--;
			*temp2=0;
		}
		get(ww->ED_write_MESS,MUIA_TextEditor_Quiet,&ed_quiet);
		set(ww->ED_write_MESS,MUIA_TextEditor_Quiet,TRUE);
		DoMethod(ww->ED_write_MESS,MUIM_TextEditor_ClearText);
		DoMethod(ww->ED_write_MESS,MUIM_TextEditor_InsertText,temp,MUIV_TextEditor_InsertText_Bottom);
		if(val>=0) {
			if( sigs[val] && *(sigs[val]) != '\0' ) {
				DoMethod(ww->ED_write_MESS,MUIM_TextEditor_InsertText,"\n-- \n",MUIV_TextEditor_InsertText_Bottom);
				DoMethod(ww->ED_write_MESS,MUIM_TextEditor_InsertText,sigs[val],MUIV_TextEditor_InsertText_Bottom);
			}
		}
		set(ww->ED_write_MESS,MUIA_TextEditor_Quiet,ed_quiet);
		FreeVec(temp); // should be FreeVec
		temp=NULL;
	}
	set(ww->ED_write_MESS,MUIA_TextEditor_ExportWrap,account.linelength);
	//return 0;
}

BOOL WriteWindow::exfile(MIMEType * mime)
{
	nLog("WriteWindow::exfile((MIMEType *)%d) called\n",mime);
	BOOL okay=TRUE;
	struct ExamineData *data;

	if((data = ExamineObjectTags(EX_StringName,mime->file,TAG_END)))
	{
		strncpy(mime->shortname,data->Name,MIMETYPE_SHORTNAME_LEN);

		mime->shortname[MIMETYPE_SHORTNAME_LEN] = '\0';
		mime->size=data->FileSize;
		*mime->type = '\0';
		if( stricmpe(mime->shortname,".ai")==0)
			strcpy(mime->type,"application/postscript");
		else if( stricmpe(mime->shortname,".aif")==0)
			strcpy(mime->type,"audio/x-aiff");
		else if( stricmpe(mime->shortname,".aifc")==0)
			strcpy(mime->type,"audio/x-aiff");
		else if( stricmpe(mime->shortname,".aiff")==0)
			strcpy(mime->type,"audio/x-aiff");
		else if( stricmpe(mime->shortname,".anim")==0)
			strcpy(mime->type,"video/x-anim");
		else if( stricmpe(mime->shortname,".asc")==0)
			strcpy(mime->type,"text/plain");
		else if( stricmpe(mime->shortname,".au")==0)
			strcpy(mime->type,"audio/basic");
		else if( stricmpe(mime->shortname,".avi")==0)
			strcpy(mime->type,"video/x-msvideo");
		else if( stricmpe(mime->shortname,".bin")==0)
			strcpy(mime->type,"application/octet-stream");
		else if( stricmpe(mime->shortname,".c")==0)
			strcpy(mime->type,"text/plain");
		else if( stricmpe(mime->shortname,".cc")==0)
			strcpy(mime->type,"text/plain");
		else if( stricmpe(mime->shortname,".cpp")==0)
			strcpy(mime->type,"text/plain");
		else if( stricmpe(mime->shortname,".class")==0)
			strcpy(mime->type,"application/octet-stream");
		else if( stricmpe(mime->shortname,".css")==0)
			strcpy(mime->type,"text/css");
		else if( stricmpe(mime->shortname,".dms")==0)
			strcpy(mime->type,"application/octet-stream");
		else if( stricmpe(mime->shortname,".doc")==0)
			strcpy(mime->type,"application/msword");
		else if( stricmpe(mime->shortname,".dvi")==0)
			strcpy(mime->type,"application/x-dvi");
		else if( stricmpe(mime->shortname,".dxf")==0)
			strcpy(mime->type,"application/dxf");
		else if( stricmpe(mime->shortname,".eps")==0)
			strcpy(mime->type,"application/postscript");
		else if( stricmpe(mime->shortname,".exe")==0)
			strcpy(mime->type,"application/octet-stream");
		else if( stricmpe(mime->shortname,".f")==0)
			strcpy(mime->type,"text/plain");
		else if( stricmpe(mime->shortname,".f90")==0)
			strcpy(mime->type,"text/plain");
		else if( stricmpe(mime->shortname,".fli")==0)
			strcpy(mime->type,"video/x-fli");
		else if( stricmpe(mime->shortname,".gif")==0)
			strcpy(mime->type,"image/gif");
		else if( stricmpe(mime->shortname,".gtar")==0)
			strcpy(mime->type,"application/x-gtar");
		else if( stricmpe(mime->shortname,".guide")==0)
			strcpy(mime->type,"text/x-aguide");
		else if( stricmpe(mime->shortname,".gzip")==0)
			strcpy(mime->type,"application/x-gzip");
		else if( stricmpe(mime->shortname,".h")==0)
			strcpy(mime->type,"text/plain");
		else if( stricmpe(mime->shortname,".hh")==0)
			strcpy(mime->type,"text/plain");
		else if( stricmpe(mime->shortname,".hqx")==0)
			strcpy(mime->type,"application/mac-binhex40");
		else if( stricmpe(mime->shortname,".htm")==0)
			strcpy(mime->type,"text/html");
		else if( stricmpe(mime->shortname,".html")==0)
			strcpy(mime->type,"text/html");
		else if( stricmpe(mime->shortname,".ief")==0)
			strcpy(mime->type,"image/ief");
		else if( stricmpe(mime->shortname,".iff")==0)
			strcpy(mime->type,"image/x-ilbm");
		else if( stricmpe(mime->shortname,".iges")==0)
			strcpy(mime->type,"model/iges");
		else if( stricmpe(mime->shortname,".igs")==0)
			strcpy(mime->type,"model/iges");
		else if( stricmpe(mime->shortname,".ips")==0)
			strcpy(mime->type,"application/x-ipscript");
		else if( stricmpe(mime->shortname,".jpe")==0)
			strcpy(mime->type,"image/jpeg");
		else if( stricmpe(mime->shortname,".jpeg")==0)
			strcpy(mime->type,"image/jpeg");
		else if( stricmpe(mime->shortname,".jpg")==0)
			strcpy(mime->type,"image/jpeg");
		else if( stricmpe(mime->shortname,".js")==0)
			strcpy(mime->type,"application/x-javascript");
		else if( stricmpe(mime->shortname,".kar")==0)
			strcpy(mime->type,"audio/midi");
		else if( stricmpe(mime->shortname,".latex")==0)
			strcpy(mime->type,"application/x-latex");
		else if( stricmpe(mime->shortname,".lha")==0)
			strcpy(mime->type,"application/x-lha");
		else if( stricmpe(mime->shortname,".lsp")==0)
			strcpy(mime->type,"application/x-lisp");
		else if( stricmpe(mime->shortname,".lzh")==0)
			strcpy(mime->type,"application/x-lzh");
		else if( stricmpe(mime->shortname,".lzx")==0)
			strcpy(mime->type,"application/x-lzx");
		else if( stricmpe(mime->shortname,".m")==0)
			strcpy(mime->type,"text/plain");
		else if( stricmpe(mime->shortname,".mesh")==0)
			strcpy(mime->type,"model/mesh");
		else if( stricmpe(mime->shortname,".mid")==0)
			strcpy(mime->type,"audio/midi");
		else if( stricmpe(mime->shortname,".midi")==0)
			strcpy(mime->type,"audio/midi");
		else if( stricmpe(mime->shortname,".mime")==0)
			strcpy(mime->type,"www/mime");
		else if( stricmpe(mime->shortname,".mov")==0)
			strcpy(mime->type,"video/quicktime");
		else if( stricmpe(mime->shortname,".movie")==0)
			strcpy(mime->type,"video/x-sgi-movie");
		else if( stricmpe(mime->shortname,".mp2")==0)
			strcpy(mime->type,"audio/mpeg");
		else if( stricmpe(mime->shortname,".mp3")==0)
			strcpy(mime->type,"audio/mpeg");
		else if( stricmpe(mime->shortname,".mpe")==0)
			strcpy(mime->type,"video/mpeg");
		else if( stricmpe(mime->shortname,".mpeg")==0)
			strcpy(mime->type,"video/mpeg");
		else if( stricmpe(mime->shortname,".mpg")==0)
			strcpy(mime->type,"video/mpeg");
		else if( stricmpe(mime->shortname,".mpga")==0)
			strcpy(mime->type,"audio/mpeg");
		else if( stricmpe(mime->shortname,".msh")==0)
			strcpy(mime->type,"audio/mesh");
		else if( stricmpe(mime->shortname,".pbm")==0)
			strcpy(mime->type,"image/x-portable-bitmap");
		else if( stricmpe(mime->shortname,".pdf")==0)
			strcpy(mime->type,"application/pdf");
		else if( stricmpe(mime->shortname,".pgm")==0)
			strcpy(mime->type,"image/x-portable-graymap");
		else if( stricmpe(mime->shortname,".png")==0)
			strcpy(mime->type,"image/png");
		else if( stricmpe(mime->shortname,".pnm")==0)
			strcpy(mime->type,"image/x-portable-anymap");
		else if( stricmpe(mime->shortname,".pot")==0)
			strcpy(mime->type,"application/mspowerpoint");
		else if( stricmpe(mime->shortname,".ppm")==0)
			strcpy(mime->type,"image/x-portable-pixmap");
		else if( stricmpe(mime->shortname,".pps")==0)
			strcpy(mime->type,"application/mspowerpoint");
		else if( stricmpe(mime->shortname,".ppt")==0)
			strcpy(mime->type,"application/mspowerpoint");
		else if( stricmpe(mime->shortname,".ppz")==0)
			strcpy(mime->type,"application/mspowerpoint");
		else if( stricmpe(mime->shortname,".ps")==0)
			strcpy(mime->type,"application/postscript");
		else if( stricmpe(mime->shortname,".qt")==0)
			strcpy(mime->type,"video/quicktime");
		else if( stricmpe(mime->shortname,".ra")==0)
			strcpy(mime->type,"audio/x-realaudio");
		else if( stricmpe(mime->shortname,".ram")==0)
			strcpy(mime->type,"audio/x-pn-realaudio");
		else if( stricmpe(mime->shortname,".ras")==0)
			strcpy(mime->type,"image/cmu-raster");
		else if( stricmpe(mime->shortname,".readme")==0)
			strcpy(mime->type,"text/plain");
		else if( stricmpe(mime->shortname,".rexx")==0)
			strcpy(mime->type,"text/plain");
		else if( stricmpe(mime->shortname,".rgb")==0)
			strcpy(mime->type,"image/x-rgb");
		else if( stricmpe(mime->shortname,".rm")==0)
			strcpy(mime->type,"audio/x-pn-realaudio");
		else if( stricmpe(mime->shortname,".rpm")==0)
			strcpy(mime->type,"audio/x-pn-realaudio-plugin");
		else if( stricmpe(mime->shortname,".rtf")==0)
			strcpy(mime->type,"text/rtf");
		else if( stricmpe(mime->shortname,".rtx")==0)
			strcpy(mime->type,"text/richtext");
		else if( stricmpe(mime->shortname,".rx")==0)
			strcpy(mime->type,"text/plain");
		else if( stricmpe(mime->shortname,".sgm")==0)
			strcpy(mime->type,"text/sgml");
		else if( stricmpe(mime->shortname,".sgml")==0)
			strcpy(mime->type,"text/sgml");
		else if( stricmpe(mime->shortname,".shtml")==0)
			strcpy(mime->type,"text/html");
		else if( stricmpe(mime->shortname,".sit")==0)
			strcpy(mime->type,"application/x-stuffit");
		else if( stricmpe(mime->shortname,".snd")==0)
			strcpy(mime->type,"audio/basic");
		else if( stricmpe(mime->shortname,".swf")==0)
			strcpy(mime->type,"application/x-shockwave-flash");
		else if( stricmpe(mime->shortname,".tar")==0)
			strcpy(mime->type,"application/x-tar");
		else if( stricmpe(mime->shortname,".tcl")==0)
			strcpy(mime->type,"application/x-tcl");
		else if( stricmpe(mime->shortname,".tex")==0)
			strcpy(mime->type,"application/x-tex");
		else if( stricmpe(mime->shortname,".texi")==0)
			strcpy(mime->type,"application/x-texinfo");
		else if( stricmpe(mime->shortname,".texinfo")==0)
			strcpy(mime->type,"application/x-texinfo");
		else if( stricmpe(mime->shortname,".tif")==0)
			strcpy(mime->type,"image/tiff");
		else if( stricmpe(mime->shortname,".tiff")==0)
			strcpy(mime->type,"image/tiff");
		else if( stricmpe(mime->shortname,".txt")==0)
			strcpy(mime->type,"text/plain");
		else if( stricmpe(mime->shortname,".vrml")==0)
			strcpy(mime->type,"model/vrml");
		else if( stricmpe(mime->shortname,".wav")==0)
			strcpy(mime->type,"audio/x-wav");
		else if( stricmpe(mime->shortname,".wrl")==0)
			strcpy(mime->type,"model/vrml");
		else if( stricmpe(mime->shortname,".xbm")==0)
			strcpy(mime->type,"image/x-xbitmap");
		else if( stricmpe(mime->shortname,".xlc")==0)
			strcpy(mime->type,"application/vnd.ms-excel");
		else if( stricmpe(mime->shortname,".xll")==0)
			strcpy(mime->type,"application/vnd.ms-excel");
		else if( stricmpe(mime->shortname,".xlm")==0)
			strcpy(mime->type,"application/vnd.ms-excel");
		else if( stricmpe(mime->shortname,".xls")==0)
			strcpy(mime->type,"application/vnd.ms-excel");
		else if( stricmpe(mime->shortname,".xlw")==0)
			strcpy(mime->type,"application/vnd.ms-excel");
		else if( stricmpe(mime->shortname,".xml")==0)
			strcpy(mime->type,"text/xml");
		else if( stricmpe(mime->shortname,".xpm")==0)
			strcpy(mime->type,"image/x-xpixmap");
		else if( stricmpe(mime->shortname,".xwd")==0)
			strcpy(mime->type,"image/x-xwindowdump");
		else if( stricmpe(mime->shortname,".zip")==0)
			strcpy(mime->type,"application/x-zip");
		/*else if( stricmpe(mime->shortname,".")==0)
			strcpy(mime->type,"/");*/
		else if(*mime->type==0)
		{
			strcpy(mime->type,"text/plain");
			okay = FALSE;
		}
		FreeDosObject(DOS_EXAMINEDATA,data);
	}
	else
		okay = FALSE;

	return okay;
}

//HOOKCL3( VOID, WriteWindow::attFunc, CPPHook *, hook, a0, Object *, object, a2, WriteWindow **, W, a1 )
void WriteWindow::attFunc(void **args)
{
	//WriteWindow * ww = *W;
	WriteWindow * ww = (WriteWindow *)args[0];
	Object *object = (Object *)args[1];
	MIMEType *ptr;
	ULONG val;

	if(object == ww->BT_write_ADDATT)
	{
		ptr=new MIMEType;
		if(LoadASL(ptr->file,CatalogStr(MSG_SELECT_FILE_TO_ATTACH,MSG_SELECT_FILE_TO_ATTACH_STR),(const char *)"",(const char *)"#?",FALSE)) {
			if( !exfile(ptr) )
			{
				// warn that we couldn't determine mime type from extension
				MUI_RequestA(app,0,0,CatalogStr(MSG_MIME_ATTACHMENT,MSG_MIME_ATTACHMENT_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_UNKNOWN_ATTACHMENT,MSG_UNKNOWN_ATTACHMENT_STR),0);
			}
			set(ww->STR_write_MIME,MUIA_String_Contents,ptr->type);
			DoMethod(ww->NLIST_write_ATT,MUIM_NList_InsertSingle,ptr,MUIV_NList_Insert_Bottom);
			set(ww->NLIST_write_ATT,MUIA_NList_Active,MUIV_NList_Active_Bottom);
		}
	}
	else if(object == ww->BT_write_DELATT)
	{
		set(ww->STR_write_MIME,MUIA_String_Contents,"");
		get(ww->NLIST_write_ATT,MUIA_NList_Active,&val);
		if(val!=MUIV_NList_Active_Off)
			DoMethod(ww->NLIST_write_ATT,MUIM_NList_Remove,val);
	}
	else if(object == ww->NLIST_write_ATT) {
		DoMethod(ww->NLIST_write_ATT,MUIM_NList_GetEntry,MUIV_NList_GetEntry_Active,&ptr);
		if(ptr)
			set(ww->STR_write_MIME,MUIA_String_Contents,ptr->type);
	}
	//return 0;
}

/*LONG ASM WriteWindow::menusFunc(REG(a0) CPPHook * hook,REG(a2) Object * object,REG(a1) APTR * param) {*/
//LONG ASM ViewWindow::menusFunc(REG(a0) CPPHook * hook,REG(a2) Object * object,REG(a1) ViewWindow ** W,int * uD) {
/*HOOKCL3( LONG, WriteWindow::menusFunc, CPPHook *, hook, a0, Object *, object, a2, APTR *, param, a1 )
	WriteWindow * ww = (WriteWindow *)param[0];
	int uD = (int)param[1];*/
void WriteWindow::menusFunc(void **args) {
	WriteWindow * ww = (WriteWindow *)args[0];
	int uD = (int)args[1];
	switch (uD) {
		case WRITE_SAVE:
			ww->save(FALSE);
			break;
	}
}

//CPPHook WriteWindow::addNGHook ={ {NULL,NULL},(CPPHOOKFUNC)&WriteWindow::addNGFunc,NULL,NULL};
//CPPHook WriteWindow::attHook ={ {NULL,NULL},(CPPHOOKFUNC)&WriteWindow::attFunc,NULL,NULL};
//CPPHook WriteWindow::disposeHook ={ {NULL,NULL},(CPPHOOKFUNC)&WriteWindow::disposeFunc,NULL,NULL};
CPPHook WriteWindow::completeHook ={ {NULL,NULL},(CPPHOOKFUNC)&WriteWindow::completeFunc,NULL,NULL};
CPPHook WriteWindow::attObjStrHook = { { NULL,NULL },(CPPHOOKFUNC)&WriteWindow::attObjStrFunc,NULL,NULL };
//CPPHook WriteWindow::menusHook ={ {NULL,NULL},(CPPHOOKFUNC)&WriteWindow::menusFunc,NULL,NULL};
int WriteWindow::count=0;
Vector *WriteWindow::ptrs = new Vector(32);

MUI_CustomClass *WriteWindow::editor_mcc_write = NULL;

WriteWindow::WriteWindow(const char *scrtitle) {
	nLog("WriteWindow((char *)%s) constructor called\n",scrtitle);
	this->count++;
	ptrs->add(this);
	if(editor_mcc_write == NULL)
		editor_mcc_write = MUI_CreateCustomClass(NULL, (CONST STRPTR)"TextEditor.mcc", NULL, 0, (void *)&WriteWindow::TextEditor_Dispatcher_write);

	InitArray(Pages_write,MSG_MESSAGE);
	NewMenu MenuDataWrite[]= {
		{NM_TITLE,(CONST STRPTR)"Message",						0,0,0,	(APTR)MENWRITE_MESSAGE},
		{NM_ITEM,(CONST STRPTR)"Save",							(CONST STRPTR)"S",0,0,	(APTR)WRITE_SAVE},

		{NM_END,NULL,0,0,0,(APTR)0},
	};
	InitMenu(MenuDataWrite,MENU_MESSAGE_2);
	SLD_write_MESS = ScrollbarObject, End;
	{
		wnd = WindowObject,
			MUIA_Window_Title,			CatalogStr(MSG_WRITE,MSG_WRITE_STR),
			MUIA_Window_ID,				MAKE_ID('W','R','T','E') + (count - 1),
			MUIA_Window_ScreenTitle,	scrtitle,
			MUIA_Window_CloseGadget,	TRUE,
			MUIA_Window_Menustrip,		menustrip=MUI_MakeObject(MUIO_MenustripNM,MenuDataWrite,0),
			MUIA_HelpNode,					"WIN_WRITE",

			WindowContents, VGroup,
				Child, RegisterGroup(Pages_write),
					MUIA_Register_Frame, TRUE,
					MUIA_CycleChain, 1,
					//page 1
					Child, VGroup,
						Child, ColGroup(2),
							Child, Label2(CatalogStr(LABEL_NEWSGROUPS,LABEL_NEWSGROUPS_STR)),
							Child, ColGroup(2),
								Child, STR_write_NG=BetterString("",NEWMESS_long+1),
								Child, BT_write_NG=SimpleButton("+/-"),
 								End,
							Child, Label2(CatalogStr(LABEL_TO,LABEL_TO_STR)), Child, STR_write_TO=BetterString("",NEWMESS_long+1),
							Child, Label2(CatalogStr(LABEL_SUBJECT,LABEL_SUBJECT_STR)), Child, STR_write_SUBJECT=BetterString("",NEWMESS_long+3),
							End,
						Child, HGroup,
							MUIA_Group_Spacing, 0,
							Child, ED_write_MESS = (Object*)NewObject(editor_mcc_write->mcc_Class, NULL,
								MUIA_TextEditor_Slider,			SLD_write_MESS,
								//MUIA_TextEditor_ColorMap,		w_cmap,
								MUIA_TextEditor_ImportHook,	MUIV_TextEditor_ImportHook_Plain,
								//MUIA_TextEditor_ImportHook,	MUIV_TextEditor_ImportHook_EMail,
								//MUIA_TextEditor_ExportHook,	MUIV_TextEditor_ExportHook_Plain,
								MUIA_TextEditor_ExportHook,	MUIV_TextEditor_ExportHook_EMail,
								MUIA_TextEditor_ExportWrap,	72,
								MUIA_TextEditor_FixedFont,		TRUE,
								MUIA_CycleChain, 1,
								End,
							Child, SLD_write_MESS,
							End,
						End,
					//page 2
					Child, VGroup,
						Child, NListviewObject,
							MUIA_NListview_NList,			NLIST_write_ATT=NListObject,
								MUIA_NList_Title,				TRUE,
								MUIA_NList_DragSortable,	TRUE,
								MUIA_NList_DragType,			MUIV_NList_DragType_Immediate,
								MUIA_NList_DisplayHook,		&DisplayHook_mime,
								MUIA_NList_DestructHook,	&DestructHook_mime,
								MUIA_NList_Format,			",,",
								MUIA_NList_MinColSortable,	0,
								End,
							End,
						Child, ColGroup(3),
							Child, BT_write_ADDATT=SimpleButton(CatalogStr(MSG_ADD_ATTACHMENT,MSG_ADD_ATTACHMENT_STR)),
							Child, BT_write_DELATT=SimpleButton(CatalogStr(MSG_DELETE_ATTACHMENT,MSG_DELETE_ATTACHMENT_STR)),
							Child, POP_write_MIME = PopobjectObject,
								MUIA_Popstring_String, STR_write_MIME=BetterString("",32),
								MUIA_Popstring_Button, PopButton(MUII_PopUp),
								MUIA_Popobject_StrObjHook, &StrObjHook,
								MUIA_Popobject_ObjStrHook, &attObjStrHook,
								MUIA_Popobject_WindowHook, &WindowHook,
								MUIA_Popobject_Object, LIST_write_MIME = ListviewObject,
									MUIA_Listview_List, ListObject,
										InputListFrame,
										MUIA_List_SourceArray, POPA_write_mime,
										End,
									End,
								End,
							End,
						End,
					//page 3
					Child, VGroup,
						Child, ColGroup(2),
							Child, Label2(CatalogStr(LABEL_FROM,LABEL_FROM_STR)), Child, STR_write_FROM=BetterString("",256),
						End,
							Child, ColGroup(3),
							Child, Label2(CatalogStr(LABEL_FOLLOWUP_TO,LABEL_FOLLOWUP_TO_STR)), Child, STR_write_FOLLOWUP=BetterString("",512),
							Child, BT_write_FU=SimpleButton("+/-"),
							End,
						End,
					End,
				Child, ColGroup(5),
				//Child, ColGroup(4),
					Child, BT_write_P=SimpleButton(CatalogStr(TBAR_POST,TBAR_POST_STR)),
					Child, BT_write_PL=SimpleButton(CatalogStr(MSG_SEND_TO_QUEUE,MSG_SEND_TO_QUEUE_STR)),
					Child, BT_write_H=SimpleButton(CatalogStr(MSG_POSTPONE,MSG_POSTPONE_STR)),
					Child, BT_write_ROT13=SimpleButton(CatalogStr(MSG_ROT13,MSG_ROT13_STR)),
					Child, BT_write_C=SimpleButton(CatalogStr(MSG_CANCEL_2,MSG_CANCEL_2_STR)),
					End,
				Child, ColGroup(2),
					Child, BT_write_READ=SimpleButton(CatalogStr(MSG_READ_FROM_FILE_2,MSG_READ_FROM_FILE_2_STR)),
					Child, CY_write_SIG=Cycle(CYA_nng_sigs),
					End,
				End,
			End;

		DoMethod(app,OM_ADDMEMBER,wnd);

		DoMethod(wnd,MUIM_Notify,MUIA_Window_CloseRequest,TRUE,
			BT_write_C,3,MUIM_CallHook,&completeHook,this);
			//BT_write_C,4,MUIM_CallHook,&hook_standard,completeFunc,this,BT_write_C);

		DoMethod(BT_write_NG,MUIM_Notify,MUIA_Pressed,FALSE,
			//MUIV_Notify_Self,3,MUIM_CallHook,&addNGHook,this);
			MUIV_Notify_Self,5,MUIM_CallHook,&hook_standard,addNGFunc,this,BT_write_NG);
		DoMethod(BT_write_FU,MUIM_Notify,MUIA_Pressed,FALSE,
			//MUIV_Notify_Self,3,MUIM_CallHook,&addNGHook,this);
			MUIV_Notify_Self,5,MUIM_CallHook,&hook_standard,addNGFunc,this,BT_write_FU);

		DoMethod(BT_write_P,MUIM_Notify,MUIA_Pressed,FALSE,
			MUIV_Notify_Self,3,MUIM_CallHook,&completeHook,this);
		DoMethod(BT_write_PL,MUIM_Notify,MUIA_Pressed,FALSE,
			MUIV_Notify_Self,3,MUIM_CallHook,&completeHook,this);
		DoMethod(BT_write_H,MUIM_Notify,MUIA_Pressed,FALSE,
			MUIV_Notify_Self,3,MUIM_CallHook,&completeHook,this);
		DoMethod(BT_write_C,MUIM_Notify,MUIA_Pressed,FALSE,
			MUIV_Notify_Self,3,MUIM_CallHook,&completeHook,this);

		DoMethod(BT_write_ROT13,MUIM_Notify,MUIA_Pressed,FALSE,
			//MUIV_Notify_Self,3,MUIM_CallHook,&rot13Hook,this);
			MUIV_Notify_Self,4,MUIM_CallHook,&hook_standard,rot13Func,this);

		DoMethod(BT_write_READ,MUIM_Notify,MUIA_Pressed,FALSE,
			//MUIV_Notify_Self,3,MUIM_CallHook,&readHook,this);
			MUIV_Notify_Self,4,MUIM_CallHook,&hook_standard,readFunc,this);

		DoMethod(CY_write_SIG,MUIM_Notify,MUIA_Cycle_Active,MUIV_EveryTime,
			//MUIV_Notify_Self,3,MUIM_CallHook,&cysigHook,this);
			MUIV_Notify_Self,4,MUIM_CallHook,&hook_standard,cysigFunc,this);

		DoMethod(BT_write_ADDATT,MUIM_Notify,MUIA_Pressed,FALSE,
			//MUIV_Notify_Self,3,MUIM_CallHook,&attHook,this);
			MUIV_Notify_Self,5,MUIM_CallHook,&hook_standard,attFunc,this,BT_write_ADDATT);
		DoMethod(BT_write_DELATT,MUIM_Notify,MUIA_Pressed,FALSE,
			//MUIV_Notify_Self,3,MUIM_CallHook,&attHook,this);
			MUIV_Notify_Self,5,MUIM_CallHook,&hook_standard,attFunc,this,BT_write_DELATT);
		DoMethod(NLIST_write_ATT,MUIM_Notify,MUIA_NList_Active,MUIV_EveryTime,
			//MUIV_Notify_Self,3,MUIM_CallHook,&attHook,this);
			MUIV_Notify_Self,4,MUIM_CallHook,&hook_standard,attFunc,this,NLIST_write_ATT);
		DoMethod(LIST_write_MIME,MUIM_Notify,MUIA_Listview_DoubleClick,TRUE,
			POP_write_MIME,2,MUIM_Popstring_Close,TRUE);
		DoMethod(STR_write_MIME,MUIM_Notify,MUIA_String_Acknowledge,MUIV_EveryTime,
			//MUIV_Notify_Self,3,MUIM_CallHook,&mimeTypeHook,this);
			MUIV_Notify_Self,4,MUIM_CallHook,&hook_standard,mimeTypeFunc,this);

		DoMethod(wnd,MUIM_Notify,MUIA_Window_MenuAction,MUIV_EveryTime,
			//app,4,MUIM_CallHook,&menusHook,this,MUIV_TriggerValue);
			app,5,MUIM_CallHook,&hook_standard,menusFunc,this,MUIV_TriggerValue);

		set(BT_write_NG,MUIA_HorizWeight,25);
		set(BT_write_FU,MUIA_HorizWeight,25);

		set(BT_write_P,MUIA_ShortHelp,CatalogStr(MSG_SEND_HELP,MSG_SEND_HELP_STR));
		set(BT_write_NG,MUIA_ShortHelp,CatalogStr(MSG_ADD_OR_REMOVE_GROUP_TO_POST_HELP,MSG_ADD_OR_REMOVE_GROUP_TO_POST_HELP_STR));
		set(BT_write_FU,MUIA_ShortHelp,CatalogStr(MSG_ADD_OR_REMOVE_GROUP_TO_FOLLOWUP_TO_HELP,MSG_ADD_OR_REMOVE_GROUP_TO_FOLLOWUP_TO_HELP_STR));
		set(BT_write_PL,MUIA_ShortHelp,CatalogStr(MSG_QUEUE_HELP,MSG_QUEUE_HELP_STR));
		set(BT_write_H,MUIA_ShortHelp,CatalogStr(MSG_HOLD_HELP,MSG_HOLD_HELP_STR));
		set(BT_write_C,MUIA_ShortHelp,CatalogStr(MSG_CANCEL_MESSAGE_HELP,MSG_CANCEL_MESSAGE_HELP_STR));
		set(BT_write_READ,MUIA_ShortHelp,CatalogStr(MSG_INSERT_TEXT_FROM_FILE_HELP,MSG_INSERT_TEXT_FROM_FILE_HELP_STR));
		set(CY_write_SIG,MUIA_ShortHelp,CatalogStr(MSG_SELECT_SIGNATURE_FOR_MESSAGE_HELP,MSG_SELECT_SIGNATURE_FOR_MESSAGE_HELP_STR));
		set(STR_write_TO,MUIA_ShortHelp,CatalogStr(MSG_TO_HELP,MSG_TO_HELP_STR));
		set(STR_write_NG,MUIA_ShortHelp,CatalogStr(MSG_NEWSGROUP_HELP,MSG_NEWSGROUP_HELP_STR));
		set(STR_write_SUBJECT,MUIA_ShortHelp,CatalogStr(MSG_SUBJECT_HELP,MSG_SUBJECT_HELP_STR));
		set(STR_write_FROM,MUIA_ShortHelp,CatalogStr(MSG_FROM_HELP,MSG_FROM_HELP_STR));
		set(STR_write_FOLLOWUP,MUIA_ShortHelp,CatalogStr(MSG_FOLLOWUP_TO_HELP,MSG_FOLLOWUP_TO_HELP_STR));
	}

	set(STR_write_TO,MUIA_String_Contents,"");
	set(STR_write_SUBJECT,MUIA_String_Contents,"");
	set(STR_write_FOLLOWUP,MUIA_String_Contents,"");

	set(ED_write_MESS,MUIA_TextEditor_ExportWrap,account.linelength);

	/*{
		static char malicious[256] = "";
		malicious[0] = 27;
		strcpy(&malicious[1],"I[4:APIPE:Echo >CON:////Hacked/AUTO/WAIT/CLOSE/ Owned]");
		//printf("%s\n",malicious);
		set(STR_write_SUBJECT,MUIA_String_Contents,malicious);
	}*/
}

WriteWindow::~WriteWindow() {
	nLog("WriteWindow::~WriteWindow() called - count = %d\n",count);
	set(wnd,MUIA_Window_Open,FALSE);
	DoMethod(ED_write_MESS,MUIM_TextEditor_ClearText);
	DoMethod(app,OM_REMMEMBER,wnd);
	/*nLog("about to dispose of TextEditor Custom Class\n");
	if(editor_mcc_write)
		MUI_DeleteCustomClass(editor_mcc_write);*/
	nLog("about to dispose wnd\n");
	MUI_DisposeObject(wnd);
	this->count--;
	ptrs->removeElement(this);
}

void WriteWindow::sleepAll(BOOL sleep)
{
	int size=ptrs->getSize();
	WriteWindow ** data = (WriteWindow **)ptrs->getData();
	for(int i=0;i<size;i++)
		set(data[i]->wnd,MUIA_Window_Sleep,sleep);
}

MessageListData *WriteWindow::write_message(BOOL hold,BOOL online,BOOL force)
{
	char head_buf[NEWMESS_long+1];
	char *translated_string=NULL;

	nLog("WriteWindow::write_message((BOOL)%d,(BOOL)%d,(BOOL)%d) called\n",hold,online,force);
	//printf("%d:%s\n",newmessage.references[0],newmessage.references[0]);
	WriteWindow *write = this;

	BOOL no_subject = FALSE;
	if(!force && *getstr(STR_write_SUBJECT)==0)
	{
		no_subject = TRUE;
		if( MUI_RequestA(app,0,0,CatalogStr(MSG_WRITE_MESSAGE,MSG_WRITE_MESSAGE_STR),CatalogStr(MSG_EDIT_OR_CONTINUE,MSG_EDIT_OR_CONTINUE_STR),CatalogStr(MSG_NO_SUBJECT,MSG_NO_SUBJECT_STR),0) == 1 ) {
			set(wnd,MUIA_Window_ActiveObject,STR_write_SUBJECT);
			return NULL;
		}
	}

	set(write->ED_write_MESS,MUIA_TextEditor_ExportWrap,0);
	char *text=(char *)DoMethod(write->ED_write_MESS,MUIM_TextEditor_ExportText);
	nLog("  Text Exported from TextEditor.mcc\n");
	set(write->ED_write_MESS,MUIA_TextEditor_ExportWrap,account.linelength);

	if( !force && !no_subject )
	{
		BOOL empty = TRUE;
		char *ptr = text;
		while(*ptr != '\0' && empty)
		{
			empty = isspace(*ptr);
			ptr++;
		}
		if(empty)
		{
			if( MUI_RequestA(app,0,0,CatalogStr(MSG_WRITE_MESSAGE,MSG_WRITE_MESSAGE_STR),CatalogStr(MSG_EDIT_OR_CONTINUE,MSG_EDIT_OR_CONTINUE_STR),CatalogStr(MSG_EMPTY_MESSAGE,MSG_EMPTY_MESSAGE_STR),0) == 1 ) {
				set(wnd,MUIA_Window_ActiveObject,ED_write_MESS);
				FreeVec(text);
				return NULL;
			}
		}
		else
		{
			BOOL all_quoted = TRUE;
			BOOL found_quote = FALSE;
			ptr = text;
			for(;;)
			{
				if(*ptr == '\0')
					break;
				if(*ptr == '\n')
				{
					ptr++;
					continue;
				}
				if( strncmp(ptr,"> ",2) == 0 )
					found_quote = TRUE;
				else if( ptr != text )
				{
					all_quoted = FALSE;
					break;
				}
				ptr = strchr(ptr,'\n');
				if(ptr == NULL)
					break;
				ptr++;
			}
			if(all_quoted && found_quote)
			{
				if( MUI_RequestA(app,0,0,CatalogStr(MSG_WRITE_MESSAGE,MSG_WRITE_MESSAGE_STR),CatalogStr(MSG_EDIT_OR_CONTINUE,MSG_EDIT_OR_CONTINUE_STR),CatalogStr(MSG_ALL_QUOTED,MSG_ALL_QUOTED_STR),0) == 1 ) {
					set(wnd,MUIA_Window_ActiveObject,ED_write_MESS);
					FreeVec(text);
					return NULL;
				}
			}
		}
	}

	set(write->wnd,MUIA_Window_Sleep,TRUE);

	BOOL bit7 = FALSE;
	translated_string=translateCharset((unsigned char *)text,account.charset_write);
	if (translated_string)
		strcpy(text,translated_string);

	char *src = text;

	char *space = NULL;
	int lnw = 0,eqlength = 20;
	int llength = account.linelength;
	if(*src=='>' || *src=='|' || *src==':' || *src=='!') // assume quoted
		llength += eqlength;
	for(;;)
	{
		if(isspace(*src))
			space = src;
		if(*src=='\n')
		{
			src++;
			lnw=0;
			llength = account.linelength;
			if(*src=='>' || *src=='|' || *src==':' || *src=='!') // assume quoted
				llength += eqlength;
		}
		else if(*src==0)
			break;
		else
		{
			src++;
			lnw++;
		}
		if(lnw>=llength)
		{
			if(isspace(*src))
			{
				*src++='\n';
				lnw=0;
				llength=account.linelength;
				if(*src=='>' || *src=='|' || *src==':' || *src=='!') // assume quoted
					llength+=eqlength;
			}
			else {
				int dist = (int)src - (int)space;
				if(dist <= llength) {
					src = space;
					*src++='\n';
					lnw=0;
					llength=account.linelength;
					if(*src=='>' || *src=='|' || *src==':' || *src=='!') // assume quoted
						llength+=eqlength;
				}
			}
		}
	}
	int textlen = (int)src - (int)text;
	nLog("  Text Wrapped\n");
	MessageListData * mdata = new MessageListData();
	mdata->init();
	mdata->flags[0]=(int)hold;
	mdata->flags[2]=(int)online;
	mdata->flags[6]=-1;
	char buffer[1024] = "";
	char boundary[256] = "";

	// mark message as replied to if this is a reply (or followup)
	if(write->newmessage.reply)
	{
		GroupData * gdata = NULL;
		get_gdata(&gdata,write->newmessage.replied_gdataID);
		if(gdata!=NULL)
		{
			MessageListData * mdata2 = NULL;
			BOOL del = get_mdata(&mdata2,gdata,write->newmessage.replied_mdataID,TRUE);
			if(mdata2 != NULL)
			{
				mdata2->flags[10]=TRUE;
				write_index_update(gdata,mdata2,NLIST_messagelistdata);
				if(del)
					delete mdata;
			}
		}
	}

	// delete old message if edited
	GroupData * gdata = NULL;
	get_gdata(&gdata,-1);
	if(write->newmessage.edit==TRUE && write->newmessage.sent==FALSE)
	{
		Vector *vector = NULL;
		MessageListData *mdata = NULL;
		GroupData * cdg = NULL;
		getGdataDisplayed(&cdg);
		if(cdg->ID == gdata->ID)
		{
			ULONG entries = 0;
			get(NLIST_messagelistdata,MUIA_NList_Entries,&entries);
			for(ULONG l=0;l<entries;l++)
			{
				MessageListData * mdata2 = NULL;
				DoMethod(NLIST_messagelistdata,MUIM_NList_GetEntry,l,&mdata2);
				if(strcmp(mdata2->messageID,write->newmessage.messageID)==0)
				{
					mdata = mdata2;
					break;
				}
			}
		}
		else {
			// todo: we end up reading in the index twice; once here, and again in write_index_delete !
			vector = new Vector(2048);
			if( read_index(gdata, vector) )
			{
				for(int i=0;i<vector->getSize();i++)
				{
					MessageListData *mdata2 = ((MessageListData **)vector->getData())[i];
					if(strcmp(mdata2->messageID,write->newmessage.messageID)==0) {
						mdata = mdata2;
						break;
					}
				}
			}
		}
		if(mdata != NULL) {
			/*delete_mess(gdata,mdata,TRUE);
			write_index_if_changed();*/
			delete_mess_n(gdata,&mdata,1,FALSE);
		}
		if(vector != NULL) {
			vector->flush();
			delete vector;
		}
	}
	//printf("%d\n",gdata->nummess);
	//get_index_entries(gdata);

	ULONG multipart = 0;
	get(write->NLIST_write_ATT,MUIA_NList_Entries,&multipart);
	char x_newsreader[256] = "";
	if(account.xnews)
		sprintf(x_newsreader,"NewsCoaster (v%s)",version);
	else
		sprintf(x_newsreader,"NewsCoaster (v%s) ["PLATFORM"] - http://newscoaster.sourceforge.net/",version);
	char date[256] = "";
	//DateHandler::get_datenow(mdata->date,getGMTOffset(),account.bst);
	//DateHandler::read_date(&mdata->ds,mdata->date,mdata->c_date,mdata->c_time);
	DateHandler::get_datenow(date,getGMTOffset(),account.bst);
	DateHandler::read_date(&mdata->ds,date,mdata->c_date,mdata->c_time);
	stripWhitespace(getstr(write->STR_write_TO));
		strncpy(write->newmessage.to,getstr(write->STR_write_TO),NEWMESS_long);
		write->newmessage.to[NEWMESS_long] = '\0';
	stripWhitespace(getstr(write->STR_write_NG));
		strncpy(write->newmessage.newsgroups,getstr(write->STR_write_NG),NEWMESS_long);
		write->newmessage.newsgroups[NEWMESS_long] = '\0';
	stripWhitespace(getstr(write->STR_write_FOLLOWUP));
		strncpy(write->newmessage.followup,getstr(write->STR_write_FOLLOWUP),NEWMESS_long);
		write->newmessage.followup[NEWMESS_long] = '\0';
	stripWhitespace(getstr(write->STR_write_FROM));
		strncpy(write->newmessage.from,getstr(write->STR_write_FROM),NEWMESS_long);
		write->newmessage.from[NEWMESS_long] = '\0';

	//Server *server = getPostingServer();
	Server *server = getPostingServerForGroup(write->newmessage.newsgroups);
	//printf("server: %s\n",server->nntp);
	create_mID(write->newmessage.messageID,write->newmessage.from,server->nntp);

	stripWhitespace(getstr(write->STR_write_SUBJECT));
		strncpy(write->newmessage.subject,getstr(write->STR_write_SUBJECT),NEWMESS_long);
		write->newmessage.subject[NEWMESS_long] = '\0';

	char * header = new char[10240 + textlen];
	char *hptr = header;
	int r = 0;
	translated_string=translateCharset((unsigned char *)write->newmessage.from,account.charset_write);
	if (translated_string)
		strcpy(write->newmessage.from,translated_string);
	strcpy(head_buf,write->newmessage.from);

	r = sprintf(hptr,"From: %s\n",head_buf); hptr += r;
	//r = sprintf(hptr,"Date: %s\n",mdata->date); hptr += r;
	r = sprintf(hptr,"Date: %s\n",date); hptr += r;
	translated_string=translateCharset((unsigned char *)write->newmessage.subject,account.charset_write);
	if (translated_string)
		strcpy(write->newmessage.subject,translated_string);
	strcpy(head_buf,write->newmessage.subject);
	r = sprintf(hptr,"Subject: %s\n",head_buf); hptr += r;
	if(*account.org!=0)
	{
		translated_string=translateCharset((unsigned char *)account.org,account.charset_write);
		if (translated_string)
			strcpy(account.org,translated_string);
		strcpy(head_buf,account.org);
		r = sprintf(hptr,"Organization: %s\n",head_buf); hptr += r;
	}
	r = sprintf(hptr,"Message-ID: %s\n",write->newmessage.messageID); hptr += r;
	if(*(write->newmessage.supersedeID)!=0)
	{
		r = sprintf(hptr,"Supersedes: %s\n",write->newmessage.supersedeID); hptr += r;
	}
	if(write->newmessage.nrefs>0)
	{
		for(int k=0;k<write->newmessage.nrefs;k++)
		{
			//printf("%d : %s\n",k,write->newmessage.references[k]);
			if(k==0)
			{
				r = sprintf(hptr,"References: %s",write->newmessage.references[k]); hptr += r;
			}
			else
			{
				r = sprintf(hptr," %s",write->newmessage.references[k]); hptr += r;
			}
			if(write->newmessage.references[k])
			{
				delete write->newmessage.references[k];
				write->newmessage.references[k]=NULL;
			}
		}
		r = sprintf(hptr,"\n"); hptr += r;
		write->newmessage.nrefs=0;
	}
	r = sprintf(hptr,"X-Newsreader: %s\n",x_newsreader); hptr += r;
	// email and/or newsgroup posting
	mdata->flags[4]=FALSE;
	mdata->flags[5]=FALSE;
	if(*(write->newmessage.to)!=0)
	{
		mdata->flags[5]=TRUE;
		translated_string=translateCharset((unsigned char *)write->newmessage.to,account.charset_write);
		if (translated_string)
			strcpy(write->newmessage.to,translated_string);
		strcpy(head_buf,write->newmessage.to);
		r = sprintf(hptr,"To: %s\n",head_buf); hptr += r;
	}
	if(*(write->newmessage.newsgroups)!=0)
	{
		mdata->flags[4]=TRUE;
		r = sprintf(hptr,"Newsgroups: %s\n",write->newmessage.newsgroups); hptr += r;
	}
	if(*(write->newmessage.followup)!=0)
	{
		r = sprintf(hptr,"Followup-To: %s\n",write->newmessage.followup); hptr += r;
	}
	r = sprintf(hptr,"Mime-Version: 1.0\n"); hptr += r;
	if(multipart==0)
	{
		//strcpy(mdata->type,"text/plain");
		if(bit7)
		{
			r = sprintf(hptr,"Content-Type: text/plain; charset=us-ascii\n"); hptr += r;
			r = sprintf(hptr,"Content-Transfer-Encoding: 7bit\n"); hptr += r;
		}
		else
		{
			r=sprintf(hptr,"Content-Type: text/plain; charset=%s\n",account.charset_write); hptr += r;
			r = sprintf(hptr,"Content-Transfer-Encoding: 8bit\n"); hptr += r;
		}
	}
	else
	{
		//strcpy(mdata->type,"multipart/mixed");
		sprintf(boundary,"BOUNDARY.NC%d.%s",rand() % 65536,mdata->messageID); //#warning was %d.%d
		r = sprintf(hptr,"Content-Type: multipart/mixed; boundary=\"%s\"\n",boundary); hptr += r;
	}
	if( account.xno==1 || (account.xno==2 && write->newmessage.xno==TRUE) )
	{
		r = sprintf(hptr,"X-No-Archive: yes\n"); hptr += r;
	}
	GroupData * gdata_this = NULL;
	get_gdata(&gdata_this,write->newmessage.newsgroups);
	if(gdata_this!=NULL)
	{
		if(gdata_this->moreflags[0]==TRUE)
		{
			r = sprintf(hptr,"Approved: %s\n",gdata_this->appv_hd); hptr += r;
		}
	}
	r = sprintf(hptr,"\n"); hptr += r;
	int headerlen = hptr - header;
	//int headerlen = strlen(header);

	strncpy(mdata->from,write->newmessage.from,IHEADENDSHORT);
	mdata->from[IHEADENDSHORT] = '\0';
	strncpy(mdata->subject,write->newmessage.subject,IHEADENDSUBJECT);
	mdata->subject[IHEADENDSUBJECT] = '\0';
	strncpy(mdata->newsgroups,write->newmessage.newsgroups,IHEADENDSHORT);
	mdata->newsgroups[IHEADENDSHORT] = '\0';
	strncpy(mdata->messageID,write->newmessage.messageID,IHEADENDMID);
	mdata->messageID[IHEADENDMID] = '\0';
	//mdata->mIDhash = calculate_mID_hash(mdata->messageID);
	mdata->size = 0;
	char *attachments[1024];
	char *attachout = NULL;
	char *output = NULL;
	int len = headerlen + textlen + 8192;
	if(multipart==0)
	{
		output = header;
		strcpy(&output[headerlen],(char *)text);
	}
	else
	{
		MIMEType *mime[1024];
		int lengths[1024];
		int maxlen = 0;
		ULONG k=0;

		for(k=0;k<multipart;k++)
		{
			DoMethod(write->NLIST_write_ATT,MUIM_NList_GetEntry,k,&mime[k]);

			BPTR lock=Lock(mime[k]->file,ACCESS_READ);
			BPTR file=Open(mime[k]->file,MODE_OLDFILE);
			if(file)
			{
				lengths[k] = GetFileSize(file);
				if(lengths[k]>maxlen)
					maxlen = lengths[k];
				attachments[k] = new char[ lengths[k] + 1024 ];
				if(attachments[k] != NULL)
				{
					int read = Read(file,attachments[k],lengths[k]);
					attachments[k][read] = 0;
					len+=(lengths[k]*5)/3+1024;
				}
				else
				{
					nLog("Error! Not enough RAM for attachments!\n");
//					printf("Error! Not enough RAM for attachments!\n");
					MUI_RequestA(app,0,0,CatalogStr(MSG_ERROR,MSG_ERROR_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_NO_RAM_FOR_ATTACHMENTS,MSG_NO_RAM_FOR_ATTACHMENTS_STR),0);
				}
				Close(file);
				file=NULL;
			}
			else
			{
				sprintf(buffer,CatalogStr(MSG_CANNOT_FIND_FILE_TO_ATTACH,MSG_CANNOT_FIND_FILE_TO_ATTACH_STR),mime[k]->file);
				MUI_RequestA(app,0,0,CatalogStr(MSG_WRITE_MESSAGE,MSG_WRITE_MESSAGE_STR),CatalogStr(MSG_CONTINUE,MSG_CONTINUE_STR),buffer,0);
				mime[k]=NULL;
			}
			if(lock)
			{
				UnLock(lock);
				lock=NULL;
			}
		}
		output = new char[len];
		attachout = new char[(maxlen*5)/3+8192];
		sprintf(output,"%s\n",header);
		sprintf(output,"%sWarning: This is a message in MIME format. Your newsreader does not support\nMIME. You may still be able to read some parts as plain text, but to read\nthe rest, you need to upgrade your newsreader.\n",output);
		/*sprintf(output,"%sWarning: This is a message in MIME format. Your newsreader does not support\nMIME. You may still be able to read some parts as plain text, but to read\nthe rest, you need to upgrade your newsreader. Here are some newsreaders\nthat support MIME:\n",output);
		sprintf(output,"%s     Amiga - NewsCoaster\n       - http://members.tripod.com/Mark_Harman/newscoaster.html\n",output);
		sprintf(output,"%s     Windows - \n",output);
		sprintf(output,"%s     Unix - \n",output);*/
		if(*text!=0)
		{
			sprintf(output,"%s\n--%s\n",output,boundary);
			if(bit7)
			{
				sprintf(output,"%sContent-Type: text/plain; charset=us-ascii\n",output);
				sprintf(output,"%sContent-Transfer-Encoding: 7bit\n\n",output);
			}
			else
			{
				sprintf(output,"%sContent-Type: text/plain; charset=%s\n",output,account.charset_write);
				sprintf(output,"%sContent-Transfer-Encoding: 8bit\n\n",output);
			}
			sprintf(output,"%s%s",output,text);
		}
		for(k=0;k<multipart;k++)
		{
			if(mime[k])
			{
				sprintf(output,"%s\n--%s\n",output,boundary);
				sprintf(output,"%sContent-Type: %s; name=\"%s\"\n",output,mime[k]->type,mime[k]->shortname);
				sprintf(output,"%sContent-Disposition: attachment; filename=\"%s\"\n",output,mime[k]->file);
				sprintf(output,"%sContent-Transfer-Encoding: %s\n\n",output,"base64");
				lengths[k]=encode_base64(attachments[k],attachout,lengths[k]);
				sprintf(output,"%s%s\n",output,attachout);
			}
		}
		sprintf(output,"%s\n--%s--\n",output,boundary);
	}
	sprintf(output,"%s\r\n\n\n",output);

	//save file
	createMessage(gdata,mdata,output);

	if(multipart>0)
	{
		for(int k=0;k<multipart;k++)
		{
			if(attachments[k])
			{
				delete [] attachments[k];
				attachments[k]=NULL;
			}
		}
		if(attachout)
		{
			delete [] attachout;
			attachout=NULL;
		}
		if(output)
		{ // only if multipart, otherwise this is same as 'header'
			delete [] output;
			output=NULL;
		}
	}
	if(text)
	{
		FreeVec(text); // should be a FreeVec
		text=NULL;
	}
	if(header)
	{
		delete [] header;
		header=NULL;
	}

	write_index_single(gdata,mdata);
	setEnabled();
	gdata->nummess++;
	GroupData * gdata2 = NULL;
	getGdataDisplayed(&gdata2);
	if(gdata2->ID==gdata->ID)
		threadView();
	/*if(write->newmessage.edit==FALSE || write->newmessage.sent==TRUE) {
		if(write->newmessage.sent==TRUE)
			gdata->nummess++;
	}*/
	redrawGdataAll();
	set(write->wnd,MUIA_Window_Sleep,FALSE);
	return mdata;
}

void WriteWindow::editMess(GroupData *gdata,MessageListData *mdata,BOOL super)
{
	nLog("WriteWindow::editMess((GroupData *)%d,(MessageListData)%d,(BOOL)%d) called\n",gdata,mdata,super);
	WriteWindow *ww = new WriteWindow(CatalogStr(MSG_NEWSCOASTER_EDITING_MESSAGE,MSG_NEWSCOASTER_EDITING_MESSAGE_STR));
	set(ww->wnd,MUIA_Window_Open,TRUE);
	set(wnd_main,MUIA_Window_Sleep,TRUE);
	char buffer[1024]="";
	ww->newmessage.edit=TRUE;
	if(gdata->ID==-2)
		ww->newmessage.sent=TRUE;
	else
		ww->newmessage.sent=FALSE;
	ww->newmessage.reply=FALSE;
	get_refs(&(ww->newmessage),gdata,mdata,GETREFS_ALL);
	if(gdata->ID==-1)
		strcpy(ww->newmessage.messageID,mdata->messageID);
	else
		strcpy(ww->newmessage.messageID,"");
	if(super)
		strcpy(ww->newmessage.supersedeID,mdata->messageID);
	strcpy(buffer,ww->newmessage.newsgroups);
	set(ww->STR_write_NG,MUIA_String_Contents,buffer);
	set(ww->STR_write_TO,MUIA_String_Contents,ww->newmessage.to);
	strcpy(buffer,ww->newmessage.followup);
	set(ww->STR_write_FOLLOWUP,MUIA_String_Contents,buffer);
	set(ww->STR_write_FROM,MUIA_String_Contents,ww->newmessage.from);
	strcpy(buffer,ww->newmessage.subject);
	set(ww->STR_write_SUBJECT,MUIA_String_Contents,buffer);
	set(ww->ED_write_MESS,MUIA_TextEditor_Quiet,TRUE);
	DoMethod(ww->ED_write_MESS,MUIM_TextEditor_ClearText);
	DoMethod(ww->NLIST_write_ATT,MUIM_NList_Clear);
	if(!replyedit_mess(FALSE,ww))
	{
		delete ww;
		return;
	}
	set(wnd_main,MUIA_Window_Sleep,FALSE);
	set(ww->ED_write_MESS,MUIA_TextEditor_CursorY,0);
	set(ww->ED_write_MESS,MUIA_TextEditor_Quiet,FALSE);
	set(ww->wnd,MUIA_Window_ActiveObject,ww->ED_write_MESS);
}

void WriteWindow::cancelMess(MessageListData * mdata2)
{
	nLog("WriteWindow::cancelMess((MessageListData *)mdata2) called\n",mdata2);
	GroupData * gdata = NULL;
	GroupData * gdata_sent = NULL;
	get_gdata(&gdata_sent,-2);
	NewMessage message;
	get_refs(&message,gdata_sent,mdata2,GETREFS_NONE);
	char * header = new char[10240];

	MessageListData * mdata = new MessageListData();
	mdata->init();
	mdata->flags[0]=FALSE;
	mdata->flags[2]=FALSE;
	mdata->flags[6]=-1;

	strcpy(mdata->from,account.email);
	strcpy(mdata->newsgroups,message.newsgroups);
	mdata->size=0;
	Server *server = getPostingServer();
	create_mID(mdata->messageID,mdata->from,server->nntp);
	//mdata->mIDhash = calculate_mID_hash(mdata->messageID);
	sprintf(mdata->subject,"cancel %s",message.messageID);
	char date[256] = "";
	//DateHandler::get_datenow(mdata->date,getGMTOffset(),account.bst);
	//DateHandler::read_date(&mdata->ds,mdata->date,mdata->c_date,mdata->c_time);
	DateHandler::get_datenow(date,getGMTOffset(),account.bst);
	DateHandler::read_date(&mdata->ds,date,mdata->c_date,mdata->c_time);
	sprintf(header,"From: %s\n",mdata->from);
	sprintf(header,"%sDate: %s\n",header,date);
	sprintf(header,"%sSubject: %s\n",header,mdata->subject);
	sprintf(header,"%sMessage-ID: %s\n",header,mdata->messageID);
	sprintf(header,"%sControl: cancel %s\n",header,message.messageID);
	sprintf(header,"%sNewsgroups: %s\n",header,mdata->newsgroups);
	// email and/or newsgroup posting
	mdata->flags[4]=TRUE;
	mdata->flags[5]=FALSE;
	sprintf(header,"%sMime-Version: 1.0\n",header);
	//strcpy(mdata->type,"text/plain");
	sprintf(header,"%sContent-Type: text/plain; charset=us-ascii\n",header);
	sprintf(header,"%sContent-Transfer-Encoding: 7bit\n",header);
	if( account.xno==1 )
		sprintf(header,"%sX-No-Archive: yes\n",header);
	sprintf(header,"%s\n",header);
	char *output=NULL;
	int len = strlen(header)+8192;

	output = new char[len];
	sprintf(output,"%sThis message has been cancelled.",header);

	//save file
	get_gdata(&gdata,-1);
	createMessage(gdata,mdata,output);
	if(output) {
		delete [] output;
		output=NULL;
	}
	if(header) {
		delete [] header;
		header=NULL;
	}

	write_index_single(gdata,mdata);
	gdata->nummess++;
	GroupData * gdata2 = NULL;
	getGdataDisplayed(&gdata2);
	if(gdata2->ID==gdata->ID)
		threadView();
	redrawGdataAll();
}

void WriteWindow::create_mID(char * str,const char * email,const char * nntp)
{
	nLog("WriteWindow::create_mID((char *)%d,(char *)%s,(char *)%s) called\n",str,email,nntp);
	struct DateStamp ds;
	DateStamp(&ds);
	get_email(status_buffer_g,email,GETEMAIL_EMAIL);
	STRPTR t=strchr(status_buffer_g,'@');
	if(t)
		*t=0;
	if(*nntp!=0)
		sprintf(str,"<%d%d%d%d.NC-%s.%s@%s>",(int)ds.ds_Days,(int)ds.ds_Minute,rand() % 65536,account.sentID,PLATFORM,status_buffer_g,nntp);
	else
		sprintf(str,"<%d%d%d%d.NC-%s.%s@anywhere.com>",(int)ds.ds_Days,(int)ds.ds_Minute,rand() % 65536,account.sentID,PLATFORM,status_buffer_g);
}

BOOL WriteWindow::createMessage(GroupData *gdata,MessageListData *mdata,const char *message)
{
	nLog("WriteWindow::createMessage((GroupData *)%d,(MessageListData *)%d,(char *)%d) called\n",gdata,mdata,message);
	static char filename[MAXFILENAME]="";
	BPTR file = NULL;
	BOOL rtn = FALSE;
	for(;;) {
		sprintf(filename,"NewsCoasterData:outgoing/news_%d",gdata->nextmID);
		if(!exists(filename) && (file=Open(filename,MODE_NEWFILE)) != 0)
			break;
		gdata->nextmID++;
	}
	mdata->ID=gdata->nextmID;
	if(file) {
		int lenm=strlen(message);
		Write(file,message,lenm);
		mdata->size+=lenm;
		gdata->nextmID++;
		account.sentID++;
		redrawGdataAll();
		Close(file);
		file = NULL;
		rtn = TRUE;
	}
	return rtn;
}

BOOL WriteWindow::replyedit_mess(BOOL reply,WriteWindow * ww,GroupData * gdata,MessageListData * mdata,void *source)
{
	char charset[100];
	char *translated_string=NULL;

	nLog("WriteWindow::replyedit_mess((BOOL)%d,(WriteWindow *)%d,(GroupData *)%d,(MessageListData *)%d,(void *)%d) called\n",reply,ww,gdata,mdata,source);
	strcpy(charset,account.charset_write);
	nLog("  %d : %d",gdata->ID,mdata->ID);
	//reply: TRUE=reply, FALSE=edit
	int type = reply ? 1 : 2;

	set(ww->ED_write_MESS,MUIA_TextEditor_ExportWrap,account.linelength);
	if(mdata->flags[12]==TRUE)
	{
		//we need to download the BODY!
		/*if(delete_dis)
		{
			MUI_RequestA(app,0,0,"Downloading Message","_Okay","\33cNewsCoaster cannot download articles whilst\nother messages are already being downloaded, sorry.",0);
			return FALSE;
		}
		else*/
		BOOL available = TRUE;
		if(!getBody(&available,gdata,mdata))
			return FALSE;
	}
	// update flags to read after having displayed message, since it takes time
	int multipart=0; //0=no, 1=yes


	char cenc[64]="7bit";
	char cname[64]="";
	char cfile[64]="";
	BOOL ctype_HTML = FALSE;
	BOOL ctype_VIEW = TRUE;
	BOOL cenc_QP = FALSE;
	char ctype[256]="";

	int currpart=2;
	int cpos=0;
	/*
	strcpy(ctype,mdata->type);
	if(*ctype==0)
		strcpy(ctype,"text/plain");
	else if(stricmp(ctype,"text/html")==0)
		ctype_HTML = TRUE;
	else if(stricmp(ctype,"text/plain")!=0)
		ctype_VIEW = FALSE;
	if(strncmp("multipart/",mdata->type,10)==0) {
		multipart=1;
		strcpy(ctype,"");
		strcpy(cenc,"");
		strcpy(cname,"");
		strcpy(cfile,"");
	}
	*/
	char boundary[256]="--";

	set(ww->wnd,MUIA_Window_Sleep,TRUE);

	//char buffer[4097] = "";
	char buffer2[1251] = "";
	char buffer3[1251] = "";

	char filename[256] = "";
	getFilePath(filename,gdata->ID,mdata->ID);
	nLog("  about to open file\n");
	BPTR lock = Lock(filename,ACCESS_READ);
	BPTR file = Open(filename,MODE_OLDFILE);
	if(0==file)
	{
		sprintf(status_buffer_g,CatalogStr(MSG_CANNOT_OPEN_FILE,MSG_CANNOT_OPEN_FILE_STR),filename);
		MUI_RequestA(::app,0,0,CatalogStr(MENU_READ_MESSAGE,MENU_READ_MESSAGE_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),status_buffer_g,0);
		if(ww!=NULL)
		{
			set(ww->wnd,MUIA_Window_Sleep,FALSE);
		}
		if(lock)
			UnLock(lock);
		return FALSE;
	}


	BOOL okay=TRUE;
	BOOL resethtml=TRUE;
	char word0[256];
	char word1[256];
	STRPTR str=NULL,str2=NULL,str3=NULL,str4=NULL;
	STRPTR message=NULL;
	int read=0;

	BOOL nl=FALSE;

	BOOL quot=FALSE;
	int fibsize=0;

	char *big_buffer = new char[big_bufsize_g + 1];
	{
		fibsize=GetFileSize(file);
		message = new char[fibsize+1024];
		if(message==NULL)
		{
			sprintf(status_buffer_g,CatalogStr(MSG_NO_RAM_TO_READ_MESSAGE,MSG_NO_RAM_TO_READ_MESSAGE_STR));
			MUI_RequestA(::app,0,0,CatalogStr(MENU_READ_MESSAGE,MENU_READ_MESSAGE_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),status_buffer_g,0);
			if(file)
				Close(file);
			if(lock)
				UnLock(lock);
			delete [] big_buffer;
			return FALSE;
		}
		FFlush(file);
		// header
		for(;;)
		{
			if(!FGets(file,big_buffer,big_bufsize_g))
				break;
			if(big_buffer[0]==10 || big_buffer[0]==13)
				break;

			int word0len=wordFirstAndLen(word0,big_buffer);
			okay=FALSE;
			if(word0[word0len-1]==':')
			{
				word0[word0len-1]=0;
				if(account.readheader_type!=0)
				{
					if(account.readheader_type==2)
						okay=TRUE;
					else
					{
						if(stristr(account.readheaders,word0)!=NULL)
							okay=TRUE;
					}
				}
			}
			if(stricmp(word0,"Content-Transfer-Encoding")==0)
			{
				word(cenc,big_buffer,2);
				if(stricmp(cenc,"Quoted-Printable")==0)
					cenc_QP = TRUE;
				else
					cenc_QP = FALSE;
			}
			else if(stricmp(word0,"Content-Type")==0)
			{
				word(ctype,big_buffer,2);
				if(*ctype != '\0')
				{
					StripChar(ctype,';');
				}
			}
			//if(multipart==1)
			{
				str=strstr(big_buffer,"boundary=\"");
				if(str) {
					str+=10;
					str2=strchr(str,'\"');
					strncpy(&boundary[2],str,(int)(str2-str));
					boundary[(int)(str2-str)+2]=0;
				}
				else {
					str=strstr(big_buffer,"boundary=");
					if(str) {
						str+=9;
						str2=strchr(str,' ');
						if(str2==NULL)
							str2=&big_buffer[strlen(big_buffer)];
						strncpy(&boundary[2],str,(int)(str2-str));
						boundary[(int)(str2-str)+2]=0;
					}
				}
			}
		}

	if(*ctype=='\0')
		strcpy(ctype,"text/plain");
	else if(stricmp(ctype,"text/html")==0)
		ctype_HTML = TRUE;
	else if(stricmp(ctype,"text/plain")!=0)
		ctype_VIEW = FALSE;
	if(strncmp("multipart/",ctype,10)==0)
	{
		multipart = 1;
		strcpy(ctype,"");
		strcpy(cenc,"");
		strcpy(cname,"");
		strcpy(cfile,"");
	}
	//printf("%s\n",ctype);

		// body
		{
			nLog("  reading body\n");
			for(;;) {
				if(!FGets(file,big_buffer,big_bufsize_g))
					break;
				if(multipart==1)
					str=strstr(big_buffer,boundary);
				else
					str = NULL;
				if(str!=NULL)
				{
					if(strncmp(&str[strlen(boundary)],"--",2)==0)
					{
						strcpy(ctype,"");
						break;
					}
					strcpy(ctype,"text/plain"); // default
					ctype_VIEW = TRUE;
					ctype_HTML = FALSE;
					strcpy(cname,"Unnamed");
					for(;;)
					{ // sub-header
						if(!FGets(file,big_buffer,big_bufsize_g))
							break;
						StripNewLine(big_buffer);
						if(*big_buffer==0)
							break;
						wordFirst(word1,big_buffer);
						if(0 != *word1)
						{
							if(stricmp(word1,"Content-Type:")==0)
							{
								word(ctype,big_buffer,2);
								if(0 != *ctype) {
									StripChar(ctype,';');
									if(stricmp(ctype,"text/plain")==0)
									{
										ctype_VIEW = TRUE;
										ctype_HTML = FALSE;
									}
									else if(stricmp(ctype,"text/html")==0)
									{
										ctype_VIEW = TRUE;
										ctype_HTML = TRUE;
									}
									else
									{
										ctype_VIEW = FALSE;
										ctype_HTML = FALSE;
									}
								}
								strcpy(buffer2,big_buffer);
								str=strstr(buffer2,"name=");
								if(str) {
									str2=strchr(str,' ');
									if(str2==NULL)
										str2=&str[strlen(str)];
									strncpy(buffer3,str,6);
									buffer3[6]=0;
									if(stricmp(buffer3,"name=\"")==0)
									{
										str2[-1]=0;
										strcpy(cname,&str[6]);
									}
									else
									{
										buffer3[5]=0;
										if(stricmp(buffer3,"name=")==0)
										{
											*str2=0;
											strcpy(cname,&str[5]);
										}
									}
								}
							}
							if(stricmp(word1,"Content-Disposition:")==0)
							{
								strcpy(buffer2,big_buffer);
								str=strstr(buffer2,"filename=");
								if(str) {
									str2=strchr(str,' ');
									if(str2==NULL)
										str2=&str[strlen(str)];
									strncpy(buffer3,str,10);
									buffer3[10]=0;
									if(stricmp(buffer3,"filename=\"")==0)
									{
										str2[-1]=0;
										strcpy(cfile,&str[10]);
									}
									else
									{
										buffer3[9]=0;
										if(stricmp(buffer3,"filename=")==0)
										{
											*str2=0;
											strcpy(cfile,&str[9]);
										}
									}
								}
							}
							if(stricmp(word1,"Content-Transfer-Encoding:")==0)
							{
								word(cenc,big_buffer,2);
								if(stricmp(cenc,"Quoted-Printable")==0)
									cenc_QP = TRUE;
								else
									cenc_QP = FALSE;

								if(stricmp(cenc,"base64")==0)
									ctype_VIEW = FALSE;
							}
						}
					}
					if(type==2)
					{
						if(ctype_VIEW == FALSE)
						{
							MIMEType *mime = new MIMEType;
							strcpy(mime->file,cfile);
							exfile(mime);
							strcpy(mime->type,ctype);
							DoMethod(ww->NLIST_write_ATT,MUIM_NList_InsertSingle,mime,MUIV_NList_Insert_Bottom);
						}
					}
				}
				else
				{
					nl=TRUE;
					if(type==1)
						nl=FALSE;
					if(cenc_QP)
					{
						decode_qprint(big_buffer,buffer2,strlen(big_buffer),nl);
						strcpy(big_buffer,buffer2);
					}
					if(ctype_HTML)
					{
						parse_html(big_buffer,buffer2,strlen(big_buffer),resethtml);
						resethtml=FALSE;
						strcpy(big_buffer,buffer2);
					}
					if(ctype_VIEW)
					{
						if(type==1)
						{
							strcpy(&message[cpos],big_buffer);
							cpos+=strlen(big_buffer);
						}
						else if(type==2)
						{
							strcpy(&message[cpos],big_buffer);
							cpos+=strlen(big_buffer);
						}
					}
				}
			}
			nLog("  done : %d\n",cpos);
		}
	}
	message[cpos]=0;
	str=message;
	if((account.flags & Account::SNIPSIG)!=0 && type==1)
	{
		str3=strstr(message,"\n-- \n");
		if(str3==NULL) {
			if(strncmp(message,"-- \n",4)==0)
				str3=message;
		}
		if(str3==NULL)
			str3=&message[cpos-1];
	}
	else
		str3=&message[cpos-1];
	if(str3>message) {
		translated_string=translateCharset((unsigned char *)message,charset);
		if (translated_string)
			message = translated_string;
		set(ww->ED_write_MESS,MUIA_TextEditor_Quiet,TRUE);
		if(account.rewrap==1) {
			quot=FALSE;
			do {
				while(*str=='\n')
					str++;
				if(*str=='>' || *str=='|' || *str==':' || *str=='!')
					quot=TRUE;
				else
					quot=FALSE;
				str2=strchr(str,'\n');
				if(str2==NULL || str2>str3)
					break;
				if(str2-str<40) { // this line is less than 40 chars in width, so probably an intended newline
					str=str2+1;
				}
				else {
					if(!isspace(str2[1]) && str2[1]!='>' && str2[1]!='|' && str2[1]!=':' && str2[1]!='!') {
						if(quot==FALSE)
							*str2=' ';
						else
							quot=FALSE;
						str=str2+1;
					}
					else {
						str=str2+1;
						while(*str=='\n')
							str++;
					}
				}
				if(str[0]==0)
					break;
			} while(1);
			str=message;
			if(type==1) {
				do {
					str2=strchr(str,'\n');
					if(*str!='>' && *str!='|' && *str!=':' && *str!='!') {
						if(str2>str+account.linelength-4 || str2==NULL) {
							str2=str+account.linelength-4; // get a line length
							str4=str;
							do { // cope with tabs
								if(*str4++=='\t') {
									str2-=TAB;
								}
							} while(str4<=str2);
							while(*str2>32) // go to previous whitespace
								str2--;
							if(str2<=str) // line is bigger than line width
								str2=str+account.linelength-4; // so split it
						}
					}
					if(str2>str3) // past end of message
						str2=str3;
					read=(int)(str2-str+1);
					strncpy(big_buffer,str,read);
					if(*str2=='\n')
						big_buffer[read]=0;
					else {
						if( read > 0 && isspace(big_buffer[read-1]) ) {
							big_buffer[read-1] = '\0';
							read--;
						}
						big_buffer[read]='\n';
						big_buffer[read+1]=0;
					}
					DoMethod(ww->ED_write_MESS,MUIM_TextEditor_InsertText,"> ",MUIV_TextEditor_InsertText_Bottom);
					DoMethod(ww->ED_write_MESS,MUIM_TextEditor_InsertText,big_buffer,MUIV_TextEditor_InsertText_Bottom);
					str=str2+1;
				} while(str2<str3);
			}
			else
				DoMethod(ww->ED_write_MESS,MUIM_TextEditor_InsertText,message,MUIV_TextEditor_InsertText_Bottom);
		}
		else {
			if(type==1) {
				do {
					str2=strchr(str,'\n');
					if(str2>str3 || str2==NULL)
						str2=str3;
					read=(int)(str2-str+1);
					strncpy(big_buffer,str,read);
					big_buffer[read]=0;
					DoMethod(ww->ED_write_MESS,MUIM_TextEditor_InsertText,"> ",MUIV_TextEditor_InsertText_Bottom);
					DoMethod(ww->ED_write_MESS,MUIM_TextEditor_InsertText,big_buffer,MUIV_TextEditor_InsertText_Bottom);
					str=str2+1;
				} while(str2!=str3);
			}
			else
				DoMethod(ww->ED_write_MESS,MUIM_TextEditor_InsertText,message,MUIV_TextEditor_InsertText_Bottom);
		}
	}
	nLog("  about to close file\n");
	if(file) {
		Close(file);
		file=NULL;
	}
	if(lock) {
		UnLock(lock);
		lock=NULL;
	}
	set(ww->wnd,MUIA_Window_Sleep,FALSE);
	nLog("  about to delete parts ( %d )\n",currpart);
	if(message) {
		delete [] message;
		message=NULL;
	}
	// update flags to READ
	if(mdata->flags[1]==TRUE) {
		mdata->flags[1]=FALSE; // no longer NEW
		gdata->num_unread--;
		update_screen_title(gdata);
		redrawGdata(gdata);

		if(gdata->flags[6]!=0)
			gdata->flags[1]=TRUE; // still mark to rewrite index if we are discarding messages
		nLog("  about to call redrawMdataActive()..\n");
		redrawMdataActive();
		nLog("  Succeeded!\n");
		write_index_update(gdata,mdata,source);

	}
	delete [] big_buffer;
	nLog("  finished replyedit_mess()\n");
	return TRUE;
}

BOOL WriteWindow::replyedit_mess(BOOL reply,WriteWindow * ww)
{
	GroupData * gdata = NULL;
	MessageListData * mdata = NULL;
	getGdataDisplayed(&gdata);
	getMdataActive(&mdata);
	return WriteWindow::replyedit_mess(reply,ww,gdata,mdata,NLIST_messagelistdata);
}

void WriteWindow::reply(GroupData * gdata,MessageListData *mdata,void *source,BOOL followup,BOOL reply)
{
	nLog("WriteWindow::reply((GroupData *)%d,(MessageListData *)%d,(void *)%d,(BOOL)%d,(BOOL)%d) called\n",gdata,mdata,source,followup,reply);
	int result = 0;
	char buffer[NEWMESS_long+3]="";
	char buffer2[NEWMESS_long+1]="";
	char temp[1024] = "";
	struct DateStamp ds;
	const int stupidage = 90;
	char *newrefptr = new char[257];

	DateStamp(&ds);
	if(ds.ds_Days - mdata->ds.ds_Days > stupidage)
	{
		// stupidity warning - warns against replying to old posts
		MUI_RequestA(::app,0,0,CatalogStr(MSG_OLD_MESSAGE,MSG_OLD_MESSAGE_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_REPLYING_TO_AN_OLD_POST,MSG_REPLYING_TO_AN_OLD_POST_STR),0);
	}

	WriteWindow *ww = new WriteWindow("NewsCoaster");
	DoMethod(ww->NLIST_write_ATT,MUIM_NList_Clear);
			/*if(gdata->ID >= 0)
				set(ww->CY_write_SIG,MUIA_Cycle_Active,gdata->defsig+1);
			else
				set(ww->CY_write_SIG,MUIA_Cycle_Active,1);*/
	set(ww->STR_write_FOLLOWUP,MUIA_String_Contents,"");
	set(ww->STR_write_FROM,MUIA_String_Contents,mdata->newsgroups);

	getUserEmail(gdata,buffer);
	set(ww->STR_write_FROM,MUIA_String_Contents,buffer);
	strcpy(ww->newmessage.getThisHeader,"reply-to:");
	get_refs(&(ww->newmessage),gdata,mdata,GETREFS_ALL);
	if(reply==FALSE && stristr_q(ww->newmessage.followup,"POSTER")!=NULL)
	{
		result=MUI_RequestA(::app,0,0,CatalogStr(MSG_WRITE_MESSAGE,MSG_WRITE_MESSAGE_STR),CatalogStr(MSG_FOLLOWUP_OR_REPLY_2,MSG_FOLLOWUP_OR_REPLY_2_STR),CatalogStr(MSG_EMAIL_REPLY_REQUESTED,MSG_EMAIL_REPLY_REQUESTED_STR),0);
		switch(result)
		{
			case 1:
				reply=FALSE;
				followup=TRUE;
				break;
			case 0:
				reply=TRUE;
				followup=FALSE;
				break;
		}
	}
	//printf("%d\n",ww->newmessage.nrefs);
	ww->newmessage.references[ww->newmessage.nrefs++] = newrefptr;
	strncpy(newrefptr,ww->newmessage.messageID,256);
	newrefptr[256] = '\0';
	//printf("%d:%s\n",ww->newmessage.references[0],ww->newmessage.references[0]);
	ww->newmessage.edit=FALSE;
	ww->newmessage.reply=TRUE;
	ww->newmessage.replied_gdataID=gdata->ID;
	ww->newmessage.replied_mdataID=mdata->ID;
	strcpy(ww->newmessage.messageID,"");
	if(followup)
	{
		if(*(ww->newmessage.followup)!=0 && stristr_q(ww->newmessage.followup,"POSTER")==NULL)
		{
			strcpy(buffer,ww->newmessage.followup);
		}
		else
			strcpy(buffer,ww->newmessage.newsgroups);

		set(ww->STR_write_NG,MUIA_String_Contents,buffer);
	}
	else
		set(ww->STR_write_NG,MUIA_String_Contents,"");
	if(reply)
	{
		if(*(ww->newmessage.replyto)!=0)
			strcpy(buffer,ww->newmessage.replyto);
		else
			strcpy(buffer,ww->newmessage.from);
		if(*(ww->newmessage.dummyHeader) != '\0')
			strcpy(buffer,ww->newmessage.dummyHeader);
		else
			strcpy(buffer,ww->newmessage.from);
		set(ww->STR_write_TO,MUIA_String_Contents,buffer);
	}
	else
		set(ww->STR_write_TO,MUIA_String_Contents,"");

	if((stricmp(mdata->subject,"Re:")>0) || (stricmp(mdata->subject,"Sv:")>0))
		strcpy(buffer,mdata->subject);
	else
		sprintf(buffer,"Re: %s",mdata->subject);

	set(ww->STR_write_SUBJECT,MUIA_String_Contents,buffer);
	set(ww->wnd,MUIA_Window_Open,TRUE);
	set(ww->wnd,MUIA_Window_Sleep,TRUE);
	set(ww->ED_write_MESS,MUIA_TextEditor_Quiet,TRUE);
	DoMethod(ww->ED_write_MESS,MUIM_TextEditor_ClearText);
	set(ww->wnd,MUIA_Window_ActiveObject,ww->ED_write_MESS);
	if(!WriteWindow::replyedit_mess(TRUE,ww,gdata,mdata,source))
	{
		//set(ww->wnd,MUIA_Window_Sleep,FALSE);
		//set(ww->wnd,MUIA_Window_Open,FALSE);
		delete ww;
		return;
	}
	// insert .sig
	/*if(gdata->defsig>=0) {
		strcpy(buffer,"-- \n");
		DoMethod(ww->ED_write_MESS,MUIM_TextEditor_InsertText,buffer,MUIV_TextEditor_InsertText_Bottom);
		if(gdata->ID>=0 && sigs[gdata->defsig]!=NULL)
			DoMethod(ww->ED_write_MESS,MUIM_TextEditor_InsertText,sigs[gdata->defsig],MUIV_TextEditor_InsertText_Bottom);
		else {
			if(sigs[0])
				DoMethod(ww->ED_write_MESS,MUIM_TextEditor_InsertText,sigs[0],MUIV_TextEditor_InsertText_Bottom);
		}
	}*/
	if(gdata->ID >= 0)
		set(ww->CY_write_SIG,MUIA_Cycle_Active,gdata->defsig+1);
	else
		set(ww->CY_write_SIG,MUIA_Cycle_Active,1);

	get_email(buffer2,ww->newmessage.from,GETEMAIL_NAMEEMAIL);
	char *strptr[16];
	char fbuffer[256];
	char percent[] = "%";
	sprintf(fbuffer,"%s\n\n",account.followup_text);
	char *ptr = fbuffer;
	int cptr=0;
	for(;;)
	{
		ptr = strchr(ptr,'%');
		if(ptr==NULL)
			break;
		ptr++;
		BOOL okay=TRUE;
		if(cptr<16) {
			if(*ptr=='n')
				strptr[cptr++] = buffer2;
			else if(*ptr=='d')
				strptr[cptr++] = ww->newmessage.date;
				//strptr[cptr++] = mdata->date;
			else if(*ptr=='g')
				strptr[cptr++] = mdata->newsgroups;
			else if(*ptr=='m')
				strptr[cptr++] = mdata->messageID;
			else if(*ptr=='%')
				strptr[cptr++] = percent;
			else
				okay=FALSE;
		}
		else
			okay=FALSE;
		if(okay)
			*ptr='s';
		else {
			ptr[-1] = ' ';
			ptr[0] = ' ';
		}
	}
	sprintf(buffer,fbuffer,strptr[0],
		strptr[1],
		strptr[2],
		strptr[3],
		strptr[4],
		strptr[5],
		strptr[6],
		strptr[7],
		strptr[8],
		strptr[9],
		strptr[10],
		strptr[11],
		strptr[12],
		strptr[13],
		strptr[14],
		strptr[15]);
	set(ww->ED_write_MESS,MUIA_TextEditor_CursorX,0);
	set(ww->ED_write_MESS,MUIA_TextEditor_CursorY,0);
	DoMethod(ww->ED_write_MESS,MUIM_TextEditor_InsertText,buffer,MUIV_TextEditor_InsertText_Cursor);
	set(ww->ED_write_MESS,MUIA_TextEditor_CursorY,0);
	set(ww->ED_write_MESS,MUIA_TextEditor_Quiet,FALSE);
	set(ww->wnd,MUIA_Window_Sleep,FALSE);
	/*}
	else
		MUI_RequestA(app,0,0,CatalogStr(MENU_WRITE_MESSAGE,MENU_WRITE_MESSAGE_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),"\33cYou may only write one message\nat a time.\nPlease finish/cancel the\nexisting message.",0);*/
}

void WriteWindow::reply(BOOL followup,BOOL reply)
{
	GroupData * gdata = NULL;
	MessageListData * mdata = NULL;
	int result = 0;
	getMdataActivePos(&result);
	if(result!=MUIV_NList_Active_Off)
	{
		getGdataDisplayed(&gdata);
		getMdataActive(&mdata);
		WriteWindow::reply(gdata,mdata,NLIST_messagelistdata,followup,reply);
	}
	else
		MUI_RequestA(::app,0,0,CatalogStr(MSG_WRITE_MESSAGE,MSG_WRITE_MESSAGE_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_YOU_MUST_SELECT_MESSAGE_TO_REPLY,MSG_YOU_MUST_SELECT_MESSAGE_TO_REPLY_STR),0);
}

void WriteWindow::freeStatics()
{
	nLog("WriteWindow::freeStatics() called\n");
	int size=ptrs->getSize();
	WriteWindow ** data = (WriteWindow **)ptrs->getData();
	for(int i=0;i<size;i++)
		delete data[i];
	delete ptrs;

	if(editor_mcc_write != NULL)
	{
		nLog("  about to dispose of WriteWindow TextEditor Custom Class\n");
		MUI_DeleteCustomClass(editor_mcc_write);
	}
}
