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

#include <mui/NListview_mcc.h>

#include <proto/locale.h>
#include <proto/socket.h>
#ifdef __STORM__
#include <pragma/socket_lib.h>
#endif

/* Do not change the order of the following two includes, this leads to
   'ERANGE redefinition' error
*/
#include <sys/errno.h>
#include "mui_headers.h"

#include <time.h>
#include <assert.h>

#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "vector.h"
#include "main.h"
#include "various.h"
#include "writewindow.h"
#include "indices.h"
#include "misc.h"
#include "datehandler.h"
#include "statuswindow.h"
#include "connection.h"
#include "netstuff.h"
#include "strings.h"
#include "viewwindow.h"
#include "lists.h"
#include "subthreads.h"
#include "newscoaster_catalog.h"

int onlineflag_len = strlen(onlineflag);

static char newline[3] = {13,10,0};

Connection *online_c = NULL;

/* Data to be passed to a subthread.
 */
struct ThreadData {
	StatusWindow *status;
	Server *server;
};

/* Wrapper for MUI_RequestA().
 * Should probably be moved elsewhere!
 */
void mui_alert(char *title, char *button,char *text) {
	MUI_RequestA(app,0,0,title,button,text,0);
}

void initNetstuff() {
	SocketBase=NULL;
}

/* Hang up the connection.
 */
void hangUp(Connection *c) {
	c->send_data("quit\r\n");
	c->close();
}

void readFailedMessage(BOOL subthread = FALSE) {
	char buffer[256] = "";
	int err = Errno();
	if( err == EBADF )
		strcpy(buffer,CatalogStr(MSG_RECV_FAILED_EBADF,MSG_RECV_FAILED_EBADF_STR));
	else if( err == ENOTCONN )
		strcpy(buffer,CatalogStr(MSG_RECV_FAILED_ENOTCONN,MSG_RECV_FAILED_ENOTCONN_STR));
	else if( err == EWOULDBLOCK )
		strcpy(buffer,CatalogStr(MSG_RECV_FAILED_EWOULDBLOCK,MSG_RECV_FAILED_EWOULDBLOCK_STR));
	else if( err == EINTR )
		strcpy(buffer,CatalogStr(MSG_RECV_FAILED_EINTR,MSG_RECV_FAILED_EINTR_STR));
	else
		sprintf(buffer,CatalogStr(MSG_RECV_FAILED_UNKNOWN,MSG_RECV_FAILED_UNKNOWN_STR),err);

	sprintf(status_buffer_g,CatalogStr(MSG_CANNOT_USE_SERVER,MSG_CANNOT_USE_SERVER_STR),buffer);
	if( subthread ) {
		if( !thread_aborted() )
			thread_call_parent_function_sync((void *)mui_alert,3,CatalogStr(MSG_ERROR,MSG_ERROR_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),status_buffer_g);
	}
	else
		mui_alert(CatalogStr(MSG_ERROR,MSG_ERROR_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),status_buffer_g);
}

/* Authenticate to the newserver 'server' through connection 'c'.
 */
int auth(Connection *c,Server *server,StatusWindow * statusWindow) {
	nLog("auth((Connection *)%d,(Server *)%d,(StatusWindow *)%d) called\n",c,server,statusWindow);
	char *line_buffer = new char[MAXLINE+1];
	//char buffer[4097] = "";
	if(server->nntp_auth) {
		statusWindow->setText(CatalogStr(MSG_AUTHENTICATING,MSG_AUTHENTICATING_STR));
		sprintf(line_buffer,"AUTHINFO USER %s%s",server->user,newline);
		c->send_data(line_buffer);
		if( c->read_data(line_buffer) == -1 ) {
			readFailedMessage(statusWindow->is_subthread);
			delete [] line_buffer;
			return FALSE;
		}
		StripNewLine(line_buffer);
		if(*line_buffer!='3') {
			nLog("USER Error: %s\n",line_buffer);
			//MUI_RequestA(app,0,0,CatalogStr(MSG_ERROR,MSG_ERROR_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_UNKNOWN_USER,MSG_UNKNOWN_USER_STR),0);
			if( statusWindow->is_subthread ) {
				if( !thread_aborted() )
					thread_call_parent_function_sync((void *)mui_alert,3,CatalogStr(MSG_ERROR,MSG_ERROR_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_UNKNOWN_USER,MSG_UNKNOWN_USER_STR));
			}
			else
				mui_alert(CatalogStr(MSG_ERROR,MSG_ERROR_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_UNKNOWN_USER,MSG_UNKNOWN_USER_STR));
			delete [] line_buffer;
			return FALSE;
		}

		sprintf(line_buffer,"AUTHINFO PASS %s%s",server->password,newline);
		c->send_data(line_buffer);
		if( c->read_data(line_buffer) == -1 ) {
			readFailedMessage(statusWindow->is_subthread);
			delete [] line_buffer;
			return FALSE;
		}
		StripNewLine(line_buffer);
		if(*line_buffer=='5') {
			nLog("PASS Error: %s\n",line_buffer);
			//MUI_RequestA(app,0,0,CatalogStr(MSG_ERROR,MSG_ERROR_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_INCORRECT_PASSWORD,MSG_INCORRECT_PASSWORD_STR),0);
			if( statusWindow->is_subthread ) {
				if( !thread_aborted() )
					thread_call_parent_function_sync((void *)mui_alert,3,CatalogStr(MSG_ERROR,MSG_ERROR_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_INCORRECT_PASSWORD,MSG_INCORRECT_PASSWORD_STR));
			}
			else
				mui_alert(CatalogStr(MSG_ERROR,MSG_ERROR_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_INCORRECT_PASSWORD,MSG_INCORRECT_PASSWORD_STR));
			delete [] line_buffer;
			return FALSE;
		}
		statusWindow->setText(CatalogStr(MSG_IN,MSG_IN_STR));
	}
	// no call to doOnlineBlocking() !
	delete [] line_buffer;
	return TRUE;
}

void groups_downloaded(Server *server,int found)
{
	account.save_data();
	ULONG val = 0;
	get(wnd_groupman,MUIA_Window_Open,&val);
	if(val==FALSE)
	{
		sprintf(status_buffer_g,CatalogStr(MSG_GROUP_LIST_DOWNLOADED,MSG_GROUP_LIST_DOWNLOADED_STR),found,server->nntp);
		MUI_RequestA(app,0,0,CatalogStr(MSG_GROUP_LIST,MSG_GROUP_LIST_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),status_buffer_g,0);
	}
	// todo: if groupman window *is* open, we need to refresh the list!
}

static int getgroups_entry(struct ThreadData *msg)
{
	thread_parent_task_can_contiue();

	Server *server = msg->server;
	StatusWindow *statusWindow = msg->status;
	//msg->success = FALSE;

	/*nLog("getgroups_entry >>> server ptr %d\n",server);
	delete statusWindow;
	return 1;*/

	closeSockets();
	Connection *c = new Connection();
	if(!c->init()) {
		//MUI_RequestA(app,0,0,CatalogStr(MSG_ERROR,MSG_ERROR_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_NO_TCP_STACK,MSG_NO_TCP_STACK_STR),0);
		if( !thread_aborted() )
			thread_call_parent_function_sync((void *)mui_alert,3,CatalogStr(MSG_ERROR,MSG_ERROR_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_NO_TCP_STACK,MSG_NO_TCP_STACK_STR));
		delete c;
		delete statusWindow;
		delete msg;
		//return -1;
		return 1;
	}
	// open a socket
	char *line_buffer = new char[MAXLINE+1];
	//StatusWindow *statusWindow = new StatusWindow(app,CatalogStr(MSG_DOWNLOADING_GROUP_LIST,MSG_DOWNLOADING_GROUP_LIST_STR));
	sprintf(status_buffer_g,CatalogStr(MSG_CONNECTING_WITH_SERVER,MSG_CONNECTING_WITH_SERVER_STR),server->nntp);
	statusWindow->setText(status_buffer_g);
	if(!c->call_socket(server)) {
		//MUI_RequestA(app,0,0,CatalogStr(MSG_ERROR,MSG_ERROR_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_CANNOT_CONNECT_WITH_SERVER,MSG_CANNOT_CONNECT_WITH_SERVER_STR),0);
		if( !thread_aborted() )
			thread_call_parent_function_sync((void *)mui_alert,3,CatalogStr(MSG_ERROR,MSG_ERROR_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_CANNOT_CONNECT_WITH_SERVER,MSG_CANNOT_CONNECT_WITH_SERVER_STR));
		delete c;
		delete [] line_buffer;
		delete statusWindow;
		delete msg;
		//return -1;
		return 1;
	}
	statusWindow->setText(CatalogStr(MSG_CONNECTED,MSG_CONNECTED_STR));
	if( c->read_data(line_buffer) == -1 ) {
		readFailedMessage(TRUE);
		delete c;
		delete [] line_buffer;
		delete statusWindow;
		delete msg;
		//return -1;
		return 1;
	}
	StripNewLine(line_buffer);
	if(*line_buffer!='2') {
		sprintf(status_buffer_g,CatalogStr(MSG_CANNOT_USE_SERVER,MSG_CANNOT_USE_SERVER_STR),line_buffer);
		//MUI_RequestA(app,0,0,CatalogStr(MSG_ERROR,MSG_ERROR_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),status_buffer_g,0);
		if( !thread_aborted() )
			thread_call_parent_function_sync((void *)mui_alert,3,CatalogStr(MSG_ERROR,MSG_ERROR_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),status_buffer_g);
		c->send_data("quit\r\n");
		delete c;
		delete [] line_buffer;
		delete statusWindow;
		delete msg;
		//return -1;
		return 1;
	}
	//authenticate
	if(!auth(c,server,statusWindow))
	{
		//sprintf(status_buffer_g,"\33cCannot Log Into Newsserver!\nError: %s",line_buffer);
		//MUI_RequestA(app,0,0,"Error!","_Okay",status_buffer_g,0);
		sprintf(line_buffer,"quit%s",newline);
		c->send_data(line_buffer);
		delete c;
		delete [] line_buffer;
		delete statusWindow;
		delete msg;
		//return -1;
		return 1;
	}
	sprintf(line_buffer,"DATE%s",newline);
	c->send_data(line_buffer);
	if( c->read_data(line_buffer) == -1 )
	{
		readFailedMessage(TRUE);
		delete c;
		delete [] line_buffer;
		delete statusWindow;
		delete msg;
		//return -1;
		return 1;
	}
	StripNewLine(line_buffer);
	char lastdate[256] = "";
	BOOL gotdate = FALSE;
	if(strncmp(line_buffer,"111",3)!=0)
	{
		nLog("Warning! - Cannot get date from newsserver!\n%s\n''Get New Newsgroups' won't work!\n",line_buffer);
	}
	else {
		word(lastdate,line_buffer,2);
		if(*lastdate != '\0')
		{
			gotdate = TRUE;
		}
		else
		{
			nLog("Warning! - Cannot get date from newsserver!\n%s\n''Get New Newsgroups' won't work!\n",line_buffer);
		}
	}

	sprintf(line_buffer,"LIST%s",newline);
	c->send_data(line_buffer);
	if( c->read_data_line(line_buffer) == -1 )
	{
		readFailedMessage(TRUE);
		delete c;
		delete [] line_buffer;
		delete statusWindow;
		delete msg;
		//return -1;
		return 1;
	}
	StripNewLine(line_buffer);
	if(*line_buffer!='2') {
		sprintf(status_buffer_g,CatalogStr(MSG_CANNOT_OBTAIN_GROUPLIST,MSG_CANNOT_OBTAIN_GROUPLIST_STR),line_buffer);
		//MUI_RequestA(app,0,0,CatalogStr(MSG_ERROR,MSG_ERROR_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),status_buffer_g,0);
		if( !thread_aborted() ) {
			c->send_data("quit\r\n");
			thread_call_parent_function_sync((void *)mui_alert,3,CatalogStr(MSG_ERROR,MSG_ERROR_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),status_buffer_g);
		}
		delete c;
		delete [] line_buffer;
		delete statusWindow;
		delete msg;
		//return -1;
		return 1;
	}
	char filename[300] = "";
	sprintf(filename,"NewsCoasterData:%s.gl",server->nntp);
	DeleteFile(filename);
	BPTR file = Open(filename,MODE_NEWFILE);
	delete_dis = TRUE;
	int found = 0;
	if(file)
	{
		int len = 0;
		char *big_buffer = new char[big_bufsize_g+1];
		do
		{
			len = c->read_data_chunk(&big_buffer[8],big_bufsize_g-9);
			// count groups
			char *str = big_buffer+7;
			while((str = strchr(str+1,'\n')))
				found++;

			Write(file,&big_buffer[8],len);
			sprintf(status_buffer_g,CatalogStr(MSG_FOUND_GROUPS,MSG_FOUND_GROUPS_STR),found);
			statusWindow->setText(status_buffer_g);
#ifdef DONT_USE_THREADS
			do_input();
			if(statusWindow->isAborted()==TRUE || running==FALSE)
				break;
#endif
			if( thread_aborted() )
				break;
			for(int l=3;l<8;l++)
				big_buffer[l] = big_buffer[big_bufsize_g-9+l];
		} while(len == big_bufsize_g-9);
		delete [] big_buffer;
		Close(file);
		file = NULL;

		// even if we've aborted at this point, we keep and display the groups we've found so far
		// todo: shouldn't we change this?
		if(gotdate) {
			strncpy(server->lastGotGroups,lastdate,14);
			server->lastGotGroups[14] = '\0';
			nLog("DATE: *%s*\n",server->lastGotGroups);
		}
		//msg->success = TRUE;
		thread_call_parent_function_async((void *)groups_downloaded,2,server,found);
		//nLog("!!!\n");
	}
	else {
			nLog("Warning! - Cannot open file %s!\n",filename);
	}

	delete_dis=FALSE;
	c->send_data("quit\r\n");
	delete c;
	delete [] line_buffer;
	delete statusWindow;
	delete msg;
	//return found;
	//msg->user_data = found;
	return 1;
}

