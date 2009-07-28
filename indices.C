/*
    NewsCoaster
    Copyright (C) 1999-2002 Mark Harman

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
#include <mui/NListview_mcc.h>
#include <proto/dos.h>
#include <proto/locale.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "vector.h"
#include "various.h"
#include "main.h"
#include "misc.h"
#include "indices.h"
#include "lists.h"
#include "statuswindow.h"
#include "newscoaster_catalog.h"

#ifdef __amigaos4__
#undef Open
#undef Close
#define Flush(a) 	FFlush(a);
#define Open(a,b) 	FOpen(a,b,65536)
#define Close(a)	FClose(a)
#endif

extern struct codeset *sysCodeset;
extern char *LocalCharset;

const int blocksize = 512; // size of chunk when loading in 'MessageListData's in chunks

//int cached_gID = -999;
//Vector *cached_index = NULL;

/* Writes message 'mdata' to the index of the folder 'gdata'.
 */
void write_index_single(GroupData * gdata,MessageListData * mdata) {
	nLog("write_index_single((GroupData *)%d,(MessageListData *)%d) called\n",gdata,mdata);
	GroupData * cdg = NULL;
	getGdataDisplayed(&cdg);
	if(cdg->ID==gdata->ID) {
		DoMethod(NLIST_messagelistdata,MUIM_NList_InsertSingle,mdata,MUIV_NList_Insert_Bottom);
		//setEnabled();
	}

	char thisversion[5]="";
	char filename[256] = "";
	getIndexPath(filename,gdata->ID);
	//write index
	BPTR lock = Lock(filename,ACCESS_READ);
	BPTR file = Open(filename,MODE_OLDFILE);
	if( (0 != lock) && ( 0 != file) )
	{
		Flush(file);
		Read(file,thisversion,4);
		thisversion[4] = '\0';
		if(strncmp(thisversion,"i",1)!=0) {
			nLog("Error - Not a valid index file!\n");
			// printf("Error - Not a valid index file!\n");
			MUI_RequestA(app,0,0,CatalogStr(MSG_INDEX_ERROR,MSG_INDEX_ERROR_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_INVALID_INDEX,MSG_INVALID_INDEX_STR),0);
		}
		else
		{
			int v = parse_index_version(thisversion);
			if(v < 100)
				MUI_RequestA(app,0,0,CatalogStr(MSG_INDEX_ERROR,MSG_INDEX_ERROR_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_INDEX_OUT_OF_DATE,MSG_INDEX_OUT_OF_DATE_STR),0);
			ChangeFilePosition(file,0,OFFSET_END);
			mdata->flags[7] = GetFilePosition(file);
			Write(file,mdata,sizeof(MessageListData));
		}
	}
	else
	{
		// todo: do something better here!
		// printf("Couldn't open index file! (write_index_single())\n");
		MUI_RequestA(app,0,0,CatalogStr(MSG_INDEX_ERROR,MSG_INDEX_ERROR_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_CANNOT_OPEN_INDEX,MSG_CANNOT_OPEN_INDEX_STR),0);
		nLog("Couldn't open index file! (write_index_single())\n");
	}
	if( 0 != file )
		Close(file);
	if( 0 != lock )
		UnLock(lock);
	nLog("  finished write_index_single() called\n");
}

/* Writes the array of messages 'mdatalist' to the index of the folder
 * 'gdata'.
 */
void write_index_multi(GroupData * gdata,MessageListData ** mdatalist,int count)
{
	nLog("write_index_multi((GroupData *)%d,(MessageListData **)%d,(int)%d) called\n",gdata,mdatalist,count);
	char thisversion[5] = "";
	GroupData * cdg = NULL;
	getGdataDisplayed(&cdg);

	char filename[256] = "";
	getIndexPath(filename,gdata->ID);
	//write index
	BPTR lock = Lock(filename,ACCESS_READ);
	BPTR file = Open(filename,MODE_OLDFILE);
	if(file)
	{
		Flush(file);
		Read(file,thisversion,4);
		thisversion[4] = '\0';
		int v = parse_index_version(thisversion);
		if(v < 100) {
			nLog("Error - Not a valid index file!\n");
			// printf("Error - Not a valid index file!\n");
			MUI_RequestA(app,0,0,CatalogStr(MSG_INDEX_ERROR,MSG_INDEX_ERROR_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_INVALID_INDEX,MSG_INVALID_INDEX_STR),0);
		}
		else
		{
			if(strncmp(thisversion,indexversion,4)!=0)
				MUI_RequestA(app,0,0,CatalogStr(MSG_INDEX_ERROR,MSG_INDEX_ERROR_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_INDEX_OUT_OF_DATE,MSG_INDEX_OUT_OF_DATE_STR),0);
			ChangeFilePosition(file,0,OFFSET_END);
			int pos = GetFilePosition(file);
			for(int k=0;k<count;k++)
			{
				mdatalist[k]->flags[7] = pos + k * sizeof(MessageListData);
				Write(file,mdatalist[k],sizeof(MessageListData));
				if(cdg->ID==gdata->ID) {
					DoMethod(NLIST_messagelistdata,MUIM_NList_InsertSingle,mdatalist[k],MUIV_NList_Insert_Bottom);
				}
			}
		}
		Close(file);
		file=NULL;
	}
	else {
		nLog("Couldn't open index file! (write_index_multi())\n");
		// printf("Couldn't open index file! (write_index_multi())\n");
		MUI_RequestA(app,0,0,CatalogStr(MSG_INDEX_ERROR,MSG_INDEX_ERROR_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_CANNOT_OPEN_INDEX,MSG_CANNOT_OPEN_INDEX_STR),0);
	}
	if(lock) {
		UnLock(lock);
		lock=NULL;
	}
	if(cdg->ID==gdata->ID) {
		setEnabled();
	}
}

