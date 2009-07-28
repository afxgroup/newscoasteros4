#ifndef __MAIN_H__
#define __MAIN_H__

#include <libraries/locale.h>

#define PLATFORM "AmigaOS4"

class Vector;
class Server;
class GroupData;
class MessageListData;
class StatusWindow;
struct NewMessage;
struct MIMEType;

#define SIZE_CTYPE      40

const int big_bufsize_g = 16384; // for large dynamically allocated buffers
const int MAXLINE = 4096;
//extern char line_buffer_g[];
extern char status_buffer_g[];
extern ULONG u_signal;
//extern char filename_g[];
//extern char filenamenew_g[];

int getGMTOffset();

void sleepAll(BOOL sleep);
void getIndexPath(char *buffer,int gID);
void getFolderPath(char *buffer,int gID);
void getFilePath(char *buffer,int gID,int mID);

//extern LONG returnID;
//#define do_input returnID = DoMethod(app,MUIM_Application_NewInput,&u_signal); if(returnID != 0) do_input_func
#define do_input() DoMethod(app,MUIM_Application_NewInput,&u_signal);
int do_input_func();
char * getstr(Object *obj);
char * gettxt(Object *obj);
void setEnabled();
void setGdataHelpText(GroupData *gdata);
void clearThreadView();
void threadView();
void update_screen_title(GroupData *gdata);
void set_and_read(int gID);
void set_and_read(GroupData * gdata);
BOOL exists(const char * buffer2);
void delete_file(GroupData * gdata,MessageListData * mdata);
void delete_file_n(GroupData * gdata,MessageListData ** mdataptr,int n);
void write_index_if_changed();
//void delete_mess(GroupData * gdata,MessageListData * mdata,BOOL visible);
void delete_mess_n(GroupData * gdata,MessageListData ** mdataptr,int n, BOOL multi);
//void move(GroupData * src_gdata,GroupData * dst_gdata,MessageListData * mdata,int type,BOOL multi);
void move_n(GroupData * gdata,GroupData * gdata2,MessageListData ** mdataptr,int n,int type,StatusWindow *statusWindow,BOOL multi);
void getUserEmail(GroupData * gdata,char * buffer);
void get_newsgroups(char *newsgroups,int gID,int mID);
enum getrefs_t {
	GETREFS_NONE = 0,
	GETREFS_ALL = 1,
	GETREFS_LAST = 2
};
void get_refs(NewMessage * newmess,char * filename,getrefs_t getRefs,BOOL trimRefs);
void get_refs(NewMessage * newmess,int gID,int mID,getrefs_t getRefs);
void get_refs(NewMessage * newmess,GroupData * gdata,MessageListData * mdata,getrefs_t getRefs);
Server * getServer(int ID);
Server * getDefaultServer();
Server * getPostingServer();
Server * getPostingServerForGroup(char *group);
void killmess(GroupData * gdata,MessageListData * mdata,int type);
void killmess(GroupData * gdata,MessageListData * mdata);
void exportAddress(GroupData * gdata,MessageListData * mdata);
void readgrouplist(Server *server,int filepos);
void rot13(Object *ED);
void getNGChoice(char * title,char * text,Object * STR,BOOL poster);
BOOL readInText(Object * textEditor,char * filename);
int parse_index_version(char *version);

class library {
private:
	Library * Base;
public:
	library(char * name,int version);
	~library();
	BOOL isopen()
	{
		if(Base==NULL)
			return FALSE;
		return TRUE;
	}
};

enum {
	LOGIN_OKAY = 1, // first enum must be > 0
	LOGIN_CANCEL,
	TOP_ENUM // must be last!
};

/* The Default Signature cycle gadget under the New Newsgroup window (wnd_nng)
 */
extern const char *CYA_nng_sigs[];

const int NSIGS=8;
static const char *StdEntries[] = {"Kind regards ", "Yours ", "Mvh ", NULL};
extern char *sigs[NSIGS];

/* Some MIME types
 */
static const char *POPA_write_mime[] = {
	"text/plain","text/html","text/x-aguide","application/octet-stream","application/rtf","application/postscript","application/x-lha","application/x-lzx","application/x-zip","image/jpeg","image/png","image/gif","image/x-ilbm","audio/basic","audio/x-8svx","audio/x-wav","video/mpeg","video/quicktime","video/x-anim","message/rfc822",NULL
};

