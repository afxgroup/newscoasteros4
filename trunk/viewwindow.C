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
#include <mui/NListtree_mcc.h>
#include <proto/locale.h>

#include <ctype.h>

#include "vector.h"
#include "various.h"
#include "viewwindow.h"
#include "writewindow.h"
#include "misc.h"
#include "main.h"
#include "indices.h"
#include "statuswindow.h"
#include "netstuff.h"
#include "general.h"
#include "strings.h"
#include "lists.h"
#include "newscoaster_catalog.h"

static LONG v_cmap[9];

#define MUIA_TextEditor_AutoClip (TextEditor_Dummy + 0x34)

#define VIEWWINDOWEDITOR_VIEWWINDOW 0x8022

// The max file name length is now a define.
// This should probably move to misc.h if the filename length is used elsewhere.
#define FILE_NAME_LENGTH 300

const int SEARCHSTRINGLEN = 120;

static const char *CYA_view_app[] = {
	"EMail Format","Plain Text",NULL
};

// static char *Pages_view[]   = {"Message","Attachments",NULL };

/* Launches the ARexx script when the user double clicks on some text at
 * 'url'.
 */
void LookupURL(char * url) {
	nLog("LookUpURL((char *)%s) called\n",url);
	//printf("LookUpURL((char *)%s) called\n",url);
	char command[1024]="";
	strcpy(command,"run sys:rexxc/rx newscoaster:gotoURL.rx \"");
	char * pos=&command[strlen(command)];
	while(0 != *url && !isspace(*url) )
		*pos++ = *url++;
	*pos++ = '\"';
	*pos = '\0';
	nLog("  executing: %s\n",command);
	//printf("  executing: %s\n",command);
	//Execute(command,NULL,NULL);
	SystemTags(command,
					SYS_Asynch, TRUE,
					NP_Name,    "NewsCoaster Process",
					TAG_END);
}


/* Rates how much a particular MIME type is preferred by NewsCoaster - used
 * for multipart/alternative messages, in order to decide what type we
 * should display.
 */
int rate(char *type) {
	//nLog("rate((char *)%s) called\n",type);
	int rate = 0;
	if(iequals("text/plain",type))
        	rate = 12;
	else if(iequals("text/richtext",type))
        	rate = 11;
	else if(iequals("text/html",type))
        	rate = 10;

	else if(iequals("application/x-lha",type))
        	rate = 3;
	else if(iequals("application/x-lzx",type))
        	rate = 2;
	else if(iequals("application/x-zip",type))
        	rate = 1;

	else if(iequals("video/mpeg",type))
        	rate = 5;
	else if(iequals("video/quicktime",type))
        	rate = 4;
	else if(iequals("image/png",type))
        	rate = 3;
	else if(iequals("image/jpeg",type))
        	rate = 2;
	else if(iequals("image/gif",type))
        	rate = 1;

	else if(iequals("audio/x-wav",type))
        	rate = 1;

	else if(iequals("application/octet-stream",type))
        	rate = 2;
	else if(iequals("application/postscript",type))
        	rate = 1;

	//nLog("  returning %d\n",rate);
   return rate;
}

/* Constructor for MimeSection. Local copies are taken of the strings.
 */
MimeSection::MimeSection(char *start,int length,char *name,char *mimetype,char *encoding,char *boundary) {
	nLog("MimeSection::MimeSection((char *)%d,(int)%d,(char *)%s,(char *)%s,(char *)%s,(char *)%s) called\n",start,length,name,mimetype,encoding,boundary);
	this->start = start;
	this->length = length;
	strncpy(this->name,name,MIMESECTION_LEN);
		this->name[MIMESECTION_LEN] = '\0';
	strncpy(this->mimetype,mimetype,MIMESECTION_LEN);
		this->mimetype[MIMESECTION_LEN] = '\0';
	strncpy(this->encoding,encoding,MIMESECTION_LEN);
		this->encoding[MIMESECTION_LEN] = '\0';
	strncpy(this->boundary,boundary,MIMESECTION_LEN);
		this->boundary[MIMESECTION_LEN] = '\0';
}

BOOL is_uuencoded(char *enc) {
	if(stricmp(enc,"x-uuencode")==0
		|| stricmp(enc,"uuencoded")==0
		|| stricmp(enc,"x-uue")==0 )
		return TRUE;
	else
		return FALSE;
}

/* Saves this attachment to file 'filename', performing any necessary
 * decoding as indicated by the encoding type.
 */
void MimeSection::saveToFile(char *filename)
{
	BPTR file=Open(filename,MODE_NEWFILE);
	BOOL fr = FALSE;
	char *ptr = this->start;
	int len = this->length;

	if(stricmp(this->encoding,"base64")==0)
	{
		char *tptr = ptr;
		ptr = new char[len + 1024];
		len = decode_base64(tptr,ptr,len);
		if(fr)
			delete [] tptr;
		fr = TRUE;
	}
	else if(stricmp(this->encoding,"quoted-printable")==0)
	{
		char *tptr = ptr;
		ptr = new char[length*2];
		decode_qprint(tptr,ptr,length,TRUE);
		if(fr)
			delete [] tptr;
		fr = TRUE;
	}
	else if(is_uuencoded(this->encoding))
	{
		uuEncodeData uudata;
		char *tptr = ptr;
		ptr = new char[len + 1024];
		len = uudecode(tptr,ptr,len,&uudata);
		if(fr)
			delete [] tptr;
		fr = TRUE;
	}
	else if(stricmp(this->encoding,"yencoded")==0)
	{
		uuEncodeData uudata;
		char *tptr = ptr;
		ptr = new char[len + 1024];
		BOOL corrupt = FALSE;
		len = ydecode(&corrupt,tptr,ptr,len,&uudata);
		if(fr)
			delete [] tptr;
		fr = TRUE;
		if(corrupt)
		{
			sprintf(status_buffer_g,CatalogStr(MSG_ATACHMENT_CORRUPTED,MSG_ATACHMENT_CORRUPTED_STR),filename);
			MUI_RequestA(app,NULL,0,CatalogStr(MSG_YDECODED_ATTACH,MSG_YDECODED_ATTACH_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),status_buffer_g,0);
		}
	}
	Write(file,ptr,len);
	Close(file);
	if(fr)
		delete [] ptr;
}

HOOKCL3( ULONG, ViewWindow::TextEditor_Dispatcher_view, IClass *, cl, a0, Object *, obj, a2, Msg, msg, a1 )
	ViewData *vd = (ViewData *)INST_DATA(cl,obj);
	ViewWindow *vw = vd->vw;
	MUIP_DragDrop *drop_msg=NULL;
	MUIP_TextEditor_HandleError *err_msg=NULL;
	MUIP_HandleInput *input_msg=NULL;
	switch(msg->MethodID) {
		case MUIM_HandleInput:
			input_msg=(MUIP_HandleInput *)msg;
			if(input_msg) {
				switch (input_msg->muikey) {
					case MUIKEY_LEFT:
						//DoMethod(obj, MUIM_CallHook, &prevHook, vw );
						DoMethod(obj, MUIM_CallHook, &hook_standard, prevFunc, vw );
						break;
					case MUIKEY_RIGHT:
						//DoMethod(obj, MUIM_CallHook, &nextHook, vw );
						DoMethod(obj, MUIM_CallHook, &hook_standard, nextFunc, vw );
						break;
					case 1:
					{
						if(vd->vw->ED_view_MESS != NULL) {
							int lines = 0;
							get(vd->vw->ED_view_MESS,MUIA_TextEditor_Prop_Visible,&lines);
							int cy = 0;
							get(vd->vw->ED_view_MESS,MUIA_TextEditor_CursorY,&cy);
							if(cy<lines)
								set(vd->vw->ED_view_MESS,MUIA_TextEditor_CursorY,2*lines-1);
							else
								set(vd->vw->ED_view_MESS,MUIA_TextEditor_CursorY,cy+lines);
							break;
						}
					}
				}
			}
			break;
		case OM_SET:
		{
			TagItem *tags = NULL,*tag = NULL;
			for(tags=((opSet *)msg)->ops_AttrList;tag=NextTagItem(&tags);) {
				switch(tag->ti_Tag) {
				case VIEWWINDOWEDITOR_VIEWWINDOW:
					vd->vw = (ViewWindow *)tag->ti_Data;
					break;
				}
			}
			break;
		}
		case MUIM_Show:
         v_cmap[6] = MUI_ObtainPen(muiRenderInfo(obj), &account.pen_acc_text_col, 0);
         v_cmap[7] = MUI_ObtainPen(muiRenderInfo(obj), &account.pen_acc_text_quote2, 0);
			break;
		case MUIM_Hide:
         if (v_cmap[6] >= 0) MUI_ReleasePen(muiRenderInfo(obj), v_cmap[6]);
         if (v_cmap[7] >= 0) MUI_ReleasePen(muiRenderInfo(obj), v_cmap[7]);
         v_cmap[6] = 0;
         v_cmap[7] = 0;
         break;
		case MUIM_DragQuery:
			return FALSE;
			break;
		case MUIM_TextEditor_HandleError:
			char *errortxt = NULL;
			err_msg = (MUIP_TextEditor_HandleError *)msg;
			switch(err_msg->errorcode) {
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
			if(errortxt)
				MUI_Request(app,NULL, 0L, NULL, CatalogStr(MSG_CONTINUE,MSG_CONTINUE_STR), errortxt);
			break;
	}
	return DoSuperMethodA(cl,obj,msg);
}

/* A safe way to delete the class from a Hook called with an object
 * belonging to the window. We can't do a 'delete vw' or whatever from
 * within the Hook, since the destructor calls MUI_DisposeObject on the MUI
 * objects associated with the class. Instead, use:
 *     DoMethod(app, MUIM_Application_PushMethod, app, 3, MUIM_CallHook,
 *         &disposeHook, vw );
 */
//HOOKCL3( LONG, ViewWindow::disposeFunc, CPPHook *, hook, a0, Object *, object, a2, ViewWindow **, W, a1 )
void ViewWindow::disposeFunc(ViewWindow **W) {
	nLog("ViewWindow::disposeFunc() called\n");
	//printf("ViewWindow::disposeFunc() called\n");
	ViewWindow * vw = *W;
	delete vw;
	//return 0;
}

/* Called when the window is closed by the user.
 */
HOOKCL3( LONG, ViewWindow::closeFunc, CPPHook *, hook, a0, Object *, object, a2, ViewWindow **, W, a1 )
	nLog("ViewWindow::closeFunc() called\n");
	//printf("ViewWindow::closeFunc() called\n");
	ViewWindow * vw = *W;
	//delete vw;
	DoMethod(app, MUIM_Application_PushMethod, app, 4, MUIM_CallHook, &hook_standard, disposeFunc, vw );
	return 0;
}

/* Called when the user requests to create a killfile based on this message.
 */
//HOOKCL3( LONG, ViewWindow::killFunc, CPPHook *, hook, a0, Object *, object, a2, ViewWindow **, W, a1 )
void ViewWindow::killFunc(ViewWindow **W) {
	nLog("ViewWindow::killFunc() called\n");
	ViewWindow * vw = *W;
	GroupData * gdata=NULL;
	MessageListData * mdata=NULL;
	get_gdata(&gdata,vw->gID);
	BOOL del = get_mdata(&mdata,gdata,vw->mIDs[vw->cmptr],FALSE);
	killmess(gdata,mdata);
	if(del) {
		delete mdata;
		mdata = NULL;
	}
	//return 0;
}

//HOOKCL3( LONG, ViewWindow::prevFunc, CPPHook *, hook, a0, Object *, object, a2, ViewWindow **, W, a1 )
void ViewWindow::prevFunc(ViewWindow **W) {
	nLog("ViewWindow::prevFunc() called\n");
	ViewWindow * vw = *W;
	vw->cmptr--;
	if(vw->cmptr==-1) {
		DoMethod(app, MUIM_Application_PushMethod, app, 4, MUIM_CallHook, &hook_standard, disposeFunc, vw );
		//delete vw;
		//return 0;
		return;
	}
	else {
		vw->readheader_type = account.readheader_type;
		if(!vw->read()) {
			DoMethod(app, MUIM_Application_PushMethod, app, 4, MUIM_CallHook, &hook_standard, disposeFunc, vw );
			//delete vw;
			//return 0;
			return;
		}
	}
	//return 0;
}