/* Updates message 'mdata' in the index of the folder 'gdata' (goes by
 * mdata->flags[7] pos).
 */
void write_index_update(GroupData * gdata,MessageListData * mdata,void *source) {
	nLog("write_index_update((GroupData *)%d,(MessageListData *)%d,(void *)%d) called\n",gdata,mdata,source);
	nLog("gID = %d, mID = %d\n",gdata->ID,mdata->ID);
	char thisversion[5]="";
	if(source != NLIST_messagelistdata) {
		GroupData * cdg = NULL;
		getGdataDisplayed(&cdg);
		if(cdg->ID == gdata->ID) {
			MessageListData *mdata_in_list = NULL;
			get_mdata(&mdata_in_list,mdata->ID);
			if( ( 0!=mdata_in_list) && ( 0 != mdata_in_list ) )
			{
				// this is for cases where the mdata is a copy (eg, in a search list)
				// we want to make sure that the mdata in the index list is up to date!
				*mdata_in_list = *mdata;
			}
		}
	}
	if(source != NLIST_search_RES)
	{
		ULONG entries = 0;
		get(NLIST_search_RES,MUIA_NList_Entries,&entries);
		for(ULONG i=0;i<entries;i++)
		{
			MessageListData *mdata_in_list = NULL;
			DoMethod(NLIST_search_RES,MUIM_NList_GetEntry,i,&mdata_in_list);
			if(mdata->ID == mdata_in_list->ID && gdata->ID == mdata_in_list->flags[6])
			{
				*mdata_in_list = *mdata;
				DoMethod(NLIST_search_RES,MUIM_NList_Redraw,MUIV_NList_Redraw_All);
				break;
			}
		}
	}
	if(source != NLIST_joinmsgs_MSGS)
	{
		ULONG entries = 0;
		// check join window
		get(NLIST_joinmsgs_MSGS,MUIA_NList_Entries,&entries);
		for(ULONG i=0;i<entries;i++) {
			MessageListData *mdata_in_list = NULL;
			DoMethod(NLIST_joinmsgs_MSGS,MUIM_NList_GetEntry,i,&mdata_in_list);
			if(mdata->ID == mdata_in_list->ID && gdata->ID == mdata_in_list->flags[6]) {
				*mdata_in_list = *mdata;
				DoMethod(NLIST_joinmsgs_MSGS,MUIM_NList_Redraw,MUIV_NList_Redraw_All);
				break;
			}
		}
	}

	char filename[256] = "";
	getIndexPath(filename,gdata->ID);
	//write index
	BPTR lock = Lock(filename,ACCESS_READ);
	BPTR file = Open(filename,MODE_OLDFILE);
	if( ( 0 != file ) && ( 0 != lock) )
	{
		Flush(file);
		Read(file,thisversion,4);
		thisversion[4] = '\0';
		if(strncmp(thisversion,"i",1)!=0) {
/*			printf("Error - Not a valid index file!\n");
			printf(": *%s*\n",thisversion);*/
			MUI_RequestA(app,0,0,CatalogStr(MSG_INDEX_ERROR,MSG_INDEX_ERROR_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_INDEX_OUT_OF_DATE,MSG_INDEX_OUT_OF_DATE_STR),0);
			nLog("Error - Not a valid index file!\n");
			nLog(": *%s*\n",thisversion);
		}
		else {
			int v = parse_index_version(thisversion);
			if(v < 100)
				MUI_RequestA(app,0,0,CatalogStr(MSG_INDEX_ERROR,MSG_INDEX_ERROR_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_INDEX_OUT_OF_DATE,MSG_INDEX_OUT_OF_DATE_STR),0);
			nLog("  seeking to %d\n",mdata->flags[7]);
			ChangeFilePosition(file,mdata->flags[7],OFFSET_BEGINNING);
			nLog("  about to write\n");
			Write(file,mdata,sizeof(MessageListData));
			nLog("  done\n");
		}
	}
	else {
		nLog("Couldn't open index file! (write_index_update())\n");
		// printf("Couldn't open index file! (write_index_update())\n");
		MUI_RequestA(app,0,0,CatalogStr(MSG_INDEX_ERROR,MSG_INDEX_ERROR_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_CANNOT_OPEN_INDEX,MSG_CANNOT_OPEN_INDEX_STR),0);
	}
	if( 0 != file )
		Close(file);
	if( 0 != lock )
		UnLock(lock);
	nLog("  finished write_index_update()\n");
}

