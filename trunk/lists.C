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
#include <mui/NListtree_mcc.h>

#include <stdio.h>
#include <string.h>

#include "vector.h"
#include "various.h"
#include "main.h"
#include "lists.h"
#include "misc.h"
#include "strings.h"
#include "indices.h"

int glist_quiet_level = 0;
int mlist_quiet_level = 0;

/* The following set of functions perform actions on, or read data from the
 * lists for the Folders (GroupData), and Messages (MessageListData). These
 * may each either be a flat list (NList) or a tree (NListtree).
 */

/* Gets the number of entries in the Folders list. If this is a tree list,
 * 'dummy' entries - ie, nodes which are parents for other nodes/leaves, but
 * don't represent an actual folder - are included in the count.
 */
void getGdataEntries(int * entries) {
	if(account.grouplistType==0)
		get(NLIST_groupdata,MUIA_NList_Entries,entries);
	else {
		int n = DoMethod(LISTTREE_groupdata,MUIM_NListtree_GetNr,tn_folders,MUIV_NListtree_GetNr_Flag_CountAll);
		*entries = n;
	}
}

/* As getGdataEntries(), but in the case of a tree list, entries which are
 * parents to other nodes/leaves, which don't represent a folder, aren't
 * counted
 */
void getGdataTrueEntries(int * entries) {
	MUI_NListtree_TreeNode * tn;
	MUI_NListtree_TreeNode * temp_tn;
	if(account.grouplistType==0)
		get(NLIST_groupdata,MUIA_NList_Entries,entries);
	else {
		int n=0;
		tn=(MUI_NListtree_TreeNode *)DoMethod(LISTTREE_groupdata,MUIM_NListtree_GetEntry,MUIV_NListtree_GetEntry_ListNode_Root,MUIV_NListtree_GetEntry_Position_Head,0);
		while(tn!=NULL) {
			temp_tn=(MUI_NListtree_TreeNode *)DoMethod(LISTTREE_groupdata,MUIM_NListtree_GetEntry,tn,MUIV_NListtree_GetEntry_Position_Head,0);
			if(temp_tn!=NULL) // we have child
				tn = temp_tn;
			else { // no child
				n++;
				do {
					temp_tn=(MUI_NListtree_TreeNode *)DoMethod(LISTTREE_groupdata,MUIM_NListtree_GetEntry,tn,MUIV_NListtree_GetEntry_Position_Next,0);
					if(temp_tn!=NULL) // next in list
						break;
					// end of this list, go to parent then next
					tn=(MUI_NListtree_TreeNode *)DoMethod(LISTTREE_groupdata,MUIM_NListtree_GetEntry,tn,MUIV_NListtree_GetEntry_Position_Parent,0);
				} while(tn!=NULL);
				if(tn!=NULL)
					tn = temp_tn;
			}
		}
		*entries = n;

	}
}

/* Returns the GroupData pointer at position 'pos' in the list. For the
   tree view, the 'parent' nodes are included in the count - a value of
   NULL is returned for these.
 */
void getGdata(int pos,GroupData ** gdataptr) {
	MUI_NListtree_TreeNode * tn;
	MUI_NListtree_TreeNode * temp_tn;
	if(account.grouplistType==0)
		DoMethod(NLIST_groupdata,MUIM_NList_GetEntry,pos,gdataptr);
	else {
		tn=(MUI_NListtree_TreeNode *)DoMethod(LISTTREE_groupdata,MUIM_NListtree_GetEntry,MUIV_NListtree_GetEntry_ListNode_Root,MUIV_NListtree_GetEntry_Position_Head,0);
		for(int i=0; i<pos && tn!=NULL; i++) {
			temp_tn=(MUI_NListtree_TreeNode *)DoMethod(LISTTREE_groupdata,MUIM_NListtree_GetEntry,tn,MUIV_NListtree_GetEntry_Position_Head,0);
			if(temp_tn!=NULL) // we have child
				tn = temp_tn;
			else { // no child
				do {
					temp_tn=(MUI_NListtree_TreeNode *)DoMethod(LISTTREE_groupdata,MUIM_NListtree_GetEntry,tn,MUIV_NListtree_GetEntry_Position_Next,0);
					if(temp_tn!=NULL) // next in list
						break;
					// end of this list, go to parent then next
					tn=(MUI_NListtree_TreeNode *)DoMethod(LISTTREE_groupdata,MUIM_NListtree_GetEntry,tn,MUIV_NListtree_GetEntry_Position_Parent,0);
				} while(tn!=NULL);
				if(tn!=NULL)
					tn = temp_tn;
			}
		}

		if(tn==NULL) {
			nLog("Error!!! pos %d gives NULL in getGdata !\n",pos);
			*gdataptr=NULL;
		}
		else {
			*gdataptr=(GroupData *)tn->tn_User;
		}
	}
}