/* Downloads a newsgroup listing from 'server'.
 */
void getgrouplist(Server *server) {
	nLog("getgrouplist((Server *)%d) called\n",server);

	ThreadData *msg = new ThreadData();
	msg->server = server;
	msg->status = new StatusWindow(app,CatalogStr(MSG_DOWNLOADING_GROUP_LIST,MSG_DOWNLOADING_GROUP_LIST_STR));
	msg->status->is_subthread = TRUE;
	//msg->success = FALSE;

	int thread = thread_start(THREAD_FUNCTION(&getgroups_entry),msg);
	nLog("getgrouplist((Server *)%d): Starting thread:\n",server,thread);
}

void display_groups(Server *server,int filepos)
{
	account.save_data();
	groupman_server = server;
	//readgrouplist(server,NLIST_groupman,filepos);
	readgrouplist(server,filepos);
	set(wnd_groupman,MUIA_Window_Open,TRUE);
}

static int getnewgroups_entry(struct ThreadData *msg)
{
	thread_parent_task_can_contiue();

	Server *server = msg->server;
	StatusWindow *statusWindow = msg->status;
	//msg->success = FALSE;

	char date[15]="";
	strncpy(date,server->lastGotGroups,14);
	date[14] = '\0';
	//printf(">>>Date: %s\n",date);

	closeSockets();
	Connection *c = new Connection();
	if(!c->init()) {
		//MUI_RequestA(app,0,0,CatalogStr(MSG_ERROR,MSG_ERROR_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_NO_TCP_STACK,MSG_NO_TCP_STACK_STR),0);
		if( !thread_aborted() )
			thread_call_parent_function_sync((void *)mui_alert,3,CatalogStr(MSG_ERROR,MSG_ERROR_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_NO_TCP_STACK,MSG_NO_TCP_STACK_STR));
		delete c;
		delete statusWindow;
		delete msg;
		//return -1;
		return 1;
	}

	//StatusWindow *statusWindow = new StatusWindow(app,CatalogStr(MSG_DOWNLOADING_NEW_GROUPS,MSG_DOWNLOADING_NEW_GROUPS_STR));
	sprintf(status_buffer_g,CatalogStr(MSG_CONNECTING_WITH_SERVER,MSG_CONNECTING_WITH_SERVER_STR),server->nntp);
	statusWindow->setText(status_buffer_g);
	if(!c->call_socket(server)) {
		//MUI_RequestA(app,0,0,CatalogStr(MSG_ERROR,MSG_ERROR_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_CANNOT_CONNECT_WITH_SERVER,MSG_CANNOT_CONNECT_WITH_SERVER_STR),0);
		if( !thread_aborted() )
			thread_call_parent_function_sync((void *)mui_alert,3,CatalogStr(MSG_ERROR,MSG_ERROR_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_CANNOT_CONNECT_WITH_SERVER,MSG_CANNOT_CONNECT_WITH_SERVER_STR));
		delete c;
		delete statusWindow;
		delete msg;
		//return FALSE;
		return 1;
	}
	char *line_buffer = new char[MAXLINE + 1];
	statusWindow->setText(CatalogStr(MSG_CONNECTED,MSG_CONNECTED_STR));
	if( c->read_data(line_buffer) == -1 )
	{
		readFailedMessage(TRUE);
		delete c;
		delete [] line_buffer;
		delete statusWindow;
		delete msg;
		//return FALSE;
		return 1;
	}
	StripNewLine(line_buffer);
	if(*line_buffer!='2')
	{
		sprintf(status_buffer_g,CatalogStr(MSG_CANNOT_USE_SERVER,MSG_CANNOT_USE_SERVER_STR),line_buffer);
		//MUI_RequestA(app,0,0,CatalogStr(MSG_ERROR,MSG_ERROR_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),status_buffer_g,0);
		if( !thread_aborted() ) {
			c->send_data("quit\r\n");
			thread_call_parent_function_sync((void *)mui_alert,3,CatalogStr(MSG_ERROR,MSG_ERROR_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),status_buffer_g);
		}
		delete c;
		delete [] line_buffer;
		delete statusWindow;
		delete msg;
		//return FALSE;
		return 1;
	}
	//authenticate
	if(!auth(c,server,statusWindow))
	{
		//sprintf(status_buffer_g,"\33cCannot Log Into Newsserver!\nError: %s",line_buffer);
		//MUI_RequestA(app,0,0,"Error!","_Okay",status_buffer_g,0);
		if( !thread_aborted() )
			c->send_data("quit\r\n");
		delete c;
		delete [] line_buffer;
		delete statusWindow;
		delete msg;
		//return FALSE;
		return 1;
	}
	char newdate[16]="";
	strncpy(newdate,&date[2],6);
	newdate[6]=' ';
	strncpy(&newdate[7],&date[8],6);
	newdate[13]=0;

	sprintf(line_buffer,"DATE%s",newline);
	c->send_data(line_buffer);
	if( c->read_data(line_buffer) == -1 )
	{
		readFailedMessage(TRUE);
		delete c;
		delete [] line_buffer;
		delete statusWindow;
		delete msg;
		//return FALSE;
		return 1;
	}
	StripNewLine(line_buffer);
	char lastdate[256] = "";
	BOOL gotdate = FALSE;
	if(strncmp(line_buffer,"111",3)!=0)
	{
		nLog("Warning! - Cannot get date from newsserver!\n%s\n''Get New Newsgroups' won't work!\n",line_buffer);
	}
	else {
		word(lastdate,line_buffer,2);
		if(*lastdate != '\0') {
			gotdate = TRUE;
		}
		else {
			nLog("Warning! - Cannot get date from newsserver!\n%s\n''Get New Newsgroups' won't work!\n",line_buffer);
		}
	}
	// todo: should really fail and exit here if gotdate is FALSE !

	sprintf(line_buffer,"NEWGROUPS %s%s",newdate,newline);
	c->send_data(line_buffer);
	if( c->read_data_line(line_buffer) == -1 )
	{
		readFailedMessage(TRUE);
		delete c;
		delete [] line_buffer;
		delete statusWindow;
		delete msg;
		//return FALSE;
		return 1;
	}
	StripNewLine(line_buffer);
	if(*line_buffer!='2')
	{
		sprintf(status_buffer_g,CatalogStr(MSG_CANNOT_OBTAIN_NEW_GROUPS_LIST,MSG_CANNOT_OBTAIN_NEW_GROUPS_LIST_STR),line_buffer);
		//MUI_RequestA(app,0,0,CatalogStr(MSG_ERROR,MSG_ERROR_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),status_buffer_g,0);
		if( !thread_aborted() )
		{
			c->send_data("quit\r\n");
			thread_call_parent_function_sync((void *)mui_alert,3,CatalogStr(MSG_ERROR,MSG_ERROR_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),status_buffer_g);
		}
		delete c;
		delete [] line_buffer;
		delete statusWindow;
		delete msg;
		//return FALSE;
		return 1;
	}
	char filename[300] = "";
	sprintf(filename,"NewsCoasterData:%s.gl",server->nntp);
	BPTR lock=Lock(filename,ACCESS_READ);
	BPTR file=Open(filename,MODE_OLDFILE);
	delete_dis=TRUE;
	if(file)
	{
		int found=0;
		ChangeFilePosition(file,0,OFFSET_END);
		int filepos = GetFilePosition(file);
		ChangeFilePosition(file,0,OFFSET_END);
		char *big_buffer = new char[big_bufsize_g + 1];
		strncpy(&big_buffer[6],newline,2);
		int len = 0;
		do
		{
#ifdef DONT_USE_THREADS
			DoMethod(app,MUIM_Application_InputBuffered);
#endif
			len = c->read_data_chunk(&big_buffer[8],big_bufsize_g-9);
			//printf("%d : *%s*\n",len,&big_buffer[8]);
			// count groups
			char *str = &big_buffer[7];
			while((str = strchr(str+1,'\n')))
				found++;
			Write(file,&big_buffer[8],len);
			sprintf(status_buffer_g,CatalogStr(MSG_FOUND_NEW_GROUPS,MSG_FOUND_NEW_GROUPS_STR),found);
			statusWindow->setText(status_buffer_g);
#ifdef DONT_USE_THREADS
			do_input();
			if(statusWindow->isAborted()==TRUE || running==FALSE)
				break;
#endif
			if( thread_aborted() )
				break;
			for(int l=3;l<8;l++)
				big_buffer[l] = big_buffer[big_bufsize_g-9+l];
		} while(len==big_bufsize_g-9);
		// even if we've aborted at this point, we keep and display the groups we've found so far
		// todo: shouldn't we change this? - if the user aborts, the user will no longer be able to get the full new groups, since the last download date will have been updated!
		if(found==0 && running)
		{
			sprintf(status_buffer_g,CatalogStr(MSG_FOUND_NEW_GROUPS_2,MSG_FOUND_NEW_GROUPS_2_STR),found);
			//MUI_RequestA(app,0,0,CatalogStr(MSG_GET_NEW_GROUPS,MSG_GET_NEW_GROUPS_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),status_buffer_g,0);
			thread_call_parent_function_sync((void *)mui_alert,3,CatalogStr(MSG_GET_NEW_GROUPS,MSG_GET_NEW_GROUPS_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),status_buffer_g);
		}
		Close(file);
		file = NULL;

		if(found > 0 && running)
		{
			// we only need to update the lastGotGroups date if some groups were found
			if(gotdate)
			{
				strncpy(server->lastGotGroups,lastdate,14);
				server->lastGotGroups[14] = '\0';
				nLog("DATE: *%s*\n",server->lastGotGroups);
				//printf("DATE: *%s*\n",server->lastGotGroups);
			}
			//thread_call_parent_function_sync(display_groups,2,server,filepos);
			thread_call_parent_function_async((void *)display_groups,2,server,filepos);
			/*account.save_data();
			groupman_server = server;
			//readgrouplist(server,NLIST_groupman,filepos);
			readgrouplist(server,filepos);
			set(wnd_groupman,MUIA_Window_Open,TRUE);*/
		}
		//msg->success = TRUE;
	}
	else
	{
		//thread_call_parent_function_sync(printf,2,"Warning! - Cannot open file %s!\n",filename);
		thread_call_parent_function_async((void *)printf,2,"Warning! - Cannot open file %s!\n",filename);
		nLog("Warning! - Cannot open file %s!\n",filename);
		// todo: should tell user!
	}
	delete_dis=FALSE;
	if(lock) {
		UnLock(lock);
		lock=NULL;
	}
	c->send_data("quit\r\n");
	delete c;
	delete [] line_buffer;
	delete statusWindow;
	delete msg;
	//return TRUE;
	return 1;
}

/* Download new newsgroups from 'server'.
 */
void getnewgroups(Server *server) {
	nLog("getnewgroups((Server *)%d) called\n",server);

	// check file
	char filename[300] = "";
	sprintf(filename,"NewsCoasterData:%s.gl",server->nntp);
	if( !exists(filename) ) {
		sprintf(status_buffer_g,CatalogStr(MSG_DATE_OF_LAST_GROUPLIST_DOWNLOAD_NOT_AVAILABLE,MSG_DATE_OF_LAST_GROUPLIST_DOWNLOAD_NOT_AVAILABLE_STR));
		mui_alert(CatalogStr(MSG_ERROR,MSG_ERROR_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),status_buffer_g);
		return;
	}

	// check date
	char date[15]="";
	strncpy(date,server->lastGotGroups,14);
	date[14] = '\0';
	nLog("Date *%s*\n",date);
	BOOL okay = TRUE;
	if(strlen(date)!=14)
		okay=FALSE;
	else {
		for(int k=0;k<14;k++) {
			if(isdigit(date[k])==FALSE) {
				okay=FALSE;
				break;
			}
		}
	}
	if(!okay) {
		sprintf(status_buffer_g,CatalogStr(MSG_DATE_OF_LAST_GROUPLIST_DOWNLOAD_NOT_AVAILABLE,MSG_DATE_OF_LAST_GROUPLIST_DOWNLOAD_NOT_AVAILABLE_STR));
		//MUI_RequestA(app,0,0,CatalogStr(MSG_ERROR,MSG_ERROR_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),status_buffer_g,0);
		//thread_call_parent_function_sync(mui_alert,3,CatalogStr(MSG_ERROR,MSG_ERROR_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),status_buffer_g);
		mui_alert(CatalogStr(MSG_ERROR,MSG_ERROR_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),status_buffer_g);
		return;
	}

	ThreadData *msg = new ThreadData();
	msg->server = server;
	msg->status = new StatusWindow(app,CatalogStr(MSG_DOWNLOADING_NEW_GROUPS,MSG_DOWNLOADING_NEW_GROUPS_STR));
	msg->status->is_subthread = TRUE;
	//msg->success = FALSE;

	//getnewgroups_entry(&msg);
	int thread = thread_start(THREAD_FUNCTION(&getnewgroups_entry),msg);

	//delete msg.status;

	//return msg.success;
}

/* Log into the newsserver.
 */