//HOOKCL3( LONG, ViewWindow::prevtFunc, CPPHook *, hook, a0, Object *, object, a2, ViewWindow **, W, a1 )
void ViewWindow::prevtFunc(ViewWindow **W) {
	nLog("ViewWindow::prevtFunc() called\n");
	ViewWindow * vw = *W;
	GroupData * gdata=NULL;
	get_gdata(&gdata,vw->gID);
	GroupData * cdg = NULL;
	getGdataDisplayed(&cdg);
	MessageListData *mdata = NULL;
	sleepAll(TRUE);
	BOOL del = get_mdata(&mdata,gdata,vw->mIDs[vw->cmptr],FALSE);
	if( mdata != NULL && *mdata->lastref != '\0' ) {
		MessageListData * parent_mdata = NULL;
		Vector *vector = NULL;

		if(cdg->ID == gdata->ID) {
			int entries = 0;
			get(NLIST_messagelistdata,MUIA_NList_Entries,&entries);
			for(int l=0;l<entries;l++) {
				MessageListData * mdata2 = NULL;
				DoMethod(NLIST_messagelistdata,MUIM_NList_GetEntry,l,&mdata2);
				if(strcmp(mdata2->messageID,mdata->lastref)==0) {
					parent_mdata = mdata2;
					break;
				}
			}
		}
		else {
			vector = new Vector(2048);
			if( read_index(gdata, vector) ) {
				for(int i=0;i<vector->getSize();i++) {
					MessageListData *mdata2 = ((MessageListData **)vector->getData())[i];
					if(strcmp(mdata2->messageID,mdata->lastref)==0) {
						parent_mdata = mdata2;
						break;
					}
				}
			}
		}
		if(parent_mdata != NULL) {
			vw->setID(vw->gID,parent_mdata->ID);
			vw->readheader_type = account.readheader_type;
			if(!vw->read(parent_mdata)) {
				DoMethod(app, MUIM_Application_PushMethod, app, 4, MUIM_CallHook, &hook_standard, disposeFunc, vw );
			}
		}
		else
			set(vw->BT_view_PREVT,MUIA_Disabled,TRUE);

		if(vector != NULL) {
			vector->flush();
			delete vector;
		}
	}
	else
		set(vw->BT_view_PREVT,MUIA_Disabled,TRUE);

	sleepAll(FALSE);
	if(mdata != NULL && del)
		delete mdata;

	//return 0;
}

//HOOKCL3( LONG, ViewWindow::delnextFunc, CPPHook *, hook, a0, Object *, object, a2, ViewWindow **, W, a1 )
void ViewWindow::delNextFunc(ViewWindow **W) {
	nLog("ViewWindow::delnextFunc() called\n");
	ViewWindow * vw = *W;
	GroupData * gdata=NULL;
	GroupData * gdata2=NULL;
	MessageListData * mdata=NULL;
	get_gdata(&gdata,vw->gID);
	BOOL del = get_mdata(&mdata,gdata,vw->mIDs[vw->cmptr],TRUE);
	vw->noclose=TRUE; // so this window doesn't get closed when deleting, but others do!
	if(gdata->ID!=-3) {
		get_gdata(&gdata2,-3);
		move_n(gdata,gdata2,&mdata,1,0,NULL,FALSE);
	}
	else {
		delete_mess_n(gdata,&mdata,1,FALSE);
	}
	vw->noclose=FALSE;
	vw->nIDs--;
	for(int i=vw->cmptr;i<vw->nIDs;i++)
		vw->mIDs[i] = vw->mIDs[i+1];
	// leave cmptr as it is
	if(vw->cmptr==vw->nIDs) {
		DoMethod(app, MUIM_Application_PushMethod, app, 4, MUIM_CallHook, &hook_standard, disposeFunc, vw );
		//delete vw;
	}
	else {
		vw->readheader_type = account.readheader_type;
		if(!vw->read()) {
			DoMethod(app, MUIM_Application_PushMethod, app, 4, MUIM_CallHook, &hook_standard, disposeFunc, vw );
			//delete vw;
		}
	}

	if(del) {
		delete mdata;
		mdata = NULL;
	}
	//return 0;
}

//HOOKCL3( LONG, ViewWindow::nextFunc, CPPHook *, hook, a0, Object *, object, a2, ViewWindow **, W, a1 )
void ViewWindow::nextFunc(ViewWindow **W) {
	nLog("ViewWindow::nextFunc() called\n");
	ViewWindow * vw = *W;
	vw->cmptr++;
	if(vw->cmptr==vw->nIDs) {
		DoMethod(app, MUIM_Application_PushMethod, app, 4, MUIM_CallHook, &hook_standard, disposeFunc, vw );
		//delete vw;
		//return 0;
		return;
	}
	else {
		vw->readheader_type = account.readheader_type;
		if(!vw->read()) {
			DoMethod(app, MUIM_Application_PushMethod, app, 4, MUIM_CallHook, &hook_standard, disposeFunc, vw );
			//delete vw;
			//return 0;
			return;
		}
	}
	//return 0;
}

//HOOKCL3( LONG, ViewWindow::replyFunc, CPPHook *, hook, a0, Object *, object, a2, ViewWindow **, W, a1 )
void ViewWindow::replyFunc(ViewWindow **W) {
	nLog("ViewWindow::replyFunc() called\n");
	ViewWindow * vw = *W;
	GroupData * gdata=NULL;
	MessageListData * mdata=NULL;
	get_gdata(&gdata,vw->gID);
	BOOL del = get_mdata(&mdata,gdata,vw->mIDs[vw->cmptr],TRUE);
	void *source = del ? NULL : NLIST_messagelistdata;
	int result=MUI_RequestA(app,vw->wnd,0,CatalogStr(MSG_FOLLOWUP_REPLY,MSG_FOLLOWUP_REPLY_STR),CatalogStr(MSG_FOLLOWUP_REPLY_OR_CANCEL,MSG_FOLLOWUP_REPLY_OR_CANCEL_STR),CatalogStr(MSG_FOLLOWUP_OR_REPLY,MSG_FOLLOWUP_OR_REPLY_STR),0);
	switch(result)
	{
		case 0:
			break;
		case 1:
			WriteWindow::reply(gdata,mdata,source,TRUE,FALSE);
			break;
		case 2:
			WriteWindow::reply(gdata,mdata,source,FALSE,TRUE);
			break;
		/*case 3:
			WriteWindow::reply(gdata,mdata,TRUE,TRUE);
			break;*/
	}

	if(del) {
		delete mdata;
		mdata = NULL;
	}
	//return 0;
}

//HOOKCL3( LONG, ViewWindow::expFunc, CPPHook *, hook, a0, Object *, object, a2, ViewWindow **, W, a1 )
void ViewWindow::expFunc(ViewWindow **W) {
	nLog("ViewWindow::expFunc() called\n");
	ViewWindow * vw = *W;
	ViewWindow *e_vw = ViewWindow::createExportWindow(app,"NewsCoaster",vw->gID,vw->mIDs[vw->cmptr]);
	set(wnd_main,MUIA_Window_Sleep,TRUE);
	if(!e_vw->read()) {
		DoMethod(app, MUIM_Application_PushMethod, app, 4, MUIM_CallHook, &hook_standard, disposeFunc, e_vw );
		//delete e_vw;
	}
	set(wnd_main,MUIA_Window_Sleep,FALSE);
	//return 0;
}

//HOOKCL3( LONG, ViewWindow::expaddFunc, CPPHook *, hook, a0, Object *, object, a2, ViewWindow **, W, a1 )
void ViewWindow::expaddFunc(ViewWindow **W) {
	nLog("ViewWindow::expaddFunc() called\n");
	ViewWindow * vw = *W;
	GroupData * gdata=NULL;
	MessageListData * mdata=NULL;
	get_gdata(&gdata,vw->gID);
	BOOL del = get_mdata(&mdata,gdata,vw->mIDs[vw->cmptr],FALSE);
	exportAddress(gdata,mdata);
	if(del) {
		delete mdata;
		mdata = NULL;
	}
	//return 0;
}

void ViewWindow::resetAppType() {
	nLog("ViewWindow::resetAppType() called\n");
	if(this->ED_view_MESS != NULL) {
		int val = 0;
		get(this->CY_view_APP,MUIA_Cycle_Active,&val);
		if(val==0) {
			set(this->ED_view_MESS,MUIA_TextEditor_ImportHook,MUIV_TextEditor_ImportHook_EMail);
			set(this->ED_view_MESS,MUIA_TextEditor_ExportHook,MUIV_TextEditor_ExportHook_EMail);
		}
		else if(val==1) {
			set(this->ED_view_MESS,MUIA_TextEditor_ImportHook,MUIV_TextEditor_ImportHook_Plain);
			set(this->ED_view_MESS,MUIA_TextEditor_ExportHook,MUIV_TextEditor_ExportHook_Plain);
		}
	}
}

//HOOKCL3( LONG, ViewWindow::appFunc, CPPHook *, hook, a0, Object *, object, a2, ViewWindow **, W, a1 )
void ViewWindow::appFunc(ViewWindow **W) {
	nLog("ViewWindow::appFunc() called\n");
	ViewWindow * vw = *W;
	if(vw->ED_view_MESS != NULL) {
		vw->resetAppType();
		DoMethod(vw->ED_view_MESS,MUIM_TextEditor_ClearText);
		if(!vw->read()) {
			DoMethod(app, MUIM_Application_PushMethod, app, 4, MUIM_CallHook, &hook_standard, disposeFunc, vw );
		}
	}
	//return 0;
}

/*HOOKCL3( LONG, ViewWindow::attachChangeFunc, CPPHook *, hook, a0, Object *, object, a2, ViewWindow **, W, a1 )
	nLog("ViewWindow::attachChangeFunc() called\n");
	ViewWindow * vw = *W;

	//int active=0;
	//get(vw->NLIST_view_ATTACH,MUIA_NList_Active,&active);
	//BOOL s = (active != MUIV_NList_Active_Off);
	// todo:
	return 0;
}*/

/* Called when the 'Find' option is used.
 */
//HOOKCL3( LONG, ViewWindow::findFunc, CPPHook *, hook, a0, Object *, object, a2, ViewWindow **, W, a1 )
void ViewWindow::findFunc(ViewWindow **W) {
	nLog("ViewWindow::findFunc() called\n");
	ViewWindow * vw = *W;

	char text[SEARCHSTRINGLEN+1] = "";
	strncpy(text,getstr(vw->STR_find_TEXT),SEARCHSTRINGLEN);
	text[SEARCHSTRINGLEN] = '\0';

	if(*text == '\0') {
		//return 0;
		return;
	}

	LONG flags = 0;
	LONG marked = FALSE;
	get(vw->ED_view_MESS,MUIA_TextEditor_AreaMarked,&marked);
	if(!marked) {
		flags += MUIF_TextEditor_Search_FromTop;
	}

	LONG casesens = FALSE;
	get(vw->CM_find_CASE,MUIA_Selected,&casesens);
	if(casesens) {
		flags += MUIF_TextEditor_Search_CaseSensitive;
	}
	//printf("marked: %d casesense: %d flags: %d\n",marked,casesens,flags);

	if(!DoMethod(vw->ED_view_MESS,MUIM_TextEditor_Search,text,flags)) {
		if(marked)
			MUI_RequestA(app,0,0,CatalogStr(MSG_FIND,MSG_FIND_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_NO_MORE_OCCURANCES,MSG_NO_MORE_OCCURANCES_STR),0);
		else
			MUI_RequestA(app,0,0,CatalogStr(MSG_FIND,MSG_FIND_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_NO_OCCURANCES_FOUND,MSG_NO_OCCURANCES_FOUND_STR),0);
		DoMethod(vw->ED_view_MESS,MUIM_TextEditor_MarkText,0,0,0,0);
	}
	//return 0;
}

