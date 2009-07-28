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

#include <proto/socket.h>

#ifdef __STORM__
#include <pragma/socket_lib.h>	//AmiTCP SDK does not have pragmas for STORM C
#endif

#include <sys/ioctl.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>

#include "vector.h"
#include "various.h"
#include "main.h"
#include "misc.h"
#include "connection.h"
#include "subthreads.h"

int Connection::connections = 0;

/* Connection constructor.
 */
Connection::Connection() {
	nLog("Connection constructor called\n");
	this->s = -1;
	this->serverID = -1;
	//doOnlineBlocking(TRUE,FALSE);
}

/* Close connecton.
 */
Connection::~Connection() {
	nLog("Connection deconstructor called\n");
	close();
	connections--;
	if(SocketBase!=NULL && connections==0) {
#ifdef __amigaos4__
		if(ISocket) {
			DropInterface((struct Interface *)ISocket);
		}
		ISocket = NULL;
#endif
		CloseLibrary(SocketBase);
		SocketBase=NULL;
	}
	//doOnlineBlocking(FALSE,FALSE);
}

/* Initialise the connection.
 */

BOOL Connection::init() {
	nLog("Connection::init() called\n");
	BOOL res = FALSE;
	if(SocketBase!=NULL || (SocketBase = OpenLibrary("bsdsocket.library",2L)) != NULL) {
#ifdef __amigaos4__
		if(!(ISocket=(struct SocketIFace *)GetInterface(SocketBase,"main", 1, NULL)))
		{
			printf("Failed to open interface to \"Socket Library\".\n");
		}
		else
		{
#endif
			res = TRUE;
			connections++;
#ifdef __amigaos4__
		}
#endif
	}
	nLog("    return %d\n",res);
	return res;
}

/* Close the connection.
 */
void Connection::close() {
	nLog("Connection::close() called\n");
	if(s!=-1) {
		shutdown(s,2);
		CloseSocket(s);
		s = -1;
	}
}

/* Opens a connection to a server.
 */
BOOL Connection::call_socket(Server *server) {
	nLog("Connection::call_socket((Server *)%d) called\n",server);
	nLog("Server ID = %d\n",server->ID);
	this->serverID = server->ID;
	return call_socket(server->nntp,server->port);
}

/* Opens a connection to a host.
 */
BOOL Connection::call_socket(char *hostname, unsigned short portnum) {
	nLog("Connection::call_socket((char *)%s,(unsigned short)%d) called\n",hostname,portnum);
	struct sockaddr_in sa;
	struct hostent     *hp = NULL;
	if(!(hp = gethostbyname((char *)hostname))) {
		nLog("FAILED gethostbyname\n");
		return FALSE;
	}
	sa.sin_len = sizeof(sa);
	sa.sin_family = 2;
	sa.sin_port = portnum;
	sa.sin_addr.s_addr = 0;
	memcpy(&sa.sin_addr, hp->h_addr, hp->h_length);
	s = socket(hp->h_addrtype, 1, 0);
	if(s == -1) {
		nLog("failed to obtain socket\n");
		return FALSE;
	}
	//LONG async = 1L;
	//IoctlSocket(s,FIOASYNC,(char *)&async);
	/*
	LONG noblock = 1L;
	IoctlSocket(s,FIONBIO,(char *)&noblock);

	struct timeval timeout;
	timeout.tv_secs = 30;
	timeout.tv_micro = 1;
	if(setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(struct timeval)) == -1)
		return -1;
	*/
	if(connect(s, (struct sockaddr *)&sa, sizeof(sa)) == -1) {
		nLog("Failed to connect\n");
		shutdown(s,2);
		CloseSocket(s);
		return FALSE;
	}
	nLog("success\n");
	return TRUE;
}

/* Reads bytes into buffer from s, until '.' is reached, or the buffer is
 * full.
 * Returns length if more data is still to come, or the amount of data
 * read, otherwise.
 */
