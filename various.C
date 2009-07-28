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

#include <string.h>
#include <stdio.h>

#include "vector.h"
#include "various.h"
#include "misc.h"
#include "strings.h"
#include "datehandler.h"

MessageListData::MessageListData() {
}

void MessageListData::init() {
	memset(this,0,sizeof(MessageListData));
}

/* Translate a NewMessage to a MessageListData.
 */
void NewMessage::copyToMessageListData(MessageListData * mdata) {
	char *translated_string=NULL;

	nLog("NewMessage::copyToMessageListData() called\n");
	strncpy(mdata->from,this->from,IHEADENDSHORT);
	mdata->from[IHEADENDSHORT]=0;
	stripEscapes(mdata->from);
	strncpy(mdata->newsgroups,this->newsgroups,IHEADENDSHORT);
	mdata->newsgroups[IHEADENDSHORT]=0;
	stripEscapes(mdata->newsgroups);
	strncpy(mdata->subject,this->subject,IHEADENDSUBJECT);
	mdata->subject[IHEADENDSUBJECT]=0;
	stripEscapes(mdata->subject);

	//strncpy(mdata->date,this->date,IHEADENDSHORT);
	//mdata->date[IHEADENDSHORT]=0;
	//DateHandler::read_date(&mdata->ds,mdata->date,mdata->c_date,mdata->c_time);
	DateHandler::read_date(&mdata->ds,this->date,mdata->c_date,mdata->c_time);

	mdata->datec[0] = '\0';
	strncpy(mdata->messageID,this->messageID,IHEADENDMID);
	mdata->messageID[IHEADENDMID]=0;
	stripEscapes(mdata->messageID);
	mdata->mIDhash = this->mIDhash;
	if(this->nrefs>0) {
		strncpy(mdata->lastref,this->references[nrefs-1],IHEADENDMID);
		mdata->lastref[IHEADENDMID]=0;
	}
	else
		*mdata->lastref=0;
	/*strncpy(mdata->type,this->type,IHEADENDSHORT-4);
	mdata->type[IHEADENDSHORT-4]=0;*/
	mdata->flags[8]=this->lines;
	mdata->flags[12]=this->online;

	if( *this->newsgroups != '\0' )
		mdata->flags[4] = TRUE;
	if( *this->to != '\0' )
		mdata->flags[5] = TRUE;
}

//STATIC ASM SAVEDS VOID hook_func_standard(REG(a0,struct Hook *h), REG(a2, Object *obj), REG(a1, ULONG * funcptr)) {
HOOK2( VOID, hook_func_standard, Object *, obj, a2, ULONG *, funcptr, a1 )
	void (*func) (ULONG *) = (void (*)(ULONG *)) (*funcptr);

	if (func)
		func(funcptr + 1);
}

CPPHook hook_standard = { { NULL,NULL },(CPPHOOKFUNC)&hook_func_standard,NULL,NULL };