/* Updates messages in the array 'mdataptr' in the index of the folder
 * 'gdata' (goes by mdata->flags[7] pos).
 */
void write_index_update_multi(GroupData * gdata,MessageListData ** mdatalist,int count,void *source) {
	nLog("write_index_update_multi((GroupData *)%d,(MessageListData **)%d,(int)%d,(void *)%d) called\n",gdata,mdatalist,count,source);
	char thisversion[5]="";
	GroupData * cdg = NULL;
	getGdataDisplayed(&cdg);

	char filename[256] = "";
	getIndexPath(filename,gdata->ID);
	//write index
	BPTR lock = Lock(filename,ACCESS_READ);
	BPTR file = Open(filename,MODE_OLDFILE);
	if( ( 0 != file ) && ( 0 != lock ) ) {
		Flush(file);
		Read(file,thisversion,4);
		thisversion[4] = '\0';
		if(strncmp(thisversion,"i",1)!=0) {
/*			printf("Error - Not a valid index file!\n");
			printf(": *%s*\n",thisversion);*/
			MUI_RequestA(app,0,0,CatalogStr(MSG_INDEX_ERROR,MSG_INDEX_ERROR_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_INVALID_INDEX,MSG_INVALID_INDEX_STR),0);
			nLog("Error - Not a valid index file!\n");
			nLog(": *%s*\n",thisversion);
		}
		else {
			int v = parse_index_version(thisversion);
			if(v < 100)
				MUI_RequestA(app,0,0,CatalogStr(MSG_INDEX_ERROR,MSG_INDEX_ERROR_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_INDEX_OUT_OF_DATE,MSG_INDEX_OUT_OF_DATE_STR),0);
			//BOOL check_copy = TRUE;
			for(int i=0;i<count;i++) {
				MessageListData *mdata = mdatalist[i];
				/*if(check_copy && cdg->ID == gdata->ID) {
					MessageListData *mdata_in_list = NULL;
					get_mdata(&mdata_in_list,mdata->ID);
					if(mdata_in_list != NULL && mdata_in_list != mdata) {
						// this is for cases where the mdata is a copy (eg, in a search list)
						// we want to make sure that the mdata in the index list is up to date!
						*mdata_in_list = *mdata;
					}
					else {
						// hack - assume they're either all copies, or no copies
						// for performance reasons
						check_copy = FALSE;
					}
				}*/
				if(source != NLIST_messagelistdata && cdg->ID == gdata->ID)
				{
					MessageListData *mdata_in_list = NULL;
					get_mdata(&mdata_in_list,mdata->ID);
					if(mdata_in_list != NULL && mdata_in_list != mdata)
					{
						// this is for cases where the mdata is a copy (eg, in a search list)
						// we want to make sure that the mdata in the index list is up to date!
						*mdata_in_list = *mdata;
					}
				}
				if(source != NLIST_search_RES)
				{
					ULONG entries = 0;
					get(NLIST_search_RES,MUIA_NList_Entries,&entries);
					for (ULONG j=0;j<entries;j++)
					{
						MessageListData *mdata_in_list = NULL;
						DoMethod(NLIST_search_RES,MUIM_NList_GetEntry,j,&mdata_in_list);
						if(mdata->ID == mdata_in_list->ID && gdata->ID == mdata_in_list->flags[6])
						{
							*mdata_in_list = *mdata;
							DoMethod(NLIST_search_RES,MUIM_NList_Redraw,MUIV_NList_Redraw_All);
							break;
						}
					}
				}
				if(source != NLIST_joinmsgs_MSGS)
				{
					ULONG entries = 0;
					// check join window
					get(NLIST_joinmsgs_MSGS,MUIA_NList_Entries,&entries);
					for(ULONG j=0;j<entries;j++)
					{
						MessageListData *mdata_in_list = NULL;
						DoMethod(NLIST_joinmsgs_MSGS,MUIM_NList_GetEntry,j,&mdata_in_list);
						if(mdata->ID == mdata_in_list->ID && gdata->ID == mdata_in_list->flags[6])
						{
							*mdata_in_list = *mdata;
							DoMethod(NLIST_joinmsgs_MSGS,MUIM_NList_Redraw,MUIV_NList_Redraw_All);
							break;
						}
					}
				}
				nLog("%d : seeking to %d\n",i,mdata->flags[7]);
				ChangeFilePosition(file,mdata->flags[7],OFFSET_BEGINNING);
				nLog("  about to write\n");
				Write(file,mdata,sizeof(MessageListData));
			}
			nLog("  done\n");
		}
	}
	else
	{
		nLog("Couldn't open index file! (write_index_update())\n");
		// printf("Couldn't open index file! (write_index_update())\n");
		MUI_RequestA(app,0,0,CatalogStr(MSG_INDEX_ERROR,MSG_INDEX_ERROR_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_CANNOT_OPEN_INDEX,MSG_CANNOT_OPEN_INDEX_STR),0);
	}
	if( 0 != file )
		Close(file);
	if( 0 != lock )
		UnLock(lock);
	nLog("  finished write_index_update()\n");
}