// Builtin charsets (with no conversion)
static const char *POPA_Builtin_Tables[]=
	{
		"iso-8859-1",
		"windows-1251",
		NULL
	};


extern Object *app;
extern Object *wnd_main;
extern Object *wnd_newnewsgroup;
extern Object *wnd_editng;
extern Object *wnd_account;
extern Object *wnd_killfile;
extern Object *pages_account;
extern Object *wnd_groupman;
extern Object *wnd_about;
extern Object *aboutwin;

extern Object *NLIST_groupdata;
extern Object *NLIST_messagelistdata;
extern Object *LISTTREE_messagelistdata;
extern Object *NLIST_groupman;
extern Object *NLIST_choice;

extern Object *BT_about_OKAY;

extern Object *TB_main;
enum TB_Index {
	TB_READ     = 0,
	TB_EXPORT 	= 1,
	TB_MOVE 		= 2,
	TB_COPY 		= 3,
	TB_DELETE 	= 4,
	TB_KILL 		= 5,
	// space
	TB_POST		= 7,
	TB_FOLLOWUP = 8,
	TB_EDIT 		= 9,
	// space
	TB_GETNEWS	= 11,
	TB_SENDNEWS	= 12,
	// space
	TB_SEARCH	= 14,
	TB_PROGSET	= 15,
	TB_KILLFILE	= 16
};

extern Object *BT_nng_OKAY;
extern Object *BT_nng_CANCEL;
extern Object *STR_nng_NAME;
extern Object *STR_nng_DESC;
extern Object *CY_nng_DEFSIG;
extern Object *CM_nng_S;
extern Object *CY_nng_MAXDL;
extern Object *STR_nng_MAXDL;
extern Object *CY_nng_MAXL;
extern Object *STR_nng_MAXL;

extern Object *BT_eng_OKAY;
extern Object *BT_eng_CANCEL;
extern Object *STR_eng_NAME;
extern Object *STR_eng_DESC;
extern Object *CY_eng_DEFSIG;
extern Object *CM_eng_S;
extern Object *CY_eng_MAXDL;
extern Object *STR_eng_MAXDL;
extern Object *CY_eng_MAXL;
extern Object *STR_eng_MAXL;
extern Object *TXT_eng_LASTDL;

extern Object *NLIST_joinmsgs_MSGS;

extern Object *BT_acc_OKAY;
extern Object *BT_acc_CANCEL;
extern Object *STR_acc_NAME;
extern Object *STR_acc_EMAIL;
extern Object *STR_acc_REALEMAIL;
extern Object *STR_acc_ORG;
extern Object *STR_acc_SMTP;
extern Object *CY_acc_TIMEZONE;
extern Object *CM_acc_BST;
extern Object *CM_acc_XNEWS;
extern Object *STR_acc_NNTP;
extern Object *CM_acc_AUTH;
extern Object *STR_acc_USER;
extern Object *STR_acc_PASS;
extern Object *NLIST_acc_SERVERS;
extern Object *BT_acc_ADDSERVER;
extern Object *BT_acc_EDITSERVER;
extern Object *BT_acc_MAKEDEFSERVER;
extern Object *BT_acc_DELETESERVER;
extern Object *BT_server_OKAY;
extern Object *BT_server_CANCEL;
extern Object *CY_acc_SIG;
extern Object *CM_acc_USESIG;
extern Object *ED_acc_SIG;
extern Object *SLD_acc_SIG;
extern Object *STR_acc_LINELENGTH;
extern Object *CM_acc_REWRAP;
extern Object *CY_acc_XNO;
extern Object *NLIST_acc_KILLFILE;
extern Object *BT_acc_NEWKILLADD;
extern Object *STR_acc_NEWKILLHEAD;
extern Object *STR_acc_NEWKILLTEXT;
extern Object *STR_acc_NEWKILLGROUP;
extern Object *BT_acc_NEWKILL;
extern Object *BT_acc_DELKILL;
extern Object *BT_acc_DUPKILL;
extern Object *NLIST_acc_MIMEPREFS;
extern Object *POP_acc_MIME;
extern Object *LIST_acc_MIME;
extern Object *STR_acc_MIME;
extern Object *POP_acc_MIMEVIEW;
extern Object *STR_acc_MIMEVIEW;
extern Object *POP_acc_MIMEDEF;
extern Object *STR_acc_MIMEDEF;
extern Object *BT_acc_MIMENEW;
extern Object *BT_acc_MIMEDEL;
extern Object *BT_write_NG;
extern Object *BT_write_FU;
extern Object *BT_write_P;
extern Object *BT_write_PL;
extern Object *BT_write_H;
extern Object *BT_write_C;
extern Object *CY_write_SIG;
extern Object *ED_write_MESS;
extern Object *SLD_write_MESS;
extern Object *STR_write_NG;
extern Object *STR_write_TO;
extern Object *STR_write_SUBJECT;
extern Object *STR_write_FROM;
extern Object *STR_write_FOLLOWUP;
extern Object *NLIST_write_ATT;
extern Object *BT_write_ADDATT;
extern Object *BT_write_DELATT;
extern Object *POP_write_MIME;
extern Object *LIST_write_MIME;
extern Object *STR_write_MIME;
extern Object *STR_groupman_FIND;
extern Object *BT_groupman_SUB;
extern Object *BT_groupman_GET;
extern Object *BT_groupman_CLOSE;
extern Object *BT_groupman_SORT;