/* Returns the GroupData of the currently displayed group.
 */
void getGdataDisplayed(GroupData ** gdataptr) {
	*gdataptr = c_gdata;
}

/* Returns the GroupData that is currently active - this will be NULL if no
   group is active.
 */
void getGdataActive(GroupData ** gdataptr)
{
	MUI_NListtree_TreeNode *tn;
	if(account.grouplistType==0)
		getGdata(MUIV_NList_GetEntry_Active,gdataptr);
	else
	{
		get(LISTTREE_groupdata,MUIA_NListtree_Active,(ULONG*)&tn);
		if(tn==MUIV_NListtree_Active_Off)
		{
			*gdataptr=NULL; // no active entry
		}
		else if(tn==NULL)
		{
			*gdataptr=NULL; // no active entry
			nLog("Warning.. NULL in getGdataActive !\n");
		}
		else
			*gdataptr=(GroupData *)tn->tn_User;
	}
}

/* Sets the active group.
 */
void setGdataActive(int pos) {
	MUI_NListtree_TreeNode * tn;
	if(account.grouplistType==0) {
		set(NLIST_groupdata,MUIA_NList_Active,pos);
		DoMethod(NLIST_groupdata,MUIM_NList_GetEntry,MUIV_NList_GetEntry_Active,&v_gdata);
	}
	else {
		tn=(MUI_NListtree_TreeNode *)DoMethod(LISTTREE_groupdata,MUIM_NListtree_GetEntry,MUIV_NListtree_GetEntry_ListNode_Root,pos,0);
		set(LISTTREE_groupdata,MUIA_NListtree_Active,tn);
		if(tn->tn_User!=NULL)
			v_gdata=(GroupData *)tn->tn_User; // necessary since the group we want to make active might not be 'visible'
	}
}

/* Resets the activation on the group list.
 */
void resetGdataActive() {
	if(account.grouplistType==0)
		set(NLIST_groupdata,MUIA_NList_Active,MUIV_NList_Active_Top);
	else {
		DoMethod(LISTTREE_groupdata,MUIM_NListtree_Open,MUIV_NListtree_Open_ListNode_Root,tn_folders,0);
		setGdataActive(1);
	}
}

/* Returns the currently activated position in the group list.
 */
void getGdataActivePos(int *pos) {
	if(account.grouplistType==0)
		get(NLIST_groupdata,MUIA_NList_Active,pos);
	else
		get(LISTTREE_groupdata,MUIA_NList_Active,pos);
}

/* Refreshes the group list display.
 */
void redrawGdataAll() {
	nLog("redrawGdataAll() called\n");
	if(account.grouplistType==0)
		DoMethod(NLIST_groupdata,MUIM_NList_Redraw,MUIV_NList_Redraw_All);
	else
		DoMethod(LISTTREE_groupdata,MUIM_NList_Redraw,MUIV_NList_Redraw_All);
		//DoMethod(LISTTREE_groupdata,MUIM_NListtree_Redraw,MUIV_NListtree_Redraw_All);
}

/* Redraws the active entry of the group list display.
 */
void redrawGdataActive() {
	if(account.grouplistType==0)
		DoMethod(NLIST_groupdata,MUIM_NList_Redraw,MUIV_NList_Redraw_Active);
	else
		DoMethod(LISTTREE_groupdata,MUIM_NList_Redraw,MUIV_NList_Redraw_Active);
}

/* Redraws a group list display.
 */
void redrawGdata(GroupData *gdata) {
	int pos = get_gdataPos(gdata->ID);
	redrawGdataPos(pos);
}

/* Redraws a single item in the group list.
 */
void redrawGdataPos(int pos) {
	if(account.grouplistType==0)
		DoMethod(NLIST_groupdata,MUIM_NList_Redraw,pos);
	else
		DoMethod(LISTTREE_groupdata,MUIM_NList_Redraw,pos);
}

/* Removes an item in the group list.
 */
void removeGdata(int pos) {
	MUI_NListtree_TreeNode * tn;
	if(account.grouplistType==0)
		DoMethod(NLIST_groupdata,MUIM_NList_Remove,pos);
	else {
		tn=(MUI_NListtree_TreeNode *)DoMethod(LISTTREE_groupdata,MUIM_NListtree_GetEntry,MUIV_NListtree_GetEntry_ListNode_Root,pos,0);
		if(tn==NULL)
			nLog("Error!!! pos %d gives NULL in removeGdata!",pos);
		DoMethod(LISTTREE_groupdata,MUIM_NListtree_Remove,MUIV_NListtree_Remove_ListNode_Root,tn,0);
	}
}