/* Deletes message 'mdata' in the index of the folder 'gdata'.
 */
void write_index_delete(GroupData * gdata,MessageListData * mdata) {
	nLog("write_index_delete((GroupData *)%d,(MessageListData *)%d) called\n",gdata,mdata);
	nLog("gID = %d, mID = %d\n",gdata->ID,mdata->ID);
	GroupData * cdg = NULL;
	getGdataDisplayed(&cdg);
	int mID = mdata->ID; // necessary, as this may get deleted (eg, when removing from message list)
	mdata = NULL;
	if(cdg->ID == gdata->ID) {
		MessageListData *mdata_in_list = NULL;
		int pos = get_mdata(&mdata_in_list,mID);
		if(pos != -1)
			DoMethod(NLIST_messagelistdata,MUIM_NList_Remove,pos);

		gdata->flags[1] = TRUE;
		// we can just write out the index, now that the message is removed
		//write_index(0);
		//threadView();
	}
	else {
		Vector *vector = new Vector(2048);
		if( read_index(gdata, vector) ) {
			//printf("%d\n",vector->getSize());
			for(int i=0;i<vector->getSize();i++) {
				MessageListData *mdata2 = ((MessageListData **)vector->getData())[i];
				if(mID == mdata2->ID) {
					//printf("%d\n",i);
					vector->removeElementAt(i);
					break;
				}
			}
			write_index(gdata,vector);
		}
		else {
			nLog("read_index() failed!\n");
		}
		vector->flush();
		delete vector;
	}

	nLog("  finished write_index_delete()\n");
}

/* Deletes messages in the index of the folder 'gdata'.
 */
void write_index_delete_multi(GroupData * gdata,MessageListData ** mdatalist,int count) {
	nLog("write_index_delete_multi((GroupData *)%d,(MessageListData **)%d,(int)%d) called\n",gdata,mdatalist,count);
	nLog("gID = %d\n",gdata->ID);
	GroupData * cdg = NULL;
	getGdataDisplayed(&cdg);
	Vector *vector = NULL;
	// initialisation
	if( cdg->ID != gdata->ID ) {
		vector = new Vector(2048);
		if( !read_index(gdata, vector) )
			nLog("read_index() failed!\n");
	}
	// main loop
	for(int i=0;i<count;i++) {
		int mID = mdatalist[i]->ID; // necessary, as this may get deleted (eg, when removing from message list)
		//mdatalist[i] = NULL;
		if( vector == NULL ) {
			MessageListData *mdata_in_list = NULL;
			int pos = get_mdata(&mdata_in_list,mID);
			if(pos != -1)
				DoMethod(NLIST_messagelistdata,MUIM_NList_Remove,pos);
		}
		else {
			for(int i=0;i<vector->getSize();i++) {
				MessageListData *mdata2 = ((MessageListData **)vector->getData())[i];
				if(mID == mdata2->ID) {
					vector->removeElementAt(i);
					break;
				}
			}
		}
	}
	// cleanup
	if( vector == NULL ) {
		gdata->flags[1] = TRUE;
		// we can just write out the index, now that the message is removed
		//write_index(0);
		//threadView();
	}
	else {
		write_index(gdata,vector);
		vector->flush();
		delete vector;
	}
	nLog("  finished write_index_delete_multi()\n");
}

/* Constructor for index_handler.
 * Opens an index ready for appending new data.
 */