BOOL logIn(Connection *c,Server * server,StatusWindow * sw) {
	nLog("logIn((Connection *)%d,(Server *)%d,(StatusWindow *)%d) called\n",c,server,sw);
	//char buffer[MAXLINE+1]="";
	//char temp[1024]="";
	if(!c->call_socket(server)) {
		MUI_RequestA(app,0,0,CatalogStr(MSG_ERROR,MSG_ERROR_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_CANNOT_CONNECT_WITH_SERVER,MSG_CANNOT_CONNECT_WITH_SERVER_STR),0);
		return FALSE;
	}
	do_input();
	if(sw->isAborted()==TRUE || running==FALSE) {
		c->close(); // exit immediately
		return FALSE;
	}

	char *line_buffer = new char[MAXLINE+1];
	if( c->read_data(line_buffer) == -1 ) {
		readFailedMessage();
		c->close(); // exit immediately
		delete [] line_buffer;
		return FALSE;
	}
	StripNewLine(line_buffer);
	if(*line_buffer!='2') {
		sprintf(status_buffer_g,CatalogStr(MSG_CANNOT_USE_SERVER,MSG_CANNOT_USE_SERVER_STR),line_buffer);
		MUI_RequestA(app,0,0,CatalogStr(MSG_ERROR,MSG_ERROR_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),status_buffer_g,0);
		hangUp(c);
		delete [] line_buffer;
		return FALSE;
	}
	do_input();
	if(sw->isAborted()==TRUE || running==FALSE) {
		c->close(); // exit immediately
		delete [] line_buffer;
		return FALSE;
	}

	//authenticate
	if(!auth(c,server,sw)) {
		//sprintf(status_buffer_g,"\33cCannot Log Into Newsserver!\nError: %s",line_buffer);
		//MUI_RequestA(app,0,0,"Error!","_Okay",status_buffer_g,0);
		hangUp(c);
		delete [] line_buffer;
		return FALSE;
	}

	//mode reader
	sprintf(line_buffer,"MODE READER%s",newline);
	c->send_data(line_buffer);
	if( c->read_data(line_buffer) == -1 ) {
		readFailedMessage();
		c->close(); // exit immediately
		delete [] line_buffer;
		return FALSE;
	}
	StripNewLine(line_buffer);
	if(*line_buffer!='2' && *line_buffer!='5') {
		nLog("Warning: %s received from 'mode reader' command\n",line_buffer);
	}
	do_input();
	if(sw->isAborted()==TRUE || running==FALSE) {
		c->close(); // exit immediately
		delete [] line_buffer;
		return FALSE;
	}

	delete [] line_buffer;
	return TRUE;
}

/* Post outgoing messages.
 * TODO: this doesn't really cope well with messages that are to be both
 * posted and emailed, if one of those fails; at the moment in such cases,
 * the message remains in outgoing, and will be both posted/emailed next
 * time (ie, even though one of those has already been done).
 */
void postnews(int type)
{
	// type: 0=normal, 1=post online (immediate) only (flags[2]==TRUE)
	nLog("postnews((int)%d) called\n",type);
	char *line_buffer = new char[MAXLINE+1];
	Server *current_server = NULL;
	BPTR fileout=NULL;
	GroupData * gdata=NULL;
	GroupData * gdata_sent=NULL;
	BOOL newsup=FALSE,emailup=FALSE;
	ULONG entries = 0;
	char filename[300] = "";

	closeSockets();
	Connection *c = new Connection();
	if(!c->init())
	{
		MUI_RequestA(app,0,0,CatalogStr(MSG_ERROR,MSG_ERROR_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_NO_TCP_STACK,MSG_NO_TCP_STACK_STR),0);
		delete c;
		return;
	}
	get_gdata(&gdata,-1); 			//Outgoing
	get_gdata(&gdata_sent,-2); 		//Sent
	setGdataQuiet(TRUE);
	setMdataQuiet(TRUE);
	//pre-scan
	Vector * vector = NULL;
	GroupData *curr_gdata = NULL;
	getGdataDisplayed(&curr_gdata);
	// read in ID history
	if(gdata->ID != curr_gdata->ID)
	{
		vector = new Vector(1024);
		BOOL res = read_index(gdata,vector);
		entries = vector->getSize();
	}
	else
		get(NLIST_messagelistdata,MUIA_NList_Entries,&entries);

	BOOL * news_sent = new BOOL[entries];
	BOOL * email_sent = new BOOL[entries];

	for(ULONG k=0;k<entries;k++)
	{
		MessageListData * mdata = NULL;
		//DoMethod(NLIST_messagelistdata,MUIM_NList_GetEntry,k,&mdata);
		if(vector!=0)
			mdata=((MessageListData **)vector->getData())[k];
		else
			DoMethod(NLIST_messagelistdata,MUIM_NList_GetEntry,k,&mdata);
		mdata->flags[3] = mdata->flags[2];
		mdata->flags[2] = FALSE; // need to unset post-immediately flag in case we fail too early!
		if( (mdata->flags[0]==FALSE && type==0) || (mdata->flags[3]==TRUE && type==1) )
		{
			if(mdata->flags[4]==TRUE)
				newsup=TRUE;
			if(mdata->flags[5]==TRUE)
				emailup=TRUE;
		}
		news_sent[k]=FALSE;
		email_sent[k]=FALSE;
	}

	if(newsup)
	{
		//BOOL cont = TRUE;
		// open a socket to newsserver
		StatusWindow *statusWindow = new StatusWindow(app,CatalogStr(MSG_SENDING_NEWS,MSG_SENDING_NEWS_STR));
		//post messages
		delete_dis=TRUE;
		//if(cont)
		{
			for(ULONG k=0;k<entries;k++)
			{
				MessageListData * mdata = NULL;
				//DoMethod(NLIST_messagelistdata,MUIM_NList_GetEntry,k,&mdata);
				if(vector!=0)
					mdata=((MessageListData **)vector->getData())[k];
				else
					DoMethod(NLIST_messagelistdata,MUIM_NList_GetEntry,k,&mdata);
				BOOL post=FALSE;
				if(type==0 && mdata->flags[0]==FALSE)
					post=TRUE;
				else if(type==1 && mdata->flags[0]==FALSE && mdata->flags[3]==TRUE)
					post=TRUE;
				if(mdata->flags[4]==FALSE)
					post=FALSE;
				if(post)
				{
					// try to fathom out which newsserver to use
					//printf("***%s\n",mdata->newsgroups);
					char ngroups[1024] = "";
					const char *delim = ",";
					//sprintf(ngroups,"%s ",mdata->newsgroups);
					get_newsgroups(ngroups,gdata->ID,mdata->ID);
					char *grp = strtok(ngroups,delim);
					/*while(grp != 0) {
						printf("group: %s\n",grp);
						Server *server = getPostingServerForGroup(grp);
						printf("server: %s\n",server->nntp);
						grp = strtok(NULL,delim);
					}*/
					//printf("group: %s\n",grp);
					Server *this_server = getPostingServerForGroup(grp);
					//printf("server: %s\n",this_server->nntp);

					if(this_server != current_server)
					{
						// change servers
						//printf("changing servers..\n");
						if(current_server != 0)
						{
							// disconnect
							if(c->connected())
								hangUp(c);
						}
						// reconnect
						current_server = this_server;
						sprintf(status_buffer_g,CatalogStr(MSG_CONNECTING_WITH_SERVER,MSG_CONNECTING_WITH_SERVER_STR),current_server->nntp);
						statusWindow->setText(status_buffer_g);

						if(!logIn(c,current_server,statusWindow))
						{
							current_server = NULL;
							continue;
						}
					}

					sprintf(line_buffer,CatalogStr(MSG_SENDING,MSG_SENDING_STR),mdata->subject);
					statusWindow->setText(line_buffer);

					//continue;
					sprintf(filename,"NewsCoasterData:outgoing/news_%d",mdata->ID);
					fileout=Open(filename,MODE_OLDFILE);
					if(fileout)
					{
						sprintf(line_buffer,"POST%s",newline);
						c->send_data(line_buffer);
						if( c->read_data(line_buffer) == -1 )
						{
							readFailedMessage();
							Close(fileout);
							fileout=NULL;
							break;
						}
						StripNewLine(line_buffer);
						if(*line_buffer!='3' && *line_buffer!='2' && *line_buffer!='4')
						{
							MUI_RequestA(app,0,0,CatalogStr(MSG_ERROR,MSG_ERROR_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_CANNOT_POST_TO_SERVER,MSG_CANNOT_POST_TO_SERVER_STR),0);
							Close(fileout);
							fileout=NULL;
							//cont=FALSE;
							break;
						}
						else
						{
							char *eof=(char*)FGets(fileout,line_buffer,MAXLINE);
							while(eof)
							{
								StripNewLine(line_buffer);
								do_input(); // input loop
								if(statusWindow->isAborted()==TRUE || running==FALSE)
									break;
								sprintf(line_buffer,"%s%s",line_buffer,newline);
								if(*line_buffer=='.')
									c->send_data(".");
								c->send_data(line_buffer);
								eof=(char*)FGets(fileout,line_buffer,MAXLINE);
							}
							Close(fileout);
							fileout=NULL;
							if(statusWindow->isAborted()==FALSE && running==TRUE)
							{
								sprintf(line_buffer,"%s.%s",newline,newline);
								c->send_data(line_buffer);
								if( c->read_data(line_buffer) == -1 )
								{
									readFailedMessage();
								}
								else
								{
									StripNewLine(line_buffer);
									if(*line_buffer!='2')
									{
										sprintf(status_buffer_g,CatalogStr(MSG_ERROR_2,MSG_ERROR_2_STR),line_buffer);
										MUI_RequestA(app,0,0,CatalogStr(MSG_POST_FAILED,MSG_POST_FAILED_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),status_buffer_g,0);
									}
									else
									{
										statusWindow->setText(CatalogStr(MSG_DONE,MSG_DONE_STR));
										news_sent[k]=TRUE;
									}
								}
							}
							else
								statusWindow->setText(CatalogStr(MSG_ABORTED,MSG_ABORTED_STR));
						}
					}
					else
					{
						printf("Error! Cannot open file: %s\n",filename);
						sprintf(status_buffer_g,CatalogStr(MSG_CANNOT_OPEN_FILE,MSG_CANNOT_OPEN_FILE_STR),filename);
						statusWindow->setText(status_buffer_g);
					}
				}
				if(statusWindow->isAborted()==TRUE || running==FALSE)
					break;
			}
		}
		if(c->connected())
			hangUp(c);
		delete statusWindow;
	}

	if(emailup)
	{
		BOOL cont = TRUE;
		// open a socket to smtp server
		StatusWindow *statusWindow = new StatusWindow(app,CatalogStr(MSG_SENDING_EMAIL,MSG_SENDING_EMAIL_STR));
		sprintf(status_buffer_g,CatalogStr(MSG_CONNECTING_WITH_SERVER,MSG_CONNECTING_WITH_SERVER_STR),account.smtp);
		statusWindow->setText(status_buffer_g);
		if(c->connected())
		{
			c->send_data("quit\r\n");
			c->close();
		}
		if(!c->call_socket(account.smtp,25))
		{
			MUI_RequestA(app,0,0,CatalogStr(MSG_ERROR,MSG_ERROR_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_CANNOT_CONNECT_WITH_SMTP,MSG_CANNOT_CONNECT_WITH_SMTP_STR),0);
			cont=FALSE;
		}
		if(cont) {
			statusWindow->setText(CatalogStr(MSG_CONNECTED,MSG_CONNECTED_STR));
			if( c->read_data(line_buffer) == -1 )
			{
				readFailedMessage();
				cont=FALSE;
			}
			else
			{
				nLog("  received: %s\n",line_buffer);
				StripNewLine(line_buffer);
				if(*line_buffer!='2')
				{
					sprintf(status_buffer_g,CatalogStr(MSG_CANNOT_USE_SMTP,MSG_CANNOT_USE_SMTP_STR),line_buffer);
					MUI_RequestA(app,0,0,CatalogStr(MSG_ERROR,MSG_ERROR_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),status_buffer_g,0);
					cont=FALSE;
				}
			}
		}
		if(cont)
		{
			char *remail = ( *account.realemail != '\0' ) ? account.realemail : account.email;
			char *ptr = strchr(remail,'@');
			if(ptr != 0)
				remail = &ptr[1];
			sprintf(line_buffer,"HELO %s%s",remail,newline);
			nLog("  sending: %s",line_buffer);
			c->send_data(line_buffer);
			c->read_data(line_buffer);
			nLog("  received: %s\n",line_buffer);
			StripNewLine(line_buffer);
			if(*line_buffer != '2') {
				sprintf(line_buffer,"HELO %s",remail);
				char *ptr = strchr(line_buffer,'@');
				if(ptr != 0) {
					// try again
					strcpy(ptr,newline);
					nLog("  sending: %s",line_buffer);
					c->send_data(line_buffer);
					c->read_data(line_buffer);
					nLog("  received: %s\n",line_buffer);
					StripNewLine(line_buffer);
				}
				if(*line_buffer != '2')
				{
					// try yet again
					sprintf(line_buffer,"HELO%s",newline);
					nLog("  sending: %s",line_buffer);
					c->send_data(line_buffer);
					c->read_data(line_buffer);
					nLog("  received: %s\n",line_buffer);
					StripNewLine(line_buffer);
				}
				if(*line_buffer != '2')
				{
					// failed
					sprintf(status_buffer_g,CatalogStr(MSG_CANNOT_SEND_TO_SMTP,MSG_CANNOT_SEND_TO_SMTP_STR),line_buffer);
					MUI_RequestA(app,0,0,CatalogStr(MSG_ERROR,MSG_ERROR_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),status_buffer_g,0);
					cont=FALSE;
				}
			}
			for(ULONG k=0;cont && k<entries;k++)
			{
				MessageListData * mdata = NULL;
				//DoMethod(NLIST_messagelistdata,MUIM_NList_GetEntry,k,&mdata);
				if(vector!=0)
					mdata=((MessageListData **)vector->getData())[k];
				else
					DoMethod(NLIST_messagelistdata,MUIM_NList_GetEntry,k,&mdata);
				BOOL post = FALSE;
				if(type==0 && mdata->flags[0]==FALSE)
					post = TRUE;
				else if(type==1 && mdata->flags[0]==FALSE && mdata->flags[3]==TRUE)
					post = TRUE;
				if(mdata->flags[5]==FALSE)
					post = FALSE;
				if(post) {
					sprintf(line_buffer,CatalogStr(MSG_SENDING,MSG_SENDING_STR),mdata->subject);
					statusWindow->setText(line_buffer);
					sprintf(filename,"NewsCoasterData:outgoing/news_%d",mdata->ID);
					fileout=Open(filename,MODE_OLDFILE);
					if(fileout) {
						NewMessage newmessage;
						strcpy(newmessage.getThisHeader,"to:");
						get_refs(&newmessage,gdata,mdata,GETREFS_NONE);
						if(*account.realemail!=0)
							sprintf(line_buffer,"MAIL FROM: <%s>%s",account.realemail,newline);
						else
							sprintf(line_buffer,"MAIL FROM: <%s>%s",account.email,newline);
						nLog("  sending: %s",line_buffer);
						c->send_data(line_buffer);
						c->read_data(line_buffer);
						nLog("  received: %s\n",line_buffer);
						StripNewLine(line_buffer);
						if(*line_buffer!='2') {
							sprintf(status_buffer_g,CatalogStr(MSG_CANNOT_SEND_TO_SMTP,MSG_CANNOT_SEND_TO_SMTP_STR),line_buffer);
							MUI_RequestA(app,0,0,CatalogStr(MSG_ERROR,MSG_ERROR_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),status_buffer_g,0);
							Close(fileout);
							fileout=NULL;
							cont=FALSE;
							break;
						}
						char buffer2[256] = "";
						int rcpt_no=1;
						while(cont) {
							char rcpt[256] = "";
							//word(rcpt,newmessage.to,rcpt_no,',');
							word(rcpt,newmessage.dummyHeader,rcpt_no,',');
							if(0 == *rcpt)
								break;
							get_email(buffer2,rcpt,GETEMAIL_EMAIL);
							sprintf(line_buffer,"RCPT TO: <%s>%s",buffer2,newline);
							nLog("  sending: %s",line_buffer);
							c->send_data(line_buffer);
							c->read_data(line_buffer);
							nLog("  received: %s\n",line_buffer);
							StripNewLine(line_buffer);
							if(*line_buffer!='2') {
								sprintf(status_buffer_g,CatalogStr(MSG_ERROR_FOR_RECIPIENT,MSG_ERROR_FOR_RECIPIENT_STR),rcpt,line_buffer);
								MUI_RequestA(app,0,0,CatalogStr(MSG_ERROR,MSG_ERROR_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),status_buffer_g,0);
								Close(fileout);
								fileout=NULL;
								cont=FALSE;
								break;
							}
							rcpt_no++;
						}
						if(cont) {
							sprintf(line_buffer,"DATA%s",newline);
							c->send_data(line_buffer);
							c->read_data(line_buffer);
							StripNewLine(line_buffer);
							char *eof=(char*)FGets(fileout,line_buffer,MAXLINE);
							while(eof) {
								StripNewLine(line_buffer);
								do_input(); // input loop
								if(statusWindow->isAborted()==TRUE || running==FALSE)
									break;
								sprintf(line_buffer,"%s%s",line_buffer,newline);
								if(*line_buffer=='.')
									c->send_data(".");
								c->send_data(line_buffer);
								eof=(char*)FGets(fileout,line_buffer,MAXLINE);
							}
							Close(fileout);
							fileout=NULL;
							if(statusWindow->isAborted()==FALSE && running==TRUE) {
								sprintf(line_buffer,"%s.%s",newline,newline);
								c->send_data(line_buffer);
								c->read_data(line_buffer);
								StripNewLine(line_buffer);
								if(*line_buffer!='2') {
									nLog("Email Failed: %s\n",line_buffer);
									sprintf(status_buffer_g,CatalogStr(MSG_EMAIL_FAILED,MSG_EMAIL_FAILED_STR),line_buffer);
									statusWindow->setText(status_buffer_g);
								}
								else {
									statusWindow->setText(CatalogStr(MSG_DONE,MSG_DONE_STR));
									email_sent[k]=TRUE;
								}
							}
							else {
								statusWindow->setText(CatalogStr(MSG_ABORTED,MSG_ABORTED_STR));
							}
						}
					}
					else {
						sprintf(status_buffer_g,CatalogStr(MSG_CANNOT_OPEN_FILE,MSG_CANNOT_OPEN_FILE_STR),filename);
						statusWindow->setText(status_buffer_g);
					}
				}
				if(statusWindow->isAborted()==TRUE || running==FALSE)
					break;
			}
		}
		c->send_data("quit\r\n");
		c->close();
		delete statusWindow;
	}

	// copy sent messages to .sent
	if(entries>0)
	{
		MessageListData ** mdata_sent = new MessageListData *[entries];
		int sent=0;
		for(ULONG k=0;k<entries;k++) {
			MessageListData *mdata = NULL;
			//DoMethod(NLIST_messagelistdata,MUIM_NList_GetEntry,k,&mdata);
			if(vector!=0)
				mdata=((MessageListData **)vector->getData())[k];
			else
				DoMethod(NLIST_messagelistdata,MUIM_NList_GetEntry,k,&mdata);
			if(mdata->flags[4]==TRUE || mdata->flags[5]==TRUE) {
				// only move to .sent if it has both posted/emailed as required
				if( (news_sent[k]==TRUE || mdata->flags[4]==FALSE) && (email_sent[k]==TRUE || mdata->flags[5]==FALSE) )
					mdata_sent[sent++] = mdata;
				// if one of emailing/posting was successful, turn off so as to not resend
				if(news_sent[k]==TRUE && mdata->flags[4]==TRUE)
					mdata->flags[4]=FALSE;
				if(email_sent[k]==TRUE && mdata->flags[5]==TRUE)
					mdata->flags[5]=FALSE;
			}
		}
		if(sent>0) {
			StatusWindow *statusWindow = new StatusWindow(app,CatalogStr(MSG_MOVING_TO_SENT,MSG_MOVING_TO_SENT_STR));
			set(wnd_main,MUIA_Window_Sleep,TRUE);
			WriteWindow::sleepAll(TRUE);
			move_n(gdata,gdata_sent,mdata_sent,sent,0,statusWindow,FALSE);
			set(wnd_main,MUIA_Window_Sleep,FALSE);
			WriteWindow::sleepAll(FALSE);
			delete statusWindow;
		}
		if(mdata_sent) {
			delete [] mdata_sent;
			mdata_sent=NULL;
		}
	}
	delete_dis=FALSE;
	setGdataQuiet(FALSE);
	setMdataQuiet(FALSE);
	if( vector != 0 )
		delete vector;
	if(news_sent) {
		delete [] news_sent;
		news_sent=NULL;
	}
	if(email_sent) {
		delete [] email_sent;
		email_sent=NULL;
	}
	delete c;
	delete [] line_buffer;
	return;
}