/* Removes the active group in the list.
 */
void removeGdataActive() {
	c_gdata=NULL; // this needs to be done before removing
	MUI_NListtree_TreeNode * tn;
	if(account.grouplistType==0)
		DoMethod(NLIST_groupdata,MUIM_NList_Remove,MUIV_NList_Remove_Active);
	else
	{
		tn=(MUI_NListtree_TreeNode *)DoMethod(LISTTREE_groupdata,MUIM_NListtree_GetEntry,MUIV_NListtree_GetEntry_ListNode_Active,MUIV_NListtree_GetEntry_Position_Active,0);
		DoMethod(LISTTREE_groupdata,MUIM_NListtree_Remove,MUIV_NListtree_Remove_ListNode_Root,tn,0);
	}
}

/* Clears the group list display.
 */
void clearGdata() {
	if(account.grouplistType==0)
		DoMethod(NLIST_groupdata,MUIM_NList_Clear);
	else
		DoMethod(LISTTREE_groupdata,MUIM_NListtree_Remove,MUIV_NListtree_Remove_ListNode_Root,MUIV_NListtree_Remove_TreeNode_All,0);
}

/* Turns 'Quiet' on or off for the group list.
 */
void setGdataQuiet(BOOL q) {
	if(q)
		glist_quiet_level++;
	else {
		glist_quiet_level--;
		if(glist_quiet_level < 0)
			glist_quiet_level = 0;
	}

	/* MUIA_NList_Quiet does not contain its own nesting count
	 * (MUIA_NListtree_Quiet does, however).
	 */
	BOOL set_quiet = glist_quiet_level > 0;
	set(NLIST_groupdata,MUIA_NList_Quiet,set_quiet);
	if(LISTTREE_groupdata != NULL) {
		set(LISTTREE_groupdata,MUIA_NListtree_Quiet,q);
	}
}

/* Inserts a new GroupData.
 */
void insertGdata(GroupData * gdata,BOOL sorted) {
	if(account.grouplistType==0) {
		if(sorted)
			DoMethod(NLIST_groupdata,MUIM_NList_InsertSingle,gdata,MUIV_NList_Insert_Sorted);
		else
			DoMethod(NLIST_groupdata,MUIM_NList_InsertSingle,gdata,MUIV_NList_Insert_Bottom);
	}
	else {
		if(gdata->ID<0)
			DoMethod(LISTTREE_groupdata,MUIM_NListtree_Insert,"",gdata,tn_folders,MUIV_NListtree_Insert_PrevNode_Tail,TNF_LIST);
		else
			DoMethod(LISTTREE_groupdata,MUIM_NListtree_Insert,"",gdata,tn_newsgroups,MUIV_NListtree_Insert_PrevNode_Tail,TNF_LIST);
	}
}

/* Returns the MessageListData at position 'pos'. Counts all entries in tree
 * mode.
 */
void getMdata(int pos,MessageListData ** mdataptr) {
	if(account.mdata_view==0)
		DoMethod(NLIST_messagelistdata,MUIM_NList_GetEntry,pos,mdataptr);
	else {
		MUI_NListtree_TreeNode *tn=(MUI_NListtree_TreeNode *)DoMethod(LISTTREE_messagelistdata,MUIM_NListtree_GetEntry,MUIV_NListtree_GetEntry_ListNode_Root,MUIV_NListtree_GetEntry_Position_Head,0);
		MUI_NListtree_TreeNode *temp_tn = NULL;
		for(int i=0; i<pos && tn!=NULL; i++) {
			temp_tn=(MUI_NListtree_TreeNode *)DoMethod(LISTTREE_messagelistdata,MUIM_NListtree_GetEntry,tn,MUIV_NListtree_GetEntry_Position_Head,0);
			if(temp_tn!=NULL) // we have child
				tn = temp_tn;
			else { // no child
				do {
					temp_tn=(MUI_NListtree_TreeNode *)DoMethod(LISTTREE_messagelistdata,MUIM_NListtree_GetEntry,tn,MUIV_NListtree_GetEntry_Position_Next,0);
					if(temp_tn!=NULL) // next in list
						break;
					// end of this list, go to parent then next
					tn=(MUI_NListtree_TreeNode *)DoMethod(LISTTREE_messagelistdata,MUIM_NListtree_GetEntry,tn,MUIV_NListtree_GetEntry_Position_Parent,0);
				} while(tn!=NULL);
				if(tn!=NULL)
					tn = temp_tn;
			}
		}

		if(tn==NULL) {
			nLog("Error!!! pos %d gives NULL in getMdata !\n",pos);
			*mdataptr=NULL;
		}
		else {
			*mdataptr=(MessageListData *)tn->tn_User;
			if(tn->tn_User==NULL)
				nLog("null mdata! name :*%s*\n",tn->tn_Name);
		}
	}
}