index_handler::index_handler(GroupData * gdata) {
	nLog("index_handler((GroupData *)%d) constructor called\n",gdata);
	file = NULL;
	lock = NULL;
	char thisversion[5] = "";
	char filename[256] = "";
	getIndexPath(filename,gdata->ID);
	lock = Lock(filename,ACCESS_READ);
	file = Open(filename,MODE_OLDFILE);
	if( 0 != file )
	{
		Flush(file);
		Read(file,thisversion,4);
		thisversion[4] = 0;
		int v = parse_index_version(thisversion);
		if(v < 100) {
			nLog("Error - Not a valid index file!\n");
//			printf("Error - Not a valid index file!\n");
			MUI_RequestA(app,0,0,CatalogStr(MSG_INDEX_ERROR,MSG_INDEX_ERROR_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_INVALID_INDEX,MSG_INVALID_INDEX_STR),0);
		}
		else {
			if(strncmp(thisversion,indexversion,4)!=0)
				MUI_RequestA(app,0,0,CatalogStr(MSG_INDEX_ERROR,MSG_INDEX_ERROR_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_INDEX_OUT_OF_DATE,MSG_INDEX_OUT_OF_DATE_STR),0);
			ChangeFilePosition(file,0,OFFSET_END);
		}
	}
	else
//		printf("Couldn't open index file! (index_handler())\n");
		MUI_RequestA(app,0,0,CatalogStr(MSG_INDEX_ERROR,MSG_INDEX_ERROR_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_CANNOT_OPEN_INDEX,MSG_CANNOT_OPEN_INDEX_STR),0);
		nLog("Couldn't open index file! (index_handler())\n");
}

/* Writes the message 'mdata' to the currently opened index file.
 */
void index_handler::write(MessageListData * mdata) {
	//nLog("index_handler::write((MessageListData *)%d) called\n",mdata);
	mdata->flags[7] = GetFilePosition(file);

	Write(file,mdata,sizeof(MessageListData));
}

/* index_handler destructor.
 * Close the file.
 */
index_handler::~index_handler() {
	nLog("index_handler() destructor called\n");
	if(file) {
		Close(file);
		file=NULL;
	}
	if(lock) {
		UnLock(lock);
		lock=NULL;
	}
}

void writeEmptyIndex(int gID) {
	nLog("writeEmptyIndex((int)%d) called\n",gID);
	char filename[256] = "";
	getIndexPath(filename,gID);

	BPTR file = Open(filename,MODE_NEWFILE);
	if(file) {
		Flush(file);
		Write(file,indexversion,strlen(indexversion));

		Close(file);
		file=NULL;
	}
}

/* Writes the messages in the message list out.
 * If 'type'==0, the currently displayed folder is written out.
 * If 'type'==1, the folder indicated by old_gdata is written out.
 */
void write_index(int type)
{
	nLog("write_index((int)%d) called\n",type);
	char filename[256] = "";
	char filenamenew[256] = "";
	GroupData * gdata=NULL;
	ULONG entries = 0;
	struct DateStamp ds;
	BPTR file = NULL;

	if(type==0)
		getGdataDisplayed(&gdata);
	else
		gdata = old_gdata;
	if(gdata == NULL)
		return;
	gdata->flags[1]=FALSE;
	getIndexPath(filename,gdata->ID);
	sprintf(filenamenew,"%s.new",filename);
	get(NLIST_messagelistdata,MUIA_NList_Entries,&entries);
	set(wnd_main,MUIA_Window_Sleep,TRUE);
	//save index
	DeleteFile(filenamenew);
	file = Open(filenamenew,MODE_NEWFILE);
	DateStamp(&ds);
	if(file)
	{
		Flush(file);
		Write(file,indexversion,strlen(indexversion));
		gdata->nummess = 0;
		if(entries>0)
		{
			int bsize = blocksize;
			MessageListData *mdataptr = new MessageListData[bsize];
			if(mdataptr==NULL)
			{
				bsize = 64;
				mdataptr = new MessageListData[bsize];
			}
			MessageListData **mdataptr_d = new MessageListData *[bsize];
			if(mdataptr_d==NULL)
			{
				bsize = 64;
				mdataptr_d = new MessageListData *[bsize];
			}
			ULONG i = 0;
			do
			{
				int written = 0;
				int deleted = 0;
				int fp = GetFilePosition(file);

				for(int j=0;j<bsize;j++)
				{
					MessageListData * mdata = NULL;
					DoMethod(NLIST_messagelistdata,MUIM_NList_GetEntry,i,&mdata);
					BOOL discard = FALSE;
					if(gdata->flags[6]==1) { // discard on age
						if(gdata->flags[7] > 0) {
							if( mdata->flags[13]!=1 ) { // don't discard important messages!
								if( ds.ds_Days - mdata->ds.ds_Days > gdata->flags[7] )
									discard=TRUE;
							}
						}
					}
					if(discard)
						mdataptr_d[deleted++] = mdata;
					else {
						mdata->flags[7] = fp;
						fp += sizeof(MessageListData);
						mdataptr[written++] = *mdata;
					}
					i++;
					if(i == entries)
						break;
				}
				if(written>0) {
					Write(file,mdataptr,written*sizeof(MessageListData));
					gdata->nummess += written;
				}
				//printf("->deleted: %d\n",deleted);
				if(deleted>0) {
					delete_file_n(gdata,mdataptr_d,deleted);
				}
			} while(i<entries);
			delete [] mdataptr;
			delete [] mdataptr_d;
		}
		redrawGdataAll();
	}
	else {
		nLog("Couldn't create new index file! (write_index())\n");
//		printf("Couldn't create new index file! (write_index())\n");
		MUI_RequestA(app,0,0,CatalogStr(MSG_INDEX_ERROR,MSG_INDEX_ERROR_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_CANNOT_CREATE_INDEX,MSG_CANNOT_CREATE_INDEX_STR),0);
	}
	if(file) {
		Close(file);
		file=NULL;
	}

	DeleteFile(filename);
	Rename(filenamenew,filename);
	set(wnd_main,MUIA_Window_Sleep,FALSE);
}