/* Being lazy and using some globals.. */
int        histlen                = 0;
char     **mIDhist                = NULL;
ULONG      nkills                 = 0;
KillFile **killfiles              = NULL;
BOOL      *replace_killfile       = NULL;
BOOL      *killfile_in_this_group = NULL;
int       *killfile_types         = NULL;
BOOL       on_refs                = FALSE;
char       isobuf[4096]           = "";

int killArticle(int article,int killindx,BOOL *important,StatusWindow *statusWindow) {
	BOOL skip = FALSE;
	KillFile *kill = killfiles[killindx];

	*important = FALSE;

											nLog("*** Message Killfile match! - %s %s %s (%d, %d)\n",kill->header,kill->text,kill->ngroups,article,killindx);
											nLog("    match = %d ; action = %d\n",kill->match,kill->action);
											if(kill->action==0) {
												skip = TRUE;
												sprintf(status_buffer_g,CatalogStr(MSG_MESSAGE_KILLED,MSG_MESSAGE_KILLED_STR),article,kill->header);
												statusWindow->setText(status_buffer_g);
											}
											else if(kill->action==1)
												*important = TRUE;

											if(!replace_killfile[killindx]) {
												KillFile * newkill = new KillFile();
												strncpy(newkill->header,kill->header,63);
												strncpy(newkill->text,kill->text,255);
												strncpy(newkill->ngroups,kill->ngroups,63);
												newkill->header[63] = '\0';
												newkill->text[255] = '\0';
												newkill->ngroups[63] = '\0';
												newkill->ds = kill->ds;
												// lastused is at current datetime now - see KillFile constructor
												newkill->expiretype = kill->expiretype;
												newkill->expire = kill->expire;
												newkill->match = kill->match;
												newkill->action = kill->action;
												newkill->carryon = kill->carryon;
												nLog("  about to replace killfile..\n");
												// copy it back
												*kill = *newkill;
												replace_killfile[killindx] = TRUE;
												delete newkill;
											}
											nLog("  done\n");
	return skip;
}

/* Process the line of header in 'buffer'.
 */
int processHeader(char * buffer,GroupData * gdata,MessageListData * mdata,char * ngroup,StatusWindow *statusWindow,int article) {
	nLog("processHeader((char *)%s,...,(MessageListData *)%d,...,(int)%d) called\n",buffer,mdata,article);
	int skip = FALSE;

	char *ptr = strstr(buffer,": ");
	const int MAXWORD = 512;
	char word1[MAXWORD + 1] = "";
	if( ptr != 0 && (int)(ptr-buffer) <= MAXWORD )
	{
		int w1len = wordFirstAndLenUpper(word1,buffer);
		//killfile
		for(ULONG l=0;l<nkills;l++)
		{
			KillFile *kill = killfiles[l];

			if(*kill->header=='\0' || equals(word1,kill->header))
			{
				//matched header
				char *match = stristr_q(&buffer[w1len+1],kill->text);
				if( (kill->match==0 && match!= 0) || (kill->match==1 && match==0) )
				{
					//matched text
					if(killfile_in_this_group[l])
					{
						//matched group
						// kill it!
						BOOL important = FALSE;
						if(killArticle(article,l,&important,statusWindow))
							skip = TRUE;

						if(important)
							mdata->flags[13] = 1;

						if(!kill->carryon) {
							nLog("skip rest\n");
							break;
						}
					}
				}
			}
		}
	}

	if(!skip)
	{
		// cope with multiple line references
		//printf("line: %s\n",buffer);
		if(on_refs) {
			if(strchr(word1,':') != 0)
				on_refs = FALSE;
			else {
				char *lastref = NULL;
				char *ptr = buffer;
				for(;;) {
					ptr = strchr(ptr,'<');
					if(ptr == NULL)
						break;
					lastref = ptr;
					ptr++;
				}
				if(lastref != 0) {
					//printf("lastref: %s\n",lastref);
					int i=0;
					while(*lastref != 0) {
						mdata->lastref[i] = *lastref;
						i++;
						if(*lastref=='>' || i==IHEADENDMID)
							break;
						lastref++;
					}
					mdata->lastref[i] = '\0';
					//printf(">>>%s\n",mdata->lastref);
				}
			}
		}

		if(strlen(word1) == strlen(buffer)) {
			// empty header
		}
		else if( *word1 == '\0' ) {
			// not a new header
		}
		else if(strcmp(word1,"NEWSGROUPS:")==0)
		{
			//strncpy(mdata->from,&buffer[6],IHEADENDSHORT);
			translateIso(isobuf,&buffer[6]);
			strncpy(mdata->newsgroups,isobuf,IHEADENDSHORT);
			mdata->newsgroups[IHEADENDSHORT]=0;
			//stripEscapes(mdata->from);
		}
		else if(strcmp(word1,"FROM:")==0)
		{
			//strncpy(mdata->from,&buffer[6],IHEADENDSHORT);
			translateIso(isobuf,&buffer[6]);
			strncpy(mdata->from,isobuf,IHEADENDSHORT);
			mdata->from[IHEADENDSHORT]=0;
			//stripEscapes(mdata->from);
		}
		else if(strcmp(word1,"SUBJECT:")==0)
		{
			//strncpy(mdata->subject,&buffer[9],IHEADENDSUBJECT);
			translateIso(isobuf,&buffer[9]);
			strncpy(mdata->subject,isobuf,IHEADENDSUBJECT);
			mdata->subject[IHEADENDSUBJECT]= '\0';
			//stripEscapes(mdata->subject);
			if(account.use_sizekill) {
				char *word2=wordLast(mdata->subject);
				if(word2) {
					if(*word2!=0) {
						BOOL okay=TRUE;
						int w2len = strlen(word2);
						for(int l=0;l<w2len;l++) {
							if(!isdigit((int)word2[l])) {
								okay=FALSE;
								break;
							}
						}
						if(okay) {
							if(strlen(word2)>=3) { // if number is 3 digits or more
								nLog("*** Message Killed! - 'Advert' %s (%d)\n",word2,article);
								sprintf(status_buffer_g,CatalogStr(MSG_MESSAGE_KILLED_ADVERT,MSG_MESSAGE_KILLED_ADVERT_STR),article);
								statusWindow->setText(status_buffer_g);
								skip=TRUE;
							}
						}
						word2=NULL;
					}
				}
			}
		}
		else if(strcmp(word1,"DATE:")==0)
		{
			*mdata->datec = '\0';
			//strncpy(mdata->date,&buffer[6],IHEADENDSHORT);
			//mdata->date[IHEADENDSHORT]=0;
			//DateHandler::read_date(&mdata->ds,mdata->date,mdata->c_date,mdata->c_time);
			DateHandler::read_date(&mdata->ds,&buffer[6],mdata->c_date,mdata->c_time);
		}
		else if(strcmp(word1,"MESSAGE-ID:")==0)
		{
			strncpy(mdata->messageID,&buffer[12],IHEADENDMID);
			mdata->messageID[IHEADENDMID]=0;
			//stripEscapes(mdata->messageID);
			char **hist = mIDhist;
			for(int l=0;l<histlen;l++) {
				if(strcmp(*hist++,mdata->messageID)==0) {
					// already downloaded!
					nLog("*** Duplicate Message! - %s (%d, %d)\n",mdata->messageID,article,l);
					sprintf(status_buffer_g,CatalogStr(MSG_MESSAGE_KILLED_DUPLICATE,MSG_MESSAGE_KILLED_DUPLICATE_STR),article);
					statusWindow->setText(status_buffer_g);
					skip=TRUE;
					break;
				}
			}
		}
		else if(strcmp(word1,"REFERENCES:")==0)
		{
			char *lastref = wordLast(buffer);
			strncpy(mdata->lastref,lastref,IHEADENDMID);
			mdata->lastref[IHEADENDMID]=0;
			on_refs = TRUE;
		}
		else if(strcmp(word1,"LINES:")==0)
		{
			//char word2[256] = "";
			//wordFirst(word2,&buffer[w1len+1]);
			//if(*word2 != '\0')
			{
				//int tline = atoi(word2);
				//int tline = atoi(&buffer[w1len+1]);
				int tline = atoi(&buffer[7]);
				mdata->flags[8] = tline;
				// 'lines' filter
				if(gdata->flags[2]>0)
				{
					if(tline>gdata->flags[2])
					{
						nLog("*** Message Killed! - %d longer than %d lines (%d)\n",tline,gdata->flags[2],article);
						sprintf(status_buffer_g,CatalogStr(MSG_MESSAGE_KILLED_TOO_MANY_LINES,MSG_MESSAGE_KILLED_TOO_MANY_LINES_STR),article);
						statusWindow->setText(status_buffer_g);
						skip=TRUE;
					}
				}
			}
		}
	}
	// no more checks; they must go before LINES
	// no call to doOnlineBlocking() !
	//nLog("  returning %d\n",skip);
	return skip;
}