void forAllMdata(mdataIteratorFunc *iterator,void *data)
{
	MessageListData *mdata = NULL;
	if(account.mdata_view==0)
	{
		ULONG entries = 0;
		get(NLIST_messagelistdata,MUIA_NList_Entries,&entries);
		for(ULONG i=0;i<entries;i++)
		{
			DoMethod(NLIST_messagelistdata,MUIM_NList_GetEntry,i,&mdata);
			//printf(":%d\n",i);
			iterator(mdata,i,data);
		}
	}
	else
	{
		MUI_NListtree_TreeNode *tn=(MUI_NListtree_TreeNode *)DoMethod(LISTTREE_messagelistdata,MUIM_NListtree_GetEntry,MUIV_NListtree_GetEntry_ListNode_Root,MUIV_NListtree_GetEntry_Position_Head,0);
		for(int i=0; /*i<pos &&*/ tn!=NULL; i++)
		{
			MessageListData *mdata = (MessageListData *)tn->tn_User;
			if(mdata == NULL) {
				//printf("ERROR!!! mdata is NULL in forAllMdata\n");
				nLog("ERROR!!! mdata is NULL in forAllMdata\n");
			}
			//printf(":%d\n",i);
			iterator(mdata,i,data);
			MUI_NListtree_TreeNode *temp_tn = (MUI_NListtree_TreeNode *)DoMethod(LISTTREE_messagelistdata,MUIM_NListtree_GetEntry,tn,MUIV_NListtree_GetEntry_Position_Head,0);
			if(temp_tn!=NULL) // we have child
				tn = temp_tn;
			else { // no child
				do {
					temp_tn=(MUI_NListtree_TreeNode *)DoMethod(LISTTREE_messagelistdata,MUIM_NListtree_GetEntry,tn,MUIV_NListtree_GetEntry_Position_Next,0);
					if(temp_tn!=NULL) // next in list
						break;
					// end of this list, go to parent then next
					tn=(MUI_NListtree_TreeNode *)DoMethod(LISTTREE_messagelistdata,MUIM_NListtree_GetEntry,tn,MUIV_NListtree_GetEntry_Position_Parent,0);
				} while(tn!=NULL);
				if(tn!=NULL)
					tn = temp_tn;
			}
		}
	}
}

/* Returns the MessageListData at position 'pos'. For a tree view, only
 * visible (ie, not in a closed node) messages are counted.
 */
void getMdataVisible(int pos,MessageListData ** mdataptr)
{
	MUI_NListtree_TreeNode * tn;
	if(account.mdata_view==0)
		DoMethod(NLIST_messagelistdata,MUIM_NList_GetEntry,pos,mdataptr);
	else
	{
		DoMethod(LISTTREE_messagelistdata,MUIM_NList_GetEntry,pos,&tn);
		*mdataptr=(MessageListData *)tn->tn_User;
	}
}

int getMdataVisibleEntries()
{
	ULONG entries = 0;
	if(account.mdata_view==0)
		get(NLIST_messagelistdata,MUIA_NList_Entries,&entries);
	else
		get(LISTTREE_messagelistdata,MUIA_NList_Entries,&entries);
	return entries;
}

/* Returns the MessageListData that is currently active - this will be NULL
 * if no message is active.
 */
void getMdataActive(MessageListData ** mdataptr) {
	MUI_NListtree_TreeNode * tn;
	if(account.mdata_view==0)
		DoMethod(NLIST_messagelistdata,MUIM_NList_GetEntry,MUIV_NList_GetEntry_Active,mdataptr);
	else
	{
		get(LISTTREE_messagelistdata,MUIA_NListtree_Active,(ULONG*)&tn);
		if(tn==MUIV_NListtree_Active_Off)
			*mdataptr=NULL; // no active entry
		else
			*mdataptr=(MessageListData *)tn->tn_User;
	}
}

/* Sets the active MessageListData. 'visible' is whether to count only
 * visible entries in a tree view.
 */