/* Writes the messages in the message vector out.
 */
void write_index(GroupData * gdata,Vector * vector) {
	nLog("write_index((GroupData *)%d,(Vector *)%d) called\n",gdata,vector);
	nLog("gID = %d\n",gdata->ID);
	ULONG entries = 0;
	char filename[256] = "";
	char filenamenew[256] = "";

	gdata->flags[1]=FALSE;
	getIndexPath(filename,gdata->ID);
	sprintf(filenamenew,"%s.new",filename);
	get(NLIST_messagelistdata,MUIA_NList_Entries,&entries);

	//save index
	DeleteFile(filenamenew);
	BPTR file = Open(filenamenew,MODE_NEWFILE);
	struct DateStamp ds;
	DateStamp(&ds);
	if(file) {
		Flush(file);
		Write(file,indexversion,strlen(indexversion));
		gdata->nummess = vector->getSize();
		for(int i=0;i<gdata->nummess;i++) {
			MessageListData *mdata2 = ((MessageListData **)vector->getData())[i];
			Write(file,mdata2,sizeof(MessageListData));
		}
		redrawGdataAll();
	}
	else {
		nLog("Couldn't create new index file! (write_index())\n");
//		printf("Couldn't create new index file! (write_index())\n");
		MUI_RequestA(app,0,0,CatalogStr(MSG_INDEX_ERROR,MSG_INDEX_ERROR_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_CANNOT_CREATE_INDEX,MSG_CANNOT_CREATE_INDEX_STR),0);
	}
	if(file) {
		Close(file);
		file=NULL;
	}

	DeleteFile(filename);
	Rename(filenamenew,filename);
}

/* Reads the index of the current folder.
 */