extern Object *NLIST_search_NG;
extern Object *BT_search_START;
extern Object *CY_search_WHERE;
extern Object *STR_search_HEAD;
extern Object *STR_search_WHAT;
extern Object *CM_search_CASESENS;
extern Object *NLIST_search_RES;

extern Object *NLIST_stats_NG;
extern Object *BT_stats_START;
extern Object *CY_stats_WHAT;
extern Object *NLIST_stats_RES;

extern Object *NLIST_groupdata;
extern Object *LISTTREE_groupdata;
extern Object *NLIST_messagelistdata;
extern Object *LISTTREE_messagelistdata;

extern struct MUI_NListtree_TreeNode *tn_folders,*tn_newsgroups;

extern Object *menustrip;
extern Object *menuitem_GETNEWS;
extern Object *menuitem_GETNEWSSINGLE;
extern Object *menuitem_SENDNEWS;
extern Object *menuitem_GETGROUPSDEF;
extern Object *menuitem_GETNEWGROUPSDEF;
extern Object *menuitem_DISCONNECT;
extern Object *menuitem_VIEWGROUPS;
extern Object *menuitem_VIEWMESSAGES;
extern Object *menuitem_UPDATEGROUPS;
extern Object *menuitem_UPDATEINDEX;
extern Object *menuitem_UPDATEALLIND;
extern Object *menuitem_IMPORT;
extern Object *menuitem_READ;
extern Object *menuitem_EDIT;
extern Object *menuitem_SUPER;
extern Object *menuitem_DELMESS;
extern Object *menuitem_UNDELMESS;
extern Object *menuitem_EXP;
extern Object *menuitem_MOVE;
extern Object *menuitem_COPY;
extern Object *menuitem_KILLMESS;
extern Object *menuitem_YAMADDRESS;
extern Object *menuitem_POST;
extern Object *menuitem_FOLLOWUP;
extern Object *menuitem_REPLY;
extern Object *menuitem_BOTH;
extern Object *menuitem_PDTHIS;
extern Object *menuitem_MARK;
extern Object *menuitem_USERS;

extern const char version[];
extern MUI_CustomClass *editor_mcc;
extern BOOL registered;
extern const CPPHook StrObjHook;
extern const CPPHook WindowHook;
extern const CPPHook DisplayHook_mime;
extern const CPPHook DestructHook_mime;
extern BOOL running;
extern BOOL accountpage;
extern BOOL delete_dis;
extern ULONG i_signal;
extern BOOL running;
extern int nextID;
extern Server *groupman_server;
extern GroupData * ng_edit;
extern GroupData *old_gdata;
extern GroupData *c_gdata;
extern GroupData *v_gdata;
extern library ASLlib; // constructor opens libraries
extern const int TAB;
extern const int MAXGROUPS;
extern int gdata_readID;

extern BOOL DEBUG;

extern Catalog *nc_Catalog;
#endif //__MAIN_H__