void setMdataActive(int pos,BOOL visible) {
	if(account.mdata_view==0)
		set(NLIST_messagelistdata,MUIA_NList_Active,pos);
	else {
		if(visible)
			set(LISTTREE_messagelistdata,MUIA_NList_Active,pos);
		else {
			MUI_NListtree_TreeNode * tn = (MUI_NListtree_TreeNode *)DoMethod(LISTTREE_messagelistdata,MUIM_NListtree_GetEntry,MUIV_NListtree_GetEntry_ListNode_Root,pos,0);
			//DoMethod(LISTTREE_messagelistdata,MUIM_NListtree_Open,MUIV_NListtree_Open_ListNode_Parent,tn,0);
			openMdata(pos);
			set(LISTTREE_messagelistdata,MUIA_NListtree_Active,tn);
			//printf("%d : %s\n",
			//	((MessageListData *)tn->tn_User)->ID,
			//	((MessageListData *)tn->tn_User)->subject );
		}
	}
}

void setMdataActive(int pos) {
	setMdataActive(pos,FALSE);
}

/* Returns the active MessageListData position. 'visible' is whether to
 * count only visible entries in a tree view.
 */
void getMdataActivePos(int *pos,BOOL visible) {
	MUI_NListtree_TreeNode * tn;
	if(account.mdata_view==0)
		get(NLIST_messagelistdata,MUIA_NList_Active,pos);
	else {
		if(visible)
			get(LISTTREE_messagelistdata,MUIA_NList_Active,pos);
		else
		{
			get(LISTTREE_messagelistdata,MUIA_NListtree_Active,(ULONG*)&tn);
			if(tn==MUIV_NListtree_Active_Off)
				*pos=MUIV_NList_Active_Off;
			else
				*pos = DoMethod(LISTTREE_messagelistdata,MUIM_NListtree_GetNr,tn,0);
		}
	}
}

void getMdataActivePos(int *pos) {
	getMdataActivePos(pos,FALSE);
}

/* Returns the Tree Node for the MessageListData with 'messageID', or NULL
 * if either it doesn't exist in this group, or not in the tree view mode.
 */
MUI_NListtree_TreeNode *findMdataTreeNode(char *messageID) {
	if(account.mdata_view==0)
		return NULL;
	MUI_NListtree_TreeNode *tn=(MUI_NListtree_TreeNode *)DoMethod(LISTTREE_messagelistdata,MUIM_NListtree_GetEntry,MUIV_NListtree_GetEntry_ListNode_Root,MUIV_NListtree_GetEntry_Position_Head,0);
	MUI_NListtree_TreeNode *temp_tn = NULL;

	for(int i=0; tn!=NULL; i++) {
		MessageListData *mdata = (MessageListData *)tn->tn_User;
		if(strcmp(mdata->messageID,messageID)==0)
			break;

		temp_tn=(MUI_NListtree_TreeNode *)DoMethod(LISTTREE_messagelistdata,MUIM_NListtree_GetEntry,tn,MUIV_NListtree_GetEntry_Position_Head,0);
		if(temp_tn!=NULL) // we have child
			tn = temp_tn;
		else { // no child
			do {
				temp_tn=(MUI_NListtree_TreeNode *)DoMethod(LISTTREE_messagelistdata,MUIM_NListtree_GetEntry,tn,MUIV_NListtree_GetEntry_Position_Next,0);
				if(temp_tn!=NULL) // next in list
					break;
				// end of this list, go to parent then next
				tn=(MUI_NListtree_TreeNode *)DoMethod(LISTTREE_messagelistdata,MUIM_NListtree_GetEntry,tn,MUIV_NListtree_GetEntry_Position_Parent,0);
			} while(tn!=NULL);
			if(tn!=NULL)
				tn = temp_tn;
		}
	}
	return tn;
}

/* Redraws the message list display.
 */
void redrawMdataAll() {
	if(account.mdata_view==0)
		DoMethod(NLIST_messagelistdata,MUIM_NList_Redraw,MUIV_NList_Redraw_All);
	else
		DoMethod(LISTTREE_messagelistdata,MUIM_NList_Redraw,MUIV_NList_Redraw_All);
}

/* Redraws the active entry of the messagelist display.
 */
void redrawMdataActive() {
	if(account.mdata_view==0)
		DoMethod(NLIST_messagelistdata,MUIM_NList_Redraw,MUIV_NList_Redraw_Active);
	else
		DoMethod(LISTTREE_messagelistdata,MUIM_NList_Redraw,MUIV_NList_Redraw_Active);
}

/* Redraws the entry at position 'pos' of the messagelist display.
 * For tree views, this goes by visible entries.
 */