/* Downloads the a message; either the entire message, or just the body.
 */
BOOL download_body(BOOL *available,Connection *c,BPTR file,int *bread,int *bout,StatusWindow *statusWindow,BOOL entire_message,char *buffer) {
	nLog("download_body() called\n");
	BOOL in_header = entire_message;
	//char buffer[MAXLINE+1] = "";
	//char *buffer = new char[MAXLINE+1];
	char *bufptr = NULL;
	BOOL done = FALSE;
	BOOL res = TRUE;
	int br = 0;
	int bm = 0;
	int len = c->read_data(buffer,MAXLINE);
	if(len > 0) {
		br += len;
		if(*buffer!='2') {
			*available = FALSE;
			nLog("not available!\n");
			res = FALSE;
		}
		else {
   	   bufptr = strstr(buffer,"\r\n") + 2;
	   	bm += (int)bufptr - (int)buffer;
	   }
		if(res) {
			int l = 0;
			char line[MAXLINE+1] = "";
			//char *line = new char[MAXLINE+1];
			for(;;) {
				do_input();
				if(statusWindow->isAborted()==TRUE || running==FALSE) {
					res = FALSE;
					break;
				}
				while( bufptr < &buffer[len] ) {
					if (*bufptr != '\r')
						line[l++] = *bufptr;
					else
						bm++;
					if(l==MAXLINE-1) {
						line[l] = '\0';
						l = 0;
						if(FPuts(file,line) == -1) {
							res = FALSE;
							break;
						}
					}
					if(*bufptr++ != '\n')
						continue;
					line[l] = '\0';
					l = 0;
					if(line[0] == '.') {
						if(line[1] == '\n') {
							bm += 2;
							done = TRUE;
							break;
						}
						else {
							l = 1;
							bm++;
						}
					}
					if(in_header) {
						if(line[0] == '\n' && line[1] == '\0')
							in_header=  FALSE;
						else {
							stripEscapes(line);
						}
					}
					if(FPuts(file,&line[l]) == -1) {
						res = FALSE;
						break;
					}
					l = 0;
				}
				if(done)
					break;
				len = c->read_data(buffer,MAXLINE);
				if(len<=0)
					break;
				br += len;
				bufptr = buffer;
			}
			//delete [] line;
		}
	}
	else {
		nLog("no response from server!\n");
		res = FALSE;
		if( len == 0 )
			readFailedMessage();
	}
	//delete [] buffer;
	FFlush(file);
	*bread = br;
	*bout = br - bm;
	nLog("returning %d\n",res);
	return res;
}

void sortGroupList(GroupData **gdata_list,int entries) {
	for(int dg=0;dg<entries-2;dg++) {
		Server *this_server = getServer( gdata_list[dg]->serverID );
		for(int j=dg+1;j<entries;j++) {
			Server *server = getServer( gdata_list[j]->serverID );
			if(server == this_server) {
				// swap with position dg+1
				if(j != dg+1) {
					GroupData *temp_gdata = gdata_list[j];
					gdata_list[j] = gdata_list[dg+1];
					gdata_list[dg+1] = temp_gdata;
				}
				break;
			}
		}
	}
}

/* Fetch news. If 'thisgroup' equals -1, then all (subscribed) groups will
 * be checked for new messages. Otherwise, it specifies the position in the
 * group list of the single group to download from.
 */
