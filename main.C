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
#ifdef __amigaos4__
#define FPrintf(fh, ...) IDOS->FPrintf(fh, __VA_ARGS__)
#define OpenCatalog(locale, ...) ILocale->OpenCatalog(locale, __VA_ARGS__)
#endif
// labels:
// Label1 for checks (std frames)
// Label2 for strings (dbl high frames)
// Label; for other noframe objects

#include "mui_headers.h"
#include <mui/NFloattext_mcc.h>
#include <mui/NListview_mcc.h>
#include <mui/TextEditor_mcc.h>
#include <mui/BetterString_mcc.h>
#include <mui/NListtree_mcc.h>
//#include <mui/Toolbar_mcc.h>
#include <mui/TheBar_mcc.h>
#include <MUI/NBitmap_mcc.h>

#include <exec/exec.h>
#include <proto/socket.h>
#include <netdb.h>
#include <proto/rexxsyslib.h>
#include <proto/locale.h>
#ifdef __amigaos4__
#include <dos/anchorpath.h>
#include <assert.h>
#endif
#include <proto/dos.h>

char *LocalCharset;

struct codesetList 	*codesetsList = NULL; 	//for codesets - GLOBAL variable
struct codeset 		*sysCodeset   = NULL;	//for codesets - GLOBAL variable

int main(int argc, char **argv);

//#include <time.h>	There was a problem with time_t redefinition in time.h
extern "C"
{
	clock_t clock(void);
}
#include <string.h>
#include <ctype.h>

#include "vector.h"
#include "various.h"
#include "viewwindow.h"
#include "writewindow.h"
#include "main.h"
#include "indices.h"
#include "misc.h"
#include "statuswindow.h"
#include "netstuff.h"
#include "general.h"
#include "choicelist.h"
#include "datehandler.h"
#include "strings.h"
#include "lists.h"
#include "subthreads.h"
#include "newscoaster_catalog.h"

__attribute__ ((used)) static const char *__stackcookie = "$STACK: 1000000";
__attribute__ ((used)) static const char *__version     = "$VER: NewsCoaster 1.65 for AmigaOS4 (22-07-2009)";

#define TextList(ftxt)	NListviewObject, MUIA_Weight, 50, MUIA_NList_Input, FALSE, MUIA_NListview_NList, NFloattextObject, MUIA_Frame, MUIV_Frame_ReadList, MUIA_Background, MUII_ReadListBack, MUIA_NFloattext_Text, ftxt, MUIA_NFloattext_TabSize, 4, MUIA_NFloattext_Justify, TRUE, End, End

struct Library *MUIMasterBase = NULL;
struct Library *CodesetsBase = NULL;
#ifdef __amigaos4__
struct MUIMasterIFace 	*IMUIMaster = NULL;
struct CodesetsIFace	*ICodesets	= NULL;
#endif
struct Catalog *nc_Catalog;

char status_buffer_g[1025] = "";

static char scrtitle[256]="";
static char base_scrtitle[256]="";

#define VERSION_STRING "1.65 " PLATFORM
const float ver=1.64;
const char version[]=VERSION_STRING;
const char copyright_string[] = "©1999-2008 NewsCoaster Dev Team";
char newscoaster_name[256] ="";
char muiappversion[256] ="";
User * currentUser = NULL;

static char messagelistformat[64] = "";

BOOL DEBUG=FALSE;

enum {
	MEN_PROJECT,MEN_GETNEWS,MEN_GETNEWSSINGLE,MEN_SENDNEWS,MEN_GETGROUPSDEF,MEN_GETNEWGROUPSDEF,MEN_DISCONNECT,MEN_USER,MEN_ABOUT,MEN_ABOUTMUI,MEN_SYSINFO,MEN_ICONIFY,MEN_QUIT,
	MEN_NEWSGROUPS, MEN_NEWNEWSGROUP, MEN_EDITNG, MEN_NGADV, MEN_DELNG, MEN_ARCHIVENG,
	MEN_VIEWGROUPS, MEN_VIEWGROUPSLIST, MEN_VIEWGROUPSTREE,
	MEN_SORT, MEN_SEARCH, MEN_STATS, MEN_RESETGROUPPTR, MEN_RESETALLPTR, MEN_GROUPMANDEF, MEN_UPDATEGROUPS, MEN_UPDATEINDEX, MEN_UPDATEALLIND, MEN_IMPORT,
	MEN_MESSAGES,
	MEN_VIEWMESSAGES, MEN_VIEWMESSAGESLIST, MEN_VIEWMESSAGESTREE,
	MEN_READ, MEN_EDIT, MEN_SUPER, MEN_CANCEL, MEN_DELMESS, MEN_UNDELMESS, MEN_POST, MEN_FOLLOWUP, MEN_REPLY, MEN_BOTH, MEN_EXP, MEN_VIEWHEADER, MEN_MOVE, MEN_COPY, MEN_KILLMESS, MEN_YAMADDRESS,
	MEN_PD, MEN_PDTHIS, MEN_PDREAD, MEN_PDONLINE, MEN_PDDUP, MEN_PDALL, MEN_PDALLALL,
	MEN_MARK, MEN_MARKUNREAD, MEN_MARKREAD, MEN_MARKHELD, MEN_MARKQUEUED, MEN_MARKNORMAL, MEN_MARKIMPORTANT,
	MEN_SELECT, MEN_SELECTALL, MEN_SELECTNONE, MEN_SELECTTOGGLE, MEN_SELECTFROM, MEN_SELECTSUBJECT, MEN_SELECTREAD, MEN_SELECTUNREAD,
	MEN_CURRENTDATE, MEN_DOWNLOADBODY, MEN_JOINMSGS,
	MEN_PREFS, MEN_ACCOUNT, MEN_KILLFILE, MEN_USERS, MEN_CPWD, MEN_MUIPREFS,

};

/* Menu for the Main Window
 */
static struct NewMenu MenuData1[]= {
	{NM_TITLE,(CONST STRPTR)"Project",						0,0,0,	(APTR)MEN_PROJECT},
	{NM_ITEM,(CONST STRPTR)"Fetch News",					0,0,0,	(APTR)MEN_GETNEWS},
	{NM_ITEM,(CONST STRPTR)"Fetch News for this Group",0,0,0,	(APTR)MEN_GETNEWSSINGLE},
	{NM_ITEM,(CONST STRPTR)"Send News",					0,0,0,	(APTR)MEN_SENDNEWS},
	{NM_ITEM,(CONST STRPTR)"Fetch Group List",			0,0,0,	(APTR)MEN_GETGROUPSDEF},
	{NM_ITEM,(CONST STRPTR)"Fetch New Groups",			0,0,0,	(APTR)MEN_GETNEWGROUPSDEF},
	{NM_ITEM,(CONST STRPTR)"Disconnect From Newsserver",0,NM_ITEMDISABLED,0,	(APTR)MEN_DISCONNECT},
	{NM_ITEM,NM_BARLABEL,					0,0,0,	(APTR)0},
//	{NM_ITEM,(CONST STRPTR)"User Info...",				0,0,0,	(APTR)MEN_USER},
	{NM_ITEM,(CONST STRPTR)"About NewsCoaster...",		(CONST STRPTR)"?",0,0,	(APTR)MEN_ABOUT},
	{NM_ITEM,(CONST STRPTR)"About MUI...",				0,0,0,	(APTR)MEN_ABOUTMUI},
	{NM_ITEM,(CONST STRPTR)"System Information...",	0,0,0,	(APTR)MEN_SYSINFO},
	{NM_ITEM,(CONST STRPTR)"Iconify",						(CONST STRPTR)"H",0,0,	(APTR)MEN_ICONIFY},
	{NM_ITEM,(CONST STRPTR)"Quit",							(CONST STRPTR)"Q",0,0,	(APTR)MEN_QUIT},

	{NM_TITLE,(CONST STRPTR)"Newsgroups",					0,0,0,	(APTR)MEN_NEWSGROUPS},
	{NM_ITEM,(CONST STRPTR)"New Newsgroup...",			(CONST STRPTR)"N",0,0,	(APTR)MEN_NEWNEWSGROUP},
	{NM_ITEM,(CONST STRPTR)"Edit Newsgroup...",			0,0,0,	(APTR)MEN_EDITNG},
	{NM_ITEM,(CONST STRPTR)"Advanced Settings...",		0,0,0,	(APTR)MEN_NGADV},
	{NM_ITEM,(CONST STRPTR)"Delete Newsgroup",			0,0,0,	(APTR)MEN_DELNG},
	{NM_ITEM,(CONST STRPTR)"Archive Newsgroup...",		0,0,0,	(APTR)MEN_ARCHIVENG},
	{NM_ITEM,(CONST STRPTR)"Groups Manager...",			0,0,0,	(APTR)MEN_GROUPMANDEF},
	{NM_ITEM,(CONST STRPTR)"View",							0,0,0,	(APTR)MEN_VIEWGROUPS},
	{NM_SUB,(CONST STRPTR)"List",							0,CHECKIT|CHECKED,0,(APTR)MEN_VIEWGROUPSLIST},
	{NM_SUB,(CONST STRPTR)"Tree",							0,CHECKIT,0,	(APTR)MEN_VIEWGROUPSTREE},
	{NM_ITEM,NM_BARLABEL,					0,0,0,	(APTR)0},
	{NM_ITEM,(CONST STRPTR)"Sort Groups",					0,0,0,	(APTR)MEN_SORT},
	{NM_ITEM,(CONST STRPTR)"Search Groups...",			0,0,0,	(APTR)MEN_SEARCH},
	{NM_ITEM,(CONST STRPTR)"Group Statistics...",		0,0,0,	(APTR)MEN_STATS},
	{NM_ITEM,NM_BARLABEL,					0,0,0,	(APTR)0},
	{NM_ITEM,(CONST STRPTR)"Reset Group Article Pointer ",0,0,0,	(APTR)MEN_RESETGROUPPTR},
	{NM_ITEM,(CONST STRPTR)"Reset All Groups' Pointers ",0,0,0,	(APTR)MEN_RESETALLPTR},
	{NM_ITEM,NM_BARLABEL,					0,0,0,	(APTR)0},
	{NM_ITEM,(CONST STRPTR)"Update Groups",				0,0,0,	(APTR)MEN_UPDATEGROUPS},
	{NM_ITEM,(CONST STRPTR)"Update Index",				0,0,0,	(APTR)MEN_UPDATEINDEX},
	{NM_ITEM,(CONST STRPTR)"Update All Indices",		0,0,0,	(APTR)MEN_UPDATEALLIND},
	{NM_ITEM,(CONST STRPTR)"Import Messages...",		0,0,0,	(APTR)MEN_IMPORT},
	{NM_TITLE,(CONST STRPTR)"Messages",					0,0,0,	(APTR)MEN_MESSAGES},
	{NM_ITEM,(CONST STRPTR)"View",							0,0,0,	(APTR)MEN_VIEWMESSAGES},
	{NM_SUB,(CONST STRPTR)"Flat",							0,CHECKIT|CHECKED,0,(APTR)MEN_VIEWMESSAGESLIST},
	{NM_SUB,(CONST STRPTR)"Threaded",						0,CHECKIT,0,	(APTR)MEN_VIEWMESSAGESTREE},
	{NM_ITEM,(CONST STRPTR)"Read Message",				0,0,0,	(APTR)MEN_READ},
	{NM_ITEM,(CONST STRPTR)"Edit Message...",			0,0,0,	(APTR)MEN_EDIT},
	{NM_ITEM,(CONST STRPTR)"Supersede Message...",		0,0,0,	(APTR)MEN_SUPER},
	{NM_ITEM,(CONST STRPTR)"Cancel Message",				0,0,0,	(APTR)MEN_CANCEL},
	{NM_ITEM,(CONST STRPTR)"Delete Message",				(CONST STRPTR)"D",0,0,	(APTR)MEN_DELMESS},
	{NM_ITEM,(CONST STRPTR)"Undelete Message",			0,0,0,	(APTR)MEN_UNDELMESS},
	{NM_ITEM,(CONST STRPTR)"Export Message",				(CONST STRPTR)"E",0,0,	(APTR)MEN_EXP},
	{NM_ITEM,(CONST STRPTR)"View Header",				   0,0,0,	(APTR)MEN_VIEWHEADER},
	{NM_ITEM,(CONST STRPTR)"Move Message...",			(CONST STRPTR)"M",0,0,	(APTR)MEN_MOVE},
	{NM_ITEM,(CONST STRPTR)"Copy Message...",			(CONST STRPTR)"C",0,0,	(APTR)MEN_COPY},
	{NM_ITEM,(CONST STRPTR)"Add to Killfile...",		0,0,0,	(APTR)MEN_KILLMESS},
	{NM_ITEM,(CONST STRPTR)"Export Address to YAM",	0,0,0,	(APTR)MEN_YAMADDRESS},
	{NM_ITEM,NM_BARLABEL,					0,0,0,	(APTR)0},
	{NM_ITEM,(CONST STRPTR)"Post New Message",			(CONST STRPTR)"P",0,0,	(APTR)MEN_POST},
	{NM_ITEM,(CONST STRPTR)"Followup Message...",		(CONST STRPTR)"F",0,0,	(APTR)MEN_FOLLOWUP},
	{NM_ITEM,(CONST STRPTR)"Reply via Email...",		(CONST STRPTR)"R",0,0,	(APTR)MEN_REPLY},
	{NM_ITEM,(CONST STRPTR)"Followup and Reply...",	(CONST STRPTR)"B",0,0,	(APTR)MEN_BOTH},
	{NM_ITEM,NM_BARLABEL,					0,0,0,	(APTR)0},
	{NM_ITEM,(CONST STRPTR)"Permanently Delete",		0,0,0,	(APTR)MEN_PD},
	{NM_SUB,(CONST STRPTR)"Selected Messages",			(CONST STRPTR)".",0,0,	(APTR)MEN_PDTHIS},
	{NM_SUB,(CONST STRPTR)"All Read Messages",			0,0,0,	(APTR)MEN_PDREAD},
	{NM_SUB,(CONST STRPTR)"All Online Headers",			0,0,0,	(APTR)MEN_PDONLINE},
	{NM_SUB,(CONST STRPTR)"All Duplicates",				0,0,0,	(APTR)MEN_PDDUP},
	{NM_SUB,(CONST STRPTR)"All in this Group",			(CONST STRPTR)"/",0,0,	(APTR)MEN_PDALL},
	{NM_SUB,NM_BARLABEL,						0,0,0,	(APTR)0},
	{NM_SUB,(CONST STRPTR)"ALL Messages",					0,0,0,	(APTR)MEN_PDALLALL},
	{NM_ITEM,(CONST STRPTR)"Select",						0,0,0,	(APTR)MEN_SELECT},
	{NM_SUB,(CONST STRPTR)"All",								(CONST STRPTR)"A",0,0,	(APTR)MEN_SELECTALL},
	{NM_SUB,(CONST STRPTR)"None",							0,0,0,	(APTR)MEN_SELECTNONE},
	{NM_SUB,(CONST STRPTR)"Toggle",							0,0,0,	(APTR)MEN_SELECTTOGGLE},
	{NM_SUB,(CONST STRPTR)"All from this poster",		0,0,0,	(APTR)MEN_SELECTFROM},
	{NM_SUB,(CONST STRPTR)"All with this subject",		0,0,0,	(APTR)MEN_SELECTSUBJECT},
	{NM_SUB,(CONST STRPTR)"All Read Messages",			0,0,0,	(APTR)MEN_SELECTREAD},
	{NM_SUB,(CONST STRPTR)"All Unread Messages",		0,0,0,	(APTR)MEN_SELECTUNREAD},
	{NM_ITEM,(CONST STRPTR)"Mark as",						0,0,0,	(APTR)MEN_MARK},
	{NM_SUB,(CONST STRPTR)"Unread",							0,0,0,	(APTR)MEN_MARKUNREAD},
	{NM_SUB,(CONST STRPTR)"Read",							0,0,0,	(APTR)MEN_MARKREAD},
	{NM_SUB,(CONST STRPTR)"Held",							0,0,0,	(APTR)MEN_MARKHELD},
	{NM_SUB,(CONST STRPTR)"Queued",							0,0,0,	(APTR)MEN_MARKQUEUED},
	{NM_SUB,(CONST STRPTR)"Normal",							0,0,0,	(APTR)MEN_MARKNORMAL},
	{NM_SUB,(CONST STRPTR)"Important",						0,0,0,	(APTR)MEN_MARKIMPORTANT},
	{NM_ITEM,(CONST STRPTR)"Change Date To Current",	0,0,0,	(APTR)MEN_CURRENTDATE},
	{NM_ITEM,(CONST STRPTR)"Download Body",				0,0,0,	(APTR)MEN_DOWNLOADBODY},
	{NM_ITEM,(CONST STRPTR)"Join Messages...",			0,0,0,	(APTR)MEN_JOINMSGS},

	{NM_TITLE,(CONST STRPTR)"Preferences",				0,0,0,	(APTR)MEN_PREFS},
	{NM_ITEM,(CONST STRPTR)"Program Settings...",		0,0,0,	(APTR)MEN_ACCOUNT},
	{NM_ITEM,(CONST STRPTR)"KillFile...",					0,0,0,	(APTR)MEN_KILLFILE},
	{NM_ITEM,(CONST STRPTR)"Users...",						0,0,0,	(APTR)MEN_USERS},
	{NM_ITEM,(CONST STRPTR)"Change Password...",		0,0,0,	(APTR)MEN_CPWD},
	{NM_ITEM,(CONST STRPTR)"MUI Settings...",			0,0,0,	(APTR)MEN_MUIPREFS},

	{NM_END,NULL,0,0,0,(APTR)0},
};

Object *menustrip = NULL;
Object *menuitem_GETNEWS = NULL;
Object *menuitem_GETNEWSSINGLE = NULL;
Object *menuitem_SENDNEWS = NULL;
Object *menuitem_GETGROUPSDEF = NULL;
Object *menuitem_GETNEWGROUPSDEF = NULL;
Object *menuitem_DISCONNECT = NULL;
Object *menuitem_VIEWGROUPS = NULL;
Object *menuitem_VIEWMESSAGES = NULL;
Object *menuitem_UPDATEGROUPS = NULL;
Object *menuitem_UPDATEINDEX = NULL;
Object *menuitem_UPDATEALLIND = NULL;
Object *menuitem_IMPORT = NULL;
Object *menuitem_READ = NULL;
Object *menuitem_EDIT = NULL;
Object *menuitem_SUPER = NULL;
Object *menuitem_CANCEL = NULL;
Object *menuitem_DELMESS = NULL;
Object *menuitem_UNDELMESS = NULL;
Object *menuitem_EXP = NULL;
Object *menuitem_MOVE = NULL;
Object *menuitem_COPY = NULL;
Object *menuitem_KILLMESS = NULL;
Object *menuitem_YAMADDRESS = NULL;
Object *menuitem_POST = NULL;
Object *menuitem_FOLLOWUP = NULL;
Object *menuitem_REPLY = NULL;
Object *menuitem_BOTH = NULL;
Object *menuitem_PDTHIS = NULL;
Object *menuitem_MARK = NULL;
Object *menuitem_CURRENTDATE = NULL;
Object *menuitem_USERS = NULL;

Object *app = NULL;

Object *grouplistGroup = NULL;
Object *grouplistGroup_NLIST = NULL;
Object *grouplistGroup_LISTTREE = NULL;
MUI_NListtree_TreeNode *tn_folders = NULL,*tn_newsgroups = NULL;
Object *messageGroup = NULL;
Object *messageGroup_NLIST = NULL;
Object *messageGroup_LISTTREE = NULL;

Object *wnd_main = NULL;
Object *wnd_newnewsgroup = NULL;
Object *wnd_editng = NULL;
Object *wnd_ngadv = NULL;
Object *wnd_joinmsgs = NULL;
Object *wnd_account = NULL;
Object *wnd_servers = NULL;
Object *wnd_killfile = NULL;
Object *wnd_newserver = NULL;
Object *pages_account = NULL;
Object *wnd_groupman = NULL;
Object *wnd_search = NULL;
Object *wnd_stats = NULL;
Object *wnd_newuser = NULL;
Object *wnd_users = NULL;
Object *wnd_login = NULL;
Object *wnd_cpwd = NULL;
Object *wnd_about = NULL;
Object *aboutwin=NULL;

Object *NLIST_groupdata = NULL;
Object *LISTTREE_groupdata = NULL;
Object *LISTVIEW_groupdata = NULL;
Object *NLIST_messagelistdata = NULL;
Object *NLISTVIEW_messagelistdata = NULL;
Object *LISTTREE_messagelistdata = NULL;
Object *LISTTREEVIEW_messagelistdata = NULL;
Object *NLIST_groupman = NULL;
Object *NLIST_choice = NULL;

Object *BT_about_OKAY = NULL;

Object *TB_main = NULL;

Object *BT_nng_OKAY = NULL;
Object *BT_nng_CANCEL = NULL;
Object *STR_nng_NAME = NULL;
Object *STR_nng_DESC = NULL;
Object *CY_nng_DEFSIG = NULL;
Object *CM_nng_S = NULL;
Object *CY_nng_MAXDL = NULL;
Object *STR_nng_MAXDL = NULL;
Object *CY_nng_MAXL = NULL;
Object *STR_nng_MAXL = NULL;
Object *CY_nng_MAXAGE = NULL;
Object *STR_nng_MAXAGE = NULL;
Object *CY_nng_OFFLINE = NULL;
Object *CY_nng_NEWSSERVER = NULL;

Object *BT_eng_OKAY;
Object *BT_eng_CANCEL;
Object *STR_eng_NAME;
Object *STR_eng_DESC;
Object *CY_eng_DEFSIG;
Object *CM_eng_S;
Object *CY_eng_MAXDL;
Object *STR_eng_MAXDL;
Object *CY_eng_MAXL;
Object *STR_eng_MAXL;
Object *CY_eng_MAXAGE;
Object *STR_eng_MAXAGE;
Object *CY_eng_OFFLINE;
Object *CY_eng_NEWSSERVER;
Object *GROUP_eng;
Object *TXT_eng_LASTDL;
Object *TXT_eng_FOLDER;

Object *BT_ngadv_OKAY;
Object *BT_ngadv_CANCEL;
Object *CM_ngadv_APPVHD;
Object *STR_ngadv_APPVHD;
Object *CM_ngadv_ALTNAME;
Object *STR_ngadv_ALTNAME;
Object *CM_ngadv_ALTEMAIL;
Object *STR_ngadv_ALTEMAIL;
Object *CM_ngadv_SERVER;
Object *CY_ngadv_SERVER;
Object *CM_ngadv_SERVERPOST;
Object *GROUP_ngadv=NULL;
Object *GROUP_ngadv_SUB=NULL;

Object *BT_joinmsgs_JOIN = NULL;
Object *BT_joinmsgs_CANCEL = NULL;
Object *NLIST_joinmsgs_MSGS = NULL;
Object *STR_joinmsgs_SUBJECT = NULL;

Object *BT_acc_OKAY;
Object *BT_acc_CANCEL;
Object *STR_acc_NAME;
Object *STR_acc_EMAIL;
Object *STR_acc_REALEMAIL;
Object *STR_acc_ORG;
Object *STR_acc_SMTP;
Object *STR_acc_DOMAIN;
Object *CY_acc_TIMEZONE;
Object *CM_acc_USELOCALETZ;
Object *CM_acc_BST;
Object *CY_acc_DATEFORMAT;
Object *CM_acc_LOGGING;
Object *CM_acc_LOGDEL;
Object *CM_acc_VGROUPDL;
Object *CM_acc_NOCONFIRMDEL;
Object *CM_acc_DELONLINE;
Object *CM_acc_CONFIRMQUIT;
Object *CM_acc_QUIETDL;
Object *CM_acc_CHECKFORDUPS;
Object *CM_acc_XNEWS;
Object *STR_acc_NNTP;
Object *STR_acc_PORT;
Object *CM_acc_AUTH;
Object *STR_acc_USER;
Object *STR_acc_PASS;
Object *NLIST_acc_SERVERS;
Object *BT_acc_ADDSERVER;
Object *BT_acc_EDITSERVER;
Object *BT_acc_MAKEDEFSERVER;
Object *BT_acc_MAKEPOSTSERVER;
Object *BT_acc_DELETESERVER;
Object *BT_acc_GETGROUPS;
Object *BT_acc_GETNEWGROUPS;
Object *BT_acc_GROUPMAN;
Object *BT_server_OKAY;
Object *BT_server_CANCEL;
Object *STR_acc_NNTPPOST;
Object *STR_acc_FOLLOWUPTEXT;
Object *POP_acc_CHARSETWRITE;
Object *STR_acc_CHARSETWRITE;
Object *LIST_acc_CHARSETWRITE;
Object *CY_acc_SIG;
//Object *CM_acc_USESIG;
Object *BT_acc_READSIG;
Object *ED_acc_SIG;
Object *SLD_acc_SIG;
Object *STR_acc_LINELENGTH;
Object *CM_acc_REWRAP;
Object *CM_acc_SNIPSIG;
Object *CY_acc_XNO;
Object *NLIST_acc_KILLFILE;
Object *BT_acc_NEWKILLADD;
Object *STR_acc_NEWKILLHEAD;
Object *STR_acc_NEWKILLTEXT;
Object *STR_acc_NEWKILLGROUP;
Object *BT_acc_NEWKILL;
Object *BT_acc_DELKILL;
Object *BT_acc_DUPKILL;
Object *CM_acc_SIZEKILL;
Object *CY_acc_EXPIREKILL;
Object *STR_acc_EXPIREKILL;
//Object *CY_acc_TYPEKILL;
Object *CY_acc_MATCHKILL;
Object *CY_acc_ACTIONKILL;
Object *CM_acc_SKIPKILL;
Object *CY_wiz_EXPIREKILL;
Object *STR_wiz_EXPIREKILL;
Object *CM_acc_LISTFLAGS;
Object *CM_acc_LISTSUBJECT;
Object *CM_acc_LISTDATE;
Object *CM_acc_LISTFROMGROUP;
Object *CM_acc_LISTSIZE;
Object *CM_acc_LISTLINES;
Object *NLIST_acc_MIMEPREFS;
Object *POP_acc_MIME;
Object *LIST_acc_MIME;
Object *STR_acc_MIME;
Object *POP_acc_MIMEVIEW;
Object *STR_acc_MIMEVIEW;
Object *POP_acc_MIMEDEF;
Object *STR_acc_MIMEDEF;
Object *BT_acc_MIMENEW;
Object *BT_acc_MIMEDEL;
Object *CY_acc_READHEADER;
Object *STR_acc_READHEADER;
Object *PEN_acc_TEXT_QUOTE2;
Object *PEN_acc_TEXT_COL;
Object *CM_acc_MULTIPLEVWS;

Object *STR_groupman_FIND;
Object *BT_groupman_SUB;
Object *BT_groupman_GET;
Object *NLIST_search_NG;
Object *BT_search_START;
Object *CY_search_WHERE;
Object *STR_search_HEAD;
Object *STR_search_WHAT;
Object *CM_search_CASESENS;
Object *NLIST_search_RES;

Object *NLIST_stats_NG;
Object *BT_stats_START;
Object *CY_stats_WHAT;
Object *NLIST_stats_RES;

Object *TXT_newuser_INFO;
Object *BT_newuser_OKAY;
Object *BT_newuser_CANCEL;
Object *STR_newuser_USER;
Object *STR_newuser_PASS;
Object *CM_newuser_PASS;
Object *STR_newuser_DATALOC;
Object *POP_newuser_DATALOC;
Object *CM_newuser_SUP;
Object *CM_newuser_COPYPREFS;

Object *NLIST_users_LIST;
Object *BT_users_NEW;
Object *BT_users_DELETE;

Object *BT_login_OKAY;
Object *BT_login_CANCEL;
Object *STR_login_USER;
Object *STR_login_PASS;

Object *TXT_cpwd_INFO;
Object *BT_cpwd_OKAY;
Object *BT_cpwd_CANCEL;
Object *STR_cpwd_PASS;
Object *CM_cpwd_PASS;

/* If this is changed, the enum TB_Index should be updated appropriately.
 */
static struct MUIS_TheBar_Button TLBbuttons[] =
{
   	 {0, 0, 	(CONST STRPTR)"_Read", 		(CONST STRPTR)"Read the selected message", 0, 0, NULL, NULL},
     {1, 1, 	(CONST STRPTR)"E_xport", 	(CONST STRPTR)"View or Save parts of the\nselected message\n(attachments)", 0, 0, NULL, NULL},
     {2, 2, 	(CONST STRPTR)"_Move", 		(CONST STRPTR)"Move the selected message(s)\nto another folder", 0, 0, NULL, NULL},
     {3, 3, 	(CONST STRPTR)"_Copy", 		(CONST STRPTR)"Copy the selected message(s)\nto another folder", 0, 0, NULL, NULL},
     {4, 4, 	(CONST STRPTR)"_Delete", 	(CONST STRPTR)"Delete the selected\nmessage(s)", 0, 0, NULL, NULL},
     {5, 5, 	(CONST STRPTR)"_Kill", 		(CONST STRPTR)"Add a new killfile based on\nthe selected message", 0, 0, NULL, NULL},
     {MUIV_TheBar_BarSpacer, 6, NULL, NULL, 0, 0, NULL, NULL},
     {6, 7, 	(CONST STRPTR)"_Post", 		(CONST STRPTR)"Post a new message", 0, 0, NULL, NULL},
     {7, 8, 	(CONST STRPTR)"_Followup", 	(CONST STRPTR)"Followup the selected\nmessage (to the newsgroup)", 0, 0, NULL, NULL},
     {8, 9, 	(CONST STRPTR)"_Edit", 		(CONST STRPTR)"Edit the selected message", 0, 0, NULL, NULL},
     {MUIV_TheBar_BarSpacer, 10, NULL, NULL, 0, 0, NULL, NULL},
     {9, 11, 	(CONST STRPTR)"Fetch", 		(CONST STRPTR)"Fetch news", 0, 0, NULL, NULL},
     {10, 12, 	(CONST STRPTR)"Send", 		(CONST STRPTR)"Send news", 0, 0, NULL, NULL},
     {MUIV_TheBar_BarSpacer, 13, NULL, NULL, 0, 0, NULL, NULL},
     {11, 14, 	(CONST STRPTR)"Search", 	(CONST STRPTR)"Search for a message", 0, 0, NULL, NULL},
     {12, 15, 	(CONST STRPTR)"Settings", 	(CONST STRPTR)"Program settings", 0, 0, NULL, NULL},
     {13, 16, 	(CONST STRPTR)"Killfile", 	(CONST STRPTR)"View and edit the killfile", 0, 0, NULL, NULL},
	 {MUIV_TheBar_End, 0, NULL, NULL, 0, 0, NULL, NULL},
 };
/* Pages on the Program Settings window (wnd_acc)
 */
static const char *Pages_acc[]   = {"Accounts","Servers","Write","Read","Lists","MIME",NULL };
const int ACCSIGPAGE=2; // must be the Write page !

/* The Date Format cycle gadget under Program Settings
 */
static const char *CYA_acc_dateformat[] = {
	"DD-MMM-YY","DD-MM-YY",NULL
};

/* The Signatures cycle gadget under Program Settings
 */
static const char *CYA_acc_sigs[] = {
	"Signature 1","Signature 2","Signature 3","Signature 4","Signature 5","Signature 6","Signature 7","Signature 8",NULL
};

/* The cycle gadget for Headers to Display under Program Settings
 */
static const char *CYA_acc_readheader[] = {
	"None","Selected","Full",NULL
};

/* The Discard cycle gadget under New Newsgroup
 */
static const char *CYA_nng_maxage[] = {
	"Don't Discard Messages","Discard After x Days:",NULL
};

/* The Offline/Online cycle gadget under New Newsgroup
 */
static const char *CYA_nng_offline[] = {
	"Offline Reading","Online Reading",NULL
};

/* The newsservers cycle gadget under New Newsgroup
 */
static const char *CYA_nng_newsservers[] = {
	"Newsserver 1","Newsserver 2","Newsserver 3","Newsserver 4","Newsserver 5","Newsserver 6","Newsserver 7","Newsserver 8",NULL
};

/* The timezone cycle gadget under New Newsgroup
 */
static const char *CYA_acc_timezone[] = {
	"GMT-12","GMT-11","GMT-10","GMT-9","GMT-8 (PDT)","GMT-7 (MDT)","GMT-6 (CMT)","GMT-5 (EST)","GMT-4 (ADT)","GMT-3","GMT-2","GMT-1",
	"GMT (GMT)","GMT+1 (MET)","GMT+2 (EET)","GMT+3","GMT+4","GMT+5","GMT+6","GMT+7","GMT+8 (PST)","GMT+9","GMT+10","GMT+11","GMT+12",NULL
};

/* The x-no-archive cycle gadget under New Newsgroup
 */
static const char *CYA_acc_xno[] = {
	"Never","Always","Only if in followed up to post",NULL
};

/* The Expire cycle gadget under the KillFile window (wnd_killfile)
 */
static const char *CYA_acc_expirekill[] = {
	"Never Expire","Expire x days from creation:","Expire x days from last usage:",NULL
};

/* The Match cycle gadget under the KillFile window (wnd_killfile)
 */
static const char *CYA_acc_matchkill[] = {
	"Contains", "Does Not Contain", NULL
};

/* The Action cycle gadget under the KillFile window (wnd_killfile)
 */
static const char *CYA_acc_actionkill[] = {
	"Kill", "Mark As Important", NULL
};

/* The Maximum Number to Download cycle gadget under New Newsgroup
 */
static const char *CYA_nng_maxdl[] = {
	"Unlimited Messages per Download","Maximum # of Messages to Download:",NULL
};

/* The Maximum Length cycle gadget under New Newsgroup
 */
static const char *CYA_nng_maxl[] = {
	"Download messages of any length","Skip messages longer than this # of lines:",NULL
};

/* The Where cycle gadget under the Search window (wnd_search)
 */
static const char *CYA_search_where[] = {
	"From","Newsgroups","Subject","MessageID","Header", "Message Body", "Entire Message",NULL
};
enum {
	SEARCH_FROM = 0, SEARCH_NEWSGROUPS = 1, SEARCH_SUBJECT = 2, SEARCH_MESSAGEID = 3, SEARCH_HEADER = 4, SEARCH_BODY = 5, SEARCH_ALL = 6
};

/* The What cycle gadget under the Stats window (wnd_stats)
 */
static const char *CYA_stats_what[] = {
	"By Person","By Subject","By X-Newsreader",NULL
};

/* The Default Signature cycle gadget under the New Newsgroup window (wnd_nng)
 */
const char *CYA_nng_sigs[] = {
	"None","Signature 1","Signature 2","Signature 3","Signature 4","Signature 5","Signature 6","Signature 7","Signature 8",NULL
};


/* For the newsserver cycle gadget (allocated dynamically) under the Newsgroup Advanced Settings window (wnd_ngadv)
 */
char **CYA_ngadv_server = NULL;
int   *server_list      = NULL;

HOOK2( LONG, StrObjFunc, Object *, pop, a2, Object *, str, a1 )
	char *x = NULL,*s = NULL;
	int i = 0;

	get(str,MUIA_String_Contents,&s);

	for (i=0;;i++) {
		DoMethod(pop,MUIM_List_GetEntry,i,&x);
		if (!x) {
			set(pop,MUIA_List_Active,MUIV_List_Active_Off);
			break;
		}
		else if (!stricmp(x,s)) {
			set(pop,MUIA_List_Active,i);
			break;
		}
	}
	return(TRUE);
}

HOOK2( LONG, StrObj2Func, Object *, pop, a2, Object *, str, a1 )
	BPTR DirLock,PrevDir;
	LONG res;
	int i;
	char *x,*s;
	struct AnchorPath *apath;

	DoMethod(pop,MUIM_List_Clear);
// STORM C does not have __aligned keyword so we have to allocate struct AnchorPath dynamically
#ifdef __amigaos4__ //AmigaOs4 dos 50.76+
	struct TagItem endtag = {TAG_DONE,0};
	apath = (struct AnchorPath*) AllocDosObject(DOS_ANCHORPATH, &endtag);
#else // pre dos 50.76+
	apath=(struct AnchorPath *)AllocMem(sizeof(struct AnchorPath),MEMF_CLEAR);
#endif
	if (apath)
	{
		DoMethod(pop,MUIM_List_Insert,POPA_Builtin_Tables,-1,MUIV_List_Insert_Sorted);
		DirLock=Lock("PROGDIR:Charsets",ACCESS_READ);
		if (DirLock)
		{
			PrevDir=CurrentDir(DirLock);
			res=MatchFirst("#?.charset",apath);
			while (!res)
			{
				if (apath->ap_Info.fib_Size == 512)
				{
					x=apath->ap_Info.fib_FileName;
					x[strlen(x)-8]='\0';
					for (i=0;POPA_Builtin_Tables[i];i++)
						if (!stricmp(POPA_Builtin_Tables[i],x))
							goto IsBuiltin;
					DoMethod(pop,MUIM_List_InsertSingle,x,MUIV_List_Insert_Sorted);
				}
IsBuiltin:
				res=MatchNext(apath);
			}
			MatchEnd(apath);
			CurrentDir(PrevDir);
			UnLock(DirLock);
		}
#ifdef __amigaos4__ //AmigaOs4 dos 50.76+
	FreeDosObject(DOS_ANCHORPATH, apath);
#else // pre dos 50.76+
		FreeMem(apath,sizeof(struct AnchorPath));
#endif
		get(str,MUIA_Text_Contents,&s);
		for (i=0;;i++) {
			DoMethod(pop,MUIM_List_GetEntry,i,&x);
			if (!x) {
				set(pop,MUIA_List_Active,MUIV_List_Active_Off);
				break;
			}
			else if (!stricmp(x,s)) {
				set(pop,MUIA_List_Active,i);
				break;
			}
		}
		return(TRUE);
	}
	else
		return(FALSE);
}

HOOK2( VOID, ObjStrAccFunc, Object *, pop, a2, Object *, str, a1 )
	char *x = NULL;
	DoMethod(pop,MUIM_List_GetEntry,MUIV_List_GetEntry_Active,&x);
	set(str,MUIA_String_Contents,x);
	int entries = 0;
	MIMEPrefs * mime = NULL;
	get(NLIST_acc_MIMEPREFS,MUIA_NList_Entries,&entries);
	if(entries>0) {
		int val = 0;
		get(NLIST_acc_MIMEPREFS,MUIA_NList_Active,&val);
		if(val!=MUIV_NList_Active_Off) {
			DoMethod(NLIST_acc_MIMEPREFS,MUIM_NList_GetEntry,MUIV_NList_GetEntry_Active,&mime);
			strncpy(mime->type,x,MIMEPREFS_TYPE_LEN);
			mime->type[MIMEPREFS_TYPE_LEN] = '\0';
			DoMethod(NLIST_acc_MIMEPREFS,MUIM_NList_Redraw,MUIV_NList_Redraw_Active);
		}
	}
}

HOOK2( VOID, ObjStrAcc2Func, Object *, pop, a2, Object *, str, a1 )
	char *x = NULL;
	DoMethod(pop,MUIM_List_GetEntry,MUIV_List_GetEntry_Active,&x);
	set(str,MUIA_String_Contents,x);
	int entries = 0;
	MIMEPrefs * mime = NULL;
	MIMEPrefs * mime2 = new MIMEPrefs;
	get(NLIST_acc_MIMEPREFS,MUIA_NList_Entries,&entries);
	if(entries>0) {
		int val = 0;
		get(NLIST_acc_MIMEPREFS,MUIA_NList_Active,&val);
		if(val!=MUIV_NList_Active_Off) {
			DoMethod(NLIST_acc_MIMEPREFS,MUIM_NList_GetEntry,MUIV_NList_GetEntry_Active,&mime);
			*mime2=*mime;
			strncpy(mime2->viewer,x,MIMEPREFS_VIEWER_LEN);
			mime2->viewer[MIMEPREFS_VIEWER_LEN] = '\0';
			DoMethod(NLIST_acc_MIMEPREFS,MUIM_NList_ReplaceSingle,mime2,MUIV_NList_Insert_Active,NOWRAP,ALIGN_LEFT);
			DoMethod(NLIST_acc_MIMEPREFS,MUIM_NList_Redraw,MUIV_NList_Redraw_Active);
		}
	}
}

HOOK2( VOID, ObjStrAcc3Func, Object *, pop, a2, Object *, str, a1 )
	char *x = NULL;
	DoMethod(pop,MUIM_List_GetEntry,MUIV_List_GetEntry_Active,&x);
	set(str,MUIA_String_Contents,x);
}

HOOK2( VOID, ObjStrAcc4Func, Object *, pop, a2, Object *, str, a1 )
	char *x = NULL;
	DoMethod(pop,MUIM_List_GetEntry,MUIV_List_GetEntry_Active,&x);
	set(str,MUIA_Text_Contents,x);
}

HOOK2( VOID, ObjStrNewuserFunc, Object *, pop, a2, Object *, str, a1 )
	char *x;
	DoMethod(pop,MUIM_List_GetEntry,MUIV_List_GetEntry_Active,&x);
	set(str,MUIA_String_Contents,x);
}

HOOK2( VOID, WindowFunc, Object *, pop, a2, Object *, win, a1 )
	set(win,MUIA_Window_DefaultObject,pop);
}


MUI_CustomClass *editor_mcc = NULL;
int cy_current_sig=0;
int sorttype_search=0;
int sorttypedir_search=1; //1=down,-1=up
int sorttype_stats=0; // 0=value, 1=freq
int sorttypedir_stats=1; //1=down,-1=up
int stats_what=0;
/*int t_abort=FALSE;
int t_save=FALSE;
int t_disp=FALSE;*/
int prefsversion=4;
/* version history:
	4 : 1.30
	*/
BOOL FORCEPATCH1=FALSE;
BOOL accountpage=FALSE;
BOOL delete_dis=FALSE;
//BOOL downloading_headers=FALSE;
int newuseradded=0; // 0=not yet,1=yes,2=cancelled
ULONG u_signal = 0;
BOOL running = TRUE;
BOOL guistarted = FALSE;

int nextID=0;
int nextServerID=1;

short Account::SNIPSIG = 1;
short Account::LOGGING = 2;
short Account::LOGDEL = 4;
short Account::NODELONLINE = 8;
short Account::QUIETDL = 16;
short Account::NOCONFIRMQUIT = 32;
short Account::CHECKFORDUPS = 64;

short Account::LISTUPDATE = 1;
short Account::LISTFLAGS = 2;
short Account::LISTSUBJECT = 4;
short Account::LISTDATE = 8;
short Account::LISTFROMGROUP = 16;
short Account::LISTSIZE = 32;
short Account::LISTLINES = 64;

BOOL nlisttree = TRUE;
Account account;
char *sigs[NSIGS];
int nng_defserver = 0;
Server *groupman_server = NULL;
GroupData * ng_edit=NULL;
GroupData * ng_editadv=NULL;
GroupData *old_gdata=NULL;
GroupData *c_gdata=NULL; // currently being displayed
GroupData *v_gdata=NULL; // set this to the gdata we wish to view (for read_index())
int gdata_readID=-100;
Server *editServer=NULL; // server currently being edited (NULL if none)

library ASLlib((CONST STRPTR)"asl.library",0); // constructor opens libraries
library Localelib((CONST STRPTR)"locale.library",0); // constructor opens libraries


char tempStatusText[256]="";
const int TAB=4;

LONG xget(Object *obj,ULONG attribute) {
	LONG x;
	get(obj,attribute,&x);
	return(x);
}

char * getstr(Object *obj) {
	return( (char *)xget(obj,MUIA_String_Contents));
}

char * gettxt(Object *obj) {
	return( (char *)xget(obj,MUIA_Text_Contents));
}

struct Library *AslBase			= NULL;
struct Library *LocaleBase		= NULL;
struct Library *IntuitionBase	= NULL;
struct Library *SocketBase		= NULL;

struct AslIFace			*IAsl		= NULL;
struct LocaleIFace		*ILocale	= NULL;
struct IntuitionIFace	*IIntuition = NULL;
struct SocketIFace		*ISocket 	= NULL;

// library constructor - opens an AmigaOS Library

library::library(char * name,int version) {
	if((Base = OpenLibrary(name,version)) == NULL)
		printf("Error - Cannot open library!\n");
}

// library destructor - closes the Library

library::~library() {
	if(Base) {
		CloseLibrary(Base);
		Base=NULL;
	}
}

/* KillFile constructor - sets the timestamps to now
 */
KillFile::KillFile() {
	nLog("KillFile() constructor called\n");
	memset(this,0,sizeof(KillFile));
	DateStamp(&ds);
	DateStamp(&lastused);
	expiretype = 0;
	expire = 0;
	//type = 0;
	match = 0;
	action = 0;
	carryon = 0;
	*header = '\0';
	*text = '\0';
	*ngroups = '\0';
}

/* Gets the current timezone offset from GMT, either as chosen by the user,
 * or uses the Locale library if he prefers
 */
int getGMTOffset() {
	if(account.uselocaletz) {
		Locale * locale = OpenLocale(NULL);
		if(locale!=0) {
			int ttz=-(int)(locale->loc_GMTOffset/60);
			return ttz;
		}
	}
	return account.timezone;
}

/* Makes sure certain buttons and menu items are enabled or disabled as
 * necessary.
 * Also sets up the 'bubble help' for the message list.
 */
void setEnabled() {
	nLog("setEnabled() called\n");
	int active = 0;
	char *translated_string=NULL;
	BOOL s,s_edit,s_super,s_cancel,s_undel,s_kill,s_changedate,s_gns;
	getMdataActivePos(&active);
	if(active==MUIV_NList_Active_Off)
		s=TRUE;
	else
		s=FALSE;
	// turn off those for irrelevant folders
	GroupData * gdata = NULL;
	getGdataDisplayed(&gdata);
	//printf("***%d\n",gdata->ID);
	s_edit = s;
	if(gdata->ID!=-1 && gdata->ID!=-2)
		s_edit = TRUE;
	s_super = s;
	s_cancel = s;
	if(gdata->ID!=-2) {
		s_super = TRUE;
		s_cancel = TRUE;
	}
	s_undel = s;
	if(gdata->ID!=-3)
		s_undel = TRUE;
	s_kill = s;
	if(gdata->ID<0)
		s_kill = TRUE;
	s_gns = gdata->ID < 0;
	s_changedate = s;
	if(gdata->ID != -1)
		s_changedate = TRUE;

	DoMethod(TB_main,MUIM_TheBar_SetAttr,TB_READ,		MUIV_TheBar_ButtonFlag_Disabled,s);
	DoMethod(TB_main,MUIM_TheBar_SetAttr,TB_EXPORT,		MUIV_TheBar_ButtonFlag_Disabled,s);
	DoMethod(TB_main,MUIM_TheBar_SetAttr,TB_MOVE,		MUIV_TheBar_ButtonFlag_Disabled,s);
	DoMethod(TB_main,MUIM_TheBar_SetAttr,TB_COPY,		MUIV_TheBar_ButtonFlag_Disabled,s);
	DoMethod(TB_main,MUIM_TheBar_SetAttr,TB_DELETE,		MUIV_TheBar_ButtonFlag_Disabled,s);
	DoMethod(TB_main,MUIM_TheBar_SetAttr,TB_KILL,		MUIV_TheBar_ButtonFlag_Disabled,s_kill);
	DoMethod(TB_main,MUIM_TheBar_SetAttr,TB_POST,		MUIV_TheBar_ButtonFlag_Disabled,FALSE);
	DoMethod(TB_main,MUIM_TheBar_SetAttr,TB_FOLLOWUP,	MUIV_TheBar_ButtonFlag_Disabled,s);
	DoMethod(TB_main,MUIM_TheBar_SetAttr,TB_EDIT,		MUIV_TheBar_ButtonFlag_Disabled,s_edit);
	s=!s;
	s_edit = !s_edit;
	s_super = !s_super;
	s_cancel = !s_cancel;
	s_undel = !s_undel;
	s_kill = !s_kill;
	s_gns = !s_gns;
	s_changedate = !s_changedate;
	set(menuitem_GETNEWSSINGLE,MUIA_Menuitem_Enabled,s_gns);
	set(menuitem_READ,MUIA_Menuitem_Enabled,s);
	set(menuitem_EDIT,MUIA_Menuitem_Enabled,s_edit);
	set(menuitem_SUPER,MUIA_Menuitem_Enabled,s_super);
	set(menuitem_CANCEL,MUIA_Menuitem_Enabled,s_cancel);
	set(menuitem_DELMESS,MUIA_Menuitem_Enabled,s);
	set(menuitem_UNDELMESS,MUIA_Menuitem_Enabled,s_undel);
	set(menuitem_EXP,MUIA_Menuitem_Enabled,s);
	set(menuitem_MOVE,MUIA_Menuitem_Enabled,s);
	set(menuitem_COPY,MUIA_Menuitem_Enabled,s);
	set(menuitem_KILLMESS,MUIA_Menuitem_Enabled,s_kill);
	set(menuitem_YAMADDRESS,MUIA_Menuitem_Enabled,s);
	set(menuitem_POST,MUIA_Menuitem_Enabled,TRUE);
	set(menuitem_FOLLOWUP,MUIA_Menuitem_Enabled,s);
	set(menuitem_REPLY,MUIA_Menuitem_Enabled,s);
	set(menuitem_BOTH,MUIA_Menuitem_Enabled,s);
	set(menuitem_PDTHIS,MUIA_Menuitem_Enabled,s);
	set(menuitem_MARK,MUIA_Menuitem_Enabled,s);
	set(menuitem_CURRENTDATE,MUIA_Menuitem_Enabled,s_changedate);

	static char helptext[4096] = "";
	MessageListData *mdata = NULL;
	getMdataActive(&mdata);
	if(mdata != 0) {
		//if(*mdata->datec == '\0')
		//	sprintf(mdata->datec,"%s %s",mdata->c_date,mdata->c_time);
		//printf(":::%d : %s\n",mdata->ID,mdata->subject);

		char filename[MAXFILENAME] = "";
		getFilePath(filename,gdata->ID,mdata->ID);

		//char newsgroups[MAXLINE] = "unknown";
		//get_newsgroups(newsgroups,gdata->ID,mdata->ID);
		//NewMessage newmess;
		//get_refs(&newmess,filename,GETREFS_NONE,TRUE);
		translated_string = translateCharset((unsigned char *)mdata->subject,NULL);
		if (translated_string)
			strcpy(mdata->subject,translated_string);

		translated_string=translateCharset((unsigned char *)mdata->from,NULL);
		if (translated_string)
			strcpy(mdata->from,translated_string);

		sprintf(helptext,
			CatalogStr(MSG_MESSAGE_HELP,MSG_MESSAGE_HELP_STR),
			mdata->from,
			//newmess.from,
			mdata->subject,
			//newmess.subject,
			mdata->c_date,
			mdata->c_time,
			mdata->newsgroups,
			//newsgroups,
			//newmess.newsgroups,
			mdata->size,
			mdata->flags[8],
			filename,
			mdata->flags[1] ? CatalogStr(MSG_UNREAD_MESSAGE,MSG_UNREAD_MESSAGE_STR) : "",
			mdata->flags[0] ? CatalogStr(MSG_HELD_MESSAGE,MSG_HELD_MESSAGE_STR) : "",
			mdata->flags[10] ? CatalogStr(MSG_REPLIED_MESSAGE,MSG_REPLIED_MESSAGE_STR) : "",
			mdata->flags[12] ? CatalogStr(MSG_BODY_NOT_DOWNLOADED,MSG_BODY_NOT_DOWNLOADED_STR) : ""
		);
	}
	else
		strcpy(helptext,CatalogStr(MSG_NO_MESSAGE_SELECTED,MSG_NO_MESSAGE_SELECTED_STR));

	set(NLIST_messagelistdata,MUIA_ShortHelp,helptext);
	if(nlisttree)
		set(LISTTREE_messagelistdata,MUIA_ShortHelp,helptext);

}

void setGdataHelpText(GroupData *gdata) {
	const char *name;
	nLog("setGdataHelpText((GroupData *)%d) called\n",gdata);
	static char helptext[4096] = "";
	if(gdata != 0) {
		char filename[MAXFILENAME] = "";
		getFolderPath(filename,gdata->ID);
		char lastdl[256] = "";
		if(gdata->ID >= 0) {
			strcpy(lastdl,CatalogStr(MSG_LAST_DOWNLOADED,MSG_LAST_DOWNLOADED_STR));
			DateHandler::get_date(&lastdl[ strlen(lastdl) ],getGMTOffset(),account.bst,gdata->lastdlds);
		}
		Server *server = (gdata->ID >= 0) ? getServer(gdata->serverID) : NULL;

		switch (gdata->ID)
		{
		case -1:
			name=CatalogStr(MSG_FOLDER_OUTGOING,MSG_FOLDER_OUTGOING_STR);
		break;
		case -2:
			name=CatalogStr(MSG_FOLDER_SENT,MSG_FOLDER_SENT_STR);
		break;
		case -3:
			name=CatalogStr(MSG_FOLDER_DELETED,MSG_FOLDER_DELETED_STR);
		break;
		default:
			name=gdata->name;
		break;
		}

		sprintf(helptext,
			CatalogStr(MSG_GROUP_HELP,MSG_GROUP_HELP_STR),
			name,
			gdata->nummess,
			gdata->num_unread,
			gdata->ID < 0 ? "" : (gdata->flags[5] ? CatalogStr(MSG_ONLINE_READING_2,MSG_ONLINE_READING_2_STR) : CatalogStr(MSG_OFFLINE_READING_2,MSG_OFFLINE_READING_2_STR)),
			server == NULL ? "" : CatalogStr(MSG_USING,MSG_USING_STR), server == NULL ? "" : server->nntp,
			gdata->ID < 0 ? "" : (gdata->s ? CatalogStr(MSG_GROUP_SUBSCRIBED,MSG_GROUP_SUBSCRIBED_STR) : ""),
			lastdl,
			filename
		);
	}
	else
		strcpy(helptext,CatalogStr(MSG_NO_GROUP_SELECTED,MSG_NO_GROUP_SELECTED_STR));

	set(NLIST_groupdata,MUIA_ShortHelp,helptext);
	if(nlisttree)
		set(LISTTREE_groupdata,MUIA_ShortHelp,helptext);
}

void sleepAll(BOOL sleep) {
	set(wnd_main,MUIA_Window_Sleep,sleep);
	ViewWindow::sleepAll(sleep);
	WriteWindow::sleepAll(sleep);
	//printf("sleep is : %d\n",sleep);
}

void delete_file(GroupData * gdata,MessageListData * mdata) {
	char filename[MAXFILENAME] = "";
	getFilePath(filename,gdata->ID,mdata->ID);
	DeleteFile(filename);
}

void delete_file_n(GroupData * gdata,MessageListData ** mdataptr,int n)
{
	char filename[MAXFILENAME] = "";
	if(gdata->ID==-1)
		sprintf(filename,"NewsCoasterData:outgoing/news_");
	else if(gdata->ID==-2)
		sprintf(filename,"NewsCoasterData:sent/news_");
	else if(gdata->ID==-3)
		sprintf(filename,"NewsCoasterData:deleted/news_");
	else if(gdata->ID>=0)
		sprintf(filename,"NewsCoasterData:folder_%d/news_",gdata->ID);
	char *ptr = &filename[ strlen(filename) ];
	for(int i=0;i<n;i++) {
		sprintf(ptr,"%d",mdataptr[i]->ID);
		DeleteFile(filename);
	}
}

BOOL changedate(GroupData *gdata,MessageListData *mdata,char *date,void *source) {
	nLog("changedate((GroupData *)%d,(MessageListData *)%d,(char *)%s,(void *)%d) called\n",gdata,mdata,date,source);
	char buffer[300] = "";
	char buffer2[300] = "";
	char line[MAXLINE] = "";
	getFilePath(buffer,gdata->ID,mdata->ID);
	sprintf(buffer2,"%s.new",buffer);
	DeleteFile(buffer2);

	BPTR file=Open(buffer,MODE_READWRITE);

	BPTR lock2=Lock(buffer2,ACCESS_WRITE);
	BPTR file2=Open(buffer2,MODE_NEWFILE);

	BOOL rtn = FALSE;

	if(file!=0 && file2!=0) {
		rtn = TRUE;
		for(;;) {
			if(!FGets(file,line,MAXLINE))
				break;
			if(strncmp(line,"Date:",5)==0) {
				char dateline[256] = "";
				sprintf(dateline,"Date: %s\n",date);
				Write(file2,dateline,strlen(dateline));
			}
			else
				Write(file2,line,strlen(line));
		}
	}
	if(file2)
		Close(file2);
	if(lock2)
		UnLock(lock2);
	if(file)
		Close(file);
	//if(lock)
	//	UnLock(lock);

	if(rtn) {
		DeleteFile(buffer);
		Rename(buffer2,buffer);
		//printf("buffer : %s\n",buffer);
		//printf("buffer2: %s\n",buffer2);
		//mdata->ID = newID;
		write_index_update(gdata,mdata,source);
	}

	return rtn;
}

void archiveGroup(GroupData *gdata,char *filename)
{
	nLog("archiveGroup((GroupData *)%d,(char *)%s) called\n",gdata,filename);
	MessageListData * mdata=NULL;
	if(gdata==0)
		return;
	int k = 0;
	int entries = 0;
	BPTR file=NULL;
	BPTR file2=NULL;
	BPTR lock2=NULL;

	char filename2[128]="NewsCoasterData:";
	int offset = strlen(filename2);
	if(gdata->ID==-1)
		sprintf(&filename2[offset],"outgoing/news_");
	else if(gdata->ID==-2)
		sprintf(&filename2[offset],"sent/news_");
	else if(gdata->ID==-3)
		sprintf(&filename2[offset],"deleted/news_");
	else if(gdata->ID>=0)
		sprintf(&filename2[offset],"folder_%d/news_",gdata->ID);
	offset = strlen(filename2);

	char *big_buffer = new char[big_bufsize_g + 1];
	char divide[1024]="";
	get(NLIST_messagelistdata,MUIA_NList_Entries,&entries);
	//archive folder
	DeleteFile(filename);
	file=Open(filename,MODE_NEWFILE);
	if(file) {
		StatusWindow *statusWindow = new StatusWindow(app,CatalogStr(MSG_ARCHIVING,MSG_ARCHIVING_STR));
		sleepAll(TRUE);
		FFlush(file);
		if(entries>0) {
			strcpy(divide,"From ???@??? ");
			for(k=0;k<entries;k++) {
				DoMethod(app,MUIM_Application_InputBuffered);
				if(k % 32 == 0) {
					do_input();
					if(statusWindow->isAborted()==TRUE || running==FALSE)
						break;
					sprintf(status_buffer_g,CatalogStr(MSG_ARCHIVING_2,MSG_ARCHIVING_2_STR),k+1,entries);
					statusWindow->setText(status_buffer_g);
				}
				DoMethod(NLIST_messagelistdata,MUIM_NList_GetEntry,k,&mdata);
				if(mdata->flags[12]) {
					// online message - skip
					continue;
				}
				sprintf(&filename2[offset],"%d",mdata->ID);
				file2=NULL;
				//lock2=Lock(filename2,ACCESS_READ);
				//if(lock2)
					file2=Open(filename2,MODE_OLDFILE);
				if(file2==0) {
					nLog("Error in archiveGroup()! - Can't open file: %s\n",filename2);
					sprintf(status_buffer_g,CatalogStr(MSG_UNABLE_TO_OPEN_FILE,MSG_UNABLE_TO_OPEN_FILE_STR),filename2);
					MUI_RequestA(app,0,0,CatalogStr(MSG_ERROR,MSG_ERROR_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),status_buffer_g,0);
					if(lock2) {
						UnLock(lock2);
						lock2=NULL;
					}
				}
				else {
					// write file to archive
					DateHandler::get_date_mailbox(&divide[13],mdata->ds);
					Write(file,divide,25 + 12);
					Write(file,(UBYTE *)"\n",1);
					int read = 0;
					/*while( (read=Read(file2,buffer,bufsize))>0 ) {
						Write(file,buffer,read);
					}*/
					while( (read=Read(file2,big_buffer,big_bufsize_g))>0 ) {
						Write(file,big_buffer,read);
					}
					Close(file2);
					file2=NULL;
					Write(file,(UBYTE *)"\n",1);
				}
			}
		}
		sleepAll(FALSE);
		delete statusWindow;
	}
	else {
		sprintf(status_buffer_g,CatalogStr(MSG_CANNOT_OPEN_FILE_TO_WRITE,MSG_CANNOT_OPEN_FILE_TO_WRITE_STR),filename);
		nLog(status_buffer_g);
		MUI_RequestA(app,0,0,CatalogStr(MSG_ERROR,MSG_ERROR_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),status_buffer_g,0);
	}
	if(file) {
		Close(file);
		file=NULL;
	}
	delete [] big_buffer;
}

int parse_index_version(char *version) {
	nLog("parse_index_version((char *)%s) called\n",version);
	version++;
	int v = atoi(version);
	nLog("  returning %d\n",v);
	return v;
}

void clearThreadView() {
	nLog("clearThreadView() called\n");
	DoMethod(LISTTREE_messagelistdata,MUIM_NListtree_Remove,MUIV_NListtree_Remove_ListNode_Root,MUIV_NListtree_Remove_TreeNode_All,0);
}

void threadView() {
	nLog("threadView() called\n");
	if(account.mdata_view!=1)
		return;
	// takes messages from NLIST_messagelistdata, and inserts into LISTTREE_messagelistdata
	int entries = 0;
	DoMethod(NLIST_messagelistdata,MUIM_NList_Sort);
	get(NLIST_messagelistdata,MUIA_NList_Entries,&entries);
	set(wnd_main,MUIA_Window_Sleep,TRUE);
	setMdataQuiet(TRUE);

	MessageListData * mdata = NULL;
	MessageListData * current_mdata = NULL;
	getMdataActive(&current_mdata);
	//printf("%d\n",current_mdata);
	if(current_mdata != 0 && get_mdataptr_pos(current_mdata) == -1)
		current_mdata = NULL; // message no longer exists in list - we don't want to reference it, in cases it's been deleted!
	//printf("-->%d\n",current_mdata);

	clearThreadView();

	if(entries>0) {
		MUI_NListtree_TreeNode ** tnptr = new MUI_NListtree_TreeNode *[entries];
		int *positions = new int[entries];
		int i=0;
		for(i=0;i<entries;i++) {
			tnptr[i] = NULL;

			DoMethod(NLIST_messagelistdata,MUIM_NList_GetEntry,i,&mdata);
			if(*mdata->lastref == '\0')
				positions[i] = -1;
			else
				get_mdata_pos(&positions[i],mdata->lastref);
		}
		int count = 0;
		int *stack = new int[entries];
		for(i=0;i<entries;i++) {
			if(tnptr[i] == NULL) {
				int pos = positions[i];
				if( pos == -1 || pos == i ) {
					// new thread
					DoMethod(NLIST_messagelistdata,MUIM_NList_GetEntry,i,&mdata);
					tnptr[i]=(MUI_NListtree_TreeNode *)DoMethod(LISTTREE_messagelistdata,MUIM_NListtree_Insert,"",mdata,MUIV_NListtree_Insert_ListNode_Root,MUIV_NListtree_Insert_PrevNode_Tail,TNF_LIST);
					count++;
				}
				else {
					int n = 0;
					stack[n++] = i;
					for(;;) {
						stack[n++] = pos;
						if(pos == -1 || tnptr[pos] != 0)
							break;

						int new_pos = positions[pos];
						if(new_pos == pos) {
							stack[n++] = pos;
							break;
						}
						pos = new_pos;
					}
					int parent = pos;
					for(int j=n-2;j>=0;j--) {
						int p = stack[j];
						if(parent == -1 || parent == p) {
							DoMethod(NLIST_messagelistdata,MUIM_NList_GetEntry,p,&mdata);
							tnptr[p]=(MUI_NListtree_TreeNode *)DoMethod(LISTTREE_messagelistdata,MUIM_NListtree_Insert,"",mdata,MUIV_NListtree_Insert_ListNode_Root,MUIV_NListtree_Insert_PrevNode_Tail,TNF_LIST);
							count++;
						}
						else {
							DoMethod(NLIST_messagelistdata,MUIM_NList_GetEntry,p,&mdata);
							tnptr[p]=(MUI_NListtree_TreeNode *)DoMethod(LISTTREE_messagelistdata,MUIM_NListtree_Insert,"",mdata,tnptr[parent],MUIV_NListtree_Insert_PrevNode_Tail,TNF_LIST);
							count++;
						}
						parent = p;
					}
				}
			}
		}
		if(count != entries) {
			sprintf(status_buffer_g,CatalogStr(MSG_COUNT_DIFFERS_FROM_ENTRIES,MSG_COUNT_DIFFERS_FROM_ENTRIES_STR),count,entries);
			MUI_RequestA(app,0,0,CatalogStr(MSG_ERROR_3,MSG_ERROR_3_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),status_buffer_g,0);
			nLog("count %d differs from entries %d !\n",count,entries);
		}
		delete [] tnptr;
		delete [] positions;
		delete [] stack;

		if(current_mdata != 0) {
			//printf("-->%d : %s\n",current_mdata->ID,current_mdata->subject);
			set_mdata(current_mdata);
		}
	}
	DoMethod(LISTTREE_messagelistdata,MUIM_NList_ColWidth,MUIV_NList_ColWidth_All,MUIV_NList_ColWidth_Default);
	setMdataQuiet(FALSE);
	set(wnd_main,MUIA_Window_Sleep,FALSE);
}

void update_screen_title(GroupData *gdata)
{
	const char *name;
	nLog("update_screen_title((GroupData *)%d) called\n",gdata);

	if(gdata->ID<0)
	{
		switch (gdata->ID)
		{
			case -1:
				name=CatalogStr(MSG_FOLDER_OUTGOING,MSG_FOLDER_OUTGOING_STR);
			break;
			case -2:
				name=CatalogStr(MSG_FOLDER_SENT,MSG_FOLDER_SENT_STR);
			break;
			case -3:
				name=CatalogStr(MSG_FOLDER_DELETED,MSG_FOLDER_DELETED_STR);
			break;
			default:
				name=gdata->name;
			break;
		}
		sprintf(scrtitle,CatalogStr(MSG_READING_TITLE,MSG_READING_TITLE_STR),base_scrtitle,&name[6],gdata->nummess,gdata->num_unread);
	}
	else
		sprintf(scrtitle,CatalogStr(MSG_READING_TITLE,MSG_READING_TITLE_STR),base_scrtitle,gdata->desc,gdata->nummess,gdata->num_unread);
	set(wnd_main,MUIA_Window_ScreenTitle,scrtitle);
}

void set_and_read(int gID) {
	nLog("set_and_read((int)%d) called\n",gID);
	GroupData * gdata2=NULL;
	getGdataDisplayed(&gdata2);
	if(gdata2->ID != gID) {
		set_gdata(gID);
		read_index();
	}
}

void set_and_read(GroupData * gdata) {
	nLog("set_and_read((GroupData *)%d) called\n",gdata);
	set_and_read(gdata->ID);
}

void set_and_read(char * name) {
	nLog("set_and_read((char *)%s) called\n",name);
	GroupData * gdata = NULL;
	getGdataDisplayed(&gdata);
	//if(gdata->name != name) {
	if(stricmp(gdata->name,name) != 0) {
		set_gdata(name);
		read_index();
	}
}

void update_groups()
{
	nLog("update_groups() called\n");
	// reads in the available groups
	GroupData * gdata = NULL;
	MessageListData * mdata = NULL;
	BPTR lock=NULL;
	char folder[256]="";
	//char filename2[256]="";
	int maxID,mgID=-1;
	sprintf(folder,"NewsCoasterData:");
	set(wnd_main,MUIA_Window_Sleep,TRUE);
	clearGdata();
	if(account.grouplistType!=0)
	{
		tn_folders=(MUI_NListtree_TreeNode *)DoMethod(LISTTREE_groupdata,MUIM_NListtree_Insert,CatalogStr(MSG_FOLDERS,MSG_FOLDERS_STR),NULL,MUIV_NListtree_Insert_ListNode_Root,MUIV_NListtree_Insert_PrevNode_Tail,TNF_LIST);
		tn_newsgroups=(MUI_NListtree_TreeNode *)DoMethod(LISTTREE_groupdata,MUIM_NListtree_Insert,CatalogStr(MSG_NEWSGROUPS,MSG_NEWSGROUPS_STR),NULL,MUIV_NListtree_Insert_ListNode_Root,MUIV_NListtree_Insert_PrevNode_Tail,TNF_LIST);
	}

	APTR context = ObtainDirContextTags(EX_StringName,folder, EX_DataFields,(EXF_NAME|EXF_TYPE|EXF_SIZE), TAG_END);
	if( context )
	{
	    struct ExamineData *dat;
		BOOL okay = FALSE;
		int entries = 0,k = 0;

	    while((dat = ExamineDir(context)))   /* until no more data.*/
		{
			okay = FALSE;
			if(STRNICMP(dat->Name,"FOLDER_",7)==0)
			{
				gdata=new GroupData();
				gdata->ID = atoi(&dat->Name[7]);
				if(gdata->ID>mgID)
					mgID=gdata->ID;
				sprintf(gdata->name,CatalogStr(MSG_UNKNOWN_GROUP,MSG_UNKNOWN_GROUP_STR),gdata->ID);
				okay=TRUE;
			}
			else if(stricmp(dat->Name,"outgoing")==0)
			{
				gdata=new GroupData();
				gdata->ID=-1;
				strcpy(gdata->name,MSG_FOLDER_OUTGOING_STR);
				okay=TRUE;
			}
			else if(stricmp(dat->Name,"sent")==0)
			{
				gdata=new GroupData();
				gdata->ID=-2;
				strcpy(gdata->name,MSG_FOLDER_SENT_STR);
				okay=TRUE;
			}
			else if(stricmp(dat->Name,"deleted")==0)
			{
				gdata=new GroupData();
				gdata->ID=-3;
				strcpy(gdata->name,MSG_FOLDER_DELETED_STR);
				okay=TRUE;
			}
			if(okay)
			{
				gdata->s=FALSE;
				gdata->defsig=-1;
				DateStamp(&gdata->lastdlds);
				gdata->max_dl=-1;
				if(account.grouplistType==0)
					DoMethod(NLIST_groupdata,MUIM_NList_InsertSingle,gdata,MUIV_NList_Insert_Sorted);
				else
				{
					if(gdata->ID<0)
						DoMethod(LISTTREE_groupdata,MUIM_NListtree_Insert,"",gdata,tn_folders,MUIV_NListtree_Insert_PrevNode_Tail,TNF_LIST);
					else
						DoMethod(LISTTREE_groupdata,MUIM_NListtree_Insert,"",gdata,tn_newsgroups,MUIV_NListtree_Insert_PrevNode_Tail,TNF_LIST);
				}
				set_and_read(gdata);
				get(NLIST_messagelistdata,MUIA_NList_Entries,&entries);
				maxID=-1;
				for(k=0;k<entries;k++)
				{
					DoMethod(NLIST_messagelistdata,MUIM_NList_GetEntry,k,&mdata);
					if(mdata->ID>maxID)
						maxID=mdata->ID;
				}
				gdata->nextmID=maxID+1;
				redrawGdataAll();
			}
		}
		ReleaseDirContext(context);
	}
	nextID=mgID+1;
	resetGdataActive();
	set(wnd_main,MUIA_Window_Sleep,FALSE);
	account.save_data();
	return;
}

void get_newsgroups(char *newsgroups,int gID,int mID) {

	char filename[MAXFILENAME] = "";
	getFilePath(filename,gID,mID);

	//nLog("get_newsgroups((NewMessage *)%d,(char *)%s,(BOOL)%d,(BOOL)%d) called\n",newmess,filename,getRefs,trimRefs);
	BPTR file=NULL;
	BPTR lock=NULL;
	lock=Lock(filename,ACCESS_READ);
	file=Open(filename,MODE_OLDFILE);
	if(file) {

		char buffer[MAXLINE] = "";

		for(;;) {
			if(!FGets(file,buffer,MAXLINE))
				break;


			if( *buffer=='\0' || *buffer=='\n' || (*buffer=='\r' && buffer[1]=='\n') )
				break;

			if(STRNICMP(buffer,"NEWSGROUPS: ",12)==0) {
				StripNewLine(&buffer[12]);
				strcpy(newsgroups,&buffer[12]);
				break;
			}
		}

		Close(file);
		file=NULL;
	}
	if(lock) {
		UnLock(lock);
		lock=NULL;
	}
}

void get_refs(NewMessage * newmess,BPTR file,getrefs_t getRefs,BOOL trimRefs)
{
	// getRefs: 0=no, 1=get all, 2=get last only
	// trimRefs: reduce number of refs if too long?
	int cref = 0;
	char ** ref=newmess->references;
	char word1[1024] = "";
	char word2[1024] = "";
	char isobuf[4096] = "";
	const int BUFLEN = 2 * MAXLINE;
	//char buffer[BUFLEN] = "";
	//char nextline[MAXLINE] = "";
	char *buffer = new char[BUFLEN];
	char *nextline = new char[MAXLINE];
	char *ptr = NULL,*ptr2 = NULL;
	int k = 0,c = 0;

	*newmess->from = '\0';
	*newmess->replyto = '\0';
	*newmess->to = '\0';
	*newmess->newsgroups = '\0';
	*newmess->followup = '\0';
	*newmess->subject = '\0';
	*newmess->messageID = '\0';
	*newmess->supersedeID = '\0';
	*newmess->date = '\0';
	*newmess->type = '\0';
	*newmess->xnewsreader = '\0';
	*newmess->dummyHeader = '\0';

	newmess->mIDhash = 0;
	newmess->xno = FALSE;
	newmess->online = FALSE;
	newmess->lines = 0;
	BOOL getNextLine = TRUE;
	BOOL lastHeader = FALSE;
	ChangeFilePosition(file,0,OFFSET_BEGINNING);
	for(;;)
	{
		int len = 0;
		for(;;)
		{
			int nlen = 0;
			if(*nextline==0)
			{
				if(!FGets(file,&buffer[len],BUFLEN - len))
				{
					lastHeader = TRUE;
					break;
				}
				StripNewLine(&buffer[len]);
				// get rid of excess whitespace
				//    ..at end
				StripTrail(&buffer[len],' ');
				//    ..at start
				ptr=&buffer[len];
				while(isspace(*ptr))
					ptr++;
				ptr2=&buffer[len];
				//printf("%d (%c) , %d (%c)\n",ptr,ptr,ptr2,ptr2);
				if(ptr!=ptr2)
				{
					while(*ptr!='\0')
						*ptr2++ = *ptr++;
					*ptr2='\0';
				}
			}
			else
			{
				strncpy(buffer,nextline,BUFLEN-1);
				buffer[BUFLEN-1] = '\0';
				StripTrail(buffer,' ');
				*nextline=0;
			}
			//printf(">%s<\n",buffer);
			nlen = strlen(buffer);
			buffer[nlen++] = ' '; // newlines should give a space
			buffer[nlen] = '\0';
			ptr2 = strchr(&buffer[len],' ');
			if(ptr2[-1]==':' && len>0)
			{ // assume line of next header
				strncpy(nextline,&buffer[len],MAXLINE-1);
				nextline[MAXLINE-1] = '\0';
				buffer[len-1]=0; // end the buffer, also removing the last space
				//printf(">>>%s<<<\n",buffer);
				break;
			}
			len = nlen;
		}
		if(len == 0)
			break;
		if(*buffer == '\0')
			break;
		if(lastHeader)
		{
			// need to remove the last space (which came from the newline character)
			if(buffer[len-1] == ' ')
				buffer[len-1] = '\0';
		}
		//StripTrail(buffer,' ');
		wordFirstUpper(word1,buffer);

		//printf(":*%s*\n",word1);

		if(strlen(word1) == strlen(buffer))
		{
			// empty header
		}
		else if(strcmp(word1,"PATH:")==0)
		{
			//do nothing
		}
		else if(strcmp(word1,"FROM:")==0)
		{
			translateIso(isobuf,&buffer[6]);

			strncpy(newmess->from,isobuf,NEWMESS_long);
			//strncpy(newmess->from,&buffer[6],NEWMESS_long);
			newmess->from[NEWMESS_long]=0;
		}
		else if(strcmp(word1,"REPLY-TO:")==0) {
			strncpy(newmess->replyto,&buffer[10],NEWMESS_long);
			newmess->replyto[NEWMESS_long]=0;
		}
		else if(strcmp(word1,"TO:")==0)
		{
			strncpy(newmess->to,&buffer[4],NEWMESS_long);
			newmess->to[NEWMESS_long]=0;
		}
		else if(strcmp(word1,"NEWSGROUPS:")==0)
		{
			strncpy(newmess->newsgroups,&buffer[12],NEWMESS_long);
			newmess->newsgroups[NEWMESS_long]=0;
		}
		else if(strcmp(word1,"FOLLOWUP-TO:")==0)
		{
			strncpy(newmess->followup,&buffer[13],NEWMESS_long);
			newmess->followup[NEWMESS_long]=0;
		}
		else if(strcmp(word1,"SUBJECT:")==0)
		{
			translateIso(isobuf,&buffer[9]);
			strncpy(newmess->subject,isobuf,NEWMESS_long);
			//strncpy(newmess->subject,&buffer[9],NEWMESS_long);
			newmess->subject[NEWMESS_long]=0;
		}
		else if(strcmp(word1,"MESSAGE-ID:")==0)
		{
			strncpy(newmess->messageID,&buffer[12],NEWMESS_short);
			newmess->messageID[NEWMESS_short]=0;
			//newmess->mIDhash = calculate_mID_hash(newmess->messageID);
		}
		else if(stricmp(buffer,"X-NO-ARCHIVE: YES")==0)
			newmess->xno=TRUE;
		else if(strcmp(word1,"DATE:")==0)
		{
			strncpy(newmess->date,&buffer[6],NEWMESS_shortshort);
			newmess->date[NEWMESS_shortshort]=0;
		}
		else if(strcmp(word1,"CONTENT-TYPE:")==0)
		{
			strncpy(newmess->type,&buffer[14],63);
			newmess->type[63]=0;
			StripChar(newmess->type,';');
		}
		else if(strcmp(word1,"X-NEWSREADER:")==0)
		{
			strncpy(newmess->xnewsreader,&buffer[14],NEWMESS_long);
			newmess->xnewsreader[NEWMESS_long] = '\0';
		}
		else if(strncmp(buffer,onlineflag,strlen(buffer))==0)
		{
			//printf("!!!\n");
			newmess->online=TRUE;
		}
		else if(strcmp(word1,"LINES:")==0)
			newmess->lines = atoi(&buffer[7]);
		else if(getRefs!=GETREFS_NONE && strcmp(word1,"REFERENCES:")==0)
		{
			getNextLine=FALSE;
			if(getRefs==GETREFS_ALL)
			{
				c = 1;
				do
				{
					word(word2,&buffer[12],c++,'>');
					char *wptr = word2;
					while(*wptr != 0 && isspace(*wptr))
						wptr++;
					if(*wptr != 0)
					{
						int wlen = strlen(wptr);
						char *w = new char[ wlen + 2 ];
						strncpy(w,wptr,wlen);
						w[wlen] = '>';
						w[wlen+1] = '\0';
						ref[cref++] = w;
						if(cref==MAX_REFS)
						{
							nLog("Error - hit MAX_REFS limit of %d\n",cref);
							break;
						}
					}
				} while(*word2 != 0);
			}
			else if(getRefs==GETREFS_LAST)
			{
				char *lastref = NULL;
				char *ptr = buffer;
				for(;;)
				{
					ptr = strchr(ptr,'<');
					if(ptr == NULL)
						break;
					lastref = ptr;
					ptr++;
				}
				if(lastref != 0)
				{
					char *end = strchr(lastref,'>');
					if(end != 0)
					{
						int wlen = (int)end - (int)lastref + 1;
						char *w = new char[wlen+1];
						strncpy(w,lastref,wlen);
						w[wlen] = '\0';
						ref[cref++] = w;
					}
				}
			}
		}
		if(*newmess->getThisHeader!=0)
		{
			if(stricmp(word1,newmess->getThisHeader)==0)
			{
				strncpy(newmess->dummyHeader,&buffer[strlen(newmess->getThisHeader)+1],NEWMESS_long);
				newmess->dummyHeader[NEWMESS_long] = '\0';
			}
		}
		if(lastHeader)
			break;
	}

	int tlen=12;
	if(getRefs==GETREFS_ALL && cref>0 && trimRefs==TRUE)
	{
		for(k=0;k<cref;k++)
		{
			tlen += strlen(ref[k]);
			tlen++;
		}
		if(tlen>996 && cref > 5)
		{
			// line too long! lose some references!
			if(ref[4] != 0 && ref[cref-1] != 0)
			{
				delete [] ref[4];
				int len = strlen( ref[cref-1] );
				ref[4] = new char[len + 1];
				strcpy(ref[4],ref[cref-1]);
				delete [] ref[cref-1];
			}
			for(k=5;k<cref;k++)
			{
				if(ref[k])
					ref[k]=NULL;
			}
			cref = 5;
		}
	}
	newmess->nrefs = cref;
	delete [] buffer;
	delete [] nextline;
}

void get_refs(NewMessage * newmess,char *filename,getrefs_t getRefs,BOOL trimRefs)
{
	nLog("get_refs((NewMessage *)%d,(char *)%s,(BOOL)%d,(BOOL)%d) called\n",newmess,filename,getRefs,trimRefs);
	BPTR file=NULL;
	BPTR lock=NULL;
	lock=Lock(filename,ACCESS_READ);
	file=Open(filename,MODE_OLDFILE);
	if(file)
	{
		get_refs(newmess,file,getRefs,trimRefs);
		Close(file);
		file=NULL;
	}
	if(lock)
	{
		UnLock(lock);
		lock=NULL;
	}
}

void get_refs(NewMessage * newmess,int gID,int mID,getrefs_t getRefs)
{
	char filename[MAXFILENAME] = "";
	getFilePath(filename,gID,mID);
	get_refs(newmess,filename,getRefs,TRUE);
}

void get_refs(NewMessage * newmess,GroupData * gdata,MessageListData * mdata,getrefs_t getRefs)
{
	get_refs(newmess,gdata->ID,mdata->ID,getRefs);
}

BOOL search_mess(GroupData * gdata,MessageListData * mdata,char * text,BOOL casesens,int section)
{
	// section: 1=header, 2=body, 3=all
	// if casesens==TRUE, text must be in upper case!
	if(*text==0)
		return FALSE;
	BOOL found=FALSE;
	BPTR file=NULL;
	BPTR lock=NULL;
	char filename[MAXFILENAME] = "";
	getFilePath(filename,gdata->ID,mdata->ID);
	lock=Lock(filename,ACCESS_READ);
	file=Open(filename,MODE_OLDFILE);
	char buffer[MAXLINE] = "";
	BOOL header=TRUE;
	if(file)
	{
		// do header first - slow bit
		for(;;)
		{
			if(!FGets(file,buffer,MAXLINE))
				break;
			StripNewLine(buffer);
			if(*buffer==0) {
				header=FALSE;
				if(section==SEARCH_HEADER) // only searching the header?
					break;
			}
			//if( (header==TRUE && (section & 1)!=0) || (header==FALSE && (section & 2)!=0) ) {
			if( (header==TRUE && section != SEARCH_BODY) || (header==FALSE && section != SEARCH_HEADER) ) {
				if(casesens) {
					if(strstr(buffer,text)!=0) {
						found=TRUE;
						break;
					}
				}
				else {
					if(stristr_q(buffer,text)!=0) {
						found=TRUE;
						break;
					}
				}
			}
		}
		Close(file);
		file=NULL;
	}
	if(lock)
	{
		UnLock(lock);
		lock=NULL;
	}
	return found;
}

BOOL readInText(Object * textEditor,char *filename)
{
	nLog("readInText((Object *)%d,(char *)%s) called\n",textEditor,filename);
	BPTR file=NULL;
	BPTR lock=NULL;
	lock=Lock(filename,ACCESS_READ);
	file=Open(filename,MODE_OLDFILE);
	char buffer[MAXLINE+1] = "";
	int length = 0;
	if(file) {
		set(textEditor,MUIA_TextEditor_Quiet,TRUE);
		do {
			length=Read(file,buffer,MAXLINE);
			if(length<=0)
				break;
			buffer[length]=0;
			DoMethod(textEditor,MUIM_TextEditor_InsertText,buffer,MUIV_TextEditor_InsertText_Bottom);
		} while(1);
		Close(file);
		set(textEditor,MUIA_TextEditor_Quiet,FALSE);
	}
	if(lock) {
		UnLock(lock);
		lock=NULL;
	}
	return (file!=0);
}

BOOL update_index()
{
	nLog("update_index() called\n");
	// updates the index of the current folder
	GroupData * gdata = NULL;
	MessageListData * mdata = NULL;
	getGdataActive(&gdata);
	if(gdata==0)
		return FALSE;
	c_gdata=gdata;
	BPTR lock=NULL;
	BPTR lock2=NULL;
	char buffer[1024]="";
	char folder[256]="";
	char filename[MAXFILENAME]="";
	int maxID=-1;
	if(gdata->ID==-1)
		sprintf(folder,"NewsCoasterData:outgoing/");
	else if(gdata->ID==-2)
		sprintf(folder,"NewsCoasterData:sent/");
	else if(gdata->ID==-3)
		sprintf(folder,"NewsCoasterData:deleted/");
	else if(gdata->ID>=0)
		sprintf(folder,"NewsCoasterData:folder_%d/",gdata->ID);
	setMdataQuiet(TRUE);
	set(wnd_main,MUIA_Window_Sleep,TRUE);
	DoMethod(NLIST_messagelistdata,MUIM_NList_Clear);

	int nummess=0;
	BOOL aborted = FALSE;

	StatusWindow * statusWindow = new StatusWindow(app,CatalogStr(MSG_SCANNING_2,MSG_SCANNING_2_STR));
	sprintf(buffer,CatalogStr(MSG_SCANNING_FOUND,MSG_SCANNING_FOUND_STR),gdata->name);
	char *swptr = &buffer[ strlen(buffer) ];
	sprintf(swptr,"%d )",nummess); //#warning was zero!
	statusWindow->setText(buffer);

	APTR context = ObtainDirContextTags(EX_StringName,folder, EX_DataFields,(EXF_NAME|EXF_TYPE|EXF_SIZE), TAG_END);
	if( context )
	{
		struct ExamineData *dat;

	    while((dat = ExamineDir(context)))   /* until no more data.*/
		{
			DoMethod(app,MUIM_Application_InputBuffered);
			if(nummess % 64 == 0)
			{
				do_input();
				if((statusWindow->isAborted()==TRUE) || running==FALSE)
				{
					aborted = TRUE;
					break;
				}
				sprintf(swptr,"%d)",nummess);
				statusWindow->setText(buffer);
			}
			if(strncmp(dat->Name,"news_",5)==0)
			{
				BOOL digits = TRUE;
				char *ptr = &dat->Name[5];
				while( *ptr != '\0' && digits )
					digits = isdigit( *ptr++ );

				if( digits )
				{
					//news item found
					nLog("  found news file: %s\n",dat->Name);
					mdata=new MessageListData();
					if(mdata == NULL)
					{
						nLog("  Not enough RAM to allocate MessageListData struct! Aborting..\n");
						MUI_RequestA(app,0,0,"Severe Error!","_Okay","Not enough RAM!",0);
						aborted = TRUE;
						break;
					}
					nLog("   mdata: %d\n",mdata);
					mdata->ID = atoi(&dat->Name[5]);
					if(mdata->ID>maxID)
						maxID=mdata->ID;

					NewMessage newmess;
					//get_refs(&newmess,gdata,mdata,GETREFS_ALL);
					get_refs(&newmess,gdata,mdata,GETREFS_LAST);
					newmess.copyToMessageListData(mdata);

					getFilePath(filename,gdata->ID,mdata->ID);
					BPTR file = Open(filename,MODE_OLDFILE);
					if(file)
					{
						mdata->size=GetFileSize(file);
						Close(file);
					}
					/*if(*mdata->type==0)
						strcpy(mdata->type,"text/plain");*/
					DoMethod(NLIST_messagelistdata,MUIM_NList_InsertSingle,mdata,MUIV_NList_Insert_Sorted);
					nummess++;
				}
			}
		}
	}
	ReleaseDirContext(context);

	nLog(" finished scanning\n");
	if(aborted)
		read_index();
	else
	{
		gdata->nummess=nummess;
		gdata->num_unread = 0;
		redrawGdataAll();
		gdata->nextmID=maxID+1;
	}
	threadView();
	if(!aborted)
		write_index(0);
	delete statusWindow;

	set(wnd_main,MUIA_Window_Sleep,FALSE);
	setMdataQuiet(FALSE);
	if(nummess>0)
		setMdataActive(0);
	setEnabled();
	nLog("  update_index() finished\n");
	return aborted;
}

void import_file(char *filename2)
{
	nLog("import_file((char *)%s) called\n",filename2);
	GroupData * gdata = NULL;
	MessageListData * mdata = NULL;
	BPTR file2=NULL;
	BPTR lock2=NULL;
	char filename[MAXFILENAME]="";
	char command[2 * MAXFILENAME + 128]="";

	file2=Open(filename2,MODE_OLDFILE);
	if(file2)
	{
		//file found
		//ctypegot=FALSE;
		mdata=new MessageListData();
		gdata=NULL;
		mdata->size=GetFileSize(file2);

		NewMessage * newmess = new NewMessage;
		get_refs(newmess,filename2,GETREFS_LAST,FALSE);
		//get_refs(newmess,filename2,GETREFS_ALL,FALSE);
		newmess->copyToMessageListData(mdata);

		if(*mdata->newsgroups!=0)
		{
			get_gdata(&gdata,mdata->newsgroups); // cross posts only go into one folder atm!
			if(gdata)
			{
				mdata->ID=gdata->nextmID;
				gdata->nextmID++;
				mdata->flags[6]=gdata->ID;
			}
		}
		Close(file2);
		file2=NULL;
		if(gdata)
		{
				char fileIn[MAXFILENAME]="";
				char fileOut[MAXFILENAME]="";
				// looks like a valid news item; successfully extracted newsgroup header
				sprintf(filename,"folder_%d/news_%d",gdata->ID,mdata->ID);

				//sprintf(command,"Copy NewsCoasterData:%s to NewsCoasterData:%s",filename2,filename);
				sprintf(fileIn,"NewsCoasterData:%s",filename2);
				sprintf(fileOut,"NewsCoasterData:%s",filename);

				//Execute(command,NULL,NULL);
				FileCopy(fileIn,fileOut);
				//System(command, NULL);
				write_index_single(gdata,mdata);
		}
		//delete mdata;
	}
}

void import_folder(char * ifolder,StatusWindow * statusWindow)
{
	nLog("import_folder((char *)%s) called\n",ifolder);
	// imports messages from another directory
	char folder[MAXFILENAME] = "";
	char subfolder[MAXFILENAME]="";
	char filename[MAXFILENAME] = "";
	char info[256]="";
	sprintf(info,CatalogStr(MSG_SCANNING_3,MSG_SCANNING_3_STR),ifolder);
	statusWindow->setText(info);

	BPTR lock=NULL;
	if(stristr(ifolder,":")==0)
		sprintf(folder,"NewsCoaster:%s",ifolder);
	else
		strcpy(folder,ifolder);

	APTR context = ObtainDirContextTags(EX_StringName,folder, EX_DataFields,(EXF_NAME|EXF_TYPE|EXF_SIZE), TAG_END);
	if( context )
	{
		struct ExamineData *dat;

		set(wnd_main,MUIA_Window_Sleep,TRUE);
		while((dat = ExamineDir(context)))   /* until no more data.*/
		{

			if(EXD_IS_DIRECTORY(dat))
			{
				//sub-directory
				sprintf(subfolder,"%s%s/",folder,dat->Name);
				import_folder(subfolder,statusWindow);
			}
			else
			{
				sprintf(filename,"%s%s",folder,dat->Name);
				import_file(filename);
			}
			do_input(); // input loop
			if(statusWindow->isAborted()==TRUE || running==FALSE)
				break;
		}
		setEnabled();
		set(wnd_main,MUIA_Window_Sleep,FALSE);
	}
	ReleaseDirContext(context);
	return;
}

void import()
{
	nLog("import() called\n");
	char folder[1024]="";
	sleepAll(TRUE);
	if(!LoadASL(folder,CatalogStr(MSG_DIRECTORY_TO_SCAN,MSG_DIRECTORY_TO_SCAN_STR),(const char *)"",(const char *)"#?",TRUE)) {
		sleepAll(FALSE);
		return;
	}
	StatusWindow * statusWindow = new StatusWindow(app,CatalogStr(MSG_SCANNING_2,MSG_SCANNING_2_STR));
	import_folder(folder,statusWindow);
	delete statusWindow;
	sleepAll(FALSE);
	read_index();
}

void save_killfile(char *filename) {
	nLog("save_killfile((char *)%s) called\n",filename);
	//save folder prefs
	char filenamenew[256] = "";
	sprintf(filenamenew,"%s.new",filename);
	DeleteFile(filenamenew);
	int entries = 0;
	get(NLIST_acc_KILLFILE,MUIA_NList_Entries,&entries);
	//Write info...
	BPTR file = Open(filenamenew,MODE_NEWFILE);
	if(file) {
		for(int k=0;k<entries;k++) {
			KillFile * kill = NULL;
			DoMethod(NLIST_acc_KILLFILE,MUIM_NList_GetEntry,k,&kill);
			Write(file,kill,sizeof(KillFile));
		}
		Close(file);
		file=NULL;
	}
	DeleteFile(filename);
	Rename(filenamenew,filename);
}

void save_killfile() {
	char filename[] = "NewsCoasterData:.killfile";
	save_killfile(filename);
}

void load_killfile() {
	nLog("load_killfile() called\n");
	KillFile * kill = NULL;
	BPTR file=NULL;
	BPTR lock=NULL;
	int temp = 0;
	//save folder prefs
	char filename[] = "NewsCoasterData:.killfile";
	lock=Lock(filename,ACCESS_READ);
	//Write info...
	file=Open(filename,MODE_OLDFILE);
	BOOL expired;
	struct DateStamp ds;
	DateStamp(&ds);
	if(file)
	{
		DoMethod(NLIST_acc_KILLFILE,MUIM_NList_Clear);
		do
		{
			kill=new KillFile();
			temp=Read(file,kill,sizeof(KillFile));
			if(temp<=0)
				break;
			nLog("  Read killfile: %s %s %s\n",kill->header,kill->text,kill->ngroups);
			// has killfile expired?
			expired=FALSE;
			if(kill->expiretype==1) // days since creation
			{
				if(ds.ds_Days - kill->ds.ds_Days > kill->expire)
					expired=TRUE;
			}
			else if(kill->expiretype==2) // days since last used
			{
				if(ds.ds_Days - kill->lastused.ds_Days > kill->expire)
					expired=TRUE;
			}
			if(!expired) {
				// convert to upper case
				toUpper(kill->header);
				toUpper(kill->text);
				toUpper(kill->ngroups);
				DoMethod(NLIST_acc_KILLFILE,MUIM_NList_InsertSingle,kill,MUIV_NList_Insert_Bottom);
			}
			else
			{
				delete kill;
				kill=NULL;
			}
		} while(1);
		if(kill)
		{
			delete kill;
			kill=NULL;
		}
		Close(file);
		file=NULL;
	}
	if(lock)
	{
		UnLock(lock);
		lock=NULL;
	}
}

Server * getDefaultServer() {
	Server *server=NULL;
	int entries = 0;
	get(NLIST_acc_SERVERS,MUIA_NList_Entries,&entries);
	for(int i=0;i<entries;i++) {
		DoMethod(NLIST_acc_SERVERS,MUIM_NList_GetEntry,i,&server);
		if(server->def==TRUE)
			return server;
		if(i==entries-1) // no default server! make last one the default
			server->def=TRUE;
	}
	return NULL;
}

Server * getPostingServer() {
	Server *server=NULL;
	int entries = 0;
	get(NLIST_acc_SERVERS,MUIA_NList_Entries,&entries);
	for(int i=0;i<entries;i++) {
		DoMethod(NLIST_acc_SERVERS,MUIM_NList_GetEntry,i,&server);
		if(server->post==TRUE)
			return server;
		if(i==entries-1) // no posting server! use default
			return getDefaultServer();
	}
	return NULL;
}

/*Server * getServerPosting() {
	if(*account.nntppost!=0) {
	}
}*/

Server * getServer(int ID) {
	if(ID==0)
		return getDefaultServer();
	Server *server=NULL;
	int entries = 0;
	get(NLIST_acc_SERVERS,MUIA_NList_Entries,&entries);
	for(int i=0;i<entries;i++) {
		DoMethod(NLIST_acc_SERVERS,MUIM_NList_GetEntry,i,&server);
		if(server->ID==ID)
			return server;
	}
	// not found! return default server instead
	return getDefaultServer();
}

Server * getPostingServerForGroup(char *group)
{
	Server *server = NULL;
	GroupData *gdata = NULL;

	if (!group)
		return getPostingServer();

	get_gdata(&gdata,group);

	if( NULL == gdata
		|| gdata->serverID == 0
		|| !gdata->moreflags[4]
		)
		server = getPostingServer(); // default
	else {
		// we're posting with a non default server
		server = getServer(gdata->serverID);
	}

	return server;
}

void freeServerCycle() {
	if(GROUP_ngadv_SUB != 0) {
		DoMethod(GROUP_ngadv,OM_REMMEMBER,GROUP_ngadv_SUB);
		MUI_DisposeObject(GROUP_ngadv_SUB);
		GROUP_ngadv_SUB = NULL;
	}
	for(int i=0;;) {
		if(CYA_ngadv_server[i] == NULL)
			break;
		delete [] CYA_ngadv_server[i++];
	}
	delete [] CYA_ngadv_server;
	delete [] server_list;
}

int setServerCycle(int ID) {
	Server *server = NULL;
	int entries = 0;
	get(NLIST_acc_SERVERS,MUIA_NList_Entries,&entries);
	CYA_ngadv_server = new char *[entries+1];
	server_list = new int[entries];

	int active=0;
	for(int i=0;i<entries;i++) {
		DoMethod(NLIST_acc_SERVERS,MUIM_NList_GetEntry,i,&server);
		CYA_ngadv_server[i] = new char[SERVER_LEN+1];
		strncpy(CYA_ngadv_server[i],server->nntp,SERVER_LEN);
		(CYA_ngadv_server[i])[SERVER_LEN] = '\0';
		server_list[i] = server->ID;
		if(server->ID == ID)
			active = i;
	}
	CYA_ngadv_server[entries] = NULL;
	GROUP_ngadv_SUB = VGroup,
		Child, HGroup,
			Child, Label2(CatalogStr(LABEL_NEWSSERVER,LABEL_NEWSSERVER_STR)),
			Child, CY_ngadv_SERVER = Cycle(CYA_ngadv_server),
			Child, Label1(CatalogStr(LABEL_DEFAULT,LABEL_DEFAULT_STR)),
			Child, CM_ngadv_SERVER = CheckMark(FALSE),
			End,
		Child, HGroup,
			Child, HSpace(0),
			Child, Label1(CatalogStr(LABEL_POST_WITH_THIS_SERVER,LABEL_POST_WITH_THIS_SERVER_STR)),
			Child, CM_ngadv_SERVERPOST = CheckMark(FALSE),
			End,
		//Child, STR_ngadv_SERVER=BetterStringObject, StringFrame, MUIA_String_Contents, "", MUIA_String_MaxLen, 64, MUIA_CycleChain, 1, End,
		End;
	DoMethod(GROUP_ngadv,OM_ADDMEMBER,GROUP_ngadv_SUB);
	//set(CY_ngadv_SERVER,MUIA_Cycle_Entries,CYA_ngadv_server);

	DoMethod(CM_ngadv_SERVER,MUIM_Notify,MUIA_Selected,TRUE,
		CY_ngadv_SERVER,3,MUIM_Set,MUIA_Disabled,TRUE);
	DoMethod(CM_ngadv_SERVER,MUIM_Notify,MUIA_Selected,FALSE,
		CY_ngadv_SERVER,3,MUIM_Set,MUIA_Disabled,FALSE);

	DoMethod(CM_ngadv_SERVER,MUIM_Notify,MUIA_Selected,TRUE,
		CM_ngadv_SERVERPOST,3,MUIM_Set,MUIA_Disabled,TRUE);
	DoMethod(CM_ngadv_SERVER,MUIM_Notify,MUIA_Selected,FALSE,
		CM_ngadv_SERVERPOST,3,MUIM_Set,MUIA_Disabled,FALSE);

	if(ID==0)
		set(CM_ngadv_SERVER,MUIA_Selected,TRUE);
	else
		set(CM_ngadv_SERVER,MUIA_Selected,FALSE);
	set(CY_ngadv_SERVER,MUIA_Cycle_Active,active);
	return active;
}

void save_servers(char *filename) {
	nLog("save_servers((char *)%s) called\n",filename);
	Server * server = NULL;
	BPTR file=NULL;
	//save folder prefs
	DeleteFile(filename);
	int entries = 0;
	get(NLIST_acc_SERVERS,MUIA_NList_Entries,&entries);
	//Write info...
	file=Open(filename,MODE_NEWFILE);
	if(file)
	{
		if(entries>0)
		{
			Write(file,&entries,sizeof(int));
			for(int k=0;k<entries;k++)
			{
				DoMethod(NLIST_acc_SERVERS,MUIM_NList_GetEntry,k,&server);
				Write(file,server,sizeof(Server));
			}
		}
		Close(file);
		file=NULL;
	}
}

void save_servers() {
	char filename[] = "NewsCoasterData:.servers";
	save_servers(filename);
}

void load_servers() {
	nLog("load_servers() called\n");
	Server * server = NULL;
	BPTR file=NULL;
	BPTR lock=NULL;
	int temp = 0;
	char filename[] = "NewsCoasterData:.servers";
	lock=Lock(filename,ACCESS_READ);
	//Write info...
	file=Open(filename,MODE_OLDFILE);
	if(file) {
		DoMethod(NLIST_acc_SERVERS,MUIM_NList_Clear);
		int entries;
		Read(file,&entries,sizeof(int));
		do {
			server=new Server;
			temp=Read(file,server,sizeof(Server));
			if(temp<=0)
				break;
			DoMethod(NLIST_acc_SERVERS,MUIM_NList_InsertSingle,server,MUIV_NList_Insert_Bottom);
			if(server->ID >= nextServerID)
				nextServerID = server->ID + 1;
		} while(1);
		if(server)
		{
			delete server;
			server=NULL;
		}
		Close(file);
		file=NULL;
	}
	if(lock)
	{
		UnLock(lock);
		lock=NULL;
	}
}

void getFolderPath(char *buffer,int gID)
{
	nLog("getFolderPath((char *)%d,(int)%d) called\n",buffer,gID);
	if(gID==-1)
		strcpy(buffer,"NewsCoasterData:outgoing");
	else if(gID==-2)
		strcpy(buffer,"NewsCoasterData:sent");
	else if(gID==-3)
		strcpy(buffer,"NewsCoasterData:deleted");
	else
		sprintf(buffer,"NewsCoasterData:folder_%d",gID);
}

void getIndexPath(char *buffer,int gID)
{
	nLog("getIndexPath((char *)%d,(int)%d) called\n",buffer,gID);
	if(gID==-1)
		strcpy(buffer,"NewsCoasterData:outgoing/.index");
	else if(gID==-2)
		strcpy(buffer,"NewsCoasterData:sent/.index");
	else if(gID==-3)
		strcpy(buffer,"NewsCoasterData:deleted/.index");
	else
		sprintf(buffer,"NewsCoasterData:folder_%d/.index",gID);
}

void getFilePath(char *buffer,int gID,int mID)
{
	nLog("getFilePath((char *)%d,(int)%d,(int)%d) called\n",buffer,gID,mID);
	if(gID==-1)
		sprintf(buffer,"NewsCoasterData:outgoing/news_%d",mID);
	else if(gID==-2)
		sprintf(buffer,"NewsCoasterData:sent/news_%d",mID);
	else if(gID==-3)
		sprintf(buffer,"NewsCoasterData:deleted/news_%d",mID);
	else
		sprintf(buffer,"NewsCoasterData:folder_%d/news_%d",gID,mID);
}

void makeFolder(int gID)
{
	nLog("makeFolder((int)%d) called\n",gID);
	char buffer[1024] = "";

	BPTR lock = NULL;
	getFolderPath(buffer,gID);
	if(!exists(buffer)) {
		lock=CreateDir(buffer);
		if(lock) {
			UnLock(lock);
			lock=NULL;
		}
	}
	getIndexPath(buffer,gID);
	if(!exists(buffer)) {
		/*sprintf(command,"copy NewsCoaster:default.index %s",buffer);
		Execute(command,NULL,NULL);*/
		writeEmptyIndex(gID);
	}
}

void createOutgoing()
{
	nLog("createOutgoing() called\n");
	GroupData * gdata=new GroupData();
	gdata->ID=-1;
	strcpy(gdata->name,MSG_FOLDER_OUTGOING_STR);
	gdata->nummess=0;
	gdata->num_unread=0;
	gdata->nextmID=0;
	gdata->s=FALSE;
	gdata->max_dl=-1;
	insertGdata(gdata,TRUE);
	makeFolder(gdata->ID);
}

void createSent()
{
	nLog("createSent() called\n");
	GroupData * gdata=new GroupData();
	gdata->ID=-2;
	strcpy(gdata->name,MSG_FOLDER_SENT_STR);
	gdata->nummess=0;
	gdata->num_unread=0;
	gdata->nextmID=0;
	gdata->s=FALSE;
	gdata->max_dl=-1;
	insertGdata(gdata,TRUE);
	makeFolder(gdata->ID);
}

void createDeleted()
{
	nLog("createDeleted() called\n");
	GroupData * gdata=new GroupData();
	gdata->ID=-3;
	strcpy(gdata->name,MSG_FOLDER_DELETED_STR);
	gdata->nummess=0;
	gdata->num_unread=0;
	gdata->nextmID=0;
	gdata->s=FALSE;
	gdata->max_dl=-1;
	insertGdata(gdata,TRUE);
	makeFolder(gdata->ID);
}

BOOL exists(const char *filename)
{
	nLog("exists((char *)%s) called\n",filename);
	BOOL res = FALSE;

	BPTR lock = Lock(filename,ACCESS_READ);
	if(lock != 0) {
		res = TRUE;
		UnLock(lock);
	}
	nLog("  finished exists() - returning %d\n",res);
	return res;
}

void resetMessageListFormat()
{
	nLog("resetMessageListFormat() called\n");
	char *ptr = messagelistformat;
	int cur_flag = 2;
	BOOL first = TRUE;
	for(int i=0;i<6;i++) {
		if((account.listflags & cur_flag) != 0) {
			if(first)
				first = FALSE;
			else
				*ptr++ = ',';
		}
		cur_flag *= 2;
	}
	*ptr = '\0';
	nLog("%s\n",messagelistformat);
	set(NLIST_messagelistdata,MUIA_NList_Format,messagelistformat);
	set(LISTTREE_messagelistdata,MUIA_NListtree_Format,messagelistformat);
}

int getInternalColumn(int col)
{
	nLog("getInternalColumn((int)%d) called\n",col);
	//return col;
	int cur_flag = 2;
	for(int i=0;i<6;i++) {
		BOOL col_showed = (account.listflags & cur_flag) != 0;
		//col_showed = TRUE;
		if(col_showed)
			col--;
		if(col < 0) {
			nLog("  returning %d\n",i);
			return i;
		}
		cur_flag *= 2;
	}
	nLog("  UNKNOWN COLUMN!!! returning 0..\n");
	return 0;
}

BOOL getSortType(int *type,int icol,int gID) {
	nLog("getSortType(int *)%d,(int)%d,(int)&d) called\n",type,icol,gID);
	//sorttype: 0=subject, 1=date, 2=from, -1=newsgroups, 3=size, 4=lines
	BOOL res = TRUE;
	*type = 0;
	switch(icol) {
	case 1:
		*type = 0;
		break;
	case 2:
		*type = 1;
		break;
	case 3:
		if(gID < 0)
			*type = -1;
		else
			*type = 2;
		break;
	case 4:
		*type = 3;
		break;
	case 5:
		*type = 4;
		break;
	default:
		res = FALSE;
		break;
	}
	nLog("  setting type to %d\n",*type);
	nLog("  returning %d\n",res);
	return res;
}

BOOL addUser(User * user) {
	char usersfile[] = "NewsCoaster:.users";
	BOOL success=FALSE;
	if(exists(usersfile)) {
		BPTR file = Open(usersfile,MODE_READWRITE);
		if(file != 0) {
			short nousers = 0,logonreq = 0;
			Read(file,&nousers,sizeof(short));
			nousers++;
			logonreq=TRUE; // now there is more than one user
			ChangeFilePosition(file,0,OFFSET_BEGINNING);
			Write(file,&nousers,sizeof(short));
			Write(file,&logonreq,sizeof(short));
			ChangeFilePosition(file,0,OFFSET_END);
			Write(file,user,sizeof(User));
			success=TRUE;
			Close(file);
			file=NULL;
		}
	}
	else {
		BPTR file = Open(usersfile,MODE_NEWFILE);
		if(file != 0) {
			short nousers = 0,logonreq = 0;
			nousers=1;
			if(user->requiresPassword())
				logonreq=TRUE;
			else
				logonreq=FALSE;
			Write(file,&nousers,sizeof(short));
			Write(file,&logonreq,sizeof(short));
			short dummy=0;
			for(int i=0;i<16;i++) // spare
				Write(file,&dummy,sizeof(short));
			Write(file,user,sizeof(User));
			success=TRUE;
			Close(file);
			file=NULL;
		}
	}
	return success;
}

BOOL replaceUser(User * user) {
	char usersfile[] = "NewsCoaster:.users";
	BOOL success=FALSE;
	if(exists(usersfile)) {
		BPTR file = Open(usersfile,MODE_READWRITE);
		if(file != 0) {
			short nousers = 0,logonreq = 0;
			Read(file,&nousers,sizeof(short));
			Read(file,&logonreq,sizeof(short));
			ChangeFilePosition(file,-sizeof(short),OFFSET_CURRENT);
			if(nousers==1 && user->requiresPassword()==TRUE && logonreq==FALSE)
				logonreq=TRUE;
			else if(nousers==1 && user->requiresPassword()==FALSE && logonreq==TRUE)
				logonreq=FALSE;
			Write(file,&logonreq,sizeof(short));
			ChangeFilePosition(file,18*sizeof(short),OFFSET_BEGINNING);
			for(int i=0;i<nousers;i++) {
				User user2;
				Read(file,&user2,sizeof(User));
				if(strcmp(user->getName(),user2.getName())==0) {
					ChangeFilePosition(file,-sizeof(User),OFFSET_CURRENT);
					Write(file,user,sizeof(User));
					success=TRUE;
					break;
				}
			}
			Close(file);
			file=NULL;
		}
	}
	return success;
}

void saveUsers() {
	nLog("saveUsers() called\n");
	int k = 0;
	User * user = NULL;
	// save folder prefs
	char filename[] = "NewsCoaster:.users";
	DeleteFile(filename);
	int entries = 0;
	get(NLIST_users_LIST,MUIA_NList_Entries,&entries);
	// Write info...
	BPTR file = Open(filename,MODE_NEWFILE);
	if(file != 0) {
		short nousers=entries,logonreq=TRUE;
		if(entries==1) {
			DoMethod(NLIST_users_LIST,MUIM_NList_GetEntry,k,&user);
			if(user->requiresPassword()==FALSE)
				logonreq=FALSE;
		}
		Write(file,&nousers,sizeof(short));
		Write(file,&logonreq,sizeof(short));
		short dummy=0;
		for(int i=0;i<16;i++) // spare
			Write(file,&dummy,sizeof(short));
		for(k=0;k<entries;k++) {
			DoMethod(NLIST_users_LIST,MUIM_NList_GetEntry,k,&user);
			Write(file,user,sizeof(User));
		}
		Close(file);
		file=NULL;
	}
}

int checkUniqueUser(char * name,char * dataLoc) {
	User * user = NULL;
	int entries = 0;
	get(NLIST_users_LIST,MUIA_NList_Entries,&entries);
	for(int k=0;k<entries;k++) {
		DoMethod(NLIST_users_LIST,MUIM_NList_GetEntry,k,&user);
		if(strcmp(user->getName(),name)==0)
			return 1;
		else if(stricmp(user->dataLocation,dataLoc)==0)
			return 2;
	}
	return 0;
}

void loadUsers() {
	nLog("loadUsers() called\n");
	User * user = NULL;
	int temp = 0;
	char filename[] = "NewsCoaster:.users";
	BPTR lock = Lock(filename,ACCESS_READ);
	BPTR file = Open(filename,MODE_OLDFILE);

	if(file != 0)
	{
		ChangeFilePosition(file,18*sizeof(short),OFFSET_CURRENT); // spare

		DoMethod(NLIST_users_LIST,MUIM_NList_Clear);
		do
		{
			user = new User;
			temp=Read(file,user,sizeof(User));

			if(temp<=0)
				break;

			DoMethod(NLIST_users_LIST,MUIM_NList_InsertSingle,user,MUIV_NList_Insert_Bottom);
		}
		while(1);

		if(user)
		{
			delete user;
			user=NULL;
		}

		Close(file);
		file=NULL;
	}
	if(lock)
	{
		UnLock(lock);
		lock=NULL;
	}
}

void readInGroups() {
	nLog("readInGroups() called\n");
	GroupData * gdata = NULL;
	char filename[] = "NewsCoasterData:.prefs";
	BPTR lock = Lock(filename,ACCESS_READ);
	//Read info...
	BPTR file=Open(filename,MODE_OLDFILE);
	if(file != 0) {
		char H[5];
		int thisprefsversion;
		Read(file,H,5);
		Read(file,&thisprefsversion,sizeof(int));
		if(thisprefsversion<2) /*{
			printf("Your current '.prefs' file appears to be from a previous version!\n");
			printf("If you encounter problems, reset your Account settings ('Preferences/Program Settings') and select 'Newsgroups/Update Groups'\n");
		}*/
		MUI_RequestA(app,0,0,CatalogStr(MSG_WARNING,MSG_WARNING_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_OLD_PREFS_FILE,MSG_OLD_PREFS_FILE_STR),0);
		Read(file,&nextID,sizeof(int));
		int entries = 0;
		Read(file,&entries,sizeof(int));
		clearGdata();
		if(account.grouplistType!=0) {
			tn_folders=(MUI_NListtree_TreeNode *)DoMethod(LISTTREE_groupdata,MUIM_NListtree_Insert,CatalogStr(MSG_FOLDERS,MSG_FOLDERS_STR),NULL,MUIV_NListtree_Insert_ListNode_Root,MUIV_NListtree_Insert_PrevNode_Tail,TNF_LIST);
			tn_newsgroups=(MUI_NListtree_TreeNode *)DoMethod(LISTTREE_groupdata,MUIM_NListtree_Insert,CatalogStr(MSG_NEWSGROUPS,MSG_NEWSGROUPS_STR),NULL,MUIV_NListtree_Insert_ListNode_Root,MUIV_NListtree_Insert_PrevNode_Tail,TNF_LIST);
		}

		for(int k=0;k<entries;k++) {
			gdata=new GroupData();
			if(thisprefsversion<3)
				Read(file,gdata,sizeof(GroupData_v2));
			else
				Read(file,gdata,sizeof(GroupData));
			/*gdata->flags[0]=FALSE;
			gdata->flags[1]=FALSE;*/
			insertGdata(gdata,FALSE);
		}
		// still needed for NListtree ?
		/*if(account.grouplistType!=0)
			DoMethod(LISTTREE_groupdata,MUIM_NListtree_Insert,"",NULL,MUIV_NListtree_Insert_ListNode_Root,MUIV_NListtree_Insert_PrevNode_Tail,TNF_LIST);*/
		Close(file);
		file=NULL;
	}
	if(lock) {
		UnLock(lock);
		lock=NULL;
	}
}

BOOL init_data() {
	char charset_file[125];
	nLog("init_data() called\n");
	int k = 0;
	for(k=0;k<NSIGS;k++)
		sigs[k]=NULL;
	//load folder prefs
	// assign NewsCoaster:
	BPTR anlock = Lock("PROGDIR:",ACCESS_READ);
	if(anlock != 0) {
		if(!AssignLock("NewsCoaster",anlock))
			UnLock(anlock);
		anlock=NULL;
	}

	// check users
	char usersfile[] = "NewsCoaster:.users";
	if(exists(usersfile)) {
		BPTR lock = Lock(usersfile,ACCESS_READ);
		BPTR file = Open(usersfile,MODE_OLDFILE);
		BOOL fail=FALSE;
		if(file != 0) {
			short nousers = 0,logonreq = 0;
			Read(file,&nousers,sizeof(short));
			Read(file,&logonreq,sizeof(short));
			ChangeFilePosition(file,16*sizeof(short),OFFSET_CURRENT); // spare
			if (logonreq==TRUE)
			{
				set(wnd_login,MUIA_Window_Open,TRUE);
				set(wnd_login,MUIA_Window_ActiveObject,STR_login_USER);
				char name[USER_LEN+1]="";
				char pass[USER_LEN+1]="";
				BOOL loginrun=TRUE;
				while(loginrun)
				{
					switch(DoMethod(app,MUIM_Application_NewInput,&u_signal))
					{
					case LOGIN_OKAY:
						loginrun=FALSE;
						break;
					case LOGIN_CANCEL:
						loginrun=FALSE;
						fail=TRUE;
						break;
					}
					if(loginrun && u_signal)
						Wait(u_signal);
				}
				set(wnd_login,MUIA_Window_Open,FALSE);
				if(fail==FALSE)
				{
					// see if login is valid!
					currentUser = new User();
					BOOL okay=FALSE;
					do
					{
printf(".");
						if(Read(file,currentUser,sizeof(User))<=0)
							break;
						strncpy(name,getstr(STR_login_USER),USER_LEN); // must do each time, since fields are cleared by User::isValid()
						name[USER_LEN] = '\0';
						strncpy(pass,getstr(STR_login_PASS),USER_LEN);
						pass[USER_LEN] = '\0';
						okay = currentUser->isValid(name,pass);
					}
					while(okay==FALSE);
					if(!okay)
					{
						MUI_RequestA(app,0,0,CatalogStr(MSG_LOGIN,MSG_LOGIN_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_LOGIN_FAILED,MSG_LOGIN_FAILED_STR),0);
						fail=TRUE;
					}
				}
			}
			else {
				// need to get dataLoc of this user
				currentUser = new User();
				Read(file,currentUser,sizeof(User));
			}
		}
		if(file) {
			Close(file);
			file=NULL;
		}
		if(lock) {
			UnLock(lock);
			lock=NULL;
		}
		if(fail)
			return FALSE;
	}
	else {
		// there is no '.users' file, so ask for a new user
		set(CM_newuser_SUP,MUIA_Disabled,TRUE); // MUST have supervisor for first user
		set(CM_newuser_COPYPREFS,MUIA_Disabled,TRUE); // this one makes no sense
		set(wnd_newuser,MUIA_Window_Open,TRUE);


		int val = 0;
		while(newuseradded==0)
		{
			do_input();
			get(wnd_newuser,MUIA_Window_Open,&val);
			if(val==FALSE)
			{
				if(newuseradded==0)
					newuseradded=-1;
			}
			if((newuseradded==0) && u_signal)
				Wait(u_signal);
		}
		set(CM_newuser_SUP,MUIA_Disabled,FALSE);
		set(CM_newuser_COPYPREFS,MUIA_Disabled,FALSE);
		if(newuseradded!=1)
			return FALSE;
	}
	if(currentUser->isSupervisor())
		set(menuitem_USERS,MUIA_Menuitem_Enabled,TRUE);
	else
		set(menuitem_USERS,MUIA_Menuitem_Enabled,FALSE);

	loadUsers();

	// assign NewsCoasterData:
	//lock=Lock(ncdata_folder,ACCESS_READ);
	if(!exists(currentUser->dataLocation)) {
		int result=MUI_RequestA(app,0,0,CatalogStr(MSG_STARTUP,MSG_STARTUP_STR),CatalogStr(MSG_CONTINUE_OR_QUIT,MSG_CONTINUE_OR_QUIT_STR),CatalogStr(MSG_NEWS_DIRECTORY_DOES_NOT_EXIST,MSG_NEWS_DIRECTORY_DOES_NOT_EXIST_STR),0);
		if(result==1)
			strcpy(currentUser->dataLocation,"PROGDIR:");
		else
			return FALSE;
	}
	assignNewsCoasterData:
	BOOL success=FALSE;
	BPTR andlock = Lock(currentUser->dataLocation,ACCESS_READ);
	if(andlock) {
		if(!AssignLock("NewsCoasterData",andlock))
			UnLock(andlock);
		else
			success=TRUE;
		andlock=NULL;
	}
	if(success==FALSE) {
		if(strcmp(currentUser->dataLocation,"PROGDIR:")==0) {
			MUI_RequestA(app,0,0,CatalogStr(MSG_STARTUP,MSG_STARTUP_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_PROGDIR_CANNOT_BE_ACCESSED,MSG_PROGDIR_CANNOT_BE_ACCESSED_STR),0);
			return FALSE;
		}
		else {
			int result=MUI_RequestA(app,0,0,CatalogStr(MSG_STARTUP,MSG_STARTUP_STR),CatalogStr(MSG_CONTINUE_OR_QUIT,MSG_CONTINUE_OR_QUIT_STR),CatalogStr(MSG_NEWS_DIRECTORY_CANNOT_BE_ACCESSED,MSG_NEWS_DIRECTORY_CANNOT_BE_ACCESSED_STR),0);
			if(result==1) {
				strcpy(currentUser->dataLocation,"PROGDIR:");
				goto assignNewsCoasterData;
			}
			else
				return FALSE;
		}
	}
	char filename[]="NewsCoasterData:.prefs";
	BPTR lock = Lock(filename,ACCESS_READ);
	BOOL bakup=FALSE;
	//Read info...
	BPTR file=Open(filename,MODE_OLDFILE);
	if(file != 0) {
		char H[5] = "";
		int thisprefsversion = 0;
		Read(file,H,5);
		Read(file,&thisprefsversion,sizeof(int));
		if(thisprefsversion<2) /*{
			printf("Your current '.prefs' file appears to be from a previous version!\n");
			printf("If you encounter problems, reset your Account settings ('Preferences/Program Settings') and select 'Newsgroups/Update Groups'\n");
		}*/
			MUI_RequestA(app,0,0,CatalogStr(MSG_WARNING,MSG_WARNING_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_OLD_PREFS_FILE,MSG_OLD_PREFS_FILE_STR),0);
		if(thisprefsversion<prefsversion)
			bakup=TRUE;
		Read(file,&nextID,sizeof(int));
		int entries=0;
		Read(file,&entries,sizeof(int));
		BOOL out_folder=FALSE;
		BOOL sent_folder=FALSE;
		BOOL delete_folder=FALSE;
		tn_folders=(MUI_NListtree_TreeNode *)DoMethod(LISTTREE_groupdata,MUIM_NListtree_Insert,CatalogStr(MSG_FOLDERS,MSG_FOLDERS_STR),NULL,MUIV_NListtree_Insert_ListNode_Root,MUIV_NListtree_Insert_PrevNode_Tail,TNF_LIST);
		tn_newsgroups=(MUI_NListtree_TreeNode *)DoMethod(LISTTREE_groupdata,MUIM_NListtree_Insert,CatalogStr(MSG_NEWSGROUPS,MSG_NEWSGROUPS_STR),NULL,MUIV_NListtree_Insert_ListNode_Root,MUIV_NListtree_Insert_PrevNode_Tail,TNF_LIST);

		GroupData ** gdataptr=NULL;
		//printf("%d\n",thisprefsversion);
		//printf("->%d\n",prefsversion);
		if(entries>0) {
			// argh, we have the group data stored before the account, so we store the info
			gdataptr = new GroupData *[entries];
			for(k=0;k<entries;k++) {
				GroupData *gdata = new GroupData();
				gdataptr[k] = gdata;
				//printf("%d : %d , %d\n",gdata,&gdata->ID,gdata->unused);
				if(thisprefsversion<3)
					Read(file,gdata,sizeof(GroupData_v2));
				else
					Read(file,gdata,sizeof(GroupData));
				//Read(file,gdata,sizeof(GroupData));
				//printf("%d : %s\n",gdata->ID,gdata->name);
				//printf("Read :%s\n",gdata->name);
				gdata->flags[0]=FALSE;
				gdata->flags[1]=FALSE;
				if(gdata->ID==-1)
					out_folder=TRUE;
				else if(gdata->ID==-2)
					sent_folder=TRUE;
				else if(gdata->ID==-3)
					delete_folder=TRUE;
				//printf("ver: %f ID: %d\n",account.version,gdata->serverID);
				//printf("ID: %d\n",gdata->serverID);

				// but if this folder has otherwise been deleted, they need to be created again!
				makeFolder(gdata->ID);

			}
		}

		BOOL accread = FALSE;
		if(FORCEPATCH1) {
			if(thisprefsversion<4) {
				printf("Patching..\n");
				int endbit = 1676;
				Read(file,&account,5676-endbit);
				ChangeFilePosition(file,156,OFFSET_CURRENT);
				char * ptr = (char *)&account;
				ptr = ptr + 5676 - endbit;
				Read(file,ptr,endbit);
				accread=TRUE;
			}
			else
				printf("FORCEPATCH1 only needed for prefs versions previous to v4\n");
		}
		if(!accread)
			Read(file,&account,sizeof(Account));
		if(account.readheader_first==0) {
			account.readheader_first=1;
			account.readheader_type=1;
			strcpy(account.readheaders,"From|To|Newsgroups|Date|Subject|Followup-To|Reply-To");
		}
		if(thisprefsversion<4)
			strcpy(account.followup_text,CatalogStr(MSG_DEFAULT_FOLLOWUP,MSG_DEFAULT_FOLLOWUP_STR));
		if(nlisttree==FALSE) {
			account.grouplistType = 0;
			account.mdata_view = 0;
		}
		// insert group data
		if(gdataptr!=0) {
			for(k=0;k<entries;k++) {
				if(account.version<1.325)
					gdataptr[k]->serverID = 0;
				insertGdata(gdataptr[k],FALSE);
			}
			//FreeVec(gdataptr);
			delete [] gdataptr;
			gdataptr=NULL;
		}

		if(!out_folder)
			createOutgoing();
		if(!sent_folder)
			createSent();
		if(!delete_folder)
			createDeleted();

		for(k=0;k<NSIGS;k++) {
			int temp = 0;
			if(Read(file,&temp,sizeof(int))==0)
				break;
			if(temp>0) {
				sigs[k] = new char[temp+256];
				if(sigs[k]!=0) {
					/*if(Read(file,sigs[k],temp)==0)
						break;*/
					int read = Read(file,sigs[k],temp);
					if(read <= 0) {
						sigs[k][0] = 0;
						break;
					}
					else
						sigs[k][read] = 0;
				}
				else
					ChangeFilePosition(file,temp,OFFSET_CURRENT);
			}
		}
		if(account.mimeprefsno>256)
			account.mimeprefsno=0;
		for(k=0;k<account.mimeprefsno;k++) {
			account.mimeprefs[k]=new MIMEPrefs;
			if(Read(file,account.mimeprefs[k],sizeof(MIMEPrefs))<=0) {
				delete account.mimeprefs[k];
				account.mimeprefsno=k;
				break;
			}
		}
		if(account.linelength<4)
			account.linelength=72;
		else if(account.linelength>256)
			account.linelength=256;
		if(account.version<1.315 || account.port==0)
			account.port=119;
	}
	else {
		account.readheader_first=1;
		account.readheader_type=1;
		strcpy(account.readheaders,"From|To|Newsgroups|Date|Subject|Followup-To|Reply-To");
		//outgoing
		createOutgoing();
		createSent();
		createDeleted();
		strcpy(account.name,CatalogStr(MSG_DEFAULT_FROM,MSG_DEFAULT_FROM_STR));
		strcpy(account.email,"me@wherever.com");
		strcpy(account.nntp,"news");
		strcpy(account.nntppost,"");
		strcpy(account.smtp,"mail");
		strcpy(account.domain,"userid");
		account.nntp_auth=FALSE;
		strcpy(account.user,"");
		strcpy(account.password,"");
		//account.usesig=TRUE;
		account.sentID=0;
		Locale * locale = OpenLocale(NULL);
		if(locale!=0)
			account.timezone=-(int)(locale->loc_GMTOffset/60);
		else
			account.timezone=0;
		account.bst=FALSE;
		strcpy(account.followup_text,CatalogStr(MSG_DEFAULT_FOLLOWUP,MSG_DEFAULT_FOLLOWUP_STR));
		account.killfiles=0;
		account.mimeprefsno=0;
		account.linelength=72;
		account.rewrap=0; // turn off as default due to it sometimes causing problems (eg, lines starting with a smiley when they aren't quoted bits..)
		account.xno=2; // put in X-No-Archive on replies that were X-No-Archive'd
		account.use_sizekill=0;
		account.multiplevws=TRUE;
		account.flags= Account::SNIPSIG | Account::LOGDEL;
		strcpy(account.defviewer,"sys:utilities/multiview \"%s\" FONTNAME=\"topaz\" FONTSIZE=8");
		account.port = 119;
	}
	if(account.listflags == 0)
		account.listflags = Account::LISTUPDATE | Account::LISTFLAGS | Account::LISTFROMGROUP | Account::LISTDATE | Account::LISTSUBJECT;
	if(account.weightHD<=0 || account.weightMESS<=0) {
		account.weightHD=50;
		account.weightMESS=100;
	}
	if(account.h_weightHD<=0 || account.weightATT<=0) {
		account.h_weightHD=100;
		account.weightATT=50;
	}
	if(account.weightGLIST<=0 || account.weightMLIST<=0) {
		account.weightGLIST=100;
		account.weightMLIST=200;
	}
	if(!(account.charset_write[0]))
		strcpy(account.charset_write,"iso-8859-1");
	set(grouplistGroup,MUIA_HorizWeight,account.weightGLIST);
	set(messageGroup,MUIA_HorizWeight,account.weightMLIST);
	if(file) {
		Close(file);
		file=NULL;
	}
	if(lock) {
		UnLock(lock);
		lock=NULL;
	}
	for (k=0;POPA_Builtin_Tables[k];k++)
		if(!(stricmp(account.charset_write,POPA_Builtin_Tables[k])))
			goto NoCheck;
	sprintf(charset_file,"PROGDIR:Charsets/%s.charset",account.charset_write);
	lock=Lock(charset_file,ACCESS_READ);
	if (lock)
	{
		UnLock(lock);
		lock=NULL;
	}
	else
	{
		MUI_Request(app,0,0,"Charset error!",CatalogStr(MSG_CONTINUE,MSG_CONTINUE_STR),CatalogStr(MSG_CHARSET_CANNOT_BE_ACCESSED,MSG_CHARSET_CANNOT_BE_ACCESSED_STR),account.charset_write);
		strcpy(account.charset_write,"iso-8859-1");
	}
NoCheck:
	if(account.sorttypedir==0) {
		account.sorttype=1;
		account.sorttypedir=1;
	}
	if(bakup)
	{
		//Execute("copy NewsCoasterData:.prefs NewsCoasterData:.prefs.bak",NULL,NULL);
		System("copy NewsCoasterData:.prefs NewsCoasterData:.prefs.bak", NULL);
	}
	if(exists("NewsCoasterData:.servers"))
		load_servers();
	else {
		// create from Account (legacy)
		Server *server = new Server();
		strcpy(server->nntp,account.nntp);
		strcpy(server->user,account.user);
		strcpy(server->password,account.password);
		server->nntp_auth = account.nntp_auth;
		server->port = account.port;
		server->def = TRUE;
		if(*account.nntppost==0)
			server->post = TRUE;
		else
			server->post = FALSE;
		server->ID = nextServerID++;
		strcpy(server->lastGotGroups,account.lastGotGroups);
		DoMethod(NLIST_acc_SERVERS,MUIM_NList_InsertSingle,server,MUIV_NList_Insert_Bottom);
		if(*account.nntppost!=0) {
			Server *serverp=new Server;
			strcpy(serverp->nntp,account.nntppost);
			strcpy(serverp->user,"");
			strcpy(serverp->password,"");
			serverp->nntp_auth = FALSE;
			serverp->port = 119;
			serverp->def = FALSE;
			serverp->post = TRUE;
			serverp->ID = nextServerID++;
			DoMethod(NLIST_acc_SERVERS,MUIM_NList_InsertSingle,serverp,MUIV_NList_Insert_Bottom);
		}
		save_servers();
	}
	load_killfile();
	account.save_data();

	char logfile[]="NewsCoaster:newscoaster.log";
	if((account.flags & Account::LOGDEL)!=0)
		DeleteFile(logfile);
	logFile=Open(logfile,MODE_READWRITE);
	if(logFile==0 && (account.flags & Account::LOGGING)!=0)
	{
		sprintf(status_buffer_g,CatalogStr(MSG_UNABLE_TO_OPEN_FILE,MSG_UNABLE_TO_OPEN_FILE_STR),logfile);
		MUI_RequestA(app,0,0,CatalogStr(MSG_ERROR,MSG_ERROR_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),status_buffer_g,0);
	}
	else if(logFile != 0)
	{
		ChangeFilePosition(logFile,0,OFFSET_END);
		if((account.flags & Account::LOGDEL)==0)
			nLog("\n\n");
	}
	nLog("NewsCoaster started\n");
	nLog("  version %s\n",version);

	resetMessageListFormat();
	return TRUE;
}

void Account::save_data() {
	nLog("Account::save_data() called\n");
	int k = 0;
	int entries = 0,trueentries = 0;
	BPTR file = NULL;
	int temp = 0;
	// initial stuff
	get(grouplistGroup,MUIA_HorizWeight,&account.weightGLIST);
	get(messageGroup,MUIA_HorizWeight,&account.weightMLIST);
	//save folder prefs
	GroupData * gdata;
	char filename[]="NewsCoasterData:.prefs";
	DeleteFile(filename);
	entries=0;
	getGdataEntries(&entries);
	getGdataTrueEntries(&trueentries);
	//Write info...
	file=Open(filename,MODE_NEWFILE);
	if(file)
	{
		nLog("  writing header\n");
		char Header[]="NRGD";
		Write(file,Header,5);
		Write(file,&prefsversion,sizeof(int));
		Write(file,&nextID,sizeof(int));
		Write(file,&trueentries,sizeof(int));
		nLog("  writing groups\n");
		if(entries>0)
		{
			for(k=0;k<entries;k++)
			{
				getGdata(k,&gdata);
				if(gdata==0)
					continue;
				Write(file,gdata,sizeof(GroupData));
			}
		}
		nLog("  writing account\n");
		account.version = ver;
		Write(file,this,sizeof(Account));
		nLog("  writing .sigs\n");
		for(k=0;k<NSIGS;k++)
		{
			if(sigs[k])
			{
				temp=strlen(sigs[k])+1;
				Write(file,&temp,sizeof(int));
				if(temp>0)
					Write(file,sigs[k],temp);
			}
			else
			{
				temp=0;
				Write(file,&temp,sizeof(int));
			}
		}
		nLog("  writing mimeprefs\n");
		if(mimeprefsno>0)
		{
			for(k=0;k<mimeprefsno;k++)
			{
				Write(file,mimeprefs[k],sizeof(MIMEPrefs));
			}
		}
		nLog("  writing complete\n");
	}
	if(file)
	{
		Close(file);
		file=NULL;
	}
	nLog("  finished\n");
}

void Account::save_tofile(char *filename) {
	nLog("Account::save_tofile((char *)%s) called\n",filename);
	int k = 0;
	BPTR file = NULL;
	int temp = 0;
	//save folder prefs
	DeleteFile(filename);
	int entries = 0;
	//Write info...
	file=Open(filename,MODE_NEWFILE);
	if(file)
	{
		char Header[]="NRGD";
		Write(file,Header,5);
		Write(file,&prefsversion,sizeof(int));
		Write(file,&nextID,sizeof(int));
		Write(file,&entries,sizeof(int));
		Write(file,this,sizeof(Account));
		for(k=0;k<NSIGS;k++)
		{
			// write empty sigs
			{
				temp=0;
				Write(file,&temp,sizeof(int));
			}
		}
		if(mimeprefsno>0)
		{
			for(k=0;k<mimeprefsno;k++)
			{
				Write(file,mimeprefs[k],sizeof(MIMEPrefs));
			}
		}
	}
	if(file)
	{
		Close(file);
		file=NULL;
	}
}

void free_data() {
	nLog("free_data() called\n");
	GroupData * gdata=NULL;
	StatusWindow * statusWindow = new StatusWindow(app,CatalogStr(MSG_SHUTTING_DOWN,MSG_SHUTTING_DOWN_STR));
	statusWindow->setText(CatalogStr(MSG_CLOSING_SOCKETS,MSG_CLOSING_SOCKETS_STR));
	closeSockets();
	int k = 0;
	// update deleted folder to 0 entries (see later for actual deletion)
	get_gdata(&gdata,-3);
	gdata->nummess=0;
	gdata->num_unread=0;

	statusWindow->setText(CatalogStr(MSG_SAVING_PREFS,MSG_SAVING_PREFS_STR));
	account.save_data();

	statusWindow->setText(CatalogStr(MSG_SAVING_KILLFILE,MSG_SAVING_KILLFILE_STR));
	save_killfile();

	statusWindow->setText(CatalogStr(MSG_SAVING_SERVERS,MSG_SAVING_SERVERS_STR));
	save_servers();
	getGdataDisplayed(&gdata);
	if(gdata->flags[1]) {
		statusWindow->setText(CatalogStr(MSG_WRITING_INDEX,MSG_WRITING_INDEX_STR));
		write_index(0);
	}

	statusWindow->setText(CatalogStr(MSG_FREEING_MEMORY,MSG_FREEING_MEMORY_STR));
	for(k=0;k<NSIGS;k++) {
		if(sigs[k]) {
			delete [] sigs[k];
			sigs[k]=NULL;
		}
	}
	freeServerCycle();
	if(currentUser != 0)
	{
		delete currentUser;
		currentUser = NULL;
	}
	ViewWindow::freeStatics();
	WriteWindow::freeStatics();

	statusWindow->setText(CatalogStr(MSG_DELETING_FILES,MSG_DELETING_FILES_STR));
	//System("delete *>NIL: >NIL: NewsCoasterData:nc_tempfile#?", NULL);
	System("delete *>NIL: >NIL: NewsCoasterData:deleted/#?",	NULL);
	statusWindow->setText("OK");
	writeEmptyIndex(-3);

	// remove assign
	/*lock=Lock("PROGDIR:",ACCESS_READ);
	if(lock)
	{
		RemAssignList("NewsCoaster",lock);
		UnLock(lock);
		lock=NULL;
	}*/
	/*Execute("assign NewsCoaster: >NIL:",NULL,NULL);
	Execute("assign NewsCoasterData: >NIL:",NULL,NULL);*/

	delete statusWindow;
	nLog("  free_data() completed\n");
}

void update_mdata_instances(int gID,int mID,int dst_gID,int dst_mID) {
	ViewWindow::checkDeleted(gID,mID);

	{
		int entries = 0;
		get(NLIST_search_RES,MUIA_NList_Entries,&entries);
		for(int i=0;i<entries;i++) {
			MessageListData *mdata_in_list = NULL;
			DoMethod(NLIST_search_RES,MUIM_NList_GetEntry,i,&mdata_in_list);
			if(mID == mdata_in_list->ID && gID == mdata_in_list->flags[6]) {
				if( dst_gID == -1 ) {
					DoMethod(NLIST_search_RES,MUIM_NList_Remove,i);
					DoMethod(NLIST_search_RES,MUIM_NList_Redraw,MUIV_NList_Redraw_All);
				}
				else {
					mdata_in_list->flags[6] = dst_gID;
					mdata_in_list->ID = dst_mID;
				}
				break;
			}
		}
	}
	{
		int entries = 0;
		// check join window
		get(NLIST_joinmsgs_MSGS,MUIA_NList_Entries,&entries);
		for(int i=0;i<entries;i++) {
			MessageListData *mdata_in_list = NULL;
			DoMethod(NLIST_joinmsgs_MSGS,MUIM_NList_GetEntry,i,&mdata_in_list);
			if(mID == mdata_in_list->ID && gID == mdata_in_list->flags[6]) {
				if( dst_gID == -1 ) {
					DoMethod(NLIST_joinmsgs_MSGS,MUIM_NList_Remove,i);
					DoMethod(NLIST_joinmsgs_MSGS,MUIM_NList_Redraw,MUIV_NList_Redraw_All);
				}
				else {
					mdata_in_list->flags[6] = dst_gID;
					mdata_in_list->ID = dst_mID;
				}
				break;
			}
		}
	}
}

/* If the group has been marked as needing to have its index written out, then
 * we do so. Also we rethread the index (for threaded display mode). This is in
 * case a change was to remove messages from the index.
 */
void write_index_if_changed() {
	GroupData *gdata = NULL;
	getGdataDisplayed(&gdata);
	if(gdata->flags[1]) {
		write_index(0);
		threadView();
		setEnabled();
	}
}

/*void delete_mess(GroupData * gdata,MessageListData * mdata,BOOL visible) {
	sleepAll(TRUE);
	if(visible) {
		nLog("delete_mess((GroupData *)%d,(MessageListData)%d,(BOOL)%d) called\n",gdata,mdata,visible);
		setGdataQuiet(TRUE);
		setMdataQuiet(TRUE);
	}

	delete_file(gdata,mdata);
	update_mdata_instances(gdata->ID,mdata->ID,-1,-1);
	write_index_delete(gdata,mdata);
	gdata->nummess--;
	if(mdata->flags[1])
		gdata->num_unread--;

	mdata = NULL;

	if(visible)
		setEnabled();

	if(visible) {
		redrawGdataAll();
		write_index_if_changed();
		setGdataQuiet(FALSE);
		setMdataQuiet(FALSE);
	}
	sleepAll(FALSE);
}*/

void delete_complete() {
	nLog("delete_complete() called\n");
	write_index_if_changed();
	account.save_data();
}

void move_complete(int type) {
	nLog("move_complete((int)%d) called\n",type);
	if( type == 0 ) {
		write_index_if_changed();
	}
	account.save_data();
}

/* Deletes messages.
 * Iff multi is set to true, then time-consuming tasks (eg, writing out the
 * index) are not performed. These *must* be done by the caller afterwards,
 * however - this should be done by calling delete_complete().
 */
void delete_mess_n(GroupData * gdata,MessageListData ** mdataptr,int n, BOOL multi) {
	nLog("delete_mess_n((GroupData *)%d,(MessageListData **)%d,(int)%d,(BOOL)%d) called\n",gdata,mdataptr,n,multi);
	sleepAll(TRUE);
	setGdataQuiet(TRUE);
	setMdataQuiet(TRUE);

	delete_file_n(gdata,mdataptr,n);
	for(int i=0;i<n;i++) {
		update_mdata_instances(gdata->ID,mdataptr[i]->ID,-1,-1);
		if(mdataptr[i]->flags[1])
			gdata->num_unread--;
	}
	gdata->nummess -= n;
	write_index_delete_multi(gdata,mdataptr,n);

	redrawGdataAll();
	if( !multi )
		delete_complete();
	setGdataQuiet(FALSE);
	setMdataQuiet(FALSE);
	sleepAll(FALSE);
}

/* Moves or copies a message from one group to another.
 * type should be one of: 0=move, 1=copy
 * Iff multi is set to true, then time-consuming tasks (eg, writing out the
 * index when moving) are not performed. These *must* be done by the caller
 * application afterwards, however - this should be done by calling
 * move_complete().
 */
/*void move(GroupData * src_gdata,GroupData * dst_gdata,MessageListData * mdata,int type,BOOL multi) {
	nLog("move((GroupData *)%d,(GroupData *)%d,(MessageListData *)%d,(int)%d,(BOOL)%d) called\n",src_gdata,dst_gdata,mdata,type,multi);
	sleepAll(TRUE);

	GroupData * gdatatemp=NULL;
	char buffer[256]="";
	char buffer2[256]="";
	char command[512]="";
	int l = 0;
	getGdataDisplayed(&gdatatemp);
	getMdataActivePos(&l);
	getFilePath(buffer,src_gdata->ID,mdata->ID);

	for(;;) { // we want a file that doesn't exist!
		getFilePath(buffer2,dst_gdata->ID,dst_gdata->nextmID);
		if(exists(buffer2))
			dst_gdata->nextmID++;
		else
			break;
	}

	if(type==0) {
		if(DEBUG)
			printf("DEBUG: calling: Rename(\"%s\",\"%s\")\n",buffer,buffer2);
		Rename(buffer,buffer2);
		if(DEBUG) {
			sprintf(command,"The program has paused after moving a message.\nIf you are having problems with this, please check to see if the file\n%s still exists,\nand also if the file\n%s has now been created.\nWhat are the contents of those files?",buffer,buffer2);
			MUI_RequestA(app,0,0,"DEBUG",CatalogStr(MSG_CONTINUE,MSG_CONTINUE_STR),command,0);
		}
	}
	else if(type==1) {
		char tempname[64] = "";
		sprintf(tempname,"NewsCoasterData:news_temp_%d",mdata->ID);
		sprintf(command,"Copy %s to %s",buffer,tempname);
		if(DEBUG)
			printf("DEBUG: executing: %s\n",command);
		Execute(command,NULL,NULL);
		while(!Rename(tempname,buffer2)) {
			dst_gdata->nextmID++;
			getFilePath(buffer2,dst_gdata->ID,dst_gdata->nextmID);
		}
	}
	int nmID = dst_gdata->nextmID;
	if( type == 0 ) {
		update_mdata_instances(src_gdata->ID,mdata->ID,dst_gdata->ID,nmID);
	}
	if( type == 0 && gdatatemp->ID != src_gdata->ID )
		set_and_read(src_gdata);
	int old_pos = get_mdata_pos(mdata->ID);
	MessageListData new_mdata = *mdata;
	new_mdata.ID = nmID;
	dst_gdata->nextmID++;
	if(dst_gdata->ID!=-3) // articles in 'deleted' remember where they came from
		new_mdata.flags[6]=dst_gdata->ID;
	dst_gdata->nummess++;
	if(new_mdata.flags[1])
		dst_gdata->num_unread++;
	write_index_single(dst_gdata,&new_mdata);
	setEnabled();
	if(type==0) {
		if(old_pos != -1) {
			DoMethod(NLIST_messagelistdata,MUIM_NList_Remove,old_pos);
			src_gdata->flags[1] = TRUE;
			src_gdata->nummess--;
			if(new_mdata.flags[1])
				src_gdata->num_unread--;
		}
	}
	sleepAll(FALSE);
	if(!multi)
		move_complete(type);
	redrawGdataAll();
	if( type == 0 && !multi && gdatatemp->ID != src_gdata->ID )
		set_and_read(gdatatemp);
	setMdataActive(l);
}*/

/* Moves or copies messages from one group to another.
 * type should be one of: 0=move, 1=copy
 * Iff multi is set to true, then time-consuming tasks (eg, writing out the
 * index when moving) are not performed. These *must* be done by the caller
 * afterwards, however - this should be done by calling move_complete().
 */
void move_n(GroupData * gdata,GroupData * gdata2,MessageListData ** mdataptr,int n,int type,StatusWindow *statusWindow,BOOL multi) {
	nLog("move_n((GroupData *)%d,(GroupData *)%d,(MessageListData *)%d,(int)%d,(int)%d,(StatusWindow *)%d,(int)%d) called\n",gdata,gdata2,mdataptr,n,type,statusWindow,multi);
	sleepAll(TRUE);
	char buffer[64]="";
	char buffer2[64]="";
	char command[256]="";
	GroupData * gdatatemp=NULL;
	getGdataDisplayed(&gdatatemp);

	if(gdata->ID==-1)
		sprintf(buffer,"NewsCoasterData:outgoing/news_");
	else if(gdata->ID==-2)
		sprintf(buffer,"NewsCoasterData:sent/news_");
	else if(gdata->ID==-3)
		sprintf(buffer,"NewsCoasterData:deleted/news_");
	else if(gdata->ID>=0)
		sprintf(buffer,"NewsCoasterData:folder_%d/news_",gdata->ID);
	char *ptr = &buffer[ strlen(buffer) ];

	if(gdata2->ID==-1)
		sprintf(buffer2,"NewsCoasterData:outgoing/news_");
	else if(gdata2->ID==-2)
		sprintf(buffer2,"NewsCoasterData:sent/news_");
	else if(gdata2->ID==-3)
		sprintf(buffer2,"NewsCoasterData:deleted/news_");
	else if(gdata2->ID>=0)
		sprintf(buffer2,"NewsCoasterData:folder_%d/news_",gdata2->ID);
	char *ptr2 = &buffer2[ strlen(buffer2) ];

	index_handler * ih = new index_handler(gdata2);
	// move
	for(int i=0;i<n;i++) {
		do_input();
		if( ( statusWindow != 0 && statusWindow->isAborted()==TRUE ) || running==FALSE )
			break;

		for(;;) { // we want a file that doesn't exist!
			sprintf(ptr,"%d",mdataptr[i]->ID);
			sprintf(ptr2,"%d",gdata2->nextmID);
			if(exists(buffer2))
				gdata2->nextmID++;
			else
				break;
		}

		MessageListData new_mdata = *mdataptr[i];
		new_mdata.ID = gdata2->nextmID;
		gdata2->nextmID++;

		if( type == 0 )
		{
			update_mdata_instances(gdata->ID,mdataptr[i]->ID,gdata2->ID,new_mdata.ID);
			Rename(buffer,buffer2);
		}
		else if(type==1)
		{
			sprintf(command,"Copy %s to %s",buffer,buffer2);
			printf("Copy %s to %s\n",buffer,buffer2);
			//Execute(command,NULL,NULL);
			System(command, NULL);
		}
		if(gdata2->ID!=-3) // articles in 'deleted' remember where they came from
			new_mdata.flags[6]=gdata2->ID;

		if(new_mdata.flags[1])
			gdata2->num_unread++;
		ih->write(&new_mdata);

		if(type==0) {
			gdata->nummess--;

			if(new_mdata.flags[1])
				gdata->num_unread--;
		}

		gdata2->nummess++;
		redrawGdata(gdata);
		redrawGdata(gdata2);

	}

	if(ih) {
		delete ih;
		ih = NULL;
	}

	sleepAll(FALSE);
	if(type==0) {
		write_index_delete_multi(gdata,mdataptr,n);
	}
	if( !multi ) {
		move_complete(type);
	}
}

void readgrouplist(Server *server,int filepos)
{
	nLog("readgrouplist((Server *)%d,(int)%d) called\n",server,filepos);
	Object *NLIST = NLIST_groupman;
	BPTR file=NULL;
	char buffer[MAXLINE]="";
	char temp[256]="";
	char filename[MAXFILENAME]="";
	const int CHUNK = 512;
	char **in_buffer = new char *[CHUNK];
	char **in_buffer_ptr = in_buffer;

	sprintf(filename,"NewsCoasterData:%s.gl",server->nntp);
	StatusWindow * statusWindow = new StatusWindow(app,CatalogStr(MSG_READING_GROUP_LIST,MSG_READING_GROUP_LIST_STR));
	statusWindow->setText(CatalogStr(MSG_READING_GROUP_LIST,MSG_READING_GROUP_LIST_STR));

	file=Open(filename,MODE_OLDFILE);
	int found=0;

	if(file)
	{
		ChangeFilePosition(file,filepos,OFFSET_BEGINNING);
		FFlush(file);
		set(NLIST,MUIA_NList_Quiet,TRUE);
		DoMethod(NLIST,MUIM_NList_Clear);
		for(;;)
		{
			if(!FGets(file,buffer,MAXLINE))
				break;
			if(*buffer == '\0')
				break;
			if (*buffer != '\n')
			{
				found++;
				char *ptr = buffer;
				while(*ptr != ' ')
					ptr++;
				int len = (int)ptr - (int)buffer;
				char *in = new char[len + 4];
				ptr = buffer;
				char *ptr_in = in;
				while(*ptr != ' ')
					*ptr_in++ = *ptr++;

				int len2 = strlen(ptr);

				*ptr_in++ = ' ';
				*ptr_in++ = ptr[len2-3];
				*ptr_in++ = '\0';

				*in_buffer_ptr++ = in;

				DoMethod(app,MUIM_Application_InputBuffered);
				if(found % CHUNK == 0)
				{

					DoMethod(NLIST,MUIM_NList_Insert,in_buffer,CHUNK,MUIV_NList_Insert_Bottom);
					in_buffer_ptr = in_buffer;

					do_input();
					if(statusWindow->isAborted()==TRUE || running==FALSE)
						break;

					sprintf(temp,CatalogStr(MSG_READ_GROUPS,MSG_READ_GROUPS_STR),found);
					statusWindow->setText(temp);
				}
			}
		}

		int rest = found % CHUNK;
		if(rest > 0)
		{

			DoMethod(NLIST,MUIM_NList_Insert,in_buffer,rest,MUIV_NList_Insert_Bottom);
			in_buffer_ptr = in_buffer;

			DoMethod(app,MUIM_Application_InputBuffered);

			sprintf(temp,CatalogStr(MSG_READ_GROUPS,MSG_READ_GROUPS_STR),found);
			statusWindow->setText(temp);
		}

		Close(file);
		file=NULL;
		statusWindow->setText(CatalogStr(MSG_SORTING_LIST,MSG_SORTING_LIST_STR));
		DoMethod(NLIST,MUIM_NList_Sort);
		set(NLIST,MUIA_NList_Quiet,FALSE);
		sprintf(temp,CatalogStr(MSG_FOUND_GROUPS_2,MSG_FOUND_GROUPS_2_STR),found);
		MUI_RequestA(app,0,0,CatalogStr(MSG_GROUP_MANAGER,MSG_GROUP_MANAGER_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),temp,0);
	}
	else
	{
		sprintf(buffer,CatalogStr(MSG_GROUP_LIST_DOES_NOT_EXIST,MSG_GROUP_LIST_DOES_NOT_EXIST_STR),account.nntp);
		MUI_RequestA(app,0,0,CatalogStr(MSG_GROUP_MANAGER,MSG_GROUP_MANAGER_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),buffer,0);
	}
	delete [] in_buffer;

	delete statusWindow;
	static char title[300]="";
	if(filepos!=0)
	{ // new newsgroups only, so don't allow writing!
		//set(BT_groupman_WRITE,MUIA_Disabled,TRUE);
		sprintf(title,CatalogStr(MSG_NEW_GROUP_FOR_SERVER,MSG_NEW_GROUP_FOR_SERVER_STR),server->nntp);
		set(wnd_groupman,MUIA_Window_Title,title);
	}
	else {
		//set(BT_groupman_WRITE,MUIA_Disabled,FALSE);
		sprintf(title,CatalogStr(MSG_GROUP_MANAGER_2,MSG_GROUP_MANAGER_2_STR),server->nntp);
		set(wnd_groupman,MUIA_Window_Title,title);
	}
	set(NLIST,MUIA_NList_Active,MUIV_NList_Active_Top);
}

/*void writegrouplist(Server *server) {
	nLog("writegrouplist((Server *)server) called\n",server);
	BPTR file=NULL;
	STRPTR out = NULL;
	char filename[MAXFILENAME]="";
	char filename2[MAXFILENAME]="";
	sprintf(filename,"NewsCoasterData:%s.gl",server->nntp);
	sprintf(filename2,"NewsCoasterData:%s.gl.bak",server->nntp);
	set(app,MUIA_Application_Sleep,TRUE);
	DeleteFile(filename2);
	Rename(filename,filename2);
	file=Open(filename,MODE_NEWFILE);
	int found=0;
	if(file) {
		FFlush(file);
		set(NLIST_groupman,MUIA_NList_Quiet,TRUE);
		int entries = 0,k = 0;
		get(NLIST_groupman,MUIA_NList_Entries,&entries);
		for(k=0;k<entries;k++) {
			DoMethod(NLIST_groupman,MUIM_NList_GetEntry,k,&out);
			if(out!=0) {
				Write(file,out,strlen(out));
				Write(file,"\n",1);
			}
		}
		Close(file);
		file=NULL;
		set(NLIST_groupman,MUIA_NList_Quiet,FALSE);
	}
	set(app,MUIA_Application_Sleep,FALSE);
}*/

void Write_file(char * filename,char * pos,int len) {
	// should be %d and not %s !
	nLog("Write_file((char *)%s,(char *)%d,(int)%d) called\n",filename,pos,len);
	DeleteFile(filename);
	BPTR file;
	file=Open(filename,MODE_NEWFILE);
	if(file)
	{
		Write(file,pos,len);
		Close(file);
		file=NULL;
	}
	nLog("  finished Write_file()\n");
}

char * selectNG(char * title,char * text,BOOL internal,BOOL poster) {
	nLog("selectNG((char *)%s,(char *)%s,(BOOL)%d,(BOOL)%d) called\n",title,text,internal,poster);
	char *str;
	int k = 0;
	int entries = 0;
	getGdataEntries(&entries);
	char ** ptrarr = new char *[entries+1];
	int *gnarr = new int [entries+1];
	int count=0;
	char *name = NULL;
	VOID * ptr = NULL;
	if(poster)
	{
		ptrarr[count]=CatalogStr(MSG_POSTER,MSG_POSTER_STR);
		gnarr[count++]=-1;
	}
	for(k=0;k<entries;k++)
	{
		getGdata(k,(GroupData **)&ptr);
		if(ptr==0)
			continue;
		if( ((GroupData *)ptr)->ID>=0 || internal==TRUE)
		{
			switch (((GroupData *)ptr)->ID)
			{
			case -1:
				name=CatalogStr(MSG_FOLDER_OUTGOING,MSG_FOLDER_OUTGOING_STR);
			break;
			case -2:
				name=CatalogStr(MSG_FOLDER_SENT,MSG_FOLDER_SENT_STR);
			break;
			case -3:
				name=CatalogStr(MSG_FOLDER_DELETED,MSG_FOLDER_DELETED_STR);
			break;
			default:
				name =((GroupData *)ptr)->name;
			}
			ptrarr[count]=name;
			gnarr[count++]=k;
		}
	}
	char * buttons[2];
	buttons[0]=new char[64]; strcpy(buttons[0],text);
	buttons[1]=new char[64]; strcpy(buttons[1],CatalogStr(MSG_CANCEL,MSG_CANCEL_STR));
	ChoiceList * choice = new ChoiceList(app,NULL,title,ptrarr,count,buttons,2);
	char * rstr = NULL;
	if(choice)
	{
		int res=choice->show();
		int sel=choice->getSelected();
		if(sel>=0 && res==0)
		{
//			char * str = ptrarr[sel];
			if (gnarr[sel]==-1)
				str=(char*)"poster";
			else
			{
				getGdata(gnarr[sel],(GroupData **)&ptr);
				str=((GroupData *)ptr)->name;
			}
			rstr = new char[strlen(str) + 1];
			strcpy(rstr,str);
		}
		delete choice;
	}
	/*for(k=0;k<count;k++) {
		if(ptrarr[k])
			delete ptrarr[k];
	}*/
	if(ptrarr)
		delete [] ptrarr;
		//FreeVec(ptrarr);
	if(gnarr)
		delete [] gnarr;
	for(k=0;k<2;k++)
	{
		if(buttons[k])
			delete buttons[k];
	}
	return rstr;
}

void getNGChoice(char * title,char * text,Object * STR,BOOL poster) {
	nLog("getNGChoice((char *)%s,(char *)%s,(Object *)%d,(BOOL)%d) called\n",title,text,STR,poster);
	char *str = selectNG(title,text,FALSE,poster);
	char buffer[1024] = "";
	if(str) {
		char *cstr = NULL;
		get(STR,MUIA_String_Contents,&cstr);
		if(cstr) {
			char *nstr = NULL;
			if(*cstr=='\0') {
				// text box is empty
				nstr = new char[strlen(str) + 1];
				strcpy(nstr,str);
			}
			else {
				char *temp = strstr(cstr,str);
				while(temp != 0) {
					BOOL ok = TRUE;
					ok = ok && ( temp == cstr || temp[-1] == ',' );
					if(ok)
						break;

					temp = strstr(&temp[1],str);
				}
				if(temp==0) {
					// append
					nstr = new char[strlen(cstr)+strlen(str)+2];
					sprintf(nstr,"%s,%s",cstr,str);
				}
				else {
					// remove
					char *temp2 = &temp[strlen(str)];
					char *temp3 = &cstr[strlen(cstr)];
					nstr = new char[strlen(cstr)+1];
					int t1 = (int)temp-(int)cstr;
					if(t1>0) {
						strncpy(nstr,cstr,t1);
						nstr[t1] = '\0';
					}
					if(t1>0 && temp2<temp3) {
						if(nstr[t1-1]==',' && *temp2==',')
							t1--;
					}
					if(temp2<temp3)
						strcpy(&nstr[t1],temp2);
					if(*nstr==',')
						sprintf(nstr,"%s",&nstr[1]);
					if(nstr[strlen(nstr)-1]==',')
						nstr[strlen(nstr)-1]=0;
				}
			}
			strcpy(buffer,nstr);
			set(STR,MUIA_String_Contents,buffer);
			if(nstr)
				delete [] nstr;
		}
		delete [] str;
	}
}

void getUserEmail(GroupData * gdata,char * buffer) {
	if(gdata->moreflags[1])
		sprintf(buffer,"\"%s\" ",gdata->alt_name);
	else
		sprintf(buffer,"\"%s\" ",account.name);
	if(gdata->moreflags[2])
		sprintf(buffer,"%s<%s>",buffer,gdata->alt_email);
	else
		sprintf(buffer,"%s<%s>",buffer,account.email);
}

void move_multi(int m,char * dest) {
	nLog("move_multi((int)%d,(char *)%s) called\n",m,dest);
	int k = 0,l = 0;
	GroupData * gdata=NULL;

	MessageListData * mdata=NULL;

	MessageListData ** mdatalist=NULL;
	getGdataDisplayed(&gdata);
	//int entries = 0;
	//get(NLIST_messagelistdata,MUIA_NList_Entries,&entries);
	int entries = getMdataVisibleEntries();
	/*int selent = 0;
	if(account.mdata_view==0) {
		MUI_NList_GetSelectInfo info;
		DoMethod(NLIST_messagelistdata,MUIM_NList_GetSelectInfo,&info);
		selent=info.vnum;
	}
	else
		DoMethod(LISTTREE_messagelistdata,MUIM_NList_Select,MUIV_NList_Select_All,MUIV_NList_Select_Ask,&selent);
	if(selent>0) {*/
	if(entries > 0) {
		char * str=NULL;
		if(dest==0) {
			if(m==0)
				str=selectNG(CatalogStr(MSG_MOVE_TO_WHICH_GROUP,MSG_MOVE_TO_WHICH_GROUP_STR),CatalogStr(MSG_MOVE,MSG_MOVE_STR),TRUE,FALSE);
			else
				str=selectNG(CatalogStr(MSG_COPY_TO_WHICH_GROUP,MSG_COPY_TO_WHICH_GROUP_STR),CatalogStr(MSG_COPY,MSG_COPY_STR),TRUE,FALSE);
		}
		else {
			str = new char[strlen(dest) + 1];
			strcpy(str,dest);
		}
		if(str) {
			GroupData * gdata2 = NULL;
			get_gdata(&gdata2,str);
			if(gdata2==0)
				return;
			setMdataQuiet(TRUE);
			if( (gdata->ID != gdata2->ID) || m==1) { // copying to the same folder should be allowed!
				mdatalist = new MessageListData *[entries];
				l=0;
				getGdataDisplayed(&gdata);
				for(k=0;k<entries;k++) {
					if(isMdataSelected(k)) {
						getMdataVisible(k,&mdata);
						mdatalist[l++] = mdata;
					}
				}
				StatusWindow *statusWindow = new StatusWindow(app,"");
				set(wnd_main,MUIA_Window_Sleep,TRUE);
				WriteWindow::sleepAll(TRUE);
				if(l>0) {
					if(m==0) {
						statusWindow->setText(CatalogStr(MSG_MOVING,MSG_MOVING_STR));
					}
					else
						statusWindow->setText(CatalogStr(MSG_COPYING,MSG_COPYING_STR));
					move_n(gdata,gdata2,mdatalist,l,m,statusWindow,FALSE);
				}
				delete statusWindow;
				set(wnd_main,MUIA_Window_Sleep,FALSE);
				WriteWindow::sleepAll(FALSE);
			}
			if(mdatalist) {
				delete [] mdatalist;
				mdatalist=NULL;
			}
			setMdataQuiet(FALSE);
			delete [] str;
		}
	}
	else {
		if(m==0)
			MUI_RequestA(app,0,0,CatalogStr(MSG_MOVE_MESSAGE,MSG_MOVE_MESSAGE_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_NO_MESSAGES_TO_MOVE,MSG_NO_MESSAGES_TO_MOVE_STR),0);
		else
			MUI_RequestA(app,0,0,CatalogStr(MSG_COPY_MESSAGE,MSG_COPY_MESSAGE_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_NO_MESSAGES_TO_COPY,MSG_NO_MESSAGES_TO_COPY_STR),0);
	}
}

void move_multi(int m) {
	move_multi(m,NULL);
}

void delete_multi(BOOL confirm,BOOL movedel) {
	nLog("delete_multi((BOOL)%d,(BOOL)%d) called\n",confirm,movedel);
			GroupData * gdata=NULL;
			GroupData * gdata2=NULL;
			MessageListData * mdata=NULL;
			MessageListData ** mdatalist=NULL;
			LONG result = 0;
			int k=0,l=0;
			getGdataDisplayed(&gdata);
			if(movedel)
            get_gdata(&gdata2,-3);
			int entries = getMdataVisibleEntries();
			if(entries>0) {
				if(confirm==TRUE && account.noconfirmdel==FALSE)
				{
					if(gdata->ID==-1)
						result=MUI_RequestA(app,0,0,CatalogStr(MSG_DELETE_MESSAGES,MSG_DELETE_MESSAGES_STR),CatalogStr(MSG_DELETE_OR_CANCEL,MSG_DELETE_OR_CANCEL_STR),CatalogStr(MSG_DELETE_NOT_SENT_ARE_YOU_SURE,MSG_DELETE_NOT_SENT_ARE_YOU_SURE_STR),0);
					else
						result=MUI_RequestA(app,0,0,CatalogStr(MSG_DELETE_MESSAGES,MSG_DELETE_MESSAGES_STR),CatalogStr(MSG_DELETE_OR_CANCEL,MSG_DELETE_OR_CANCEL_STR),CatalogStr(MSG_DELETE_MESSAGES_ARE_YOU_SURE,MSG_DELETE_MESSAGES_ARE_YOU_SURE_STR),0);
				}
				if(result==1 || confirm==FALSE || account.noconfirmdel==TRUE)
				{
					setMdataQuiet(TRUE);
					mdatalist = new MessageListData *[entries];
					l=0;
					for(k=0;k<entries;k++) {
						if(isMdataSelected(k)) {
							getMdataVisible(k,&mdata);
							/*mdatalist[l]=new MessageListData;
							*mdatalist[l++]=*mdata;*/
							mdatalist[l++] = mdata;
						}
					}
					if(l>0)
					{
						StatusWindow *statusWindow = new StatusWindow(app,CatalogStr(MSG_DELETING,MSG_DELETING_STR));
						statusWindow->setText(CatalogStr(MSG_DELETING,MSG_DELETING_STR));
						set(wnd_main,MUIA_Window_Sleep,TRUE);
						WriteWindow::sleepAll(TRUE);
						if(movedel)
							move_n(gdata,gdata2,mdatalist,l,0,statusWindow,FALSE);
						else {
							/*for(k=0;k<l;k++) {
								//DoMethod(app,MUIM_Application_InputBuffered);
								//if(k % 16 == 0) {
									if(statusWindow->isAborted() || running==FALSE)
										break;
									do_input();
								//}
								delete_mess(gdata,mdatalist[k],FALSE);
							}*/
							delete_mess_n(gdata,mdatalist,l,FALSE);
							//write_index_if_changed();
							//redrawGdataAll();
							//account.save_data(); // update group info on disk!
						}
						delete statusWindow;
						set(wnd_main,MUIA_Window_Sleep,FALSE);
						WriteWindow::sleepAll(FALSE);
					}
					if(mdatalist) {
						//FreeVec(mdatalist);
						delete [] mdatalist;
						mdatalist=NULL;
					}
					setMdataQuiet(FALSE);
				}
			}
			else
				MUI_RequestA(app,0,0,CatalogStr(MSG_DELETE_MESSAGE,MSG_DELETE_MESSAGE_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_NO_MESSAGES_TO_DELETE,MSG_NO_MESSAGES_TO_DELETE_STR),0);
}

void delete_multi(BOOL confirm)
{
	GroupData * gdata=NULL;
	BOOL movedel=FALSE;
	getGdataDisplayed(&gdata);
	if(gdata->ID!=-3)
		movedel=TRUE;
	delete_multi(confirm,movedel);
}

void undelete_multi(BOOL confirm) {
	nLog("undelete_multi((BOOL)%d) called\n",confirm);
	GroupData * gdata=NULL;
	GroupData * gdata2=NULL;
	MessageListData * mdata=NULL;
	MessageListData **mdatalist=NULL;
	LONG result = 0;
	//int entries = 0,k = 0,l = 0;
	int k=0,l=0;
   get_gdata(&gdata,-3);
	//get(NLIST_messagelistdata,MUIA_NList_Entries,&entries);
	int entries = getMdataVisibleEntries();
	if(entries>0) {
		if(confirm)
			result=MUI_RequestA(app,0,0,CatalogStr(MSG_UNDELETE_MESSAGES,MSG_UNDELETE_MESSAGES_STR),CatalogStr(MSG_UNDELETE_OR_CANCEL,MSG_UNDELETE_OR_CANCEL_STR),CatalogStr(MSG_UNDELETE_ARE_YOU_SURE,MSG_UNDELETE_ARE_YOU_SURE_STR),0);
		if(result==1 || confirm==FALSE) {
			setMdataQuiet(TRUE);
			mdatalist = new MessageListData *[entries];
			l=0;
			for(k=0;k<entries;k++) {
				if(isMdataSelected(k)) {
					getMdataVisible(k,&mdata);
					mdatalist[l++] = mdata;
				}
			}
			if(l>0) {
				StatusWindow *statusWindow = new StatusWindow(app,CatalogStr(MSG_UNDELETING,MSG_UNDELETING_STR));
				statusWindow->setText(CatalogStr(MSG_UNDELETING,MSG_UNDELETING_STR));
				set(wnd_main,MUIA_Window_Sleep,TRUE);
				WriteWindow::sleepAll(TRUE);
				for(k=0;k<l;k++) {
					//DoMethod(app,MUIM_Application_InputBuffered);
					//if(k % 16 == 0) {
						do_input();
						if(statusWindow->isAborted()==TRUE || running==FALSE)
							break;
					//}
		         get_gdata(&gdata2,mdatalist[k]->flags[6]);
					move_n(gdata,gdata2,&mdatalist[k],1,0,NULL,TRUE);
				}
				delete statusWindow;
				set(wnd_main,MUIA_Window_Sleep,FALSE);
				WriteWindow::sleepAll(FALSE);
				redrawGdataAll();
				move_complete(0);
				/*write_index(0);
				threadView();
				setEnabled();
				account.save_data();*/
			}
			if(mdatalist) {
				delete [] mdatalist;
				mdatalist=NULL;
			}
			setMdataQuiet(FALSE);
		}
	}
	else
		MUI_RequestA(app,0,0,CatalogStr(MSG_UNDELETE_MESSAGE,MSG_UNDELETE_MESSAGE_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_NO_MESSAGES_TO_UNDELETE,MSG_NO_MESSAGES_TO_UNDELETE_STR),0);
}

void deleteFolderContents(GroupData *gdata) {
	nLog("deleteFolderContents((GroupData *)%d) called\n",gdata);
	char buffer[300]="";
	if(gdata->ID==-1)
		sprintf(buffer,"delete >NIL: NewsCoasterData:outgoing/news_#?");
	else if(gdata->ID==-2)
		sprintf(buffer,"delete >NIL: NewsCoasterData:sent/news_#?");
	else if(gdata->ID==-3)
		sprintf(buffer,"delete >NIL: NewsCoasterData:deleted/news_#?");
	else if(gdata->ID>=0)
		sprintf(buffer,"delete >NIL: NewsCoasterData:folder_%d/news_#?",gdata->ID);
	//Execute(buffer,NULL,NULL);
		System(buffer, NULL);

	// read in ID history
	int s_entries = 0;
	get(NLIST_search_RES,MUIA_NList_Entries,&s_entries);
	int j_entries = 0;
	get(NLIST_joinmsgs_MSGS,MUIA_NList_Entries,&j_entries);
	Vector * vector = new Vector(2048);
	read_index(gdata,vector);
	int n_mdata = vector->getSize();
	for(int i=0;i<n_mdata;i++)
	{
		int j=0;
		MessageListData *mdata=((MessageListData **)vector->getData())[i];
		ViewWindow::checkDeleted(gdata->ID,mdata->ID);

		// check search window
		for(j=0;j<s_entries;j++)
		{
			MessageListData *mdata2 = NULL;
			DoMethod(NLIST_search_RES,MUIM_NList_GetEntry,j,&mdata2);
			if(mdata->ID == mdata2->ID && gdata->ID == mdata2->flags[6]) {
				DoMethod(NLIST_search_RES,MUIM_NList_Remove,j);
				DoMethod(NLIST_search_RES,MUIM_NList_Redraw,MUIV_NList_Redraw_All);
				break;
			}
		}
		// check join window
		for(j=0;j<j_entries;j++)
		{
			MessageListData *mdata2 = NULL;
			DoMethod(NLIST_joinmsgs_MSGS,MUIM_NList_GetEntry,j,&mdata2);
			if(mdata->ID == mdata2->ID && gdata->ID == mdata2->flags[6]) {
				DoMethod(NLIST_joinmsgs_MSGS,MUIM_NList_Remove,j);
				DoMethod(NLIST_joinmsgs_MSGS,MUIM_NList_Redraw,MUIV_NList_Redraw_All);
				break;
			}
		}
	}
	if(vector!=0)
	{
		for(int i=0;i<vector->getSize();i++)
			delete (MessageListData *)vector->getData()[i];
		delete (Vector *)vector;
	}

	writeEmptyIndex(gdata->ID);
	gdata->nummess = 0;
	gdata->num_unread = 0;
	gdata->flags[1] = FALSE;
	GroupData *gdata2;
	getGdataDisplayed(&gdata2);
	if(gdata->ID == gdata2->ID)
		read_index();

}

void deleteFolderContents() {
	nLog("deleteFolderContents() called\n");
	GroupData * gdata = NULL;
	getGdataDisplayed(&gdata);
	deleteFolderContents(gdata);
}

void killmess(GroupData * gdata,MessageListData * mdata,int type) {
	// type: -1=ask user, otherwise 0,1,2 = type, +4=this group only, else, all groups
	nLog("killmess((GroupData *)%d,(MessageListData *)%d,(int)%d) called\n",gdata,mdata,type);
	NewMessage message;
	strcpy(message.getThisHeader,"nntp-posting-host:");
	get_refs(&message,gdata,mdata,GETREFS_NONE);

	int res=0,sel=0;
	int k=0;
	int expiretype=0,expire=0;
	int b_mid = -1;
	int b_nph = -1;
	set(wnd_killfile,MUIA_Window_Sleep,TRUE);
	if(type==-1) {
		char * ptrarr[5];
		int count = 0;
		ptrarr[count]=new char[64]; strcpy(ptrarr[count],CatalogStr(MSG_MESSAGES_FROM_THIS_AUTHOR,MSG_MESSAGES_FROM_THIS_AUTHOR_STR));
			count++;
		ptrarr[count]=new char[64]; strcpy(ptrarr[count],CatalogStr(MSG_RESPONSE_TO_MESSAGE,MSG_RESPONSE_TO_MESSAGE_STR));
			count++;
		ptrarr[count]=new char[64]; strcpy(ptrarr[count],CatalogStr(MSG_ALL_WHITH_THE_SAME_SUBJECT,MSG_ALL_WHITH_THE_SAME_SUBJECT_STR));
			count++;
		{
			char *ptr = strchr(message.messageID,'@');
			if(ptr != 0 && ptr[1] != '\0') {
				ptr++;
				int len = 64 + strlen(ptr);
				ptrarr[count]=new char[len]; sprintf(ptrarr[count],CatalogStr(MSG_MESSAGES_FROM_SERVER,MSG_MESSAGES_FROM_SERVER_STR),ptr);
				(ptrarr[count])[ strlen( ptrarr[count] ) - 1 ] = '\0'; // don't display the final '>' - though we still put this in the killfile
				b_mid = count;
				count++;
			}
		}
		{
			if(*message.dummyHeader != '\0') {
				int len = 64 + strlen(message.dummyHeader);
				ptrarr[count]=new char[len]; sprintf(ptrarr[count],CatalogStr(MSG_MESSAGES_FROM_POSTING_HOST,MSG_MESSAGES_FROM_POSTING_HOST_STR),message.dummyHeader);
				b_nph = count;
				count++;
			}
		}
		char * buttons[3];
		int nbuttons=0,cancel=0;
		if(*message.newsgroups != '\0' && strchr(message.newsgroups,',') == NULL) {
			// not a crosspost
			buttons[0]=new char[64]; strcpy(buttons[0],CatalogStr(MSG_KILL_IN_ALL_GROUPS,MSG_KILL_IN_ALL_GROUPS_STR));
			buttons[1]=new char[64]; strcpy(buttons[1],CatalogStr(MSG_KILL_IN_THIS_GROUP,MSG_KILL_IN_THIS_GROUP_STR));
			buttons[2]=new char[64]; strcpy(buttons[2],CatalogStr(MSG_CANCEL,MSG_CANCEL_STR));
			nbuttons=3;
			cancel=2;
		}
		else {
			// a crosspost
			buttons[0]=new char[64]; strcpy(buttons[0],CatalogStr(MSG_KILL_IN_ALL_GROUPS,MSG_KILL_IN_ALL_GROUPS_STR));
			buttons[1]=new char[64]; strcpy(buttons[1],CatalogStr(MSG_CANCEL,MSG_CANCEL_STR));
			nbuttons=2;
			cancel=1;
		}
		Object * group = HGroup,
			Child, CY_wiz_EXPIREKILL=Cycle(CYA_acc_expirekill),
			Child, STR_wiz_EXPIREKILL=BetterStringObject, StringFrame, MUIA_String_Integer, account.defexpire, MUIA_String_MaxLen, 5, MUIA_String_Accept, "1234567890", MUIA_CycleChain, 1, End,
			End;
		DoMethod(CY_wiz_EXPIREKILL,MUIM_Notify,MUIA_Cycle_Active,0,
			STR_wiz_EXPIREKILL,3,MUIM_Set,MUIA_Disabled,TRUE);
		DoMethod(CY_wiz_EXPIREKILL,MUIM_Notify,MUIA_Cycle_Active,1,
			STR_wiz_EXPIREKILL,3,MUIM_Set,MUIA_Disabled,FALSE);
		DoMethod(CY_wiz_EXPIREKILL,MUIM_Notify,MUIA_Cycle_Active,2,
			STR_wiz_EXPIREKILL,3,MUIM_Set,MUIA_Disabled,FALSE);

		set(STR_wiz_EXPIREKILL,MUIA_Disabled,TRUE);
		set(CY_wiz_EXPIREKILL,MUIA_Cycle_Active,account.defexpiretype);
		ChoiceList * choice = new ChoiceList(app,wnd_main,CatalogStr(MSG_KILL,MSG_KILL_STR),ptrarr,count,buttons,nbuttons,group);
		if(choice) {
			res=choice->show();
			sel=choice->getSelected();
			get(CY_wiz_EXPIREKILL,MUIA_Cycle_Active,&expiretype);
			char buffer[256]="";
			strcpy(buffer,getstr(STR_wiz_EXPIREKILL));
			expire = atoi(buffer);
			if(res==cancel || res==-1) {
				res=-1;
				sel=-1;
			}
		}
		if(choice)
			delete choice;
		MUI_DisposeObject(group);
		for(k=0;k<count;k++) {
			if(ptrarr[k])
				delete [] ptrarr[k];
		}
		for(k=0;k<nbuttons;k++) {
			if(buttons[k])
				delete buttons[k];
		}
	}
	else {
		sel = type % 4;
		res = type & 4;
		expiretype=0;
		expire=0;
	}
	KillFile * kill=new KillFile();
	switch (sel)
	{
	case 0:
		// by author
		strcpy(kill->header,"From:");
		strcpy(kill->text,message.from);
		break;
	case 1:
		// responses
		strcpy(kill->header,"References:");
		strcpy(kill->text,message.messageID);
		break;
	case 2:
		// subject
		strcpy(kill->header,"Subject:");
		strcpy(kill->text,message.subject);
		break;
	default:
		if(b_mid != -1 && sel == b_mid) {
			// server
			char *ptr = strchr(message.messageID,'@');
			if(ptr != 0 && ptr[1] != '\0') {
				ptr++;
				strcpy(kill->header,"Message-ID:");
				strcpy(kill->text,ptr);
			}
			else {
				delete kill;
				kill = NULL;
			}
		}
		else if(b_nph != -1 && sel == b_nph) {
			// NNTP posting host
			strcpy(kill->header,"NNTP-Posting-Host:");
			strcpy(kill->text,message.dummyHeader);
		}
		else {
			delete kill;
			kill = NULL;
		}
		break;
	}
	if(kill != 0) {
		if(res==0)
			strcpy(kill->ngroups,"");
		else
			strcpy(kill->ngroups,message.newsgroups);
		kill->expiretype = expiretype;
		kill->expire = expire;
		account.defexpiretype = expiretype;
		account.defexpire = expire;
		// convert to upper case
		toUpper(kill->header);
		toUpper(kill->text);
		toUpper(kill->ngroups);
		DoMethod(NLIST_acc_KILLFILE,MUIM_NList_InsertSingle,kill,MUIV_NList_Insert_Bottom);
		save_killfile();
	}
	set(wnd_killfile,MUIA_Window_Sleep,FALSE);
}

void killmess(GroupData * gdata,MessageListData * mdata) {
	killmess(gdata,mdata,-1);
}

void exportAddress(GroupData * gdata,MessageListData * mdata) {
	nLog("exportAddress((GroupData *)%d,(MessageListData *)%d) called\n",gdata,mdata);
	char yamfile[300] = "YAM:.addressbook";
	char buffer[1024]="";
	strcpy(buffer,"copy >NIL: YAM:.addressbook YAM:.addressbook.bak");
	//Execute(buffer,NULL,NULL);
	System(buffer, NULL);

	BPTR file = NULL;
	file=Open(yamfile,MODE_OLDFILE);
	if(file) {
		NewMessage * newmess = new NewMessage;
		get_refs(newmess,gdata,mdata,GETREFS_NONE);
		char name[256]="";
		char email[256]="";
		get_email(name,newmess->from,GETEMAIL_NAMEEMAIL);
		get_email(email,newmess->from,GETEMAIL_EMAIL);
		sprintf(buffer,CatalogStr(MSG_EXPORT_ADDRESS_ARE_YOU_SURE,MSG_EXPORT_ADDRESS_ARE_YOU_SURE_STR),name,email);
		LONG result=MUI_RequestA(app,0,0,CatalogStr(MSG_EXPORT_ADDRESS,MSG_EXPORT_ADDRESS_STR),CatalogStr(MSG_YES_OR_CANCEL,MSG_YES_OR_CANCEL_STR),buffer,0);
		if(result==1) {
			ChangeFilePosition(file,0,OFFSET_END);
			Write(file,(UBYTE *)"\n",1);
			Write(file,(UBYTE *)"@USER ",6);
			Write(file,name,strlen(name));
			Write(file,(UBYTE *)"\n",1);
			Write(file,email,strlen(email));
			Write(file,(UBYTE *)"\n",1);
			Write(file,name,strlen(name));
			Write(file,(UBYTE *)"\n",1);
			Write(file,(UBYTE *)"\n\n\n\n\n\n",6);
			Write(file,(UBYTE *)"00000000\n\n\n",11);
			Write(file,(UBYTE *)"@ENDUSER",8);
		}
		Close(file);
		delete newmess;
	}
	else
		MUI_RequestA(app,0,0,CatalogStr(MSG_EXPORT_ADDRESS,MSG_EXPORT_ADDRESS_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_CANNOT_ACCESS_YAM_ADDRESSBOOK,MSG_CANNOT_ACCESS_YAM_ADDRESSBOOK_STR),0);
	/*if(lock)
		UnLock(lock);*/
}

void resetArticlePointers(GroupData * gdata) {
	nLog("resetArticlePointers((GroupData *)%d) called\n",gdata);
	gdata->flags[3]=0;
	gdata->flags[4]=0; // not really needed
	gdata->moreflags[3]=FALSE;
}

void resetArticlePointers() {
	nLog("resetArticlePointers() called\n");
	// do all groups
	int entries = 0;
	GroupData * gdata=NULL;
	getGdataEntries(&entries);
	for(int k=0;k<entries;k++) {
		getGdata(k,&gdata);
		if(gdata != 0) {
			if(gdata->ID >= 0)
				resetArticlePointers(gdata);
		}
	}
}

void jumpto_unread() {
	nLog("jumpto_unread() called\n");
	MessageListData * mdata = NULL;
	int entries = 0;
	get(NLIST_messagelistdata,MUIA_NList_Entries,&entries);
	if(entries==0)
		return;

	BOOL done = FALSE;
	if(account.mdata_view==0) {
		for(int k=0;k<entries;k++) {
			DoMethod(NLIST_messagelistdata,MUIM_NList_GetEntry,k,&mdata);
			if(mdata->flags[1]) {
				setMdataActive(k);
				done = TRUE;
				break;
			}
		}
	}
	else {
		MUI_NListtree_TreeNode *temp_tn = NULL;
		MUI_NListtree_TreeNode *tn = (MUI_NListtree_TreeNode *)DoMethod(LISTTREE_messagelistdata,MUIM_NListtree_GetEntry,MUIV_NListtree_GetEntry_ListNode_Root,MUIV_NListtree_GetEntry_Position_Head,0);
		setMdataQuiet(TRUE);
		for(int i=0; i<entries && tn!=0; i++) {
			mdata = (MessageListData *)tn->tn_User;
			if(mdata->flags[1]) {
				temp_tn = (MUI_NListtree_TreeNode *)DoMethod(LISTTREE_messagelistdata,MUIM_NListtree_GetEntry,tn,MUIV_NListtree_GetEntry_Position_Parent,0);
				DoMethod(LISTTREE_messagelistdata,MUIM_NListtree_Open,MUIV_NListtree_Open_ListNode_Parent,temp_tn,0);
				set(LISTTREE_messagelistdata,MUIA_NListtree_Active,tn);
				done = TRUE;
				break;
			}
			temp_tn=(MUI_NListtree_TreeNode *)DoMethod(LISTTREE_messagelistdata,MUIM_NListtree_GetEntry,tn,MUIV_NListtree_GetEntry_Position_Head,0);
			if(temp_tn!=0) // we have child
				tn = temp_tn;
			else { // no child
				do {
					temp_tn=(MUI_NListtree_TreeNode *)DoMethod(LISTTREE_messagelistdata,MUIM_NListtree_GetEntry,tn,MUIV_NListtree_GetEntry_Position_Next,0);
					if(temp_tn!=0) // next in list
						break;
					// end of this list, go to parent then next
					tn=(MUI_NListtree_TreeNode *)DoMethod(LISTTREE_messagelistdata,MUIM_NListtree_GetEntry,tn,MUIV_NListtree_GetEntry_Position_Parent,0);
				} while(tn!=0);
				if(tn!=0)
					tn = temp_tn;
			}
		}
		setMdataQuiet(FALSE);
	}
	if( !done )
		setMdataActive(0);
}

void correctDrawer(char *dest,char *src) {
	if(strchr(src,':')==0)
		sprintf(dest,"%s:",src);
	else {
		if(src[strlen(src)-1]==':' || src[strlen(src)-1]=='/')
			strcpy(dest,src);
		else
			sprintf(dest,"%s/",src);
	}
}

BOOL createDrawer(char * drawer) {
	char * p = drawer;
	char * p2 = NULL;
	BOOL success=TRUE;
	char buffer[300] = "";
	do {
		p2 = strchr(p,'/');
		if(p2==0)
			break;
		strncpy(buffer,drawer,(int)p2-(int)drawer); // don't include final '/'!
		buffer[(int)p2-(int)drawer]=0;
		if(!exists(buffer)) {
			// create this drawer
			BPTR lock = CreateDir(buffer);
			if(lock==0)
				return FALSE; // failed!
			UnLock(lock);
			lock=NULL;
		}
		p = p2+1;
	} while(*p != '\0');
	return TRUE;
}

void rot13(Object *ED) {
	int val = 0;
	get(ED,MUIA_TextEditor_AreaMarked,&val);
	if(val==FALSE)
		return;
	char *temp=(STRPTR)DoMethod(ED,MUIM_TextEditor_ExportText);
	if(temp) {
		int sx=0,sy=0,ex=0,ey=0;
		DoMethod(ED,MUIM_TextEditor_BlockInfo,&sx,&sy,&ex,&ey);
		int len=strlen(temp);
		if(ex<len && ey<len) {
			int cx=0,cy=0;
			char * cpos=temp;
			while(*cpos!=0) {
				if( (cy>sy && cy<ey) || (cy==sy && cx>=sx && sy!=ey) || (cy==ey && cx<ex && sy!=ey) || (sy==ey && cy==sy && cx>=sx && cx<ex) ) {
					if(*cpos >= 'a' && *cpos <= 'z') {
						*cpos -= 13;
						if(*cpos<'a')
							*cpos += 26;
					}
					else if(*cpos >= 'A' && *cpos <= 'Z') {
						*cpos -= 13;
						if(*cpos<'A')
							*cpos += 26;
					}
				}
				if(*cpos == '\n') {
					cx=0;
					cy++;
				}
				else
					cx++;
				cpos++;
			}
			set(ED,MUIA_TextEditor_Quiet,TRUE);
			DoMethod(ED,MUIM_TextEditor_ClearText);
			DoMethod(ED,MUIM_TextEditor_InsertText,temp,0);
			DoMethod(ED,MUIM_TextEditor_MarkText,sx,sy,ex,ey);
			set(ED,MUIA_TextEditor_Quiet,FALSE);
		}
		FreeVec(temp); // should be a FreeVec
	}
}

void callback_app_doublestart() {
		MUI_RequestA(app,0,0,"NewsCoaster",CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_ONLY_ONE_NEWSCOASTER_COPY_ALLOWED,MSG_ONLY_ONE_NEWSCOASTER_COPY_ALLOWED_STR),0);
}

void callback_ngchanged() {
	if(old_gdata) {
		if(old_gdata->flags[1])
			write_index(1);
	}
	GroupData *gdata = NULL;
	getGdataActive(&gdata);
	if(gdata!=0) {
		v_gdata=gdata;
		if(gdata->ID != gdata_readID) {
			read_index();
			//find oldest unread message (if there is)
			jumpto_unread();
		}
		gdata_readID=gdata->ID;
		getGdataActive(&old_gdata);
		setEnabled();
	}
}

void callback_newsort() {
	int val = 0;
	if(account.mdata_view==0)
		get(NLIST_messagelistdata,MUIA_NList_TitleClick,&val);
	else
		get(LISTTREE_messagelistdata,MUIA_NList_TitleClick,&val);

	int icol = getInternalColumn(val);
	int sorttype = 0;
	GroupData *gdata = NULL;
	getGdataDisplayed(&gdata);
	if(getSortType(&sorttype,icol,gdata->ID)) {
		if(account.sorttype == sorttype)
			account.sorttypedir = -account.sorttypedir;
		else {
			account.sorttype = sorttype;
			account.sorttypedir = 1;
		}
		set(wnd_main,MUIA_Window_Sleep,TRUE);
		DoMethod(NLIST_messagelistdata,MUIM_NList_Sort);
		set(wnd_main,MUIA_Window_Sleep,FALSE);
		threadView();
	}
}

void callback_joinmsgs_join()
{
		char *translated_string=NULL;
		nLog("JOINMSGS_JOIN:\n");
		int entries = 0;
			get(NLIST_joinmsgs_MSGS,MUIA_NList_Entries,&entries);
			if(entries > 0)
			{
				MessageListData *mdata = NULL;
				DoMethod(NLIST_joinmsgs_MSGS,MUIM_NList_GetEntry,0,&mdata);
				GroupData *gdata = NULL;
				getGdataDisplayed(&gdata);
				if(gdata != 0)
				{
					set(app,MUIA_Application_Sleep,TRUE);

					mdata = new MessageListData();
					mdata->init();
					mdata->flags[0]=FALSE;
					mdata->flags[2]=FALSE;
					mdata->flags[6]=-1;

					char *header = new char[10240];
					char *hptr = header;

					int r = 0;

					strcpy(mdata->from,CatalogStr(MSG_JOINED_MESSAGE,MSG_JOINED_MESSAGE_STR));
					translated_string = translateCharset((unsigned char *)mdata->from,NULL);
					if (translated_string)
						strcpy(mdata->from,translated_string);
					r = sprintf(hptr,"From: %s\n",mdata->from); hptr += r;

					char date[256] = "";
					DateHandler::get_datenow(date,getGMTOffset(),account.bst);
					DateHandler::read_date(&mdata->ds,date,mdata->c_date,mdata->c_time);
					r = sprintf(hptr,"Date: %s\n",date); hptr += r;

					strncpy(mdata->subject,getstr(STR_joinmsgs_SUBJECT),IHEADENDSUBJECT);
					mdata->subject[IHEADENDSUBJECT] = '\0';

					translated_string=translateCharset((unsigned char *)mdata->subject,NULL);
					if (translated_string)
						strcpy(mdata->subject,translated_string);
					r = sprintf(hptr,"Subject: %s\n",mdata->subject); hptr += r;
					nLog("subject: %s\n",mdata->subject);

					struct DateStamp ds;
					DateStamp(&ds);
					sprintf(mdata->messageID,"<joined_message_%d%d%d@null>",(int)ds.ds_Days,(int)ds.ds_Minute,rand() % 65536);
					r = sprintf(hptr,"Message-ID: %s\n",mdata->messageID); hptr += r;

					strcpy(mdata->newsgroups,CatalogStr(MSG_NONE,MSG_NONE_STR));
					r = sprintf(hptr,"Newsgroups: %s\n",mdata->newsgroups); hptr += r;

					r = sprintf(hptr,"Content-Type: text/plain;\n"); hptr += r;

					r = sprintf(hptr,"\n"); hptr += r;
					int headerlen = hptr - header;
printf("headerlen:%d\n",headerlen);
					mdata->size = 0;

					char *big_buffer = new char[big_bufsize_g + 1];
					BPTR file = NULL;
					BOOL ok = FALSE;
					char filename[MAXFILENAME] = "";
					for(;;) {
						getFilePath(filename,gdata->ID,gdata->nextmID);
						if(!exists(filename) && (file=Open(filename,MODE_NEWFILE)) != 0)
							break;
						gdata->nextmID++;
					}
					mdata->ID=gdata->nextmID;
					nLog("filename: %s\n",filename);
					nLog("nextmID %d\n",gdata->nextmID);
					nLog("     ID %d\n",mdata->ID);
					if(file) {
						Write(file,header,headerlen);
						mdata->size += headerlen;

						for(int i=0;i<entries;i++) {
							MessageListData *this_mdata = NULL;
 							DoMethod(NLIST_joinmsgs_MSGS,MUIM_NList_GetEntry,i,&this_mdata);

							char filename2[MAXFILENAME] = "";
							getFilePath(filename2,this_mdata->flags[6],this_mdata->ID);
							nLog("%d : %d : %d : %s\n",i,this_mdata->flags[6],this_mdata->ID,filename2);
							BPTR this_lock = Lock(filename2,ACCESS_READ);
							BPTR this_file = Open(filename2,MODE_OLDFILE);
							if(this_file != 0) {
								BOOL more = TRUE;
								BOOL header = TRUE;
								for(;;) {
									if(!FGets(this_file,big_buffer,big_bufsize_g)) {
										more = FALSE;
										break;
									}
									StripNewLine(big_buffer);
									if(header && *big_buffer == '\0')
										header = FALSE;
									else if(!header && *big_buffer != '\0') {
										int len = strlen(big_buffer);
										Write(file,big_buffer,len);
										Write(file,(UBYTE *)"\n",1);
										mdata->size += len + 1;
										break;
									}
								}
								FFlush(this_file);
								if(more) {
									for(;;) {
										//int read = Read(this_file,buffer,bufsize);
										int read = Read(this_file,big_buffer,big_bufsize_g);
										if(read <= 0)
											break;
										big_buffer[read] = '\0';
										Write(file,big_buffer,read);
										mdata->size += read;
									}
								}
								/*
								BOOL header = TRUE;
								BOOL body = FALSE;
								int n_nl = 0;
								for(;;) {
									if(!FGets(this_file,buffer,MAXLINE))
										break;
									StripNewLine(buffer);
									if(header && *buffer == '\0') {
										header = FALSE;
									}
									else if(!header) {
										if(*buffer == '\0') {
											if(body)
												n_nl++;
										}
										else {
											// line that is part of the body - write it out
											body = TRUE;
											if(n_nl > 0) {
												// write out newlines
												for(int j=0;j<n_nl;j++)
													Write(file,"\n",1);
												mdata->size += n_nl;
												n_nl = 0;
											}
											int len = strlen(buffer);
											Write(file,buffer,len);
											mdata->size+=len;
											Write(file,"\n",1);
											mdata->size++;
										}
									}
								}
								*/
								Close(this_file);
							}
							if(this_lock != 0)
								UnLock(this_lock);
						}

						gdata->nextmID++;
						Close(file);
						file = NULL;
						ok = TRUE;

						write_index_single(gdata,mdata);
						setEnabled();
						//printf("subject: %s\n",mdata->subject);
						//printf("     ID %d\n",mdata->ID);
						gdata->nummess++;

						GroupData * gdata2 = NULL;
						getGdataDisplayed(&gdata2);
						if(gdata2->ID==gdata->ID)
							threadView();
						redrawGdataAll();
					}
					set(app,MUIA_Application_Sleep,FALSE);
					delete [] big_buffer;
					delete [] header;
				}
			}
	set(wnd_joinmsgs,MUIA_Window_Open,FALSE);
}

void callback_joinmsgs_cancel() {
	set(wnd_joinmsgs,MUIA_Window_Open,FALSE);
}

void callback_acc_pagesig() {
	if(accountpage==FALSE) {
		int val = 0;
		get(pages_account,MUIA_Group_ActivePage,&val);
		if(val==ACCSIGPAGE) {
			if(sigs[0])
				DoMethod(ED_acc_SIG,MUIM_TextEditor_InsertText,sigs[0],0);
			accountpage=TRUE;
		}
	}
}

void callback_acc_addserver() {
	int val = 0;
	get(wnd_servers,MUIA_Window_Open,&val);
	if(val==FALSE) {
		set(STR_acc_NNTP,MUIA_String_Contents,"");
		set(STR_acc_PORT,MUIA_String_Contents,"119");
		set(STR_acc_USER,MUIA_String_Contents,"");
		set(STR_acc_PASS,MUIA_String_Contents,"");
		set(CM_acc_AUTH,MUIA_Selected,FALSE);
		set(wnd_servers,MUIA_Window_Open,TRUE);
	}
	else
		MUI_RequestA(app,0,0,CatalogStr(MSG_ADD_SERVER,MSG_ADD_SERVER_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_PLEASE_FINISH_EDITING_SERVER_BEFORE_ADDING_NEW,MSG_PLEASE_FINISH_EDITING_SERVER_BEFORE_ADDING_NEW_STR),0);
}

void callback_acc_editserver() {
	int val = 0;
	get(wnd_servers,MUIA_Window_Open,&val);
	if(val==FALSE && editServer==0) {
		int val2 = 0;
		get(NLIST_acc_SERVERS,MUIA_NList_Active,&val2);
		if(val2!=MUIV_NList_Active_Off) {
			Server *server;
			DoMethod(NLIST_acc_SERVERS,MUIM_NList_GetEntry,val2,&server);
			set(STR_acc_NNTP,MUIA_String_Contents,server->nntp);
			set(STR_acc_PORT,MUIA_String_Integer,server->port);
			set(STR_acc_USER,MUIA_String_Contents,server->user);
			set(STR_acc_PASS,MUIA_String_Contents,server->password);
			set(CM_acc_AUTH,MUIA_Selected,server->nntp_auth);
			set(wnd_servers,MUIA_Window_Open,TRUE);
			editServer = server;
		}
	}
	else
		MUI_RequestA(app,0,0,CatalogStr(MSG_EDIT_SERVER,MSG_EDIT_SERVER_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_PLEASE_FINISH_EDITING_SERVER_BEFORE_EDITING_NEW,MSG_PLEASE_FINISH_EDITING_SERVER_BEFORE_EDITING_NEW_STR),0);
}

void callback_acc_makedefserver() {
	int entries = 0;
	get(NLIST_acc_SERVERS,MUIA_NList_Entries,&entries);
	int val = 0;
	get(NLIST_acc_SERVERS,MUIA_NList_Active,&val);
			if(entries>0 && val!=MUIV_NList_Active_Off) {
				Server *server = NULL;
				int i=0;
				for(i=0;i<entries;i++) {
					DoMethod(NLIST_acc_SERVERS,MUIM_NList_GetEntry,i,&server);
					if(i==val)
						server->def=TRUE;
					else
						server->def=FALSE;
				}
				DoMethod(NLIST_acc_SERVERS,MUIM_NList_Redraw,MUIV_NList_Redraw_All);
				GroupData *active_gdata = NULL;
				getGdataActive(&active_gdata);
				// update article pointers for all default groups
				int result = MUI_RequestA(app,0,0,CatalogStr(MSG_RESET_ARTICLE_POINTERS,MSG_RESET_ARTICLE_POINTERS_STR),CatalogStr(MSG_RESET_OR_NOT,MSG_RESET_OR_NOT_STR),CatalogStr(MSG_DEFAULT_SERVER_CHANGED_RESET_ALL_POINTERS,MSG_DEFAULT_SERVER_CHANGED_RESET_ALL_POINTERS_STR),0);
				getGdataEntries(&entries);
				for(i=0;i<entries;i++) {
					GroupData *gdata = NULL;
					getGdata(i,&gdata);
					if(gdata!=0) {
						if(gdata->serverID==0) {
							if(result == 1) {
								//printf("Reset for group: %s (%d)\n",gdata->name,gdata->serverID);
								resetArticlePointers(gdata);
							}
							if(gdata == active_gdata)
								setGdataHelpText(active_gdata); // update help bubble text, since using a different newsserver
						}
					}
				}
			}
}

void callback_acc_makepostserver() {
	int entries = 0;
	get(NLIST_acc_SERVERS,MUIA_NList_Entries,&entries);
			int val = 0;
			get(NLIST_acc_SERVERS,MUIA_NList_Active,&val);
			if(entries>0 && val!=MUIV_NList_Active_Off) {
				Server *server = NULL;
				int i=0;
				for(i=0;i<entries;i++) {
					DoMethod(NLIST_acc_SERVERS,MUIM_NList_GetEntry,i,&server);
					if(i==val)
						server->def=TRUE;
					else
						server->def=FALSE;
				}
				DoMethod(NLIST_acc_SERVERS,MUIM_NList_Redraw,MUIV_NList_Redraw_All);
				GroupData *active_gdata = NULL;
				getGdataActive(&active_gdata);
				// update article pointers for all default groups
				int result = MUI_RequestA(app,0,0,CatalogStr(MSG_RESET_ARTICLE_POINTERS,MSG_RESET_ARTICLE_POINTERS_STR),CatalogStr(MSG_RESET_OR_NOT,MSG_RESET_OR_NOT_STR),CatalogStr(MSG_DEFAULT_SERVER_CHANGED_RESET_ALL_POINTERS,MSG_DEFAULT_SERVER_CHANGED_RESET_ALL_POINTERS_STR),0);
				getGdataEntries(&entries);
				for(i=0;i<entries;i++) {
					GroupData *gdata = NULL;
					getGdata(i,&gdata);
					if(gdata!=0) {
						if(gdata->serverID==0) {
							if(result == 1) {
								//printf("Reset for group: %s (%d)\n",gdata->name,gdata->serverID);
								resetArticlePointers(gdata);
							}
							if(gdata == active_gdata)
								setGdataHelpText(active_gdata); // update help bubble text, since using a different newsserver
						}
					}
				}
			}
}

void callback_acc_deleteserver() {
			int val = 0;
			get(NLIST_acc_SERVERS,MUIA_NList_Active,&val);
			if(val!=MUIV_NList_Active_Off)
			{
				int entries = 0;
				get(NLIST_acc_SERVERS,MUIA_NList_Entries,&entries);
				if(entries>1) {
					Server *server;
					DoMethod(NLIST_acc_SERVERS,MUIM_NList_GetEntry,val,&server);
					if(editServer!=server) {
						int result = MUI_RequestA(app,0,0,CatalogStr(MSG_DELETE_SERVER,MSG_DELETE_SERVER_STR),CatalogStr(MSG_DELETE_OR_CANCEL,MSG_DELETE_OR_CANCEL_STR),CatalogStr(MSG_DELETE_SERVER_ARE_YOU_SURE,MSG_DELETE_SERVER_ARE_YOU_SURE_STR),0);
						if(result == 1) {
							int def = server->def;
							int post = server->post;

							int reset = MUI_RequestA(app,0,0,CatalogStr(MSG_RESET_ARTICLE_POINTERS,MSG_RESET_ARTICLE_POINTERS_STR),CatalogStr(MSG_RESET_OR_NOT,MSG_RESET_OR_NOT_STR),CatalogStr(MSG_SERVER_DELETED_RESET_ARTICLE_POINTERS,MSG_SERVER_DELETED_RESET_ARTICLE_POINTERS_STR),0);
							getGdataEntries(&entries);
							GroupData *active_gdata = NULL;
							getGdataActive(&active_gdata);
							for(int i=0;i<entries;i++) {
								GroupData *gdata = NULL;
								getGdata(i,&gdata);
								if(gdata!=0) {
									if( (gdata->serverID==0 && def==TRUE) || gdata->serverID==server->ID ) {
										if( reset == 1 ) {
											//printf("Reset for group: %s (%d)\n",gdata->name,gdata->serverID);
											resetArticlePointers(gdata);
										}
										if(gdata == active_gdata)
											setGdataHelpText(active_gdata); // update help bubble text, since using a different newsserver
									}
									if( gdata->serverID==server->ID ) {
										//printf("Set group to default: %s (%d)\n",gdata->name,gdata->serverID);
										gdata->serverID = 0;
									}
								}
							}

							DoMethod(NLIST_acc_SERVERS,MUIM_NList_Remove,val);
							if(def==TRUE) { // make another default
								DoMethod(NLIST_acc_SERVERS,MUIM_NList_GetEntry,0,&server);
								server->def=TRUE;
							}
							if(post==TRUE) { // make another the posting
								//DoMethod(NLIST_acc_SERVERS,MUIM_NList_GetEntry,0,&server);
								server = getDefaultServer();
								server->post=TRUE;
							}
							DoMethod(NLIST_acc_SERVERS,MUIM_NList_Redraw,MUIV_NList_Redraw_All);
						}
					}
					else
						MUI_RequestA(app,0,0,CatalogStr(MSG_DELETE_SERVER,MSG_DELETE_SERVER_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_CANNOT_DELETE_SERVER_BEING_EDITED,MSG_CANNOT_DELETE_SERVER_BEING_EDITED_STR),0);
				}
				else
					MUI_RequestA(app,0,0,CatalogStr(MSG_DELETE_SERVER,MSG_DELETE_SERVER_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_CANNOT_DELETE_LAST_SERVER,MSG_CANNOT_DELETE_LAST_SERVER_STR),0);
			}
}

void callback_acc_cysig() {
			if(accountpage)
			{
				int val = 0;
				get(CY_acc_SIG,MUIA_Cycle_Active,&val);
				if(sigs[cy_current_sig]) {
					delete [] sigs[cy_current_sig];
					sigs[cy_current_sig]=NULL;
				}
				char *sig = (char *)DoMethod(ED_acc_SIG,MUIM_TextEditor_ExportText);
				sigs[cy_current_sig] = new char[ strlen(sig) + 1 ];
				strcpy(sigs[cy_current_sig],sig);
				FreeVec(sig);
				sig = NULL;
				cy_current_sig=val;
				DoMethod(ED_acc_SIG,MUIM_TextEditor_ClearText);
				if(sigs[val])
					DoMethod(ED_acc_SIG,MUIM_TextEditor_InsertText,sigs[val],MUIV_TextEditor_InsertText_Top);
			}
}

void callback_acc_readsig() {
	sleepAll(TRUE);
	char filename[MAXFILENAME] = "";
	BOOL ok = LoadASL(filename,CatalogStr(MSG_SELECT_FILE_TO_READ,MSG_SELECT_FILE_TO_READ_STR),(const char *)"",(const char *)"#?",FALSE);
	sleepAll(FALSE);
	if(ok)
		readInText(ED_acc_SIG,filename);
}

void callback_acc_killchanged() {
			int val = 0;
			get(NLIST_acc_KILLFILE,MUIA_NList_Active,&val);
			if(val!=MUIV_NList_Active_Off)
			{
				KillFile *ptr = NULL;
				DoMethod(NLIST_acc_KILLFILE,MUIM_NList_GetEntry,MUIV_NList_GetEntry_Active,&ptr);
				set(STR_acc_NEWKILLHEAD,MUIA_String_Contents,((KillFile *)ptr)->header);
				set(STR_acc_NEWKILLTEXT,MUIA_String_Contents,((KillFile *)ptr)->text);
				set(STR_acc_NEWKILLGROUP,MUIA_String_Contents,((KillFile *)ptr)->ngroups);
				set(CY_acc_EXPIREKILL,MUIA_Cycle_Active,((KillFile *)ptr)->expiretype);
				sprintf(status_buffer_g,"%d",((KillFile *)ptr)->expire);
				set(STR_acc_EXPIREKILL,MUIA_String_Contents,status_buffer_g);
				//set(CY_acc_TYPEKILL,MUIA_Cycle_Active,(int)((KillFile *)ptr)->type);
				set(CY_acc_MATCHKILL,MUIA_Cycle_Active,(int)((KillFile *)ptr)->match);
				set(CY_acc_ACTIONKILL,MUIA_Cycle_Active,(int)((KillFile *)ptr)->action);
				set(CM_acc_SKIPKILL,MUIA_Selected,(((KillFile *)ptr)->carryon ? FALSE : TRUE));
			}
			else
			{
				set(STR_acc_NEWKILLHEAD,MUIA_String_Contents,"");
				set(STR_acc_NEWKILLTEXT,MUIA_String_Contents,"");
				set(STR_acc_NEWKILLGROUP,MUIA_String_Contents,"");
				set(CY_acc_EXPIREKILL,MUIA_Cycle_Active,0);
				set(STR_acc_EXPIREKILL,MUIA_String_Contents,"");
				//set(CY_acc_TYPEKILL,MUIA_Cycle_Active,0);
				set(CY_acc_MATCHKILL,MUIA_Cycle_Active,0);
				set(CY_acc_ACTIONKILL,MUIA_Cycle_Active,0);
				set(CM_acc_SKIPKILL,MUIA_Selected,TRUE);
			}
			set(wnd_account,MUIA_Window_ActiveObject,STR_acc_NEWKILLHEAD);
}

void callback_acc_newkill() {
			set(STR_acc_NEWKILLHEAD,MUIA_String_Contents,"");
			set(STR_acc_NEWKILLTEXT,MUIA_String_Contents,"");
			set(STR_acc_NEWKILLGROUP,MUIA_String_Contents,"");
			set(CY_acc_EXPIREKILL,MUIA_Cycle_Active,account.defexpiretype);
			set(STR_acc_EXPIREKILL,MUIA_String_Integer,account.defexpire);
			//set(CY_acc_TYPEKILL,MUIA_Cycle_Active,0);
			set(CY_acc_MATCHKILL,MUIA_Cycle_Active,0);
			set(CY_acc_ACTIONKILL,MUIA_Cycle_Active,0);
			set(CM_acc_SKIPKILL,MUIA_Selected,TRUE);
			set(NLIST_acc_KILLFILE,MUIA_NList_Active,MUIV_NList_Active_Off);
			set(wnd_account,MUIA_Window_ActiveObject,STR_acc_NEWKILLHEAD);
}

void callback_acc_delkill() {
			int val = 0;
			get(NLIST_acc_KILLFILE,MUIA_NList_Active,&val);
			if(val!=MUIV_NList_Active_Off)
			{
				int result = MUI_RequestA(app,0,0,CatalogStr(MSG_DELETE_KILL_ENTRY,MSG_DELETE_KILL_ENTRY_STR),CatalogStr(MSG_DELETE_OR_CANCEL,MSG_DELETE_OR_CANCEL_STR),CatalogStr(MSG_DELETE_KILL_ENTRY_ARE_YOU_SURE,MSG_DELETE_KILL_ENTRY_ARE_YOU_SURE_STR),0);
				if(result==1)
				{
					DoMethod(NLIST_acc_KILLFILE,MUIM_NList_Remove,val);
					set(STR_acc_NEWKILLHEAD,MUIA_String_Contents,"");
					set(STR_acc_NEWKILLTEXT,MUIA_String_Contents,"");
					set(STR_acc_NEWKILLGROUP,MUIA_String_Contents,"");
					set(CY_acc_EXPIREKILL,MUIA_Cycle_Active,0);
					set(STR_acc_EXPIREKILL,MUIA_String_Contents,"");
					//set(CY_acc_TYPEKILL,MUIA_Cycle_Active,0);
					set(CY_acc_MATCHKILL,MUIA_Cycle_Active,0);
					set(CY_acc_ACTIONKILL,MUIA_Cycle_Active,0);
					set(CM_acc_SKIPKILL,MUIA_Selected,TRUE);
					set(wnd_account,MUIA_Window_ActiveObject,STR_acc_NEWKILLHEAD);
				}
			}
}

void callback_acc_dupkill() {
			int val = 0;
			get(NLIST_acc_KILLFILE,MUIA_NList_Active,&val);
			if(val!=MUIV_NList_Active_Off)
			{
				KillFile *ptr = NULL;
				KillFile *ptr2 = NULL;
				DoMethod(NLIST_acc_KILLFILE,MUIM_NList_GetEntry,MUIV_NList_GetEntry_Active,&ptr);
				ptr2 = new KillFile();
				*((KillFile *)ptr2)=*((KillFile *)ptr);
				// not an identical copy, since we have new creation and lastused dates
				DateStamp(&((KillFile *)ptr2)->ds);
				DateStamp(&((KillFile *)ptr2)->lastused);
				DoMethod(NLIST_acc_KILLFILE,MUIM_NList_InsertSingle,ptr2,MUIV_NList_Insert_Active);
			}
}

void callback_acc_mimenew() {
	MIMEPrefs *ptr = new MIMEPrefs();
			strcpy(((MIMEPrefs *)ptr)->type,"text/plain");
			strcpy(((MIMEPrefs *)ptr)->viewer,"SYS:Utilities/Multiview \"%s\"");
			DoMethod(NLIST_acc_MIMEPREFS,MUIM_NList_InsertSingle,ptr,MUIV_NList_Insert_Bottom);
			set(NLIST_acc_MIMEPREFS,MUIA_NList_Active,MUIV_NList_Active_Bottom);
			set(STR_acc_MIME,MUIA_String_Contents,((MIMEPrefs *)ptr)->type);
			set(STR_acc_MIMEVIEW,MUIA_String_Contents,((MIMEPrefs *)ptr)->viewer);
}

void callback_acc_mimedel() {
	//DoMethod(NLIST_acc_MIMEPREFS,MUIM_NList_GetEntry,MUIV_NList_GetEntry_Active,&ptr);
	//if(ptr)
	//	delete ptr;
	DoMethod(NLIST_acc_MIMEPREFS,MUIM_NList_Remove,MUIV_NList_Remove_Active);
}

void callback_acc_mimeclick() {
	MIMEPrefs *ptr = NULL;
	DoMethod(NLIST_acc_MIMEPREFS,MUIM_NList_GetEntry,MUIV_NList_GetEntry_Active,&ptr);
	if(ptr) {
		set(STR_acc_MIME,MUIA_String_Contents,((MIMEPrefs *)ptr)->type);
		set(STR_acc_MIMEVIEW,MUIA_String_Contents,((MIMEPrefs *)ptr)->viewer);
	}
}

void callback_acc_mimestr() {
			int val = 0;
			get(NLIST_acc_MIMEPREFS,MUIA_NList_Active,&val);
			MIMEPrefs *ptr = new MIMEPrefs();
			strcpy(((MIMEPrefs *)ptr)->type,getstr(STR_acc_MIME));
			strcpy(((MIMEPrefs *)ptr)->viewer,getstr(STR_acc_MIMEVIEW));
			if( *((MIMEPrefs *)ptr)->type==0 && *((MIMEPrefs *)ptr)->viewer==0 )
				delete ptr;
			else
			{
				if(val==MUIV_NList_Active_Off)
					DoMethod(NLIST_acc_MIMEPREFS,MUIM_NList_InsertSingle,ptr,MUIV_NList_Insert_Bottom);
				else
					DoMethod(NLIST_acc_MIMEPREFS,MUIM_NList_ReplaceSingle,ptr,val,NOWRAP,ALIGN_LEFT);
				set(STR_acc_MIME,MUIA_String_Contents,"");
				set(STR_acc_MIMEVIEW,MUIA_String_Contents,"");
			}
			set(NLIST_acc_MIMEPREFS,MUIA_NList_Active,MUIV_NList_Active_Off);
}

void callback_kill_close() {
	//if(!downloading_headers) {
		set(wnd_killfile,MUIA_Window_Open,FALSE);
		get(CM_acc_SIZEKILL,MUIA_Selected,&account.use_sizekill);
		save_killfile();
	//}
	//else
	//	MUI_RequestA(app,0,0,CatalogStr(MSG_SAVE_KILLFILE,MSG_SAVE_KILLFILE_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_PLEASE_FINISH_DOWNLOADING_FIRST,MSG_PLEASE_FINISH_DOWNLOADING_FIRST_STR),0);
}

void callback_groupman_sub() {
	// copied & modified from newnewsgroup!
	char *temp = NULL;
	DoMethod(NLIST_groupman,MUIM_NList_GetEntry,MUIV_NList_GetEntry_Active,&temp);
	wordFirst(status_buffer_g,temp);
	set(STR_nng_NAME,MUIA_String_Contents,status_buffer_g);
	set(wnd_newnewsgroup,MUIA_Window_Open,TRUE);
	set(wnd_newnewsgroup,MUIA_Window_ActiveObject,STR_nng_NAME);
	nng_defserver = groupman_server->ID;
}

void callback_groupman_find() {
			int val = 0;
			int val2 = 0;
			get(NLIST_groupman,MUIA_NList_Entries,&val);
			get(NLIST_groupman,MUIA_NList_Active,&val2);
			strcpy(status_buffer_g,getstr(STR_groupman_FIND));
			BOOL yesno=FALSE;
			if(val>0 && val2>=0 && val2+1<val) {
				for(int k=val2+1;k<val;k++) {
					char *temp = NULL;
					DoMethod(NLIST_groupman,MUIM_NList_GetEntry,k,&temp);
					if(strstr(temp,status_buffer_g)) {
						set(NLIST_groupman,MUIA_NList_Active,k);
						yesno=TRUE;
						break;
					}
				}
			}
			if(!yesno)
				MUI_RequestA(app,0,0,CatalogStr(MSG_GROUP_MANAGER,MSG_GROUP_MANAGER_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_GROUP_NOT_FOUND,MSG_GROUP_NOT_FOUND_STR),0);
}

void callback_groupman_close() {
	set(wnd_groupman,MUIA_Window_Open,FALSE);
	DoMethod(NLIST_groupman,MUIM_NList_Clear); // needed to free up memory
}

void callback_search_start() {
	if(delete_dis)
		MUI_RequestA(app,0,0,CatalogStr(MSG_NOT_AVAILABLE,MSG_NOT_AVAILABLE_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_FUNCTION_NOT_AVAILABLE_WHEN_CONNECTED,MSG_FUNCTION_NOT_AVAILABLE_WHEN_CONNECTED_STR),0);
	else
	{
			int entries = 0;
			get(NLIST_search_NG,MUIA_NList_Entries,&entries);
			DoMethod(NLIST_search_RES,MUIM_NList_Clear);
			if(entries>0) {
				MUI_NList_GetSelectInfo info;
				DoMethod(NLIST_search_NG,MUIM_NList_GetSelectInfo,&info);
				int n_selected = info.vnum;
				if(n_selected == 0) {
					MUI_RequestA(app,0,0,CatalogStr(MSG_SEARCH_GROUPS,MSG_SEARCH_GROUPS_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_PLEASE_SELECT_SOME_GROUPS,MSG_PLEASE_SELECT_SOME_GROUPS_STR),0);
				}
				else
				{
				int where = 0;
				char head[256] = "",what[256] = "";
				int casesens = 0;
				get(CY_search_WHERE,MUIA_Cycle_Active,&where);
				strcpy(head,getstr(STR_search_HEAD));
				strcpy(what,getstr(STR_search_WHAT));
				get(CM_search_CASESENS,MUIA_Selected,&casesens);
				if(casesens==FALSE)
					toUpper(what);
				sleepAll(TRUE);
				StatusWindow *statusWindow = new StatusWindow(app,CatalogStr(MSG_SEARCHING,MSG_SEARCHING_STR));
				BOOL aborted = FALSE;
				for(int k=0;k<entries && !aborted;k++) {
					LONG sel = 0;
					DoMethod(NLIST_search_NG,MUIM_NList_Select,k,MUIV_NList_Select_Ask,&sel);
					if(sel==MUIV_NList_Select_On) {
						GroupData *gdata = NULL;
						DoMethod(NLIST_search_NG,MUIM_NList_GetEntry,k,&gdata);
						if(gdata)
						{
							sprintf(status_buffer_g,CatalogStr(MSG_SEARCHING_2,MSG_SEARCHING_2_STR),gdata->name);
							char *bufptr = &status_buffer_g[ strlen(status_buffer_g) ];
							statusWindow->setText(status_buffer_g);
							Vector * vector = new Vector(2048);
							read_index(gdata,vector);
							int entries2 = vector->getSize();
							MessageListData ** mdataptr = (MessageListData **)vector->getData();
							for(int l=0;l<entries2;l++)
							{
								DoMethod(app,MUIM_Application_InputBuffered);
								if(l % 16 == 0) {
									do_input();
									if(statusWindow->isAborted()==TRUE || running==FALSE)
									{
										aborted = TRUE;
										break;
									}
									sprintf(bufptr," ( %d / %d )",l,entries2);
									statusWindow->setText(status_buffer_g);
								}
								MessageListData *mdata = mdataptr[l];
								BOOL found=FALSE;
								//if(where < SEARCH_HEADER) { // quick version
								if(	where == SEARCH_FROM ||	where == SEARCH_NEWSGROUPS || where == SEARCH_SUBJECT || where == SEARCH_MESSAGEID )
								{ // quick version
									if(casesens)
									{
										switch(where)
										{
											case SEARCH_FROM:
												if( strstr(mdata->from,what) )
													found=TRUE;
												break;
											case SEARCH_NEWSGROUPS:
											{
												char newsgroups[MAXLINE] = "";
												get_newsgroups(newsgroups,gdata->ID,mdata->ID);
												if( strstr(newsgroups,what) )
												//if( strstr(mdata->newsgroups,what) )
													found=TRUE;
												break;
											}
											case SEARCH_SUBJECT:
												if( strstr(mdata->subject,what) )
													found=TRUE;
												break;
											case SEARCH_MESSAGEID:
												if( strstr(mdata->messageID,what) )
													found=TRUE;
												break;
											//case SEARCH_CONTENTTYPE:
											//	if( strstr(mdata->type,what) )
											//		found=TRUE;
											//	break;
										}
									}
									else
									{
										switch(where)
										{
											case SEARCH_FROM:
												if( stristr_q(mdata->from,what) )
													found=TRUE;
												break;
											case SEARCH_NEWSGROUPS:
											{
												char newsgroups[MAXLINE] = "";
												get_newsgroups(newsgroups,gdata->ID,mdata->ID);
												if( stristr_q(newsgroups,what) )
												//if( stristr_q(mdata->newsgroups,what) )
													found=TRUE;
												break;
											}
											case SEARCH_SUBJECT:
												if( stristr_q(mdata->subject,what) )
													found=TRUE;
												break;
											case SEARCH_MESSAGEID:
												if( stristr_q(mdata->messageID,what) )
													found=TRUE;
												break;
											//case SEARCH_CONTENTTYPE:
											//if( stristr_q(mdata->type,what) )
											//	found=TRUE;
											//break;
										}
									}
								}
								else
								{
									if(casesens)
										found = search_mess(gdata,mdata,what,casesens,where);
									else
										found = search_mess(gdata,mdata,what,casesens,where);
								}
								if(found)
								{
									MessageListData * mdata2 = new MessageListData();
									*mdata2 = *mdata;
									mdata2->flags[6] = gdata->ID;
									DoMethod(NLIST_search_RES,MUIM_NList_InsertSingle,mdata2,MUIV_NList_Insert_Bottom);
								}
							}

							for(int i=0;i<vector->getSize();i++)
								delete (MessageListData *)vector->getData()[i];
							delete (Vector *)vector;
						}
					}
				}
				//if(whatU)
				//	delete whatU;

				//set(wnd_main,MUIA_Window_Sleep,FALSE);
				//WriteWindow::sleepAll(FALSE);
				//set(wnd_search,MUIA_Window_Sleep,FALSE);
				sleepAll(FALSE);
				delete statusWindow;
				}
			}
	}
}

void callback_search_message() {
			int result = 0;
			get(NLIST_search_RES,MUIA_NList_Active,&result);
			if(result!=MUIV_NList_Active_Off) {
				MessageListData *mdata = NULL;
				DoMethod(NLIST_search_RES,MUIM_NList_GetEntry,MUIV_NList_GetEntry_Active,&mdata);
				int gID = mdata->flags[6];
				set(wnd_search,MUIA_Window_Sleep,TRUE);
				// now make sure we get an up to date copy of it!
				NewMessage newmess;
				get_refs(&newmess,gID,mdata->ID,GETREFS_LAST);
				newmess.copyToMessageListData(mdata);
				mdata->flags[6] = gID;
				ViewWindow *vw = NULL;
				if(ViewWindow::ptrs->getSize()==0 || account.multiplevws==TRUE)
					vw = new ViewWindow("NewsCoaster");
				else
					vw = ((ViewWindow **)ViewWindow::ptrs->getData())[0];
				//printf(":%d %d\n",gID,mdata->ID);
				vw->resetSingle(gID,mdata->ID);
				if(!vw->read())
					delete vw;
				set(wnd_search,MUIA_Window_Sleep,FALSE);
			}
}

void callback_search_sort() {
			int val = 0;
			get(NLIST_search_RES,MUIA_NList_TitleClick,&val);
			int icol = getInternalColumn(val);
			int sorttype = 0;
			//getGdataDisplayed(&gdata);
			if(getSortType(&sorttype,icol,0)) {
				if(sorttype_search == sorttype)
					sorttypedir_search = -sorttypedir_search;
				else {
					sorttype_search = sorttype;
					sorttypedir_search = 1;
				}
				//printf("%d %d %d\n",val,icol,sorttype_stats);
				set(wnd_search,MUIA_Window_Sleep,TRUE);
				DoMethod(NLIST_search_RES,MUIM_NList_Sort);
				set(wnd_search,MUIA_Window_Sleep,FALSE);
			}
}

void callback_stats_start() {
	if(delete_dis)
		MUI_RequestA(app,0,0,CatalogStr(MSG_NOT_AVAILABLE,MSG_NOT_AVAILABLE_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_FUNCTION_NOT_AVAILABLE_WHEN_CONNECTED,MSG_FUNCTION_NOT_AVAILABLE_WHEN_CONNECTED_STR),0);
	else {
			StatsList * statsList=NULL;
			int entries = 0;
			get(NLIST_stats_NG,MUIA_NList_Entries,&entries);
			DoMethod(NLIST_stats_RES,MUIM_NList_Clear);
			if(entries>0) {
				MUI_NList_GetSelectInfo info;
				DoMethod(NLIST_stats_NG,MUIM_NList_GetSelectInfo,&info);
				int n_selected = info.vnum;
				if(n_selected == 0) {
					MUI_RequestA(app,0,0,CatalogStr(MSG_STATISTICS,MSG_STATISTICS_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_PLEASE_SELECT_SOME_GROUPS,MSG_PLEASE_SELECT_SOME_GROUPS_STR),0);
				}
				else
				{
				int what = 0;
				get(CY_stats_WHAT,MUIA_Cycle_Active,&what);
				stats_what = what;
				set(wnd_main,MUIA_Window_Sleep,TRUE);
				WriteWindow::sleepAll(TRUE);
				set(wnd_stats,MUIA_Window_Sleep,TRUE);
				StatusWindow *statusWindow = new StatusWindow(app,CatalogStr(MSG_SCANNING_2,MSG_SCANNING_2_STR));
				BOOL aborted = FALSE;
				for(int k=0;k<entries && !aborted;k++) {
					LONG sel = 0;
					DoMethod(NLIST_stats_NG,MUIM_NList_Select,k,MUIV_NList_Select_Ask,&sel);
					if(sel==MUIV_NList_Select_On) {
						GroupData *gdata = NULL;
						DoMethod(NLIST_stats_NG,MUIM_NList_GetEntry,k,&gdata);
						if(gdata) {
							char * cmp = NULL;
							NewMessage newmessageTemp;
//							char unknown[] = "Unknown";
							char *unknown = CatalogStr(MSG_UNKNOWN,MSG_UNKNOWN_STR);
							sprintf(status_buffer_g,CatalogStr(MSG_SCANNING,MSG_SCANNING_STR),gdata->name);
							char *bufptr = &status_buffer_g[ strlen(status_buffer_g) ];
							statusWindow->setText(status_buffer_g);
							statusWindow->resize();
							Vector * vector = new Vector(2048);
							read_index(gdata,vector);
							int entries2 = vector->getSize();
							MessageListData ** mdataptr = (MessageListData **)vector->getData();
							for(int l=0;l<entries2;l++) {
								DoMethod(app,MUIM_Application_InputBuffered);
								if(l % 16 == 0) {
									do_input();
									if(statusWindow->isAborted()==TRUE || running==FALSE) {
										aborted = TRUE;
										break;
									}
									sprintf(bufptr," ( %d / %d )",l,entries2);
									statusWindow->setText(status_buffer_g);
								}
								MessageListData *mdata = mdataptr[l];
								//nLog("Stats: (%d) %s\n",mdata->ID,mdata->subject);
								// see if an entry for this already exists in our list
								int entries3 = 0;
								get(NLIST_stats_RES,MUIA_NList_Entries,&entries3);
								if(what==0)
									cmp=mdata->from;
								else if(what==1) {
									cmp=mdata->subject;
									while(*cmp!=0 && (STRNICMP(cmp,"RE: ",4)==0 || STRNICMP(cmp,"SV: ",4)==0))
										cmp += 4;
								}
								else if(what==2) {
									get_refs(&newmessageTemp,gdata,mdata,GETREFS_NONE);
									if(*newmessageTemp.xnewsreader == '\0')
										cmp = unknown;
									else
										cmp = newmessageTemp.xnewsreader;
								}
								BOOL found=FALSE;
								for(int m=0;m<entries3;m++) {
									DoMethod(NLIST_stats_RES,MUIM_NList_GetEntry,m,&statsList);
									if(strcmp(cmp,statsList->value)==0) {
										found=TRUE;
										// insert new frequency value here
										statsList->freq++;
										DoMethod(NLIST_stats_RES,MUIM_NList_Redraw,m);
										break;
									}
								}
								if(!found) { // insert a new entry
									statsList = new StatsList;
									strcpy(statsList->value,cmp);
									statsList->freq=1;
									DoMethod(NLIST_stats_RES,MUIM_NList_InsertSingle,statsList,MUIV_NList_Insert_Bottom);
								}
							}
							for(int i=0;i<vector->getSize();i++)
								delete (MessageListData *)vector->getData()[i];
							delete (Vector *)vector;
						}
					}
				}
				set(wnd_main,MUIA_Window_Sleep,FALSE);
				WriteWindow::sleepAll(FALSE);
				set(wnd_stats,MUIA_Window_Sleep,FALSE);
				delete statusWindow;
				}
			}
	}
}

void callback_stats_sort() {
	int val = 0;
	get(NLIST_stats_RES,MUIA_NList_TitleClick,&val);
	if(val==sorttype_stats)
		sorttypedir_stats = -sorttypedir_stats;
	else {
		sorttype_stats = val;
		sorttypedir_stats = 1;
	}
	set(wnd_stats,MUIA_Window_Sleep,TRUE);
	DoMethod(NLIST_stats_RES,MUIM_NList_Sort);
	set(wnd_stats,MUIA_Window_Sleep,FALSE);
}

void callback_users_new () {
	BOOL yesno = FALSE;
	get(wnd_newuser,MUIA_Window_Open,&yesno);
	if(!yesno) {
		set(STR_newuser_USER,MUIA_String_Contents,"");
		set(STR_newuser_PASS,MUIA_String_Contents,"");
		set(CM_newuser_PASS,MUIA_Selected,FALSE);
		set(STR_newuser_DATALOC,MUIA_String_Contents,"PROGDIR:");
		set(CM_newuser_SUP,MUIA_Selected,TRUE);
		set(CM_newuser_SUP,MUIA_Selected,FALSE);
	}
	set(wnd_newuser,MUIA_Window_Open,TRUE);
	set(wnd_newuser,MUIA_Window_ActiveObject,STR_newuser_USER);
}

void callback_users_delete () {
	int val = 0;
	get(NLIST_users_LIST,MUIA_NList_Active,&val);
	if(val!=MUIV_NList_Active_Off) {
		User *user = NULL;
		DoMethod(NLIST_users_LIST,MUIM_NList_GetEntry,val,&user);
		if(stricmp(currentUser->getName(),user->getName())==0)
			MUI_RequestA(app,0,0,CatalogStr(MSG_DELETE_USER,MSG_DELETE_USER_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_UNABLE_TO_DELETE_CURRENT_USER,MSG_UNABLE_TO_DELETE_CURRENT_USER_STR),0);
		else {
				int result = MUI_RequestA(app,0,0,CatalogStr(MSG_DELETE_USER,MSG_DELETE_USER_STR),CatalogStr(MSG_DELETE_OR_CANCEL,MSG_DELETE_OR_CANCEL_STR),CatalogStr(MSG_DELETE_USER_ARE_YOU_SURE,MSG_DELETE_USER_ARE_YOU_SURE_STR),0);
				if(result==1)
				{
					//result=MUI_RequestA(app,0,0,CatalogStr(MSG_DELETE_USER,MSG_DELETE_USER_STR),"Prefs and News Items|Prefs Only|Neither","The database entry for this user has been deleted.\nYou may now either:\n- Delete all their prefs and news items\n- Delete their prefs only\n- Or do neither.",0);
					result = MUI_RequestA(app,0,0,CatalogStr(MSG_DELETE_USER,MSG_DELETE_USER_STR),CatalogStr(MSG_YES_OR_NO,MSG_YES_OR_NO_STR),CatalogStr(MSG_USER_DELETED_DELETE_PREFS,MSG_USER_DELETED_DELETE_PREFS_STR),0);
					correctDrawer(status_buffer_g,user->dataLocation);
					if(result!=0)
					{
						// delete prefs
						char command[MAXFILENAME+300] = "";
						sprintf(command,"delete \"%s.prefs\" >NIL:",status_buffer_g);
						//Execute(command,NULL,NULL);
						System(command, NULL);
						sprintf(command,"delete \"%s.killfile\" >NIL:",status_buffer_g);
						//Execute(command,NULL,NULL);
						System(command, NULL);
					}
					//if(result==1) {
						// delete all news items
					//	result=MUI_RequestA(app,0,0,"Delete All News Items",CatalogStr(MSG_DELETE_OR_CANCEL,MSG_DELETE_OR_CANCEL_STR),"\33cAre you *sure* you wish\nto delete all news items for this user?\n",0);
					//}
					DoMethod(NLIST_users_LIST,MUIM_NList_Remove,val);
					saveUsers();
				}
		}
	}
}

void callback_newuser_okay () {
			strcpy(status_buffer_g,getstr(STR_newuser_USER));
			if(strlen(status_buffer_g)<3)
			{
				MUI_RequestA(app,0,0,CatalogStr(MSG_NEW_USER,MSG_NEW_USER_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_USER_NAME_TOO_SHORT,MSG_USER_NAME_TOO_SHORT_STR),0);
				return;
				//break;
			}
			char filename[MAXFILENAME] = "";
			correctDrawer(filename,getstr(STR_newuser_DATALOC));
			int val = checkUniqueUser(getstr(STR_newuser_USER),filename);
			if(val==1) {
				MUI_RequestA(app,0,0,CatalogStr(MSG_NEW_USER,MSG_NEW_USER_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_USER_NAME_EXISTS,MSG_USER_NAME_EXISTS_STR),0);
				return;
				//break;
			}
			else if(val==2) {
				MUI_RequestA(app,0,0,CatalogStr(MSG_NEW_USER,MSG_NEW_USER_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_NEWS_DIRECTORY_IN_USE,MSG_NEWS_DIRECTORY_IN_USE_STR),0);
				return;
				//break;
			}
			get(CM_newuser_PASS,MUIA_Selected,&val);
			strcpy(status_buffer_g,getstr(STR_newuser_PASS));
			if(val==TRUE && strlen(status_buffer_g)<4) {
				MUI_RequestA(app,0,0,CatalogStr(MSG_NEW_USER,MSG_NEW_USER_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_PASSWORD_TOO_SHORT,MSG_PASSWORD_TOO_SHORT_STR),0);
				return;
				//break;
			}
			if(createDrawer(filename)==FALSE)
			{
				sprintf(status_buffer_g,CatalogStr(MSG_CANNOT_CREATE_FOLDER,MSG_CANNOT_CREATE_FOLDER_STR),getstr(STR_newuser_DATALOC));
				MUI_RequestA(app,0,0,CatalogStr(MSG_NEW_USER,MSG_NEW_USER_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),status_buffer_g,0);
				return;
				//break;
			}
			set(wnd_newuser,MUIA_Window_Open,FALSE);
			User *user = NULL;
			if(val==TRUE)
				user = new User(getstr(STR_newuser_USER),getstr(STR_newuser_PASS));
			else
				user = new User(getstr(STR_newuser_USER),NULL);
			//strcpy(user->dataLocation,getstr(STR_newuser_DATALOC));
			//correctDrawer(user->dataLocation,getstr(STR_newuser_DATALOC));
			strcpy(user->dataLocation,filename);
			get(CM_newuser_SUP,MUIA_Selected,&val);
			user->setSupervisor(val);
			addUser(user);
			// copy prefs?
			get(CM_newuser_COPYPREFS,MUIA_Selected,&val);
			if(val) {
				sprintf(filename,"%s.prefs",user->dataLocation);
				account.save_tofile(filename);
				sprintf(filename,"%s.servers",user->dataLocation);
				save_servers(filename);
			}
			// decides whether in login stage or not by seeing if wnd_main is closed or open..
			get(wnd_main,MUIA_Window_Open,&val);
			if(val)
				DoMethod(NLIST_users_LIST,MUIM_NList_InsertSingle,user,MUIV_NList_Insert_Bottom);
			else
				currentUser=user; // must be during login process
			newuseradded=1;
			guistarted=TRUE;
}

void callback_cpwd_okay () {
			set(wnd_cpwd,MUIA_Window_Open,FALSE);
			int val = 0;
			get(CM_cpwd_PASS,MUIA_Selected,&val);
			strcpy(status_buffer_g,getstr(STR_cpwd_PASS));
			if(val==TRUE && strlen(status_buffer_g) < 4) {
				MUI_RequestA(app,0,0,CatalogStr(MSG_CHANGE_PASSWORD,MSG_CHANGE_PASSWORD_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_PASSWORD_TOO_SHORT,MSG_PASSWORD_TOO_SHORT_STR),0);
				return;
				//break;
			}
			if(val==TRUE)
				currentUser->setPassword(status_buffer_g);
			else
				currentUser->setPassword(NULL);
			replaceUser(currentUser);
			loadUsers();
}

void callback_nng_okay()
{
			char filename[128] = "";
			while(TRUE) {
				sprintf(filename,"NewsCoasterData:Folder_%d",nextID);
				if(exists(filename))
					nextID++;
				else
					break;
			}
			BPTR lock=CreateDir(filename);
			if(lock) {
				UnLock(lock);
				lock=NULL;
				GroupData *gdata = new GroupData();
				gdata->ID=nextID;
				gdata->flags[0]=FALSE;
				strcpy(gdata->name,getstr(STR_nng_NAME));
				strcpy(gdata->desc,getstr(STR_nng_DESC));

				char comment[81] = "";
				strncpy(comment,*gdata->desc=='\0' ? gdata->name : gdata->desc,80);
				comment[80] = '\0';
				SetComment(filename,comment);

				gdata->nummess=0;
				gdata->num_unread=0;
				gdata->nextmID=0;
				get(CM_nng_S,MUIA_Selected,&gdata->s);
				get(CY_nng_DEFSIG,MUIA_Cycle_Active,&gdata->defsig);
				gdata->defsig--;
				DateStamp(&gdata->lastdlds); // download from now on
				int val = 0;
				get(CY_nng_MAXDL,MUIA_Cycle_Active,&val);
				if(val==0)
					gdata->max_dl=-1;
				else
					get(STR_nng_MAXDL,MUIA_String_Integer,&gdata->max_dl);
				get(CY_nng_MAXL,MUIA_Cycle_Active,&val);
				if(val==0)
					gdata->flags[2]=0;
				else {
					get(STR_nng_MAXL,MUIA_String_Integer,&gdata->flags[2]);
					if(gdata->flags[2]==0)
						gdata->flags[2]=1;
				}
				get(CY_nng_MAXAGE,MUIA_Cycle_Active,&gdata->flags[6]);
				get(STR_nng_MAXAGE,MUIA_String_Integer,&gdata->flags[7]);
				get(CY_nng_OFFLINE,MUIA_Cycle_Active,&gdata->flags[5]);
				gdata->serverID = 0;
				if(nng_defserver != 0) {
					// messing about so that we use default if the Server has since been deleted
					Server *server = getServer(nng_defserver); // will choose default if been deleted
					if(server->ID != nng_defserver) // force default
						gdata->serverID = 0;
					else if(server->def)
						gdata->serverID = 0;
					else
						gdata->serverID = server->ID;
				}
				insertGdata(gdata,FALSE);
				redrawGdataAll();
				//sprintf(command,"copy NewsCoaster:default.index NewsCoasterData:Folder_%d/.index",nextID);
				//Execute(command,NULL,NULL);
				writeEmptyIndex(gdata->ID);

				nextID++;
				account.save_data();
			}
			else
				MUI_RequestA(app,0,0,CatalogStr(MSG_ERROR_3,MSG_ERROR_3_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_CANNOT_CREATE_FOLDER_2,MSG_CANNOT_CREATE_FOLDER_2_STR),0);
			set(wnd_newnewsgroup,MUIA_Window_Open,FALSE);
			set(STR_nng_NAME,MUIA_String_Contents,"");
			set(STR_nng_DESC,MUIA_String_Contents,"");
			nng_defserver = 0;
}

void callback_nng_cancel() {
	set(wnd_newnewsgroup,MUIA_Window_Open,FALSE);
	set(STR_nng_NAME,MUIA_String_Contents,"");
	set(STR_nng_DESC,MUIA_String_Contents,"");
	nng_defserver = 0;
}

void callback_eng_okay() {
			strcpy(ng_edit->name,getstr(STR_eng_NAME));
			strcpy(ng_edit->desc,getstr(STR_eng_DESC));

			char filename[128] = "";
			getFolderPath(filename,ng_edit->ID);
			char comment[81] = "";
			strncpy(comment,*ng_edit->desc=='\0' ? ng_edit->name : ng_edit->desc,80);
			comment[80] = '\0';
			SetComment(filename,comment);

			get(CY_eng_DEFSIG,MUIA_Cycle_Active,&ng_edit->defsig);
			ng_edit->defsig--;
			get(CM_eng_S,MUIA_Selected,&ng_edit->s);
			int val = 0;
			get(CY_eng_MAXDL,MUIA_Cycle_Active,&val);
			if(val==0)
				ng_edit->max_dl=-1;
			else
				get(STR_eng_MAXDL,MUIA_String_Integer,&ng_edit->max_dl);
			get(CY_eng_MAXL,MUIA_Cycle_Active,&val);
			if(val==0)
				ng_edit->flags[2]=0;
			else {
				get(STR_eng_MAXL,MUIA_String_Integer,&ng_edit->flags[2]);
				if(ng_edit->flags[2]==0)
					ng_edit->flags[2]=1;
			}
			get(CY_eng_MAXAGE,MUIA_Cycle_Active,&ng_edit->flags[6]);
			get(STR_eng_MAXAGE,MUIA_String_Integer,&ng_edit->flags[7]);
			get(CY_eng_OFFLINE,MUIA_Cycle_Active,&ng_edit->flags[5]);
			redrawGdataAll();
			ng_edit=NULL;
			account.save_data();
			set(wnd_editng,MUIA_Window_Open,FALSE);
			set(STR_eng_NAME,MUIA_String_Contents,"");
			set(STR_nng_DESC,MUIA_String_Contents,"");
}

void callback_eng_cancel() {
	set(wnd_editng,MUIA_Window_Open,FALSE);
	set(STR_eng_NAME,MUIA_String_Contents,"");
	set(STR_nng_DESC,MUIA_String_Contents,"");
}

void callback_ngadv_okay() {
			get(CM_ngadv_APPVHD,MUIA_Selected,&ng_editadv->moreflags[0]);
			strcpy(ng_editadv->appv_hd,getstr(STR_ngadv_APPVHD));
			get(CM_ngadv_ALTNAME,MUIA_Selected,&ng_editadv->moreflags[1]);
			strcpy(ng_editadv->alt_name,getstr(STR_ngadv_ALTNAME));
			get(CM_ngadv_ALTEMAIL,MUIA_Selected,&ng_editadv->moreflags[2]);
			strcpy(ng_editadv->alt_email,getstr(STR_ngadv_ALTEMAIL));
			Server *oldsv = getServer(ng_editadv->serverID);

			int val = 0;
			get(CM_ngadv_SERVER,MUIA_Selected,&val);
			if(val)
				ng_editadv->serverID = 0;
			else {
				get(CY_ngadv_SERVER,MUIA_Cycle_Active,&val);
				Server *server = getServer(server_list[val]); // will choose default if been deleted
				if(server->ID != server_list[val]) // force default
					ng_editadv->serverID = 0;
				else
					ng_editadv->serverID = server->ID;
			}

			get(CM_ngadv_SERVERPOST,MUIA_Selected,&ng_editadv->moreflags[4]);

			Server *newsv = getServer(ng_editadv->serverID);
			if(oldsv->ID != newsv->ID) { // reset article pointer?
				int result = MUI_RequestA(app,0,0,CatalogStr(MSG_RESET_ARTICLE_POINTER,MSG_RESET_ARTICLE_POINTER_STR),"_Reset|_Don't Reset",CatalogStr(MSG_SERVER_CHANGED_RESET_ARTICLE_POINTER,MSG_SERVER_CHANGED_RESET_ARTICLE_POINTER_STR),0);
				if(result == 1)
					resetArticlePointers(ng_editadv);

				GroupData *active_gdata = NULL;
				getGdataActive(&active_gdata);
				if(ng_editadv == active_gdata)
					setGdataHelpText(active_gdata); // update help bubble text, since using a different newsserver
			}
			redrawGdataAll();
			ng_editadv=NULL;
			account.save_data();
			set(wnd_ngadv,MUIA_Window_Open,FALSE);
}

void callback_ngadv_cancel() {
	set(wnd_ngadv,MUIA_Window_Open,FALSE);
}

void callback_server_okay() {
			ULONG portvalue;
			get(STR_acc_PORT,MUIA_String_Integer,&portvalue);
			if (portvalue>65535)
			{
				DisplayBeep(0);
				set(wnd_servers,MUIA_Window_ActiveObject,STR_acc_PORT);
				//break;
				return;
			}
			Server *server = new Server();
			strcpy(server->nntp,getstr(STR_acc_NNTP));
			strcpy(server->user,getstr(STR_acc_USER));
			strcpy(server->password,getstr(STR_acc_PASS));
			get(CM_acc_AUTH,MUIA_Selected,&server->nntp_auth);
			server->port=(int)portvalue;
			server->def = FALSE;
			server->post = FALSE;
			int changed = FALSE;
			if(editServer==0) {
				server->ID = nextServerID++;
				DoMethod(NLIST_acc_SERVERS,MUIM_NList_InsertSingle,server,MUIV_NList_Insert_Bottom);
			}
			else {
				if(stricmp(editServer->nntp,server->nntp)!=0)
					changed = TRUE;
				server->ID = editServer->ID;
				server->def = editServer->def;
				server->post = editServer->post;
				int pos=-1;
				int entries = 0;
				get(NLIST_acc_SERVERS,MUIA_NList_Entries,&entries);
				Server *server2;
				for(int i=0;i<entries;i++) {
					DoMethod(NLIST_acc_SERVERS,MUIM_NList_GetEntry,i,&server2);
					if(server2==editServer) {
						pos = i;
						break;
					}
				}
				if(pos==-1) // not found! (this shouldn't happen..)
					DoMethod(NLIST_acc_SERVERS,MUIM_NList_InsertSingle,server,MUIV_NList_Insert_Bottom);
				else
					DoMethod(NLIST_acc_SERVERS,MUIM_NList_ReplaceSingle,server,pos,NOWRAP,ALIGN_LEFT);
			}
			if(changed) {
				int result = MUI_RequestA(app,0,0,CatalogStr(MSG_RESET_ARTICLE_POINTERS,MSG_RESET_ARTICLE_POINTERS_STR),CatalogStr(MSG_RESET_OR_NOT,MSG_RESET_OR_NOT_STR),CatalogStr(MSG_SERVER_ADDRESS_CHANGED_RESET_POINTERS,MSG_SERVER_ADDRESS_CHANGED_RESET_POINTERS_STR),0);
				GroupData *active_gdata = NULL;
				getGdataActive(&active_gdata);

				int entries = 0;
				getGdataEntries(&entries);
				for(int i=0;i<entries;i++) {
					GroupData *gdata = NULL;
					getGdata(i,&gdata);
					if(gdata!=0) {
						if( (gdata->serverID==0 && server->def==TRUE) || gdata->serverID==server->ID ) {
							if(result == 1) {
								//printf("Reset for group: %s (%d)\n",gdata->name,gdata->serverID);
								resetArticlePointers(gdata);
							}
							if(gdata == active_gdata)
								setGdataHelpText(active_gdata); // update help bubble text, since using a different newsserver
						}
					}
				}
			}
			DoMethod(NLIST_acc_SERVERS,MUIM_NList_Redraw,MUIV_NList_Redraw_All);
			editServer = NULL;
			set(wnd_servers,MUIA_Window_Open,FALSE);
}

void callback_server_cancel() {
			editServer = NULL;
			set(wnd_servers,MUIA_Window_Open,FALSE);
}

void callback_acc_okay() {
			strcpy(account.name,getstr(STR_acc_NAME));
			strcpy(status_buffer_g,getstr(STR_acc_EMAIL));
			if(strchr(status_buffer_g,'@')==0 || (strchr(status_buffer_g,'@')==&status_buffer_g[strlen(status_buffer_g)-1]))
				MUI_RequestA(app,0,0,CatalogStr(TBAR_SETTINGS_HELP,TBAR_SETTINGS_HELP_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_YOU_MUST_ENTER_EMAIL_ADDRESS,MSG_YOU_MUST_ENTER_EMAIL_ADDRESS_STR),0);
			else
				strcpy(account.email,getstr(STR_acc_EMAIL));

			strcpy(status_buffer_g,getstr(STR_acc_REALEMAIL));
			if( (strchr(status_buffer_g,'@')==0 || (strchr(status_buffer_g,'@')==&status_buffer_g[strlen(status_buffer_g)-1])) && *status_buffer_g!=0)
				MUI_RequestA(app,0,0,CatalogStr(TBAR_SETTINGS_HELP,TBAR_SETTINGS_HELP_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_YOU_MUST_ENTER_EMAIL_ADDRESS,MSG_YOU_MUST_ENTER_EMAIL_ADDRESS_STR),0);
			else
				strcpy(account.realemail,getstr(STR_acc_REALEMAIL));

			strcpy(account.org,getstr(STR_acc_ORG));

			strcpy(account.smtp,getstr(STR_acc_SMTP));
			strcpy(account.domain,getstr(STR_acc_DOMAIN));
			strcpy(account.followup_text,getstr(STR_acc_FOLLOWUPTEXT));
			strcpy(account.charset_write,gettxt(STR_acc_CHARSETWRITE));
			get(CY_acc_TIMEZONE,MUIA_Cycle_Active,&account.timezone);
			account.timezone -= 12;
			get(CM_acc_USELOCALETZ,MUIA_Selected,&account.uselocaletz);
			get(CM_acc_BST,MUIA_Selected,&account.bst);
			get(CY_acc_DATEFORMAT,MUIA_Cycle_Active,&account.dateformat);

			account.flags = 0;

			int val = 0;
			get(CM_acc_SNIPSIG,MUIA_Selected,&val);
			account.flags = account.flags | Account::SNIPSIG;
			if(!val)
				account.flags -= Account::SNIPSIG;

			get(CM_acc_LOGGING,MUIA_Selected,&val);
			account.flags = account.flags | Account::LOGGING;
			if(!val)
				account.flags -= Account::LOGGING;

			get(CM_acc_LOGDEL,MUIA_Selected,&val);
			account.flags = account.flags | Account::LOGDEL;
			if(!val)
				account.flags -= Account::LOGDEL;

			get(CM_acc_DELONLINE,MUIA_Selected,&val);
			account.flags = account.flags | Account::NODELONLINE;
			if(val) // reverse logic
				account.flags -= Account::NODELONLINE;

			get(CM_acc_CONFIRMQUIT,MUIA_Selected,&val);
			account.flags = account.flags | Account::NOCONFIRMQUIT;
			if(val) // reverse logic
				account.flags -= Account::NOCONFIRMQUIT;

			get(CM_acc_QUIETDL,MUIA_Selected,&val);
			if(val) account.flags |= Account::QUIETDL;

			get(CM_acc_CHECKFORDUPS,MUIA_Selected,&val);
			if(val) account.flags |= Account::CHECKFORDUPS;

			get(CM_acc_VGROUPDL,MUIA_Selected,&account.vgroupdl);
			get(CM_acc_NOCONFIRMDEL,MUIA_Selected,&account.noconfirmdel);
			get(CM_acc_XNEWS,MUIA_Selected,&account.xnews);
			//get(CM_acc_USESIG,MUIA_Selected,&account.usesig);
			strcpy(status_buffer_g,getstr(STR_acc_LINELENGTH));
			account.linelength = atoi(status_buffer_g);
			if(account.linelength<4)
				account.linelength=4;
			if(account.linelength<60 || account.linelength>80) {
				sprintf(status_buffer_g,CatalogStr(MSG_BAD_LINE_LENGTH,MSG_BAD_LINE_LENGTH_STR),account.linelength);
				MUI_RequestA(app,0,0,CatalogStr(TBAR_SETTINGS_HELP,TBAR_SETTINGS_HELP_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),status_buffer_g,0);
			}
			get(CM_acc_REWRAP,MUIA_Selected,&val);
			if(val)
				account.rewrap = TRUE;
			else
				account.rewrap = FALSE;

			get(CY_acc_XNO,MUIA_Cycle_Active,&account.xno);
			if(accountpage) {
				char* sigString;
				int length;
				if(sigs[cy_current_sig]) {
					delete [] sigs[cy_current_sig];
					sigs[cy_current_sig]=NULL;
				}
				sigString=(STRPTR)DoMethod(ED_acc_SIG,MUIM_TextEditor_ExportText);
				length = strlen(sigString);
				if (length>0)
				{
					sigs[cy_current_sig] = new char[strlen(sigString)+1];
					strcpy(sigs[cy_current_sig], sigString);
				}
				// accountpage set to FALSE at end of ACC_CANCEL bit
			}

			account.listflags = 0;
			get(CM_acc_LISTFLAGS,MUIA_Selected,&val);
			if(val)
				account.listflags |= Account::LISTFLAGS;
			get(CM_acc_LISTFROMGROUP,MUIA_Selected,&val);
			if(val)
				account.listflags |= Account::LISTFROMGROUP;
			get(CM_acc_LISTDATE,MUIA_Selected,&val);
			if(val)
				account.listflags |= Account::LISTDATE;
			get(CM_acc_LISTSUBJECT,MUIA_Selected,&val);
			if(val)
				account.listflags |= Account::LISTSUBJECT;
			get(CM_acc_LISTSIZE,MUIA_Selected,&val);
			if(val)
				account.listflags |= Account::LISTSIZE;
			get(CM_acc_LISTLINES,MUIA_Selected,&val);
			if(val)
				account.listflags |= Account::LISTLINES;
			resetMessageListFormat();
			redrawMdataAll();
			DoMethod(NLIST_messagelistdata,MUIM_NList_ColWidth,MUIV_NList_ColWidth_All,MUIV_NList_ColWidth_Default);
			DoMethod(LISTTREE_messagelistdata,MUIM_NList_ColWidth,MUIV_NList_ColWidth_All,MUIV_NList_ColWidth_Default);
			DoMethod(NLIST_search_RES,MUIM_NList_Redraw,MUIV_NList_Redraw_All);
			DoMethod(NLIST_search_RES,MUIM_NList_ColWidth,MUIV_NList_ColWidth_All,MUIV_NList_ColWidth_Default);

			if(account.mimeprefsno>0) {
				for(int k=0;k<account.mimeprefsno;k++)
					delete account.mimeprefs[k];
			}
			get(NLIST_acc_MIMEPREFS,MUIA_NList_Entries,&val);
			if(val>0) {
				if(val>256)
					val=256;
				for(int k=0;k<val;k++) {
					MIMEPrefs *ptr = NULL;
					DoMethod(NLIST_acc_MIMEPREFS,MUIM_NList_GetEntry,k,&ptr);
					account.mimeprefs[k]=new MIMEPrefs;
					*account.mimeprefs[k]=*(MIMEPrefs *)ptr;
				}
			}
			account.mimeprefsno=val;
			strcpy(account.defviewer,getstr(STR_acc_MIMEDEF));
			int result = 0;
			get(CY_acc_READHEADER,MUIA_Cycle_Active,&result);
			account.readheader_type=(int)result;
			strncpy(account.readheaders,getstr(STR_acc_READHEADER),READHEADER_LEN);
			account.readheaders[READHEADER_LEN] = '\0';
			MUI_PenSpec *ptr = NULL;
			get(PEN_acc_TEXT_QUOTE2,MUIA_Pendisplay_Spec,&ptr);
			account.pen_acc_text_quote2 = *((MUI_PenSpec *)ptr);
			get(PEN_acc_TEXT_COL,MUIA_Pendisplay_Spec,&ptr);
			account.pen_acc_text_col = *((MUI_PenSpec *)ptr);
			get(CM_acc_MULTIPLEVWS,MUIA_Selected,&account.multiplevws);
			account.save_data();
			save_servers();

			set(wnd_account,MUIA_Window_Open,FALSE);
			//if(returnID == ACC_CANCEL) { // needed cos no break in ACC_OKAY
			//	load_servers();
			//} // not done due to issues with changing article pointers
			set(STR_acc_NAME,MUIA_String_Contents,"");
			set(STR_acc_EMAIL,MUIA_String_Contents,"");
			set(STR_acc_REALEMAIL,MUIA_String_Contents,"");
			set(STR_acc_ORG,MUIA_String_Contents,"");
			set(STR_acc_NNTPPOST,MUIA_String_Contents,"");
			set(STR_acc_SMTP,MUIA_String_Contents,"");
			set(STR_acc_DOMAIN,MUIA_String_Contents,"");
			set(STR_acc_FOLLOWUPTEXT,MUIA_String_Contents,"");
			set(CM_acc_BST,MUIA_Selected,FALSE);
			set(CM_acc_VGROUPDL,MUIA_Selected,FALSE);
			set(CM_acc_DELONLINE,MUIA_Selected,TRUE);
			set(CM_acc_CONFIRMQUIT,MUIA_Selected,TRUE);
			set(CM_acc_QUIETDL,MUIA_Selected,FALSE);
			set(CM_acc_CHECKFORDUPS,MUIA_Selected,FALSE);
			set(CM_acc_NOCONFIRMDEL,MUIA_Selected,FALSE);
			set(CM_acc_XNEWS,MUIA_Selected,FALSE);
			//set(CM_acc_USESIG,MUIA_Selected,FALSE);
			DoMethod(ED_acc_SIG,MUIM_TextEditor_ClearText);
			DoMethod(NLIST_acc_MIMEPREFS,MUIM_NList_Clear);
			accountpage=FALSE;
}

void callback_acc_cancel() {
			set(wnd_account,MUIA_Window_Open,FALSE);
			//if(returnID == ACC_CANCEL) { // needed cos no break in ACC_OKAY
			//	load_servers();
			//} // not done due to issues with changing article pointers
			set(STR_acc_NAME,MUIA_String_Contents,"");
			set(STR_acc_EMAIL,MUIA_String_Contents,"");
			set(STR_acc_REALEMAIL,MUIA_String_Contents,"");
			set(STR_acc_ORG,MUIA_String_Contents,"");
			set(STR_acc_NNTPPOST,MUIA_String_Contents,"");
			set(STR_acc_SMTP,MUIA_String_Contents,"");
			set(STR_acc_DOMAIN,MUIA_String_Contents,"");
			set(STR_acc_FOLLOWUPTEXT,MUIA_String_Contents,"");
			set(CM_acc_BST,MUIA_Selected,FALSE);
			set(CM_acc_VGROUPDL,MUIA_Selected,FALSE);
			set(CM_acc_DELONLINE,MUIA_Selected,TRUE);
			set(CM_acc_CONFIRMQUIT,MUIA_Selected,TRUE);
			set(CM_acc_QUIETDL,MUIA_Selected,FALSE);
			set(CM_acc_CHECKFORDUPS,MUIA_Selected,FALSE);
			set(CM_acc_NOCONFIRMDEL,MUIA_Selected,FALSE);
			set(CM_acc_XNEWS,MUIA_Selected,FALSE);
			//set(CM_acc_USESIG,MUIA_Selected,FALSE);
			DoMethod(ED_acc_SIG,MUIM_TextEditor_ClearText);
			DoMethod(NLIST_acc_MIMEPREFS,MUIM_NList_Clear);
			accountpage=FALSE;
}

enum {
	GETGROUPS_MENU,
	GETGROUPS_ACC,
	GETGROUPS_GROUPMAN
};

void callback_getgroups(int *type_ptr) {
			//int returnID = *rID;
			int type = *type_ptr;
			Server *server = NULL;
			//if(returnID == ACC_GETGROUPS) {
			if(type == GETGROUPS_ACC) {
				int val = 0;
				get(NLIST_acc_SERVERS,MUIA_NList_Active,&val);
				if(val!=MUIV_NList_Active_Off)
					DoMethod(NLIST_acc_SERVERS,MUIM_NList_GetEntry,val,&server);
			}
			//else if(returnID == GROUPMAN_GETGROUPS)
			else if(type == GETGROUPS_GROUPMAN)
				server = groupman_server;
			else
				server = getDefaultServer();
			if(server != 0) {
				if(delete_dis)
					MUI_RequestA(app,0,0,CatalogStr(MSG_NOT_AVAILABLE,MSG_NOT_AVAILABLE_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_FUNCTION_NOT_AVAILABLE_WHEN_CONNECTED,MSG_FUNCTION_NOT_AVAILABLE_WHEN_CONNECTED_STR),0);
				else {
					sprintf(status_buffer_g,CatalogStr(MSG_DOWNLOAD_GROUP_LIST_ARE_YOU_SURE,MSG_DOWNLOAD_GROUP_LIST_ARE_YOU_SURE_STR),server->nntp);
					int result = MUI_RequestA(app,0,0,CatalogStr(MSG_GET_GROUP_LIST,MSG_GET_GROUP_LIST_STR),CatalogStr(MSG_YES_OR_CANCEL,MSG_YES_OR_CANCEL_STR),status_buffer_g,0);
					if(result==1) {
						getgrouplist(server);
						/*result=getgrouplist(server);
						if(result!=-1) {
							int val = 0;
							get(wnd_groupman,MUIA_Window_Open,&val);
							if(val==FALSE) {
								sprintf(status_buffer_g,CatalogStr(MSG_GROUP_LIST_DOWNLOADED,MSG_GROUP_LIST_DOWNLOADED_STR),result,server->nntp);
								MUI_RequestA(app,0,0,CatalogStr(MSG_GROUP_LIST,MSG_GROUP_LIST_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),status_buffer_g,0);
							}
						}*/
					}
				}
			}
}

void callback_getnewgroups(int *type_ptr) {
			int type = *type_ptr;
	//printf("%d : %d %d\n",returnID,ACC_GETNEWGROUPS,MEN_GETNEWGROUPSDEF);
			Server *server = NULL;
			//if(returnID == ACC_GETNEWGROUPS) {
			if(type == GETGROUPS_ACC) {
				int val = 0;
				get(NLIST_acc_SERVERS,MUIA_NList_Active,&val);
				if(val!=MUIV_NList_Active_Off)
					DoMethod(NLIST_acc_SERVERS,MUIM_NList_GetEntry,val,&server);
			}
			else
				server = getDefaultServer();
			if(server != 0) {
				if(delete_dis)
					MUI_RequestA(app,0,0,CatalogStr(MSG_NOT_AVAILABLE,MSG_NOT_AVAILABLE_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_FUNCTION_NOT_AVAILABLE_WHEN_CONNECTED,MSG_FUNCTION_NOT_AVAILABLE_WHEN_CONNECTED_STR),0);
				else {
					getnewgroups(server);
				}
			}
}

void callback_groupman(int *acc_ptr) {
			int acc = *acc_ptr;
			Server *server = NULL;
			if(acc) {
				// from account settings
				int val = 0;
				get(NLIST_acc_SERVERS,MUIA_NList_Active,&val);
				if(val!=MUIV_NList_Active_Off) {
					DoMethod(NLIST_acc_SERVERS,MUIM_NList_GetEntry,val,&server);
					//printf(":::%s\n",server->nntp);
				}
			}
			else {
				// from menu
				server = getDefaultServer();
			}
			if(server != 0) {
				BOOL yesno = FALSE;
				get(wnd_groupman,MUIA_Window_Open,&yesno);
				if(!yesno) {
					int result = MUI_RequestA(app,0,0,CatalogStr(MSG_GROUP_MANAGER,MSG_GROUP_MANAGER_STR),CatalogStr(MSG_YES_OR_CANCEL,MSG_YES_OR_CANCEL_STR),CatalogStr(MSG_OPEN_GROUP_MANAGER_ARE_YOU_SURE,MSG_OPEN_GROUP_MANAGER_ARE_YOU_SURE_STR),0);
					if(result==1) {
						groupman_server = server;
						readgrouplist(groupman_server,0);
						set(wnd_groupman,MUIA_Window_Open,TRUE);
					}
				}
				else
					set(wnd_groupman,MUIA_Window_Open,TRUE);
			}
}

void callback_acc_strkill(int *temp_ptr) {
	int temp = *temp_ptr;
	//printf("temp: %d\n", temp);
			KillFile *ptr = new KillFile();
			int val = 0;
			get(NLIST_acc_KILLFILE,MUIA_NList_Active,&val);
			if(val!=MUIV_NList_Active_Off || !temp) { // clicking cycle on blank shouldn't do anything here..
				if(val!=MUIV_NList_Active_Off) {
					KillFile *ptr2 = NULL;
					DoMethod(NLIST_acc_KILLFILE,MUIM_NList_GetEntry,MUIV_NList_GetEntry_Active,&ptr2);
					*((KillFile *)ptr)=*((KillFile *)ptr2);
				}
				strcpy(((KillFile *)ptr)->header,getstr(STR_acc_NEWKILLHEAD));
				strcpy(((KillFile *)ptr)->text,getstr(STR_acc_NEWKILLTEXT));
				strcpy(((KillFile *)ptr)->ngroups,getstr(STR_acc_NEWKILLGROUP));
				if(strlen(((KillFile *)ptr)->header)==0 && strlen(((KillFile *)ptr)->text)==0 && strlen(((KillFile *)ptr)->ngroups)==0) {
					// oops we don't want blank entries!
					delete ptr;
					ptr=NULL;
				}
				else {
					get(CY_acc_EXPIREKILL,MUIA_Cycle_Active,&((KillFile *)ptr)->expiretype);
					strcpy(status_buffer_g,getstr(STR_acc_EXPIREKILL));
					((KillFile *)ptr)->expire = atoi(status_buffer_g);
					account.defexpiretype=((KillFile *)ptr)->expiretype;
					account.defexpire=((KillFile *)ptr)->expire;

					/*get(CY_acc_TYPEKILL,MUIA_Cycle_Active,&val2);
					((KillFile *)ptr)->type = val2;*/
					int val2 = 0;
					get(CY_acc_MATCHKILL,MUIA_Cycle_Active,&val2);
					((KillFile *)ptr)->match = val2;
					get(CY_acc_ACTIONKILL,MUIA_Cycle_Active,&val2);
					((KillFile *)ptr)->action = val2;
					get(CM_acc_SKIPKILL,MUIA_Selected,&val2);
					((KillFile *)ptr)->carryon = val2 ? 0 : 1;

					val2 = strlen( ((KillFile *)ptr)->header ); // add colon if not there and not blank
					if(val2>0) {
						if( ((KillFile *)ptr)->header[val2-1] != ':') {
							((KillFile *)ptr)->header[val2]=':';
							((KillFile *)ptr)->header[val2+1]=0;
						}
					}
					get(NLIST_acc_KILLFILE,MUIA_NList_Active,&val);
					// convert to upper case
					toUpper(((KillFile *)ptr)->header);
					toUpper(((KillFile *)ptr)->text);
					toUpper(((KillFile *)ptr)->ngroups);
					if(val==MUIV_NList_Active_Off) {
						DoMethod(NLIST_acc_KILLFILE,MUIM_NList_InsertSingle,ptr,MUIV_NList_Insert_Bottom);
						set(wnd_account,MUIA_Window_ActiveObject,STR_acc_NEWKILLHEAD);
					}
					else
						DoMethod(NLIST_acc_KILLFILE,MUIM_NList_ReplaceSingle,ptr,val,NOWRAP,ALIGN_LEFT);
					if( !temp ) { // so temp==true, generated by cycle gadget changing, doesn't clear the entries
						set(STR_acc_NEWKILLHEAD,MUIA_String_Contents,"");
						set(STR_acc_NEWKILLTEXT,MUIA_String_Contents,"");
						set(STR_acc_NEWKILLGROUP,MUIA_String_Contents,"");
						set(CY_acc_EXPIREKILL,MUIA_Cycle_Active,0);
						set(STR_acc_EXPIREKILL,MUIA_String_Contents,"");
						//set(CY_acc_TYPEKILL,MUIA_Cycle_Active,0);
						set(CY_acc_MATCHKILL,MUIA_Cycle_Active,0);
						set(CY_acc_ACTIONKILL,MUIA_Cycle_Active,0);
						set(CM_acc_SKIPKILL,MUIA_Selected,TRUE);
						set(NLIST_acc_KILLFILE,MUIA_NList_Active,MUIV_NList_Active_Off);
					}
				}
			}
}

void mainMenusFunc(int *item) {
	nLog("mainMenusFunc() called\n");

	GroupData * gdata=NULL;
	MessageListData * mdata=NULL;;
	//char filename[MAXFILENAME] = "";
	//char command[300]="";

	int yesno = 0,entries = 0;
	STRPTR temp = NULL,temp2 = NULL;

	VOID *ptr = NULL;
	//switch(returnID) {
	switch(*item) {
		case MEN_GETNEWS:
			if(delete_dis)
				MUI_RequestA(app,0,0,CatalogStr(MSG_NOT_AVAILABLE,MSG_NOT_AVAILABLE_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_FUNCTION_NOT_AVAILABLE_WHEN_CONNECTED,MSG_FUNCTION_NOT_AVAILABLE_WHEN_CONNECTED_STR),0);
			else
				getnews(-1);
			break;
		case MEN_GETNEWSSINGLE:
			if(delete_dis)
				MUI_RequestA(app,0,0,CatalogStr(MSG_NOT_AVAILABLE,MSG_NOT_AVAILABLE_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_FUNCTION_NOT_AVAILABLE_WHEN_CONNECTED,MSG_FUNCTION_NOT_AVAILABLE_WHEN_CONNECTED_STR),0);
			else {
				int val = 0;
				getGdataActivePos(&val);
				getGdataActive(&gdata);
				if(gdata==0)
					MUI_RequestA(app,0,0,CatalogStr(MSG_FETCH_NEWS,MSG_FETCH_NEWS_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_PLEASE_SELECT_GROUP_FOLDER,MSG_PLEASE_SELECT_GROUP_FOLDER_STR),0);
				else if(gdata->ID >= 0)
					getnews(gdata->ID);
				else
					MUI_RequestA(app,0,0,CatalogStr(MSG_FETCH_NEWS,MSG_FETCH_NEWS_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_PLEASE_SELECT_GROUP_FOLDER,MSG_PLEASE_SELECT_GROUP_FOLDER_STR),0);
			}
			break;
		case MEN_SENDNEWS:
			if(delete_dis)
				MUI_RequestA(app,0,0,CatalogStr(MSG_NOT_AVAILABLE,MSG_NOT_AVAILABLE_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_FUNCTION_NOT_AVAILABLE_WHEN_CONNECTED,MSG_FUNCTION_NOT_AVAILABLE_WHEN_CONNECTED_STR),0);
			else
				postnews(0);
			break;
		case MEN_GETGROUPSDEF:
			DoMethod(app, MUIM_CallHook, &hook_standard, callback_getgroups, GETGROUPS_MENU);
			//callback_getgroups(&GETGROUPS_MENU);
			break;
		/*case ACC_GETGROUPS:
			DoMethod(app, MUIM_CallHook, &hook_standard, callback_getgroups, GETGROUPS_ACC);
			//callback_getgroups(&GETGROUPS_ACC);
			break;
		case GROUPMAN_GETGROUPS:
			DoMethod(app, MUIM_CallHook, &hook_standard, callback_getgroups, GETGROUPS_GROUPMAN);
			//callback_getgroups(&GETGROUPS_GROUPMAN);
			break;*/
		case MEN_GETNEWGROUPSDEF:
			DoMethod(app, MUIM_CallHook, &hook_standard, callback_getnewgroups, GETGROUPS_MENU);
			//callback_getnewgroups(&GETGROUPS_MENU);
			break;
		/*case ACC_GETNEWGROUPS:
			DoMethod(app, MUIM_CallHook, &hook_standard, callback_getnewgroups, GETGROUPS_ACC);
			//callback_getnewgroups(&GETGROUPS_ACC);
			break;*/
		case MEN_DISCONNECT:
			closeSockets();
			break;
		case MEN_ABOUT:
			set(wnd_about,MUIA_Window_Open,TRUE);
			break;
		case MEN_ABOUTMUI:
			if(!aboutwin) {
				aboutwin = AboutmuiObject,
									MUIA_Aboutmui_Application,app,
									End;
			}
			if(aboutwin)
				set(aboutwin,MUIA_Window_Open,TRUE);
			else
				DisplayBeep(0);
			break;
		case MEN_SYSINFO:
		{
			set(wnd_main,MUIA_Window_Sleep,TRUE);
			char filename[] = "RAM:sysinfo.txt";
			DeleteFile(filename);
			BPTR file = Open(filename,MODE_NEWFILE);

			SystemTags("c:cpu", SYS_Output, file, TAG_END);

			FPrintf(file,"\nCurrent Memory:\n"); FFlush(file);
			SystemTags("c:avail", SYS_Output, file, TAG_END);

			FPrintf(file,"\nVersion Information:\n"); FFlush(file);
			SystemTags("c:version", SYS_Output, file, TAG_END);
			SystemTags("c:version NewsCoaster:NewsCoaster", 	SYS_Output, file, TAG_END);
			SystemTags("c:version libs:mui/BetterString.mcc", 	SYS_Output, file, TAG_END);
			SystemTags("c:version libs:mui/BetterString.mcp", 	SYS_Output, file, TAG_END);
			SystemTags("c:version libs:mui/NList.mcc", 			SYS_Output, file, TAG_END);
			SystemTags("c:version libs:mui/NList.mcc", 			SYS_Output, file, TAG_END);
			SystemTags("c:version libs:mui/NListview.mcc", 		SYS_Output, file, TAG_END);
			SystemTags("c:version libs:mui/NListviews.mcp", 	SYS_Output, file, TAG_END);
			SystemTags("c:version libs:mui/TextEditor.mcc", 	SYS_Output, file, TAG_END);
			SystemTags("c:version libs:mui/TextEditor.mcp", 	SYS_Output, file, TAG_END);
			SystemTags("c:version libs:mui/NListtree.mcc", 		SYS_Output, file, TAG_END);
			SystemTags("c:version libs:mui/NListtree.mcp", 		SYS_Output, file, TAG_END);
			SystemTags("c:version libs:mui/TheBar.mcc", 		SYS_Output, file, TAG_END);

			Close(file);

			MUI_RequestA(app,0,0,CatalogStr(MSG_SYSTEM_INFORMATION,MSG_SYSTEM_INFORMATION_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_SYSTEM_INFORMATION_SAVED,MSG_SYSTEM_INFORMATION_SAVED_STR),0);
			set(wnd_main,MUIA_Window_Sleep,FALSE);
			break;
		}
		case MEN_ICONIFY:
			set(app,MUIA_Application_Iconified,TRUE);
			break;
		//case MUIV_Application_ReturnID_Quit:
		case MEN_QUIT:
		{
			int force = 0;
			get(app,MUIA_Application_ForceQuit,&force);
			int result = 0;
			if( force != 0 || (account.flags & Account::NOCONFIRMQUIT)!=0 )
				result = 1;
			else
				result = MUI_RequestA(app,0,0,CatalogStr(MSG_QUIT_PROGRAM,MSG_QUIT_PROGRAM_STR),CatalogStr(MSG_QUIT_OR_CANCEL,MSG_QUIT_OR_CANCEL_STR),CatalogStr(MSG_QUIT_ARE_YOU_SURE,MSG_QUIT_ARE_YOU_SURE_STR),0);
			if(result==1) {
				WriteWindow **ptrs = (WriteWindow **)WriteWindow::ptrs->getData();
				int size = WriteWindow::ptrs->getSize();
				BOOL quit = TRUE;
				for(int i=0;i<size && quit;i++) {
					WriteWindow *ww = ptrs[i];
					//set(ww->wnd,MUIA_Window_Open,TRUE); // commented out, since it causes the Requester to immediately get moved to the back (though we open the requester afterwards)?!
					int res = 0;
					if(force)
						res = 1;
					else
						res = MUI_RequestA(app,ww->wnd,0,CatalogStr(MSG_SAVE_MESSAGE,MSG_SAVE_MESSAGE_STR),CatalogStr(MSG_SAVE_DISCARD_OR_CANCEL,MSG_SAVE_DISCARD_OR_CANCEL_STR),CatalogStr(MSG_SAVE_CHANGES_ARE_YOU_SURE,MSG_SAVE_CHANGES_ARE_YOU_SURE_STR),0);

					if(res == 0)
						quit = FALSE;
					else if(res == 1) {
						if( !ww->save(force) ) {
							// message not saved - should be due to user deciding to edit further
							quit = FALSE;
						}
					}
				}
				if(quit)
					running=FALSE;
			}
			break;
		}
		case MEN_NEWNEWSGROUP:
			/*if(delete_dis)
				MUI_RequestA(app,0,0,CatalogStr(MSG_NOT_AVAILABLE,MSG_NOT_AVAILABLE_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_FUNCTION_NOT_AVAILABLE_WHEN_CONNECTED,MSG_FUNCTION_NOT_AVAILABLE_WHEN_CONNECTED_STR),0);
			else*/
			{
			set(wnd_newnewsgroup,MUIA_Window_Open,TRUE);
			set(wnd_newnewsgroup,MUIA_Window_ActiveObject,STR_nng_NAME);
			nng_defserver = 0;
			}
			break;
		case MEN_EDITNG:
			/*if(delete_dis)
				MUI_RequestA(app,0,0,CatalogStr(MSG_NOT_AVAILABLE,MSG_NOT_AVAILABLE_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_FUNCTION_NOT_AVAILABLE_WHEN_CONNECTED,MSG_FUNCTION_NOT_AVAILABLE_WHEN_CONNECTED_STR),0);
			else*/
			{
			get(wnd_editng,MUIA_Window_Open,&yesno);
			if(!yesno) {
				getGdataActive(&gdata);
				if(gdata==0)
					MUI_RequestA(app,0,0,CatalogStr(MSG_EDIT_NEWSGROUP,MSG_EDIT_NEWSGROUP_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_PLEASE_SELECT_GROUP_TO_EDIT,MSG_PLEASE_SELECT_GROUP_TO_EDIT_STR),0);
				else if(gdata->ID>=0) {
					set(STR_eng_NAME,MUIA_String_Contents,gdata->name);
					set(STR_eng_DESC,MUIA_String_Contents,gdata->desc);
					set(CY_eng_DEFSIG,MUIA_Cycle_Active,gdata->defsig+1);
					if(gdata->max_dl==-1) {
						set(CY_eng_MAXDL,MUIA_Cycle_Active,0);
						set(STR_eng_MAXDL,MUIA_String_Integer,0);
					}
					else {
						set(CY_eng_MAXDL,MUIA_Cycle_Active,1);
						set(STR_eng_MAXDL,MUIA_String_Integer,gdata->max_dl);
					}
					if(gdata->flags[2]<=0) {
						set(CY_eng_MAXL,MUIA_Cycle_Active,0);
						set(STR_eng_MAXL,MUIA_String_Integer,1);
					}
					else {
						set(CY_eng_MAXL,MUIA_Cycle_Active,1);
						set(STR_eng_MAXL,MUIA_String_Integer,gdata->flags[2]);
					}
					set(CY_eng_MAXAGE,MUIA_Cycle_Active,gdata->flags[6]);
					set(STR_eng_MAXAGE,MUIA_String_Integer,gdata->flags[7]);
					set(CY_eng_OFFLINE,MUIA_Cycle_Active,gdata->flags[5]);
					DateHandler::get_date(status_buffer_g,getGMTOffset(),account.bst,gdata->lastdlds);
					set(TXT_eng_LASTDL,MUIA_Text_Contents,status_buffer_g);
					if(gdata->ID==-1)
						strcpy(status_buffer_g,"outgoing/");
					else if(gdata->ID==-2)
						strcpy(status_buffer_g,"sent/");
					else if(gdata->ID==-3)
						strcpy(status_buffer_g,"deleted/");
					else
						sprintf(status_buffer_g,"folder_%d/",gdata->ID);
					set(TXT_eng_FOLDER,MUIA_Text_Contents,status_buffer_g);
					set(CM_eng_S,MUIA_Selected,gdata->s);
					set(wnd_editng,MUIA_Window_Open,TRUE);
					if(DoMethod(GROUP_eng,MUIM_Group_InitChange))
						DoMethod(GROUP_eng,MUIM_Group_ExitChange);
					set(wnd_editng,MUIA_Window_ActiveObject,STR_eng_NAME);
					ng_edit=gdata;
				}
				else
					MUI_RequestA(app,0,0,CatalogStr(MSG_EDIT_NEWSGROUP,MSG_EDIT_NEWSGROUP_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_PLEASE_SELECT_GROUP_TO_EDIT,MSG_PLEASE_SELECT_GROUP_TO_EDIT_STR),0);
			}
			else {
				MUI_RequestA(app,0,0,CatalogStr(MSG_EDIT_NEWSGROUP,MSG_EDIT_NEWSGROUP_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_PLEASE_FINISH_EDITING_GROUP,MSG_PLEASE_FINISH_EDITING_GROUP_STR),0);
				set(wnd_editng,MUIA_Window_Open,TRUE);
			}
			}
			break;
		case MEN_NGADV:
			/*if(delete_dis)
				MUI_RequestA(app,0,0,CatalogStr(MSG_NOT_AVAILABLE,MSG_NOT_AVAILABLE_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_FUNCTION_NOT_AVAILABLE_WHEN_CONNECTED,MSG_FUNCTION_NOT_AVAILABLE_WHEN_CONNECTED_STR),0);
			else*/
			{
			get(wnd_ngadv,MUIA_Window_Open,&yesno);
			if(!yesno) {
				getGdataActive(&gdata);
				if(gdata==0)
					MUI_RequestA(app,0,0,CatalogStr(MSG_GROUP_ADVANCED_SETTINGS,MSG_GROUP_ADVANCED_SETTINGS_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_PLEASE_SELECT_GROUP_TO_EDIT,MSG_PLEASE_SELECT_GROUP_TO_EDIT_STR),0);
				else if(gdata->ID>=0) {
					set(CM_ngadv_APPVHD,MUIA_Selected,gdata->moreflags[0]);
					set(STR_ngadv_APPVHD,MUIA_String_Contents,gdata->appv_hd);
					set(CM_ngadv_ALTNAME,MUIA_Selected,gdata->moreflags[1]);
					set(STR_ngadv_ALTNAME,MUIA_String_Contents,gdata->alt_name);
					set(CM_ngadv_ALTEMAIL,MUIA_Selected,gdata->moreflags[2]);
					set(STR_ngadv_ALTEMAIL,MUIA_String_Contents,gdata->alt_email);
					freeServerCycle();
					int active=setServerCycle(gdata->serverID);
					set(CM_ngadv_SERVERPOST,MUIA_Selected,gdata->moreflags[4]);

					ng_editadv=gdata;
					set(wnd_ngadv,MUIA_Window_Open,TRUE);
					set(CY_ngadv_SERVER,MUIA_Cycle_Active,active);
				}
				else
					MUI_RequestA(app,0,0,CatalogStr(MSG_GROUP_ADVANCED_SETTINGS,MSG_GROUP_ADVANCED_SETTINGS_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_PLEASE_SELECT_GROUP_TO_EDIT,MSG_PLEASE_SELECT_GROUP_TO_EDIT_STR),0);
			}
			else {
				MUI_RequestA(app,0,0,CatalogStr(MSG_GROUP_ADVANCED_SETTINGS,MSG_GROUP_ADVANCED_SETTINGS_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_PLEASE_FINISH_EDITING_GROUP,MSG_PLEASE_FINISH_EDITING_GROUP_STR),0);
				set(wnd_ngadv,MUIA_Window_Open,TRUE);
			}
			}
			break;
		case MEN_DELNG:
			if(delete_dis)
				MUI_RequestA(app,0,0,CatalogStr(MSG_NOT_AVAILABLE,MSG_NOT_AVAILABLE_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_FUNCTION_NOT_AVAILABLE_WHEN_CONNECTED,MSG_FUNCTION_NOT_AVAILABLE_WHEN_CONNECTED_STR),0);
			else {
				getGdataActive(&gdata);
				if(gdata==0)
					MUI_RequestA(app,0,0,CatalogStr(MENU_DELETE_NEWSGROUP,MENU_DELETE_NEWSGROUP_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_PLEASE_SELECT_GROUP,MSG_PLEASE_SELECT_GROUP_STR),0);
				else if(gdata->ID>=0) {
					int result = MUI_RequestA(app,0,0,CatalogStr(MENU_DELETE_NEWSGROUP,MENU_DELETE_NEWSGROUP_STR),CatalogStr(MSG_DELETE_OR_CANCEL,MSG_DELETE_OR_CANCEL_STR),CatalogStr(MSG_DELETE_GROUP_ARE_YOU_SURE,MSG_DELETE_GROUP_ARE_YOU_SURE_STR),0);
					if(result==1) {
						set(wnd_main,MUIA_Window_Sleep,TRUE);
						char command[128] = "";
						sprintf(command,"delete NewsCoasterData:Folder_%d/ all >NIL:",gdata->ID);
						//Execute(command,NULL,NULL);
						System(command, NULL);
						removeGdataActive();
						set(wnd_main,MUIA_Window_Sleep,FALSE);
						account.save_data();
					}
				}
				else
					MUI_RequestA(app,0,0,CatalogStr(MENU_DELETE_NEWSGROUP,MENU_DELETE_NEWSGROUP_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_YOU_MAY_NOT_DELETE_FOLDER,MSG_YOU_MAY_NOT_DELETE_FOLDER_STR),0);
			}
			break;
		case MEN_ARCHIVENG:
			if(delete_dis)
				MUI_RequestA(app,0,0,CatalogStr(MSG_NOT_AVAILABLE,MSG_NOT_AVAILABLE_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_FUNCTION_NOT_AVAILABLE_WHEN_CONNECTED,MSG_FUNCTION_NOT_AVAILABLE_WHEN_CONNECTED_STR),0);
			else {
				getGdataActive(&gdata);
				if(gdata==0)
					MUI_RequestA(app,0,0,CatalogStr(MSG_ARCHIVE_GROUP,MSG_ARCHIVE_GROUP_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_PLEASE_SELECT_GROUP,MSG_PLEASE_SELECT_GROUP_STR),0);
				else {
					sprintf(status_buffer_g,"%s.mbx",gdata->name);
					sleepAll(TRUE);
					char *filename = new char[1024];
					BOOL ok = LoadASL(filename,CatalogStr(MSG_FILE_TO_SAVE_ARCHIVE_AS,MSG_FILE_TO_SAVE_ARCHIVE_AS_STR),status_buffer_g,(const char *)"#?",FALSE);
					sleepAll(FALSE);
					if(ok)
						archiveGroup(gdata,filename);
					delete [] filename;
				}
			}
			break;
		case MEN_VIEWGROUPSLIST:
			if(account.grouplistType!=0) {
				account.save_data(); // also needed to save out current grouplist
				account.grouplistType=0;
				readInGroups();
				set(wnd_main,MUIA_Window_Open,FALSE);
				DoMethod(grouplistGroup,OM_REMMEMBER,grouplistGroup_LISTTREE);
				DoMethod(grouplistGroup,OM_ADDMEMBER,grouplistGroup_NLIST);
				set(wnd_main,MUIA_Window_Open,TRUE);
				DoMethod(menuitem_VIEWGROUPS,MUIM_SetUData,MEN_VIEWGROUPSLIST,MUIA_Menuitem_Checked,account.grouplistType==0);
				DoMethod(menuitem_VIEWGROUPS,MUIM_SetUData,MEN_VIEWGROUPSTREE,MUIA_Menuitem_Checked,account.grouplistType==1);
				account.save_data(); //now save changed view type
				resetGdataActive();
			}
			break;
		case MEN_VIEWGROUPSTREE:
			if(account.grouplistType!=1 && nlisttree==TRUE) {
				account.save_data(); // also needed to save out current grouplist
				account.grouplistType=1;
				readInGroups();
				set(wnd_main,MUIA_Window_Open,FALSE);
				DoMethod(grouplistGroup,OM_REMMEMBER,grouplistGroup_NLIST);
				DoMethod(grouplistGroup,OM_ADDMEMBER,grouplistGroup_LISTTREE);
				set(wnd_main,MUIA_Window_Open,TRUE);
				DoMethod(menuitem_VIEWGROUPS,MUIM_SetUData,MEN_VIEWGROUPSLIST,MUIA_Menuitem_Checked,account.grouplistType==0);
				DoMethod(menuitem_VIEWGROUPS,MUIM_SetUData,MEN_VIEWGROUPSTREE,MUIA_Menuitem_Checked,account.grouplistType==1);
				account.save_data(); //now save changed view type
				resetGdataActive();
			}
			break;
		case MEN_SORT:
		{
			int result = MUI_RequestA(app,0,0,CatalogStr(MSG_SORT_NEWSGROUP,MSG_SORT_NEWSGROUP_STR),CatalogStr(MSG_SORT_OR_CANCEL,MSG_SORT_OR_CANCEL_STR),CatalogStr(MSG_SORT_GROUP_ARE_YOU_SURE,MSG_SORT_GROUP_ARE_YOU_SURE_STR),0);
			if(result==1) {
				if(account.grouplistType==0)
					DoMethod(NLIST_groupdata,MUIM_NList_Sort);
				account.save_data();
			}
			break;
		}
		case MEN_SEARCH:
		{
			getGdataEntries(&entries);
			DoMethod(NLIST_search_NG,MUIM_NList_Clear);
			for(int k=0;k<entries;k++) {
				GroupData *gdata = NULL;
				getGdata(k,&gdata);
				if(gdata==0)
					continue;
				// make a copy!
				/*ptr2=new char[256];
				strcpy((char *)ptr2,((GroupData *)ptr)->name);
				DoMethod(NLIST_search_NG,MUIM_NList_InsertSingle,ptr2,MUIV_NList_Insert_Bottom);*/
				GroupData *gdata_c = new GroupData();
				*gdata_c = *gdata;
				DoMethod(NLIST_search_NG,MUIM_NList_InsertSingle,gdata_c,MUIV_NList_Insert_Bottom);
			}
			set(wnd_search,MUIA_Window_Open,TRUE);
			break;
		}
		case MEN_STATS:
		{
			getGdataEntries(&entries);
			DoMethod(NLIST_stats_NG,MUIM_NList_Clear);
			for(int k=0;k<entries;k++) {
				GroupData *gdata = NULL;
				getGdata(k,&gdata);
				if(gdata==0)
					continue;
				// make a copy!
				/*ptr2=new char[256];
				strcpy((char *)ptr2,((GroupData *)ptr)->name);
				DoMethod(NLIST_stats_NG,MUIM_NList_InsertSingle,(GroupData *)ptr2,MUIV_NList_Insert_Bottom);*/
				GroupData *gdata_c = new GroupData();
				*gdata_c = *gdata;
				DoMethod(NLIST_stats_NG,MUIM_NList_InsertSingle,gdata_c,MUIV_NList_Insert_Bottom);
			}
			set(wnd_stats,MUIA_Window_Open,TRUE);
			break;
		}
		case MEN_RESETGROUPPTR:
			getGdataActive(&gdata);
			if(gdata==0)
				MUI_RequestA(app,0,0,CatalogStr(MSG_RESET_ARTICLE_POINTER,MSG_RESET_ARTICLE_POINTER_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_PLEASE_SELECT_GROUP,MSG_PLEASE_SELECT_GROUP_STR),0);
			else if(gdata->ID>=0) {
				int result = MUI_RequestA(app,0,0,CatalogStr(MSG_RESET_ARTICLE_POINTER,MSG_RESET_ARTICLE_POINTER_STR),CatalogStr(MSG_RESET_OR_CANCEL,MSG_RESET_OR_CANCEL_STR),CatalogStr(MSG_RESET_ARTICLE_POINTER_ARE_YOU_SURE,MSG_RESET_ARTICLE_POINTER_ARE_YOU_SURE_STR),0);
				if(result==1)
					resetArticlePointers(gdata);
			}
			else
				MUI_RequestA(app,0,0,CatalogStr(MSG_RESET_ARTICLE_POINTER,MSG_RESET_ARTICLE_POINTER_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_PLEASE_SELECT_GROUP,MSG_PLEASE_SELECT_GROUP_STR),0);
			break;
		case MEN_RESETALLPTR:
		{
			int result = MUI_RequestA(app,0,0,CatalogStr(MSG_RESET_ARTICLE_POINTERS,MSG_RESET_ARTICLE_POINTERS_STR),CatalogStr(MSG_RESET_OR_CANCEL,MSG_RESET_OR_CANCEL_STR),CatalogStr(MSG_RESET_ALL_POINTERS_ARE_YOU_SURE,MSG_RESET_ALL_POINTERS_ARE_YOU_SURE_STR),0);
			if(result==1)
				resetArticlePointers();
			break;
		}
		case MEN_GROUPMANDEF:
		//case ACC_GROUPMAN:
		{
			DoMethod(app, MUIM_CallHook, &hook_standard, callback_groupman, FALSE );
			break;
		}
		case MEN_UPDATEGROUPS:
			if(delete_dis)
				MUI_RequestA(app,0,0,CatalogStr(MSG_NOT_AVAILABLE,MSG_NOT_AVAILABLE_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_FUNCTION_NOT_AVAILABLE_WHEN_CONNECTED,MSG_FUNCTION_NOT_AVAILABLE_WHEN_CONNECTED_STR),0);
			else {
				int result = MUI_RequestA(app,0,0,CatalogStr(MENU_UPDATE_GROUPS,MENU_UPDATE_GROUPS_STR),CatalogStr(MSG_UPDATE_OR_CANCEL,MSG_UPDATE_OR_CANCEL_STR),CatalogStr(MSG_UPDATE_GROUP_LIST_ARE_YOU_SURE,MSG_UPDATE_GROUP_LIST_ARE_YOU_SURE_STR),0);
				if(result==1)
					update_groups();
			}
			break;
		case MEN_UPDATEINDEX:
			if(delete_dis)
				MUI_RequestA(app,0,0,CatalogStr(MSG_NOT_AVAILABLE,MSG_NOT_AVAILABLE_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_FUNCTION_NOT_AVAILABLE_WHEN_CONNECTED,MSG_FUNCTION_NOT_AVAILABLE_WHEN_CONNECTED_STR),0);
			else {
				int result = MUI_RequestA(app,0,0,CatalogStr(MENU_UPDATE_INDEX,MENU_UPDATE_INDEX_STR),CatalogStr(MSG_UPDATE_OR_CANCEL,MSG_UPDATE_OR_CANCEL_STR),CatalogStr(MSG_UPDATE_GROUP_INDEX_ARE_YOU_SURE,MSG_UPDATE_GROUP_INDEX_ARE_YOU_SURE_STR),0);
				if(result==1) {
					getGdataActive(&gdata);
					if(gdata==0)
						MUI_RequestA(app,0,0,CatalogStr(MENU_UPDATE_INDEX,MENU_UPDATE_INDEX_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_PLEASE_SELECT_GROUP,MSG_PLEASE_SELECT_GROUP_STR),0);
					else
						update_index();
				}
			}
			break;
		case MEN_UPDATEALLIND:
			if(delete_dis)
				MUI_RequestA(app,0,0,CatalogStr(MSG_NOT_AVAILABLE,MSG_NOT_AVAILABLE_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_FUNCTION_NOT_AVAILABLE_WHEN_CONNECTED,MSG_FUNCTION_NOT_AVAILABLE_WHEN_CONNECTED_STR),0);
			else {
				int result = MUI_RequestA(app,0,0,CatalogStr(MSG_UPDATE_INDICES,MSG_UPDATE_INDICES_STR),CatalogStr(MSG_UPDATE_OR_CANCEL,MSG_UPDATE_OR_CANCEL_STR),CatalogStr(MSG_UPDATE_ALL_INDICES_ARE_YOU_SURE,MSG_UPDATE_ALL_INDICES_ARE_YOU_SURE_STR),0);
				if(result==1) {
					getGdataEntries(&entries);
					for(int k=0;k<entries;k++) {
						getGdata(k,&gdata);
						if(gdata==0)
							continue;
						set_gdata(gdata);
						if(update_index())
							break;
					}
				}
			}
			break;
		case MEN_IMPORT:
			if(delete_dis)
				MUI_RequestA(app,0,0,CatalogStr(MSG_NOT_AVAILABLE,MSG_NOT_AVAILABLE_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_FUNCTION_NOT_AVAILABLE_WHEN_CONNECTED,MSG_FUNCTION_NOT_AVAILABLE_WHEN_CONNECTED_STR),0);
			else
				import();
			break;
		case MEN_VIEWMESSAGESLIST:
			if(account.mdata_view!=0) {
				MessageListData * current_mdata = NULL;
				getMdataActive(&current_mdata);
				//printf("%d\n",current_mdata);
				//if(current_mdata != 0 && get_mdataptr_pos(current_mdata) == -1)
				//	current_mdata = NULL; // message no longer exists in list - we don't want to reference it, in cases it's been deleted!

				account.save_data(); // also needed to save out current grouplist
				account.mdata_view=0;
				set(wnd_main,MUIA_Window_Open,FALSE);
				DoMethod(messageGroup,OM_REMMEMBER,messageGroup_LISTTREE);
				DoMethod(messageGroup,OM_ADDMEMBER,messageGroup_NLIST);
				set(wnd_main,MUIA_Window_Open,TRUE);
				DoMethod(menuitem_VIEWMESSAGES,MUIM_SetUData,MEN_VIEWMESSAGESLIST,MUIA_Menuitem_Checked,account.mdata_view==0);
				DoMethod(menuitem_VIEWMESSAGES,MUIM_SetUData,MEN_VIEWMESSAGESTREE,MUIA_Menuitem_Checked,account.mdata_view==1);
				account.save_data(); //now save changed view type

				if(current_mdata != 0) {
					//printf("-->%d : %s\n",current_mdata->ID,current_mdata->subject);
					set_mdata(current_mdata);
				}
			}
			break;
		case MEN_VIEWMESSAGESTREE:
			if(account.mdata_view!=1 && nlisttree==TRUE) {
				MessageListData * current_mdata = NULL;
				getMdataActive(&current_mdata);
				//printf("%d\n",current_mdata);
				//if(current_mdata != 0 && get_mdataptr_pos(current_mdata) == -1)
				//	current_mdata = NULL; // message no longer exists in list - we don't want to reference it, in cases it's been deleted!

				account.save_data(); // also needed to save out current grouplist
				account.mdata_view=1;
				threadView();

				set(wnd_main,MUIA_Window_Open,FALSE);
				DoMethod(messageGroup,OM_REMMEMBER,messageGroup_NLIST);
				DoMethod(messageGroup,OM_ADDMEMBER,messageGroup_LISTTREE);
				set(wnd_main,MUIA_Window_Open,TRUE);
				DoMethod(menuitem_VIEWMESSAGES,MUIM_SetUData,MEN_VIEWMESSAGESLIST,MUIA_Menuitem_Checked,account.mdata_view==0);
				DoMethod(menuitem_VIEWMESSAGES,MUIM_SetUData,MEN_VIEWMESSAGESTREE,MUIA_Menuitem_Checked,account.mdata_view==1);
				account.save_data(); //now save changed view type

				if(current_mdata != 0) {
					//printf("-->%d : %s\n",current_mdata->ID,current_mdata->subject);
					set_mdata(current_mdata);
				}
			}
			break;
		case MEN_READ:
		{
			int result = 0;
			getMdataActivePos(&result);
			if(result!=MUIV_NList_Active_Off) {
				ViewWindow *vw;
				if(ViewWindow::ptrs->getSize()==0 || account.multiplevws==TRUE)
					vw = new ViewWindow("NewsCoaster");
				else {
					vw = ((ViewWindow **)ViewWindow::ptrs->getData())[0];
					vw->reset();
				}
				set(wnd_main,MUIA_Window_Sleep,TRUE);
				if(!vw->read())
					delete vw;
				set(wnd_main,MUIA_Window_Sleep,FALSE);
			}
			else
				MUI_RequestA(app,0,0,CatalogStr(MENU_READ_MESSAGE,MENU_READ_MESSAGE_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_PLEASE_SELECT_MESSAGE,MSG_PLEASE_SELECT_MESSAGE_STR),0);
			break;
		}
		case MEN_EDIT:
			/*get(wnd_write,MUIA_Window_Open,&yesno);
			if(!yesno)*/
			{
				int result = 0;
				getMdataActivePos(&result);
				if(result!=MUIV_NList_Active_Off) {
					getGdataDisplayed(&gdata);
					if(gdata->ID<0) {
						getMdataActive(&mdata);
						// search currently edited messages
						WriteWindow **data = (WriteWindow **)WriteWindow::ptrs->getData();
						int size = WriteWindow::ptrs->getSize();
						WriteWindow *ww = NULL;
						for(int i=0;i<size;i++) {
							if(data[i]->newmessage.edit && stricmp(data[i]->newmessage.messageID,mdata->messageID)==0) {
								ww = data[i];
								break;
							}
						}
						if(ww == NULL) {
							if(gdata->ID == -2)
								MUI_RequestA(app,0,0,CatalogStr(MSG_EDIT_MESSAGE,MSG_EDIT_MESSAGE_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_MESSAGE_TO_EDIT_IS_SENT,MSG_MESSAGE_TO_EDIT_IS_SENT_STR),0);
							WriteWindow::editMess(gdata,mdata,FALSE);
						}
						else {
							set(ww->wnd,MUIA_Window_Open,TRUE);
						}
					}
					else
						MUI_RequestA(app,0,0,CatalogStr(MSG_EDIT_MESSAGE,MSG_EDIT_MESSAGE_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_YOU_CAN_EDIT_ONLY_YOUR_MESSAGES,MSG_YOU_CAN_EDIT_ONLY_YOUR_MESSAGES_STR),0);
				}
				else
					MUI_RequestA(app,0,0,CatalogStr(MSG_EDIT_MESSAGE,MSG_EDIT_MESSAGE_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_YOU_MUST_SELECT_MESSAGE_TO_EDIT,MSG_YOU_MUST_SELECT_MESSAGE_TO_EDIT_STR),0);
			}
			/*else
				MUI_RequestA(app,0,0,CatalogStr(MSG_EDIT_MESSAGE,MSG_EDIT_MESSAGE_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),"\33cYou may only write one message\nat a time.\nPlease finish/cancel the\nexisting message.",0);*/
			break;
		case MEN_SUPER:
			/*get(wnd_write,MUIA_Window_Open,&yesno);
			if(!yesno)*/
			{
				int result = 0;
				getMdataActivePos(&result);
				if(result!=MUIV_NList_Active_Off) {
					getGdataDisplayed(&gdata);
					if(gdata->ID==-2) {
						getMdataActive(&mdata);
						WriteWindow::editMess(gdata,mdata,TRUE);
					}
					else
						MUI_RequestA(app,0,0,CatalogStr(MSG_SUPERSEDE_MESSAGE,MSG_SUPERSEDE_MESSAGE_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_YOU_CAN_SUPERSEDE_ONLY_SENT_MESSAGES,MSG_YOU_CAN_SUPERSEDE_ONLY_SENT_MESSAGES_STR),0);
				}
				else
					MUI_RequestA(app,0,0,CatalogStr(MSG_SUPERSEDE_MESSAGE,MSG_SUPERSEDE_MESSAGE_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_YOU_MUST_SELECT_MESSAGE_TO_SUPERSEDE,MSG_YOU_MUST_SELECT_MESSAGE_TO_SUPERSEDE_STR),0);
			}
			/*else
				MUI_RequestA(app,0,0,CatalogStr(MSG_SUPERSEDE_MESSAGE,MSG_SUPERSEDE_MESSAGE_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),"\33cYou may only write one message\nat a time.\nPlease finish/cancel the\nexisting message.",0);*/
			break;
		case MEN_CANCEL:
			//get(wnd_write,MUIA_Window_Open,&yesno);
			//if(!yesno)
			{
				int result = 0;
				getMdataActivePos(&result);
				if(result!=MUIV_NList_Active_Off) {
					getGdataDisplayed(&gdata);
					if(gdata->ID==-2) {
						int result = MUI_RequestA(app,0,0,CatalogStr(MENU_CANCEL_MESSAGE,MENU_CANCEL_MESSAGE_STR),CatalogStr(MSG_CANCEL_MESSAGE_OR_NOT,MSG_CANCEL_MESSAGE_OR_NOT_STR),CatalogStr(MSG_CANCEL_MESSAGE_ARE_YOU_SURE,MSG_CANCEL_MESSAGE_ARE_YOU_SURE_STR),0);
						if(result==1) {
							getMdataActive(&mdata);
							WriteWindow::cancelMess(mdata);
							MUI_RequestA(app,0,0,CatalogStr(MENU_CANCEL_MESSAGE,MENU_CANCEL_MESSAGE_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_CANCEL_MESSAGE_QUEUED,MSG_CANCEL_MESSAGE_QUEUED_STR),0);
						}
					}
					else
						MUI_RequestA(app,0,0,CatalogStr(MENU_CANCEL_MESSAGE,MENU_CANCEL_MESSAGE_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_YOU_CAN_SUPERSEDE_ONLY_SENT_MESSAGES,MSG_YOU_CAN_SUPERSEDE_ONLY_SENT_MESSAGES_STR),0);
				}
				else
					MUI_RequestA(app,0,0,CatalogStr(MENU_CANCEL_MESSAGE,MENU_CANCEL_MESSAGE_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_YOU_MUST_SELECT_MESSAGE_TO_SUPERSEDE,MSG_YOU_MUST_SELECT_MESSAGE_TO_SUPERSEDE_STR),0);
			}
			break;
		case MEN_DELMESS:
			/*if(delete_dis)
				MUI_RequestA(app,0,0,CatalogStr(MSG_NOT_AVAILABLE,MSG_NOT_AVAILABLE_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_FUNCTION_NOT_AVAILABLE_WHEN_CONNECTED,MSG_FUNCTION_NOT_AVAILABLE_WHEN_CONNECTED_STR),0);
			else*/
			delete_multi(TRUE);
			break;
		case MEN_UNDELMESS:
			getGdataDisplayed(&gdata);
			if(gdata->ID==-3)
				undelete_multi(TRUE);
			break;
		case MEN_PDTHIS:
		{
			int result = MUI_RequestA(app,0,0,CatalogStr(MSG_DELETE_MESSAGES,MSG_DELETE_MESSAGES_STR),CatalogStr(MSG_DELETE_OR_CANCEL,MSG_DELETE_OR_CANCEL_STR),CatalogStr(MSG_IMMEDIATE_DELETE_ARE_YOU_SURE,MSG_IMMEDIATE_DELETE_ARE_YOU_SURE_STR),0);
			if(result==1)
				delete_multi(FALSE,FALSE); // permanent delete
			break;
		}
		case MEN_PDREAD:
		{
			int result = MUI_RequestA(app,0,0,CatalogStr(MSG_DELETE_READ_MESSAGES,MSG_DELETE_READ_MESSAGES_STR),CatalogStr(MSG_DELETE_OR_CANCEL,MSG_DELETE_OR_CANCEL_STR),CatalogStr(MSG_IMMEDIATE_DELETE_READ_ARE_YOU_SURE,MSG_IMMEDIATE_DELETE_READ_ARE_YOU_SURE_STR),0);
			if(result==1) {
				get(NLIST_messagelistdata,MUIA_NList_Entries,&entries);
				getGdataDisplayed(&gdata);
				setMdataQuiet(TRUE);
				set(wnd_main,MUIA_Window_Sleep,TRUE);
				MessageListData ** mdatalist = new MessageListData *[entries];
				int k=0,l=0;
				for(k=0;k<entries;k++) {
					//getMdataVisible(k,&mdata);
					DoMethod(NLIST_messagelistdata,MUIM_NList_GetEntry,k,&mdata);
					if(!mdata->flags[1]) {
							//mdatalist[l] = new MessageListData();
							//*mdatalist[l++]=*mdata;
							mdatalist[l++] = mdata;
					}
				}
				if(l>0) {
					/*for(k=0;k<l;k++) {
						delete_mess(gdata,mdatalist[k],FALSE);
					}
					write_index_if_changed();*/
					delete_mess_n(gdata,mdatalist,l,FALSE);
				}
				if(mdatalist) {
					delete [] mdatalist;
					mdatalist=NULL;
				}
				setEnabled();
				redrawGdataAll();
				set(wnd_main,MUIA_Window_Sleep,FALSE);
				setMdataQuiet(FALSE);
			}
			break;
		}
		case MEN_PDONLINE:
		{
			int result = MUI_RequestA(app,0,0,CatalogStr(MSG_DELETE_HEADERS,MSG_DELETE_HEADERS_STR),CatalogStr(MSG_DELETE_OR_CANCEL,MSG_DELETE_OR_CANCEL_STR),CatalogStr(MSG_REMOVE_HEADERS_ARE_YOU_SURE,MSG_REMOVE_HEADERS_ARE_YOU_SURE_STR),0);
			if(result==1) {
				get(NLIST_messagelistdata,MUIA_NList_Entries,&entries);
				getGdataDisplayed(&gdata);
				setMdataQuiet(TRUE);
				set(wnd_main,MUIA_Window_Sleep,TRUE);
				MessageListData ** mdatalist = new MessageListData *[entries];
				int l=0;
				for(int k=0;k<entries;k++) {
					//getMdataVisible(k,&mdata);
					DoMethod(NLIST_messagelistdata,MUIM_NList_GetEntry,k,&mdata);
					if(mdata->flags[12]) {
							//mdatalist[l] = new MessageListData();
							//*mdatalist[l++]=*mdata;
							mdatalist[l++] = mdata;
					}
				}
				if(l>0) {
					/*for(k=0;k<l;k++)
						delete_mess(gdata,mdatalist[k],FALSE);
					write_index_if_changed();*/
					delete_mess_n(gdata,mdatalist,l,FALSE);
				}
				if(mdatalist) {
					delete [] mdatalist;
					mdatalist=NULL;
				}
				setEnabled();
				redrawGdataAll();
				set(wnd_main,MUIA_Window_Sleep,FALSE);
				setMdataQuiet(FALSE);
			}
			break;
		}
		case MEN_PDDUP:
		{
			int result = MUI_RequestA(app,0,0,CatalogStr(MSG_DELETE_DUPLICATES,MSG_DELETE_DUPLICATES_STR),CatalogStr(MSG_DELETE_OR_CANCEL,MSG_DELETE_OR_CANCEL_STR),CatalogStr(MSG_DELETE_DUPLICATES_ARE_YOU_SURE,MSG_DELETE_DUPLICATES_ARE_YOU_SURE_STR),0);
			if(result==1) {
				get(NLIST_messagelistdata,MUIA_NList_Entries,&entries);
				getGdataDisplayed(&gdata);
				setMdataQuiet(TRUE);
				set(wnd_main,MUIA_Window_Sleep,TRUE);
				MessageListData **mdatalist = new MessageListData *[entries];
				MessageListData **store = new MessageListData *[entries];
				int n_store = 0;
				BOOL *del = new BOOL[entries];
				int k=0;
				for(k=0;k<entries;k++) {
					del[k] = FALSE;
				}
				int l=0;
				for(k=0;k<entries;k++) {
					if(del[k]) {
						// will already delete
						continue;
					}
					DoMethod(NLIST_messagelistdata,MUIM_NList_GetEntry,k,&mdata);
					store[n_store++] = mdata;
					// now find if duplicates
					for(int j=0;j<entries;j++) {
						if( del[j] || j==k)
							continue;

						MessageListData *mdata2 = NULL;
						DoMethod(NLIST_messagelistdata,MUIM_NList_GetEntry,j,&mdata2);
						if( strcmp(mdata->messageID,mdata2->messageID)==0 ) {
							// duplicate!
							store[n_store++] = mdata2;
							del[j] = TRUE;
							//mdatalist[l++] = mdata2;
						}
					}

					// store is now an array of duplicates - so keep the 'best' one, and discard the rest
					if(n_store > 1) {
						int keep = 0;
						int j=0;
						for(j=1;j<n_store;j++) {
							//printf("%d : %d\n",j,store[j]->flags[12]);
							if( !store[j]->flags[12] ) {
								// offline message
								keep = j;
								//printf("set keep to %d\n",keep);
								break;
							}
						}
						// are any marked as read?
						for(j=0;j<n_store;j++) {
							if( !store[j]->flags[1] ) {
								// mark the kept one as read
								store[keep]->flags[1] = FALSE;
								break;
							}
						}

						//printf("keep = %d\n\n",keep);
						for(j=0;j<n_store;j++) {
							if(j != keep) {
								mdatalist[l++] = store[j];
							}
						}
					}
					n_store = 0;
				}
				if(l>0) {
					/*for(k=0;k<l;k++) {
						//printf("del: %s\n",mdatalist[k]->subject);
						delete_mess(gdata,mdatalist[k],FALSE);
					}
					write_index_if_changed();*/
					delete_mess_n(gdata,mdatalist,l,FALSE);
				}
				if(mdatalist) {
					delete [] mdatalist;
					mdatalist = NULL;
				}
				if(store) {
					delete [] store;
					store = NULL;
				}
				if(del) {
					delete [] del;
					del = NULL;
				}
				setEnabled();
				redrawGdataAll();
				set(wnd_main,MUIA_Window_Sleep,FALSE);
				setMdataQuiet(FALSE);
			}
			break;
		}
		case MEN_PDALL:
		{
			int result = MUI_RequestA(app,0,0,CatalogStr(MSG_DELETE_MESSAGES,MSG_DELETE_MESSAGES_STR),CatalogStr(MSG_DELETE_OR_CANCEL,MSG_DELETE_OR_CANCEL_STR),CatalogStr(MSG_IMMEDIATE_DELETE_ALL_ARE_YOU_SURE,MSG_IMMEDIATE_DELETE_ALL_ARE_YOU_SURE_STR),0);
			if(result==1) {
				set(wnd_main,MUIA_Window_Sleep,TRUE);
				setMdataQuiet(TRUE);
				deleteFolderContents();
				redrawGdataAll();
				setMdataQuiet(FALSE);
				set(wnd_main,MUIA_Window_Sleep,FALSE);
			}
			break;
		}
		case MEN_PDALLALL:
		{
			int result = MUI_RequestA(app,0,0,CatalogStr(MSG_DELETE_MESSAGES,MSG_DELETE_MESSAGES_STR),CatalogStr(MSG_DELETE_OR_CANCEL,MSG_DELETE_OR_CANCEL_STR),CatalogStr(MSG_IMMEDIATE_DELETE_ALL_GROUPS_ARE_YOU_SURE,MSG_IMMEDIATE_DELETE_ALL_GROUPS_ARE_YOU_SURE_STR),0);
			if(result==1) {
				result=MUI_RequestA(app,0,0,CatalogStr(MSG_DELETE_MESSAGES,MSG_DELETE_MESSAGES_STR),CatalogStr(MSG_SURE_OR_CANCEL,MSG_SURE_OR_CANCEL_STR),CatalogStr(MSG_DELETE_ALL_GROUPS_LAST_CHANCE,MSG_DELETE_ALL_GROUPS_LAST_CHANCE_STR),0);
				if(result==1) {
					getGdataEntries(&entries);
					set(wnd_main,MUIA_Window_Sleep,TRUE);
					for(int k=0;k<entries;k++) {
						getGdata(k,&gdata);
						if(gdata==0)
							continue;
						if(gdata->ID>=0)
							deleteFolderContents(gdata);
					}
					redrawGdataAll();
					set(wnd_main,MUIA_Window_Sleep,FALSE);
				}
			}
			break;
		}
		case MEN_POST:
			/*get(wnd_write,MUIA_Window_Open,&yesno);
			if(!yesno)*/
			{
				WriteWindow *ww = new WriteWindow(CatalogStr(MSG_NEWSCOASTER_COMPOSING_MESSAGE,MSG_NEWSCOASTER_COMPOSING_MESSAGE_STR));
				set(ww->wnd,MUIA_Window_Open,TRUE);
				getGdataDisplayed(&gdata);
				if(gdata->ID>=0) {
					strcpy(status_buffer_g,gdata->name);
					set(ww->STR_write_NG,MUIA_String_Contents,status_buffer_g);
					set(ww->CY_write_SIG,MUIA_Cycle_Active,gdata->defsig+1);
				}
				else {
					set(ww->STR_write_NG,MUIA_String_Contents,"");
					set(ww->CY_write_SIG,MUIA_Cycle_Active,1);
				}
				set(ww->ED_write_MESS,MUIA_TextEditor_CursorY,0);
				getUserEmail(gdata,status_buffer_g);
				set(ww->STR_write_FROM,MUIA_String_Contents,status_buffer_g);
				set(ww->wnd,MUIA_Window_ActiveObject,ww->ED_write_MESS);
				/*if( gdata->defsig >= 0 && gdata->ID >= 0
						//&& account.usesig==TRUE
					) {
					strcpy(status_buffer_g,"-- \n");
					DoMethod(ww->ED_write_MESS,MUIM_TextEditor_InsertText,status_buffer_g,MUIV_TextEditor_InsertText_Bottom);
					if(sigs[gdata->defsig])
						DoMethod(ww->ED_write_MESS,MUIM_TextEditor_InsertText,sigs[gdata->defsig],MUIV_TextEditor_InsertText_Bottom);
				}*/
				ww->newmessage.nrefs=0;
				ww->newmessage.edit=FALSE;
				ww->newmessage.reply=FALSE;
				strcpy(ww->newmessage.messageID,"");
				strcpy(ww->newmessage.supersedeID,"");
				DoMethod(ww->NLIST_write_ATT,MUIM_NList_Clear);
			}
			break;
		case MEN_FOLLOWUP:
			WriteWindow::reply(TRUE,FALSE);
			break;
		case MEN_REPLY:
			WriteWindow::reply(FALSE,TRUE);
			break;
		case MEN_BOTH:
			WriteWindow::reply(TRUE,TRUE);
			break;
		case MEN_EXP:
		{
			getGdataDisplayed(&gdata);
			int result = 0;
			getMdataActivePos(&result);
			if(result!=MUIV_NList_Active_Off) {
				getMdataActive(&mdata);
				ViewWindow *vw = ViewWindow::createExportWindow(app,"NewsCoaster",gdata->ID,mdata->ID);
				set(wnd_main,MUIA_Window_Sleep,TRUE);
				if(!vw->read())
					delete vw;
				set(wnd_main,MUIA_Window_Sleep,FALSE);
			}
			else
				MUI_RequestA(app,0,0,CatalogStr(MENU_EXPORT_MESSAGE,MENU_EXPORT_MESSAGE_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_PLEASE_SELECT_MESSAGE,MSG_PLEASE_SELECT_MESSAGE_STR),0);
			break;
		}
		case MEN_VIEWHEADER:
		{
			getGdataDisplayed(&gdata);
			int result = 0;
			getMdataActivePos(&result);
			if(result!=MUIV_NList_Active_Off)
			{
				char filenamenew[256] = "";
				char tempname[L_tmpnam];
				char *line = new char[MAXLINE + 1];
				char filename[MAXFILENAME] = "";
				getMdataActive(&mdata);
				set(wnd_main,MUIA_Window_Sleep,TRUE);

				getFilePath(filename,gdata->ID,mdata->ID);
				//sprintf(filenamenew,"NewsCoasterData:nc_tempfile%d",rand() % 65536);
				sprintf(filenamenew,"%s",tmpnam(tempname));

				BPTR lock = Lock(filename,ACCESS_READ);
				BPTR file = Open(filename,MODE_OLDFILE);
				BPTR newfile = NULL;
				if(file != 0)
				{
					newfile = Open(filenamenew,MODE_NEWFILE);
					if(newfile != 0)
					{
						char command[MAXFILENAME+128] = "";

						for(;;)
						{
							if(!FGets(file,line,MAXLINE))
								break;
							if(*line=='\n' || *line=='\r')
								break;
							if(strncmp(line,"X-NewsCoaster",13)==0)
								continue; // don't show this line
							Write(newfile,line,strlen(line));
						}

						if(*account.defviewer==0)
							sprintf(command,"run >NIL: sys:utilities/multiview \"%s\"",filenamenew);
						else
						{
							sprintf(status_buffer_g,"run >NIL: %s",account.defviewer);
							sprintf(command,status_buffer_g,filenamenew);
						}
						for(int k=0;k<account.mimeprefsno;k++)
						{
							if(strstr("text/plain",account.mimeprefs[k]->type)!=0)
							{
								sprintf(status_buffer_g,"run >NIL: %s",account.mimeprefs[k]->viewer);
								sprintf(command,status_buffer_g,filename);
							}
						}
						System(command, NULL);

						Close(newfile);
						Close(file);
					}
					else
						Close(file);
				}
				set(wnd_main,MUIA_Window_Sleep,FALSE);
				if(lock != 0) UnLock(lock);
				delete [] line;
			}
			else
				MUI_RequestA(app,0,0,CatalogStr(MSG_VIEW_HEADERS,MSG_VIEW_HEADERS_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_PLEASE_SELECT_MESSAGE,MSG_PLEASE_SELECT_MESSAGE_STR),0);
			break;
		}
		case MEN_MOVE:
			move_multi(0);
			break;
		case MEN_COPY:
			move_multi(1);
			break;
		case MEN_KILLMESS:
		{
			getGdataDisplayed(&gdata);
			int result = 0;
			getMdataActivePos(&result);
			if(result!=MUIV_NList_Active_Off) {
				//if(!downloading_headers) {
					getMdataActive(&mdata);
					killmess(gdata,mdata);
				/*}
				else
					MUI_RequestA(app,0,0,CatalogStr(MSG_KILL_MESSAGE,MSG_KILL_MESSAGE_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_PLEASE_FINISH_DOWNLOADING_FIRST,MSG_PLEASE_FINISH_DOWNLOADING_FIRST_STR),0);*/
			}
			else
				MUI_RequestA(app,0,0,CatalogStr(MSG_KILL,MSG_KILL_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_PLEASE_SELECT_MESSAGE,MSG_PLEASE_SELECT_MESSAGE_STR),0);
			break;
		}
		case MEN_YAMADDRESS:
		{
			getGdataDisplayed(&gdata);
			int result = 0;
			getMdataActivePos(&result);
			if(result!=MUIV_NList_Active_Off) {
				getMdataActive(&mdata);
				exportAddress(gdata,mdata);
			}
			else
				MUI_RequestA(app,0,0,CatalogStr(MSG_EXPORT_ADDRESS,MSG_EXPORT_ADDRESS_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_PLEASE_SELECT_MESSAGE,MSG_PLEASE_SELECT_MESSAGE_STR),0);
			break;
		}
		case MEN_DOWNLOADBODY:
			/*if(delete_dis)
				MUI_RequestA(app,0,0,CatalogStr(MSG_DOWNLOADING_MESSAGE,MSG_DOWNLOADING_MESSAGE_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_CANNOT_DOWNLOAD_WHILE_DOWNLOADING_OTHER_MESSAGES,MSG_CANNOT_DOWNLOAD_WHILE_DOWNLOADING_OTHER_MESSAGES_STR),0);
			else*/
			{
				//get(NLIST_messagelistdata,MUIA_NList_Entries,&entries);
				entries = getMdataVisibleEntries();
				getGdataDisplayed(&gdata);
				sleepAll(TRUE);
				int max = 0;
				int k;
				for(k=0;k<entries;k++) {
					if(isMdataSelected(k)) {
						getMdataVisible(k,&mdata);
						if(mdata->flags[12])
							max++;
					}
				}
				if(max > 0) {
					StatusWindow * statusWindow = new StatusWindow(app,CatalogStr(MSG_DOWNLOADING_BODIES,MSG_DOWNLOADING_BODIES_STR));;
					MessageListData **mdataptr = new MessageListData *[max];
					int index = 0;
					int ndel = 0;
					for(k=0;k<entries;k++) {
						if(isMdataSelected(k)) {
							getMdataVisible(k,&mdata);
							if(mdata->flags[12]==TRUE) {
								index++; // increment first
								BOOL available = TRUE;
								BOOL okay = getBody(&available,gdata,mdata,statusWindow,index,max,(index==1),FALSE);
								if(!available && (account.flags & Account::NODELONLINE)==0 ) {
									mdataptr[ndel++] = mdata;
								}
								if(!okay && available)
									break;
								redrawMdata(k);
							}
						}
					}
					redrawGdataAll();
					if(ndel > 0 )
						statusWindow->setText(CatalogStr(MSG_DELETING_NOT_AVAILABLE_HEADERS,MSG_DELETING_NOT_AVAILABLE_HEADERS_STR));
					if(ndel > 0) {
						/*for(k=0;k<ndel;k++) {
							// delete message
							delete_mess(gdata,mdataptr[k],FALSE);
						}
						write_index_if_changed();*/
						delete_mess_n(gdata,mdataptr,ndel,FALSE);
					}
					delete [] mdataptr;
					delete statusWindow;
				}
				sleepAll(FALSE);
			}
			break;
		case MEN_SELECTALL:
			if(account.mdata_view==0)
				DoMethod(NLIST_messagelistdata,MUIM_NList_Select,MUIV_NList_Select_All,MUIV_NList_Select_On,NULL);
			else {
				DoMethod(LISTTREE_messagelistdata, MUIM_NListtree_Open, MUIV_NListtree_Open_ListNode_Root, MUIV_NListtree_Open_TreeNode_All, NULL);
				DoMethod(LISTTREE_messagelistdata,MUIM_NList_Select,MUIV_NList_Select_All,MUIV_NList_Select_On,NULL);
         }
			break;
		case MEN_SELECTNONE:
			if(account.mdata_view==0)
				DoMethod(NLIST_messagelistdata,MUIM_NList_Select,MUIV_NList_Select_All,MUIV_NList_Select_Off,NULL);
			else
				DoMethod(LISTTREE_messagelistdata,MUIM_NList_Select,MUIV_NList_Select_All,MUIV_NList_Select_Off,NULL);
			break;
		case MEN_SELECTTOGGLE:
			if(account.mdata_view==0)
				DoMethod(NLIST_messagelistdata,MUIM_NList_Select,MUIV_NList_Select_All,MUIV_NList_Select_Toggle,NULL);
			else {
				//DoMethod(LISTTREE_messagelistdata,MUIM_NList_Select,MUIV_NList_Select_All,MUIV_NList_Select_Toggle,NULL);
				setMdataQuiet(TRUE);
				get(NLIST_messagelistdata,MUIA_NList_Entries,&entries);
				for(int k=0;k<entries;k++) {
					BOOL sel = !isMdataSelected(k,FALSE);
					setMdataSelected(k,sel,FALSE);
				}
				setMdataQuiet(FALSE);
			}
			break;
		case MEN_SELECTFROM:
		{
			setMdataQuiet(TRUE);
			//entries = getMdataVisibleEntries();
			get(NLIST_messagelistdata,MUIA_NList_Entries,&entries);
			int result = 0;
			getMdataActivePos(&result);
			if(result!=MUIV_NList_Active_Off) {
				MessageListData * mdata2=NULL;
				getMdataActive(&mdata);
				for(int k=0;k<entries;k++) {
					//getMdataVisible(k,&mdata2);
					getMdata(k,&mdata2);
					//DoMethod(NLIST_messagelistdata,MUIM_NList_GetEntry,k,&mdata2);
					if(stricmp(mdata->from,mdata2->from)==0) {
						/*if(account.mdata_view==0)
							DoMethod(NLIST_messagelistdata,MUIM_NList_Select,k,MUIV_NList_Select_On,NULL);
						else {
							DoMethod(LISTTREE_messagelistdata,MUIM_NList_Select,k,MUIV_NList_Select_On,NULL);
						}*/
						//setMdataSelected(k,TRUE,TRUE);
						setMdataSelected(k,TRUE,FALSE);
					}
					else {
						/*if(account.mdata_view==0)
							DoMethod(NLIST_messagelistdata,MUIM_NList_Select,k,MUIV_NList_Select_Off,NULL);
						else
							DoMethod(LISTTREE_messagelistdata,MUIM_NList_Select,k,MUIV_NList_Select_Off,NULL);*/
						//setMdataSelected(k,FALSE,TRUE);
						setMdataSelected(k,FALSE,FALSE);
					}
				}
			}
			setMdataQuiet(FALSE);
			break;
		}
		case MEN_SELECTSUBJECT:
		{
			setMdataQuiet(TRUE);
			//entries = getMdataVisibleEntries();
			get(NLIST_messagelistdata,MUIA_NList_Entries,&entries);
			int result = 0;
			getMdataActivePos(&result);
			if(result!=MUIV_NList_Active_Off) {
				MessageListData * mdata2=NULL;
				getMdataActive(&mdata);
				temp = mdata->subject;
				while(*temp!=0 && (STRNICMP(temp,"RE: ",4)==0 || STRNICMP(temp,"SV: ",4)==0))
					temp += 4;
				for(int k=0;k<entries;k++) {
					//getMdataVisible(k,&mdata2);
					getMdata(k,&mdata2);
					temp2 = mdata2->subject;
					while(*temp2!=0 && (STRNICMP(temp2,"RE: ",4)==0 || STRNICMP(temp2,"SV: ",4)==0))
						temp2 += 4;
					BOOL sel = stricmp(temp,temp2) == 0;
					setMdataSelected(k,sel,FALSE);
					/*if(stricmp(temp,temp2)==0) {
						if(account.mdata_view==0)
							DoMethod(NLIST_messagelistdata,MUIM_NList_Select,k,MUIV_NList_Select_On,NULL);
						else
							DoMethod(LISTTREE_messagelistdata,MUIM_NList_Select,k,MUIV_NList_Select_On,NULL);
					}
					else {
						if(account.mdata_view==0)
							DoMethod(NLIST_messagelistdata,MUIM_NList_Select,k,MUIV_NList_Select_Off,NULL);
						else
							DoMethod(LISTTREE_messagelistdata,MUIM_NList_Select,k,MUIV_NList_Select_Off,NULL);
					}*/
				}
			}
			setMdataQuiet(FALSE);
			break;
		}
		case MEN_SELECTREAD:
		case MEN_SELECTUNREAD: {
			setMdataQuiet(TRUE);
			//entries = getMdataVisibleEntries();
			get(NLIST_messagelistdata,MUIA_NList_Entries,&entries);
			MessageListData * mdata=NULL;
			for(int k=0;k<entries;k++) {
				//getMdataVisible(k,&mdata);
				getMdata(k,&mdata);
				/*if(mdata->flags[1] == (returnID==MEN_SELECTUNREAD)) {
					if(account.mdata_view==0)
						DoMethod(NLIST_messagelistdata,MUIM_NList_Select,k,MUIV_NList_Select_On,NULL);
					else
						DoMethod(LISTTREE_messagelistdata,MUIM_NList_Select,k,MUIV_NList_Select_On,NULL);
				}
				else {
					if(account.mdata_view==0)
						DoMethod(NLIST_messagelistdata,MUIM_NList_Select,k,MUIV_NList_Select_Off,NULL);
					else
						DoMethod(LISTTREE_messagelistdata,MUIM_NList_Select,k,MUIV_NList_Select_Off,NULL);
				}*/
				BOOL sel = ( mdata->flags[1] == ( (*item) == MEN_SELECTUNREAD ) );
				setMdataSelected(k,sel,FALSE);
			}
			setMdataQuiet(FALSE);
			break;
		}
		case MEN_MARKUNREAD:
		case MEN_MARKREAD:
			//get(NLIST_messagelistdata,MUIA_NList_Entries,&entries);
			entries = getMdataVisibleEntries();
			if(entries>0) {
				getGdataDisplayed(&gdata);
				if(gdata->ID>=0) {
					setMdataQuiet(TRUE);
					set(app,MUIA_Application_Sleep,TRUE);
					MessageListData **mdataptr = new MessageListData *[entries];
					int count = 0;
					for(int k=0;k<entries;k++) {
						if(isMdataSelected(k)) {
							getMdataVisible(k,&mdata);
							if( (*item) == MEN_MARKUNREAD ) {
								if(!mdata->flags[1]) {
									mdata->flags[1]=TRUE;
									mdataptr[count++] = mdata;
								}
							}
							else {
								if(mdata->flags[1]) {
									mdata->flags[1]=FALSE;
									mdataptr[count++] = mdata;
								}
							}
							redrawMdata(k);
						}
					}
					//gdata->flags[1]=TRUE;
					write_index_update_multi(gdata,mdataptr,count,NLIST_messagelistdata);
					delete [] mdataptr;
					if( (*item) == MEN_MARKUNREAD )
						gdata->num_unread += count;
					else
						gdata->num_unread -= count;
					redrawGdataActive();
					setMdataQuiet(FALSE);
					update_screen_title(gdata);
					set(app,MUIA_Application_Sleep,FALSE);
				}
				else {
					if( (*item) == MEN_MARKUNREAD )
						MUI_RequestA(app,0,0,CatalogStr(MSG_MARK_MESSAGE,MSG_MARK_MESSAGE_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_ONLY_MESSAGES_IN_GROUPS_CAN_BE_MARKED_UNREAD,MSG_ONLY_MESSAGES_IN_GROUPS_CAN_BE_MARKED_UNREAD_STR),0);
					else
						MUI_RequestA(app,0,0,CatalogStr(MSG_MARK_MESSAGE,MSG_MARK_MESSAGE_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_ONLY_MESSAGES_IN_GROUPS_CAN_BE_MARKED_READ,MSG_ONLY_MESSAGES_IN_GROUPS_CAN_BE_MARKED_READ_STR),0);
				}
			}
			break;
		case MEN_MARKHELD:
		case MEN_MARKQUEUED:
		case MEN_MARKIMPORTANT:
		case MEN_MARKNORMAL:
			//get(NLIST_messagelistdata,MUIA_NList_Entries,&entries);
			entries = getMdataVisibleEntries();
			if(entries>0) {
				getGdataDisplayed(&gdata);
				if(gdata->ID==-1 || ((*item)!=MEN_MARKHELD && (*item)!=MEN_MARKQUEUED)) {
					setMdataQuiet(TRUE);
					set(app,MUIA_Application_Sleep,TRUE);
					MessageListData **mdataptr = new MessageListData *[entries];
					int count = 0;
					for(int k=0;k<entries;k++) {
						if(isMdataSelected(k)) {
							getMdataVisible(k,&mdata);
							if( (*item) == MEN_MARKHELD ) {
								if(!mdata->flags[0]) {
									mdata->flags[0] = TRUE;
									mdataptr[count++] = mdata;
								}
							}
							else if( (*item) == MEN_MARKQUEUED ) {
								if(mdata->flags[0]) {
									mdata->flags[0] = FALSE;
									mdataptr[count++] = mdata;
								}
							}
							else if( (*item) == MEN_MARKNORMAL ) {
								if(mdata->flags[13]) {
									mdata->flags[13] = FALSE;
									mdataptr[count++] = mdata;
								}
							}
							else if( (*item) == MEN_MARKIMPORTANT ) {
								if(!mdata->flags[13]) {
									mdata->flags[13] = TRUE;
									mdataptr[count++] = mdata;
								}
							}
							redrawMdata(k);
						}
					}
					//gdata->flags[1]=TRUE;
					write_index_update_multi(gdata,mdataptr,count,NLIST_messagelistdata);
					delete [] mdataptr;
					setMdataQuiet(FALSE);
					set(app,MUIA_Application_Sleep,FALSE);
				}
				else {
					if( (*item) == MEN_MARKHELD )
						MUI_RequestA(app,0,0,CatalogStr(MSG_MARK_MESSAGE,MSG_MARK_MESSAGE_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_ONLY_MESSAGES_IN_OUTGOING_CAN_BE_MARKED_HELD,MSG_ONLY_MESSAGES_IN_OUTGOING_CAN_BE_MARKED_HELD_STR),0);
					else
						MUI_RequestA(app,0,0,CatalogStr(MSG_MARK_MESSAGE,MSG_MARK_MESSAGE_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_ONLY_MESSAGES_IN_OUTGOING_CAN_BE_MARKED_QUEUED,MSG_ONLY_MESSAGES_IN_OUTGOING_CAN_BE_MARKED_QUEUED_STR),0);
				}
			}
			break;
		case MEN_CURRENTDATE:
			//get(NLIST_messagelistdata,MUIA_NList_Entries,&entries);
			entries = getMdataVisibleEntries();
			if(entries>0) {
				getGdataDisplayed(&gdata);
				if(gdata->ID==-1) {
					setMdataQuiet(TRUE);
					set(app,MUIA_Application_Sleep,TRUE);
					//char date[256] = "";
					for(int k=0;k<entries;k++) {
						if(isMdataSelected(k)) {
							getMdataVisible(k,&mdata);
							//DateHandler::get_datenow(mdata->date,getGMTOffset(),account.bst);
							//DateHandler::read_date(&mdata->ds,mdata->date,mdata->c_date,mdata->c_time);
							DateHandler::get_datenow(status_buffer_g,getGMTOffset(),account.bst);
							//printf("DATE: %s\n",status_buffer_g);
							DateHandler::read_date(&mdata->ds,status_buffer_g,mdata->c_date,mdata->c_time);
							//printf("%s %s\n",mdata->c_date,mdata->c_time);
							mdata->datec[0] = '\0';
							changedate(gdata,mdata,status_buffer_g,NLIST_messagelistdata);
							redrawMdata(k);
						}
					}
					setMdataQuiet(FALSE);
					//gdata->flags[1]=TRUE;
					set(app,MUIA_Application_Sleep,FALSE);
				}
				else
					MUI_RequestA(app,0,0,CatalogStr(MSG_CHANGE_DATE,MSG_CHANGE_DATE_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_YOU_CAN_CHANGE_DATE_OF_NOT_SENT,MSG_YOU_CAN_CHANGE_DATE_OF_NOT_SENT_STR),0);
			}
			break;
		case MEN_JOINMSGS:
			get(wnd_joinmsgs,MUIA_Window_Open,&yesno);
			if(!yesno) {
				set(STR_joinmsgs_SUBJECT,MUIA_String_Contents,"");
				DoMethod(NLIST_joinmsgs_MSGS,MUIM_NList_Clear);
				//get(NLIST_messagelistdata,MUIA_NList_Entries,&entries);
				entries = getMdataVisibleEntries();
				if(entries>0)
				{
					getGdataDisplayed(&gdata);
					setMdataQuiet(TRUE);
					set(app,MUIA_Application_Sleep,TRUE);

					int count = 0;
					for(int k=0;k<entries;k++)
					{
						if(isMdataSelected(k))
						{
							getMdataVisible(k,&mdata);

							if(mdata->flags[12]==TRUE) {
								// we need to download the BODY!
								sleepAll(TRUE);
								BOOL available = TRUE;
								getBody(&available,gdata,mdata);
								sleepAll(FALSE);
							}

							if(mdata->flags[12]==FALSE) {
								if(count==0) {
									NewMessage newmess;
									get_refs(&newmess,gdata,mdata,GETREFS_NONE);
									set(STR_joinmsgs_SUBJECT,MUIA_String_Contents,newmess.subject);
								}

								MessageListData * mdata2 = new MessageListData();
								*mdata2 = *mdata;
								mdata2->flags[6] = gdata->ID;
								DoMethod(NLIST_joinmsgs_MSGS,MUIM_NList_InsertSingle,mdata2,MUIV_NList_Insert_Bottom);
								count++;
							}
						}
					}

					write_index_if_changed();
					setMdataQuiet(FALSE);
					set(app,MUIA_Application_Sleep,FALSE);
					set(wnd_joinmsgs,MUIA_Window_Open,TRUE);
				}
				set(wnd_joinmsgs,MUIA_Window_Open,TRUE);
			}
			else {
				set(wnd_joinmsgs,MUIA_Window_Open,TRUE);
			}
			break;
		case MEN_ACCOUNT:
			/*get(wnd_write,MUIA_Window_Open,&yesno);
			if(!yesno)
			{*/
				get(wnd_account,MUIA_Window_Open,&yesno);
				if(yesno)
					set(wnd_account,MUIA_Window_Open,TRUE);
				else
				{
				set(STR_acc_NAME,MUIA_String_Contents,account.name);
				set(STR_acc_EMAIL,MUIA_String_Contents,account.email);
				set(STR_acc_REALEMAIL,MUIA_String_Contents,account.realemail);
				set(STR_acc_ORG,MUIA_String_Contents,account.org);
				set(STR_acc_SMTP,MUIA_String_Contents,account.smtp);
				set(STR_acc_DOMAIN,MUIA_String_Contents,account.domain);
				set(STR_acc_FOLLOWUPTEXT,MUIA_String_Contents,account.followup_text);
				set(STR_acc_CHARSETWRITE,MUIA_Text_Contents,account.charset_write);
				set(CY_acc_TIMEZONE,MUIA_Cycle_Active,account.timezone+12);
				set(CM_acc_USELOCALETZ,MUIA_Selected,account.uselocaletz);
				set(CM_acc_BST,MUIA_Selected,account.bst);
				set(CY_acc_DATEFORMAT,MUIA_Cycle_Active,account.dateformat);
				if((account.flags & Account::LOGGING)!=0)
					set(CM_acc_LOGGING,MUIA_Selected,TRUE);
				else
					set(CM_acc_LOGGING,MUIA_Selected,FALSE);
				if((account.flags & Account::LOGDEL)!=0)
					set(CM_acc_LOGDEL,MUIA_Selected,TRUE);
				else
					set(CM_acc_LOGDEL,MUIA_Selected,FALSE);
				set(CM_acc_VGROUPDL,MUIA_Selected,account.vgroupdl);
				set(CM_acc_NOCONFIRMDEL,MUIA_Selected,account.noconfirmdel);
				if((account.flags & Account::NODELONLINE)==0) {
					// opposite logic, so that it is turned on by default when
					// upgrading from older version before this flag
					set(CM_acc_DELONLINE,MUIA_Selected,TRUE);
				}
				else
					set(CM_acc_DELONLINE,MUIA_Selected,FALSE);

				if((account.flags & Account::NOCONFIRMQUIT)==0) {
					// opposite logic, so that it is turned on by default when
					// upgrading from older version before this flag
					set(CM_acc_CONFIRMQUIT,MUIA_Selected,TRUE);
				}
				else
					set(CM_acc_CONFIRMQUIT,MUIA_Selected,FALSE);

				set(CM_acc_QUIETDL,MUIA_Selected,((account.flags & Account::QUIETDL)!=0));

				set(CM_acc_CHECKFORDUPS,MUIA_Selected,((account.flags & Account::CHECKFORDUPS)!=0));

				set(CM_acc_XNEWS,MUIA_Selected,account.xnews);
				//set(CM_acc_USESIG,MUIA_Selected,account.usesig);
				set(STR_acc_LINELENGTH,MUIA_String_Integer,account.linelength);
				if(account.rewrap==1)
					set(CM_acc_REWRAP,MUIA_Selected,TRUE);
				else
					set(CM_acc_REWRAP,MUIA_Selected,FALSE);
				if((account.flags & Account::SNIPSIG)!=0)
					set(CM_acc_SNIPSIG,MUIA_Selected,TRUE);
				else
					set(CM_acc_SNIPSIG,MUIA_Selected,FALSE);
				set(CY_acc_XNO,MUIA_Cycle_Active,account.xno);
				set(CY_acc_SIG,MUIA_Cycle_Active,0);
				cy_current_sig=0;

				set(CM_acc_LISTFLAGS,MUIA_Selected,((account.listflags & Account::LISTFLAGS)!=0));
				set(CM_acc_LISTFROMGROUP,MUIA_Selected,((account.listflags & Account::LISTFROMGROUP)!=0));
				set(CM_acc_LISTDATE,MUIA_Selected,((account.listflags & Account::LISTDATE)!=0));
				set(CM_acc_LISTSUBJECT,MUIA_Selected,((account.listflags & Account::LISTSUBJECT)!=0));
				set(CM_acc_LISTSIZE,MUIA_Selected,((account.listflags & Account::LISTSIZE)!=0));
				set(CM_acc_LISTLINES,MUIA_Selected,((account.listflags & Account::LISTLINES)!=0));

				DoMethod(NLIST_acc_MIMEPREFS,MUIM_NList_Clear);
				for(int k=0;k<account.mimeprefsno;k++) {
					ptr=(MIMEPrefs *)new MIMEPrefs;
					*(MIMEPrefs *)ptr=*account.mimeprefs[k];
					DoMethod(NLIST_acc_MIMEPREFS,MUIM_NList_InsertSingle,ptr,MUIV_NList_Insert_Bottom);
				}
				if(account.mimeprefsno>0)
					set(NLIST_acc_MIMEPREFS,MUIA_NList_Active,MUIV_NList_Active_Top);
				set(STR_acc_MIMEDEF,MUIA_String_Contents,account.defviewer);
				set(STR_acc_MIME,MUIA_String_Contents,"");
				set(STR_acc_MIMEVIEW,MUIA_String_Contents,"");
				set(CY_acc_READHEADER,MUIA_Cycle_Active,account.readheader_type);
				set(STR_acc_READHEADER,MUIA_String_Contents,account.readheaders);
				set(PEN_acc_TEXT_QUOTE2,MUIA_Pendisplay_Spec,&account.pen_acc_text_quote2);
				set(PEN_acc_TEXT_COL,MUIA_Pendisplay_Spec,&account.pen_acc_text_col);
				set(CM_acc_MULTIPLEVWS,MUIA_Selected,account.multiplevws);
				set(wnd_account,MUIA_Window_ActiveObject,STR_acc_NAME);
				DoMethod(ED_acc_SIG,MUIM_TextEditor_ClearText);
				set(NLIST_acc_SERVERS,MUIA_NList_Active,MUIV_NList_Active_Top);
				set(pages_account,MUIA_Group_ActivePage,0);
				set(wnd_account,MUIA_Window_Open,TRUE);
				}
			/*}
			else
			{
				MUI_RequestA(app,0,0,CatalogStr(MSG_ACCOUNTS,MSG_ACCOUNTS_STR),CatalogStr(MSG_OKAY,MSG_OKAY_STR),CatalogStr(MSG_IMPOSSIBLE_TO_CHANGE_SETTINGS_WHEN_EDITING_MESSAGE,MSG_IMPOSSIBLE_TO_CHANGE_SETTINGS_WHEN_EDITING_MESSAGE_STR),0);
				set(wnd_write,MUIA_Window_Open,TRUE);
			}*/
			break;
		case MEN_KILLFILE:
			set(CM_acc_SIZEKILL,MUIA_Selected,account.use_sizekill);
			set(wnd_killfile,MUIA_Window_Open,TRUE);
			break;
		case MEN_USERS:
			set(wnd_users,MUIA_Window_Open,TRUE);
			break;
		case MEN_CPWD:
			get(wnd_cpwd,MUIA_Window_Open,&yesno);
			if(!yesno)
			{
				set(STR_cpwd_PASS,MUIA_String_Contents,"");
				set(CM_cpwd_PASS,MUIA_Selected,currentUser->requiresPassword());
				set(STR_cpwd_PASS,MUIA_Disabled,!currentUser->requiresPassword());
				/*strcpy(status_buffer_g,CatalogStr(MSG_ENTER_NEW_PASSWORD,MSG_ENTER_NEW_PASSWORD_STR));
				set(TXT_cpwd_INFO,MUIA_String_Contents,status_buffer_g);*/
			}
			set(wnd_cpwd,MUIA_Window_Open,TRUE);
			break;
		case MEN_MUIPREFS:
			DoMethod(app,MUIM_Application_OpenConfigWindow,0);
			break;
		/*case ACC_STRKILL:
			callback_acc_strkill(FALSE);
			break;
		case ACC_STRKILLTEMP:
			callback_acc_strkill(TRUE);
			break;*/
	}
	//return TRUE;
}

HOOK3( ULONG, TextEditor_Dispatcher, IClass *, cl, a0, Object *, obj, a2, MUIP_TextEditor_HandleError *, msg, a1 )
	MUIP_DragDrop *drop_msg=NULL;
	switch(msg->MethodID)
	{
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
				MUI_Request(app,wnd_account, 0L, NULL, CatalogStr(MSG_CONTINUE,MSG_CONTINUE_STR), errortxt);
			}
			break;
	}
	return DoSuperMethodA(cl,obj,(Msg)msg);
}

HOOK2( void, desfunc_groupdata, APTR, pool, a2, GroupData *, entry, a1 )
	/*if(entry)
		delete [] ((char *)entry);*/
	if(entry)
		delete entry;
}

HOOK3( void, desfunc_groupdatatree, CPPHook *, hook, a0, Object *, obj, a2, MUIP_NListtree_DestructMessage *, msg, a1 )
	if(msg->UserData)
	{
		delete (GroupData *)msg->UserData;
		msg->UserData=NULL;
	}
}

HOOK2( void, desfunc_messagelistdata, APTR, pool, a2, MessageListData *, entry, a1 )
	if(entry)
		delete entry;
}

HOOK2( void, desfunc_search, APTR, pool, a2, MessageListData *, entry, a1 )
	if(entry)
		delete entry;
}

HOOK2( void, desfunc_joinmsgs, APTR, pool, a2, MessageListData *, entry, a1 )
	if(entry)
		delete entry;
}

HOOK2( void, desfunc_stats, APTR, pool, a2, StatsList *, entry, a1 )
	if(entry)
		delete entry;
}

HOOK2( void, desfunc_users, APTR, pool, a2, User *, entry, a1 )
	if(entry)
		delete entry;
}

HOOK2( void, desfunc_groupman, APTR, pool, a2, char *, entry, a1 )
	if(entry)
		delete entry;
}

HOOK2( void, desfunc_servers, APTR, pool, a2, Server *, entry, a1 )
	if(entry)
		delete entry;
}

HOOK2( void, desfunc_killfile, APTR, pool, a2, KillFile *, entry, a1 )
	if(entry)
		delete entry;
}

HOOK2( void, desfunc_mime, APTR, pool, a2, MIMEType *, entry, a1 )
	if(entry)
		delete entry;
}

HOOK2( void, desfunc_mimeprefs, APTR, pool, a2, MIMEPrefs *, entry, a1 )
	if(entry)
		delete entry;
}

HOOK2( LONG, dispfunc_simpleglist, char **, array, a2, GroupData *, data, a1 )
	if(data) {
		switch (data->ID)
		{
		case -1:
			*array=CatalogStr(MSG_FOLDER_OUTGOING,MSG_FOLDER_OUTGOING_STR);
		break;
		case -2:
			*array=CatalogStr(MSG_FOLDER_SENT,MSG_FOLDER_SENT_STR);
		break;
		case -3:
			*array=CatalogStr(MSG_FOLDER_DELETED,MSG_FOLDER_DELETED_STR);
		break;
		default:
			*array = (*data->desc!=0) ? data->desc : data->name;
		}
	}
	else {
		*array = CatalogStr(MSG_GROUP,MSG_GROUP_STR);
	}
	return(0);
}

HOOK2( LONG, dispfunc_groupdata, char **, array, a2, GroupData *, data, a1 )
	static char buf0[512];
	static char buf1[16];
	char *name = NULL;
	if(data) {
		switch (data->ID)
		{
		case -1:
			name=CatalogStr(MSG_FOLDER_OUTGOING,MSG_FOLDER_OUTGOING_STR);
		break;
		case -2:
			name=CatalogStr(MSG_FOLDER_SENT,MSG_FOLDER_SENT_STR);
		break;
		case -3:
			name=CatalogStr(MSG_FOLDER_DELETED,MSG_FOLDER_DELETED_STR);
		break;
		default:
			name = ( *data->desc != 0 ) ? data->desc : data->name;
		break;
		}
		if(data->num_unread > 0) {
			sprintf(buf0,"\33b%s",name);
			*array++ = buf0;
		}
		else
			*array++ = name;

		sprintf(buf1,"%d",data->nummess);
		*array++ = buf1;

		if(data->s)
			*array++ = (char*)"S";
		else
			*array++ = (char*)"";
		if(data->ID<0)
			*array = (char*)"";
		else if(data->flags[5])
			*array = CatalogStr(MSG_Y,MSG_Y_STR);
		else
			*array = CatalogStr(MSG_N,MSG_N_STR);
	}
	else {
		*array++ = CatalogStr(MSG_GROUP,MSG_GROUP_STR);
		*array++ = CatalogStr(MSG_MSGS,MSG_MSGS_STR);
		*array++ = CatalogStr(MSG_SUB,MSG_SUB_STR);
		*array = CatalogStr(MSG_ONLINE,MSG_ONLINE_STR);
	}
	return(0);
}

HOOK3( LONG, dispfunc_groupdatatree, CPPHook *, hook, a0, Object *, obj, a2, MUIP_NListtree_DisplayMessage *, msg, a1 )
	static char buf0[512];
	static char buf1[16];
	char **array = msg->Array;
	char *name = NULL;
	if(msg->TreeNode) {
		GroupData * data = (GroupData *)msg->TreeNode->tn_User;
		if(data) {
			switch (data->ID)
			{
			case -1:
				name=CatalogStr(MSG_FOLDER_OUTGOING,MSG_FOLDER_OUTGOING_STR);
			break;
			case -2:
				name=CatalogStr(MSG_FOLDER_SENT,MSG_FOLDER_SENT_STR);
			break;
			case -3:
				name=CatalogStr(MSG_FOLDER_DELETED,MSG_FOLDER_DELETED_STR);
			break;
			default:
				name = ( *data->desc != 0 ) ? data->desc : data->name;
			break;
			}
			if(data->num_unread > 0) {
				sprintf(buf0,"\33b%s",name);
				*array++ = buf0;
			}
			else
				*array++ = name;

			sprintf(buf1,"%d",data->nummess);
			*array++ = buf1;
			if(data->s)
				*array++ = (char *)"S";
			else
				*array++ = (char *)"";
			if(data->ID<0)
				*array = (char *)"";
			else if(data->flags[5])
				*array = CatalogStr(MSG_Y,MSG_Y_STR);
			else
				*array = CatalogStr(MSG_N,MSG_N_STR);
		}
		else {
			if(msg->TreeNode->tn_Name)
				*array++ = msg->TreeNode->tn_Name;
			else
				*array++ = (char *)"";

			*array++ = (char *)"";
			*array = (char *)"";
		}
	}
	else {
		*array++ = CatalogStr(MSG_GROUP,MSG_GROUP_STR);
		*array++ = CatalogStr(MSG_MSGS,MSG_MSGS_STR);
		*array++ = CatalogStr(MSG_SUB,MSG_SUB_STR);
		*array = CatalogStr(MSG_ONLINE,MSG_ONLINE_STR);
	}
	return(0);
}

HOOK3( LONG, confunc_groupdatatree, CPPHook *, hook, a0, Object *, obj, a2, MUIP_NListtree_ConstructMessage *, msg, a1 )
	GroupData *gdata = (GroupData *)msg->UserData;
	return ((LONG)gdata);
}


HOOK2( LONG, dispfunc_messagelistdata, char **, array, a2, MessageListData *, data, a1 )
	char *translated_string = NULL;
	static int gID;
	static char buf0[16];
	static char buf1[512];
	static char buf2[16];
	static char buf3[16];
	static char buf4[64];
	//static char buf5[MAXLINE];
	if(data) {
		//printf("%s\n",data->subject);
		if((account.listflags & Account::LISTFLAGS)!=0)
		{
			char *buf0_ptr = buf0;
			if(data->flags[0]==TRUE)
			{
				strcpy(buf0, "\033o[4] ");
			}
			else if(data->flags[10]==TRUE)
			{
				strcpy(buf0, "\033o[8] ");
			}
			else if(data->flags[1]==TRUE)
			{
				strcpy(buf0, "\033o[1] ");
			}
			else if(data->flags[1]==FALSE)
			{
				strcpy(buf0, "\033o[2] ");
			}
			if(data->flags[12]==TRUE)
			{
				strcat(buf0, "\033o[9] ");
			}
			*array++ = buf0;
		}
		else
		{
			char *buf0_ptr = buf0;
			*buf0_ptr = '\0';
			*array++ = buf0;
		}
		translated_string = translateCharset((unsigned char *)data->subject,NULL);
		if (translated_string)
			strcpy(data->subject,translated_string);

		if((account.listflags & Account::LISTSUBJECT)!=0) {
			if(!data->flags[13])
				*array++ = data->subject;
			else {
				sprintf(buf1,"\0338%.63s",data->subject);
				*array++ = buf1;
			}
		}

		if((account.listflags & Account::LISTDATE)!=0) {
			//if(*data->datec == '\0')
			//	sprintf(data->datec,"%s %s",data->c_date,data->c_time);
			//*array++ = data->datec;
			sprintf(buf4,"%s %s",data->c_date,data->c_time);
			*array++ = buf4;
		}

		if((account.listflags & Account::LISTFROMGROUP)!=0) {
			if(gID<0) {
				/*NewMessage newmess;
				get_refs(&newmess,gID,data->ID,GETREFS_NONE);
				*array++ = newmess.newsgroups;*/
				/*get_newsgroups(buf5,gID,data->ID);
				*array++ = buf5;*/
				*array++ = data->newsgroups;
			}
			else
			{
				translated_string = translateCharset((unsigned char *)data->from,NULL);
				if (translated_string)
					strcpy(data->from,translated_string);
				*array++ = data->from;
			}
		}

		if((account.listflags & Account::LISTSIZE)!=0) {
			sprintf(buf2,"%d",data->size);
			*array++ = buf2;
		}

		if((account.listflags & Account::LISTLINES)!=0) {
			sprintf(buf3,"%d",data->flags[8]);
			*array++ = buf3;
		}

		array--;
	}
	else {
		GroupData * gdata=NULL;
		getGdataDisplayed(&gdata);
		if(gdata)
			gID=gdata->ID;
		else
			gID=0;
		if((account.listflags & Account::LISTFLAGS)!=0)
			*array++ = (char *)" ";
		if((account.listflags & Account::LISTSUBJECT)!=0)
			*array++ = CatalogStr(MSG_SUBJECT_BOLD,MSG_SUBJECT_BOLD_STR);
		if((account.listflags & Account::LISTDATE)!=0)
			*array++ = CatalogStr(MSG_DATE,MSG_DATE_STR);
		if((account.listflags & Account::LISTFROMGROUP)!=0) {
			if(gID<0)
				*array++ = CatalogStr(MSG_NEWSGROUPS_BOLD,MSG_NEWSGROUPS_BOLD_STR);
			else
				*array++ = CatalogStr(MSG_FROM_BOLD,MSG_FROM_BOLD_STR);
		}
		if((account.listflags & Account::LISTSIZE)!=0)
			*array++ = CatalogStr(MSG_SIZE,MSG_SIZE_STR);
		if((account.listflags & Account::LISTLINES)!=0)
			*array++ = CatalogStr(MSG_LINES,MSG_LINES_STR);

		array--;
	}
	return(0);
}

HOOK3( LONG, dispfunc_messagelistdatatree, CPPHook *, hook, a0, Object *, obj, a2, MUIP_NListtree_DisplayMessage *, msg, a1 )
	static int gID;
	static char buf0[16];
	static char buf1[512];
	static char buf2[16];
	static char buf3[16];
	static char buf4[64];
	char **array = msg->Array;
	if(msg->TreeNode) {
		MessageListData * data = (MessageListData *)msg->TreeNode->tn_User;
		if(data)
		{
			if((account.listflags & Account::LISTFLAGS)!=0)
			{
				char *buf0_ptr = buf0;
				if(data->flags[0]==TRUE)
				{
					strcpy(buf0, "\033o[4] ");
				}
				else if(data->flags[10]==TRUE)
				{
					strcpy(buf0, "\033o[8] ");
				}
				else if(data->flags[1]==TRUE)
				{
					strcpy(buf0, "\033o[1] ");
				}
				else if(data->flags[1]==FALSE)
				{
					strcpy(buf0, "\033o[2] ");
				}
				if(data->flags[12]==TRUE)
				{
					strcat(buf0, "\033o[9] ");
				}
				*array++ = buf0;
			}

			if((account.listflags & Account::LISTSUBJECT)!=0) {
				if(!data->flags[13])
					*array++ = data->subject;
				else {
					sprintf(buf1,"\0338%.63s",data->subject);
					*array++ = buf1;
				}
			}

			if((account.listflags & Account::LISTDATE)!=0) {
				//if(*data->datec == '\0')
				//	sprintf(data->datec,"%s %s",data->c_date,data->c_time);
				//*array++ = data->datec;
				sprintf(buf4,"%s %s",data->c_date,data->c_time);
				*array++ = buf4;
			}

			if((account.listflags & Account::LISTFROMGROUP)!=0) {
				if(gID<0)
					*array++ = data->newsgroups;
				else
					*array++ = data->from;
			}

			if((account.listflags & Account::LISTSIZE)!=0) {
				sprintf(buf2,"%d",data->size);
				*array++ = buf2;
			}

			if((account.listflags & Account::LISTLINES)!=0) {
				sprintf(buf3,"%d",data->flags[8]);
				*array++ = buf3;
			}

			array--;
		}
		else {
			*array++ = msg->TreeNode->tn_Name;
			*array++ = (char *)"";
			*array = (char *)"";
		}
	}
	else {
		GroupData * gdata=NULL;
		getGdataDisplayed(&gdata);
		if(gdata)
			gID=gdata->ID;
		else
			gID=0;

		if((account.listflags & Account::LISTFLAGS)!=0)
			*array++ = (char *)" ";
		if((account.listflags & Account::LISTSUBJECT)!=0)
			*array++ = CatalogStr(MSG_SUBJECT_BOLD,MSG_SUBJECT_BOLD_STR);
		if((account.listflags & Account::LISTDATE)!=0)
			*array++ = CatalogStr(MSG_DATE,MSG_DATE_STR);
		if((account.listflags & Account::LISTFROMGROUP)!=0) {
			if(gID<0)
				*array++ = CatalogStr(MSG_NEWSGROUPS_BOLD,MSG_NEWSGROUPS_BOLD_STR);
			else
				*array++ = CatalogStr(MSG_FROM_BOLD,MSG_FROM_BOLD_STR);
		}
		if((account.listflags & Account::LISTSIZE)!=0)
			*array++ = CatalogStr(MSG_SIZE,MSG_SIZE_STR);
		if((account.listflags & Account::LISTLINES)!=0)
			*array++ = CatalogStr(MSG_LINES,MSG_LINES_STR);

		array--;

		/**array++ = " ";
		*array++ = CatalogStr(MSG_SUBJECT_BOLD,MSG_SUBJECT_BOLD_STR);
		*array++ = CatalogStr(MSG_DATE,MSG_DATE_STR);
		if(gID<0)
			*array++ = CatalogStr(MSG_NEWSGROUPS_BOLD,MSG_NEWSGROUPS_BOLD_STR);
		else
			*array++ = CatalogStr(MSG_FROM_BOLD,MSG_FROM_BOLD_STR);
		*array++ = CatalogStr(MSG_SIZE,MSG_SIZE_STR);
		*array = CatalogStr(MSG_LINES,MSG_LINES_STR);*/
	}
	return(0);
}

HOOK2( LONG, dispfunc_search, char **, array, a2, MessageListData *, data, a1 )
	static char buf0[16];
	static char buf1[512];
	static char buf2[16];
	static char buf3[16];
	static char buf4[64];
	if(data)
	{
		if((account.listflags & Account::LISTFLAGS)!=0)
		{
			char *buf0_ptr = buf0;
			if(data->flags[0]==TRUE)
			{
				strcpy(buf0, "\033o[4] ");
			}
			else if(data->flags[10]==TRUE)
			{
				strcpy(buf0, "\033o[8] ");
			}
			else if(data->flags[1]==TRUE)
			{
				strcpy(buf0, "\033o[1] ");
			}
			else if(data->flags[1]==FALSE)
			{
				strcpy(buf0, "\033o[2] ");
			}
			if(data->flags[12]==TRUE)
			{
				strcat(buf0, "\033o[9] ");
			}
			*array++ = buf0;
		}

		if((account.listflags & Account::LISTSUBJECT)!=0) {
			if(!data->flags[13])
				*array++ = data->subject;
			else {
				sprintf(buf1,"\0338%.63s",data->subject);
				*array++ = buf1;
			}
		}

		if((account.listflags & Account::LISTDATE)!=0) {
			//if(*data->datec == '\0')
			//	sprintf(data->datec,"%s %s",data->c_date,data->c_time);
			//*array++ = data->datec;
			sprintf(buf4,"%s %s",data->c_date,data->c_time);
			*array++ = buf4;
		}

		if((account.listflags & Account::LISTFROMGROUP)!=0)
			*array++ = data->from;

		if((account.listflags & Account::LISTSIZE)!=0) {
			sprintf(buf2,"%d",data->size);
			*array++ = buf2;
		}

		if((account.listflags & Account::LISTLINES)!=0) {
			sprintf(buf3,"%d",data->flags[8]);
			*array++ = buf3;
		}

		array--;

	}
	else {
		if((account.listflags & Account::LISTFLAGS)!=0)
			*array++ = (char *)" ";
		if((account.listflags & Account::LISTSUBJECT)!=0)
			*array++ = CatalogStr(MSG_SUBJECT_BOLD,MSG_SUBJECT_BOLD_STR);
		if((account.listflags & Account::LISTDATE)!=0)
			*array++ = CatalogStr(MSG_DATE,MSG_DATE_STR);
		if((account.listflags & Account::LISTFROMGROUP)!=0)
			*array++ = CatalogStr(MSG_FROM_BOLD,MSG_FROM_BOLD_STR);
		if((account.listflags & Account::LISTSIZE)!=0)
			*array++ = CatalogStr(MSG_SIZE,MSG_SIZE_STR);
		if((account.listflags & Account::LISTLINES)!=0)
			*array++ = CatalogStr(MSG_LINES,MSG_LINES_STR);

		array--;

	}
	return(0);
}

HOOK2( LONG, dispfunc_joinmsgs, char **, array, a2, MessageListData *, data, a1 )
	static char buf0[16];
	static char buf1[512];
	static char buf2[16];
	static char buf3[16];
	static char buf4[64];
	if(data)
	{
		if((account.listflags & Account::LISTFLAGS)!=0)
		{
			char *buf0_ptr = buf0;
			if(data->flags[0]==TRUE)
			{
				strcpy(buf0, "\033o[4] ");
			}
			else if(data->flags[10]==TRUE)
			{
				strcpy(buf0, "\033o[8] ");
			}
			else if(data->flags[1]==TRUE)
			{
				strcpy(buf0, "\033o[1] ");
			}
			else if(data->flags[1]==FALSE)
			{
				strcpy(buf0, "\033o[2] ");
			}
			if(data->flags[12]==TRUE)
			{
				strcat(buf0, "\033o[9] ");
			}
			*array++ = buf0;
		}

		if((account.listflags & Account::LISTSUBJECT)!=0) {
			if(!data->flags[13])
				*array++ = data->subject;
			else {
				sprintf(buf1,"\0338%.63s",data->subject);
				*array++ = buf1;
			}
		}

		if((account.listflags & Account::LISTDATE)!=0) {
			sprintf(buf4,"%s %s",data->c_date,data->c_time);
			*array++ = buf4;
		}

		if((account.listflags & Account::LISTFROMGROUP)!=0)
			*array++ = data->from;

		if((account.listflags & Account::LISTSIZE)!=0) {
			sprintf(buf2,"%d",data->size);
			*array++ = buf2;
		}

		if((account.listflags & Account::LISTLINES)!=0) {
			sprintf(buf3,"%d",data->flags[8]);
			*array++ = buf3;
		}

		array--;

	}
	else {
		if((account.listflags & Account::LISTFLAGS)!=0)
			*array++ = (char *)" ";
		if((account.listflags & Account::LISTSUBJECT)!=0)
			*array++ = CatalogStr(MSG_SUBJECT_BOLD,MSG_SUBJECT_BOLD_STR);
		if((account.listflags & Account::LISTDATE)!=0)
			*array++ = CatalogStr(MSG_DATE,MSG_DATE_STR);
		if((account.listflags & Account::LISTFROMGROUP)!=0)
			*array++ = CatalogStr(MSG_FROM_BOLD,MSG_FROM_BOLD_STR);
		if((account.listflags & Account::LISTSIZE)!=0)
			*array++ = CatalogStr(MSG_SIZE,MSG_SIZE_STR);
		if((account.listflags & Account::LISTLINES)!=0)
			*array++ = CatalogStr(MSG_LINES,MSG_LINES_STR);

		array--;

	}
	return(0);
}

HOOK2( LONG, dispfunc_stats, char **, array, a2, StatsList *, data, a1 )
	static char buf1[16];
	if(data) {
		sprintf(buf1,"%d",data->freq);
		*array++ = data->value;
		*array = buf1;
	}
	else {
		*array++ = CatalogStr(MSG_VALUE,MSG_VALUE_STR);
		*array = CatalogStr(MSG_FREQUENCY,MSG_FREQUENCY_STR);
	}
	return(0);
}

HOOK2( LONG, dispfunc_users, char **, array, a2, User *, data, a1 )
	if(data) {
		*array++ = data->getName();
		*array++ = data->dataLocation;
		if(data->requiresPassword())
			*array++ = CatalogStr(MSG_Y,MSG_Y_STR);
		else
			*array++ = CatalogStr(MSG_N,MSG_N_STR);
		if(data->isSupervisor())
			*array = CatalogStr(MSG_Y,MSG_Y_STR);
		else
			*array = CatalogStr(MSG_N,MSG_N_STR);
	}
	else {
		*array++ = CatalogStr(MSG_NAME,MSG_NAME_STR);
		*array++ = CatalogStr(MSG_NEWS_DIRECTORY,MSG_NEWS_DIRECTORY_STR);
		*array++ = CatalogStr(MSG_PASSWORD_PROTECTED,MSG_PASSWORD_PROTECTED_STR);
		*array = CatalogStr(MSG_SUPERVISOR,MSG_SUPERVISOR_STR);
	}
	return(0);
}

HOOK2( LONG, dispfunc_mime, char **, array, a2, MIMEType *, data, a1 )
	static char buf1[16];
	if(data) {
		sprintf(buf1,"%d",data->size);
		*array++ = data->file;
		*array++ = data->type;
		*array = buf1;
	}
	else {
		*array++ = CatalogStr(MSG_FILENAME,MSG_FILENAME_STR);
		*array++ = CatalogStr(MSG_MIME_TYPE,MSG_MIME_TYPE_STR);
		*array = CatalogStr(MSG_SIZE,MSG_SIZE_STR);
	}
	return(0);
}

HOOK2( LONG, dispfunc_mimeprefs, char **, array, a2, MIMEPrefs *, data, a1 )
	if(data) {
		*array++ = data->type;
		*array = data->viewer;
	}
	else {
		*array++ = CatalogStr(MSG_MIME_TYPE,MSG_MIME_TYPE_STR);
		*array = CatalogStr(MSG_VIEWER,MSG_VIEWER_STR);
	}
	return(0);
}

HOOK2( LONG, dispfunc_killfile, char **, array, a2, KillFile *, data, a1 )
	static char buf1[128],buf2[128],buf3[64];
	static DateTime dt;
	if(data) {
		dt.dat_Stamp=data->ds;
		dt.dat_Format=FORMAT_DOS;
		dt.dat_Flags=NULL;
		dt.dat_StrDay=NULL;
		dt.dat_StrDate=buf1;
		dt.dat_StrTime=NULL;
		DateToStr(&dt);
		dt.dat_Stamp=data->lastused;
		dt.dat_StrDate=buf2;
		DateToStr(&dt);
		if(data->expiretype==0)
			strcpy(buf3,CatalogStr(MSG_INDEFINATELY,MSG_INDEFINATELY_STR));
		else if(data->expiretype==1)
			sprintf(buf3,CatalogStr(MSG_DAYS_SINCE_CREATION,MSG_DAYS_SINCE_CREATION_STR),data->expire);
		else if(data->expiretype==2)
			sprintf(buf3,CatalogStr(MSG_DAYS_SINCE_LAST_USED,MSG_DAYS_SINCE_LAST_USED_STR),data->expire);

		*array++ = data->header;
		*array++ = data->text;
		*array++ = data->ngroups;
		/*if(data->type == 0)
			*array++ = CatalogStr(MSG_KILL,MSG_KILL_STR);
		else if(data->type == 1)
			*array++ = "Keep";
		else if(data->type == 2)
			*array++ = CatalogStr(MENU_IMPORTANT,MENU_IMPORTANT_STR);
		else
			*array++ = CatalogStr(MSG_UNKOWN_2,MSG_UNKOWN_2_STR);*/

		if(data->match == 0)
			*array++ = CatalogStr(MSG_CONTAINS,MSG_CONTAINS_STR);
		else if(data->match == 1)
			*array++ = CatalogStr(MSG_DOES_NOT_CONTAIN_2,MSG_DOES_NOT_CONTAIN_2_STR);
		else
			*array++ = CatalogStr(MSG_UNKOWN_2,MSG_UNKOWN_2_STR);

		if(data->action == 0)
			*array++ = CatalogStr(MSG_KILL,MSG_KILL_STR);
		else if(data->action == 1)
			*array++ = CatalogStr(MSG_MARK_AS_IMPORTANT,MSG_MARK_AS_IMPORTANT_STR);
		else
			*array++ = CatalogStr(MSG_UNKOWN_2,MSG_UNKOWN_2_STR);

		*array++ = data->carryon ? CatalogStr(MSG_N,MSG_N_STR) : CatalogStr(MSG_Y,MSG_Y_STR);

		*array++ = buf1;
		*array++ = buf2;
		*array = buf3;
	}
	else {
		*array++ = CatalogStr(MSG_HEADER_BOLD,MSG_HEADER_BOLD_STR);
		*array++ = CatalogStr(MSG_TEXT,MSG_TEXT_STR);
		*array++ = CatalogStr(MSG_GROUPS,MSG_GROUPS_STR);
		*array++ = CatalogStr(MSG_MATCH,MSG_MATCH_STR);
		*array++ = CatalogStr(MSG_ACTION,MSG_ACTION_STR);
		*array++ = CatalogStr(MSG_SKIP_REST,MSG_SKIP_REST_STR);
		*array++ = CatalogStr(MSG_CREATION,MSG_CREATION_STR);
		*array++ = CatalogStr(MSG_LAST_USED,MSG_LAST_USED_STR);
		*array = CatalogStr(MSG_TIME_LIMIT,MSG_TIME_LIMIT_STR);
	}
	return(0);
}

HOOK2( LONG, dispfunc_servers, char **, array, a2, Server *, data, a1 )
	static char buf1[16],buf2[4],buf3[4],buf4[4];
	if(data) {
		sprintf(buf1,"%d",data->port);

		if(data->nntp_auth)
			strcpy(buf2,CatalogStr(MSG_Y,MSG_Y_STR));
		else
			strcpy(buf2,CatalogStr(MSG_N,MSG_N_STR));

		if(data->def)
			strcpy(buf3,CatalogStr(MSG_Y,MSG_Y_STR));
		else
			strcpy(buf3,CatalogStr(MSG_N,MSG_N_STR));

		if(data->post)
			strcpy(buf4,CatalogStr(MSG_Y,MSG_Y_STR));
		else
			strcpy(buf4,CatalogStr(MSG_N,MSG_N_STR));

		*array++ = data->nntp;
		*array++ = buf1;
		*array++ = data->user;
		//*array++ = data->password;
		*array++ = buf2;
		*array++ = buf3;
		*array = buf4;
	}
	else {
		*array++ = CatalogStr(MSG_SERVER_BOLD,MSG_SERVER_BOLD_STR);
		*array++ = CatalogStr(MSG_PORT,MSG_PORT_STR);
		*array++ = CatalogStr(MSG_USERNAME,MSG_USERNAME_STR);
		//*array++ = "\33bPassword";
		*array++ = CatalogStr(MSG_AUTHENCICATION,MSG_AUTHENCICATION_STR);
		*array++ = CatalogStr(MSG_DEFAULT,MSG_DEFAULT_STR);
		*array = CatalogStr(MSG_POSTING,MSG_POSTING_STR);
	}
	return(0);
}

HOOK2( LONG, cmpfunc_groupdata,  GroupData*, gdata2, a2, GroupData *, gdata1, a1)
	STRPTR n1,n2;
	if(gdata1->ID<0 && gdata2->ID>=0)
		return -1;
	if(gdata2->ID<0 && gdata1->ID>=0)
		return 1;

	if(gdata1->ID==-3) {
		if(gdata2->ID<0)
			return 1;
		else
			return -1;
	}
	if(gdata2->ID==-3) {
		if(gdata1->ID<0)
			return -1;
		else
			return 1;
	}

	if(*gdata1->desc!=0)
		n1=gdata1->desc;
	else
		n1=gdata1->name;
	if(*gdata2->desc!=0)
		n2=gdata2->desc;
	else
		n2=gdata2->name;
	return(stricmp(n1,n2));
}

LONG compare_messagelistdata(MessageListData *mdata1,MessageListData *mdata2,int sorttype,int sorttypedir) {
	// sorttype: 0=subject, 1=date, 2=from, -1=newsgroups, 3=size, 4=lines
	static char buffer1[256]="";
	static char buffer2[256]="";
	char *b1 = NULL,*b2 = NULL;
	LONG result = 0;
	//printf("%d %d\n",mdata1->ID,mdata2->ID);
	switch(sorttype) {
	case 0:
		//subject
		b1 = mdata1->subject;
		b2 = mdata2->subject;
		while(*b1!=0 && (STRNICMP(b1,"RE: ",4)==0 || STRNICMP(b1,"SV: ",4)==0))
			b1 += 4;
		while(*b2!=0 && (STRNICMP(b2,"RE: ",4)==0 || STRNICMP(b2,"SV: ",4)==0))
			b2 += 4;
		result=stricmp(b1,b2);
		break;
	case 1:
		//date
		result=-CompareDates(&mdata1->ds,&mdata2->ds);
		break;
	case 2:
		//from
		get_email(buffer1,mdata1->from,GETEMAIL_NAMEEMAIL);
		get_email(buffer2,mdata2->from,GETEMAIL_NAMEEMAIL);
		if(*buffer1=='\"')
			b1=buffer1+1;
		else
			b1=buffer1;
		if(*buffer2=='\"')
			b2=buffer2+1;
		else
			b2=buffer2;
		result=stricmp(b1,b2);
		break;
	case -1:
		//newsgroups
		result=stricmp(mdata1->newsgroups,mdata2->newsgroups);
		break;
	case 3:
		//size
		if(mdata1->size < mdata2->size)
			result=-1;
		else if(mdata1->size > mdata2->size)
			result=1;
		else
			result=0;
		break;
	case 4:
		//lines
		if(mdata1->flags[8] < mdata2->flags[8])
			result=-1;
		else if(mdata1->flags[8] > mdata2->flags[8])
			result=1;
		else
			result=0;
		break;
	}
	if(result!=0 || sorttype==1)
		return result * sorttypedir;
	//date
	result=-CompareDates(&mdata1->ds,&mdata2->ds);
	return result * sorttypedir;
}

HOOK2( LONG, cmpfunc_messagelistdata, MessageListData *, mdata2, a2, MessageListData *, mdata1, a1)
	return compare_messagelistdata(mdata1,mdata2,account.sorttype,account.sorttypedir);
}

HOOK3( LONG, cmpfunc_messagelistdatatree, CPPHook *, hook, a0, Object *, obj, a2, MUIP_NListtree_CompareMessage *, msg, a1 )
	//return cmpfunc_messagelistdata((MessageListData *)msg->TreeNode1->tn_User,(MessageListData *)msg->TreeNode2->tn_User);
	return compare_messagelistdata((MessageListData *)msg->TreeNode1->tn_User,(MessageListData *)msg->TreeNode2->tn_User,account.sorttype,account.sorttypedir);
}

HOOK2( LONG, cmpfunc_search, MessageListData *, mdata2, a2, MessageListData *, mdata1, a1)
	return compare_messagelistdata(mdata1,mdata2,sorttype_search,sorttypedir_search);
}

HOOK2( LONG, cmpfunc_joinmsgs, MessageListData *, mdata2, a2, MessageListData *, mdata1, a1)
	return compare_messagelistdata(mdata1,mdata2,sorttype_search,sorttypedir_search);
}

HOOK2( LONG, cmpfunc_stats, StatsList *, data2, a2, StatsList *, data1, a1)
	// sorttype_stats: 0=value, 1=frequency
	static char buffer1[256]="";
	static char buffer2[256]="";
	char *b1 = NULL,*b2 = NULL;
	LONG result = 0;
	if(sorttype_stats==0) {
		//value
		if(stats_what==0) {
			// from
			get_email(buffer1,data1->value,GETEMAIL_NAMEEMAIL);
			get_email(buffer2,data2->value,GETEMAIL_NAMEEMAIL);
			if(*buffer1=='\"')
				b1=buffer1+1;
			else
				b1=buffer1;
			if(*buffer2=='\"')
				b2=buffer2+1;
			else
				b2=buffer2;
			result=stricmp(b1,b2);
		}
		else if(stats_what==1) {
			// subject
			/*b1 = data1->value;
			b2 = data2->value;
			while(*b1!=0 && (STRNICMP(b1,"RE: ",4)==0 || STRNICMP(b1,"SV: ",4)==0))
				b1 += 4;
			while(*b2!=0 && (STRNICMP(b2,"RE: ",4)==0 || STRNICMP(b2,"SV: ",4)==0))
				b2 += 4;
			result=stricmp(b1,b2);*/
			result=stricmp(data1->value,data2->value);
		}
		else if(stats_what==2) {
			// xnewsreader
			result=stricmp(data1->value,data2->value);
		}
	}
	else if(sorttype_stats==1) {
		if(data1->freq > data2->freq)
			result = 1;
		else if (data1->freq < data2->freq)
			result = -1;
	}
	return result * sorttypedir_stats;
}

HOOK2( LONG, cmpfunc_users,  User *, data2, a2, User *, data1, a1)
	return stricmp(data1->getName(),data2->getName());
}

void mdata_changed() {
	setEnabled();
}

HOOK2( LONG, do_arexx, Object*, app, a2, RexxMsg *, msg, a1)
	GroupData * gdata = NULL;
	MessageListData * mdata = NULL;
	char *rexx=NULL;
	char word1[1024] = "";
	char word2[1024] = "";
	char word3[1024] = "";
	char buffer[1024]="";
	LONG rc=0;
	ULONG entries=0;
	for(int k=0;k<16;k++) {
		rexx = msg->rm_Args[k];
		if(rexx!=0) {
			word(word1,rexx,1);
			word(word2,rexx,2);
			word(word3,rexx,3);
			if(stricmp(word1,"BUSY")==0)
				set(app,MUIA_Application_Sleep,TRUE);
			else if(stricmp(word1,"FETCHNEWS")==0) {
				if(delete_dis)
					rc=1;
				else {
					BOOL quiet = ( stricmp(word2,"quiet") == 0 );
					getnews(-1,quiet);
				}
			}
			else if(stricmp(word1,"FETCHNEWSBYID")==0) {
				if(*word2 != 0) {
					if(delete_dis)
						rc=1;
					else {
						int id=0;
						id = atoi(word2);
						int pos=get_gdataPos(id);
						if(pos==-1)
							rc=1;
						else {
							if(id>=0) {
								BOOL quiet = ( stricmp(word3,"quiet") == 0 );
								getnews(id,quiet);
							}
							else
								rc=1;
						}
					}
				}
				else
					rc=1;
			}
			else if(stricmp(word1,"FETCHNEWSBYNAME")==0) {
				if(*word2 != 0) {
					if(delete_dis)
						rc=1;
					else {
						int pos=get_gdataPos(word2);
						if(pos==-1)
							rc=1;
						else {
							getGdata(pos,&gdata);
							if(gdata) {
								if(gdata->ID>=0) {
									BOOL quiet = ( stricmp(word3,"quiet") == 0 );
									getnews(gdata->ID,quiet);
								}
								else
									rc=1;
							}
							else
								rc=1;
						}
					}
				}
				else
					rc=1;
			}
			else if(stricmp(word1,"FOLDERINFO")==0) {
				getGdataDisplayed(&gdata);
				sprintf(buffer,"%d %s %s %d %d %d %d",gdata->ID,gdata->name,gdata->desc,gdata->nummess,gdata->nextmID,gdata->s,gdata->max_dl);
				set(app,MUIA_Application_RexxString,buffer);
			}
			/*else if(stricmp(word1,"GETSELECTEDMSGS")==0) {
				if(*word2 == '\0') {
					rc = 5;
				}
				else {
					get(NLIST_messagelistdata,MUIA_NList_Entries,&entries);
					sprintf(buffer,"%s.length",word2);
					sprintf(value,"%d",entries);
					printf("*%s*\n",buffer);
					printf("    *%s*\n",entries);
					//SetRexxVar(o_msg, buffer, value, strlen(value)+1);
					// SetRexxVar won't link in StormC !?!
				}
			}*/
			/*else if(stricmp(word1,"GETNUMVISIBLEENTRIES")==0) {
				rc = getMdataVisibleEntries();
			}*/
			else if(stricmp(word1,"GETNUMMESSAGES")==0) {
				get(NLIST_messagelistdata,MUIA_NList_Entries,&entries);
				sprintf(buffer,"%d",entries);
				set(app,MUIA_Application_RexxString,buffer);
			}
			else if(stricmp(word1,"HIDE")==0)
				set(app,MUIA_Application_Iconified,TRUE);
			else if(stricmp(word1,"IMPORTFOLDER")==0) {
				if(*word2 != 0) {
					StatusWindow *statusWindow = new StatusWindow(app,CatalogStr(MSG_IMPORTING_FOLDER,MSG_IMPORTING_FOLDER_STR));
					import_folder(word2,statusWindow);
					delete statusWindow;
				}
			}
			else if(stricmp(word1,"IMPORTMESSAGE")==0)
			{
				if(*word2 != 0)
					import_file(word2);
			}
			else if(stricmp(word1,"ISMSGSELECTED")==0)
			{
				if( *word2 == '\0') {
					rc =  5;
				}
				else {
					get(NLIST_messagelistdata,MUIA_NList_Entries,&entries);
					int pos = atoi(word2);
					if(pos>=0 && pos<entries)
					{
						BOOL res = isMdataSelected(pos, FALSE);
						//printf("<<< %d : %d\n",pos,res);
						rc = res ? 1 : 0;
					}
					else
						rc=5;
				}
			}
			else if(stricmp(word1,"ISONLINE")==0) {
				if(delete_dis)
					rc=1;
				else
					rc=0;
			}
			else if(stricmp(word1,"MESSAGECOPY")==0) {
				if(*word2 != 0)
					move_multi(1,word2);
			}
			else if(stricmp(word1,"MESSAGEDELETE")==0) {
				BOOL confirm=TRUE;
				if(*word2 != 0) {
					if(stricmp(word2,"FORCE")==0)
						confirm=FALSE;
				}
				delete_multi(confirm);
			}
			else if(stricmp(word1,"MESSAGEEDIT")==0) {
				//DoMethod(app,MUIM_Application_ReturnID,MEN_EDIT);
				DoMethod(app, MUIM_CallHook, &hook_standard, mainMenusFunc, MEN_EDIT);
			}
			else if(stricmp(word1,"MESSAGEEXPORT")==0) {
				//DoMethod(app,MUIM_Application_ReturnID,MEN_EXP);
				DoMethod(app, MUIM_CallHook, &hook_standard, mainMenusFunc, MEN_EXP);
			}
			else if(stricmp(word1,"MESSAGEFETCH")==0) {
				getGdataDisplayed(&gdata);
				getMdataActive(&mdata);
				if( gdata == NULL || mdata == NULL )
					rc = 5;
				else {
					if(mdata->flags[12]==TRUE) {
						BOOL available = FALSE;
						BOOL quiet = ( stricmp(word2,"quiet") == 0 );
						if( getBody(&available,gdata,mdata,quiet) ) {
							rc = 0;
							redrawMdataActive();
						}
						else
							rc = 2;
					}
					else
						rc = 1; // don't need to download
				}
			}
			else if(stricmp(word1,"MESSAGEFOLLOWUP")==0) {
				//DoMethod(app,MUIM_Application_ReturnID,MEN_FOLLOWUP);
				DoMethod(app, MUIM_CallHook, &hook_standard, mainMenusFunc, MEN_FOLLOWUP);
			}
			else if(stricmp(word1,"MESSAGEFUANDR")==0) {
				//DoMethod(app,MUIM_Application_ReturnID,MEN_BOTH);
				DoMethod(app, MUIM_CallHook, &hook_standard, mainMenusFunc, MEN_BOTH);
			}
			else if(stricmp(word1,"MESSAGEGETHEADER")==0)
			{
				if(*word2 != 0) {
					getGdataDisplayed(&gdata);
					getMdataActive(&mdata);
					if( gdata == NULL || mdata == NULL )
						rc = 5;
					else
					{
						NewMessage tempNM;
						strncpy(tempNM.getThisHeader,word2,NEWMESS_shortshort);
						tempNM.getThisHeader[NEWMESS_shortshort] = '\0';
						get_refs(&tempNM,gdata,mdata,GETREFS_NONE);
						strcpy(buffer,tempNM.dummyHeader);
						set(app,MUIA_Application_RexxString,buffer);
					}
				}
				else
					rc=5;
			}
			else if(stricmp(word1,"MESSAGEGETPATH")==0)
			{
				getGdataDisplayed(&gdata);
				getMdataActive(&mdata);
				if( gdata == NULL || mdata == NULL )
					rc = 5;
				else
				{
					getFilePath(buffer,gdata->ID,mdata->ID);
					set(app,MUIA_Application_RexxString,buffer);
				}
			}
			else if(stricmp(word1,"MESSAGEINFO")==0)
			{
				getGdataDisplayed(&gdata);
				getMdataActive(&mdata);
				if( gdata == NULL || mdata == NULL )
					rc = 5;
				else
				{
					NewMessage newmess;
					get_refs(&newmess,gdata->ID,mdata->ID,GETREFS_NONE);
					sprintf(buffer,"%d %s %s %s %s %s %s %s",mdata->ID,newmess.from,newmess.newsgroups,newmess.subject,mdata->c_date,mdata->c_time,newmess.messageID,newmess.type);
					set(app,MUIA_Application_RexxString,buffer);
				}
			}
			else if(stricmp(word1,"MESSAGEKILL")==0)
			{
				int type=0;
				if(*word2 != 0)
				{
					type = atoi(word2); // must be 0,1,2
					if(type<0 || type>2)
						rc=5;
					else
					{
						if(*word3 != 0)
						{
							if(stricmp(word3,"THISGROUP")==0)
								type += 4;
						}
					}
				}
				else
					type=-1;
				if(rc==0)
				{
					getGdataDisplayed(&gdata);
					getMdataActive(&mdata);
					if( gdata == NULL || mdata == NULL )
						rc = 5;
					else
					{
						killmess(gdata,mdata,type);
					}
				}
			}
			else if(stricmp(word1,"MESSAGEMOVE")==0)
			{
				if(*word2 != 0)
					move_multi(0,word2);
			}
			else if(stricmp(word1,"MESSAGEPOST")==0)
			{
				//DoMethod(app,MUIM_Application_ReturnID,MEN_POST);
				DoMethod(app, MUIM_CallHook, &hook_standard, mainMenusFunc, MEN_POST);
			}
			else if(stricmp(word1,"MESSAGEREPLY")==0)
			{
				//DoMethod(app,MUIM_Application_ReturnID,MEN_REPLY);
				DoMethod(app, MUIM_CallHook, &hook_standard, mainMenusFunc, MEN_REPLY);
			}
			else if(stricmp(word1,"MESSAGESUPERSEDE")==0)
			{
				//DoMethod(app,MUIM_Application_ReturnID,MEN_SUPER);
				DoMethod(app, MUIM_CallHook, &hook_standard, mainMenusFunc, MEN_SUPER);
			}
			else if(stricmp(word1,"NOBUSY")==0)
				set(app,MUIA_Application_Sleep,FALSE);
			else if(stricmp(word1,"QUIT")==0)
			{
				//DoMethod(app,MUIM_Application_ReturnID,MUIV_Application_ReturnID_Quit);
				DoMethod(app, MUIM_CallHook, &hook_standard, mainMenusFunc, MEN_QUIT);
			}
			else if(stricmp(word1,"SELECTMESSAGE")==0)
			{
				if(*word2 != 0)
				{
					int v=0; // 1-=delselect, 1=select, 0=toggle
					v = atoi(word2);
					int which = MUIV_NList_Select_Active;
					if(*word3 != 0)
					{
						if(stricmp(word3,"ALL")==0)
						{
							if(account.mdata_view==0)
								which=MUIV_NList_Select_All;
							else
								which=MUIV_NList_Select_All;
						}
					}
					if(account.mdata_view==0)
					{
						if(v==-1)
							DoMethod(NLIST_messagelistdata,MUIM_NList_Select,which,MUIV_NList_Select_Off,NULL);
						else if(v==0)
							DoMethod(NLIST_messagelistdata,MUIM_NList_Select,which,MUIV_NList_Select_Toggle,NULL);
						else if(v==1)
							DoMethod(NLIST_messagelistdata,MUIM_NList_Select,which,MUIV_NList_Select_On,NULL);
					}
					else
					{
						if(v==-1)
							DoMethod(LISTTREE_messagelistdata,MUIM_NList_Select,which,MUIV_NList_Select_Off,NULL);
						else if(v==0)
							DoMethod(LISTTREE_messagelistdata,MUIM_NList_Select,which,MUIV_NList_Select_Toggle,NULL);
						else if(v==1)
							DoMethod(LISTTREE_messagelistdata,MUIM_NList_Select,which,MUIV_NList_Select_On,NULL);
					}
				}
				else
					rc=5;
			}
			else if(stricmp(word1,"SENDNEWS")==0)
			{
				if(delete_dis)
					rc=1;
				else
					postnews(0);
			}
			else if(stricmp(word1,"SETFOLDER")==0)
			{ // by pos
				if(*word2 != 0)
				{
					int pos=0;
					pos = atoi(word2);
					getGdataEntries((int*)&entries);
					if(pos>=0 && pos<entries)
						setGdataActive(pos);
					else
						rc=5;
				}
			}
			else if(stricmp(word1,"SETFOLDERBYID")==0)
			{ // by ID
				if(*word2 != 0)
				{
					int id=0;
					id = atoi(word2);
					set_and_read(id);
				}
			}
			else if(stricmp(word1,"SETFOLDERBYNAME")==0)
			{ // by name
				if(*word2 != 0)
					set_and_read(word2);
			}
			else if(stricmp(word1,"SETMESSAGE")==0)
			{ // by pos
				if(*word2 != 0)
				{
					int pos = atoi(word2);
					get(NLIST_messagelistdata,MUIA_NList_Entries,&entries);
					if(pos>=0 && pos<entries)
						setMdataActive(pos);
					else
						rc=5;
				}
				else
					rc=5;
			}
			else if(stricmp(word1,"SHOW")==0)
				set(app,MUIA_Application_Iconified,FALSE);
		}
	}
	return rc;
}

HOOK3( VOID, openfunc_messagelistdata, CPPHook *, hook, a0, Object*, obj, a2, struct MUIP_NListtree_OpenMessage *, openmsg, a1 )
	MUI_NListtree_TreeNode *treenode = openmsg->TreeNode;
	MUI_NListtree_TreeNode *child = (MUI_NListtree_TreeNode *)DoMethod(obj,MUIM_NListtree_GetEntry,treenode,MUIV_NListtree_GetEntry_Position_Head,0);

	//set(obj, MUIA_NListtree_OpenHook, NULL);
	//printf("!!!%d\n",hook);

	while(child != 0) {
		//printf("child..\n");
		DoMethod(obj, MUIM_NListtree_Open, treenode, child, 0);
		//DoMethod(obj, MUIM_NListtree_Open, child, MUIV_NListtree_Open_TreeNode_All, 0);
		child = (MUI_NListtree_TreeNode *)DoMethod(obj,MUIM_NListtree_GetEntry,child,MUIV_NListtree_GetEntry_Position_Next,0);
	}

	//set(obj, MUIA_NListtree_OpenHook, hook);
	//printf("..\n");
}
const CPPHook OpenHook_messagelistdata={ {NULL,NULL},(CPPHOOKFUNC)&openfunc_messagelistdata,NULL,NULL};

/*VOID HOOK2( intuimsg_func, IntuiMessage *, imsg, a1, FileRequester *, req, a2 )
	if(imsg->Class == IDCMP_REFRESHWINDOW)
		DoMethod(app,MUIM_Application_CheckRefresh);
}*/

const CPPHook StrObjHook = { { NULL,NULL },(CPPHOOKFUNC)&StrObjFunc,NULL,NULL };
const CPPHook StrObj2Hook = { { NULL,NULL },(CPPHOOKFUNC)&StrObj2Func,NULL,NULL };
const CPPHook ObjStrAccHook = { { NULL,NULL },(CPPHOOKFUNC)&ObjStrAccFunc,NULL,NULL };
const CPPHook ObjStrAcc2Hook = { { NULL,NULL },(CPPHOOKFUNC)&ObjStrAcc2Func,NULL,NULL };
const CPPHook ObjStrAcc3Hook = { { NULL,NULL },(CPPHOOKFUNC)&ObjStrAcc3Func,NULL,NULL };
const CPPHook ObjStrAcc4Hook = { { NULL,NULL },(CPPHOOKFUNC)&ObjStrAcc4Func,NULL,NULL };
const CPPHook ObjStrNewuserHook = { { NULL,NULL },(CPPHOOKFUNC)&ObjStrNewuserFunc,NULL,NULL };
const CPPHook WindowHook = { { NULL,NULL },(CPPHOOKFUNC)&WindowFunc,NULL,NULL };
const CPPHook DestructHook_groupdata={ {NULL,NULL},(CPPHOOKFUNC)&desfunc_groupdata,NULL,NULL};
const CPPHook DestructHook_groupdatatree={ {NULL,NULL},(CPPHOOKFUNC)&desfunc_groupdatatree,NULL,NULL};
const CPPHook DestructHook_messagelistdata={ {NULL,NULL},(CPPHOOKFUNC)&desfunc_messagelistdata,NULL,NULL};
const CPPHook DestructHook_search={ {NULL,NULL},(CPPHOOKFUNC)&desfunc_search,NULL,NULL};
const CPPHook DestructHook_joinmsgs={ {NULL,NULL},(CPPHOOKFUNC)&desfunc_joinmsgs,NULL,NULL};
const CPPHook DestructHook_stats={ {NULL,NULL},(CPPHOOKFUNC)&desfunc_stats,NULL,NULL};
const CPPHook DestructHook_users={ {NULL,NULL},(CPPHOOKFUNC)&desfunc_users,NULL,NULL};
const CPPHook DestructHook_groupman={ {NULL,NULL},(CPPHOOKFUNC)&desfunc_groupman,NULL,NULL};
const CPPHook DestructHook_servers={ {NULL,NULL},(CPPHOOKFUNC)&desfunc_servers,NULL,NULL};
const CPPHook DestructHook_killfile={ {NULL,NULL},(CPPHOOKFUNC)&desfunc_killfile,NULL,NULL};
const CPPHook DestructHook_mime={ {NULL,NULL},(CPPHOOKFUNC)&desfunc_mime,NULL,NULL};
const CPPHook DestructHook_mimeprefs={ {NULL,NULL},(CPPHOOKFUNC)&desfunc_mimeprefs,NULL,NULL};
const CPPHook DisplayHook_simpleglist={ {NULL,NULL},(CPPHOOKFUNC)&dispfunc_simpleglist,NULL,NULL};
const CPPHook DisplayHook_groupdata={ {NULL,NULL},(CPPHOOKFUNC)&dispfunc_groupdata,NULL,NULL};
const CPPHook DisplayHook_groupdatatree={ {NULL,NULL},(CPPHOOKFUNC)&dispfunc_groupdatatree,NULL,NULL};
const CPPHook ConstructHook_groupdatatree={ {NULL,NULL},(CPPHOOKFUNC)&confunc_groupdatatree,NULL,NULL};
const CPPHook DisplayHook_messagelistdata={ {NULL,NULL},(CPPHOOKFUNC)&dispfunc_messagelistdata,NULL,NULL};
const CPPHook DisplayHook_messagelistdatatree={ {NULL,NULL},(CPPHOOKFUNC)&dispfunc_messagelistdatatree,NULL,NULL};
const CPPHook DisplayHook_search={ {NULL,NULL},(CPPHOOKFUNC)&dispfunc_search,NULL,NULL};
const CPPHook DisplayHook_joinmsgs={ {NULL,NULL},(CPPHOOKFUNC)&dispfunc_joinmsgs,NULL,NULL};
const CPPHook DisplayHook_stats={ {NULL,NULL},(CPPHOOKFUNC)&dispfunc_stats,NULL,NULL};
const CPPHook DisplayHook_users={ {NULL,NULL},(CPPHOOKFUNC)&dispfunc_users,NULL,NULL};
const CPPHook DisplayHook_mime={ {NULL,NULL},(CPPHOOKFUNC)&dispfunc_mime,NULL,NULL};
const CPPHook DisplayHook_mimeprefs={ {NULL,NULL},(CPPHOOKFUNC)&dispfunc_mimeprefs,NULL,NULL};
const CPPHook DisplayHook_servers={ {NULL,NULL},(CPPHOOKFUNC)&dispfunc_servers,NULL,NULL};
const CPPHook DisplayHook_killfile={ {NULL,NULL},(CPPHOOKFUNC)&dispfunc_killfile,NULL,NULL};
const CPPHook CompareHook_groupdata={ {NULL,NULL},(CPPHOOKFUNC)&cmpfunc_groupdata,NULL,NULL};
const CPPHook CompareHook_messagelistdata={ {NULL,NULL},(CPPHOOKFUNC)&cmpfunc_messagelistdata,NULL,NULL};
const CPPHook CompareHook_messagelistdatatree={ {NULL,NULL},(CPPHOOKFUNC)&cmpfunc_messagelistdatatree,NULL,NULL};
const CPPHook CompareHook_search={ {NULL,NULL},(CPPHOOKFUNC)&cmpfunc_search,NULL,NULL};
const CPPHook CompareHook_joinmsgs={ {NULL,NULL},(CPPHOOKFUNC)&cmpfunc_joinmsgs,NULL,NULL};
const CPPHook CompareHook_stats={ {NULL,NULL},(CPPHOOKFUNC)&cmpfunc_stats,NULL,NULL};
const CPPHook CompareHook_users={ {NULL,NULL},(CPPHOOKFUNC)&cmpfunc_users,NULL,NULL};
//const CPPHook MdataChangedHook={ {NULL,NULL},(CPPHOOKFUNC)&mdata_changed,NULL,NULL};
const CPPHook ARexxHook={ {NULL,NULL},(CPPHOOKFUNC)&do_arexx,NULL,NULL};

/*const CPPHook IntuiMsgHook = { { NULL,NULL },(CPPHOOKFUNC)&intuimsg_func,NULL,NULL };*/

int realmain_2()
{
	if (!init_threads())
	{
		printf("FAILED TO INITIALISE THREADS!!!\n");
		return 0;
	}

	if (!(IntuitionBase = OpenLibrary("intuition.library",52)))
		fail(NULL,"Failed to open Intuition.library.");
	if(!(IIntuition = (struct IntuitionIFace *)GetInterface(IntuitionBase, "main", 1, NULL)))
		fail(NULL,"Failed to open interface to intuition.library.");

	if (!(CodesetsBase = OpenLibrary("codesets.library",6)))
		fail(NULL,"Failed to open codesets.library.");
	if(!(ICodesets=(struct CodesetsIFace *)GetInterface(CodesetsBase,"main", 1, NULL)))
		fail(NULL,"Failed to open interface to codesets.library.");

	if (!(AslBase = OpenLibrary("asl.library",52)))
		fail(NULL,"Failed to open ASL.library.");
	if(!(IAsl = (struct AslIFace *)GetInterface(AslBase, "main", 1, NULL)))
		fail(NULL,"Failed to open interface to asl.library.");

	if (!(LocaleBase = OpenLibrary("locale.library",52)))
		fail(NULL,"Failed to open locale.library.");
	if(!(ILocale = (struct LocaleIFace *)GetInterface(LocaleBase, "main", 1, NULL)))
		fail(NULL,"Failed to open interface to locale.library.");

	LocalCharset=(char*)malloc(SIZE_CTYPE+1);
	memset(LocalCharset,0,SIZE_CTYPE+1);

	codesetsList = CodesetsListCreateA(NULL);
	sysCodeset = CodesetsFindA(NULL, NULL); // get the system's default codeset
    strlcpy(LocalCharset, sysCodeset->name, SIZE_CTYPE+1);

	nc_Catalog=OpenCatalog(0,"newscoaster.catalog",OC_Version,2,TAG_DONE);
	InitMenu(MenuData1,MENU_PROJECT);
	InitToolbar(TLBbuttons,TBAR_READ);
	InitArray(Pages_acc,MSG_ACCOUNTS);
	InitArray(CYA_acc_dateformat,MSG_DATEFORMAT_1);
	InitArray(CYA_acc_sigs,MSG_SIGNATURE_1);
	InitArray(CYA_acc_readheader,MSG_NONE_2);
	InitArray(CYA_nng_maxage,MSG_DONT_DISCARD_MESSAGES);
	InitArray(CYA_nng_offline,MSG_OFFLINE_READING);
	InitArray(CYA_nng_newsservers,MSG_NEWSSERVER_1);
	InitArray(CYA_nng_sigs,MSG_NONE);
	InitArray(CYA_acc_timezone,MSG_GMT_M12);
	InitArray(CYA_acc_xno,MSG_NEVER);
	InitArray(CYA_acc_expirekill,MSG_NEVER_EXPIRE);
	InitArray(CYA_acc_matchkill,MSG_CONTAINS);
	InitArray(CYA_acc_actionkill,MSG_KILL);
	InitArray(CYA_nng_maxdl,MSG_UNLIMITED_MESSAGES_PER_DOWNLOAD);
	InitArray(CYA_nng_maxl,MSG_DOWNLOAD_MESSAGES_OF_ANY_LENGTH);
	InitArray(CYA_search_where,MSG_FROM);
	InitArray(CYA_stats_what,MSG_BY_PERSON);

	initNetstuff();

	init();

	char *TEXT_About = CatalogStr(MSG_SEE_GUIDE_FOR_MORE_INFO,MSG_SEE_GUIDE_FOR_MORE_INFO_STR);
	sprintf(newscoaster_name,"NewsCoaster v%s",version);
	sprintf(base_scrtitle,CatalogStr(MSG_AUTHORS,MSG_AUTHORS_STR),newscoaster_name, copyright_string);
	strcpy(scrtitle,base_scrtitle);

	if(!(editor_mcc = MUI_CreateCustomClass(NULL, "TextEditor.mcc", NULL, 0, (void *)&TextEditor_Dispatcher)))
	{
		fail(app,CatalogStr(MSG_FAILED_TO_CREATE_EDITOR_MCC,MSG_FAILED_TO_CREATE_EDITOR_MCC_STR));
		return 0;
	}
	char abouttext[256]="";
	sprintf(abouttext,CatalogStr(MSG_ABOUTTEXT,MSG_ABOUTTEXT_STR),version, copyright_string);

	CYA_ngadv_server = new char *[2];
	CYA_ngadv_server[0] = new char[64];
	CYA_ngadv_server[1] = NULL;
	strcpy(CYA_ngadv_server[0],"");
	server_list = new int[1];
	server_list[0] = 0;

	SLD_acc_SIG = ScrollbarObject, End;

	APTR   unread_mail_icon=(Object*)NBitmapFile("TBImages:list_mailunread");
	APTR      new_mail_icon=(Object*)NBitmapFile("TBImages:list_mailnew");
	APTR     read_mail_icon=(Object*)NBitmapFile("TBImages:list_mailold");
//30-04-2008
	APTR     hold_mail_icon=(Object*)NBitmapFile("TBImages:list_hold");
	APTR       flagred_icon=(Object*)NBitmapFile("TBImages:list_flagred");
	APTR     flaggreen_icon=(Object*)NBitmapFile("TBImages:list_flaggreen");
	APTR    flagyellow_icon=(Object*)NBitmapFile("TBImages:list_flagyellow");
	APTR  replied_mail_icon=(Object*)NBitmapFile("TBImages:list_mailreply");
	APTR onserver_mail_icon=(Object*)NBitmapFile("TBImages:list_mailspam");
	APTR      outgoing_icon=(Object*)NBitmapFile("TBImages:list_folderoutgoing");
	APTR          sent_icon=(Object*)NBitmapFile("TBImages:list_foldersent");
	APTR       deleted_icon=(Object*)NBitmapFile("TBImages:list_delete");

//	sprintf(status_buffer_g,CatalogStr(MSG_COPYRIGHT,MSG_COPYRIGHT_STR),"©1999-2003 Mark Harman","Pavel Fedin");
	app = ApplicationObject,
		MUIA_Application_Title      , "NewsCoaster",
		MUIA_Application_Version    , "$VER: NewsCoaster for AmigaOS4 "VERSION_STRING,
//		MUIA_Application_Copyright  , status_buffer_g,
		MUIA_Application_Copyright  , copyright_string,
		MUIA_Application_Author     , "Andrea Palmatè",
		MUIA_Application_Description, CatalogStr(MSG_NEWSREADER,MSG_NEWSREADER_STR),
		MUIA_Application_Base       , "NEWSCOASTER",
		MUIA_Application_SingleTask , TRUE,
		MUIA_Application_HelpFile   , "NewsCoaster:Newscoaster.guide",
		MUIA_Application_RexxHook   , &ARexxHook,

		SubWindow, wnd_main = WindowObject,
			MUIA_Window_Title,			"NewsCoaster",
			MUIA_Window_ID,				MAKE_ID('M','A','I','N'),
			MUIA_Window_ScreenTitle,	scrtitle,
    		MUIA_HorizWeight,        	30,
    		MUIA_VertWeight,       		30,
			MUIA_Window_Menustrip,		menustrip=MUI_MakeObject(MUIO_MenustripNM,MenuData1,0),
			MUIA_HelpNode,					"WIN_MAIN",

			WindowContents, VGroup,
				Child, HGroupV,
					Child, TB_main = TheBarObject,
						MUIA_Group_Horiz,       	TRUE,
						MUIA_TheBar_EnableKeys,		TRUE,
						MUIA_TheBar_Buttons,    	TLBbuttons,
						MUIA_TheBar_PicsDrawer,		"PROGDIR:icons",
						MUIA_TheBar_Strip,			"main.toolbar",
						MUIA_TheBar_SelStrip,		"main.toolbar",
						MUIA_TheBar_DisStrip,		"main_G.toolbar",
						MUIA_TheBar_StripCols,  	14,
						End,
					Child, HSpace(0),
					End,

				Child, HGroup,
					Child, grouplistGroup=VGroup,
						MUIA_Group_Spacing, 0,
						End,
					Child, BalanceObject, End,
					Child, messageGroup=VGroup,
						MUIA_Group_Spacing, 0,
						End,
					End,
				End,
			End,

		SubWindow, wnd_newnewsgroup = WindowObject,
			MUIA_Window_Title,			CatalogStr(MSG_NEW_NEWSGROUP,MSG_NEW_NEWSGROUP_STR),
			MUIA_Window_ID,				MAKE_ID('N','N','G','R'),
			MUIA_Window_ScreenTitle,	scrtitle,
			MUIA_Window_CloseGadget,	FALSE,
			MUIA_HelpNode,					"WIN_NNG",

			WindowContents, VGroup,
				Child, ColGroup(2),
					Child, Label2(CatalogStr(LABEL_NEWSGROUP_NAME,LABEL_NEWSGROUP_NAME_STR)), Child, STR_nng_NAME=BetterStringObject, StringFrame, MUIA_String_Contents, "", MUIA_String_MaxLen, 64, MUIA_String_Reject, ",", MUIA_CycleChain, 1, End,
					Child, Label2(CatalogStr(LABEL_DESCRIPTION,LABEL_DESCRIPTION_STR)), Child, STR_nng_DESC=BetterStringObject, StringFrame, MUIA_String_Contents, "", MUIA_String_MaxLen, 64, MUIA_CycleChain, 1, End,
					Child, Label1(CatalogStr(LABEL_DEFAULT_SIG,LABEL_DEFAULT_SIG_STR)), Child, CY_nng_DEFSIG=Cycle(CYA_nng_sigs),
					End,
				Child, ColGroup(1),
					Child, CY_nng_MAXDL=Cycle(CYA_nng_maxdl),
					Child, STR_nng_MAXDL=BetterStringObject, StringFrame, MUIA_String_Integer, 0, MUIA_String_MaxLen, 6, MUIA_String_Accept, "1234567890", MUIA_CycleChain, 1, End,
					Child, CY_nng_MAXL=Cycle(CYA_nng_maxl),
					Child, STR_nng_MAXL=BetterStringObject, StringFrame, MUIA_String_Integer, 0, MUIA_String_MaxLen, 6, MUIA_String_Accept, "1234567890", MUIA_CycleChain, 1, End,
					Child, CY_nng_MAXAGE=Cycle(CYA_nng_maxage),
					Child, STR_nng_MAXAGE=BetterStringObject, StringFrame, MUIA_String_Integer, 0, MUIA_String_MaxLen, 5, MUIA_String_Accept, "1234567890", MUIA_CycleChain, 1, End,
					Child, CY_nng_OFFLINE=Cycle(CYA_nng_offline),
					End,
				Child, ColGroup(2),
					Child, Label1(CatalogStr(LABEL_SUBSCRIBE,LABEL_SUBSCRIBE_STR)), Child, CM_nng_S = CheckMark(TRUE),
					End,
				Child, ColGroup(2),
					Child, BT_nng_OKAY=SimpleButton(CatalogStr(MSG_OKAY,MSG_OKAY_STR)),
					Child, BT_nng_CANCEL=SimpleButton(CatalogStr(MSG_CANCEL_2,MSG_CANCEL_2_STR)),
					End,
				End,
			End,

		SubWindow, wnd_editng = WindowObject,
			MUIA_Window_Title,			CatalogStr(MSG_EDIT_NEWSGROUP,MSG_EDIT_NEWSGROUP_STR),
			MUIA_Window_ID,				MAKE_ID('E','N','G','R'),
			MUIA_Window_ScreenTitle,	scrtitle,
			MUIA_Window_CloseGadget,	FALSE,
			MUIA_HelpNode,					"WIN_ENG",

			WindowContents, VGroup,
				Child, ColGroup(2),
					Child, Label2(CatalogStr(LABEL_NEWSGROUP_NAME,LABEL_NEWSGROUP_NAME_STR)), Child, STR_eng_NAME=BetterStringObject, StringFrame, MUIA_String_Contents, "", MUIA_String_MaxLen, 64, MUIA_String_Reject, ",", MUIA_CycleChain, 1, End,
					Child, Label2(CatalogStr(LABEL_DESCRIPTION,LABEL_DESCRIPTION_STR)), Child, STR_eng_DESC=BetterStringObject, StringFrame, MUIA_String_Contents, "", MUIA_String_MaxLen, 64, MUIA_CycleChain, 1, End,
					Child, Label1(CatalogStr(LABEL_DEFAULT_SIG,LABEL_DEFAULT_SIG_STR)), Child, CY_eng_DEFSIG=Cycle(CYA_nng_sigs),
					End,
				Child, ColGroup(1),
					Child, CY_eng_MAXDL=Cycle(CYA_nng_maxdl),
					Child, STR_eng_MAXDL=BetterStringObject, StringFrame, MUIA_String_Integer, 0, MUIA_String_MaxLen, 6, MUIA_String_Accept, "1234567890", MUIA_CycleChain, 1, End,
					Child, CY_eng_MAXL=Cycle(CYA_nng_maxl),
					Child, STR_eng_MAXL=BetterStringObject, StringFrame, MUIA_String_Integer, 0, MUIA_String_MaxLen, 6, MUIA_String_Accept, "1234567890", MUIA_CycleChain, 1, End,
					Child, CY_eng_MAXAGE=Cycle(CYA_nng_maxage),
					Child, STR_eng_MAXAGE=BetterStringObject, StringFrame, MUIA_String_Integer, 0, MUIA_String_MaxLen, 5, MUIA_String_Accept, "1234567890", MUIA_CycleChain, 1, End,
					Child, CY_eng_OFFLINE=Cycle(CYA_nng_offline),
					End,
				Child, GROUP_eng = ColGroup(2),
					Child, Label2(CatalogStr(MSG_LAST_DOWNLOAD_DATE,MSG_LAST_DOWNLOAD_DATE_STR)), Child, TXT_eng_LASTDL=TextObject, GroupFrame, MUIA_Text_Contents, "", MUIA_Text_PreParse, "\33c", MUIA_Text_SetMin, TRUE, End,
					Child, Label2(CatalogStr(MSG_FOLDER_LOCATION,MSG_FOLDER_LOCATION_STR)), Child, TXT_eng_FOLDER=TextObject, GroupFrame, MUIA_Text_Contents, "", MUIA_Text_PreParse, "\33c", MUIA_Text_SetMin, TRUE, End,
					Child, Label1(CatalogStr(LABEL_SUBSCRIBE,LABEL_SUBSCRIBE_STR)), Child, CM_eng_S = CheckMark(TRUE),
					End,
				Child, ColGroup(2),
					Child, BT_eng_OKAY=SimpleButton(CatalogStr(MSG_OKAY,MSG_OKAY_STR)),
					Child, BT_eng_CANCEL=SimpleButton(CatalogStr(MSG_CANCEL_2,MSG_CANCEL_2_STR)),
					End,
				End,
			End,

		SubWindow, wnd_ngadv = WindowObject,
			MUIA_Window_Title,			CatalogStr(MSG_GROUP_ADVANCED_SETTINGS,MSG_GROUP_ADVANCED_SETTINGS_STR),
			MUIA_Window_ID,				MAKE_ID('N','A','D','V'),
			MUIA_Window_ScreenTitle,	scrtitle,
			MUIA_Window_CloseGadget,	FALSE,

			WindowContents, VGroup,
				Child, ColGroup(3),
					Child, Label1(CatalogStr(MSG_APPROVED_HEADER,MSG_APPROVED_HEADER_STR)),
					Child, CM_ngadv_APPVHD = CheckMark(FALSE),
					Child, STR_ngadv_APPVHD=BetterStringObject, StringFrame, MUIA_String_Contents, "", MUIA_String_MaxLen, 64, MUIA_CycleChain, 1, End,
					Child, Label1(CatalogStr(MSG_ALTERNATIVE_NAME,MSG_ALTERNATIVE_NAME_STR)),
					Child, CM_ngadv_ALTNAME = CheckMark(FALSE),
					Child, STR_ngadv_ALTNAME=BetterStringObject, StringFrame, MUIA_String_Contents, "", MUIA_String_MaxLen, 32, MUIA_CycleChain, 1, End,
					Child, Label1(CatalogStr(MSG_ALTERNATIVE_EMAIL,MSG_ALTERNATIVE_EMAIL_STR)),
					Child, CM_ngadv_ALTEMAIL = CheckMark(FALSE),
					Child, STR_ngadv_ALTEMAIL=BetterStringObject, StringFrame, MUIA_String_Contents, "", MUIA_String_MaxLen, 32, MUIA_CycleChain, 1, End,
					End,
				Child, GROUP_ngadv=VGroup,
					End,
				Child, ColGroup(2),
					Child, BT_ngadv_OKAY=SimpleButton(CatalogStr(MSG_OKAY,MSG_OKAY_STR)),
					Child, BT_ngadv_CANCEL=SimpleButton(CatalogStr(MSG_CANCEL_2,MSG_CANCEL_2_STR)),
					End,
				End,
			End,

		SubWindow, wnd_joinmsgs = WindowObject,
			MUIA_Window_Title,			CatalogStr(MSG_JOIN_MESSAGES,MSG_JOIN_MESSAGES_STR),
			MUIA_Window_ID,				MAKE_ID('J','M','S','G'),
			MUIA_Window_ScreenTitle,	scrtitle,
			MUIA_Window_CloseGadget,	TRUE,

			WindowContents, VGroup,
				Child, NListviewObject,
					MUIA_NListview_NList,			NLIST_joinmsgs_MSGS=NListObject,
						MUIA_NList_Title,			TRUE,
						MUIA_NList_DragType,		MUIV_NList_DragType_Immediate,
						MUIA_NList_DragSortable,	TRUE,
						MUIA_NList_DisplayHook,		&DisplayHook_joinmsgs,
						MUIA_NList_CompareHook,		&CompareHook_joinmsgs,
						MUIA_NList_DestructHook,	&DestructHook_joinmsgs,
						MUIA_NList_Format,			",,,,,",
						MUIA_NList_AutoVisible,		TRUE,
						MUIA_NList_MinColSortable,	0,
						End,
					End,
				Child, ColGroup(2),
					Child, Label2(CatalogStr(LABEL_SUBJECT,LABEL_SUBJECT_STR)), Child, STR_joinmsgs_SUBJECT=BetterString("",1024),
					End,
				Child, ColGroup(2),
					Child, BT_joinmsgs_JOIN=SimpleButton(CatalogStr(MSG_JOIN,MSG_JOIN_STR)),
					Child, BT_joinmsgs_CANCEL=SimpleButton(CatalogStr(MSG_CANCEL_2,MSG_CANCEL_2_STR)),
					End,
				End,
			End,

		SubWindow, wnd_account = WindowObject,
			MUIA_Window_Title,			CatalogStr(TBAR_SETTINGS_HELP,TBAR_SETTINGS_HELP_STR),
			MUIA_Window_ID,				MAKE_ID('A','C','C','S'),
			MUIA_Window_ScreenTitle,	scrtitle,
			MUIA_Window_CloseGadget,	TRUE,
			MUIA_HelpNode,					"WIN_ACC",

			WindowContents, VGroup,

				Child, pages_account=RegisterGroup(Pages_acc),
					MUIA_Register_Frame, TRUE,
					MUIA_CycleChain, 1,
					//page 1
					Child, VGroup,
						Child, ColGroup(2),
							Child, Label2(CatalogStr(LABEL_YOUR_NAME,LABEL_YOUR_NAME_STR)), Child, STR_acc_NAME=BetterString("",64),
							Child, Label2(CatalogStr(LABEL_EMAIL_FOR_POSTING,LABEL_EMAIL_FOR_POSTING_STR)), Child, STR_acc_EMAIL=BetterString("",64),
							Child, Label2(CatalogStr(LABEL_REAL_EMAIL_FOR_POSTING,LABEL_REAL_EMAIL_FOR_POSTING_STR)), Child, STR_acc_REALEMAIL=BetterString("",64),
							Child, Label2(CatalogStr(LABEL_ORGANISATION,LABEL_ORGANISATION_STR)), Child, STR_acc_ORG=BetterString("",64),
							End,
						Child, ColGroup(4),
							Child, Label1(CatalogStr(LABEL_TIME_ZONE,LABEL_TIME_ZONE_STR)), Child, CY_acc_TIMEZONE=Cycle(CYA_acc_timezone),
							Child, Label1(CatalogStr(LABEL_DATE_DISPLAY_FORMAT,LABEL_DATE_DISPLAY_FORMAT_STR)), Child, CY_acc_DATEFORMAT=Cycle(CYA_acc_dateformat),
							End,

						Child, ColGroup(4),
							Child, Label1(CatalogStr(LABEL_ADD_DST_ADJUSTMENT,LABEL_ADD_DST_ADJUSTMENT_STR)), Child, CM_acc_BST = CheckMark(FALSE),
							Child, Label1(CatalogStr(LABEL_USE_LOCALE_PREFS,LABEL_USE_LOCALE_PREFS_STR)), Child, CM_acc_USELOCALETZ=CheckMark(FALSE),

							Child, Label1(CatalogStr(LABEL_ALWAYS_CHECK_FOR_DUPES,LABEL_ALWAYS_CHECK_FOR_DUPES_STR)), Child, CM_acc_CHECKFORDUPS = CheckMark(FALSE),
							Child, Label1(CatalogStr(LABEL_DELETE_HEADER_IF_NO_BODY,LABEL_DELETE_HEADER_IF_NO_BODY_STR)), Child, CM_acc_DELONLINE = CheckMark(TRUE),

							Child, Label1(CatalogStr(LABEL_DONT_SHOW_FROM_SUBJ_WHEN_DL,LABEL_DONT_SHOW_FROM_SUBJ_WHEN_DL_STR)), Child, CM_acc_QUIETDL = CheckMark(FALSE),
							Child, Label1(CatalogStr(LABEL_SWITCH_GROUPS_WHEN_DL,LABEL_SWITCH_GROUPS_WHEN_DL_STR)), Child, CM_acc_VGROUPDL = CheckMark(FALSE),

							Child, Label1(CatalogStr(LABEL_DONT_CONFIRM_WHEN_DELETING,LABEL_DONT_CONFIRM_WHEN_DELETING_STR)), Child, CM_acc_NOCONFIRMDEL = CheckMark(FALSE),
							Child, Label1(CatalogStr(LABEL_ALWAYS_CONFIRM_ON_QUIT,LABEL_ALWAYS_CONFIRM_ON_QUIT_STR)), Child, CM_acc_CONFIRMQUIT = CheckMark(TRUE),

							Child, Label1(CatalogStr(LABEL_LOGGING_ENABLED,LABEL_LOGGING_ENABLED_STR)), Child, CM_acc_LOGGING = CheckMark(FALSE),
							Child, Label1(CatalogStr(LABEL_DELETE_LOG_ON_STARTUP,LABEL_DELETE_LOG_ON_STARTUP_STR)), Child, CM_acc_LOGDEL = CheckMark(TRUE),

							//Child, HSpace(0),
							End,
						End,
					//page 2
					Child, VGroup,
						Child, NListviewObject,
							MUIA_NListview_NList,			NLIST_acc_SERVERS=NListObject,
								MUIA_NList_Title,				TRUE,
								MUIA_NList_DragType,			MUIV_NList_DragType_Immediate,
								MUIA_NList_DragSortable,	TRUE,
								MUIA_NList_DisplayHook,		&DisplayHook_servers,
								MUIA_NList_DestructHook,	&DestructHook_servers,
								MUIA_NList_Format,			",,,,,",
								MUIA_NList_MinColSortable,	0,
								End,
							End,
						Child, ColGroup(5),
							Child, BT_acc_ADDSERVER=SimpleButton(CatalogStr(MSG_NEW_SERVER,MSG_NEW_SERVER_STR)),
							Child, BT_acc_EDITSERVER=SimpleButton(CatalogStr(MSG_EDIT_SERVER_2,MSG_EDIT_SERVER_2_STR)),
							Child, BT_acc_MAKEDEFSERVER=SimpleButton(CatalogStr(MSG_MAKE_DEFAULT,MSG_MAKE_DEFAULT_STR)),
							Child, BT_acc_MAKEPOSTSERVER=SimpleButton(CatalogStr(MSG_USE_FOR_POSTING,MSG_USE_FOR_POSTING_STR)),
							Child, BT_acc_DELETESERVER=SimpleButton(CatalogStr(MSG_DELETE_SERVER_2,MSG_DELETE_SERVER_2_STR)),
							End,
						Child, ColGroup(3),
							Child, BT_acc_GETGROUPS=SimpleButton(CatalogStr(MSG_GET_GROUPS,MSG_GET_GROUPS_STR)),
							Child, BT_acc_GETNEWGROUPS=SimpleButton(CatalogStr(MSG_GET_NEW_GROUPS,MSG_GET_NEW_GROUPS_STR)),
							Child, BT_acc_GROUPMAN=SimpleButton(CatalogStr(MSG_GROUPS_MANAGER,MSG_GROUPS_MANAGER_STR)),
							End,
						Child, ColGroup(2),
							Child, Label2(CatalogStr(LABEL_SMTP_SERVER,LABEL_SMTP_SERVER_STR)),
								Child, STR_acc_SMTP=BetterString("",64),
							End,
						End,
					//page 3
					Child, VGroup,
						Child, ColGroup(2),
							Child, Label2(CatalogStr(LABEL_FOLLOWUP_ATTRIBUTATION_LINE,LABEL_FOLLOWUP_ATTRIBUTATION_LINE_STR)), Child, STR_acc_FOLLOWUPTEXT=BetterString("",128),
								Child, Label2(CatalogStr(LABEL_CHARSET,LABEL_CHARSET_STR)),
								Child, POP_acc_CHARSETWRITE = PopobjectObject,
									MUIA_Popstring_String, STR_acc_CHARSETWRITE=TextObject,
										MUIA_Background,MUII_TextBack,
										MUIA_Frame,MUIV_Frame_Text,
										End,
									MUIA_Popstring_Button, PopButton(MUII_PopUp),
									MUIA_Popobject_StrObjHook, &StrObj2Hook,
									MUIA_Popobject_ObjStrHook, &ObjStrAcc4Hook,
									MUIA_Popobject_WindowHook, &WindowHook,
									MUIA_Popobject_Object, LIST_acc_CHARSETWRITE = ListviewObject,
										MUIA_Listview_List, ListObject,
											InputListFrame,
											MUIA_List_ConstructHook,MUIV_List_ConstructHook_String,
											MUIA_List_DestructHook,MUIV_List_DestructHook_String,
											End,
										End,
									End,
								End,
						Child, ColGroup(2),
							Child, CY_acc_SIG=Cycle(CYA_acc_sigs),
							Child, BT_acc_READSIG=SimpleButton(CatalogStr(MSG_READ_FROM_FILE,MSG_READ_FROM_FILE_STR)),
							End,
						Child, HGroup,
							MUIA_Group_Spacing, 0,
							Child, ED_acc_SIG = (Object*)NewObject(editor_mcc->mcc_Class, NULL,
								MUIA_TextEditor_Slider, SLD_acc_SIG,
								MUIA_CycleChain, 1,
								End,
							Child, SLD_acc_SIG,
							End,
						Child, ColGroup(4),
							Child, Label2(CatalogStr(LABEL_LINE_LENGTH,LABEL_LINE_LENGTH_STR)), Child, STR_acc_LINELENGTH=BetterStringObject, StringFrame, MUIA_String_Integer, 0, MUIA_String_MaxLen, 3, MUIA_String_Accept, "1234567890", MUIA_CycleChain, 1, End,
							Child, Label1(CatalogStr(LABEL_AUTO_REWRAP,LABEL_AUTO_REWRAP_STR)), Child, CM_acc_REWRAP = CheckMark(TRUE),
							End,
						Child, ColGroup(2),
							Child, Label1(CatalogStr(LABEL_USE_X_NO_ARCHIVE,LABEL_USE_X_NO_ARCHIVE_STR)), Child, CY_acc_XNO=Cycle(CYA_acc_xno),
							End,
						Child, ColGroup(4),
							Child, Label1(CatalogStr(LABEL_USE_SHORT_X_NEWSREADER,LABEL_USE_SHORT_X_NEWSREADER_STR)), Child, CM_acc_XNEWS = CheckMark(FALSE),
							Child, Label1(CatalogStr(LABEL_SNIP_SIG,LABEL_SNIP_SIG_STR)), Child, CM_acc_SNIPSIG = CheckMark(FALSE),
							End,
						End,
					//page 4
					Child, VGroup,
						Child, ColGroup(5),
							Child, Label1(CatalogStr(MSG_HEADER,MSG_HEADER_STR)),
							Child, CY_acc_READHEADER=Cycle(CYA_acc_readheader),
							Child, STR_acc_READHEADER=BetterString("",READHEADER_LEN),
							Child, Label2(CatalogStr(LABEL_MULTIPLE_VIEW_WINDOWS,LABEL_MULTIPLE_VIEW_WINDOWS_STR)),
							Child, CM_acc_MULTIPLEVWS = CheckMark(FALSE),
							End,
						Child, ColGroup(2),
							Child, FreeLabel(CatalogStr(LABEL_2ND_LEVEL_QUOTING,LABEL_2ND_LEVEL_QUOTING_STR)),
							Child, PEN_acc_TEXT_QUOTE2 = PoppenObject,
								MUIA_CycleChain, 1,
								MUIA_Window_Title, CatalogStr(MSG_2ND_LEVEL_QUOTE,MSG_2ND_LEVEL_QUOTE_STR)),
							Child, FreeLabel(CatalogStr(LABEL_COLOURED,LABEL_COLOURED_STR)),
							Child, PEN_acc_TEXT_COL = PoppenObject,
								MUIA_CycleChain, 1,
								MUIA_Window_Title, CatalogStr(MSG_COLOURED_TEXT,MSG_COLOURED_TEXT_STR)),
							End,
						End,
					//page 5
					Child, VGroup,
						Child, ColGroup(2),
							Child, Label2(CatalogStr(LABEL_FLAGS,LABEL_FLAGS_STR)), Child, CM_acc_LISTFLAGS = CheckMark(TRUE),
							Child, Label2(CatalogStr(LABEL_SUBJECT_2,LABEL_SUBJECT_2_STR)), Child, CM_acc_LISTSUBJECT = CheckMark(TRUE),
							Child, Label2(CatalogStr(LABEL_DATE,LABEL_DATE_STR)), Child, CM_acc_LISTDATE = CheckMark(TRUE),
							Child, Label2(CatalogStr(LABEL_FROM_NEWSGROUP,LABEL_FROM_NEWSGROUP_STR)), Child, CM_acc_LISTFROMGROUP = CheckMark(TRUE),
							Child, Label2(CatalogStr(LABEL_SIZE,LABEL_SIZE_STR)), Child, CM_acc_LISTSIZE = CheckMark(TRUE),
							Child, Label2(CatalogStr(LABEL_N_OF_LINES,LABEL_N_OF_LINES_STR)), Child, CM_acc_LISTLINES = CheckMark(TRUE),
							End,
						End,
					//page 6
					Child, VGroup,
						Child, NListviewObject,
							MUIA_NListview_NList,			NLIST_acc_MIMEPREFS=NListObject,
								MUIA_NList_Title,				TRUE,
								MUIA_NList_DragSortable,	TRUE,
								MUIA_NList_DragType,			MUIV_NList_DragType_Immediate,
								MUIA_NList_DisplayHook,		&DisplayHook_mimeprefs,
								MUIA_NList_DestructHook,	&DestructHook_mimeprefs,
								MUIA_NList_Format,			",",
								End,
							End,
						Child, VGroup,
							Child, ColGroup(4),
								Child, Label2(CatalogStr(LABEL_MIME_TYPE,LABEL_MIME_TYPE_STR)),
								Child, POP_acc_MIME = PopobjectObject,
									MUIA_Popstring_String, STR_acc_MIME=BetterString("",32),
									MUIA_Popstring_Button, PopButton(MUII_PopUp),
									MUIA_Popobject_StrObjHook, &StrObjHook,
									MUIA_Popobject_ObjStrHook, &ObjStrAccHook,
									MUIA_Popobject_WindowHook, &WindowHook,
									MUIA_Popobject_Object, LIST_acc_MIME = ListviewObject,
										MUIA_Listview_List, ListObject,
											InputListFrame,
											MUIA_List_SourceArray, POPA_write_mime,
											End,
										End,
									End,
								Child, Label2(CatalogStr(LABEL_VIEWER,LABEL_VIEWER_STR)),
								Child, POP_acc_MIMEVIEW = PopaslObject,
									MUIA_Popstring_String, STR_acc_MIMEVIEW=BetterString("",MAXFILENAME),
									MUIA_Popstring_Button, PopButton(MUII_PopFile),
									MUIA_Popobject_ObjStrHook, &ObjStrAcc2Hook,
									ASLFR_TitleText, CatalogStr(MSG_PLEASE_SELECT_VIEWER,MSG_PLEASE_SELECT_VIEWER_STR),
									End,
								End,
							Child, ColGroup(2),
								Child, Label2(CatalogStr(LABEL_DEFAULT_VIEWER,LABEL_DEFAULT_VIEWER_STR)),
								Child, POP_acc_MIMEDEF = PopaslObject,
									MUIA_Popstring_String, STR_acc_MIMEDEF=BetterString("",MAXFILENAME),
									MUIA_Popstring_Button, PopButton(MUII_PopFile),
									MUIA_Popobject_ObjStrHook, &ObjStrAcc3Hook,
									ASLFR_TitleText, CatalogStr(MSG_PLEASE_SELECT_VIEWER,MSG_PLEASE_SELECT_VIEWER_STR),
									End,
								End,
							Child, ColGroup(2),
								Child, BT_acc_MIMENEW=SimpleButton(CatalogStr(MSG_NEW,MSG_NEW_STR)),
								Child, BT_acc_MIMEDEL=SimpleButton(CatalogStr(TBAR_DELETE,TBAR_DELETE_STR)),
								End,
							End,
						End,
					End,

				Child, ColGroup(2),
					Child, BT_acc_OKAY=SimpleButton(CatalogStr(MSG_OKAY,MSG_OKAY_STR)),
					Child, BT_acc_CANCEL=SimpleButton(CatalogStr(MSG_CANCEL,MSG_CANCEL_STR)),
					End,

				End,
			End,

		SubWindow, wnd_servers = WindowObject,
			MUIA_Window_Title,			CatalogStr(MSG_SERVER,MSG_SERVER_STR),
			MUIA_Window_ID,				MAKE_ID('S','E','R','V'),
			MUIA_Window_ScreenTitle,	scrtitle,
			MUIA_Window_CloseGadget,	FALSE,

			WindowContents, VGroup,
				Child, ColGroup(4),
					Child, Label2(CatalogStr(LABEL_NNTP_SERVER,LABEL_NNTP_SERVER_STR)), Child, STR_acc_NNTP=BetterString("",64),
					Child, Label2(CatalogStr(LABEL_PORT,LABEL_PORT_STR)), Child, STR_acc_PORT=BetterStringObject, StringFrame, MUIA_String_Integer, 119, MUIA_String_MaxLen, 6, MUIA_String_Accept, "1234567890", MUIA_CycleChain, 1, End,
					End,
				Child, ColGroup(6),
					Child, Label1(CatalogStr(LABEL_USE_NNTP_AUTH,LABEL_USE_NNTP_AUTH_STR)), Child, CM_acc_AUTH = CheckMark(FALSE),
					Child, Label2(CatalogStr(LABEL_USER,LABEL_USER_STR)), Child, STR_acc_USER=BetterString("",64),
					Child, Label2(CatalogStr(LABEL_PASSWORD,LABEL_PASSWORD_STR)), Child, STR_acc_PASS=BetterStringObject, StringFrame, MUIA_String_Contents, "", MUIA_String_MaxLen, 64, MUIA_String_Secret, TRUE, MUIA_CycleChain, 1, End,
					End,
				Child, ColGroup(2),
					Child, BT_server_OKAY=SimpleButton(CatalogStr(MSG_OKAY,MSG_OKAY_STR)),
					Child, BT_server_CANCEL=SimpleButton(CatalogStr(MSG_CANCEL_2,MSG_CANCEL_2_STR)),
					End,
				End,
			End,

		SubWindow, wnd_killfile = WindowObject,
			MUIA_Window_Title,			CatalogStr(LABEL_KILLFILE_SETTINGS,LABEL_KILLFILE_SETTINGS_STR),
			MUIA_Window_ID,				MAKE_ID('K','I','L','L'),
			MUIA_Window_ScreenTitle,	scrtitle,
			MUIA_Window_CloseGadget,	TRUE,
			MUIA_HelpNode,					"WIN_KILLFILE",

			WindowContents, VGroup,
				Child, NListviewObject,
					MUIA_NListview_NList,			NLIST_acc_KILLFILE=NListObject,
						MUIA_NList_Title,				TRUE,
						MUIA_NList_DragType,			MUIV_NList_DragType_Immediate,
						MUIA_NList_DragSortable,	TRUE,
						MUIA_NList_DisplayHook,		&DisplayHook_killfile,
						MUIA_NList_DestructHook,	&DestructHook_killfile,
						MUIA_NList_Format,			",,,,,,,,",
						MUIA_NList_MinColSortable,	0,
						End,
					End,
				Child, ColGroup(2),
					Child, Label2(CatalogStr(LABEL_IF_HEADER,LABEL_IF_HEADER_STR)),
					Child, STR_acc_NEWKILLHEAD=BetterStringObject, StringFrame, MUIA_String_Contents, "", MUIA_String_MaxLen, 63, MUIA_CycleChain, 1, End,
					//Child, Label2("Text to Kill:"),
					Child, CY_acc_MATCHKILL=Cycle(CYA_acc_matchkill),
					Child, STR_acc_NEWKILLTEXT=BetterStringObject, StringFrame, MUIA_String_Contents, "", MUIA_String_MaxLen, 256, MUIA_CycleChain, 1, End,
					Child, Label2(CatalogStr(LABEL_IN_THE_GROUPS,LABEL_IN_THE_GROUPS_STR)),
					Child, STR_acc_NEWKILLGROUP=BetterStringObject, StringFrame, MUIA_String_Contents, "", MUIA_String_MaxLen, 64, MUIA_String_Reject, ",", MUIA_CycleChain, 1, End,
					End,
				Child, ColGroup(4),
					Child, Label(CatalogStr(LABEL_THEN,LABEL_THEN_STR)),
					Child, CY_acc_ACTIONKILL=Cycle(CYA_acc_actionkill),
					Child, Label1(CatalogStr(LABEL_SKIP_REST,LABEL_SKIP_REST_STR)),
					Child, CM_acc_SKIPKILL=CheckMark(TRUE),
					End,
				Child, HGroup,
					Child, CY_acc_EXPIREKILL=Cycle(CYA_acc_expirekill),
					Child, STR_acc_EXPIREKILL=BetterStringObject, StringFrame, MUIA_String_Integer, 0, MUIA_String_MaxLen, 5, MUIA_String_Accept, "1234567890", MUIA_CycleChain, 1, End,
					End,
				//Child, CY_acc_TYPEKILL=Cycle(CYA_acc_typekill),
				Child, ColGroup(2),
					Child, ColGroup(2),
						Child, BT_acc_NEWKILL=SimpleButton(CatalogStr(MSG_NEW,MSG_NEW_STR)),
						Child, BT_acc_NEWKILLADD=SimpleButton(CatalogStr(MSG_ADD_REPLACE,MSG_ADD_REPLACE_STR)),
						Child, BT_acc_DUPKILL=SimpleButton(CatalogStr(MSG_DUPLICATE,MSG_DUPLICATE_STR)),
						Child, BT_acc_DELKILL=SimpleButton(CatalogStr(TBAR_DELETE,TBAR_DELETE_STR)),
						End,
					Child, ColGroup(2),
						Child, Label1(CatalogStr(LABEL_ADVERT_KILLFILE,LABEL_ADVERT_KILLFILE_STR)),
						Child, CM_acc_SIZEKILL=CheckMark(FALSE),
						End,
					End,
				End,
			End,

		SubWindow, wnd_groupman = WindowObject,
			MUIA_Window_Title,			CatalogStr(MSG_GROUP_MANAGER,MSG_GROUP_MANAGER_STR),
			MUIA_Window_ID,				MAKE_ID('G','M','A','N'),
			MUIA_Window_ScreenTitle,	scrtitle,
			MUIA_HelpNode,					"WIN_GMAN",

			WindowContents, VGroup,
				Child, NListviewObject,
					MUIA_NListview_NList,			NLIST_groupman=NListObject,
						MUIA_NList_DestructHook,	&DestructHook_groupman,
						End,
					End,
				Child, ColGroup(2),
					Child, Label2(CatalogStr(LABEL_FIND,LABEL_FIND_STR)), Child, STR_groupman_FIND=BetterString("",64),
					End,
				//Child, ColGroup(5),
				Child, ColGroup(2),
					Child, BT_groupman_SUB=SimpleButton(CatalogStr(MSG_SUBSCRIBE,MSG_SUBSCRIBE_STR)),
					Child, BT_groupman_GET=SimpleButton(CatalogStr(MSG_DOWLOAD_GROUP_LIST,MSG_DOWLOAD_GROUP_LIST_STR)),
					//Child, BT_groupman_SORT=SimpleButton("S_ort Groups"),
					//Child, BT_groupman_WRITE=SimpleButton("_Write List"),
					//Child, BT_groupman_CLOSE=SimpleButton("_Close Window"),
					End,
				End,
			End,

		SubWindow, wnd_search = WindowObject,
			MUIA_Window_Title,			CatalogStr(TBAR_SEARCH,TBAR_SEARCH_STR),
			MUIA_Window_ID,				MAKE_ID('S','E','A','R'),
			MUIA_Window_ScreenTitle,	scrtitle,
			MUIA_HelpNode,					"WIN_SEARCH",

			WindowContents, VGroup,
				Child, VGroup,
					MUIA_Group_Spacing, 0,
					MUIA_VertWeight, 100,
					Child, VGroup,
						Child, NListviewObject,
							MUIA_NListview_NList,			NLIST_search_NG=NListObject,
								MUIA_NList_Title,				TRUE,
								MUIA_NList_MultiSelect,		MUIV_NList_MultiSelect_Default,
								MUIA_NList_DisplayHook,		&DisplayHook_simpleglist,
								MUIA_NList_DestructHook,	&DestructHook_groupdata,
								End,
							End,
						Child, VGroup,
							Child, ColGroup(2),
								Child, Label1(CatalogStr(LABEL_SEARCH_IN,LABEL_SEARCH_IN_STR)), Child, CY_search_WHERE=Cycle(CYA_search_where),
								End,
							Child, ColGroup(4),
								Child, Label2(CatalogStr(LABEL_SEARCH_FOR,LABEL_SEARCH_FOR_STR)), Child, STR_search_WHAT=BetterString("",64),
								Child, Label1(CatalogStr(LABEL_CASE_SENSITIVE,LABEL_CASE_SENSITIVE_STR)), Child, CM_search_CASESENS=CheckMark(FALSE),
								End,
							End,
						End,
					End,
				Child, BalanceObject, End,
				Child, VGroup,
					MUIA_Group_Spacing, 0,
					MUIA_VertWeight, 100,
					Child, NListviewObject,
						MUIA_NListview_NList,			NLIST_search_RES=NListObject,
							MUIA_NList_Title,				TRUE,
							MUIA_NList_MultiSelect,		MUIV_NList_MultiSelect_Default,
							MUIA_NList_DragType,			MUIV_NList_DragType_None,
							MUIA_NList_DisplayHook,		&DisplayHook_search,
							MUIA_NList_CompareHook,		&CompareHook_search,
							MUIA_NList_DestructHook,	&DestructHook_search,
							MUIA_NList_Format,			",,,,,",
							MUIA_NList_AutoVisible,		TRUE,
							MUIA_NList_MinColSortable,	0,
							End,
						End,
					Child, BT_search_START=SimpleButton(CatalogStr(MSG_START_SEARCH,MSG_START_SEARCH_STR)),
					End,
				End,
			End,

		SubWindow, wnd_stats = WindowObject,
			MUIA_Window_Title,			CatalogStr(MSG_STATISTICS,MSG_STATISTICS_STR),
			MUIA_Window_ID,				MAKE_ID('S','T','A','T'),
			MUIA_Window_ScreenTitle,	scrtitle,

			WindowContents, VGroup,
				Child, VGroup,
					MUIA_Group_Spacing, 0,
					MUIA_VertWeight, 100,
					Child, VGroup,
						Child, NListviewObject,
							MUIA_NListview_NList,			NLIST_stats_NG=NListObject,
								MUIA_NList_Title,				TRUE,
								MUIA_NList_MultiSelect,		MUIV_NList_MultiSelect_Default,
								MUIA_NList_DisplayHook,		&DisplayHook_simpleglist,
								MUIA_NList_DestructHook,	&DestructHook_groupdata,
								End,
							End,
						Child, VGroup,
							Child, ColGroup(2),
								Child, Label1(CatalogStr(LABEL_TYPE,LABEL_TYPE_STR)), Child, CY_stats_WHAT=Cycle(CYA_stats_what),
								End,
							End,
						End,
					End,
				Child, BalanceObject, End,
				Child, VGroup,
					MUIA_Group_Spacing, 0,
					MUIA_VertWeight, 100,
					Child, NListviewObject,
						MUIA_NListview_NList,			NLIST_stats_RES=NListObject,
							MUIA_NList_DisplayHook,		&DisplayHook_stats,
							MUIA_NList_DestructHook,	&DestructHook_stats,
							MUIA_NList_CompareHook,		&CompareHook_stats,
							MUIA_NList_MultiSelect,		MUIV_NList_MultiSelect_Default,
							MUIA_NList_Title,				TRUE,
							MUIA_NList_Format,			",",
							MUIA_NList_AutoVisible,		TRUE,
							End,
						End,
					Child, BT_stats_START=SimpleButton(CatalogStr(MSG_START,MSG_START_STR)),
					End,
				End,
			End,

		SubWindow, wnd_newuser = WindowObject,
			MUIA_Window_Title,			CatalogStr(MSG_NEW_USER,MSG_NEW_USER_STR),
			MUIA_Window_ID,				MAKE_ID('N','U','S','R'),
			MUIA_Window_ScreenTitle,	scrtitle,

			WindowContents, VGroup,
				Child, TXT_newuser_INFO=TextObject, GroupFrame, MUIA_Text_Contents, CatalogStr(MSG_CREATE_NEW_USER_DESCRIPTION,MSG_CREATE_NEW_USER_DESCRIPTION_STR), MUIA_Text_PreParse, "\33c", MUIA_Text_SetMin, TRUE, End,
				Child, ColGroup(2),
					Child, ColGroup(2),
						Child, Label2(CatalogStr(LABEL_USER_NAME,LABEL_USER_NAME_STR)), Child, STR_newuser_USER=BetterString("",9),
						Child, Label2(CatalogStr(LABEL_PASSWORD_2,LABEL_PASSWORD_2_STR)), Child, STR_newuser_PASS=BetterStringObject, StringFrame, MUIA_String_MaxLen, 9, MUIA_CycleChain, 1, MUIA_String_Secret, TRUE, End,
						Child, Label2(CatalogStr(LABEL_NEWS_DIRECTORY,LABEL_NEWS_DIRECTORY_STR)),
						Child, POP_newuser_DATALOC = PopaslObject,
							MUIA_Popstring_String, STR_newuser_DATALOC=BetterString("PROGDIR:",300),
							MUIA_Popstring_Button, PopButton(MUII_PopFile),
							MUIA_Popobject_ObjStrHook, &ObjStrNewuserHook,
							ASLFR_TitleText, CatalogStr(MSG_SELECT_DIRECTORY_TO_STORE_NEWS,MSG_SELECT_DIRECTORY_TO_STORE_NEWS_STR),
							End,
						End,
					Child, ColGroup(2),
						Child, Label1(CatalogStr(LABEL_SUPERVISOR,LABEL_SUPERVISOR_STR)), Child, CM_newuser_SUP=CheckMark(TRUE),
						Child, Label1(CatalogStr(LABEL_USE_PASSWORD,LABEL_USE_PASSWORD_STR)), Child, CM_newuser_PASS=CheckMark(FALSE),
						Child, Label1(CatalogStr(MSG_COPY_PREFS,MSG_COPY_PREFS_STR)), Child, CM_newuser_COPYPREFS=CheckMark(FALSE),
						End,
					End,
				Child, ColGroup(2),
					Child, BT_newuser_OKAY=SimpleButton(CatalogStr(MSG_OKAY,MSG_OKAY_STR)),
					Child, BT_newuser_CANCEL=SimpleButton(CatalogStr(MSG_CANCEL_2,MSG_CANCEL_2_STR)),
					End,
				End,
			End,

		SubWindow, wnd_users = WindowObject,
			MUIA_Window_Title,			CatalogStr(MSG_USERS,MSG_USERS_STR),
			MUIA_Window_ID,				MAKE_ID('U','S','E','R'),
			MUIA_Window_ScreenTitle,	scrtitle,

			WindowContents, VGroup,
				Child, NListviewObject,
					MUIA_NListview_NList,			NLIST_users_LIST=NListObject,
						MUIA_NList_DisplayHook,		&DisplayHook_users,
						MUIA_NList_DestructHook,	&DestructHook_users,
						MUIA_NList_CompareHook,		&CompareHook_users,
						MUIA_NList_MultiSelect,		FALSE,
						MUIA_NList_Title,			TRUE,
						MUIA_NList_Format,			",,,",
						MUIA_NList_AutoVisible,		TRUE,
						End,
					End,
				Child, ColGroup(2),
					Child, BT_users_NEW=SimpleButton(CatalogStr(MSG_ADD_NEW_USER,MSG_ADD_NEW_USER_STR)),
					Child, BT_users_DELETE=SimpleButton(CatalogStr(MSG_DELETE_USER_2,MSG_DELETE_USER_2_STR)),
					End,
				End,
			End,

		SubWindow, wnd_login = WindowObject,
			MUIA_Window_Title,			CatalogStr(MSG_USER_LOGIN,MSG_USER_LOGIN_STR),
			MUIA_Window_ID,				MAKE_ID('U','L','O','G'),
			MUIA_Window_ScreenTitle,	scrtitle,

			WindowContents, VGroup,
				Child, ColGroup(2),
					Child, Label2(CatalogStr(LABEL_USER_NAME,LABEL_USER_NAME_STR)), Child, STR_login_USER=BetterStringObject,StringFrame,MUIA_String_MaxLen,USER_LEN+1,MUIA_CycleChain,TRUE,MUIA_String_AdvanceOnCR,TRUE,End,
					Child, Label2(CatalogStr(LABEL_PASSWORD_2,LABEL_PASSWORD_2_STR)), Child, STR_login_PASS=BetterStringObject, StringFrame, MUIA_String_MaxLen, USER_LEN+1, MUIA_CycleChain, 1, MUIA_String_Secret, TRUE, End,
					End,
				Child, ColGroup(2),
					Child, BT_login_OKAY=SimpleButton(CatalogStr(MSG_OKAY,MSG_OKAY_STR)),
					Child, BT_login_CANCEL=SimpleButton(CatalogStr(MSG_CANCEL_2,MSG_CANCEL_2_STR)),
					End,
				End,
			End,

		SubWindow, wnd_cpwd = WindowObject,
			MUIA_Window_Title,			CatalogStr(MSG_CHANGE_PASSWORD,MSG_CHANGE_PASSWORD_STR),
			MUIA_Window_ID,				MAKE_ID('C','P','W','D'),
			MUIA_Window_ScreenTitle,	scrtitle,

			WindowContents, VGroup,
				Child, TXT_cpwd_INFO=TextObject, GroupFrame, MUIA_Text_Contents, CatalogStr(MSG_PLEASE_ENTER_NEW_PASSWORD,MSG_PLEASE_ENTER_NEW_PASSWORD_STR), MUIA_Text_PreParse, "\33c", MUIA_Text_SetMin, TRUE, End,
				Child, ColGroup(4),
					Child, Label2(CatalogStr(LABEL_PASSWORD_2,LABEL_PASSWORD_2_STR)), Child, STR_cpwd_PASS=BetterStringObject, StringFrame, MUIA_String_MaxLen, 9, MUIA_CycleChain, 1, MUIA_String_Secret, TRUE, End,
					Child, Label1(CatalogStr(LABEL_USE_PASSWORD,LABEL_USE_PASSWORD_STR)), Child, CM_cpwd_PASS=CheckMark(FALSE),
					End,
				Child, ColGroup(2),
					Child, BT_cpwd_OKAY=SimpleButton(CatalogStr(MSG_OKAY,MSG_OKAY_STR)),
					Child, BT_cpwd_CANCEL=SimpleButton(CatalogStr(MSG_CANCEL_2,MSG_CANCEL_2_STR)),
					End,
				End,
			End,

		SubWindow, wnd_about = WindowObject,
			MUIA_Window_Title,			CatalogStr(MSG_ABOUT,MSG_ABOUT_STR),
			MUIA_Window_ID,				MAKE_ID('A','B','T','P'),

			WindowContents, VGroup,
				Child, TextObject, GroupFrame, MUIA_Text_Contents, abouttext, End,
				Child, TextList(TEXT_About),
				Child, BT_about_OKAY=SimpleButton(CatalogStr(MSG_OKAY,MSG_OKAY_STR)),
				End,
			End,

		End;

	if (!app)
	{
		fail(app,CatalogStr(MSG_FAILED_TO_CREATE_APPLICATION,MSG_FAILED_TO_CREATE_APPLICATION_STR));
		return 0;
	}

	grouplistGroup_NLIST = VGroup,
						Child, NListviewObject,
							MUIA_NListview_NList,			NLIST_groupdata=NListObject,
								MUIA_NList_Title,				TRUE,
								MUIA_NList_DragType,			MUIV_NList_DragType_Immediate,
								MUIA_NList_DragSortable,	TRUE,
								MUIA_NList_DisplayHook,		&DisplayHook_groupdata,
								MUIA_NList_CompareHook,		&CompareHook_groupdata,
								MUIA_NList_DestructHook,	&DestructHook_groupdata,
								MUIA_NList_Format,			",,,",
								MUIA_NList_AutoVisible,		TRUE,
								MUIA_NList_MinColSortable,	0,
								End,
							End,
						End;

	grouplistGroup_LISTTREE = VGroup,
						Child, LISTVIEW_groupdata=NListviewObject,
							MUIA_NListview_NList,			LISTTREE_groupdata=NListtreeObject,
								InputListFrame,
								MUIA_NListtree_EmptyNodes,		TRUE,
								MUIA_NListtree_Title,			TRUE,
								MUIA_NListtree_DisplayHook,	&DisplayHook_groupdatatree,
								MUIA_NListtree_DestructHook,	&DestructHook_groupdatatree,
								MUIA_NListtree_Format,			",,,",
								MUIA_NListtree_DoubleClick,	MUIV_NListtree_DoubleClick_Tree,
								MUIA_NListtree_DragDropSort,	TRUE,
								MUIA_NList_MinColSortable,		0,
								End,
							End,
						End;

	messageGroup_NLIST = VGroup,
						Child, NLISTVIEW_messagelistdata=NListviewObject,
							MUIA_NListview_NList,	NLIST_messagelistdata=NListObject,
								MUIA_NList_Title,			TRUE,
								MUIA_NList_MultiSelect,		MUIV_NList_MultiSelect_Default,
								MUIA_NList_DragType,		MUIV_NList_DragType_None,
								MUIA_NList_DisplayHook,		&DisplayHook_messagelistdata,
								MUIA_NList_CompareHook,		&CompareHook_messagelistdata,
								MUIA_NList_DestructHook,	&DestructHook_messagelistdata,
								MUIA_NList_Format,			",,,,,",
								MUIA_NList_AutoVisible,		TRUE,
								MUIA_NList_MinColSortable,	0,
								End,
							End,
						End;

	messageGroup_LISTTREE = VGroup,
						Child, LISTTREEVIEW_messagelistdata=NListviewObject,
							MUIA_NListview_NList,				LISTTREE_messagelistdata=NListtreeObject,
								InputListFrame,
								MUIA_NListtree_Title,			TRUE,
								MUIA_NListtree_MultiSelect,		MUIV_NListtree_MultiSelect_Default,
								MUIA_NList_DragType,			MUIV_NList_DragType_None,
								MUIA_NListtree_EmptyNodes,		TRUE,
								MUIA_NListtree_DisplayHook,		&DisplayHook_messagelistdatatree,
								MUIA_NListtree_DoubleClick,		MUIV_NListtree_DoubleClick_Off,
								MUIA_NListtree_DragDropSort,	FALSE,
								MUIA_NListtree_Format,			",,,,,",
								MUIA_NListtree_AutoVisible,		MUIV_NListtree_AutoVisible_FirstOpen,
								MUIA_NListtree_OpenHook,		&OpenHook_messagelistdata,
								MUIA_NList_MinColSortable,		0,
								End,
							End,
						End;
	if(grouplistGroup_LISTTREE==0 || messageGroup_LISTTREE==0)
		nlisttree = FALSE;
	else
		nlisttree = TRUE;

	if (unread_mail_icon)
	{
		DoMethod((Object*)NLIST_messagelistdata, MUIM_NList_UseImage, unread_mail_icon, 1, 0L);
		DoMethod((Object*)LISTTREEVIEW_messagelistdata, MUIM_NList_UseImage, unread_mail_icon, 1, 0L);
		DoMethod((Object*)NLIST_joinmsgs_MSGS, MUIM_NList_UseImage, unread_mail_icon, 1, 0L);
		DoMethod((Object*)NLIST_search_RES, MUIM_NList_UseImage, unread_mail_icon, 1, 0L);
	}
	if (read_mail_icon)
	{
		DoMethod((Object*)NLIST_messagelistdata, MUIM_NList_UseImage, read_mail_icon, 2, 0L);
		DoMethod((Object*)LISTTREEVIEW_messagelistdata, MUIM_NList_UseImage, read_mail_icon, 2, 0L);
		DoMethod((Object*)NLIST_joinmsgs_MSGS, MUIM_NList_UseImage, read_mail_icon, 2, 0L);
		DoMethod((Object*)NLIST_search_RES, MUIM_NList_UseImage, read_mail_icon, 2, 0L);
	}
	if (new_mail_icon)
	{
		DoMethod((Object*)NLIST_messagelistdata, MUIM_NList_UseImage, new_mail_icon, 3, 0L);
		DoMethod((Object*)LISTTREEVIEW_messagelistdata, MUIM_NList_UseImage, new_mail_icon, 3, 0L);
		DoMethod((Object*)NLIST_joinmsgs_MSGS, MUIM_NList_UseImage, new_mail_icon, 3, 0L);
		DoMethod((Object*)NLIST_search_RES, MUIM_NList_UseImage, new_mail_icon, 3, 0L);
	}

	if (hold_mail_icon)
	{
		DoMethod((Object*)NLIST_messagelistdata, MUIM_NList_UseImage, hold_mail_icon, 4, 0L);
		DoMethod((Object*)LISTTREEVIEW_messagelistdata, MUIM_NList_UseImage, hold_mail_icon, 4, 0L);
		DoMethod((Object*)NLIST_joinmsgs_MSGS, MUIM_NList_UseImage, hold_mail_icon, 4, 0L);
		DoMethod((Object*)NLIST_search_RES, MUIM_NList_UseImage, hold_mail_icon, 4, 0L);
	}

	if (flagred_icon)
	{
		DoMethod((Object*)NLIST_messagelistdata, MUIM_NList_UseImage, flagred_icon, 5, 0L);
		DoMethod((Object*)LISTTREEVIEW_messagelistdata, MUIM_NList_UseImage, flagred_icon, 5, 0L);
		DoMethod((Object*)NLIST_joinmsgs_MSGS, MUIM_NList_UseImage, flagred_icon, 5, 0L);
		DoMethod((Object*)NLIST_search_RES, MUIM_NList_UseImage, flagred_icon, 5, 0L);
	}

	if (flaggreen_icon)
	{
		DoMethod((Object*)NLIST_messagelistdata, MUIM_NList_UseImage, flaggreen_icon, 6, 0L);
		DoMethod((Object*)LISTTREEVIEW_messagelistdata, MUIM_NList_UseImage, flaggreen_icon, 6, 0L);
		DoMethod((Object*)NLIST_joinmsgs_MSGS, MUIM_NList_UseImage, flaggreen_icon, 6, 0L);
		DoMethod((Object*)NLIST_search_RES, MUIM_NList_UseImage, flaggreen_icon, 6, 0L);
	}

	if (flagyellow_icon)
	{
		DoMethod((Object*)NLIST_messagelistdata, MUIM_NList_UseImage, flagyellow_icon, 7, 0L);
		DoMethod((Object*)LISTTREEVIEW_messagelistdata, MUIM_NList_UseImage, flagyellow_icon, 7, 0L);
		DoMethod((Object*)NLIST_joinmsgs_MSGS, MUIM_NList_UseImage, flagyellow_icon, 7, 0L);
		DoMethod((Object*)NLIST_search_RES, MUIM_NList_UseImage, flagyellow_icon, 7, 0L);
	}

	if (replied_mail_icon)
	{
		DoMethod((Object*)NLIST_messagelistdata, MUIM_NList_UseImage, replied_mail_icon, 8, 0L);
		DoMethod((Object*)LISTTREEVIEW_messagelistdata, MUIM_NList_UseImage, replied_mail_icon, 8, 0L);
		DoMethod((Object*)NLIST_joinmsgs_MSGS, MUIM_NList_UseImage, replied_mail_icon, 8, 0L);
		DoMethod((Object*)NLIST_search_RES, MUIM_NList_UseImage, replied_mail_icon, 8, 0L);
	}

	if (onserver_mail_icon)
	{
		DoMethod((Object*)NLIST_messagelistdata, MUIM_NList_UseImage, onserver_mail_icon, 9, 0L);
		DoMethod((Object*)LISTTREEVIEW_messagelistdata, MUIM_NList_UseImage, onserver_mail_icon, 9, 0L);
		DoMethod((Object*)NLIST_joinmsgs_MSGS, MUIM_NList_UseImage, onserver_mail_icon, 9, 0L);
		DoMethod((Object*)NLIST_search_RES, MUIM_NList_UseImage, onserver_mail_icon, 9, 0L);
	}

	if (outgoing_icon)
	{
		DoMethod((Object*)NLIST_messagelistdata, MUIM_NList_UseImage, outgoing_icon, 10, 0L);
		DoMethod((Object*)LISTTREEVIEW_messagelistdata, MUIM_NList_UseImage, outgoing_icon, 10, 0L);
		DoMethod((Object*)NLIST_joinmsgs_MSGS, MUIM_NList_UseImage, outgoing_icon, 10, 0L);
		DoMethod((Object*)NLIST_search_RES, MUIM_NList_UseImage, outgoing_icon, 10, 0L);
		DoMethod((Object*)NLIST_groupdata, MUIM_NList_UseImage, outgoing_icon, 10, 0L);
		DoMethod((Object*)LISTTREE_groupdata, MUIM_NList_UseImage, outgoing_icon, 10, 0L);
	}
	if (sent_icon)
	{
		DoMethod((Object*)NLIST_messagelistdata, MUIM_NList_UseImage, sent_icon, 11, 0L);
		DoMethod((Object*)LISTTREEVIEW_messagelistdata, MUIM_NList_UseImage, sent_icon, 11, 0L);
		DoMethod((Object*)NLIST_joinmsgs_MSGS, MUIM_NList_UseImage, sent_icon, 11, 0L);
		DoMethod((Object*)NLIST_search_RES, MUIM_NList_UseImage, sent_icon, 11, 0L);
		DoMethod((Object*)NLIST_groupdata, MUIM_NList_UseImage, outgoing_icon, 11, 0L);
		DoMethod((Object*)LISTTREE_groupdata, MUIM_NList_UseImage, outgoing_icon, 11, 0L);
	}
	if (deleted_icon)
	{
		DoMethod((Object*)NLIST_messagelistdata, MUIM_NList_UseImage, deleted_icon, 12, 0L);
		DoMethod((Object*)LISTTREEVIEW_messagelistdata, MUIM_NList_UseImage, deleted_icon, 12, 0L);
		DoMethod((Object*)NLIST_joinmsgs_MSGS, MUIM_NList_UseImage, deleted_icon, 12, 0L);
		DoMethod((Object*)NLIST_search_RES, MUIM_NList_UseImage, deleted_icon, 12, 0L);
		DoMethod((Object*)NLIST_groupdata, MUIM_NList_UseImage, outgoing_icon, 12, 0L);
		DoMethod((Object*)LISTTREE_groupdata, MUIM_NList_UseImage, outgoing_icon, 12, 0L);
	}

	menuitem_GETNEWS=(Object *)DoMethod(menustrip,MUIM_FindUData,MEN_GETNEWS);
	menuitem_GETNEWSSINGLE=(Object *)DoMethod(menustrip,MUIM_FindUData,MEN_GETNEWSSINGLE);
	menuitem_SENDNEWS=(Object *)DoMethod(menustrip,MUIM_FindUData,MEN_SENDNEWS);
	menuitem_GETGROUPSDEF=(Object *)DoMethod(menustrip,MUIM_FindUData,MEN_GETGROUPSDEF);
	menuitem_GETNEWGROUPSDEF=(Object *)DoMethod(menustrip,MUIM_FindUData,MEN_GETNEWGROUPSDEF);
	menuitem_DISCONNECT=(Object *)DoMethod(menustrip,MUIM_FindUData,MEN_DISCONNECT);
	menuitem_VIEWGROUPS=(Object *)DoMethod(menustrip,MUIM_FindUData,MEN_VIEWGROUPS);
	menuitem_VIEWMESSAGES=(Object *)DoMethod(menustrip,MUIM_FindUData,MEN_VIEWMESSAGES);
	menuitem_UPDATEGROUPS=(Object *)DoMethod(menustrip,MUIM_FindUData,MEN_UPDATEGROUPS);
	menuitem_UPDATEINDEX=(Object *)DoMethod(menustrip,MUIM_FindUData,MEN_UPDATEINDEX);
	menuitem_UPDATEALLIND=(Object *)DoMethod(menustrip,MUIM_FindUData,MEN_UPDATEALLIND);
	menuitem_IMPORT=(Object *)DoMethod(menustrip,MUIM_FindUData,MEN_IMPORT);
	menuitem_READ=(Object *)DoMethod(menustrip,MUIM_FindUData,MEN_READ);
	menuitem_EDIT=(Object *)DoMethod(menustrip,MUIM_FindUData,MEN_EDIT);
	menuitem_SUPER=(Object *)DoMethod(menustrip,MUIM_FindUData,MEN_SUPER);
	menuitem_CANCEL=(Object *)DoMethod(menustrip,MUIM_FindUData,MEN_CANCEL);
	menuitem_DELMESS=(Object *)DoMethod(menustrip,MUIM_FindUData,MEN_DELMESS);
	menuitem_UNDELMESS=(Object *)DoMethod(menustrip,MUIM_FindUData,MEN_UNDELMESS);
	menuitem_EXP=(Object *)DoMethod(menustrip,MUIM_FindUData,MEN_EXP);
	menuitem_MOVE=(Object *)DoMethod(menustrip,MUIM_FindUData,MEN_MOVE);
	menuitem_COPY=(Object *)DoMethod(menustrip,MUIM_FindUData,MEN_COPY);
	menuitem_KILLMESS=(Object *)DoMethod(menustrip,MUIM_FindUData,MEN_KILLMESS);
	menuitem_YAMADDRESS=(Object *)DoMethod(menustrip,MUIM_FindUData,MEN_YAMADDRESS);
	menuitem_POST=(Object *)DoMethod(menustrip,MUIM_FindUData,MEN_POST);
	menuitem_FOLLOWUP=(Object *)DoMethod(menustrip,MUIM_FindUData,MEN_FOLLOWUP);
	menuitem_REPLY=(Object *)DoMethod(menustrip,MUIM_FindUData,MEN_REPLY);
	menuitem_BOTH=(Object *)DoMethod(menustrip,MUIM_FindUData,MEN_BOTH);
	menuitem_PDTHIS=(Object *)DoMethod(menustrip,MUIM_FindUData,MEN_PDTHIS);
	menuitem_MARK=(Object *)DoMethod(menustrip,MUIM_FindUData,MEN_MARK);
	menuitem_CURRENTDATE=(Object *)DoMethod(menustrip,MUIM_FindUData,MEN_CURRENTDATE);
	menuitem_USERS=(Object *)DoMethod(menustrip,MUIM_FindUData,MEN_USERS);
	//menuitem_=(Object *)DoMethod(menustrip,MUIM_FindUData,MEN_);

	// Help System
	set(BT_nng_OKAY,MUIA_ShortHelp,CatalogStr(MSG_CREATE_GROUP_HELP,MSG_CREATE_GROUP_HELP_STR));
	set(BT_nng_CANCEL,MUIA_ShortHelp,CatalogStr(MSG_CANCEL_CREATE_GROUP_HELP,MSG_CANCEL_CREATE_GROUP_HELP_STR));
	set(STR_nng_NAME,MUIA_ShortHelp,CatalogStr(MSG_GROUP_NAME_HELP,MSG_GROUP_NAME_HELP_STR));
	set(STR_nng_DESC,MUIA_ShortHelp,CatalogStr(MSG_GROUP_DESCRIPTION_HELP,MSG_GROUP_DESCRIPTION_HELP_STR));
	set(CY_nng_DEFSIG,MUIA_ShortHelp,CatalogStr(MSG_DEFAULT_SIGNATURE_HELP,MSG_DEFAULT_SIGNATURE_HELP_STR));
	set(CY_nng_MAXDL,MUIA_ShortHelp,CatalogStr(MSG_MAXIMUM_MESSAGES_TO_DOWNLOAD_HELP,MSG_MAXIMUM_MESSAGES_TO_DOWNLOAD_HELP_STR));
	set(CY_nng_MAXL,MUIA_ShortHelp,CatalogStr(MSG_MAXIMUM_LINES_TO_DOWNLOAD_HELP,MSG_MAXIMUM_LINES_TO_DOWNLOAD_HELP_STR));
	set(CY_nng_OFFLINE,MUIA_ShortHelp,CatalogStr(MSG_OFFLINE_VIEWING_HELP,MSG_OFFLINE_VIEWING_HELP_STR));
	set(CM_nng_S,MUIA_ShortHelp,CatalogStr(MSG_ONLY_SUBSCRIBED_GROUPS_WILL_BE_DOWNLOADED_HELP,MSG_ONLY_SUBSCRIBED_GROUPS_WILL_BE_DOWNLOADED_HELP_STR));
	set(BT_eng_OKAY,MUIA_ShortHelp,CatalogStr(MSG_UPDATE_GROUP_HELP,MSG_UPDATE_GROUP_HELP_STR));
	set(BT_eng_CANCEL,MUIA_ShortHelp,CatalogStr(MSG_CANCEL_UPDATE_GROUP_HELP,MSG_CANCEL_UPDATE_GROUP_HELP_STR));
	set(STR_eng_NAME,MUIA_ShortHelp,CatalogStr(MSG_GROUP_NAME_TO_SUBSCRIBE_HELP,MSG_GROUP_NAME_TO_SUBSCRIBE_HELP_STR));
	set(STR_eng_DESC,MUIA_ShortHelp,CatalogStr(MSG_GROUP_DESCRIPTION_HELP,MSG_GROUP_DESCRIPTION_HELP_STR));
	set(CY_eng_DEFSIG,MUIA_ShortHelp,CatalogStr(MSG_DEFAULT_SIGNATURE_HELP,MSG_DEFAULT_SIGNATURE_HELP_STR));
	set(CY_eng_MAXDL,MUIA_ShortHelp,CatalogStr(MSG_MAXIMUM_MESSAGES_TO_DOWNLOAD_HELP,MSG_MAXIMUM_MESSAGES_TO_DOWNLOAD_HELP_STR));
	set(CY_eng_MAXL,MUIA_ShortHelp,CatalogStr(MSG_MAXIMUM_LINES_TO_DOWNLOAD_HELP,MSG_MAXIMUM_LINES_TO_DOWNLOAD_HELP_STR));
	set(TXT_eng_LASTDL,MUIA_ShortHelp,CatalogStr(MSG_LAST_DOWNLOAD_HELP,MSG_LAST_DOWNLOAD_HELP_STR));
	set(CM_eng_S,MUIA_ShortHelp,CatalogStr(MSG_ONLY_SUBSCRIBED_GROUPS_WILL_BE_DOWNLOADED_HELP,MSG_ONLY_SUBSCRIBED_GROUPS_WILL_BE_DOWNLOADED_HELP_STR));
	set(BT_acc_OKAY,MUIA_ShortHelp,CatalogStr(MSG_KEEP_CHANGES_HELP,MSG_KEEP_CHANGES_HELP_STR));
	set(BT_acc_CANCEL,MUIA_ShortHelp,CatalogStr(MSG_CANCEL_CHANGES_HELP,MSG_CANCEL_CHANGES_HELP_STR));
	set(STR_acc_NAME,MUIA_ShortHelp,CatalogStr(MSG_YOUR_NAME_HELP,MSG_YOUR_NAME_HELP_STR));
	set(STR_acc_EMAIL,MUIA_ShortHelp,CatalogStr(MSG_YOUR_EMAIL_HELP,MSG_YOUR_EMAIL_HELP_STR));
	set(STR_acc_REALEMAIL,MUIA_ShortHelp,CatalogStr(MSG_YOUR_REAL_EMAIL_HELP,MSG_YOUR_REAL_EMAIL_HELP_STR));
	set(STR_acc_ORG,MUIA_ShortHelp,CatalogStr(MSG_ORGANISATION_HELP,MSG_ORGANISATION_HELP_STR));
	set(STR_acc_NNTP,MUIA_ShortHelp,CatalogStr(MSG_SERVER_ADDRESS_HELP,MSG_SERVER_ADDRESS_HELP_STR));
	set(STR_acc_PORT,MUIA_ShortHelp,CatalogStr(MSG_PORT_HELP,MSG_PORT_HELP_STR));
	set(STR_acc_NNTPPOST,MUIA_ShortHelp,CatalogStr(MSG_POSTING_SERVER_HELP,MSG_POSTING_SERVER_HELP_STR));
	set(STR_acc_SMTP,MUIA_ShortHelp,CatalogStr(MSG_SMTP_SERVER_HELP,MSG_SMTP_SERVER_HELP_STR));
	set(STR_acc_DOMAIN,MUIA_ShortHelp,CatalogStr(MSG_SMTP_DOMAIN_HELP,MSG_SMTP_DOMAIN_HELP_STR));
	set(STR_acc_FOLLOWUPTEXT,MUIA_ShortHelp,CatalogStr(MSG_FOLLOWUP_TEXT_HELP,MSG_FOLLOWUP_TEXT_HELP_STR));
	set(STR_acc_CHARSETWRITE,MUIA_ShortHelp,CatalogStr(MSG_SELECT_CHARSET_HELP,MSG_SELECT_CHARSET_HELP_STR));
	set(CY_acc_TIMEZONE,MUIA_ShortHelp,CatalogStr(MSG_SET_TIME_ZONE_HELP,MSG_SET_TIME_ZONE_HELP_STR));
	set(CM_acc_USELOCALETZ,MUIA_ShortHelp,CatalogStr(MSG_USE_LOCALE_HELP,MSG_USE_LOCALE_HELP_STR));
	set(CM_acc_BST,MUIA_ShortHelp,CatalogStr(MSG_DST_HELP,MSG_DST_HELP_STR));
	set(CY_acc_DATEFORMAT,MUIA_ShortHelp,CatalogStr(MSG_DATE_FORMAT_HELP,MSG_DATE_FORMAT_HELP_STR));
	set(CM_acc_LOGGING,MUIA_ShortHelp,CatalogStr(MSG_WRITE_LOG_HELP,MSG_WRITE_LOG_HELP_STR));
	set(CM_acc_LOGDEL,MUIA_ShortHelp,CatalogStr(MSG_DELETE_LOG_HELP,MSG_DELETE_LOG_HELP_STR));
	set(CM_acc_VGROUPDL,MUIA_ShortHelp,CatalogStr(MSG_SWITCH_GROUP_HELP,MSG_SWITCH_GROUP_HELP_STR));
	set(CM_acc_DELONLINE,MUIA_ShortHelp,CatalogStr(MSG_MODE_HELP,MSG_MODE_HELP_STR));
	set(CM_acc_CONFIRMQUIT,MUIA_ShortHelp,CatalogStr(MSG_CONFIRM_QUIT_HELP,MSG_CONFIRM_QUIT_HELP_STR));
	set(CM_acc_QUIETDL,MUIA_ShortHelp,CatalogStr(MSG_SHOW_FROM_HELP,MSG_SHOW_FROM_HELP_STR));
	set(CM_acc_NOCONFIRMDEL,MUIA_ShortHelp,CatalogStr(MSG_CONFIRM_DELETE_HELP,MSG_CONFIRM_DELETE_HELP_STR));
	set(CM_acc_CHECKFORDUPS,MUIA_ShortHelp,CatalogStr(MSG_DUPE_CHECK_HELP,MSG_DUPE_CHECK_HELP_STR));
	set(CY_acc_XNO,MUIA_ShortHelp,CatalogStr(MSG_X_NO_ARCHIVE_HELP,MSG_X_NO_ARCHIVE_HELP_STR));
	set(CM_acc_XNEWS,MUIA_ShortHelp,CatalogStr(MSG_SHORT_X_NEWSREADER_HELP,MSG_SHORT_X_NEWSREADER_HELP_STR));
	set(CY_acc_SIG,MUIA_ShortHelp,CatalogStr(MSG_SELECT_SIGNATURE_HELP,MSG_SELECT_SIGNATURE_HELP_STR));
	//set(CM_acc_USESIG,MUIA_ShortHelp,"Tick this box so that your\nsignature will appear by\ndefault when posting messages\n(which one depends on the\nNewsgroup parameters)");
	set(BT_acc_READSIG,MUIA_ShortHelp,CatalogStr(MSG_GET_SIGNATURE_FROM_FILE_HELP,MSG_GET_SIGNATURE_FROM_FILE_HELP_STR));
	set(ED_acc_SIG,MUIA_ShortHelp,CatalogStr(MSG_TYPE_SIGNATURE_HERE_HELP,MSG_TYPE_SIGNATURE_HERE_HELP_STR));
	set(STR_acc_LINELENGTH,MUIA_ShortHelp,CatalogStr(MSG_LINE_LENGTH_HELP,MSG_LINE_LENGTH_HELP_STR));
	set(CM_acc_REWRAP,MUIA_ShortHelp,CatalogStr(MSG_AUTO_REWRAP_HELP,MSG_AUTO_REWRAP_HELP_STR));
	set(CM_acc_SNIPSIG,MUIA_ShortHelp,CatalogStr(MSG_STRIP_SIGNATURE_HELP,MSG_STRIP_SIGNATURE_HELP_STR));
	set(CM_acc_LISTFLAGS,MUIA_ShortHelp,CatalogStr(MSG_DISPLAY_FLAGS_HELP,MSG_DISPLAY_FLAGS_HELP_STR));
	set(CM_acc_LISTFROMGROUP,MUIA_ShortHelp,CatalogStr(MSG_DISPLAY_FROM_HELP,MSG_DISPLAY_FROM_HELP_STR));
	set(CM_acc_LISTDATE,MUIA_ShortHelp,CatalogStr(MSG_DISPLAY_DATE_HELP,MSG_DISPLAY_DATE_HELP_STR));
	set(CM_acc_LISTSUBJECT,MUIA_ShortHelp,CatalogStr(MSG_DISPLAY_SUBJECT_HELP,MSG_DISPLAY_SUBJECT_HELP_STR));
	set(CM_acc_LISTSIZE,MUIA_ShortHelp,CatalogStr(MSG_DISPLAY_SIZE_HELP,MSG_DISPLAY_SIZE_HELP_STR));
	set(CM_acc_LISTLINES,MUIA_ShortHelp,CatalogStr(MSG_DISPLAY_LINES,MSG_DISPLAY_LINES_STR));
	set(BT_acc_NEWKILLADD,MUIA_ShortHelp,CatalogStr(MSG_ADD_KILFILE_HELP,MSG_ADD_KILFILE_HELP_STR));
	set(BT_acc_NEWKILL,MUIA_ShortHelp,CatalogStr(MSG_NEW_KILLFILE_HELP,MSG_NEW_KILLFILE_HELP_STR));
	set(BT_acc_DELKILL,MUIA_ShortHelp,CatalogStr(MSG_DELETE_KILLFILE_HELP,MSG_DELETE_KILLFILE_HELP_STR));
	set(BT_acc_DUPKILL,MUIA_ShortHelp,CatalogStr(MSG_DUPLICATE_KILLFILE_HELP,MSG_DUPLICATE_KILLFILE_HELP_STR));
	set(STR_acc_NEWKILLHEAD,MUIA_ShortHelp,CatalogStr(MSG_KILL_HEADER_HELP,MSG_KILL_HEADER_HELP_STR));
	set(STR_acc_NEWKILLTEXT,MUIA_ShortHelp,CatalogStr(MSG_KILL_TEXT_HELP,MSG_KILL_TEXT_HELP_STR));
	set(STR_acc_NEWKILLGROUP,MUIA_ShortHelp,CatalogStr(MSG_KILL_GROUP_HELP,MSG_KILL_GROUP_HELP_STR));
	set(CY_acc_EXPIREKILL,MUIA_ShortHelp,CatalogStr(MSG_KILLFILE_EXPIRE_HELP,MSG_KILLFILE_EXPIRE_HELP_STR));
	set(STR_acc_EXPIREKILL,MUIA_ShortHelp,CatalogStr(MSG_DAYS_LAST_FOR_HELP,MSG_DAYS_LAST_FOR_HELP_STR));
	//set(CY_acc_TYPEKILL,MUIA_ShortHelp,CatalogStr(MSG_WHAT_TO_DO_IF_MATCHED_HELP,MSG_WHAT_TO_DO_IF_MATCHED_HELP_STR));
	set(CY_acc_MATCHKILL,MUIA_ShortHelp,CatalogStr(MSG_SORT_OF_MATCH_HELP,MSG_SORT_OF_MATCH_HELP_STR));
	set(CY_acc_ACTIONKILL,MUIA_ShortHelp,CatalogStr(MSG_WHAT_TO_DO_IF_KILLFILE_MATCHED_HELP,MSG_WHAT_TO_DO_IF_KILLFILE_MATCHED_HELP_STR));
	set(CM_acc_SKIPKILL,MUIA_ShortHelp,CatalogStr(MSG_CHECK_REST_HELP,MSG_CHECK_REST_HELP_STR));
	set(CM_acc_SIZEKILL,MUIA_ShortHelp,CatalogStr(MSG_ADVERT_KILLFILE,MSG_ADVERT_KILLFILE_STR));
	set(CM_acc_MULTIPLEVWS,MUIA_ShortHelp,CatalogStr(MSG_OPEN_NEW_VIEW_WINDOW_HELP,MSG_OPEN_NEW_VIEW_WINDOW_HELP_STR));
	set(STR_acc_MIME,MUIA_ShortHelp,CatalogStr(MSG_SELECT_MIME_TYPE_HELP,MSG_SELECT_MIME_TYPE_HELP_STR));
	set(STR_acc_MIMEVIEW,MUIA_ShortHelp,CatalogStr(MSG_SELECT_VIEWER_HELP,MSG_SELECT_VIEWER_HELP_STR));
	set(STR_acc_MIMEDEF,MUIA_ShortHelp,CatalogStr(MSG_SELECT_DEFAULT_VIEWER_HELP,MSG_SELECT_DEFAULT_VIEWER_HELP_STR));
	set(BT_acc_MIMENEW,MUIA_ShortHelp,CatalogStr(MSG_CREATE_NEW_MIME_TYPE_HELP,MSG_CREATE_NEW_MIME_TYPE_HELP_STR));
	set(BT_acc_MIMEDEL,MUIA_ShortHelp,CatalogStr(MSG_DELETE_MIME_TYPE_HELP,MSG_DELETE_MIME_TYPE_HELP_STR));

	set(BT_groupman_SUB,MUIA_ShortHelp,CatalogStr(MSG_NEW_FOLDER_HELP,MSG_NEW_FOLDER_HELP_STR));
	set(BT_groupman_GET,MUIA_ShortHelp,CatalogStr(MSG_DOWNLOAD_GROUP_LIST_HELP,MSG_DOWNLOAD_GROUP_LIST_HELP_STR));
	set(STR_groupman_FIND,MUIA_ShortHelp,CatalogStr(MSG_SEARCH_GROUP_HELP,MSG_SEARCH_GROUP_HELP_STR));
	set(CY_search_WHERE,MUIA_ShortHelp,CatalogStr(MSG_SEARCH_HEADER_HELP,MSG_SEARCH_HEADER_HELP_STR));
	set(STR_search_WHAT,MUIA_ShortHelp,CatalogStr(MSG_SEARCH_STRING_HELP,MSG_SEARCH_STRING_HELP_STR));
	set(CM_search_CASESENS,MUIA_ShortHelp,CatalogStr(MSG_CASE_SENSITIVE_SEARCH_HELP,MSG_CASE_SENSITIVE_SEARCH_HELP_STR));
	set(CY_stats_WHAT,MUIA_ShortHelp,CatalogStr(MSG_STATISTICS_CRITERIA_HELP,MSG_STATISTICS_CRITERIA_HELP_STR));
	set(BT_newuser_OKAY,MUIA_ShortHelp,CatalogStr(MSG_ADD_THIS_USER_HELP,MSG_ADD_THIS_USER_HELP_STR));
	set(BT_newuser_CANCEL,MUIA_ShortHelp,CatalogStr(MSG_DO_NOT_ADD_USER,MSG_DO_NOT_ADD_USER_STR));
	set(STR_newuser_USER,MUIA_ShortHelp,CatalogStr(MSG_USERNAME_HELP,MSG_USERNAME_HELP_STR));
	set(STR_newuser_PASS,MUIA_ShortHelp,CatalogStr(MSG_PASSWORD_HELP,MSG_PASSWORD_HELP_STR));
	set(CM_newuser_PASS,MUIA_ShortHelp,CatalogStr(MSG_PASSWORD_PROTECTION_HELP,MSG_PASSWORD_PROTECTION_HELP_STR));
	set(STR_newuser_DATALOC,MUIA_ShortHelp,CatalogStr(MSG_NEWS_DIRECTORY_HELP,MSG_NEWS_DIRECTORY_HELP_STR));
	set(CM_newuser_SUP,MUIA_ShortHelp,CatalogStr(MSG_SUPERVISOR_HELP,MSG_SUPERVISOR_HELP_STR));
	set(CM_newuser_COPYPREFS,MUIA_ShortHelp,CatalogStr(MSG_COPY_PREFS_HELP,MSG_COPY_PREFS_HELP_STR));
	set(BT_users_NEW,MUIA_ShortHelp,CatalogStr(MSG_ADD_USER_HELP,MSG_ADD_USER_HELP_STR));
	set(BT_users_DELETE,MUIA_ShortHelp,CatalogStr(MSG_DELETE_USER_HELP,MSG_DELETE_USER_HELP_STR));
	set(STR_login_USER,MUIA_ShortHelp,CatalogStr(MSG_ENTER_USERNAME_HELP,MSG_ENTER_USERNAME_HELP_STR));
	set(STR_login_PASS,MUIA_ShortHelp,CatalogStr(MSG_ENTER_PASSWORD_HELP,MSG_ENTER_PASSWORD_HELP_STR));
	set(STR_newuser_PASS,MUIA_ShortHelp,CatalogStr(MSG_NEW_PASSWORD_HELP,MSG_NEW_PASSWORD_HELP_STR));
	set(CM_newuser_PASS,MUIA_ShortHelp,CatalogStr(MSG_PASSWORD_PROTECTION_HELP,MSG_PASSWORD_PROTECTION_HELP_STR));
	set(BT_cpwd_OKAY,MUIA_ShortHelp,CatalogStr(MSG_CHANGE_PASSWORD_HELP,MSG_CHANGE_PASSWORD_HELP_STR));
	set(BT_cpwd_CANCEL,MUIA_ShortHelp,(LONG)CatalogStr(MSG_CANCEL_CHANGES_2_HELP,MSG_CANCEL_CHANGES_2_HELP_STR));
	//set(,MUIA_ShortHelp,"");

	// must be before init_data() !
	DoMethod(CM_newuser_PASS,MUIM_Notify,MUIA_Selected,TRUE,
		STR_newuser_PASS,3,MUIM_Set,MUIA_Disabled,FALSE);
	DoMethod(CM_newuser_PASS ,MUIM_Notify,MUIA_Selected,FALSE,
		STR_newuser_PASS,3,MUIM_Set,MUIA_Disabled,TRUE);
	DoMethod(BT_newuser_OKAY,MUIM_Notify,MUIA_Pressed,FALSE,
		app,3,MUIM_CallHook,&hook_standard,callback_newuser_okay);
	DoMethod(BT_newuser_CANCEL,MUIM_Notify,MUIA_Pressed,FALSE,
		wnd_newuser,3,MUIM_Set,MUIA_Window_Open,FALSE);
	DoMethod(wnd_newuser,MUIM_Notify,MUIA_Window_CloseRequest,TRUE,
		wnd_newuser,3,MUIM_Set,MUIA_Window_Open,FALSE);

	DoMethod(STR_login_PASS,MUIM_Notify,MUIA_String_Acknowledge,MUIV_EveryTime,
		app,2,MUIM_Application_ReturnID,LOGIN_OKAY);
	DoMethod(BT_login_OKAY,MUIM_Notify,MUIA_Pressed,FALSE,
		app,2,MUIM_Application_ReturnID,LOGIN_OKAY);
	DoMethod(BT_login_CANCEL,MUIM_Notify,MUIA_Pressed,FALSE,
		app,2,MUIM_Application_ReturnID,LOGIN_CANCEL);
	DoMethod(wnd_login,MUIM_Notify,MUIA_Window_CloseRequest,TRUE,
		app,2,MUIM_Application_ReturnID,LOGIN_CANCEL);

	set(STR_newuser_PASS,MUIA_Disabled,TRUE);

	BOOL success=init_data();
	if(success)
	{

		if(account.grouplistType==0)
			DoMethod(grouplistGroup,OM_ADDMEMBER,grouplistGroup_NLIST);
		else
			DoMethod(grouplistGroup,OM_ADDMEMBER,grouplistGroup_LISTTREE);

		DoMethod(menuitem_VIEWGROUPS,MUIM_SetUData,MEN_VIEWGROUPSLIST,MUIA_Menuitem_Checked,account.grouplistType==0);
		DoMethod(menuitem_VIEWGROUPS,MUIM_SetUData,MEN_VIEWGROUPSTREE,MUIA_Menuitem_Checked,account.grouplistType==1);

		if(account.mdata_view==0)
			DoMethod(messageGroup,OM_ADDMEMBER,messageGroup_NLIST);
		else
			DoMethod(messageGroup,OM_ADDMEMBER,messageGroup_LISTTREE);

		DoMethod(menuitem_VIEWMESSAGES,MUIM_SetUData,MEN_VIEWMESSAGESLIST,MUIA_Menuitem_Checked,account.mdata_view==0);
		DoMethod(menuitem_VIEWMESSAGES,MUIM_SetUData,MEN_VIEWMESSAGESTREE,MUIA_Menuitem_Checked,account.mdata_view==1);

		DoMethod(app,MUIM_Notify,MUIA_Application_DoubleStart,TRUE,
			app,3,MUIM_CallHook,&hook_standard,callback_app_doublestart);

		DoMethod(wnd_main,MUIM_Notify,MUIA_Window_CloseRequest,TRUE,
			app,4,MUIM_CallHook,&hook_standard,mainMenusFunc,MEN_QUIT);
		DoMethod(wnd_about,MUIM_Notify,MUIA_Window_CloseRequest,TRUE,
			wnd_about,3,MUIM_Set,MUIA_Window_Open,FALSE);

		DoMethod(NLIST_groupdata,MUIM_Notify,MUIA_NList_SelectChange,TRUE,
			app,3,MUIM_CallHook,&hook_standard,callback_ngchanged);
		DoMethod(NLIST_groupdata,MUIM_Notify,MUIA_NList_DoubleClick,MUIV_EveryTime,
			app,4,MUIM_CallHook,&hook_standard,mainMenusFunc,MEN_EDITNG);
		if(nlisttree)
		{
			DoMethod(LISTTREE_groupdata,MUIM_Notify,MUIA_NListtree_Active,MUIV_EveryTime,
				app,3,MUIM_CallHook,&hook_standard,callback_ngchanged);
			DoMethod(LISTTREE_groupdata,MUIM_Notify,MUIA_NListtree_DoubleClick,MUIV_EveryTime,
				app,4,MUIM_CallHook,&hook_standard,mainMenusFunc,MEN_EDITNG);
		}

		DoMethod(NLIST_messagelistdata,MUIM_Notify,MUIA_NList_DoubleClick,MUIV_EveryTime,
			app,4,MUIM_CallHook,&hook_standard,mainMenusFunc,MEN_READ);
		DoMethod(NLIST_messagelistdata,MUIM_Notify,MUIA_NList_TitleClick,MUIV_EveryTime,
			app,3,MUIM_CallHook,&hook_standard,callback_newsort);
		DoMethod(NLIST_messagelistdata,MUIM_Notify,MUIA_NList_SelectChange,TRUE,
			app,3,MUIM_CallHook,&hook_standard,mdata_changed);
		if(nlisttree)
		{
			DoMethod(LISTTREE_messagelistdata,MUIM_Notify,MUIA_NListtree_DoubleClick,MUIV_EveryTime,
				app,4,MUIM_CallHook,&hook_standard,mainMenusFunc,MEN_READ);
			DoMethod(LISTTREE_messagelistdata,MUIM_Notify,MUIA_NList_TitleClick,MUIV_EveryTime,
				app,3,MUIM_CallHook,&hook_standard,callback_newsort);
			DoMethod(LISTTREE_messagelistdata,MUIM_Notify,MUIA_NListtree_Active,MUIV_EveryTime,
				app,3,MUIM_CallHook,&hook_standard,mdata_changed);
		}

		DoMethod(wnd_main,MUIM_Notify,MUIA_Window_MenuAction,MUIV_EveryTime,
			app,4,MUIM_CallHook,&hook_standard,mainMenusFunc,MUIV_TriggerValue);

		DoMethod(TB_main,MUIM_TheBar_Notify,TB_READ,		MUIA_Pressed,FALSE,app,4,MUIM_CallHook,&hook_standard,mainMenusFunc,MEN_READ);
		DoMethod(TB_main,MUIM_TheBar_Notify,TB_EXPORT,		MUIA_Pressed,FALSE,app,4,MUIM_CallHook,&hook_standard,mainMenusFunc,MEN_EXP);
		DoMethod(TB_main,MUIM_TheBar_Notify,TB_MOVE,		MUIA_Pressed,FALSE,app,4,MUIM_CallHook,&hook_standard,mainMenusFunc,MEN_MOVE);
		DoMethod(TB_main,MUIM_TheBar_Notify,TB_COPY,		MUIA_Pressed,FALSE,app,4,MUIM_CallHook,&hook_standard,mainMenusFunc,MEN_COPY);
		DoMethod(TB_main,MUIM_TheBar_Notify,TB_DELETE,		MUIA_Pressed,FALSE,app,4,MUIM_CallHook,&hook_standard,mainMenusFunc,MEN_DELMESS);
		DoMethod(TB_main,MUIM_TheBar_Notify,TB_KILL,		MUIA_Pressed,FALSE,app,4,MUIM_CallHook,&hook_standard,mainMenusFunc,MEN_KILLMESS);
		DoMethod(TB_main,MUIM_TheBar_Notify,TB_POST,		MUIA_Pressed,FALSE,app,4,MUIM_CallHook,&hook_standard,mainMenusFunc,MEN_POST);
		DoMethod(TB_main,MUIM_TheBar_Notify,TB_FOLLOWUP,	MUIA_Pressed,FALSE,app,4,MUIM_CallHook,&hook_standard,mainMenusFunc,MEN_FOLLOWUP);
		DoMethod(TB_main,MUIM_TheBar_Notify,TB_EDIT,		MUIA_Pressed,FALSE,app,4,MUIM_CallHook,&hook_standard,mainMenusFunc,MEN_EDIT);
		DoMethod(TB_main,MUIM_TheBar_Notify,TB_GETNEWS,		MUIA_Pressed,FALSE,app,4,MUIM_CallHook,&hook_standard,mainMenusFunc,MEN_GETNEWS);
		DoMethod(TB_main,MUIM_TheBar_Notify,TB_SENDNEWS,	MUIA_Pressed,FALSE,app,4,MUIM_CallHook,&hook_standard,mainMenusFunc,MEN_SENDNEWS);
		DoMethod(TB_main,MUIM_TheBar_Notify,TB_SEARCH,		MUIA_Pressed,FALSE,app,4,MUIM_CallHook,&hook_standard,mainMenusFunc,MEN_SEARCH);
		DoMethod(TB_main,MUIM_TheBar_Notify,TB_PROGSET,		MUIA_Pressed,FALSE,app,4,MUIM_CallHook,&hook_standard,mainMenusFunc,MEN_ACCOUNT);
		DoMethod(TB_main,MUIM_TheBar_Notify,TB_KILLFILE,	MUIA_Pressed,FALSE,app,4,MUIM_CallHook,&hook_standard,mainMenusFunc,MEN_KILLFILE);

		DoMethod(BT_nng_OKAY,MUIM_Notify,MUIA_Pressed,FALSE,
			//app,2,MUIM_Application_ReturnID,NNG_OKAY);
			app,3,MUIM_CallHook,&hook_standard,callback_nng_okay);
		DoMethod(BT_nng_CANCEL,MUIM_Notify,MUIA_Pressed,FALSE,
			//app,2,MUIM_Application_ReturnID,NNG_CANCEL);
			app,3,MUIM_CallHook,&hook_standard,callback_nng_cancel);
		DoMethod(CY_nng_MAXDL,MUIM_Notify,MUIA_Cycle_Active,0,
			STR_nng_MAXDL,3,MUIM_Set,MUIA_Disabled,TRUE);
		DoMethod(CY_nng_MAXDL,MUIM_Notify,MUIA_Cycle_Active,1,
			STR_nng_MAXDL,3,MUIM_Set,MUIA_Disabled,FALSE);
		DoMethod(CY_nng_MAXL,MUIM_Notify,MUIA_Cycle_Active,0,
			STR_nng_MAXL,3,MUIM_Set,MUIA_Disabled,TRUE);
		DoMethod(CY_nng_MAXL,MUIM_Notify,MUIA_Cycle_Active,1,
			STR_nng_MAXL,3,MUIM_Set,MUIA_Disabled,FALSE);
		DoMethod(CY_nng_MAXAGE,MUIM_Notify,MUIA_Cycle_Active,0,
			STR_nng_MAXAGE,3,MUIM_Set,MUIA_Disabled,TRUE);
		DoMethod(CY_nng_MAXAGE,MUIM_Notify,MUIA_Cycle_Active,1,
			STR_nng_MAXAGE,3,MUIM_Set,MUIA_Disabled,FALSE);
		DoMethod(CY_nng_MAXAGE,MUIM_Notify,MUIA_Cycle_Active,2,
			STR_nng_MAXAGE,3,MUIM_Set,MUIA_Disabled,FALSE);

		DoMethod(BT_eng_OKAY,MUIM_Notify,MUIA_Pressed,FALSE,
			//app,2,MUIM_Application_ReturnID,ENG_OKAY);
			app,3,MUIM_CallHook,&hook_standard,callback_eng_okay);
		DoMethod(BT_eng_CANCEL,MUIM_Notify,MUIA_Pressed,FALSE,
			//app,2,MUIM_Application_ReturnID,ENG_CANCEL);
			app,3,MUIM_CallHook,&hook_standard,callback_eng_cancel);
		DoMethod(CY_eng_MAXDL ,MUIM_Notify,MUIA_Cycle_Active,0,
			STR_eng_MAXDL,3,MUIM_Set,MUIA_Disabled,TRUE);
		DoMethod(CY_eng_MAXDL ,MUIM_Notify,MUIA_Cycle_Active,1,
			STR_eng_MAXDL,3,MUIM_Set,MUIA_Disabled,FALSE);
		DoMethod(CY_eng_MAXL ,MUIM_Notify,MUIA_Cycle_Active,0,
			STR_eng_MAXL,3,MUIM_Set,MUIA_Disabled,TRUE);
		DoMethod(CY_eng_MAXL ,MUIM_Notify,MUIA_Cycle_Active,1,
			STR_eng_MAXL,3,MUIM_Set,MUIA_Disabled,FALSE);
		DoMethod(CY_eng_MAXAGE,MUIM_Notify,MUIA_Cycle_Active,0,
			STR_eng_MAXAGE,3,MUIM_Set,MUIA_Disabled,TRUE);
		DoMethod(CY_eng_MAXAGE,MUIM_Notify,MUIA_Cycle_Active,1,
			STR_eng_MAXAGE,3,MUIM_Set,MUIA_Disabled,FALSE);
		DoMethod(CY_eng_MAXAGE,MUIM_Notify,MUIA_Cycle_Active,2,
			STR_eng_MAXAGE,3,MUIM_Set,MUIA_Disabled,FALSE);

		DoMethod(BT_ngadv_OKAY,MUIM_Notify,MUIA_Pressed,FALSE,
			//app,2,MUIM_Application_ReturnID,NGADV_OKAY);
			app,3,MUIM_CallHook,&hook_standard,callback_ngadv_okay);
		DoMethod(BT_ngadv_CANCEL,MUIM_Notify,MUIA_Pressed,FALSE,
			//app,2,MUIM_Application_ReturnID,NGADV_CANCEL);
			app,3,MUIM_CallHook,&hook_standard,callback_ngadv_cancel);
		DoMethod(CM_ngadv_APPVHD,MUIM_Notify,MUIA_Selected,TRUE,
			STR_ngadv_APPVHD,3,MUIM_Set,MUIA_Disabled,FALSE);
		DoMethod(CM_ngadv_APPVHD,MUIM_Notify,MUIA_Selected,FALSE,
			STR_ngadv_APPVHD,3,MUIM_Set,MUIA_Disabled,TRUE);
		DoMethod(CM_ngadv_ALTEMAIL,MUIM_Notify,MUIA_Selected,TRUE,
			STR_ngadv_ALTEMAIL,3,MUIM_Set,MUIA_Disabled,FALSE);
		DoMethod(CM_ngadv_ALTEMAIL,MUIM_Notify,MUIA_Selected,FALSE,
			STR_ngadv_ALTEMAIL,3,MUIM_Set,MUIA_Disabled,TRUE);
		DoMethod(CM_ngadv_ALTNAME,MUIM_Notify,MUIA_Selected,TRUE,
			STR_ngadv_ALTNAME,3,MUIM_Set,MUIA_Disabled,FALSE);
		DoMethod(CM_ngadv_ALTNAME,MUIM_Notify,MUIA_Selected,FALSE,
			STR_ngadv_ALTNAME,3,MUIM_Set,MUIA_Disabled,TRUE);

		DoMethod(BT_joinmsgs_JOIN,MUIM_Notify,MUIA_Pressed,FALSE,
			//app,2,MUIM_Application_ReturnID,JOINMSGS_JOIN);
			app,3,MUIM_CallHook,&hook_standard,callback_joinmsgs_join);
		DoMethod(BT_joinmsgs_CANCEL,MUIM_Notify,MUIA_Pressed,FALSE,
			//app,2,MUIM_Application_ReturnID,JOINMSGS_CANCEL);
			app,3,MUIM_CallHook,&hook_standard,callback_joinmsgs_cancel);
		DoMethod(wnd_joinmsgs,MUIM_Notify,MUIA_Window_CloseRequest,TRUE,
			//app,2,MUIM_Application_ReturnID,JOINMSGS_CANCEL);
			app,3,MUIM_CallHook,&hook_standard,callback_joinmsgs_cancel);

		DoMethod(wnd_account,MUIM_Notify,MUIA_Window_CloseRequest,TRUE,
			//app,2,MUIM_Application_ReturnID,ACC_CANCEL);
			app,3,MUIM_CallHook,&hook_standard,callback_acc_cancel);
		DoMethod(BT_acc_OKAY,MUIM_Notify,MUIA_Pressed,FALSE,
			//app,2,MUIM_Application_ReturnID,ACC_OKAY);
			app,3,MUIM_CallHook,&hook_standard,callback_acc_okay);
		DoMethod(BT_acc_CANCEL,MUIM_Notify,MUIA_Pressed,FALSE,
			//app,2,MUIM_Application_ReturnID,ACC_CANCEL);
			app,3,MUIM_CallHook,&hook_standard,callback_acc_cancel);
		DoMethod(CY_acc_SIG,MUIM_Notify,MUIA_Cycle_Active,MUIV_EveryTime,
			//app,2,MUIM_Application_ReturnID,ACC_CYSIG);
			app,3,MUIM_CallHook,&hook_standard,callback_acc_cysig);
		DoMethod(CM_acc_USELOCALETZ,MUIM_Notify,MUIA_Selected,TRUE,
			CY_acc_TIMEZONE,3,MUIM_Set,MUIA_Disabled,TRUE);
		DoMethod(CM_acc_USELOCALETZ,MUIM_Notify,MUIA_Selected,FALSE,
			CY_acc_TIMEZONE,3,MUIM_Set,MUIA_Disabled,FALSE);

		DoMethod(CM_acc_AUTH,MUIM_Notify,MUIA_Selected,TRUE,
			STR_acc_USER,3,MUIM_Set,MUIA_Disabled,FALSE);
		DoMethod(CM_acc_AUTH,MUIM_Notify,MUIA_Selected,FALSE,
			STR_acc_USER,3,MUIM_Set,MUIA_Disabled,TRUE);
		DoMethod(CM_acc_AUTH,MUIM_Notify,MUIA_Selected,TRUE,
			STR_acc_PASS,3,MUIM_Set,MUIA_Disabled,FALSE);
		DoMethod(CM_acc_AUTH ,MUIM_Notify,MUIA_Selected,FALSE,
			STR_acc_PASS,3,MUIM_Set,MUIA_Disabled,TRUE);
		DoMethod(BT_acc_ADDSERVER,MUIM_Notify,MUIA_Pressed,FALSE,
			//app,2,MUIM_Application_ReturnID,ACC_ADDSERVER);
			app,3,MUIM_CallHook,&hook_standard,callback_acc_addserver);
		DoMethod(BT_acc_EDITSERVER,MUIM_Notify,MUIA_Pressed,FALSE,
			//app,2,MUIM_Application_ReturnID,ACC_EDITSERVER);
			app,3,MUIM_CallHook,&hook_standard,callback_acc_editserver);
		DoMethod(BT_acc_MAKEDEFSERVER,MUIM_Notify,MUIA_Pressed,FALSE,
			//app,2,MUIM_Application_ReturnID,ACC_MAKEDEFSERVER);
			app,3,MUIM_CallHook,&hook_standard,callback_acc_makedefserver);
		DoMethod(BT_acc_MAKEPOSTSERVER,MUIM_Notify,MUIA_Pressed,FALSE,
			//app,2,MUIM_Application_ReturnID,ACC_MAKEPOSTSERVER);
			app,3,MUIM_CallHook,&hook_standard,callback_acc_makepostserver);
		DoMethod(BT_acc_DELETESERVER,MUIM_Notify,MUIA_Pressed,FALSE,
			//app,2,MUIM_Application_ReturnID,ACC_DELETESERVER);
			app,3,MUIM_CallHook,&hook_standard,callback_acc_deleteserver);
		DoMethod(BT_acc_GETGROUPS,MUIM_Notify,MUIA_Pressed,FALSE,
			//app,2,MUIM_Application_ReturnID,ACC_GETGROUPS);
			app,4,MUIM_CallHook,&hook_standard,callback_getgroups,GETGROUPS_ACC);
		DoMethod(BT_acc_GETNEWGROUPS,MUIM_Notify,MUIA_Pressed,FALSE,
			//app,2,MUIM_Application_ReturnID,ACC_GETNEWGROUPS);
			app,4,MUIM_CallHook,&hook_standard,callback_getnewgroups,GETGROUPS_ACC);
		DoMethod(BT_acc_GROUPMAN,MUIM_Notify,MUIA_Pressed,FALSE,
			//app,2,MUIM_Application_ReturnID,ACC_GROUPMAN);
			app,4,MUIM_CallHook,&hook_standard,callback_groupman,TRUE);
		DoMethod(BT_server_OKAY,MUIM_Notify,MUIA_Pressed,FALSE,
			//app,2,MUIM_Application_ReturnID,SERVER_OKAY);
			app,3,MUIM_CallHook,&hook_standard,callback_server_okay);
		DoMethod(BT_server_CANCEL,MUIM_Notify,MUIA_Pressed,FALSE,
			//app,2,MUIM_Application_ReturnID,SERVER_CANCEL);
			app,3,MUIM_CallHook,&hook_standard,callback_server_cancel);

		DoMethod(CY_acc_READHEADER,MUIM_Notify,MUIA_Cycle_Active,0,
			STR_acc_READHEADER,3,MUIM_Set,MUIA_Disabled,TRUE);
		DoMethod(CY_acc_READHEADER,MUIM_Notify,MUIA_Cycle_Active,1,
			STR_acc_READHEADER,3,MUIM_Set,MUIA_Disabled,FALSE);
		DoMethod(CY_acc_READHEADER,MUIM_Notify,MUIA_Cycle_Active,2,
			STR_acc_READHEADER,3,MUIM_Set,MUIA_Disabled,TRUE);

		DoMethod(BT_acc_READSIG,MUIM_Notify,MUIA_Pressed,FALSE,
			//app,2,MUIM_Application_ReturnID,ACC_READSIG);
			app,3,MUIM_CallHook,&hook_standard,callback_acc_readsig);
		DoMethod(NLIST_acc_KILLFILE,MUIM_Notify,MUIA_NList_SelectChange,TRUE,
			//app,2,MUIM_Application_ReturnID,ACC_KILLCHANGED);
			app,3,MUIM_CallHook,&hook_standard,callback_acc_killchanged);
		DoMethod(BT_acc_NEWKILLADD,MUIM_Notify,MUIA_Pressed,FALSE,
			//app,2,MUIM_Application_ReturnID,ACC_STRKILL);
			app,4,MUIM_CallHook,&hook_standard,callback_acc_strkill,FALSE);
		DoMethod(STR_acc_NEWKILLHEAD,MUIM_Notify,MUIA_String_Acknowledge,MUIV_EveryTime,
			//app,2,MUIM_Application_ReturnID,ACC_STRKILL);
			app,4,MUIM_CallHook,&hook_standard,callback_acc_strkill,FALSE);
		DoMethod(STR_acc_NEWKILLTEXT,MUIM_Notify,MUIA_String_Acknowledge,MUIV_EveryTime,
			//app,2,MUIM_Application_ReturnID,ACC_STRKILL);
			app,4,MUIM_CallHook,&hook_standard,callback_acc_strkill,FALSE);
		DoMethod(STR_acc_NEWKILLGROUP,MUIM_Notify,MUIA_String_Acknowledge,MUIV_EveryTime,
			//app,2,MUIM_Application_ReturnID,ACC_STRKILL);
			app,4,MUIM_CallHook,&hook_standard,callback_acc_strkill,FALSE);
		DoMethod(STR_acc_EXPIREKILL,MUIM_Notify,MUIA_String_Acknowledge,MUIV_EveryTime,
			//app,2,MUIM_Application_ReturnID,ACC_STRKILL);
			app,4,MUIM_CallHook,&hook_standard,callback_acc_strkill,FALSE);
		DoMethod(CY_acc_EXPIREKILL,MUIM_Notify,MUIA_Cycle_Active,MUIV_EveryTime,
			//app,2,MUIM_Application_ReturnID,ACC_STRKILLTEMP);
			app,4,MUIM_CallHook,&hook_standard,callback_acc_strkill,TRUE);
		/*DoMethod(CY_acc_TYPEKILL,MUIM_Notify,MUIA_Cycle_Active,MUIV_EveryTime,
			app,2,MUIM_Application_ReturnID,ACC_STRKILLTEMP);*/
		DoMethod(CY_acc_MATCHKILL,MUIM_Notify,MUIA_Cycle_Active,MUIV_EveryTime,
			//app,2,MUIM_Application_ReturnID,ACC_STRKILLTEMP);
			app,4,MUIM_CallHook,&hook_standard,callback_acc_strkill,TRUE);
		DoMethod(CY_acc_ACTIONKILL,MUIM_Notify,MUIA_Cycle_Active,MUIV_EveryTime,
			//app,2,MUIM_Application_ReturnID,ACC_STRKILLTEMP);
			app,4,MUIM_CallHook,&hook_standard,callback_acc_strkill,TRUE);
		DoMethod(CM_acc_SKIPKILL,MUIM_Notify,MUIA_Selected,MUIV_EveryTime,
			//app,2,MUIM_Application_ReturnID,ACC_STRKILLTEMP);
			app,4,MUIM_CallHook,&hook_standard,callback_acc_strkill,TRUE);
		DoMethod(BT_acc_NEWKILL,MUIM_Notify,MUIA_Pressed,FALSE,
			//app,2,MUIM_Application_ReturnID,ACC_NEWKILL);
			app,3,MUIM_CallHook,&hook_standard,callback_acc_newkill);
		DoMethod(BT_acc_DELKILL,MUIM_Notify,MUIA_Pressed,FALSE,
			//app,2,MUIM_Application_ReturnID,ACC_DELKILL);
			app,3,MUIM_CallHook,&hook_standard,callback_acc_delkill);
		DoMethod(BT_acc_DUPKILL,MUIM_Notify,MUIA_Pressed,FALSE,
			//app,2,MUIM_Application_ReturnID,ACC_DUPKILL);
			app,3,MUIM_CallHook,&hook_standard,callback_acc_dupkill);
		DoMethod(CY_acc_EXPIREKILL,MUIM_Notify,MUIA_Cycle_Active,0,
			STR_acc_EXPIREKILL,3,MUIM_Set,MUIA_Disabled,TRUE);
		DoMethod(CY_acc_EXPIREKILL,MUIM_Notify,MUIA_Cycle_Active,1,
			STR_acc_EXPIREKILL,3,MUIM_Set,MUIA_Disabled,FALSE);
		DoMethod(CY_acc_EXPIREKILL,MUIM_Notify,MUIA_Cycle_Active,2,
			STR_acc_EXPIREKILL,3,MUIM_Set,MUIA_Disabled,FALSE);
		DoMethod(LIST_acc_CHARSETWRITE,MUIM_Notify,MUIA_Listview_DoubleClick,TRUE,
			POP_acc_CHARSETWRITE,2,MUIM_Popstring_Close,TRUE);
		DoMethod(LIST_acc_MIME,MUIM_Notify,MUIA_Listview_DoubleClick,TRUE,
			POP_acc_MIME,2,MUIM_Popstring_Close,TRUE);
		DoMethod(NLIST_acc_MIMEPREFS,MUIM_Notify,MUIA_NList_Active,MUIV_EveryTime,
			//app,2,MUIM_Application_ReturnID,ACC_MIMECLICK);
			app,3,MUIM_CallHook,&hook_standard,callback_acc_mimeclick);
		DoMethod(BT_acc_MIMENEW,MUIM_Notify,MUIA_Pressed,FALSE,
			//app,2,MUIM_Application_ReturnID,ACC_MIMENEW);
			app,3,MUIM_CallHook,&hook_standard,callback_acc_mimenew);
		DoMethod(BT_acc_MIMEDEL,MUIM_Notify,MUIA_Pressed,FALSE,
			//app,2,MUIM_Application_ReturnID,ACC_MIMEDEL);
			app,3,MUIM_CallHook,&hook_standard,callback_acc_mimedel);
		DoMethod(STR_acc_MIME,MUIM_Notify,MUIA_String_Acknowledge,MUIV_EveryTime,
			//app,2,MUIM_Application_ReturnID,ACC_MIMESTR);
			app,3,MUIM_CallHook,&hook_standard,callback_acc_mimestr);
		DoMethod(STR_acc_MIMEVIEW,MUIM_Notify,MUIA_String_Acknowledge,MUIV_EveryTime,
			//app,2,MUIM_Application_ReturnID,ACC_MIMESTR);
			app,3,MUIM_CallHook,&hook_standard,callback_acc_mimestr);
		DoMethod(pages_account,MUIM_Notify,MUIA_Group_ActivePage,MUIV_EveryTime,
			//app,2,MUIM_Application_ReturnID,ACC_PAGESIG);
			app,3,MUIM_CallHook,&hook_standard,callback_acc_pagesig);

		DoMethod(wnd_killfile,MUIM_Notify,MUIA_Window_CloseRequest,TRUE,
			//app,2,MUIM_Application_ReturnID,KILL_CLOSE);
			app,3,MUIM_CallHook,&hook_standard,callback_kill_close);

		DoMethod(STR_groupman_FIND,MUIM_Notify,MUIA_String_Acknowledge,MUIV_EveryTime,
			//app,2,MUIM_Application_ReturnID,GROUPMAN_FIND);
			app,3,MUIM_CallHook,&hook_standard,callback_groupman_find);
		DoMethod(NLIST_groupman,MUIM_Notify,MUIA_NList_DoubleClick,TRUE,
			//app,2,MUIM_Application_ReturnID,GROUPMAN_SUB);
			app,3,MUIM_CallHook,&hook_standard,callback_groupman_sub);
		DoMethod(BT_groupman_SUB,MUIM_Notify,MUIA_Pressed,FALSE,
			//app,2,MUIM_Application_ReturnID,GROUPMAN_SUB);
			app,3,MUIM_CallHook,&hook_standard,callback_groupman_sub);
		DoMethod(BT_groupman_GET,MUIM_Notify,MUIA_Pressed,FALSE,
			//app,2,MUIM_Application_ReturnID,GROUPMAN_GETGROUPS);
			app,4,MUIM_CallHook,&hook_standard,callback_getgroups,GETGROUPS_GROUPMAN);
		/*DoMethod(BT_groupman_WRITE,MUIM_Notify,MUIA_Pressed,FALSE,
			app,2,MUIM_Application_ReturnID,GROUPMAN_WRITE);
		DoMethod(BT_groupman_SORT,MUIM_Notify,MUIA_Pressed,FALSE,
			app,2,MUIM_Application_ReturnID,GROUPMAN_SORT);*/
		/*DoMethod(wnd_groupman,MUIM_Notify,MUIA_Window_CloseRequest,TRUE,
			wnd_groupman,3,MUIM_Set,MUIA_Window_Open,FALSE);
		DoMethod(BT_groupman_CLOSE,MUIM_Notify,MUIA_Pressed,FALSE,
			wnd_groupman,3,MUIM_Set,MUIA_Window_Open,FALSE);*/
		DoMethod(wnd_groupman,MUIM_Notify,MUIA_Window_CloseRequest,TRUE,
			//app,2,MUIM_Application_ReturnID,GROUPMAN_CLOSE);
			app,3,MUIM_CallHook,&hook_standard,callback_groupman_close);

		DoMethod(wnd_search,MUIM_Notify,MUIA_Window_CloseRequest,TRUE,
			wnd_search,3,MUIM_Set,MUIA_Window_Open,FALSE);
		DoMethod(BT_search_START,MUIM_Notify,MUIA_Pressed,FALSE,
			//app,2,MUIM_Application_ReturnID,SEARCH_START);
			app,3,MUIM_CallHook,&hook_standard,callback_search_start);
		DoMethod(NLIST_search_RES,MUIM_Notify,MUIA_NList_DoubleClick,MUIV_EveryTime,
			//app,2,MUIM_Application_ReturnID,SEARCH_MESSAGE);
			app,3,MUIM_CallHook,&hook_standard,callback_search_message);
		DoMethod(NLIST_search_RES,MUIM_Notify,MUIA_NList_TitleClick,MUIV_EveryTime,
			//app,2,MUIM_Application_ReturnID,SEARCH_SORT);
			app,3,MUIM_CallHook,&hook_standard,callback_search_sort);
		DoMethod(wnd_stats,MUIM_Notify,MUIA_Window_CloseRequest,TRUE,
			wnd_stats,3,MUIM_Set,MUIA_Window_Open,FALSE);
		DoMethod(BT_stats_START,MUIM_Notify,MUIA_Pressed,FALSE,
			//app,2,MUIM_Application_ReturnID,STATS_START);
			app,3,MUIM_CallHook,&hook_standard,callback_stats_start);
		DoMethod(NLIST_stats_RES,MUIM_Notify,MUIA_NList_TitleClick,MUIV_EveryTime,
			//app,2,MUIM_Application_ReturnID,STATS_SORT);
			app,3,MUIM_CallHook,&hook_standard,callback_stats_sort);

		DoMethod(wnd_users,MUIM_Notify,MUIA_Window_CloseRequest,TRUE,
			wnd_users,3,MUIM_Set,MUIA_Window_Open,FALSE);
		DoMethod(BT_users_NEW,MUIM_Notify,MUIA_Pressed,FALSE,
			//app,2,MUIM_Application_ReturnID,USERS_NEW);
			app,3,MUIM_CallHook,&hook_standard,callback_users_new);
		DoMethod(BT_users_DELETE,MUIM_Notify,MUIA_Pressed,FALSE,
			//app,2,MUIM_Application_ReturnID,USERS_DELETE);
			app,3,MUIM_CallHook,&hook_standard,callback_users_delete);

		DoMethod(CM_cpwd_PASS,MUIM_Notify,MUIA_Selected,TRUE,
			STR_cpwd_PASS,3,MUIM_Set,MUIA_Disabled,FALSE);
		DoMethod(CM_cpwd_PASS ,MUIM_Notify,MUIA_Selected,FALSE,
			STR_cpwd_PASS,3,MUIM_Set,MUIA_Disabled,TRUE);
		DoMethod(BT_cpwd_OKAY,MUIM_Notify,MUIA_Pressed,FALSE,
			//app,2,MUIM_Application_ReturnID,CPWD_OKAY);
			app,3,MUIM_CallHook,&hook_standard,callback_cpwd_okay);
		DoMethod(BT_cpwd_CANCEL,MUIM_Notify,MUIA_Pressed,FALSE,
			wnd_cpwd,3,MUIM_Set,MUIA_Window_Open,FALSE);
		DoMethod(wnd_cpwd,MUIM_Notify,MUIA_Window_CloseRequest,TRUE,
			wnd_cpwd,3,MUIM_Set,MUIA_Window_Open,FALSE);

		DoMethod(BT_about_OKAY,MUIM_Notify,MUIA_Pressed,FALSE,
			wnd_about,3,MUIM_Set,MUIA_Window_Open,FALSE);

		srand(clock());
		set(CY_nng_DEFSIG,MUIA_Cycle_Active,1);
		set(STR_nng_MAXDL,MUIA_Disabled,TRUE);
		set(STR_eng_MAXDL,MUIA_Disabled,TRUE);
		set(STR_nng_MAXL,MUIA_Disabled,TRUE);
		set(STR_eng_MAXL,MUIA_Disabled,TRUE);
		set(STR_nng_MAXAGE,MUIA_Disabled,TRUE);
		set(STR_eng_MAXAGE,MUIA_Disabled,TRUE);
		set(STR_ngadv_APPVHD,MUIA_Disabled,TRUE);
		set(STR_ngadv_ALTNAME,MUIA_Disabled,TRUE);
		set(STR_ngadv_ALTEMAIL,MUIA_Disabled,TRUE);
		set(CY_ngadv_SERVER,MUIA_Disabled,TRUE);
		set(CM_ngadv_SERVERPOST,MUIA_Disabled,TRUE);
		set(STR_acc_USER,MUIA_Disabled,TRUE);
		set(STR_acc_PASS,MUIA_Disabled,TRUE);
		set(STR_acc_READHEADER,MUIA_Disabled,TRUE);
		set(STR_acc_EXPIREKILL,MUIA_Disabled,TRUE);
		resetGdataActive();
		set(wnd_main,MUIA_Window_Open,TRUE);

		do_input();
		do {
			if(u_signal)
				u_signal = Wait(u_signal);
		#ifndef DONT_USE_THREADS
			if(u_signal)
				thread_handle();
		#endif
			do_input();
		} while(running);

		set(wnd_main,MUIA_Window_Open,FALSE);

		free_data();

	} // end of success (for login)

	for(int i=1; i <= 12; i++)
	{
		DoMethod((Object*)NLIST_messagelistdata, 		MUIM_NList_UseImage, NULL, i, 0L);
		DoMethod((Object*)LISTTREEVIEW_messagelistdata, MUIM_NList_UseImage, NULL, i, 0L);
		DoMethod((Object*)NLIST_joinmsgs_MSGS, 			MUIM_NList_UseImage, NULL, i, 0L);
		DoMethod((Object*)NLIST_search_RES, 			MUIM_NList_UseImage, NULL, i, 0L);
	}


	DoMethod((Object*)NLIST_groupdata,		MUIM_NList_UseImage, NULL, 10, 0L);
	DoMethod((Object*)LISTTREE_groupdata,	MUIM_NList_UseImage, NULL, 10, 0L);
	DoMethod((Object*)NLIST_groupdata,		MUIM_NList_UseImage, NULL, 11, 0L);
	DoMethod((Object*)LISTTREE_groupdata,	MUIM_NList_UseImage, NULL, 11, 0L);
	DoMethod((Object*)NLIST_groupdata, 		MUIM_NList_UseImage, NULL, 12, 0L);
	DoMethod((Object*)LISTTREE_groupdata, 	MUIM_NList_UseImage, NULL, 12, 0L);

	if (unread_mail_icon)
	{
		MUI_DisposeObject((Object*)unread_mail_icon);
		unread_mail_icon = NULL;
	}
	if (read_mail_icon)
	{
		MUI_DisposeObject((Object*)read_mail_icon);
		read_mail_icon = NULL;
	}
	if (new_mail_icon)
	{
		MUI_DisposeObject((Object*)new_mail_icon);
		new_mail_icon = NULL;
	}

	if (hold_mail_icon)
	{
		MUI_DisposeObject((Object*)hold_mail_icon);
		hold_mail_icon = NULL;
	}

	if (flagred_icon)
	{
		MUI_DisposeObject((Object*)flagred_icon);
		flagred_icon = NULL;
	}

	if (flaggreen_icon)
	{
		MUI_DisposeObject((Object*)flaggreen_icon);
		flaggreen_icon = NULL;
	}

	if (flagyellow_icon)
	{
		MUI_DisposeObject((Object*)flagyellow_icon);
		flagyellow_icon = NULL;
	}

	if (replied_mail_icon)
	{
		MUI_DisposeObject((Object*)replied_mail_icon);
		replied_mail_icon = NULL;
	}

	if (onserver_mail_icon)
	{
		MUI_DisposeObject((Object*)onserver_mail_icon);
		onserver_mail_icon = NULL;
	}

	if (outgoing_icon)
	{
		MUI_DisposeObject((Object*)outgoing_icon);
		outgoing_icon = NULL;
	}

	if (sent_icon)
	{
		MUI_DisposeObject((Object*)sent_icon);
		sent_icon = NULL;
	}

	if (deleted_icon)
	{
		MUI_DisposeObject((Object*)deleted_icon);
		deleted_icon = NULL;
	}

	// should halt threads as soon as possible
	nLog("about to cleanup threads\n");
	cleanup_threads();

	nLog("about to call MUI_DisposeObject(app)\n");
	if(0 != app) MUI_DisposeObject(app);
	nLog("  MUI_DisposeObject() called\n");

	if(account.grouplistType!=0 && grouplistGroup_NLIST && guistarted) {
		nLog("dispose gdata nlist\n");
		MUI_DisposeObject(grouplistGroup_NLIST);
		nLog("gdata nlist disposed\n");
	}
	if(account.grouplistType!=1 && grouplistGroup_LISTTREE && guistarted) {
		nLog("dispose gdata listtree\n");
		MUI_DisposeObject(grouplistGroup_LISTTREE);
		nLog("gdata listtree disposed\n");
	}
	if(account.mdata_view!=0 && messageGroup_NLIST && guistarted) {
		nLog("dispose mdata nlist\n");
		MUI_DisposeObject(messageGroup_NLIST);
		nLog("mdata nlist disposed\n");
	}
	if(account.mdata_view!=1 && messageGroup_LISTTREE && guistarted) {
		nLog("dispose mdata listtree\n");
		MUI_DisposeObject(messageGroup_LISTTREE);
		nLog("mdata listtree disposed\n");
	}

	/*
	if(0 != editor_mcc)
	{
		nLog("about to dispose of TextEditor Custom Class\n");
		MUI_DeleteCustomClass(editor_mcc);
		nLog("  MUI_DeleteCustomClass() called\n");
	}
	*/
	nLog("Closing catalog...");
	CloseCatalog(nc_Catalog);

	nLog("freeing private codesets list...");
	if(codesetsList != NULL)
	{
		CodesetsListDelete(CSA_CodesetList, codesetsList, TAG_DONE);
		codesetsList = 0;
	}
	nLog("about to call CloseLibrary(Codesets.library)\n");
	if (CodesetsBase) {
		CloseLibrary(CodesetsBase);
		CodesetsBase=0;
	}
	if(ICodesets) {
		DropInterface((struct Interface *)ICodesets);
	}


	if (LocaleBase) {
		CloseLibrary(LocaleBase);
		LocaleBase=0;
	}
	if(ILocale) {
		DropInterface((struct Interface *)ILocale);
	}

	nLog("about to call CloseLibrary(MUIMasterBase)\n");
	if(MUIMasterBase)
   	CloseLibrary(MUIMasterBase);
	nLog("  CloseLibrary() called\n");

	if (IntuitionBase) {
		CloseLibrary(IntuitionBase);
		IntuitionBase=0;
	}
	if(IIntuition) {
		DropInterface((struct Interface *)IIntuition);
	}

	nLog("NewsCoaster finishing - closing log file\n");
	if(logFile!=0) {
		Close(logFile);
		logFile=NULL;
	}
	#ifdef _DEBUG
	dumpAllocs();
	#endif
	return 0;
}

int realmain()
{
	int ret = 0;
	ret = realmain_2();

	return ret;
}

int main(int argc,char **argv) {
	for(int i=0;i<argc;i++)
	{
		if(stricmp(argv[i],"DEBUG")==0)
		{
			DEBUG=TRUE;
			printf("DEBUG mode activated\n");
		}
		else if(stricmp(argv[i],"FORCEPATCH1")==0)
		{
			FORCEPATCH1=TRUE;
			printf("FORCEPATCH1 mode activated\n");
		}
	}
	return realmain();
}

int wbmain(WBStartup* wbmsg) {
	return realmain();
}