void redrawMdata(int pos) {
	if(account.mdata_view==0)
		DoMethod(NLIST_messagelistdata,MUIM_NList_Redraw,pos);
	else
		DoMethod(LISTTREE_messagelistdata,MUIM_NList_Redraw,pos);
}

/* Is the message at position 'pos' in the list selected? 'visible' is whether
 * to count only visible entries in a tree view.
 */
BOOL isMdataSelected(int pos,BOOL visible) {
	BOOL sel = FALSE;
	int res = 0;
	if(account.mdata_view==0) {
		DoMethod(NLIST_messagelistdata,MUIM_NList_Select,pos,MUIV_NList_Select_Ask,&res);
		if(res==MUIV_NList_Select_On)
			sel=TRUE;
	}
	else {
		if(visible) {
			DoMethod(LISTTREE_messagelistdata,MUIM_NList_Select,pos,MUIV_NList_Select_Ask,&res);
			if(res==MUIV_NList_Select_On)
				sel=TRUE;
		}
		else {
			MUI_NListtree_TreeNode * tn = (MUI_NListtree_TreeNode *)DoMethod(LISTTREE_messagelistdata,MUIM_NListtree_GetEntry,MUIV_NListtree_GetEntry_ListNode_Root,pos,0);
			/*
			// This doesn't seem to work?
			DoMethod(LISTTREE_messagelistdata,MUIM_NListtree_Select,tn,MUIV_NListtree_Select_Ask,0,&res);
			printf("%d : %d\n",pos,res);
			if(res==MUIV_NListtree_Select_On)
				sel=TRUE;
			*/
			int visible_pos = DoMethod(LISTTREE_messagelistdata,MUIM_NListtree_GetNr,tn,MUIV_NListtree_GetNr_Flag_Visible);
			if( visible_pos == -1 ) {
				// invisible, so not selected
				/* I'm not sure if MUIV_NListtree_Select_Ask not working is a bug or not.
				 * Either way, it is important that hidden entries are treated as not being
				 * selected (ie, irrespective of whether NListree might still consider a
				 * hidden message to selected or not). The standard behaviour in NewsCoaster
				 * when looking for selected messages is (and should always be) to ignore
				 * hidden entries. It would probably not be a good idea, for example, for
				 * hidden entries to be deleted because they were still selected, but  the
				 * user did not realise this!
				 */
			}
			else {
				DoMethod(LISTTREE_messagelistdata,MUIM_NList_Select,visible_pos,MUIV_NList_Select_Ask,&res);
				if(res==MUIV_NList_Select_On)
					sel=TRUE;
				//printf("%d , %d : %d\n",pos,visible_pos,res);
			}
		}
	}
	return sel;
}

BOOL isMdataSelected(int pos) {
	return isMdataSelected(pos, TRUE);
}

/* Set the selection status of the message at position 'pos' in the list.
 * 'visible' is whether to count only visible entries in a tree view.
 */
void setMdataSelected(int pos,BOOL select,BOOL visible) {
//	int res = 0;
	if(account.mdata_view==0) {
		DoMethod(NLIST_messagelistdata,MUIM_NList_Select,pos,select ? MUIV_NList_Select_On : MUIV_NList_Select_Off,NULL);
	}
	else {
		if(visible) {
			DoMethod(LISTTREE_messagelistdata,MUIM_NList_Select,pos,select ? MUIV_NList_Select_On : MUIV_NList_Select_Off,NULL);
		}
		else {
			MUI_NListtree_TreeNode * tn = (MUI_NListtree_TreeNode *)DoMethod(LISTTREE_messagelistdata,MUIM_NListtree_GetEntry,MUIV_NListtree_GetEntry_ListNode_Root,pos,0);
			DoMethod(LISTTREE_messagelistdata,MUIM_NListtree_Select,tn,select ? MUIV_NListtree_Select_On : MUIV_NListtree_Select_Off,0,NULL);
			if( select ) {
				// make sure the node is actually visible
				openMdata(pos);
			}
		}
	}
}

/* Position works on all (ie, not just visible) entries in the tree.
 */
void openMdata(int pos) {
	if(account.mdata_view!=0) {
		MUI_NListtree_TreeNode * tn = NULL;
		tn = (MUI_NListtree_TreeNode *)DoMethod(LISTTREE_messagelistdata,MUIM_NListtree_GetEntry,MUIV_NListtree_GetEntry_ListNode_Root,pos,0);
		tn = (MUI_NListtree_TreeNode *)DoMethod(LISTTREE_messagelistdata,MUIM_NListtree_GetEntry,tn,MUIV_NListtree_GetEntry_Position_Parent,0);
		DoMethod(LISTTREE_messagelistdata,MUIM_NListtree_Open,MUIV_NListtree_Open_ListNode_Parent,tn,0);
	}
}