//HOOKCL3( LONG, ViewWindow::menusFunc, CPPHook *, hook, a0, Object *, object, a2, APTR *, param, a1 )
void ViewWindow::menusFunc(void **args) {
	nLog("ViewWindow::menusFunc() called\n");
	ViewWindow * vw = (ViewWindow *)args[0];
	int uD = (int)args[1];
	GroupData *gdata = NULL;
	MessageListData *mdata = NULL;
	Object *object = app;
	switch (uD) {
		case VIEW_PREV:
			//return DoMethod(object, MUIM_CallHook, &prevHook, vw );
			DoMethod(object, MUIM_CallHook, &hook_standard, prevFunc, vw);
			break;
		case VIEW_PREVT:
			//return DoMethod(object, MUIM_CallHook, &prevtHook, vw );
			DoMethod(object, MUIM_CallHook, &hook_standard, prevtFunc, vw);
			break;
		case VIEW_NEXT:
			//return DoMethod(object, MUIM_CallHook, &nextHook, vw );
			DoMethod(object, MUIM_CallHook, &hook_standard, nextFunc, vw);
			break;
		case VIEW_DELNEXT:
			//return DoMethod(object, MUIM_CallHook, &delnextHook, vw );
			DoMethod(object, MUIM_CallHook, &hook_standard, delNextFunc, vw);
			break;
		case VIEW_KILL:
			//return DoMethod(object, MUIM_CallHook, &killHook, vw );
			DoMethod(object, MUIM_CallHook, &hook_standard, killFunc, vw);
			break;
		case VIEW_EXP:
			//return DoMethod(object, MUIM_CallHook, &expHook, vw );
			DoMethod(object, MUIM_CallHook, &hook_standard, expFunc, vw);
			break;
		case VIEW_MARKNORMAL:
		{
			get_gdata(&gdata,vw->gID);
			BOOL del = get_mdata(&mdata,gdata,vw->mIDs[vw->cmptr],TRUE);
			//gdata->flags[1]=TRUE;
			mdata->flags[13]=0;
			//redrawMdataAll();
			void *source = del ? NULL : NLIST_messagelistdata;
			write_index_update(gdata,mdata,source);
			if(del) {
				delete mdata;
				mdata = NULL;
			}
			break;
		}
		case VIEW_MARKIMPORTANT:
		{
			get_gdata(&gdata,vw->gID);
			BOOL del = get_mdata(&mdata,gdata,vw->mIDs[vw->cmptr],TRUE);
			//gdata->flags[1]=TRUE;
			mdata->flags[13]=1;
			//redrawMdataAll();
			void *source = del ? NULL : NLIST_messagelistdata;
			write_index_update(gdata,mdata,source);
			if(del) {
				delete mdata;
				mdata = NULL;
			}
			break;
		}
		case VIEW_EXPADD:
			//return DoMethod(object, MUIM_CallHook, &expaddHook, vw );
			DoMethod(object, MUIM_CallHook, &hook_standard, expaddFunc, vw);
			break;
		case VIEW_REPLY:
			//return DoMethod(object, MUIM_CallHook, &replyHook, vw );
			DoMethod(object, MUIM_CallHook, &hook_standard, replyFunc, vw);
			break;
		case VIEW_O:
			DoMethod(object, MUIM_CallHook, &closeHook, vw );
			break;
		case VIEW_ROT13:
			if(vw->ED_view_MESS != NULL)
				rot13(vw->ED_view_MESS);
			break;
		case VIEW_COPY:
		{
			if(vw->ED_view_MESS != NULL) {
				//set(vw->ED_view_MESS,MUIA_TextEditor_ExportHook,MUIV_TextEditor_ExportHook_Plain);
				DoMethod(vw->ED_view_MESS,MUIM_TextEditor_ARexxCmd,"COPY");
				//vw->resetAppType();
			}
			break;
		}
		case VIEW_FIND:
			set(vw->wnd_find,MUIA_Window_Open,TRUE);
			set(vw->wnd_find,MUIA_Window_ActiveObject,vw->STR_find_TEXT);
			break;
		case VIEW_FINDNEXT:
			//return DoMethod(object, MUIM_CallHook, &findHook, vw );
			DoMethod(object, MUIM_CallHook, &hook_standard, findFunc, vw );
			break;
		case VIEW_HEADERS:
			if(vw->readheader_type==2)
				vw->readheader_type = 1;
			else
				vw->readheader_type = 2;

			if(!vw->read()) {
				DoMethod(app, MUIM_Application_PushMethod, app, 4, MUIM_CallHook, &hook_standard, disposeFunc, vw );
			}
			break;
		case VIEW_ATTACHVIEW:
		case VIEW_UUVIEW:
		{
			MimeSection *ms = NULL;
			DoMethod(vw->NLIST_view_ATTACH,MUIM_NList_GetEntry,MUIV_NList_GetEntry_Active,&ms);
			if(ms != NULL)
			{
				char filename[FILE_NAME_LENGTH] = "";
				char buffer[512] = "";
				char name[L_tmpnam];
				char encodingstore[MIMESECTION_LEN+1] = "";

				sprintf(filename,"%s",tmpnam(name));
				if(uD == VIEW_UUVIEW)
				{
					strncpy(encodingstore,ms->encoding,MIMESECTION_LEN);
						encodingstore[MIMESECTION_LEN] = '\0';
					strcpy(ms->encoding,"uuencoded");
				}
				ms->saveToFile(filename);

				if(*account.defviewer==0)
					sprintf(status_buffer_g,"run >NIL: sys:utilities/multiview \"%s\"",filename);
				else
				{
					sprintf(buffer,"run >NIL: %s",account.defviewer);
					sprintf(status_buffer_g,buffer,filename);
				}
				for(int k=0;k<account.mimeprefsno;k++)
				{
					if(strstr(ms->mimetype,account.mimeprefs[k]->type)!=NULL)
					{
						sprintf(buffer,"run >NIL: %s",account.mimeprefs[k]->viewer);
						sprintf(status_buffer_g,buffer,filename);
					}
				}
				SystemTags(status_buffer_g,
							SYS_Asynch, FALSE,
							NP_Name,    "NewsCoaster Process",
							TAG_END);
				if(uD == VIEW_UUVIEW)
					strcpy(ms->encoding,encodingstore);
			}
			break;
		}
		case VIEW_ATTACHSAVE:
		case VIEW_UUSAVE:
			MimeSection * ms = NULL;
			DoMethod(vw->NLIST_view_ATTACH,MUIM_NList_GetEntry,MUIV_NList_GetEntry_Active,&ms);
			if(ms != NULL) {
				char filename[FILE_NAME_LENGTH] = "";
				char name[MIMESECTION_LEN+1] = "";
				if(strcmp(ms->name,CatalogStr(MSG_ENTIRE_MESSAGE,MSG_ENTIRE_MESSAGE_STR))==0)
					sprintf(name,"message_%d",vw->mIDs[vw->cmptr]);
				else if(strcmp(ms->name,CatalogStr(MSG_HEADERS,MSG_HEADERS_STR))==0)
					sprintf(name,"message_headers_%d",vw->mIDs[vw->cmptr]);
				else if(strcmp(ms->name,CatalogStr(MSG_PLAIN_TEXT_SECTION,MSG_PLAIN_TEXT_SECTION_STR))==0 || strcmp(ms->name,CatalogStr(MSG_HTML_SECTION,MSG_HTML_SECTION_STR))==0)
					sprintf(name,"message_body_%d",vw->mIDs[vw->cmptr]);
				else {
					strncpy(name,ms->name,MIMESECTION_LEN);
					name[MIMESECTION_LEN] = '\0';
				}
				char *nameptr = name;
				char *ptr = NULL;
				ptr = strchr(nameptr,':');
				if(ptr != NULL)
					nameptr = ptr + 1;
				while( (ptr = strchr(nameptr,'/')) != NULL )
					nameptr = ptr + 1;
				while( (ptr = strchr(nameptr,'\\')) != NULL )
					nameptr = ptr + 1;

				if(*nameptr == '\0')
					nameptr = name;

				char encodingstore[MIMESECTION_LEN+1] = "";
				if(uD == VIEW_UUVIEW) {
					strncpy(encodingstore,ms->encoding,MIMESECTION_LEN);
					encodingstore[MIMESECTION_LEN] = '\0';
					strcpy(ms->encoding,"uuencoded");
				}
				if(LoadASL(filename,CatalogStr(MSG_SAVE_NEWS_ITEM_AS,MSG_SAVE_NEWS_ITEM_AS_STR),(const char *)nameptr,(const char *)"#?",FALSE))
				{
            	BPTR file;
               LONG result;
               char *fileNameP = NULL;
               while((file = Open(filename, MODE_OLDFILE)))
               {
                   // The file exists. Do the overwrite/retry
                   Close(file);
                   result = MUI_RequestA(app,0,0,CatalogStr(MSG_DUPLICATING_FILE,MSG_DUPLICATING_FILE_STR),CatalogStr(MSG_RENAME_OVERWRITE_OR_CANCEL,MSG_RENAME_OVERWRITE_OR_CANCEL_STR),CatalogStr(MSG_FILE_ALREADY_EXISTS,MSG_FILE_ALREADY_EXISTS_STR),0);
                   if(result==1)
                   {
                       if(!LoadASL(filename,CatalogStr(MSG_SAVE_NEWS_ITEM_AS,MSG_SAVE_NEWS_ITEM_AS_STR),(const char *)nameptr,(const char *)"#?",FALSE))
                       {
                           *filename = '\0';
                           break;
                       }
                   }
                   else if(result == 0)
                   {
                       *filename = '\0';
                       break;
                   }
                   else if (result == 2)
                   {
                       break;
                   }
                   // by now, provided that we didn't hit cancel, we have the filename we are really going to use.
               }
               if(*filename)
						ms->saveToFile(filename);
				}
				if(uD == VIEW_UUVIEW)
					strcpy(ms->encoding,encodingstore);
			}
			break;
	}
}

HOOKCL1( LONG, ViewWindow::dblFunc, struct ClickMessage *, clickmsg, a1 )
	nLog("ViewWindow::dblFunc() called\n");
	UWORD pos = clickmsg->ClickPosition;
	nLog("  clickmsg->ClickPosition = %d\n",pos);
	nLog("  clickmsg->LineContents = %s\n",clickmsg->LineContents);
	//printf("  clickmsg->ClickPosition = %d\n",pos);
	//printf("  clickmsg->LineContents = %s\n",clickmsg->LineContents);
	while(pos!=0 && *(clickmsg->LineContents+pos-1) != ' ' && *(clickmsg->LineContents+pos-1) != '<')
		pos--;
	//printf("*%s*\n",clickmsg->LineContents + pos);
	if(STRNICMP(clickmsg->LineContents+pos,"HTTP:",5)!=0 && STRNICMP(clickmsg->LineContents+pos,"FILE:",5)!=0)
		return FALSE;
	LookupURL(clickmsg->LineContents+pos);
	return TRUE;
}

HOOKCL2( LONG, ViewWindow::dispattachFunc, char **, array, a2, MimeSection *, data, a1 )
	static char buf1[256];
	if(data) {
		*array++ = data->name;
		*array++ = data->mimetype;
		*array++ = data->encoding;
		sprintf(buf1,"%d",data->length);
		*array = buf1;
	}
	else {
		*array++ = CatalogStr(MSG_NAME,MSG_NAME_STR);
		*array++ = CatalogStr(MSG_MIME_TYPE,MSG_MIME_TYPE_STR);
		*array++ = CatalogStr(MSG_ENCODING,MSG_ENCODING_STR);
		*array = CatalogStr(MSG_LENGTH,MSG_LENGTH_STR);
	}
	return(0);
}

/*HOOKCL3( LONG, ViewWindow::dblAttachFunc, CPPHook *, hook, a0, Object *, object, a2, ViewWindow **, W, a1 )
	return(0);
}*/

//CPPHook ViewWindow::disposeHook ={ {NULL,NULL},(HOOKFUN)&ViewWindow::disposeFunc,NULL,NULL};
CPPHook ViewWindow::closeHook ={ {NULL,NULL},(CPPHOOKFUNC)&ViewWindow::closeFunc,NULL,NULL};
/*CPPHook ViewWindow::killHook ={ {NULL,NULL},(CPPHOOKFUNC)&ViewWindow::killFunc,NULL,NULL};
CPPHook ViewWindow::prevHook ={ {NULL,NULL},(CPPHOOKFUNC)&ViewWindow::prevFunc,NULL,NULL};
CPPHook ViewWindow::prevtHook ={ {NULL,NULL},(CPPHOOKFUNC)&ViewWindow::prevtFunc,NULL,NULL};
CPPHook ViewWindow::delnextHook ={ {NULL,NULL},(CPPHOOKFUNC)&ViewWindow::delnextFunc,NULL,NULL};
CPPHook ViewWindow::nextHook ={ {NULL,NULL},(CPPHOOKFUNC)&ViewWindow::nextFunc,NULL,NULL};
CPPHook ViewWindow::replyHook ={ {NULL,NULL},(CPPHOOKFUNC)&ViewWindow::replyFunc,NULL,NULL};
CPPHook ViewWindow::expHook ={ {NULL,NULL},(CPPHOOKFUNC)&ViewWindow::expFunc,NULL,NULL};
CPPHook ViewWindow::expaddHook ={ {NULL,NULL},(CPPHOOKFUNC)&ViewWindow::expaddFunc,NULL,NULL};
CPPHook ViewWindow::appHook ={ {NULL,NULL},(CPPHOOKFUNC)&ViewWindow::appFunc,NULL,NULL};
CPPHook ViewWindow::findHook ={ {NULL,NULL},(CPPHOOKFUNC)&ViewWindow::findFunc,NULL,NULL};*/
//CPPHook ViewWindow::attachChangeHook ={ {NULL,NULL},(CPPHOOKFUNC)&ViewWindow::attachChangeFunc,NULL,NULL};
//CPPHook ViewWindow::menusHook ={ {NULL,NULL},(CPPHOOKFUNC)&ViewWindow::menusFunc,NULL,NULL};
CPPHook ViewWindow::dblHook={ {NULL,NULL},(CPPHOOKFUNC)&ViewWindow::dblFunc,NULL,NULL};
CPPHook ViewWindow::dispattachHook={ {NULL,NULL},(CPPHOOKFUNC)&ViewWindow::dispattachFunc,NULL,NULL};
int ViewWindow::count=0;
Vector *ViewWindow::ptrs = new Vector(32);

MUI_CustomClass *ViewWindow::editor_mcc_view = NULL;

/* Bare constructor for ViewWindow/
 */
ViewWindow::ViewWindow() {
	nLog("ViewWindow() constructor called\n");
	this->wnd = NULL;
	this->wnd_find = NULL;
	this->ED_view_HD = NULL;
	this->ED_view_MESS = NULL;
	this->editor_mcc_view = NULL;
	//this->app = NULL;
	count++;
	this->readheader_type = account.readheader_type;
	this->noclose = FALSE;
	this->body = NULL;
	mIDs=NULL;
	nIDs=0;
	nLog("  done\n");
}