void getnews(int thisgroup,BOOL quiet)
{
	nLog("getnews((int)%d) called\n",thisgroup);
	char *buffer = new char[MAXLINE + 1];

	int entries = 0;
	getGdataEntries(&entries);
	GroupData **gdata_list = new GroupData *[entries];
	//pre-scan
	ULONG dg=0,i=0;
	int count = 0;
	for(dg=0;dg<entries;dg++)
	{
		GroupData *gdata = NULL;
		getGdata(dg,&gdata);
		if(gdata == NULL)
			continue;
		gdata_list[count++] = gdata;
	}
	entries = count;
	if(entries == 0)
	{
		MUI_RequestA(app,0,0,CatalogStr(MSG_FETCH_NEWS,MSG_FETCH_NEWS_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_CREATE_SOME_NEWSGROUPS,MSG_CREATE_SOME_NEWSGROUPS_STR),0);
		delete [] gdata_list;
		delete [] buffer;
		return;
	}

	GroupData *dl_this_gdata = NULL;
	if(thisgroup != -1) {
		//dl_this_gdata = gdata_list[thisgroup];
		for(dg=0;dg<entries;dg++)
		{
			GroupData *gdata = gdata_list[dg];
			if(gdata->ID == thisgroup) {
				dl_this_gdata = gdata;
				break;
			}
		}
	}
	if(thisgroup != -1 && dl_this_gdata == NULL)
	{
		nLog("Error - can't find group ID %d\n",thisgroup);
		delete [] gdata_list;
		delete [] buffer;
		return;
	}

	for(dg=0;dg<entries;dg++)
	{
		//getGdata(dg,&gdata_list[dg]);
		GroupData *gdata = gdata_list[dg];
		//if(gdata==0)
		//	continue;
		if( (gdata->s && dl_this_gdata==0) || gdata == dl_this_gdata )
		{
		//if( (gdata->s && thisgroup==-1) || dg==thisgroup) {
			//set correct article pointer
			if(gdata->flags[3]==0)
			{
				//first time
				gdata->flags[4]=0;
				if(gdata->max_dl>100 || gdata->max_dl==-1)
				{
					LONG result = 0;
					if(gdata->max_dl==-1)
					{
						do_input(); // seems necessary to fix weird bug where the following requester immediately closes without user-input!
						sprintf(status_buffer_g,CatalogStr(MSG_NO_NEWS_FOR_GROUP,MSG_NO_NEWS_FOR_GROUP_STR),gdata->name);
						result = MUI_RequestA(app,0,0,CatalogStr(MSG_DOWNLOAD,MSG_DOWNLOAD_STR),CatalogStr(MSG_LAST_100_OR_ALL,MSG_LAST_100_OR_ALL_STR),status_buffer_g,0);
					}
					else
					{
						sprintf(status_buffer_g,CatalogStr(MSG_OLD_NEWS_IN_GROUP,MSG_OLD_NEWS_IN_GROUP_STR),gdata->name,gdata->max_dl);
						sprintf(buffer,CatalogStr(MSG_HOW_MANY_MESSAGES,MSG_HOW_MANY_MESSAGES_STR),gdata->max_dl);
						result = MUI_RequestA(app,0,0,CatalogStr(MSG_DOWNLOAD,MSG_DOWNLOAD_STR),buffer,status_buffer_g,0);
					}
					if(result==0)
						gdata->flags[4]=0;
					else
						gdata->flags[4]=-1;
				}
				gdata->flags[3]=-1;
			}
		}
	}

	// perform a sort based on servers
	sortGroupList(gdata_list,entries);

	closeSockets();
	Connection *c = new Connection();
	if(!c->init())
	{
		MUI_RequestA(app,0,0,CatalogStr(MSG_ERROR,MSG_ERROR_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_NO_TCP_STACK,MSG_NO_TCP_STACK_STR),0);
		delete c;
		delete [] gdata_list;
		delete [] buffer;
		return;
	}
	char *big_buffer = new char[big_bufsize_g + 1];
	char windowtitle[1024] = "";
	clock_t time1=0,time2=0;

	mIDhist = NULL;
	delete_dis = TRUE;
	StatusWindow *statusWindow = new StatusWindow(app,CatalogStr(MSG_DOWNLOADING,MSG_DOWNLOADING_STR),quiet);
	statusWindow->setText("\n");
	statusWindow->resize();
	Server *server = NULL;

	// killfiles
	nkills = 0;
	get(NLIST_acc_KILLFILE,MUIA_NList_Entries,&nkills);
	nLog("found %d killfiles\n",nkills);
	killfiles = new KillFile *[nkills];
	replace_killfile = new BOOL[nkills];
	killfile_in_this_group = new BOOL[nkills];
	killfile_types = new int[nkills];
	for (ULONG i=0; i<nkills ;i++)
	{
		KillFile *okill = NULL;
		DoMethod(NLIST_acc_KILLFILE,MUIM_NList_GetEntry,i,&okill);
		killfiles[i] = new KillFile();
		nLog("  about to copy across killfile %d\n",i);
		*killfiles[i] = *okill;
		replace_killfile[i] = FALSE;
		killfile_in_this_group[i] = FALSE;

		KillFile *kill = killfiles[i];
		kill->header[63] = '\0';
		kill->text[255] = '\0';
		kill->ngroups[63] = '\0';

		int type = 0; // 0=unknown, 1=subject, 2=from, 3=date, 4=message-ID, 5=refs
		if( equals("SUBJECT:",kill->header) )
			type = 1;
		else if( equals("FROM:",kill->header) )
			type = 2;
		else if( equals("DATE:",kill->header) )
			type = 3;
		else if( equals("MESSAGE-ID:",kill->header) )
			type = 4;
		else if( equals("REFERENCES:",kill->header) )
			type = 5;
		killfile_types[i] = type;
	}

	for(dg=0;dg<entries;dg++)
	{
		GroupData *gdata = gdata_list[dg];

		// initialise killfiles
		for(i=0;i<nkills;i++)
		{
			KillFile *kill = killfiles[i];
			kill->header[63] = '\0';
			kill->text[255] = '\0';
			kill->ngroups[63] = '\0';

			killfile_in_this_group[i] = FALSE;
			if(stristr_q(gdata->name,kill->ngroups)!=0) {
				killfile_in_this_group[i] = TRUE;
			}
		}

		//if( (gdata->s && thisgroup==-1) || dg==thisgroup) {
		if( ( gdata->s && dl_this_gdata == NULL ) || gdata == dl_this_gdata ) {

			//if(server==0 || (server->ID != gdata->serverID && (gdata->serverID!=0 || server->def==FALSE))) {
			if(server != getServer(gdata->serverID)) {
				if(c->connected())
					hangUp(c);
				server = getServer(gdata->serverID);
				// open a socket
				nLog("Connecting to %s\n",server->nntp);
				sprintf(windowtitle,CatalogStr(MSG_CONNECTING_FOR,MSG_CONNECTING_FOR_STR),gdata->name);
				statusWindow->setTitle(windowtitle);
				sprintf(status_buffer_g,CatalogStr(MSG_CONNECTING_WITH_SERVER,MSG_CONNECTING_WITH_SERVER_STR),server->nntp);
				statusWindow->setText(status_buffer_g);
				if(!logIn(c,server,statusWindow)) {
					if(c->connected())
						hangUp(c);
					// scan to next server
					while(dg<entries-1) {
						dg++;
						gdata = gdata_list[dg];
						if( ( gdata->s && dl_this_gdata == NULL ) || gdata == dl_this_gdata ) {
							if(server != getServer(gdata->serverID)) {
								// this group is on a different server
								dg--; // since it will increment when we continue
								break;
							}
						}
					}
					continue;
				}
			}
			nLog("Downloading: %s\n",gdata->name);
			sprintf(windowtitle,CatalogStr(MSG_DOWNLOADING_FROM,MSG_DOWNLOADING_FROM_STR),gdata->name,server->nntp);
			statusWindow->setTitle(windowtitle);

			gdata->flags[0]=TRUE;
			//read this group
			sprintf(status_buffer_g,CatalogStr(MSG_SCANNING,MSG_SCANNING_STR),gdata->name);
			statusWindow->setText(status_buffer_g);

			sprintf(buffer,"group %s%s",gdata->name,newline);
			c->send_data(buffer);
			if( c->read_data(buffer) == -1 ) {
				readFailedMessage();
				continue;
			}
			StripNewLine(buffer);

			if(statusWindow->isAborted()==TRUE || running==FALSE)
				break;
			if(*buffer!='2')	 {
				sprintf(status_buffer_g,CatalogStr(MSG_CANNOT_GET_GROUP_INFO,MSG_CANNOT_GET_GROUP_INFO_STR),buffer);
				MUI_RequestA(app,0,0,CatalogStr(MSG_ERROR,MSG_ERROR_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),status_buffer_g,0);
				continue;
			}

			char cps[32]="";
			int first = -1, last = -1, totalarts = -1;
			{
				char word1[256] = "";
				word(word1,buffer,2);
				totalarts = atoi(word1);
				word(word1,buffer,3);
				first = atoi(word1);
				word(word1,buffer,4);
				last = atoi(word1);
			}
			nLog("first = %d, last = %d, totalarts = %d\n",first,last,totalarts);
			/* Note that sometimes there will be 'dummy' entries on the
			 * newsserver, ie, IDs in the range first->last for which an article
			 * does not exist. So in general, totarts will not equal
			 * (last-first+1).
			 * Unfortunately when we download only some of the most recent
			 * articles, we no longer know the true number of articles. However,
			 * we only need this information for the status window.
			 */
			//if( totalarts <= getlast )
			if(gdata->flags[4]>last+1) // if flags[4]=article+1, then no new news
				gdata->flags[4]=first; // wraparound; this test shouldn't really be needed!
			/*if(gdata->flags[4]==-1) {
				// special flag to indicate only download the last 100 articles
				int getlast = 100;
				if(gdata->max_dl!=-1) {
					//if( (gdata->max_dl>100 && gdata->flags[4]==0) || gdata->max_dl<=100)
					if( gdata->max_dl<=100 )
						getlast=gdata->max_dl;
				}
				gdata->flags[4]=last-getlast+1;
			}
			if(gdata->max_dl!=-1) {
				if(gdata->flags[4]<last-gdata->max_dl+1)
					gdata->flags[4]=last-gdata->max_dl+1;
			}*/
			int getlast = 0;
			if( gdata->flags[4] == -1 )
				getlast = 100; // special flag to indicate only download the last 100 articles
			else if( gdata->max_dl != -1 )
				getlast = gdata->max_dl;

			if( getlast != 0 && totalarts < getlast ) {
				/* This situation occurs when there are gaps in the article list
				 * (see above). Although the number of articles from start to last
				 * may be greater than the number we wish to download, the true
				 * number of articles is fewer. So we set it to download all of
				 * them.
				 */
				nLog("totalarts (%d) < getlast (%d)\n",totalarts,getlast);
				getlast = last-first+1;
			}
			nLog("getlast = %d\n",getlast);

			if(getlast != 0 && gdata->flags[4]<last-getlast+1)
				gdata->flags[4]=last-getlast+1;

			if(gdata->flags[4]<first)
				gdata->flags[4]=first;
			if(gdata->flags[4]==last+1) {
				sprintf(status_buffer_g,CatalogStr(MSG_NO_NEW_NEWS_FOR,MSG_NO_NEW_NEWS_FOR_STR),gdata->name);
				statusWindow->setText(status_buffer_g);
			}
			else {
				sprintf(status_buffer_g,CatalogStr(MSG_DOWNLOADING_ARTICLES,MSG_DOWNLOADING_ARTICLES_STR),gdata->flags[4],last);
				statusWindow->setText(status_buffer_g);
			}
			int start = gdata->flags[4];
			//printf("%d : %d\n",last-start+1,totalarts);
			int nidz = last - start + 1;
			//printf("%d %d %d\n",first,start,last);
			if(gdata->flags[4] <= 0)
				gdata->moreflags[3]=FALSE; // need to check for message IDs
			// download first to last (inclusive)
			int bread=0,bout=0;
			int tline=0;
			int mcachepos = 0;
			BOOL uptodate = FALSE;
			if(start!=last+1) { // only if new news!
				mIDhist = NULL;
				histlen = 0;
				// only read index if we need to check for message IDs !
				nLog("up to date with newsgroup? %d\n",gdata->moreflags[3]);
				BOOL needIDs = FALSE;
				if(gdata->moreflags[3] == FALSE)
					needIDs = TRUE;

				// we now switch to false, in case there is a crash / connectionloss /
				// whatever whilst downloading - that way, next time we will check for
				// duplicates as we should.
				// otherwise, the flag will be set to TRUE (if appropriate) after
				// downloading for this group.
				uptodate = gdata->moreflags[3];
				gdata->moreflags[3] = FALSE;
				account.save_data(); // so this setting is saved!

				// switch to current group
				if(account.vgroupdl!=0) // change group always
					set_and_read(gdata);

				if(needIDs || (account.flags & Account::CHECKFORDUPS) != 0) {
					sprintf(status_buffer_g,CatalogStr(MSG_READING_MSGID_HISTORY,MSG_READING_MSGID_HISTORY_STR));
					statusWindow->setText(status_buffer_g);

					Vector * vector = NULL;
					GroupData *gdata2 = NULL;
					getGdataDisplayed(&gdata2);
					// read in ID history
					if(gdata->ID!=gdata2->ID) {
						vector = new Vector(2048);
						BOOL res = read_index(gdata,vector);
						histlen=vector->getSize();
					}
					else
						get(NLIST_messagelistdata,MUIA_NList_Entries,&histlen);
					if(histlen>0) {
						mIDhist = new char *[histlen];
						nLog("about to copy messageIDs..\n");
						for(int k=0;k<histlen;k++) {
							MessageListData * mdata = NULL;
							if(vector!=0)
								mdata=((MessageListData **)vector->getData())[k];
							else
								DoMethod(NLIST_messagelistdata,MUIM_NList_GetEntry,k,&mdata);
							mIDhist[k] = new char[256];
							strncpy(mIDhist[k],mdata->messageID,255);
							mIDhist[k][255] = '\0';
							//mIDhist[k] = mdata->mIDhash;
						}
					}
					else
						mIDhist = NULL;
					if(vector!=0) {
						nLog("about to delete vector contents..\n");
						for(i=0;i<vector->getSize();i++)
							delete (MessageListData **)vector->getData()[i];
						nLog("  done\n");
						delete vector;
					}
				}

				//index_handler *ih = new index_handler(gdata);
				//fetch required articles
				mcachepos=0;

				int starttime = clock();
				/*BOOL *skip_articles = new BOOL[nidz];
				BOOL *mark_important = new BOOL[nidz];
				for(int i=0;i<nidz;i++) {
					skip_articles[i] = FALSE;
					mark_important[i] = FALSE;
				}*/

				BOOL use_xover = FALSE;
				char *xover_ptr = NULL;
				if(gdata->flags[5]==TRUE)
					use_xover = TRUE;

				// compatible with killfiles for this group?
				for(int l=0;use_xover && l<nkills;l++) {
					if( killfile_in_this_group[l] && killfile_types[l] == 0 ) {
						use_xover = FALSE;
					}
				}
				nLog("use_xover = %d\n",use_xover);

				if(use_xover) {
					// killfiles - get info for any headers
					//killWithXHDR(skip_articles,mark_important,c,start,last,&statusWindow);

					sprintf(buffer,"XOVER %d-%d%s",start,last,newline);
					c->send_data(buffer);
					int thislen = c->read_data(big_buffer,big_bufsize_g);
					if(*big_buffer!='2') {
						nLog("  no xover!\n");
						use_xover = FALSE;
					}
					else {
						xover_ptr = strchr(big_buffer,'\n');
						while(xover_ptr == NULL) {
							// grab a bit more
							if(thislen < 5 || strncmp(&big_buffer[thislen-5],"\r\n.\r\n",5)!=0) {
								char *hdptr = &big_buffer[thislen];
								bread = c->read_data_chunk(hdptr,big_bufsize_g - thislen);
							}
							xover_ptr = strchr(big_buffer,'\n');
						}
						xover_ptr++;
					}
				}

				sprintf(status_buffer_g,CatalogStr(MSG_DOWNLOADING_2,MSG_DOWNLOADING_2_STR),nidz,cps);
				statusWindow->setText(status_buffer_g);
				const int N_MAX_MDATA_BUFFER_SIZE = 8;
				int n_mdata_buffer_size = gdata->flags[5] ? N_MAX_MDATA_BUFFER_SIZE : 1; // see later comments for more info
				int n_mdata_buffered = 0;
				MessageListData *mdata_buffer[N_MAX_MDATA_BUFFER_SIZE];
				int group_last_downloaded = -1;
				for(int k=start;k<=last;k++)
				{
					nLog("  downloading %d ( %d )\n",k,k-start);
					//printf("  downloading %d ( %d )\n",k,k-start);
					int k1 = k;
					do_input();
					if(statusWindow->isAborted()==TRUE || running==FALSE)
						break;

					unsigned long long bytes_downloaded=0;
					time1=clock();
					int thislen = 0;
					if(!use_xover)
					{
						sprintf(buffer,"HEAD %d%s",k1,newline);
						c->send_data(buffer);
						thislen = c->read_data(big_buffer,big_bufsize_g);
						bytes_downloaded += thislen;
						if(*big_buffer!='2')
						{
							nLog("  no head!\n");
							gdata->flags[4] = k1+1;
							k1 = -1;
						}
					}
					int id1 = 0;
					char filename[300] = "";
					MessageListData * mdata = NULL;
					BPTR fileout = NULL;
					if(k1!=-1) {
						for(;;) { // we want a file that doesn't exist!
							sprintf(filename,"NewsCoasterData:Folder_%d/news_%d",gdata->ID,gdata->nextmID++);

							/*
							if(!exists(filename)) {
								fileout=Open(filename,MODE_NEWFILE);
								if(fileout!=0)
									break;
								else
									gdata->nextmID+=10;
							}
							else
								gdata->nextmID+=10;
							*/
							fileout=Open(filename,MODE_NEWFILE);
							if(fileout != 0)
								break;
							else
								gdata->nextmID += 8;

						}
						id1=gdata->nextmID-1;
						nLog("got file ID %d\n",id1);
					}

					if(fileout==0)
						k1=-1;

					if(k1!=-1)
					{
						mdata = new MessageListData();
						if(mdata==0)
						{
							printf("Error - Can't Allocate mdata Object!\n");
							nLog("Error - Can't Allocate mdata Object!\n");
							delete_dis=FALSE;
							delete c;
							delete [] gdata_list;
							delete [] buffer;
							for(i=0;i<nkills;i++)
								delete killfiles[i];
							delete [] killfiles;
							killfiles = NULL;
							delete [] replace_killfile;
							replace_killfile = NULL;
							delete [] killfile_in_this_group;
							killfile_in_this_group = NULL;
							delete [] killfile_types;
							killfile_types = NULL;
							nkills = 0;
							delete [] big_buffer;
							delete statusWindow;
							return;
						}
						mdata->init();
						for(i=0;i<NMFLAGS;i++)
							mdata->flags[i] = 0;
						mdata->ID=id1;
						mdata->flags[6]=gdata->ID;
						mdata->size=0;
						mdata->flags[1]=TRUE;
						if(gdata->flags[5]==TRUE)
							mdata->flags[12]=TRUE;
						else
							mdata->flags[12]=FALSE;
						mdata->flags[11] = k;

						/*if(mark_important[k-start])
							mdata->flags[13] = 1;*/
					}

					do_input();
					if(statusWindow->isAborted()==TRUE || running==FALSE) {
						k1 = -1;
					}

					if(k1!=-1)
					{
						if(use_xover)
						{
							char *nextline = strchr(xover_ptr,'\n');
							if(nextline == NULL)
							{
								// grab some more
								char *ptr = big_buffer;
								while(*xover_ptr != '\0')
								{
									*ptr = *xover_ptr;
									ptr++;
									xover_ptr++;
								}
								int clen = (int)ptr - (int)big_buffer;
								int thislen = c->read_data(ptr,big_bufsize_g - clen);
								xover_ptr = big_buffer;
								nextline = strchr(xover_ptr,'\n');
							}
							assert(nextline != NULL);
							*nextline = '\0';
							//printf("LINE: *%s*\n",xover_ptr);

							//char *start = strchr(big_buffer,'\n');
							int len = 0;
							char *buffer1_2 = NULL;
							char *buffer1_3 = NULL;

							/*if(start == NULL) {
								//printf("NULL!\n");
								goto cont;
							}
							goto cont;
							start++;*/

							int aid = atoi(xover_ptr);
							if(aid != k) {
								nLog("ID %d != requested %d : skip\n",aid,k);
								*nextline = '\n';
								k1 = -1;
							}
							else
							{
								//BOOL skip = skip_articles[k-start];
								BOOL skip = FALSE;
								char *subject = NULL;
								char *from = NULL;
								char *date = NULL;
								char *messageID = NULL;
								char *references = NULL;
								if(skip)
									goto cont;

								// subject
								if( (buffer1_2 = strchr(xover_ptr,'\t')) == NULL )
									goto cont;
								buffer1_2++;
								if( (buffer1_3 = strchr(buffer1_2,'\t')) == NULL )
									goto cont;
								*buffer1_3 = '\0';
								subject = buffer1_2;
								translateIso(isobuf,buffer1_2);
								strncpy(mdata->subject,isobuf,IHEADENDSUBJECT);
								//strncpy(mdata->subject,buffer1_2,IHEADENDSUBJECT);
								mdata->subject[IHEADENDSUBJECT]= '\0';
								len = sprintf(buffer,"Subject: %s\n",buffer1_2);
								Write(fileout,buffer,len);
								mdata->size += len;
								// advert kill
								if(account.use_sizekill) {
									char *word2 = wordLast(subject);
									if(word2) {
										if(*word2 != '\0') {
											BOOL okay = TRUE;
											int w2len = strlen(word2);
											for(int l=0;l<w2len;l++) {
												if(!isdigit((int)word2[l])) {
													okay = FALSE;
													break;
												}
											}
											if(okay) {
												if(strlen(word2)>=3) { // if number is 3 digits or more
													nLog("*** Message Killed! - 'Advert' %s (%d)\n",word2,k1 - start + 1);
													sprintf(status_buffer_g,CatalogStr(MSG_MESSAGE_KILLED_ADVERT,MSG_MESSAGE_KILLED_ADVERT_STR),k1-start+1);
													statusWindow->setText(status_buffer_g);
													skip = TRUE;
													goto cont;
												}
											}
										}
									}
								}

								// from
								buffer1_2 = &buffer1_3[1];
								if( (buffer1_3 = strchr(buffer1_2,'\t')) == NULL )
									goto cont;
								*buffer1_3 = '\0';
								from = buffer1_2;
								translateIso(isobuf,buffer1_2);
								strncpy(mdata->from,isobuf,IHEADENDSHORT);
								//strncpy(mdata->from,buffer1_2,IHEADENDSHORT);
								mdata->from[IHEADENDSHORT]=0;
								len = sprintf(buffer,"From: %s\n",buffer1_2);
								Write(fileout,buffer,len);
								mdata->size += len;

								// date
								buffer1_2 = &buffer1_3[1];
								if( (buffer1_3 = strchr(buffer1_2,'\t')) == NULL )
									goto cont;
								*buffer1_3 = '\0';
								date = buffer1_2;
								*mdata->datec = '\0';
								DateHandler::read_date(&mdata->ds,buffer1_2,mdata->c_date,mdata->c_time);
								len = sprintf(buffer,"Date: %s\n",buffer1_2);
								Write(fileout,buffer,len);
								mdata->size += len;

								// messageID
								buffer1_2 = &buffer1_3[1];
								if( (buffer1_3 = strchr(buffer1_2,'\t')) == NULL )
									goto cont;
								*buffer1_3 = '\0';
								messageID = buffer1_2;
								strncpy(mdata->messageID,buffer1_2,IHEADENDMID);
								mdata->messageID[IHEADENDMID]=0;
								len = sprintf(buffer,"Message-ID: %s\n",buffer1_2);
								Write(fileout,buffer,len);
								mdata->size += len;
								// duplicate check
								{
									char **hist = mIDhist;
									for(int l=0;l<histlen;l++) {
										if(strcmp(*hist++,mdata->messageID)==0)
										{
											// already downloaded!
											nLog("*** Duplicate Message! - %s (%d, %d)\n",mdata->messageID,k1 - start + 1,l);
											sprintf(status_buffer_g,CatalogStr(MSG_MESSAGE_KILLED_DUPLICATE,MSG_MESSAGE_KILLED_DUPLICATE_STR),k1-start+1);
											statusWindow->setText(status_buffer_g);
											skip = TRUE;
											goto cont;
										}
									}
								}

								// references
								buffer1_2 = &buffer1_3[1];
								if( (buffer1_3 = strchr(buffer1_2,'\t')) == NULL )
									goto cont;
								*buffer1_3 = '\0';
								references = buffer1_2;
								{
									char *lastref = wordLast(buffer1_2);
									strncpy(mdata->lastref,lastref,IHEADENDMID);
									mdata->lastref[IHEADENDMID]=0;
								}
								len = sprintf(buffer,"References: %s\n",buffer1_2);
								Write(fileout,buffer,len);
								mdata->size += len;

								// bytes
								buffer1_2 = &buffer1_3[1];
								if( (buffer1_3 = strchr(buffer1_2,'\t')) == NULL )
									goto cont;
								*buffer1_3 = '\0';

								// lines
								buffer1_2 = &buffer1_3[1];
								if( (buffer1_3 = strchr(buffer1_2,'\t')) == NULL )
									goto cont;
								*buffer1_3 = '\0';
								len = sprintf(buffer,"Lines: %s\n",buffer1_2);
								Write(fileout,buffer,len);
								mdata->size += len;
								mdata->flags[8] = atoi(buffer1_2);
								// kill on # of lines
								if(gdata->flags[2]>0)
								{
									if( mdata->flags[8] > gdata->flags[2])
									{
										nLog("*** Message Killed! - %d longer than %d lines (%d)\n",mdata->flags[8],gdata->flags[2],k1 - start + 1);
										sprintf(status_buffer_g,CatalogStr(MSG_MESSAGE_KILLED_TOO_MANY_LINES,MSG_MESSAGE_KILLED_TOO_MANY_LINES_STR),k1-start+1);
										statusWindow->setText(status_buffer_g);
										skip = TRUE;
										goto cont;
									}
								}

								cont:
								// killfile check
								for(int l=0;l<nkills;l++)
								{
									if(killfile_in_this_group[l])
									{
										KillFile *kill = killfiles[l];

										int type = killfile_types[l];

										if(type == 1 && subject == NULL)
											type = 0;
										else if(type == 2 && from == NULL)
											type = 0;
										else if(type == 3 && date == NULL)
											type = 0;
										else if(type == 4 && messageID == NULL)
											type = 0;
										else if(type == 5 && references == NULL)
											type = 0;

										char *match = NULL;
										BOOL found = TRUE;
										if(type == 1)
											match = stristr_q(subject,kill->text);
										else if(type == 2)
											match = stristr_q(from,kill->text);
										else if(type == 3)
											match = stristr_q(date,kill->text);
										else if(type == 4)
											match = stristr_q(messageID,kill->text);
										else if(type == 5)
											match = stristr_q(references,kill->text);
										else if(type == 0){
											// we shouldn't be using XHDR here
											found = FALSE;
										}
										else
										{
											// something went wrong
											found = FALSE;
										}

										BOOL matched_text = ( kill->match==0 && match!= 0 ) || ( kill->match==1 && match==0 );
										if( found && matched_text ) {
											//matched text

											// kill it!
											BOOL important = FALSE;
											if(killArticle(k1-start+1,l,&important,statusWindow))
												skip = TRUE;

											if(important)
												mdata->flags[13] = 1;

											if(!kill->carryon) {
												nLog("skip rest\n");
												break;
											}
										}
									}
								}

								if(skip) {
									k1 = -1;
								}
								// move to next line
								xover_ptr = &nextline[1];
							}

						}
						else {
							if(thislen < 5 || strncmp(&big_buffer[thislen-5],"\r\n.\r\n",5)!=0) {
								char *hdptr = &big_buffer[thislen];
								bread = c->read_data_chunk(hdptr,big_bufsize_g - thislen);
								bytes_downloaded += bread;
							}
							else {
								big_buffer[thislen-3] = 0;
								bread = -3;
							}

							stripEscapes(big_buffer);

							char *buffer1_2 = strstr(big_buffer,"\r\n") +2;
							bread -= (int)buffer1_2 - (int)big_buffer;

							Write(fileout,buffer1_2,thislen + bread);
							mdata->size += thislen + bread;

							//nLog("about to process header\n");
							on_refs = FALSE;
							for(;;)
							{
								char *buffer1_3 = strstr(buffer1_2,newline);
								if(buffer1_3 != 0) {
									*buffer1_3 = '\0';
									//nLog("  buffer1_2 = %d\n",buffer1_2);
									//nLog("  buffer1_3 = %d\n",buffer1_3);
									if(processHeader(buffer1_2,gdata,mdata,gdata->name,statusWindow,k1-start+1))
									{
										k1=-1;
										break;
									}
									buffer1_2 = &buffer1_3[2];
									if(*buffer1_2 == '\0')
										break;
								}
								else
								{
									big_buffer[bread] = '\0';
									//nLog("  buffer1_2     = %d\n",buffer1_2);
									//nLog("  big_buffer  = %d\n",big_buffer);
									//nLog("  bread         = %d\n",bread);
									if(processHeader(buffer1_2,gdata,mdata,gdata->name,statusWindow,k1-start+1))
									{
										k1=-1;
										break;
									}
									break;
								}
							}
						}
					}
					if(gdata->flags[5]==TRUE && k1!=-1)
					{
						if((account.flags & Account::QUIETDL) != 0)
							sprintf(status_buffer_g,CatalogStr(MSG_DOWNLOADING_3,MSG_DOWNLOADING_3_STR),k1-start+1,nidz,cps);
						else
							sprintf(status_buffer_g,CatalogStr(MSG_DOWNLOADING_4,MSG_DOWNLOADING_4_STR),k1-start+1,nidz,cps,mdata->from,mdata->subject);
						//sprintf(status_buffer_g,"%d\n",k1-start+1);
						statusWindow->setText(status_buffer_g);

						Write(fileout,onlineflag,onlineflag_len);
						mdata->size += onlineflag_len;
					}

					if(gdata->flags[5]==FALSE)
					{
						if(k1!=-1)
						{
							Write(fileout,(void *)"\n",1);
							mdata->size ++;
							sprintf(buffer,"BODY %d%s",k1,newline);
							c->send_data(buffer);

							if((account.flags & Account::QUIETDL) != 0)
								sprintf(status_buffer_g,CatalogStr(MSG_DOWNLOADING_3,MSG_DOWNLOADING_3_STR),k1-start+1,nidz,cps);
							else
								sprintf(status_buffer_g,CatalogStr(MSG_DOWNLOADING_4,MSG_DOWNLOADING_4_STR),k1-start+1,nidz,cps,mdata->from,mdata->subject);
							statusWindow->setText(status_buffer_g);
						}
						if(k1!=-1)
						{
							// now read the body data
							BOOL available = TRUE;
							if(download_body(&available,c,fileout,&bread,&bout,statusWindow,FALSE,buffer) == FALSE) {
								if(!available)
									statusWindow->setText(CatalogStr(MSG_BODY_NOT_FOUND,MSG_BODY_NOT_FOUND_STR));
								k1 = -1;
							}
							bytes_downloaded += bread;
							mdata->size += bout;
						}
					}
					Close(fileout);
					fileout=NULL;

					do_input();
					if(k1==-1 || statusWindow->isAborted()==TRUE || running==FALSE)
					{
						nLog("  deleting file %s\n",filename);
						if(*filename!=0)
							DeleteFile(filename);
						if(mdata) {
							delete mdata;
							mdata=NULL;
						}
						nLog("  done\n");
					}
					else
					{
						/*if(*mdata->type==0)
							strcpy(mdata->type,"text/plain");*/
						/*GroupData * cdg = NULL;
						getGdataDisplayed(&cdg);
						if(cdg->ID==gdata->ID) {
							DoMethod(NLIST_messagelistdata,MUIM_NList_InsertSingle,mdata,MUIV_NList_Insert_Bottom);
							//setEnabled();
						}*/
						//ih->write(mdata);
						//write_index_single(gdata,mdata);
						mdata_buffer[ n_mdata_buffered++ ] = mdata;
						gdata->nummess++;
						gdata->num_unread++;
						group_last_downloaded = k;
					}
					/* Write buffered mdata structs to index
					 * We only write every n_mdata_buffer_size times for performance.
					 * Note that this means that if a crash occurs, NewsCoaster will
					 * end redownloading up to n_mdata_buffer_size messages again
					 * twice. We won't actually end up with multiple copies in the
					 * indices, but this could be considered wasteful in terms of
					 * performance and disk space. Therefore, we only do this
					 * buffering in online mode, where files are smaller, so
					 * downloading twice is less of an issue, and the time spent
					 * accessing the .index file is greater in proportion.
					 * Also note that when a crash occurs, we start downloading from
					 * the same place that we downloaded from the last time (since
					 * the group information with gdata->flags[4] won't have been
					 * saved to disk), but if the messages have been saved to the
					 * .index, duplicate checking means wee won't downnload again.
					 */
					if( n_mdata_buffered == n_mdata_buffer_size || k == last )
					{
						write_index_multi(gdata,mdata_buffer,n_mdata_buffered);
						n_mdata_buffered = 0;
						if( group_last_downloaded != -1 )
							gdata->flags[4] = group_last_downloaded+1;
					}

					time2 = clock() - time1;
					if(time2 == 0)
						time2++;
					if( use_xover )
						*cps = '\0';
					else
					{
						//printf("bytes_downloaded: %llu\n",bytes_downloaded);
						//printf("time1           : %llu\n",time1);
						//printf("time2           : %llu\n",time2);
						//printf("time2-time1     : %llu\n",time1-time2);
						//printf("cps             : %llu\n",(50 * bytes_downloaded * 100) / time2);
						sprintf(cps,"(%llu KBps)", (50 * bytes_downloaded * 100) / time2 );
					}
					if(statusWindow->isAborted()==TRUE || running==FALSE)
						break;
				}
				GroupData * cdg = NULL;
				getGdataDisplayed(&cdg);
				if(cdg->ID==gdata->ID) {
					DoMethod(NLIST_messagelistdata,MUIM_NList_Sort);
					threadView();
					setEnabled();
				}
				// end this group
				//delete ih;
				//ih = NULL;
				//delete [] skip_articles;
				//skip_articles = NULL;
				//delete [] mark_important;
				//mark_important = NULL;

				if(use_xover) {
					int len = strlen(xover_ptr);
					// final line
					char *nextline = strchr(xover_ptr,'\n');
					if(nextline == NULL) {
						// grab some more
						/*char *ptr = big_buffer;
						while(*xover_ptr != '\0') {
							*ptr = *xover_ptr;
							ptr++;
							xover_ptr++;
						}
						int clen = (int)ptr - (int)big_buffer;
						int thislen = c->read_data(ptr,big_bufsize_g - clen);
						xover_ptr = big_buffer;
						nextline = strchr(xover_ptr,'\n');*/
						c->read_data(big_buffer,big_bufsize_g);
					}
				}

				/*int endtime = clock();
				int totaltime = endtime - starttime;
				printf("TIME = %d ( %fs )\n", totaltime, totaltime/50.0);*/
			}
			DateStamp(&gdata->lastdlds);
			redrawGdataAll();
			if( gdata->flags[4] > 0 &&
				( gdata->flags[4] > last || uptodate ) ) {
				// we're up to date with the newsgroup
				// so no need to check for duplicates anymore, until we
				// reset the group article pointer
				gdata->moreflags[3] = TRUE;
			}
			account.save_data(); // save the changed group information!

			if(mIDhist) {
				for(int k=0;k<histlen;k++) {
					if(mIDhist[k]) {
						delete [] mIDhist[k];
						mIDhist[k]=NULL;
					}
				}
				delete [] mIDhist;
				mIDhist=NULL;
			}
			if(statusWindow->isAborted()==TRUE || running==FALSE) {
				break;
			}
			// next newsgroup
		}
	}

	int enkills = 0;
	get(NLIST_acc_KILLFILE,MUIA_NList_Entries,&enkills);
	if(enkills > nkills)
		enkills = nkills;
	for(i=0;i<enkills;i++) {
		if(!replace_killfile[i])
			continue;
		KillFile *kill = killfiles[i];
		KillFile *ekill = NULL;
		DoMethod(NLIST_acc_KILLFILE,MUIM_NList_GetEntry,i,&ekill);
		// check match in case changed
		if(equals(kill->header,ekill->header) &&
			equals(kill->text,ekill->text) &&
			equals(kill->ngroups,ekill->ngroups)) {
			nLog("replacing killfile at position %d ( %s %s %s )\n",i,kill->header,kill->text,kill->ngroups);
			*ekill = *kill;
			DoMethod(NLIST_acc_KILLFILE,MUIM_NList_Redraw,i);
		}
	}
	for(i=0;i<nkills;i++)
		delete killfiles[i];
	delete [] killfiles;
	killfiles = NULL;
	delete [] replace_killfile;
	replace_killfile = NULL;
	delete [] killfile_in_this_group;
	killfile_in_this_group = NULL;
	delete [] killfile_types;
	killfile_types = NULL;
	nkills = 0;

	delete [] gdata_list;
	gdata_list = NULL;

	if(c->connected())
		hangUp(c);
	//finished all
	statusWindow->setVisible(FALSE);
	delete_dis=FALSE;

	delete c;
	delete [] buffer;
	delete [] big_buffer;
	delete statusWindow;
	return;
}