void read_index()
{
	nLog("read_index() called\n");
	GroupData * gdata = NULL;
	char filename[256] = "";
	char thisversion[5]="";

	if(v_gdata==NULL)
		getGdataActive(&gdata);
	else
		gdata = v_gdata;
	c_gdata = gdata;
	//printf("c_gdata = %d\n",c_gdata->ID);
	int nummess=0;
	int num_unread = 0;
	if(old_gdata)
	{
		if(old_gdata->flags[1])
			write_index(1);
	}
	getIndexPath(filename,gdata->ID);
	setMdataQuiet(TRUE);
	set(wnd_main,MUIA_Window_Sleep,TRUE);
	DoMethod(NLIST_messagelistdata,MUIM_NList_Clear);
	nLog("  read_index() : about to lock file: %s\n",filename);
	StatusWindow *sW = NULL;
	//load index
	BPTR lock = Lock(filename,ACCESS_READ);
	BPTR file = Open(filename,MODE_OLDFILE);
	if( 0 != file )
	{
		Flush(file);
		Read(file,thisversion,4);
		thisversion[4] = 0;
		if(strncmp(thisversion,"i",1)!=0) {
			nLog("Error - Not a valid index file!\n");
//			printf("Error - Not a valid index file!\n");
			MUI_RequestA(app,0,0,CatalogStr(MSG_INDEX_ERROR,MSG_INDEX_ERROR_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_INVALID_INDEX,MSG_INVALID_INDEX_STR),0);
		}
		else
		{
			int v = parse_index_version(thisversion);
			if(v < 100)
				MUI_RequestA(app,0,0,CatalogStr(MSG_INDEX_ERROR,MSG_INDEX_ERROR_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_INDEX_OUT_OF_DATE,MSG_INDEX_OUT_OF_DATE_STR),0);
			else if(strncmp(thisversion,indexversion,4)!=0)
			{
				sW = new StatusWindow(app,CatalogStr(MSG_READING_INDEX,MSG_READING_INDEX_STR));
				sW->setText(CatalogStr(MSG_INDEX_FORMAT_CHANGED,MSG_INDEX_FORMAT_CHANGED_STR));
			}
			const int mds = sizeof(MessageListData);
			//int t1=clock();

			// new version inserts in blocks - much faster :)
			MessageListData ** mdataptr = new MessageListData *[blocksize];
			BOOL eof = FALSE;
			nLog("  about to start reading\n");
			do
			{
				int numblock = 0;
				MessageListData ** ptr = mdataptr;
				do
				{
					//*ptr = new MessageListData;
					MessageListData *mdata = new MessageListData();
					if(mdata == NULL)
						nLog("Fatal - mdata==NULL!\n");
					*ptr = mdata;
					int fp = GetFilePosition(file);

					if(Read(file,mdata,mds)<=0)
					{
						nLog("  reached EOF\n");
						eof=TRUE;
						nLog("  about to delete mdata %d\n",*ptr);
						delete mdata;
						nLog("  deleted mdata\n");
						*ptr=NULL;
						nLog("  about to exit\n");
						break;
					}
					/*(*ptr)->flags[7] = fp;
					if((*ptr)->flags[1])
						num_unread++;*/
					mdata->flags[7] = fp;
					if(mdata->flags[1])
						num_unread++;
					ptr++;
				}
				while(numblock++ < blocksize);

				if(numblock>0)
				{
					nLog("about to insert %d entries\n",numblock);
					DoMethod(NLIST_messagelistdata,MUIM_NList_Insert,mdataptr,numblock,MUIV_NList_Insert_Bottom);
					nummess += numblock;
					nLog("  done - nummess now %d\n",nummess);
				}
			}
			while(eof == FALSE);
			/*int t2=clock();
			int t3=clock();
			printf("Time to read : %d\n",t2-t1);
			printf("Time to sort : %d\n",t3-t2);
			printf("Total Time : %d\n",t3-t1);*/
			nLog("  about to sort\n");
			DoMethod(NLIST_messagelistdata,MUIM_NList_Sort);
			nLog("  read_index() : about to free mdataptr\n");
			if(mdataptr)
			{
				delete [] mdataptr;
				mdataptr=NULL;
			}
			if(account.mdata_view==1)
				threadView();
		}
	}
	nLog("  read_index() : finished reading\n");
	gdata->nummess=nummess;
	gdata->num_unread=num_unread;
	redrawGdataAll();
	update_screen_title(gdata);
	setMdataQuiet(FALSE);
	set(wnd_main,MUIA_Window_Sleep,FALSE);
	nLog("..\n");
	if(nummess>0)
		setMdataActive(0);
	DoMethod(NLIST_messagelistdata,MUIM_NList_ColWidth,MUIV_NList_ColWidth_All,MUIV_NList_ColWidth_Default);
	if(file)
	{
		Close(file);
		file=NULL;
	}
	if(lock) {
		UnLock(lock);
		lock=NULL;
	}
	nLog("..\n");
	setEnabled();
	nLog("..\n");
	if(sW!=NULL)
		delete sW;
	// need to do write_index for group other than current!
	if(strncmp(thisversion,indexversion,4)!=0) {
		StatusWindow *statusWindow = new StatusWindow(app,CatalogStr(MSG_WRITING_INDEX,MSG_WRITING_INDEX_STR));
		statusWindow->setText(CatalogStr(MSG_NEW_INDEX_FORMAT,MSG_NEW_INDEX_FORMAT_STR));
		write_index(0); // write version in update format!
		delete statusWindow;
	}
	setGdataHelpText(gdata);
	nLog("  finished read_index()\n");
}

/* Reads the index of 'gdata' into a Vector.
 */