struct getMdataPos_data {
	int pos;
	int mID;
};

void getMdataPos_iterator(MessageListData *mdata,int index,void *data) {
	getMdataPos_data *i_data = (getMdataPos_data *)data;
	if(mdata->ID == i_data->mID)
		i_data->pos = index;
}

int getMdataPos(int mID) {
	/*MessageListData * mdata = NULL;
	int entries = 0;
	get(NLIST_messagelistdata,MUIA_NList_Entries,&entries);
	if(entries==0)
		return -1;
	for(int k=0;k<entries;k++) {
		getMdata(k,&mdata);
		if(mdata->ID == mID)
			return k;
	}
	return -1;
	*/
	getMdataPos_data data = { -1, mID };
	forAllMdata(getMdataPos_iterator,&data);
	return data.pos;
}

/* Turns 'Quiet' on or off for the message list.
 */
void setMdataQuiet(BOOL q) {
	if(q)
		mlist_quiet_level++;
	else {
		mlist_quiet_level--;
		if(mlist_quiet_level < 0)
			mlist_quiet_level = 0;
	}

	/* MUIA_NList_Quiet does not contain its own nesting count
	 * (MUIA_NListtree_Quiet does, however).
	 */
	BOOL set_quiet = mlist_quiet_level > 0;
	set(NLIST_messagelistdata,MUIA_NList_Quiet,set_quiet);
	if(LISTTREE_messagelistdata != NULL) {
		set(LISTTREE_messagelistdata,MUIA_NListtree_Quiet,q);
	}
}

int get_gdataPos(int gID) {
	int entries = 0;
	GroupData * gdata = NULL;
	getGdataEntries(&entries);
	for(int k=0;k<entries;k++) {
		getGdata(k,&gdata);
		if(gdata) {
			if(gID == gdata->ID)
				return k;
		}
	}
	return -1;
}

int get_gdataPos(char * name) {
	int entries = 0;
	getGdataEntries(&entries);
	for(int k=0;k<entries;k++) {
		GroupData *gdata = NULL;
		getGdata(k,&gdata);
		if(gdata != NULL) {
			if(stricmp(name,gdata->name)==0)
				return k;
		}
	}
	return -1;
}

BOOL set_gdata(int gID)
{
	nLog("set_gdata((int)%d) called\n",gID);
	int pos=get_gdataPos(gID);
	if(pos!=-1)
	{
		setGdataActive(pos);
		gdata_readID=gID;
		return TRUE;
	}
	return FALSE;
}

BOOL set_gdata(GroupData * gdata)
{
	nLog("set_gdata((GroupData *)%d) called\n",gdata);
	return set_gdata(gdata->ID);
}

BOOL set_gdata(char * name)
{
	nLog("set_gdata((char *)%s) called\n",name);
	int pos=get_gdataPos(name);
	if(pos!=-1)
	{
		setGdataActive(pos);
		GroupData * gdata;
		getGdata(pos,&gdata);
		gdata_readID=gdata->ID;
		return TRUE;
	}
	return FALSE;
}

BOOL set_mdata(int mID,BOOL hidden)
{
	nLog("set_mdata((int)%d) called\n",mID);
	BOOL okay=FALSE;
	int pos = -1;
	if(hidden)
	{
		ULONG entries = 0;
		get(NLIST_messagelistdata,MUIA_NList_Entries,&entries);
		for(ULONG k=0;k<entries;k++)
		{
			MessageListData * mdata = NULL;
			DoMethod(NLIST_messagelistdata,MUIM_NList_GetEntry,k,&mdata);
			if(mdata->ID == mID)
			{
				okay = TRUE;
				pos = k;
				break;
			}
		}
	}
	else {
		pos = getMdataPos(mID);
		okay = (pos != -1);
	}
	if(okay) {
		if(hidden)
			set(NLIST_messagelistdata,MUIA_NList_Active,pos);
		else
			setMdataActive(pos);
	}
	return okay;
}

BOOL set_mdata(int mID)
{
	return set_mdata(mID,FALSE);
}

BOOL set_mdata(MessageListData * mdata,BOOL hidden)
{
	nLog("set_mdata((MessageListData *)%d,(BOOL)%d) called\n",mdata,hidden);
	return set_mdata(mdata->ID,hidden);
}

BOOL set_mdata(MessageListData * mdata)
{
	return set_mdata(mdata,FALSE);
}