int shrink(BPTR file,int length,int oldsize)
{
	nLog("shrink((BPTR)%d,(int)%d,(int)%d) called\n",file,length,oldsize);
	if(ChangeFileSize(file,length,OFFSET_BEGINNING) == -1)
	{
		nLog("ChangeFileSize() failed - pad out with extra bytes\n");
		// Alternative to shrinking the file size
		ChangeFilePosition(file,length,OFFSET_BEGINNING);
		for(int i=0;i<oldsize - length;i++)
			FPutC(file,'\0');
		return oldsize;
	}
	return length;
}

BOOL getBody(BOOL *available,GroupData * gdata,MessageListData * mdata,BOOL quiet) {
	StatusWindow *statusWindow = new StatusWindow(app,"Downloading body..",quiet);
	BOOL res = getBody(available,gdata,mdata,statusWindow,-1,-1,TRUE,TRUE);
	delete statusWindow;
	return res;
}

BOOL getBody(BOOL *available,GroupData * gdata,MessageListData * mdata,StatusWindow * statusWindow,int index,int max,BOOL first, BOOL delmess) {
	nLog("getBody((GroupData *)%d,(MessageListData *)%d,(StatusWindow *)%d,(int)%d,(int)%d,(BOOL)%d,(BOOL)%d) called\n",gdata,mdata,statusWindow,index,max,first,delmess);
	Server *server = getServer(gdata->serverID);
	if(online_c != 0 && online_c->serverID != -1
		&& online_c->serverID != server->ID) {
		nLog("Different server - close sockets\n");
		closeSockets();
	}

	if(online_c == NULL) {
		online_c = new Connection();
		if(!online_c->init()) {
			MUI_RequestA(app,0,0,CatalogStr(MSG_ERROR,MSG_ERROR_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_NO_TCP_STACK,MSG_NO_TCP_STACK_STR),0);
			delete online_c;
			online_c = NULL;
			return FALSE;
		}
	}
	*available = TRUE;
	//ViewWindow::sleepAll(TRUE);
	/*statusWindow->setText("\n");
	statusWindow->resize();*/
	GroupData * gdata_curr=NULL;
	BOOL OKAY = TRUE;
	char *line_buffer = new char[MAXLINE+1];
	char *big_buffer = new char[big_bufsize_g + 1];
	unsigned long long bytes_downloaded=0;
	int bread = 0,bout = 0;

	int entries = 0;
	getGdataEntries(&entries);

	delete_dis=TRUE;

	char filename[300] = "";
	getFilePath(filename,gdata->ID,mdata->ID);
	BPTR lock = Lock(filename,ACCESS_READ); // READ?
	BPTR fileout = Open(filename,MODE_OLDFILE);

	LONG oldpos = 0;
	int oldsize = mdata->size;
	//BOOL xheader_zapped = FALSE;

	char bodyline[MAXLINE] = "";
	char command[] = "ARTICLE";
	if(strlen(mdata->messageID) >= IHEADENDMID-1) {
		NewMessage nm;
		strcpy(nm.getThisHeader,"Message-ID:");
		get_refs(&nm,gdata->ID,mdata->ID,GETREFS_NONE);
		sprintf(bodyline,"%s %s%s",command,nm.dummyHeader,newline);
	}
	else
		sprintf(bodyline,"%s %s%s",command,mdata->messageID,newline);

	set(menuitem_DISCONNECT,MUIA_Menuitem_Enabled,FALSE);

	BOOL connected = FALSE;

	int hdlen = 0;

	//while(!connected) {
	for(int attempts=0;attempts<2 && !connected;attempts++) {
		nLog("attempt number %d\n",attempts);

		connected = TRUE;

		if(!online_c->connected()) {
			sprintf(status_buffer_g,CatalogStr(MSG_CONNECTING_WITH_SERVER,MSG_CONNECTING_WITH_SERVER_STR),server->nntp);
			statusWindow->setText(status_buffer_g);
			if(!online_c->call_socket(server)) {
				MUI_RequestA(app,0,0,CatalogStr(MSG_ERROR,MSG_ERROR_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_CANNOT_CONNECT_WITH_SERVER,MSG_CANNOT_CONNECT_WITH_SERVER_STR),0);
				OKAY = FALSE;
			}
			if(OKAY) {
				if( online_c->read_data(line_buffer) == -1 ) {
					readFailedMessage();
					OKAY = FALSE;
				}
			}
			if(OKAY) {
				StripNewLine(line_buffer);
				if(*line_buffer!='2' || statusWindow->isAborted()==TRUE) {
					if(statusWindow->isAborted()==FALSE) {
						sprintf(status_buffer_g,CatalogStr(MSG_CANNOT_USE_SERVER,MSG_CANNOT_USE_SERVER_STR),line_buffer);
						MUI_RequestA(app,0,0,CatalogStr(MSG_ERROR,MSG_ERROR_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),status_buffer_g,0);
					}
					online_c->send_data("quit\r\n");
					OKAY = FALSE;
				}
			}
			//authenticate
			if(OKAY && !auth(online_c,server,statusWindow)) {
				//sprintf(status_buffer_g,"\33cCannot Log Into Newsserver!\nError: %s",line_buffer);
				//MUI_RequestA(app,0,0,"Error!","_Okay",status_buffer_g,0);
				online_c->send_data("quit\r\n");
				OKAY = FALSE;
			}
			if(OKAY) {
				sprintf(line_buffer,"mode reader%s",newline);
				online_c->send_data(line_buffer);
				online_c->read_data(line_buffer);
				StripNewLine(line_buffer);
				if(statusWindow->isAborted()==FALSE) {
					if(*line_buffer!='2' && *line_buffer!='5') {
						nLog("Warning: %s received from 'mode reader' command\n",line_buffer);
					}
				}
			}
		}
		if(!OKAY) {
			nLog("not okay - exiting loop\n");
			break;
		}

		if(index > 0 && max > 0)
		{
			sprintf(status_buffer_g,CatalogStr(MSG_DOWNLOADNG_OF,MSG_DOWNLOADNG_OF_STR),index,max);
			statusWindow->setText(status_buffer_g);
		}
		else
			statusWindow->setText(CatalogStr(MSG_DOWNLOADING_5,MSG_DOWNLOADING_5_STR));

		statusWindow->resize();

		if(first)
		{
			sprintf(line_buffer,"group %s%s",gdata->name,newline);
			//printf("send: %s",line_buffer);
			online_c->send_data(line_buffer);
			online_c->read_data(line_buffer);
		}

		if(fileout!=0 && lock!=0)
		{
			/*if(!xheader_zapped) {
				xheader_zapped = TRUE;
				ChangeFilePosition(fileout,-onlineflag_len,OFFSET_END);
				Read(fileout,line_buffer,onlineflag_len);
				if(strncmp(line_buffer,onlineflag,onlineflag_len)==0) {
					ChangeFilePosition(fileout,-onlineflag_len,OFFSET_CURRENT);
					mdata->size -= onlineflag_len;
				}
				else
					ChangeFilePosition(fileout,0,OFFSET_END);
				oldpos = ChangeFilePosition(fileout,0,OFFSET_CURRENT);
				// only set online flag afterwards, if necessary
				//mdata->flags[12]=FALSE;
				//gdata->flags[1]=TRUE;
				Write(fileout,"\n",1);
				mdata->size++;
			}*/
			hdlen = 0;
			for(;;) {
				if(!FGets(fileout,&big_buffer[hdlen],big_bufsize_g - hdlen))
					break;

				if(big_buffer[hdlen] == '\0' || big_buffer[hdlen] == '\n' || big_buffer[hdlen] == '\r')
					break;

				hdlen += strlen(&big_buffer[hdlen]);

				if(hdlen >= big_bufsize_g) {
					hdlen = big_bufsize_g;
					break;
				}
			}
			ChangeFilePosition(fileout,0,OFFSET_BEGINNING);
			big_buffer[hdlen] = '\0';
			mdata->size = 0;
			//printf("*%s*\n",big_buffer);

			for(int go=0;go<2;go++) {
				nLog("(%d) sending: %s\n",go,bodyline);
				online_c->send_data(bodyline);
				// now read the body data
				if(download_body(available,online_c,fileout,&bread,&bout,statusWindow,TRUE,line_buffer)) {
					// ok!
					nLog("ok!\n");
					break;
				}
				else {
					nLog("failed - available = %d\n",*available);
					if(bread <= 0) {
						nLog("lost connection?\n");
						connected = FALSE; // lost connection?
						first = TRUE;
						online_c->close();
						break;
					}
					else {
						BOOL another_go = TRUE;
						if(go==0) {
							// try with article number?
							int id = mdata->flags[11];
							nLog("trying with article number %d\n",id);
							//printf("trying with article number %d\n",id);
							if(id==0)
								another_go = FALSE;
							if( *available )
								another_go = FALSE; // don't try again if the problem was something other than BODY not available
							if(another_go) {
								sprintf(line_buffer,"group %s%s",gdata->name,newline);
								nLog("send: %s\n",line_buffer);
								//printf("send: %s",line_buffer);
								online_c->send_data(line_buffer);
								online_c->read_data(line_buffer);
								if(*line_buffer != '2') {
									nLog("group command returned %s - fail\n",line_buffer);
									another_go = FALSE;
								}
							}
							if(another_go) {
								*available = TRUE; // reset!
								sprintf(line_buffer,"stat %d%s",id,newline);
								online_c->send_data(line_buffer);
								nLog("send: %s\n",line_buffer);
								//printf("send: %s",line_buffer);
								online_c->read_data(line_buffer);
								nLog("rcvd: %s\n",line_buffer);
								//printf("rcvd: %s\n",line_buffer);
								if(*line_buffer != '2')
									another_go = FALSE;
								else {
									char word3[1024] = "";
									word(word3,line_buffer,3);
									if(stricmp(word3,mdata->messageID)!=0)
										another_go = FALSE;
								}
								if(!another_go)
									*available = FALSE; // put back again
							}
							if(another_go) {
								sprintf(bodyline,"%s %d%s",command,id,newline);
							}
						}
						if(go==1 || !another_go) {
							if( !(*available) )
								statusWindow->setText(CatalogStr(MSG_MESSAGE_IS_NO_LONGER_AVAILABLE,MSG_MESSAGE_IS_NO_LONGER_AVAILABLE_STR));
							OKAY=FALSE;
							break;
						}
					}
				}
			}
		}
	}
	if(!connected) {
		// couldn't get connection
		OKAY = FALSE;
	}
	if(fileout!=0 && lock!=0) {
		bytes_downloaded += bread;
		mdata->size += bout;
		if( statusWindow->isAborted()==TRUE || running==FALSE || OKAY==FALSE ) {
			nLog("rewrite original header\n");
		//if( FALSE ) {
			/*if( xheader_zapped ) {
				Flush(fileout);
				ChangeFilePosition(fileout,oldpos,OFFSET_BEGINNING);
				Write(fileout,onlineflag,onlineflag_len);
				mdata->size = oldpos + onlineflag_len;
				// flags[12] should still be TRUE
				//mdata->flags[12]=TRUE;
			}*/
			FFlush(fileout);
			ChangeFilePosition(fileout,0,OFFSET_BEGINNING);
			if(hdlen > 0)
			{
				Write(fileout,big_buffer,hdlen);
			}
			if(mdata->size > hdlen)
			{
				mdata->size = shrink(fileout,hdlen,mdata->size);
			}
			mdata->size = hdlen;
			nLog("done\n");
		}
		else {
			mdata->flags[12]=FALSE;
			gdata->flags[1]=TRUE;
			if(mdata->size < oldsize) {
				// small bodies - we need to overwrite the 'onlineflag' text
				nLog("small body case\n");
				/*if(SetFileSize(fileout,mdata->size,OFFSET_BEGINNING) == -1) {
					nLog("SetFileSize() failed - pad out with extra bytes\n");
					// Alternative to shrinking the file size
					for(int i=0;i<oldsize - mdata->size;i++)
						FPutC(fileout,'\0');
					mdata->size = oldsize;
				}*/
				mdata->size = shrink(fileout,mdata->size,oldsize);
			}
		}
		Close(fileout);
		fileout=NULL;
		UnLock(lock);
		lock=NULL;
	}
	else {
		//printf("Can't Write To File: %s\n",filename);
		nLog("Can't Write To File: %s\n",filename);
		OKAY=FALSE;
		sprintf(line_buffer,CatalogStr(MSG_UNABLE_TO_OPEN_FILE,MSG_UNABLE_TO_OPEN_FILE_STR),filename);
		MUI_RequestA(app,0,0,CatalogStr(MSG_ERROR,MSG_ERROR_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),line_buffer,0);
	}
	if(max == -1)
		redrawGdataAll();

	//finished all
	delete_dis=FALSE;
	if(statusWindow->isAborted()==TRUE || running==FALSE || OKAY==FALSE) {
		closeSockets();
	}
	else {
		set(menuitem_DISCONNECT,MUIA_Menuitem_Enabled,TRUE);
		online_c->doOnlineBlocking(FALSE,FALSE);
	}
	//ViewWindow::sleepAll(FALSE);

	nLog("  available = %d\n",*available);
	if( !(*available) ) {
		if(delmess && (account.flags & Account::NODELONLINE)==0) {
			// delete message
			//set_and_read(gdata);
			nLog(" >>> deleting message header\n");
			//delete_mess(gdata,mdata,FALSE);
			delete_mess_n(gdata,&mdata,1,TRUE);
			redrawGdataAll();
		}
	}

	delete [] big_buffer;
	delete [] line_buffer;

	nLog("OKAY = %d\n",OKAY);
	return OKAY;
}

void closeSockets() {
	nLog("closeSockets() called\n");
	set(menuitem_DISCONNECT,MUIA_Menuitem_Enabled,FALSE);
	if(online_c != 0) {
		hangUp(online_c);
		delete online_c;
		online_c = NULL;
	}
}