BOOL read_index(GroupData * gdata,Vector * vector) {
	nLog("read_index((GroupData *)%d,(Vector *)%d) called\n",gdata,vector);
	BOOL res = TRUE;
	char filename[256] = "";
	getIndexPath(filename,gdata->ID);
	char thisversion[5] = "";
	nLog("  read_index() : about to lock file: %s\n",filename);
	StatusWindow *sW = NULL;
	//load index
	BPTR lock = Lock(filename,ACCESS_READ);
	BPTR file = Open(filename,MODE_OLDFILE);
	if( 0 != file )
	{
		Flush(file);
		Read(file,thisversion,4);
		thisversion[4] = 0;
		if(strncmp(thisversion,"i",1)!=0) {
			nLog("Error - Not a valid index file!\n");
//			printf("Error - Not a valid index file!\n");
			MUI_RequestA(app,0,0,CatalogStr(MSG_INDEX_ERROR,MSG_INDEX_ERROR_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_INVALID_INDEX,MSG_INVALID_INDEX_STR),0);
		}
		else {
			int v = parse_index_version(thisversion);
			if(v < 100)
				MUI_RequestA(app,0,0,CatalogStr(MSG_INDEX_ERROR,MSG_INDEX_ERROR_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_INDEX_OUT_OF_DATE,MSG_INDEX_OUT_OF_DATE_STR),0);
			else if(strncmp(thisversion,indexversion,4)!=0) {
				sW = new StatusWindow(app,CatalogStr(MSG_READING_INDEX,MSG_READING_INDEX_STR));
				sW->setText(CatalogStr(MSG_INDEX_FORMAT_CHANGED,MSG_INDEX_FORMAT_CHANGED_STR));
			}
			const int mds = sizeof(MessageListData);
			nLog("  about to start reading\n");
			int fp = GetFilePosition(file);

			for(;;)
			{
				MessageListData *mdata = new MessageListData();
				if(mdata==NULL)
				{
					nLog("Fatal - mdata==NULL!\n");
					res = FALSE;
					break;
				}
				if(Read(file,mdata,mds)<=0)
				{
					nLog("  reached EOF\n");
					delete mdata;
					mdata=NULL;
					break;
				}
        		ULONG strLen = 0;

        		char *str = translateCharset((unsigned char*)mdata->subject,NULL);
        		if (str)
        			strcpy(mdata->subject,str);

				mdata->flags[7] = fp;
				vector->add(mdata);
				fp += mds;
			}
		}
	}
	nLog("  read_index() : finished reading\n");
	gdata->nummess=vector->getSize();
	if(file) {
		Close(file);
		file=NULL;
	}
	if(lock) {
		UnLock(lock);
		lock=NULL;
	}
	nLog("..\n");
	if(sW != NULL)
		delete sW;
	nLog("..\n");
	if(strncmp(thisversion,indexversion,4)!=0) {
		StatusWindow *statusWindow = new StatusWindow(app,CatalogStr(MSG_WRITING_INDEX,MSG_WRITING_INDEX_STR));
		statusWindow->setText(CatalogStr(MSG_NEW_INDEX_FORMAT,MSG_NEW_INDEX_FORMAT_STR));
		write_index(0); // write version in update format!
		delete statusWindow;
	}
	nLog("  finished read_index()\n");
	return res;
}

/* Reads the index of 'gdata' to find a single message.
 */
BOOL get_mdata_from_index(MessageListData **mdataptr,GroupData *gdata,int mID) {
	nLog("get_mdata_from_index((MessageListData **)%d,(GroupData *)%d,(int)%d) called\n",mdataptr,gdata,mID);
	BOOL res = FALSE;
	char filename[256] = "";
	getIndexPath(filename,gdata->ID);
	char thisversion[5] = "";
	nLog("  get_mdata_from_index() : about to lock file: %s\n",filename);
	*mdataptr = NULL;
	//load index
	BPTR lock = Lock(filename,ACCESS_READ);
	BPTR file = Open(filename,MODE_OLDFILE);
	if( 0 != file )
	{
		Flush(file);
		Read(file,thisversion,4);
		thisversion[4] = 0;
		if(strncmp(thisversion,"i",1)!=0) {
			nLog("Error - Not a valid index file!\n");
//			printf("Error - Not a valid index file!\n");
			MUI_RequestA(app,0,0,CatalogStr(MSG_INDEX_ERROR,MSG_INDEX_ERROR_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_INVALID_INDEX,MSG_INVALID_INDEX_STR),0);
		}
		else {
			int v = parse_index_version(thisversion);
			if(v < 100)
				MUI_RequestA(app,0,0,CatalogStr(MSG_INDEX_ERROR,MSG_INDEX_ERROR_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_INDEX_OUT_OF_DATE,MSG_INDEX_OUT_OF_DATE_STR),0);
			const int mds = sizeof(MessageListData);
			nLog("  about to start reading\n");
			int fp = GetFilePosition(file);

			MessageListData mdata;
			for(;;) {
				if(Read(file,&mdata,mds)<=0)
				{
					nLog("  reached EOF\n");
					break;
				}
				if(mdata.ID == mID) {
					mdata.flags[7] = fp;
					*mdataptr = new MessageListData();
					*(*mdataptr) = mdata;
					res = TRUE;
					break;
				}
				fp += mds;
			}
		}
	}
	nLog("  get_mdata_from_index() : finished reading\n");
	if(file) {
		Close(file);
		file=NULL;
	}
	if(lock) {
		UnLock(lock);
		lock=NULL;
	}
	nLog("  finished read_index()\n");
	return res;
}
