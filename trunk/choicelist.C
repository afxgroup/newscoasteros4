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

#include "vector.h"
#include "various.h"
#include "main.h"
#include "choicelist.h"

int ChoiceList::show()
{
	Object *tempGroup = NULL;
	if(extraGroup==NULL) {
		wnd = WindowObject,
			MUIA_Window_Title,	title,
			MUIA_Window_ID,		MAKE_ID('C','H','C','L'),
			WindowContents, VGroup,
				Child, NListviewObject,
					MUIA_NListview_NList,		NLIST_choice=NListObject,
						End,
					End,
				Child, group=VGroup,
					End,
				End,
			End;
	}
	else
	{
		wnd = WindowObject,
			MUIA_Window_Title,	title,
			MUIA_Window_ID,		MAKE_ID('C','H','C','L'),
			WindowContents, VGroup,
				Child, NListviewObject,
					MUIA_NListview_NList,		NLIST_choice=NListObject,
						End,
					End,
				Child, tempGroup=VGroup,
					End,
				Child, group=VGroup,
					End,
				End,
			End;
	}
	if(!wnd)
		return -1;
	if(extraGroup!=NULL)
		DoMethod(tempGroup,OM_ADDMEMBER,extraGroup);

	Object ** Buttons=NULL;
	Object * buttonGroup = HGroup, End;

	if(nb>0)
	{
		Buttons = new Object *[nb];
		for(int k=0;k<nb;k++)
		{
			Buttons[k] = SimpleButton(buttons[k]);
			if(!Buttons[k])
				return -1;
			DoMethod(buttonGroup,OM_ADDMEMBER,Buttons[k]);
			DoMethod(Buttons[k],MUIM_Notify,MUIA_Pressed,FALSE,
				app,2,MUIM_Application_ReturnID,TOP_ENUM+k+1);
		}
	}
	DoMethod(group,OM_ADDMEMBER,buttonGroup);
	DoMethod(NLIST_choice,MUIM_NList_Insert,entries,count,MUIV_NList_Insert_Top);
	DoMethod(NLIST_choice,MUIM_Notify,MUIA_NList_DoubleClick,MUIV_EveryTime,
		app,2,MUIM_Application_ReturnID,TOP_ENUM+1);
	DoMethod(wnd,MUIM_Notify,MUIA_Window_CloseRequest,TRUE,
		app,2,MUIM_Application_ReturnID,-1);

	set(app,MUIA_Application_Sleep,TRUE);
	DoMethod(app,OM_ADDMEMBER,wnd);
	set(NLIST_choice,MUIA_NList_Active,MUIV_NList_Active_Top);
	set(wnd,MUIA_Window_Open,TRUE);

	int u_signal;
	BOOL running=TRUE;
	int res=0;

	while(running)
	{
		res=DoMethod(app,MUIM_Application_NewInput,&u_signal);
		if(res<0 || res>TOP_ENUM)
		{
			running=FALSE;
		}
		if(running && u_signal)
			u_signal = Wait(u_signal);
	}

	get(NLIST_choice,MUIA_NList_Active,&selected);
	set(wnd,MUIA_Window_Open,FALSE);
	if(extraGroup!=NULL) // we must remove this group since we do not wish to dispose it! (if it needs disposing, do it yourself!)
		DoMethod(tempGroup,OM_REMMEMBER,extraGroup);
	DoMethod(app,OM_REMMEMBER,wnd);
	MUI_DisposeObject(wnd);
	if(Buttons)
		delete [] Buttons;
	set(app,MUIA_Application_Sleep,FALSE);
	if(res==-1)
		return -1;
	else
		return (res-1-TOP_ENUM);
}

int ChoiceList::getSelected() {
	return selected;
}