void ViewWindow::createMcc() {
	nLog("ViewWindow::createMcc() called\n");
	editor_mcc_view = MUI_CreateCustomClass(NULL, "TextEditor.mcc", NULL, sizeof(ViewData), (void *)&ViewWindow::TextEditor_Dispatcher_view);
	nLog("  done\n");
}

/* Creates a new ViewWindow, intended to display a message, along with a
 * list of available attachments to view/save.
 */
//ViewWindow::ViewWindow(Object *app,char *scrtitle) {
ViewWindow::ViewWindow(const char *scrtitle) {
	//nLog("ViewWindow((Object *)%d,(char *)%s) constructor called\n",app,scrtitle);
	nLog("ViewWindow((char *)%s) constructor called\n",scrtitle);
	InitArray(CYA_view_app,MSG_EMAIL_FORMAT);
	//this->app=app;
	this->count++;
	this->readheader_type = account.readheader_type;
	this->noclose=FALSE;
	ptrs->add(this);
	this->body = NULL;
	//this->editor_mcc_view = MUI_CreateCustomClass(NULL, "TextEditor.mcc", NULL, 0, (void *)&this->TextEditor_Dispatcher_view);
	//this->editor_mcc_view = MUI_CreateCustomClass(NULL, "TextEditor.mcc", NULL, sizeof(ViewData), (void *)&ViewWindow::TextEditor_Dispatcher_view);
	if(editor_mcc_view == NULL) {
		//const int vds = sizeof(struct ViewData);
		//editor_mcc_view = MUI_CreateCustomClass(NULL, MUIC_TextEditor, NULL, vds, TextEditor_Dispatcher_view);
		//editor_mcc_view = MUI_CreateCustomClass(NULL, MUIC_TextEditor, NULL, vds, (void *)&ViewWindow::TextEditor_Dispatcher_view);
		//editor_mcc_view = MUI_CreateCustomClass(NULL, "TextEditor.mcc", NULL, vds, (void *)&ViewWindow::TextEditor_Dispatcher_view);
		//editor_mcc_view = MUI_CreateCustomClass(NULL, "TextEditor.mcc", NULL, 0, (void *)&ViewWindow::TextEditor_Dispatcher_view);
		//editor_mcc_view = createMcc();
		//editor_mcc_view = MUI_CreateCustomClass(NULL, "TextEditor.mcc", NULL, sizeof(ViewData), (void *)&ViewWindow::TextEditor_Dispatcher_view);
		createMcc();
	}

	nLog("  menu data\n");
	NewMenu MenuDataView[]= {
		{NM_TITLE,(CONST STRPTR)"Message",						0,0,0,	(APTR)MENVIEW_MESSAGE},
		{NM_ITEM,(CONST STRPTR)"Followup/Reply...",			0,0,0,	(APTR)VIEW_REPLY},
		{NM_ITEM,(CONST STRPTR)"Export...",					0,0,0,	(APTR)VIEW_EXP},
		{NM_ITEM,(CONST STRPTR)"Add to Killfile...",		0,0,0,	(APTR)VIEW_KILL},
		{NM_ITEM,(CONST STRPTR)"Export Address to YAM",	0,0,0,	(APTR)VIEW_EXPADD},
		{NM_ITEM,(CONST STRPTR)"Mark as",						0,0,0,	(APTR)VIEW_MARK},
		{NM_SUB,(CONST STRPTR)"Normal",							0,0,0,	(APTR)VIEW_MARKNORMAL},
		{NM_SUB,(CONST STRPTR)"Important",						0,0,0,	(APTR)VIEW_MARKIMPORTANT},
		{NM_ITEM,NM_BARLABEL,					0,0,0,	(APTR)0},
		{NM_ITEM,(CONST STRPTR)"Toggle All/Standard Headers",(CONST STRPTR)"H",0,0,(APTR)VIEW_HEADERS},
		{NM_ITEM,(CONST STRPTR)"Previous in Thread",		0,0,0,	(APTR)VIEW_PREVT},
		{NM_ITEM,(CONST STRPTR)"Previous",						0,0,0,	(APTR)VIEW_PREV},
		{NM_ITEM,(CONST STRPTR)"Next",							0,0,0,	(APTR)VIEW_NEXT},
		{NM_ITEM,(CONST STRPTR)"Delete then Next",			0,0,0,	(APTR)VIEW_DELNEXT},
		{NM_ITEM,NM_BARLABEL,					0,0,0,	(APTR)0},
		{NM_ITEM,(CONST STRPTR)"Close Window",				0,0,0,	(APTR)VIEW_O},
		{NM_TITLE,(CONST STRPTR)"Edit",							0,0,0,	(APTR)MENVIEW_EDIT},
		{NM_ITEM,(CONST STRPTR)"Find in Message...",		(CONST STRPTR)"F",0,0,	(APTR)VIEW_FIND},
		{NM_ITEM,(CONST STRPTR)"Find Next",					(CONST STRPTR)"G",0,0,	(APTR)VIEW_FINDNEXT},
		{NM_ITEM,(CONST STRPTR)"Copy",							(CONST STRPTR)"C",0,0,	(APTR)VIEW_COPY},
		{NM_ITEM,(CONST STRPTR)"Rot13 Decode",				(CONST STRPTR)"R",0,0,	(APTR)VIEW_ROT13},
		{NM_TITLE,(CONST STRPTR)"Attachments",				0,0,0,	(APTR)MENVIEW_ATTACH},
		{NM_ITEM,(CONST STRPTR)"View Attachment",			0,0,0,	(APTR)VIEW_ATTACHVIEW},
		{NM_ITEM,(CONST STRPTR)"Save Attachment...",		0,0,0,	(APTR)VIEW_ATTACHSAVE},
		//{NM_ITEM,"View UUEncoded Attachment", 0,0,0,	(APTR)VIEW_UUVIEW},
		//{NM_ITEM,"Save UUEncoded Attachment", 0,0,0,	(APTR)VIEW_UUSAVE},

		{NM_END,NULL,0,0,0,(APTR)0},
	};
	InitMenu(MenuDataView,MENU_MESSAGE);

	nLog("  create the scrollbar objects\n");
	SLD_view_HD = ScrollbarObject, End;
	SLD_view_MESS = ScrollbarObject, End;
	//{
		nLog("  create wnd\n");
		wnd = WindowObject,
			MUIA_Window_Title,			CatalogStr(MSG_VIEW,MSG_VIEW_STR),
			MUIA_Window_ID,				MAKE_ID('V','I','E','W') + count,
			MUIA_Window_ScreenTitle,	scrtitle,
#ifdef IWANTBUG
			MUIA_Window_Menustrip,		menustrip=MUI_MakeObject(MUIO_MenustripNM,MenuDataView,0),
#else
			MUIA_Window_Menustrip,		MUI_MakeObject(MUIO_MenustripNM,MenuDataView,0),
#endif
			MUIA_HelpNode,					"WIN_VIEW",

			WindowContents, VGroup,
				Child, GROUP_view_HD=HGroup,
					MUIA_Group_Spacing, 0,
					MUIA_VertWeight, account.weightHD,
					Child, ED_view_HD = (Object*)NewObject(editor_mcc->mcc_Class, NULL,
						MUIA_TextEditor_Slider,			SLD_view_HD,
						MUIA_TextEditor_ReadOnly,		TRUE,
						MUIA_TextEditor_AutoClip,		TRUE,
						MUIA_TextEditor_FixedFont,		TRUE,
						MUIA_CycleChain, 1,
						MUIA_HorizWeight,account.h_weightHD,
						End,
					Child, SLD_view_HD,
					Child, BalanceObject, End,
					Child, GROUP_view_ATTACH=VGroup,
						MUIA_HorizWeight,account.weightATT,
						Child, NListviewObject,
							MUIA_NListview_NList,			NLIST_view_ATTACH=NListObject,
								MUIA_NList_Title,				TRUE,
								MUIA_NList_DragType,			MUIV_NList_DragType_Immediate,
								MUIA_NList_DisplayHook,		&dispattachHook,
								MUIA_NList_Format,			",,,",
								MUIA_NList_MinColSortable,	0,
								MUIA_CycleChain,TRUE,
								MUIA_Font,MUIV_Font_Tiny,
								End,
							MUIA_ShortHelp,CatalogStr(MSG_ATTACHMENTS_LIST_HELP,MSG_ATTACHMENTS_LIST_HELP_STR),
							End,
						Child, HGroup,
							Child,BT_view_ATTVIEW=TextObject,
								MUIA_Text_PreParse,"\33c",
								MUIA_Text_Contents,CatalogStr(MSG_VIEW,MSG_VIEW_STR),
								MUIA_Text_HiChar,'v',
								MUIA_InputMode,MUIV_InputMode_RelVerify,
								MUIA_Background,MUII_ButtonBack,
								MUIA_Frame,MUIV_Frame_Button,
								MUIA_CycleChain,TRUE,
								MUIA_ShortHelp,CatalogStr(MSG_VIEW_ATTACHMENT_HELP,MSG_VIEW_ATTACHMENT_HELP_STR),
								MUIA_Font,MUIV_Font_Tiny,
								MUIA_ControlChar,'v',
								End,
							Child,BT_view_ATTSAVE=TextObject,
								MUIA_Text_PreParse,"\33c",
								MUIA_Text_Contents,CatalogStr(MSG_SAVE_2,MSG_SAVE_2_STR),
								MUIA_Text_HiChar,'s',
								MUIA_InputMode,MUIV_InputMode_RelVerify,
								MUIA_Background,MUII_ButtonBack,
								MUIA_Frame,MUIV_Frame_Button,
								MUIA_CycleChain,TRUE,
								MUIA_ShortHelp,CatalogStr(MSG_SAVE_ATTACHMENT_HELP,MSG_SAVE_ATTACHMENT_HELP_STR),
								MUIA_Font,MUIV_Font_Tiny,
								MUIA_ControlChar,'s',
								End,
							End,
						End,
					End,
				Child, BalanceObject, End,
				Child, ColGroup(3),
					Child, BT_view_REPLY=SimpleButton(CatalogStr(MSG_FOLLOWUP_REPLY_2,MSG_FOLLOWUP_REPLY_2_STR)),
					Child, BT_view_KILL=SimpleButton(CatalogStr(MSG_ADD_TO_KILLFILE,MSG_ADD_TO_KILLFILE_STR)),
					Child, CY_view_APP=Cycle(CYA_view_app),
					End,
				Child, ColGroup(4),
					Child, BT_view_PREV=SimpleButton(CatalogStr(MSG_PREVIOUS_MESSAGE,MSG_PREVIOUS_MESSAGE_STR)),
					Child, BT_view_PREVT=SimpleButton(CatalogStr(MSG_PREVIOUS_IN_THREAD,MSG_PREVIOUS_IN_THREAD_STR)),
					Child, BT_view_DELNEXT=SimpleButton(CatalogStr(MSG_DELETE_NEXT,MSG_DELETE_NEXT_STR)),
					Child, BT_view_NEXT=SimpleButton(CatalogStr(MSG_NEXT_MESSAGE,MSG_NEXT_MESSAGE_STR)),
					End,
				Child,
					GROUP_view_MESS=VGroup,
					MUIA_Group_Spacing, 0,
					MUIA_VertWeight, account.weightMESS,
					Child, HGroup,
						Child, ED_view_MESS = (Object*)NewObject(editor_mcc_view->mcc_Class, NULL,
							MUIA_TextEditor_Slider,				SLD_view_MESS,
							MUIA_TextEditor_ColorMap,			v_cmap,
							MUIA_TextEditor_ReadOnly,			TRUE,
							MUIA_TextEditor_AutoClip,			FALSE,
							MUIA_TextEditor_ImportHook,		MUIV_TextEditor_ImportHook_EMail,
							MUIA_TextEditor_ExportHook,		MUIV_TextEditor_ExportHook_EMail,
							MUIA_TextEditor_DoubleClickHook,	&dblHook,
							MUIA_TextEditor_FixedFont,			TRUE,
							MUIA_CycleChain, 1,
							End,
						Child, SLD_view_MESS,
							End,
						End,
				End,
			End;

	nLog("  create wnd_find\n");
	wnd_find = WindowObject,
		MUIA_Window_Title,			CatalogStr(MSG_FIND_TEXT,MSG_FIND_TEXT_STR),
		MUIA_Window_ID,				MAKE_ID('V','F','N','D') + count,
		MUIA_Window_ScreenTitle,	scrtitle,
		//MUIA_Window_Menustrip,		menustrip=MUI_MakeObject(MUIO_MenustripNM,MenuDataView,0),
		MUIA_HelpNode,					"WIN_VIEW",

		WindowContents, VGroup,
			Child, ColGroup(2),
				Child, Label2(CatalogStr(LABEL_FIND_2,LABEL_FIND_2_STR)), Child, STR_find_TEXT=BetterString("",SEARCHSTRINGLEN),
				End,
			Child, ColGroup(3),
				Child, Label1(CatalogStr(LABEL_CASE_SENSITIVE_2,LABEL_CASE_SENSITIVE_2_STR)), Child, CM_find_CASE = CheckMark(FALSE),
				Child, BT_find_OKAY=SimpleButton(CatalogStr(MSG_FIND_2,MSG_FIND_2_STR)),
				End,
			End,
		End;

		nLog("  set some defaults\n");
		set(CY_view_APP,MUIA_Cycle_Active,account.displayFormat);
		set(CM_find_CASE,MUIA_Selected,FALSE);

		nLog("  add wnd %d to app %d\n",wnd,app);
		DoMethod(app,OM_ADDMEMBER,wnd);
		nLog("  add wnd %d to app %d\n",wnd_find,app);
		DoMethod(app,OM_ADDMEMBER,wnd_find);

		nLog("  set up notifys\n");
		nLog("    close\n");
		DoMethod(wnd,MUIM_Notify,MUIA_Window_CloseRequest,TRUE,
			app,3,MUIM_CallHook,&closeHook,this);
		nLog("    kill\n");
		DoMethod(BT_view_KILL,MUIM_Notify,MUIA_Pressed,FALSE,
			//app,3,MUIM_CallHook,&killHook,this);
			app,4,MUIM_CallHook,&hook_standard,killFunc,this);
		nLog("    prev\n");
		DoMethod(BT_view_PREV,MUIM_Notify,MUIA_Pressed,FALSE,
			//app,3,MUIM_CallHook,&prevHook,this);
			app,4,MUIM_CallHook,&hook_standard,prevFunc,this);
		nLog("    prevt\n");
		DoMethod(BT_view_PREVT,MUIM_Notify,MUIA_Pressed,FALSE,
			//app,3,MUIM_CallHook,&prevtHook,this);
			app,4,MUIM_CallHook,&hook_standard,prevtFunc,this);
		nLog("    delnext\n");
		DoMethod(BT_view_DELNEXT,MUIM_Notify,MUIA_Pressed,FALSE,
			//app,3,MUIM_CallHook,&delnextHook,this);
			app,4,MUIM_CallHook,&hook_standard,delNextFunc,this);
		nLog("    next\n");
		DoMethod(BT_view_NEXT,MUIM_Notify,MUIA_Pressed,FALSE,
			//app,3,MUIM_CallHook,&nextHook,this);
			app,4,MUIM_CallHook,&hook_standard,nextFunc,this);
		nLog("    reply\n");
		DoMethod(BT_view_REPLY,MUIM_Notify,MUIA_Pressed,FALSE,
			//app,3,MUIM_CallHook,&replyHook,this);
			app,4,MUIM_CallHook,&hook_standard,replyFunc,this);
		/*nLog("    exp\n");
		nLog("    %d\n",this);
		nLog("    %d\n",&expHook);
		nLog("    %d\n",BT_view_EXP);
		DoMethod(BT_view_EXP,MUIM_Notify,MUIA_Pressed,FALSE,
			app,3,MUIM_CallHook,&expHook,this);*/
		nLog("    app\n");
		DoMethod(CY_view_APP,MUIM_Notify,MUIA_Cycle_Active,MUIV_EveryTime,
			//app,3,MUIM_CallHook,&appHook,this);
			app,4,MUIM_CallHook,&hook_standard,appFunc,this);
		/*nLog("    attachchange\n");
		DoMethod(NLIST_view_ATTACH,MUIM_Notify,MUIA_NList_Active,MUIV_EveryTime,
			app,3,MUIM_CallHook,&attachChangeHook,this);*/
		nLog("    menu\n");
		DoMethod(wnd,MUIM_Notify,MUIA_Window_MenuAction,MUIV_EveryTime,
			//app,4,MUIM_CallHook,&menusHook,this,MUIV_TriggerValue);
			app,5,MUIM_CallHook,&hook_standard,menusFunc,this,MUIV_TriggerValue);
		nLog("    attachdbl\n");
		DoMethod(NLIST_view_ATTACH,MUIM_Notify,MUIA_NList_DoubleClick,MUIV_EveryTime,
			//app,4,MUIM_CallHook,&menusHook,this,VIEW_ATTACHVIEW);
			app,5,MUIM_CallHook,&hook_standard,menusFunc,this,VIEW_ATTACHVIEW);
		DoMethod(BT_view_ATTVIEW,MUIM_Notify,MUIA_Pressed,FALSE,
			//app,4,MUIM_CallHook,&menusHook,this,VIEW_ATTACHVIEW);
			app,5,MUIM_CallHook,&hook_standard,menusFunc,this,VIEW_ATTACHVIEW);
		DoMethod(BT_view_ATTSAVE,MUIM_Notify,MUIA_Pressed,FALSE,
			//app,4,MUIM_CallHook,&menusHook,this,VIEW_ATTACHSAVE);
			app,5,MUIM_CallHook,&hook_standard,menusFunc,this,VIEW_ATTACHSAVE);
		nLog("    findclose\n");
		DoMethod(wnd_find,MUIM_Notify,MUIA_Window_CloseRequest,TRUE,
			wnd_find,3,MUIM_Set,MUIA_Window_Open,FALSE);
		nLog("    findtext\n");
		DoMethod(STR_find_TEXT,MUIM_Notify,MUIA_String_Acknowledge,MUIV_EveryTime,
			//app,3,MUIM_CallHook,&findHook,this);
			app,4,MUIM_CallHook,&hook_standard,findFunc,this);
		nLog("    findokay\n");
		DoMethod(BT_find_OKAY,MUIM_Notify,MUIA_Pressed,FALSE,
			//app,3,MUIM_CallHook,&findHook,this);
			app,4,MUIM_CallHook,&hook_standard,findFunc,this);

		nLog("  set help bubble text\n");
		set(ED_view_MESS,VIEWWINDOWEDITOR_VIEWWINDOW,this);
		set(BT_view_KILL,MUIA_ShortHelp,CatalogStr(MSG_ADD_TO_KILLFILE_HELP,MSG_ADD_TO_KILLFILE_HELP_STR));
		set(BT_view_PREV,MUIA_ShortHelp,CatalogStr(MSG_PREVIOUS_HELP,MSG_PREVIOUS_HELP_STR));
		set(BT_view_PREVT,MUIA_ShortHelp,CatalogStr(MSG_PREVIOUS_IN_THREAD_HELP,MSG_PREVIOUS_IN_THREAD_HELP_STR));
		set(BT_view_DELNEXT,MUIA_ShortHelp,CatalogStr(MSG_DELETE_NEXT_HELP,MSG_DELETE_NEXT_HELP_STR));
		set(BT_view_NEXT,MUIA_ShortHelp,CatalogStr(MSG_NEXT_HELP,MSG_NEXT_HELP_STR));
		set(BT_view_REPLY,MUIA_ShortHelp,CatalogStr(MSG_FOLLOWUP_REPLY_HELP,MSG_FOLLOWUP_REPLY_HELP_STR));
		//set(BT_view_EXP,MUIA_ShortHelp,CatalogStr(MSG_EXPORT_HELP,MSG_EXPORT_HELP_STR));
		set(CY_view_APP,MUIA_ShortHelp,CatalogStr(MSG_EMAIL_STYLE_HELP,MSG_EMAIL_STYLE_HELP_STR));

		set(STR_find_TEXT,MUIA_ShortHelp,CatalogStr(MSG_TEXT_TO_SEARCH_HELP,MSG_TEXT_TO_SEARCH_HELP_STR));
		set(BT_find_OKAY,MUIA_ShortHelp,CatalogStr(MSG_FIND_NEXT_HELP,MSG_FIND_NEXT_HELP_STR));
		//set(_view_,MUIA_ShortHelp,"");
	//}

	mIDs=NULL;
	nIDs=0;
	resetAppType();
	reset();
	nLog("  done\n");
}

/* Creates a new ViewWindow, intended to display a list of available
 * attachments to view/save.
 */
ViewWindow *ViewWindow::createExportWindow(Object *app,const char *scrtitle,int gID,int mID) {
	nLog("ViewWindow::createExportWindow((Object *)%d,(char *)%s,(int)%d,(int)%d) called\n",app,scrtitle,gID,mID);

	ViewWindow *vw = new ViewWindow();
	Object *BT_VIEW = NULL;
	Object *BT_SAVE = NULL;
	Object *BT_UUVIEW = NULL;
	Object *BT_UUSAVE = NULL;

	//vw->app = app;

		vw->wnd = WindowObject,
			MUIA_Window_Title,			CatalogStr(MSG_EXPORT,MSG_EXPORT_STR),
			MUIA_Window_ID,				MAKE_ID('X','P','R','T') + count,
			MUIA_Window_ScreenTitle,	scrtitle,

			WindowContents, VGroup,
				Child, NListviewObject,
					MUIA_NListview_NList,			vw->NLIST_view_ATTACH=NListObject,
						MUIA_NList_Title,				TRUE,
						MUIA_NList_DragType,			MUIV_NList_DragType_Immediate,
						MUIA_NList_DisplayHook,		&dispattachHook,
						MUIA_NList_Format,			",,,",
						MUIA_NList_MinColSortable,	0,
						End,
					End,

				Child, ColGroup(2),
					Child, BT_VIEW=SimpleButton(CatalogStr(MSG_VIEW_PART,MSG_VIEW_PART_STR)),
					Child, BT_SAVE=SimpleButton(CatalogStr(MSG_SAVE_PART,MSG_SAVE_PART_STR)),
					//Child, BT_UUVIEW=SimpleButton(CatalogStr(MSG_VIEW_UUE,MSG_VIEW_UUE_STR)),
					//Child, BT_UUSAVE=SimpleButton(CatalogStr(MSG_SAVE_UUE,MSG_SAVE_UUE_STR)),
					End,

				End,
			End;

		DoMethod(app,OM_ADDMEMBER,vw->wnd);
		DoMethod(vw->wnd,MUIM_Notify,MUIA_Window_CloseRequest,TRUE,
			app,3,MUIM_CallHook,&closeHook,vw);
		DoMethod(vw->NLIST_view_ATTACH,MUIM_Notify,MUIA_NList_DoubleClick,MUIV_EveryTime,
			//app,4,MUIM_CallHook,&menusHook,vw,VIEW_ATTACHVIEW);
			app,5,MUIM_CallHook,&hook_standard,menusFunc,vw,VIEW_ATTACHVIEW);
		/*DoMethod(vw->NLIST_view_ATTACH,MUIM_Notify,MUIA_NList_Active,MUIV_EveryTime,
			app,3,MUIM_CallHook,&attachChangeHook,vw);*/
		DoMethod(BT_VIEW,MUIM_Notify,MUIA_Pressed,FALSE,
			//app,4,MUIM_CallHook,&menusHook,vw,VIEW_ATTACHVIEW);
			app,5,MUIM_CallHook,&hook_standard,menusFunc,vw,VIEW_ATTACHVIEW);
		DoMethod(BT_SAVE,MUIM_Notify,MUIA_Pressed,FALSE,
			//app,4,MUIM_CallHook,&menusHook,vw,VIEW_ATTACHSAVE);
			app,5,MUIM_CallHook,&hook_standard,menusFunc,vw,VIEW_ATTACHSAVE);
		/*DoMethod(BT_UUVIEW,MUIM_Notify,MUIA_Pressed,FALSE,
			app,4,MUIM_CallHook,&menusHook,vw,VIEW_UUVIEW);
		DoMethod(BT_UUSAVE,MUIM_Notify,MUIA_Pressed,FALSE,
			app,4,MUIM_CallHook,&menusHook,vw,VIEW_UUSAVE);*/

		set(BT_VIEW,MUIA_ShortHelp,CatalogStr(MSG_VIEW_ATTACHMENT_HELP,MSG_VIEW_ATTACHMENT_HELP_STR));
		set(BT_SAVE,MUIA_ShortHelp,CatalogStr(MSG_SAVE_ATTACHMENT_HELP,MSG_SAVE_ATTACHMENT_HELP_STR));

	vw->mIDs=NULL;
	vw->nIDs=0;
	vw->resetSingle(gID,mID);

	return vw;
}

/* Destructor for ViewWindow.
 */
ViewWindow::~ViewWindow() {
	nLog("ViewWindow destructor called\n");
	set(wnd,MUIA_Window_Open,FALSE);
	set(wnd_find,MUIA_Window_Open,FALSE);
	get(GROUP_view_HD,MUIA_VertWeight,&account.weightHD);
	get(GROUP_view_MESS,MUIA_VertWeight,&account.weightMESS);
	get(ED_view_HD,MUIA_HorizWeight,&account.h_weightHD);
	get(GROUP_view_ATTACH,MUIA_HorizWeight,&account.weightATT);
	get(CY_view_APP,MUIA_Cycle_Active,&account.displayFormat);
	if(mIDs!=NULL)
		delete [] mIDs;
	if(body!=NULL)
		delete [] body;
	DoMethod(ED_view_HD,MUIM_TextEditor_ClearText);
	DoMethod(ED_view_MESS,MUIM_TextEditor_ClearText);
	nLog("about to clear attachments\n");
	clearAttachmentList();
	nLog("about to remove wnd from app\n");
	DoMethod(app,OM_REMMEMBER,wnd);
	nLog("about to remove wnd_find from app\n");
	DoMethod(app,OM_REMMEMBER,wnd_find);

	nLog("about to dispose wnd\n");
	MUI_DisposeObject(wnd);
	nLog("about to dispose wnd_find\n");
	MUI_DisposeObject(wnd_find);
	nLog("about to remove ptr\n");
	this->count--;
	ptrs->removeElement(this);
	nLog("done\n");
}

void ViewWindow::reset() {
	nLog("ViewWindow::reset() called\n");
	readInIDs();
	setCurrentID();
}

void ViewWindow::resetSingle(int gID,int mID) {
	nLog("ViewWindow::resetSingle((int)%d,(int)%d) called\n",gID,mID);
	if(mIDs!=NULL)
		delete [] mIDs;
	mIDs = new int[1];
	mIDs[0] = mID;
	nIDs=1;
	this->gID = gID;
	cmptr = 0;
}

void iterator(MessageListData *mdata,int index,void *data) {
	//printf("->%d : %d\n",data,index);
	int *mIDs = (int *)data;
	mIDs[index] = mdata->ID;
}

void ViewWindow::readInIDs() {
	nLog("ViewWindow::readInIDs() called\n");
	if(mIDs!=NULL)
		delete [] mIDs;
	ULONG entries = 0;
	get(NLIST_messagelistdata,MUIA_NList_Entries,&entries);
	nIDs = entries;
	if( entries > 0 )
	{
		mIDs = new int[entries];
		forAllMdata(iterator,mIDs);
	}
}

void ViewWindow::setID(int gID,int mID) {
	nLog("ViewWindow::setID((int)%d,(int)%d) called\n",gID,mID);
	this->gID = gID;
	cmptr = -1;
	for(int k=0;k<nIDs;k++) {
		if( mIDs[k] == mID ) {
			cmptr = k;
			break;
		}
	}
}

void ViewWindow::setCurrentID() {
	nLog("ViewWindow::setCurrentID() called\n");
	GroupData * gdata = NULL;
	MessageListData * mdata = NULL;
	getGdataDisplayed(&gdata);
	getMdataActive(&mdata);
	if( mdata != NULL ) {
		setID(gdata->ID,mdata->ID);
	}
}

BOOL ViewWindow::read(MessageListData *mdata)
{
	nLog("ViewWindow::read((MessageListData *)%d) called\n",mdata);

	GroupData *gdata = NULL;
	get_gdata(&gdata,gID);
	GroupData * cdg = NULL;
	getGdataDisplayed(&cdg);

	BOOL wasonline = FALSE;

	if(mdata->flags[12]==TRUE)
	{
		// we need to download the BODY!
		wasonline = TRUE;
		sleepAll(TRUE);
		BOOL available = TRUE;
		//printf("%d %s : %d %s\n",gdata->ID,gdata->name,mdata->ID,mdata->subject);
		if(!getBody(&available,gdata,mdata))
		{
		//if(TRUE) {
			sleepAll(FALSE);
			return FALSE;
		}
		sleepAll(FALSE);
	}

	char sep[4096] = "";
	char enc[1024] = "";
	char charset[100];
	strcpy(charset,account.charset_write);
	clearAttachmentList();
	if(!insertMessage(mdata,enc,sep,charset))
	{
		ViewWindow::sleepAll(FALSE);
		return FALSE;
	}

	// update flags to READ
	//printf("write? %d %d %d %s\n",gdata->ID,mdata->ID,mdata->flags[1],mdata->subject);
	if(wasonline || mdata->flags[1]==TRUE)
	{
		mdata->flags[1]=FALSE; // no longer NEW
		gdata->num_unread--;
		update_screen_title(gdata);
		redrawGdata(gdata);

		if(gdata->flags[6]!=0)
			gdata->flags[1]=TRUE; // still mark to rewrite index if we are discarding messages
		//nLog("  about to call redrawMdataActive()..\n");
		//redrawMdataActive();
		nLog("  Succeeded!\n");
		nLog("online? %d\n",mdata->flags[12]);
		//printf("writing %d %d %s\n",gdata->ID,mdata->ID,mdata->subject);
		void *source = gdata->ID == cdg->ID ? NLIST_messagelistdata : NULL;
		write_index_update(gdata,mdata,source);
	}

	if(gdata->ID == cdg->ID)
	{
		int pos = getMdataPos(mIDs[cmptr]);
		setMdataActive(pos);
		redrawMdataActive();
	}
	return TRUE;
}

BOOL ViewWindow::read()
{
	nLog("ViewWindow::read() called\n");
	int mID = mIDs[cmptr];
	GroupData * gdata = NULL;
	get_gdata(&gdata,gID);

	MessageListData * mdata = NULL;
	BOOL del = get_mdata(&mdata,gdata,mID,TRUE);
	if(mdata == NULL)
		return FALSE;
	BOOL rtn = read(mdata);
	if(del)
	{
		delete mdata;
		mdata = NULL;
	}
	return rtn;
}

void ViewWindow::checkDeleted(int d_gID,int d_mID) {
	nLog("ViewWindow::checkDeleted((int)%d,(int)%d) called\n",d_gID,d_mID);
	int size=ptrs->getSize();
	if(size==0)
		return;
	ViewWindow ** data = (ViewWindow **)ptrs->getData();
	ViewWindow **del = new ViewWindow *[size];
	int k = 0;
	for(k=0;k<size;k++) {
		ViewWindow * vw = data[k];
		if(d_gID == vw->gID) {
			if(d_mID == vw->mIDs[vw->cmptr] && vw->noclose==FALSE) {
				del[k]=vw;
			}
			else
				del[k]=NULL;
		}
	}
	// deleting ViewWindows will change the Vector ptrs, so we must do deleting afterwards!
	for(k=0;k<size;k++) {
		if(del[k]!=NULL) {
			//delete del[k];
			DoMethod(app, MUIM_Application_PushMethod, app, 4, MUIM_CallHook, &hook_standard, disposeFunc, del[k] );
			del[k] = NULL;
		}
	}
	delete [] del;
}

BOOL ViewWindow::insertMessage(MessageListData *mdata,char *enc,char *sep,char *charset)
{
	nLog("ViewWindow::insertMessage((MessageListData)%d,(char *)%s,(char *)%s) called",mdata->ID,enc,sep);
	char buffer[MAXLINE] = "";
	char isobuffer[MAXLINE] = "";
	char word0[1024] = "";
	char filename[256] = "";

	if (!enc || !sep)
		return FALSE;

	getFilePath(filename,gID,mdata->ID);
	set(wnd,MUIA_Window_Sleep,TRUE);
	sprintf(title,"%s (%s)",mdata->subject,mdata->from);
	set(wnd,MUIA_Window_Title,title);
	if(ED_view_HD != NULL)
	{
		set(ED_view_HD,MUIA_TextEditor_Quiet,TRUE);
		DoMethod(ED_view_HD,MUIM_TextEditor_ClearText);
	}
	if(ED_view_MESS != NULL)
	{
		set(ED_view_MESS,MUIA_TextEditor_Quiet,TRUE);
		DoMethod(ED_view_MESS,MUIM_TextEditor_ClearText);
	}
	if(*mdata->lastref==0)
		set(BT_view_PREVT,MUIA_Disabled,TRUE);
	else
		set(BT_view_PREVT,MUIA_Disabled,FALSE);

	strcpy(enc,"7bit");
	strcpy(sep,"");
	BOOL accept = readheader_type==2;
	BOOL cth = FALSE;

	nLog("  about to open file\n");
	BPTR lock = Lock(filename,ACCESS_READ);
	BPTR file = Open(filename,MODE_OLDFILE);

	if(0==file)
	{
		if(0!=lock)
		{
			lock = NULL;
			UnLock(lock);
		}
		sprintf(buffer,CatalogStr(MSG_UNABLE_TO_OPEN_FILE,MSG_UNABLE_TO_OPEN_FILE_STR),filename);
		MUI_RequestA(app,0,0,CatalogStr(MSG_ERROR_3,MSG_ERROR_3_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),buffer,0);
		set(wnd,MUIA_Window_Sleep,FALSE);
		return FALSE;
	}

	set(wnd,MUIA_Window_Open,TRUE);

	// header
	char contenttype[256] = "";
	char buffer2[1];
	for(;;)
	{
		memset(buffer,0x0,MAXLINE);
		struct FReadLineData *frld = (struct FReadLineData *)AllocDosObjectTags(DOS_FREADLINEDATA,ADO_TermChar, '\n',0);
		if( frld )
		{
			int lineLen = 0;
			int32 rc = FReadLine(file, frld);
			lineLen = frld->frld_LineLength;

			if (lineLen>0)
			{
				char *pch;
				strncpy(buffer,frld->frld_Line,frld->frld_LineLength);
				pch = strstr(buffer,"\r\n");
				if (pch)
					StripChar(pch,'\n');
			}
			else
			{
				FreeDosObject(DOS_FREADLINEDATA,frld);
				break;
			}
			FreeDosObject(DOS_FREADLINEDATA,frld);
		}
		else
			break;

		//printf("buffer: len:%ld, %s\n",strlen(buffer),buffer);

		if ((*buffer=='\n') && (strlen(buffer) == 1))
			break;

		if (*buffer!='\r' && *buffer!='\n')
		{
			int word0len=wordFirstAndLen(word0,buffer);
			if(word0[word0len-1] == ':')
			{
				word0[word0len-1] = '\0';
				if(readheader_type==1)
					accept = (stristr(account.readheaders,word0)!=NULL);
				cth = (stricmp(word0,"Content-Type")==0);
				if(cth)
				{
					char word2[256] = "";
					wordFirst(word2,&buffer[word0len+1]);
					if(*word2 != '\0')
					{
						strcpy(contenttype,word2);
						StripChar(contenttype,';');
					}
				}

				if(stricmp(word0,"Content-Transfer-Encoding")==0)
				{
					char *t = &buffer[word0len + 1];
					char temp[1024] = "";
					wordFirst(temp,t);
					if(*temp != '\0')
					{
						strncpy(enc,temp,MIMESECTION_LEN);
						enc[MIMESECTION_LEN] = '\0';
					}
				}
			}
			if(cth)
			{
				// search for a boundary
				char *bndy = stristr_q(buffer,"BOUNDARY=\"");
				if(bndy!=NULL)
				{
					char *start = &bndy[10];
					char *end = strchr(start,'\"');
					if(end==NULL)
						end = &start[ strlen(start) - 1 ];
					strncpy(sep,start,(int)end - (int)start);
					sep[(int)end - (int)start] = '\0';
				}
				else
				{
					bndy = stristr_q(buffer,"BOUNDARY=");
					if(bndy!=NULL)
					{
						char *start = &bndy[9];
						char *end = start +1;
						while(!isspace(*end) && *end!='\0')
							end++;
						strncpy(sep,start,(int)end - (int)start);
						sep[(int)end - (int)start] = '\0';
					}
				}
				// search for a charset
				char *chrs = stristr_q(buffer,"CHARSET=\"");
				if(chrs!=NULL)
				{
					char *start = &chrs[9];
					char *end = strchr(start,'\"');
					if(end==NULL)
						end = &start[ strlen(start) - 1 ];
					strncpy(charset,start,(int)end - (int)start);
					if (charset[(int)end - (int)start - 1]==';')
						charset[(int)end - (int)start - 1] = '\0';
				}
				else
				{
					chrs = stristr_q(buffer,"CHARSET=");
					if(chrs!=NULL) {
						char *start = &chrs[8];
						char *end = start +1;
						while(!isspace(*end) && *end!='\0')
							end++;
						strncpy(charset,start,(int)end - (int)start);
						if (charset[(int)end - (int)start - 1]==';')
							charset[(int)end - (int)start - 1] = '\0';
					}
				}
				if (chrs)
					nLog ("Charset specification found, charset=%s\n",charset);
			}
			if(accept && ED_view_HD != NULL)
			{
				translateIso(isobuffer,buffer);
				DoMethod(ED_view_HD,MUIM_TextEditor_InsertText,isobuffer,MUIV_TextEditor_InsertText_Bottom);
			}
		}
	}

	// body
	if(body != NULL)
	{
		delete [] body;
		body = NULL;
	}

	int fpos = GetFilePosition(file);
	int size = GetFileSize(file);
	int wholesize = size;

	ChangeFilePosition(file,0,OFFSET_BEGINNING);

	body = new char[size + 1];
	FFlush(file);
	int read = Read(file,body,size);
	body[read] = '\0';

	MimeSection *section = new MimeSection(body,size,CatalogStr(MSG_ENTIRE_MESSAGE,MSG_ENTIRE_MESSAGE_STR),(CONST STRPTR)"text/plain",(CONST STRPTR)"",(CONST STRPTR)"");
	addAttachment(section);
	section = new MimeSection(body,fpos - 1,CatalogStr(MSG_HEADERS,MSG_HEADERS_STR),(CONST STRPTR)"text/plain",(CONST STRPTR)"",(CONST STRPTR)"");
	addAttachment(section);

	//printf("%s\n",contenttype);
	if(*contenttype == '\0')
		strcpy(contenttype,"text/plain");

	//parse(&body[fpos],size - fpos,"",mdata->type,enc,sep);
	parse(&body[fpos],size - fpos,(CONST STRPTR)"",contenttype,enc,sep,charset);

	if(0!=file)
		Close(file);
	if(0!=lock)
		UnLock(lock);

	if(ED_view_HD != NULL)
	{
		set(ED_view_HD,MUIA_TextEditor_CursorY,0);
		set(ED_view_HD,MUIA_TextEditor_Quiet,FALSE);
	}
	if(ED_view_MESS != NULL)
	{
		set(ED_view_MESS,MUIA_TextEditor_CursorY,0);
		set(ED_view_MESS,MUIA_TextEditor_Quiet,FALSE);
	}
	set(NLIST_view_ATTACH,MUIA_NList_Active,MUIV_NList_Active_Top);
	if(ED_view_MESS != NULL)
		set(wnd,MUIA_Window_ActiveObject,ED_view_MESS);
	set(wnd,MUIA_Window_Sleep,FALSE);

	return TRUE;
}

void ViewWindow::parse(char *text,int length,char *name,char *mimetype,char *enc,char *sep,char *charset)
{
	char *translated_string=NULL;
	nLog("parse((char *)%d,(int)%d,(char *)%s,(char *)%s,(char *)%s,(char *)%s,(char *)%s) called\n",text,length,name,mimetype,enc,sep,charset);
	//nLog("%s\n",text);
	//if( *mimetype=='\0' || stricmp(mimetype,"text/plain")==0 || stricmp(mimetype,"text/richtext")==0  || stricmp(mimetype,"text")==0 ) {
	//else if(stricmp(mimetype,"text/html")==0) {
	if(stricmp(mimetype,"text/html")==0)
	{
		MimeSection *section = new MimeSection(text,length,CatalogStr(MSG_HTML_SECTION,MSG_HTML_SECTION_STR),mimetype,enc,sep);
		addAttachment(section);
		if(ED_view_MESS != NULL)
		{
			char *dtext = text;
			BOOL fr = FALSE;
			if(stricmp(enc,"quoted-printable")==0)
			{
				char *ptr = dtext;
				dtext = new char[length*2];
				decode_qprint(ptr,dtext,length,TRUE);
				if(fr)
					delete [] ptr;
				fr = TRUE;
			}
			else if(stricmp(enc,"base64")==0)
			{
				char *ptr = dtext;
				dtext = new char[length + 1024];
				length = decode_base64(ptr,dtext,length);
				if(fr)
					delete [] ptr;
				fr = TRUE;
			}
			else if(is_uuencoded(enc))
			{
				uuEncodeData uudata;
				char *ptr = dtext;
				dtext = new char[length + 1024];
				length = uudecode(ptr,dtext,length,&uudata);
				if(fr)
					delete [] ptr;
				fr = TRUE;
			}
			char *buffer = new char[length*2];
			//stripEscapes(dtext); // HTML input shouldn't contain escape chars
			parse_html(dtext,buffer,length,TRUE);
			translated_string=translateCharset((unsigned char *)buffer,charset);
			if (translated_string)
				buffer = translated_string;

			DoMethod(ED_view_MESS,MUIM_TextEditor_InsertText,buffer,MUIV_TextEditor_InsertText_Bottom);
			DoMethod(ED_view_MESS,MUIM_TextEditor_InsertText,"\n",MUIV_TextEditor_InsertText_Bottom);
			if(fr)
				delete [] dtext;
			delete [] buffer;
		}
	}
	else if( *mimetype=='\0' || STRNICMP(mimetype,"text",4)==0 )
	{

		char *thisname = CatalogStr(MSG_PLAIN_TEXT_SECTION,MSG_PLAIN_TEXT_SECTION_STR);
		if(*name != '\0')
			thisname = name;
		MimeSection *section = new MimeSection(text,length,thisname,mimetype,enc,sep);
		//MimeSection *section = new MimeSection(text,length,CatalogStr(MSG_PLAIN_TEXT_SECTION,MSG_PLAIN_TEXT_SECTION_STR),mimetype,enc,sep);
		addAttachment(section);
		char *dtext = text;
		BOOL fr = FALSE;
		if(stricmp(enc,"quoted-printable")==0)
		{
			char *ptr = dtext;
			dtext = new char[length*2];
			decode_qprint(ptr,dtext,length,TRUE);
			if(fr)
				delete [] ptr;
			fr = TRUE;
		}
		else if(stricmp(enc,"base64")==0)
		{
			char *ptr = dtext;
			dtext = new char[length + 1024];
			length = decode_base64(ptr,dtext,length);
			if(fr)
				delete [] ptr;
			fr = TRUE;
		}
		else if(is_uuencoded(enc))
		{
			uuEncodeData uudata;
			char *ptr = dtext;
			dtext = new char[length + 1024];
			length = uudecode(ptr,dtext,length,&uudata);
			if(fr)
				delete [] ptr;
			fr = TRUE;
		}

		// cope with uuencoded files

/*	Sometimes attachments begin with:

BEGIN -------UUENCODED ATTACHMENT CUT HERE-------
begin 644 bumby.jpg

but not with:

------UUENCODED ATTACHMENT CUT HERE-------

This fix was originally made by Stephen Burgoyne Coulson
<scoulson@axionet.com>.
Included by Pavel Fedin. */

		char *str = dtext;
		for(;;)
		{
			char *uustart = str;
			char *nl;
			if(STRNICMP(str,"begin ",6)==0)
				str=uustart+6;
			else
			{
			// find a candidate uuencode begin
				uustart = stristr_q(str,"\nBEGIN ");
				if(uustart == NULL)
					break;
				str=uustart+7; // Get to end of search "\nBEGIN "
			}
			nLog("may have found a uuencoded file..\n");

			//check for form
			// begin ??? file name to new line
			// where ??? is a 3 digit octal number (unix permissions ala chmod)
			// To give the benefit of the doubt I won't hold that there will be exactly
			// one space between begin and ??? and between ??? and file name
			// look for the first printable char before end of line (or string)
			while(*str && !isprint(*str) && *str != '\n')
			{
				str++;
			}
			if (*str == '\n') // end of line. This line doesn't have a valid begin
			{
				nLog("begin not valid - looking for another\n");
				continue;
			}
			if (*str == '\0') // end of string. We're all done searching
			{
				nLog("found end of text while validating begin.\n");
				break;
			}
			// OK. the next 4 chars should be 0-7,0-7,0-7,0x20(space)
			// One may be tempted to use a loop here. I'm not.
			if(!(str[0] >= '0' && str[0] <= '7' &&
			str[1] >= '0' && str[1] <= '7' &&
			str[2] >= '0' && str[2] <= '7' &&
			str[3] == ' '))
			{
				nLog("no mode after begin -- not a valid begin\n");
				continue;
			}
			// Great, now we can look for the next non space char and
			// copy to end of line as file name
			str += 4;
			while(*str != '\0' && !isprint(*str))
			{
				str++;
			}
			if (!*str) // end of text. die
			{
				break;
			}
			nl = str;

			int index = 0;
			char filename[FILE_NAME_LENGTH] = "";

			while( 0 != *nl && *nl != '\n' && *nl != '\r' && index < 256)
			{
				filename[index++] = *nl++;
			}
			if (!*nl)
			{
				break;
			}
			if (*nl == '\r')
			{
				nl++;
			}
			filename[index] = '\0'; // make sure we end the string properly!
			nLog("Got potential uuencode, file name %s\n",filename);
			BOOL ok = FALSE;
			int n_M = 0;
			int n_lines = 0;
			while( *++nl )  // We are on a '\n', go head 1
			{
				// If all has gone well up to here, we are at the start of a line.
				n_lines++;
				if(*nl == 'M')
					n_M++;
				else if(STRNICMP(nl,"END",3) == 0 && ( nl[3] == '\n' || ( nl[3] == '\r' && nl[4] == '\n' ) ) )
				{
					nLog("found END\n");
					ok = TRUE;
					break;
				}

				nl = strchr(nl,'\n');
			}
			nLog("%d %d\n",n_M,n_lines);
			if(ok && n_M < n_lines-3)
			{
				nLog("not enough M lines\n");
				ok = FALSE;
			}

			if(!ok)
			{
				nLog("  not accepted as a uuencoded file\n");
//				str = &uustart[1];
				continue;
			}

			nLog("  uuencoded file detected\n");
/*			char *w3 = wordStart(uustart,3);
			char *f = filename;
			if(w3 != NULL) {
				while(*w3 != '\r' && *w3 != '\n' && *w3 != '\0')
				*f++ = *w3++;
			}
			*f = '\0';
			if(w3 == NULL || *filename == '\0')*/
			if(*filename == '\0')
				sprintf(filename,"mess_%d_unnamedattach",mIDs[cmptr]);
			nLog("found name: %s\n",filename);
			char *uuend = strstr(uustart,"\n\n");
			if(uuend==NULL)
				uuend = strstr(uustart,"\r\n\r\n");
			int len = 0;
			if(uuend==NULL)
				len = length - (int)uustart + (int)dtext;
			else {
				uuend++;
				len = (int)uuend - (int)uustart;
			}

			char uutype[64] = "";
			char *ext = &filename[-1];
			for(;;) {
				char *next = strchr( &ext[1], '.' );
				if(next != NULL)
					ext = next;
				else
					break;
			}
			getMIMEType(uutype,ext);
			if(*uutype == '\0')
				strcpy(uutype,"text/plain");
			MimeSection *section = new MimeSection(uustart,len,filename,uutype,(CONST STRPTR)"uuencoded",(CONST STRPTR)"");
			addAttachment(section);

			if(uuend==NULL)
				break;
			else
				str = uuend;
		}

		// cope with yencoded files
		str = dtext;
		for(;;)
		{
			char *ystart = NULL;
			if(STRNICMP(str,"=ybegin ",8)==0)
				ystart = str;
			else
			{
				ystart = stristr_q(str,"\n=YBEGIN ");
				if(ystart == NULL)
					break;
				ystart++;
			}

			char filename[FILE_NAME_LENGTH] = "";
			nLog("  yencoded file detected\n");

			char *ptr = stristr_q(ystart,"NAME=");
			if(ptr != NULL)
			{
				ptr += 5;
				char *eptr = NULL;
				if(*ptr == '\"')
				{
					ptr++;
					eptr = strchr(ptr,'\"');
				}
				else
				{
					// According to yEnc1.3 grammar, name field ends with
					// CRLF. Therefore, space searching is a bit ... wrong.
					//eptr = strchr(ptr,' ');
					//char *nl = strchr(ptr,'\n');
					//if( (nl != NULL && nl > eptr) || eptr == NULL )
					//	eptr = nl;
					// Then again, by the time we get here, our CRLFs have been
					// converted to LF so searching for CRLF won't work.
					//char CRLF[]={0x0d,0x0a,0x00};
					//eptr = strstr(ptr,CRLF);

					eptr = strchr(ptr,'\n');

				}
				if(eptr == NULL)
				{
					// changing strcpy to strncpy for safety
					strncpy(filename,ptr,FILE_NAME_LENGTH);
				}
				else {
					strncpy(filename,ptr,(int)(eptr - ptr));
					filename[(int)(eptr - ptr)] = '\0';
				}
				//printf("%s\n",filename);
				StripTrail(filename,'\r');
				//printf("%s\n",filename);
			}
			BOOL noname = FALSE;

			if(*filename == '\0') {
				sprintf(filename,"mess_%d_unnamedattach",mIDs[cmptr]);
				noname = TRUE;
			}
			nLog("found name: %s\n",filename);
			char *yend = strstr(ystart,"=yend");
			yend = strchr(yend,'\n');
			int len = 0;
			if(yend==NULL)
				len = length - (int)ystart + (int)dtext;
			else {
				len = (int)yend - (int)ystart;
			}
			char ytype[64] = "";
			char *ext = &filename[-1];
			for(;;) {
				char *next = strchr( &ext[1], '.' );
				if(next != NULL)
					ext = next;
				else
					break;
			}
			getMIMEType(ytype,ext);
			if(*ytype == '\0')
				strcpy(ytype,"text/plain");

			BOOL found = FALSE;
			if(!noname) {
				// search for existing entry for multifile messages
				ULONG entries = 0;
				get(NLIST_view_ATTACH,MUIA_NList_Entries,&entries);
				for(ULONG i=0;i<entries;i++)
				{
					MimeSection *this_sec = NULL;
					DoMethod(NLIST_view_ATTACH,MUIM_NList_GetEntry,i,&this_sec);
					if(strcmp(this_sec->name,filename)==0) {
						// found
						nLog("  found existing entry at %d\n",i);
						nLog("  %d / %d\n",this_sec->start,this_sec->length);
						if( ystart > &this_sec->start[ this_sec->length ] ) {
							int nlen = (int)ystart + len - (int)this_sec->start;
							if( nlen > this_sec->length )
								this_sec->length = nlen;
						}
						else {
							// just in case
							this_sec->length = (int)this_sec->start + this_sec->length - (int)ystart;
							this_sec->start = ystart;
						}
						nLog("  is now %d / %d\n",this_sec->start,this_sec->length);
						found = TRUE;
						break;
					}
				}
			}
			if(!found) {
				MimeSection *section = new MimeSection(ystart,len,filename,ytype,(CONST STRPTR)"yencoded",(CONST STRPTR)"");
				addAttachment(section);
			}

			if(yend==NULL)
				break;
			else
				str = yend;
		}

		if(ED_view_MESS != NULL) {

			nLog("  entering into texteditor\n");
			translated_string=translateCharset((unsigned char *)dtext,charset);
			if (translated_string)
				dtext = translated_string;
			char *sig = strstr(dtext,"\n-- \n");

			//if(sig==NULL) {
			//	if(strncmp(dtext,"-- \n",4)==0)
			//		sig=dtext;
			//}
			//sig = NULL; // for no .sig separator
			//stripEscapes(dtext,length); // text shouldn't contain escape chars
			if(sig==NULL) {
				DoMethod(ED_view_MESS,MUIM_TextEditor_InsertText,dtext,MUIV_TextEditor_InsertText_Bottom);
			}
			//else if(str>dtext)
			else {
				*sig = '\0';
				DoMethod(ED_view_MESS,MUIM_TextEditor_InsertText,dtext,MUIV_TextEditor_InsertText_Bottom);
				set(ED_view_MESS,MUIA_TextEditor_ImportHook,MUIV_TextEditor_ImportHook_Plain);
				set(ED_view_MESS,MUIA_TextEditor_ExportHook,MUIV_TextEditor_ExportHook_Plain);
				DoMethod(ED_view_MESS,MUIM_TextEditor_InsertText,"\n\33c\33[s:2]\n",MUIV_TextEditor_InsertText_Bottom);
				ULONG val = 0;
				get(CY_view_APP,MUIA_Cycle_Active,&val);
				if(val==0)
				{
					set(ED_view_MESS,MUIA_TextEditor_ImportHook,MUIV_TextEditor_ImportHook_EMail);
					set(ED_view_MESS,MUIA_TextEditor_ExportHook,MUIV_TextEditor_ExportHook_EMail);
				}
				DoMethod(ED_view_MESS,MUIM_TextEditor_InsertText,&sig[5],MUIV_TextEditor_InsertText_Bottom);
				*sig = '\n';
			}
			//else {
			//	set(ED_view_MESS,MUIA_TextEditor_ImportHook,MUIV_TextEditor_ImportHook_Plain);
			//	set(ED_view_MESS,MUIA_TextEditor_ExportHook,MUIV_TextEditor_ExportHook_Plain);
			//	DoMethod(ED_view_MESS,MUIM_TextEditor_InsertText,"\33c\33[s:2]\n",MUIV_TextEditor_InsertText_Bottom);
			//	int val = 0;
			//	get(CY_view_APP,MUIA_Cycle_Active,&val);
			//	if(val==0) {
			//		set(ED_view_MESS,MUIA_TextEditor_ImportHook,MUIV_TextEditor_ImportHook_EMail);
			//		set(ED_view_MESS,MUIA_TextEditor_ExportHook,MUIV_TextEditor_ExportHook_EMail);
			//	}
			//	DoMethod(ED_view_MESS,MUIM_TextEditor_InsertText,&dtext[4],MUIV_TextEditor_InsertText_Bottom);
			//}
			nLog("  done: %d %d\n",dtext,sig);
			//printf("  done: %d %d\n",dtext,sig);

			//DoMethod(ED_view_MESS,MUIM_TextEditor_InsertText,dtext,MUIV_TextEditor_InsertText_Bottom);
			DoMethod(ED_view_MESS,MUIM_TextEditor_InsertText,"\n",MUIV_TextEditor_InsertText_Bottom);
		}
		if(fr)
			delete [] dtext;
	}
	else if(STRNICMP(mimetype,"MULTIPART",9)==0)
	{
		// todo: multipart/digest type
		nLog("  multipart\n");
		Vector *subsections = new Vector();
		char boundary[MIMESECTION_LEN+1] = "";
		//sprintf(boundary,"--%s\n",sep);
		sprintf(boundary,"--%s",sep);
		char endboundary[MIMESECTION_LEN+1] = "";
		sprintf(endboundary,"\n--%s--",sep);
		char *pos = text;
		BOOL first = TRUE;
		for(;;) {
			char *newpos = strstr(pos,boundary);
			if(newpos==NULL)
				newpos = strstr(pos,endboundary);
			//nLog("  (%d) *%s*\n",newpos,pos);
			nLog("  (%d) %d\n",newpos,pos);
			if(newpos==NULL)
				break;
			if(!first) {
				// mini-header
				char submimetype[MIMESECTION_LEN+1] = "text/plain";
				char subenc[MIMESECTION_LEN+1] = "7bit";
				char subsep[MIMESECTION_LEN+1] = "";
				//char subfilename[256] = "unnamed";
				char subfilename[512] = "";
				BOOL cth = FALSE;
				//BOOL cd = FALSE;
				for(;;) {
					while(*pos == '\r')
						pos++;
					char *str = strchr(pos,'\n');
					if(str==NULL || str==pos)
						break;

					//char temp = *str;
					//*str = '\0';
					char word0[1024] = "";
					int len = wordFirstAndLen(word0,pos);
					nLog("%d : *%s*\n",len,word0);
					//printf("%d : *%s*\n",len,word0);
					if(word0[len-1] == ':') {
						cth = FALSE;
						//cd = TRUE;
					}
					if(stricmp(word0,"content-type:")==0) {
						cth = TRUE;
						char *t = &pos[len+1];
						char type[1024] = "";
						wordFirst(type,t);
						StripTrail(type,';');
						if(*type != 0) {
							strncpy(submimetype,type,MIMESECTION_LEN);
							submimetype[MIMESECTION_LEN] = '\0';
						}
					}
					else if(stricmp(word0,"content-transfer-encoding:")==0) {
						char *t = &pos[len+1];
						char type[1024] = "";
						wordFirst(type,t);
						if(*type != 0) {
							strncpy(subenc,type,MIMESECTION_LEN);
							subenc[MIMESECTION_LEN]= '\0';
						}
					}
					//else if(stricmp(word0,"content-transfer-encoding:")==0) {
					//	cd = TRUE;
					//}
					//printf("cth: %d\n",cth);
					//printf("cd : %d\n",cd);
					if(cth) {
						// search for a boundary
						char *bndy = stristr_q(pos,"BOUNDARY=\"");
						if(bndy != NULL) {
							char *start = &bndy[10];
							char *end = strchr(start,'\"');
							if(end==NULL)
								end = &start[strlen(start)-1];
							strncpy(subsep,start,(int)end - (int)start);
							subsep[(int)end - (int)start] = '\0';
						}
						else {
							bndy = stristr_q(pos,"BOUNDARY=");
							if(bndy!=NULL) {
								char *start = &bndy[9];
								char *end = start +1;
								while(!isspace(*end) && *end!='\0')
									end++;
								strncpy(subsep,start,(int)end - (int)start);
								subsep[(int)end - (int)start] = '\0';
							}
						}
						// search for a charset
						char *chrs = stristr_q(pos,"CHARSET=\"");
						if(chrs != NULL) {
							char *start = &chrs[9];
							char *end = strchr(start,'\"');
							if(end==NULL)
								end = &start[strlen(start)-1];
							strncpy(charset,start,(int)end - (int)start);
							charset[(int)end - (int)start] = '\0';
						}
						else {
							chrs = stristr_q(pos,"CHARSET=");
							if(chrs!=NULL) {
								char *start = &chrs[8];
								char *end = start +1;
								while(!isspace(*end) && *end!='\0')
									end++;
								strncpy(charset,start,(int)end - (int)start);
								charset[(int)end - (int)start] = '\0';
							}
						}
						nLog ("Charset specification found, charset=%s\n",charset);
					}
					//else if(cd) {
					//else {
					if( *subfilename == '\0' ) {
						// search for a filename
						char *bndy = stristr_q(pos,"FILENAME=\"");
						if(bndy != NULL) {
							char *start = &bndy[10];
							if(start < str) {
								char *end = strchr(start,'\"');
								if(end==NULL)
									end = &start[strlen(start)-1];
								strncpy(subfilename,start,(int)end - (int)start);
								subfilename[(int)end - (int)start] = '\0';
							}
						}
					}
					//*str = temp;
					pos = str+1;
				}
				pos++;
				int sublength = (int)newpos - (int)pos;
				if( *subfilename == '\0' ) {
					if(stricmp(submimetype,"text/plain")==0)
						strcpy(subfilename,CatalogStr(MSG_PLAIN_TEXT_SECTION,MSG_PLAIN_TEXT_SECTION_STR));
					else
						strcpy(subfilename,"unnamed");
				}
				//pos[sublength] = '\0';
				/*MimeSection *section = new MimeSection(pos,sublength,subfilename,submimetype,subenc,subsep);
				addAttachment(section);*/
				//printf("%s\n",subfilename);
				if(stricmp(mimetype,"multipart/alternative")==0) {
					MimeSection *subsection = new MimeSection(pos,sublength,subfilename,submimetype,subenc,subsep);
					subsections->add(subsection);
				}
				else {
					char temp = pos[sublength];
					pos[sublength] = '\0';
					parse(pos,sublength,subfilename,submimetype,subenc,subsep,charset);
					pos[sublength] = temp;
				}
			}
			else
				first = FALSE;

			pos = newpos + strlen(boundary);
			while(*pos == '\r')
				pos++;
			while(*pos == '\n')
				pos++;
		}
		if(stricmp(mimetype,"multipart/alternative")==0 && subsections->getSize() > 0) {
			int i = 0;
			int best = 0;
			int best_rating = 0;
			MimeSection **sections = (MimeSection **)subsections->getData();
			for(i=0;i<subsections->getSize();i++) {
				MimeSection *section = sections[i];
				nLog("%d : %d , %d\n",i,section,section->mimetype);
				nLog("  *%s*\n",section->mimetype);
				int rating = rate(section->mimetype);
				if(rating > best_rating) {
					best_rating = rating;
					best = i;
				}
			}
			nLog("  chosen %d (rating %d)\n",best,best_rating);
			// need to make sure the other parts are added to the attachment list
			for(i=0;i<subsections->getSize();i++) {
				MimeSection *section = sections[i];
				if(i==best) {
					char temp = pos[section->length];
					pos[section->length] = '\0';
					parse(section->start,section->length,section->name,section->mimetype,section->encoding,section->boundary,charset);
					pos[section->length] = temp;
				}
				else
					addAttachment(section);
			}
			/*MimeSection *section = sections[best];
			parse(section->start,section->length,section->name,section->mimetype,section->encoding,section->boundary);*/
			// free mem
			/*for(i=0;i<subsections->getSize();i++) {
				if(i != best)
					delete sections[i];
			}
			*/
		}
		delete subsections;
	}
	else {
		MimeSection *section = new MimeSection(text,length,name,mimetype,enc,sep);
		addAttachment(section);
	}
}

void ViewWindow::clearAttachmentList() {
	nLog("ViewWindow::clearAttachmentList() called\n");
	ULONG entries = 0;
	get(NLIST_view_ATTACH,MUIA_NList_Entries,&entries);
	for(ULONG i=0;i<entries;i++)
	{
		MimeSection * ms = NULL;
		DoMethod(NLIST_view_ATTACH,MUIM_NList_GetEntry,i,&ms);
		delete ms;
	}
	DoMethod(NLIST_view_ATTACH,MUIM_NList_Clear);
}

void ViewWindow::addAttachment(MimeSection *section)
{
	nLog("ViewWindow::addAttachment((MimeSection *)%d) called\n",section);
	DoMethod(NLIST_view_ATTACH,MUIM_NList_InsertSingle,section,MUIV_NList_Insert_Bottom);
}

void ViewWindow::sleepAll(BOOL sleep) {
	nLog("ViewWindow::sleepAll((BOOL)%d) called\n",sleep);
	int size=ptrs->getSize();
	ViewWindow ** data = (ViewWindow **)ptrs->getData();
	for(int i=0;i<size;i++)
		set(data[i]->wnd,MUIA_Window_Sleep,sleep);
}

/* Free memory allocated by static members.
 */
void ViewWindow::freeStatics() {
	nLog("ViewWindow::freeStatics() called\n");
	int size=ptrs->getSize();
	ViewWindow ** data = (ViewWindow **)ptrs->getData();
	for(int i=0;i<size;i++)
		delete data[i]; // free window - okay to use delete, just don't call this function from a ViewWindow Hook!
	delete ptrs;
	ptrs = NULL;

	if(editor_mcc_view != NULL) {
		nLog("  about to dispose of ViewWindow TextEditor Custom Class\n");
		MUI_DeleteCustomClass(editor_mcc_view);
		nLog("  done\n");
		editor_mcc_view = NULL;
	}
}