int get_mdata(MessageListData ** mdataptr,int mID)
{
	// for currently displayed index only!
	nLog("get_mdata((MessageListData **)%d,(int)%d) called\n",mdataptr,mID);
	ULONG entries = 0;
	get(NLIST_messagelistdata,MUIA_NList_Entries,&entries);
	for(ULONG l=0;l<entries;l++)
	{
		DoMethod(NLIST_messagelistdata,MUIM_NList_GetEntry,l,mdataptr);
		if((*mdataptr)->ID==mID)
			return l;
	}
	*mdataptr = NULL;
	return -1;
}

BOOL get_mdata(MessageListData ** mdataptr,GroupData * gdata,int mID,BOOL need_flags) {
	GroupData * gdata2;
	getGdataDisplayed(&gdata2);
	if(gdata->ID == gdata2->ID) {
		get_mdata(mdataptr,mID);
		return FALSE;
	}
	if(need_flags) {
		// need to read index
		sleepAll(TRUE);
		get_mdata_from_index(mdataptr,gdata,mID);
		/*Vector *vector = new Vector(2048);
		read_index(gdata,vector);
		MessageListData **mdatas = (MessageListData **)vector->getData();
		int len = vector->getSize();
		int i=0;
		for(i=0;i<len;i++) {
			if(mdatas[i]->ID == mID) {
				*mdataptr = mdatas[i];
				break;
			}
		}
		// delete all the rest
		for(i=0;i<len;i++) {
			if(mdatas[i]->ID != mID)
				delete mdatas[i];
		}
		delete vector;*/
		sleepAll(FALSE);
	}
	else {
		// this code doesn't get the flags, but is quicker
		NewMessage nm;
		get_refs(&nm,gdata->ID,mID,GETREFS_LAST);
		*mdataptr = new MessageListData();
		nm.copyToMessageListData(*mdataptr);
		(*mdataptr)->ID=mID;
	}
	return TRUE;
}

int get_mdata_pos(int mID)
{
	nLog("get_mdata_pos((int)%d) called\n",mID);
	ULONG k=0,entries=0;
	MessageListData * mdata2=NULL;
	get(NLIST_messagelistdata,MUIA_NList_Entries,&entries);
	for(k=0;k<entries;k++)
	{
		DoMethod(NLIST_messagelistdata,MUIM_NList_GetEntry,k,&mdata2);
		if(mID==mdata2->ID)
			return k;
	}
	return -1;
}

int get_mdataptr_pos(MessageListData *mdata)
{
	nLog("get_mdataptr_pos((int)%d) called\n",mdata);
	// do not change this to access the ID; compare on pointers (in case pointer has been deleted)
	ULONG entries=0, k=0;
	get(NLIST_messagelistdata,MUIA_NList_Entries,&entries);
	for(k=0;k<entries;k++)
	{
		MessageListData * mdata2 = NULL;
		DoMethod(NLIST_messagelistdata,MUIM_NList_GetEntry,k,&mdata2);
		if(mdata==mdata2)
			return k;
	}
	return -1;
}

void get_mdata_pos(int * pos,char * messageID)
{
	ULONG entries=0,l=0;
	get(NLIST_messagelistdata,MUIA_NList_Entries,&entries);
	static MessageListData * mdata = NULL;
	for(l=0;l<entries;l++)
	{
		DoMethod(NLIST_messagelistdata,MUIM_NList_GetEntry,l,&mdata);
		if(stricmp(mdata->messageID,messageID)==0)
		{
			*pos = l;
			return;
		}
	}
	*pos = -1;
}

void get_gdata(GroupData ** gdataptr,int gID)
{
	nLog("get_gdata((GroupData **)%d,(int)%d) called\n",gdataptr,gID);
	int entries = 0;
	getGdataEntries(&entries);
	for(int l=0;l<entries;l++)
	{
		getGdata(l,gdataptr);
		if(*gdataptr) {
			if((*gdataptr)->ID==gID)
				return;
		}
	}
	*gdataptr=NULL;
}

void get_gdata(GroupData ** gdataptr,const char * name)
{
	nLog("get_gdata((GroupData **)%d,(char *)%s) called\n",gdataptr,name);
	int entries = 0;
	char name2[256]="";
	getGdataEntries(&entries);

	for(int l=0;l<entries;l++)
	{
		getGdata(l,gdataptr);
		if(*gdataptr==NULL)
			continue;
		if(stricmp(name,(*gdataptr)->name)==0)
			return;
		sprintf(name2,"%s,",(*gdataptr)->name);
		if(strstr(name,name2)) // crossposts
			return;
		sprintf(name2,",%s",(*gdataptr)->name);
		if(strstr(name,name2)) // crossposts
			return;
	}
	*gdataptr=NULL;
}