int Connection::read_data_chunk(char *buffer,int length) {
	*buffer = '\0';

	LONG eof = 0;
	int bytesread = 0;
	int lentogo = length;
	static char end[] = {13,10,'.',13,10,0};
	char * startpos = buffer;

	for(;;) {
		//DoMethod(app,MUIM_Application_InputBuffered);
		if( thread_aborted() )
			break;
		eof=recv(s,(unsigned char*)buffer,lentogo,0);
		//printf("%d\n",eof);
		if(eof>0) {
			buffer += eof;
			*buffer = '\0';
			bytesread += eof;
			//*buffer = '\0';
			if(strcmp(&buffer[-5],end)==0) {
				buffer[-3] = '\0';
			   //stripEscapes(startpos);
				return ((int)&buffer[-3] - (int)startpos);
			}
			lentogo -= eof;
			if(lentogo<=0) {
			   //stripEscapes(startpos);
				return length;
			}
		}
	}
   //stripEscapes(startpos);
	return bytesread;
}

/* Reads up to 'len'-1 bytes into buffer.
 * Returns the number of bytes read.
 */
int Connection::read_data(char *buffer,int len) {
   //DoMethod(app,MUIM_Application_InputBuffered);
	int lenread = recv(s,(unsigned char*)buffer,len-1,0);
	if( lenread == -1 ) {
		*buffer = '\0';
		return -1;
	}
   buffer[lenread] = '\0';
   //stripEscapes(buffer);
   return lenread;
}

int Connection::read_data(char *buffer) {
	return read_data(buffer,MAXLINE);
}

/* Reads bytes into buffer from s, until asc(10,13) is reached.
 * Returns the number of bytes read.
 */
int Connection::read_data_line(char *buffer) {
	*buffer = '\0';

	LONG eof = 0;
	char *start = buffer;

	for(;;) {
		//DoMethod(app,MUIM_Application_InputBuffered);
		if( thread_aborted() )
			break;
		eof=recv(s,(unsigned char*)buffer,2,0);
		if( eof == -1 ) {
			*buffer = '\0';
			return -1;
		}
		buffer += eof;
		if( ((int)buffer - (int)start) >= 2 && buffer[-2]==13) {
			// must have read 13/10
			*buffer=0;
			break;
		}
		else if( ((int)buffer - (int)start) >= 1 && buffer[-1]==13) {
			eof=recv(s,(unsigned char*)buffer,1,0); // get asc(10)
			buffer++;
			*buffer=0;
			break;
		}
	}
	return (int)buffer - (int)start;
}

void Connection::doOnlineBlocking(BOOL online,BOOL getbody) {
	// getbody not used atm, since we block during getbody
	// should not set status of anything set in setEnabled(), to avoid conflict!
	BOOL s=!online;
	set(menuitem_GETNEWS,MUIA_Menuitem_Enabled,s);
	//set(menuitem_GETNEWSSINGLE,MUIA_Menuitem_Enabled,s);
	set(menuitem_SENDNEWS,MUIA_Menuitem_Enabled,s);
	set(menuitem_GETGROUPSDEF,MUIA_Menuitem_Enabled,s);
	set(menuitem_GETNEWGROUPSDEF,MUIA_Menuitem_Enabled,s);
	set(menuitem_UPDATEGROUPS,MUIA_Menuitem_Enabled,s);
	set(menuitem_UPDATEINDEX,MUIA_Menuitem_Enabled,s);
	set(menuitem_UPDATEALLIND,MUIA_Menuitem_Enabled,s);
	set(menuitem_IMPORT,MUIA_Menuitem_Enabled,s);
	set(BT_search_START,MUIA_Disabled,online);
	set(BT_stats_START,MUIA_Disabled,online);
	if(online==FALSE)
		setEnabled();
}

void Connection::send_data(const char *buffer) {
   //DoMethod(app,MUIM_Application_InputBuffered);
	send(s,(unsigned char*)buffer,strlen(buffer),0);
}
