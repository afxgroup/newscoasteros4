#ifndef __various_h__
#define __various_h__

#ifdef __MORPHOS__
#include <clib/debug_protos.h>
void dprintf(char *, ...);
#endif
#if defined(__MORPHOS__) || defined(__amigaos4__)
#pragma pack(2)
#endif

#define STRNICMP(a,b,c) (Strnicmp(a,b,c))
#ifndef __amigaos4__
#define FFlush Flush
#endif

const int MIMEPREFS_TYPE_LEN = 31;
const int MIMEPREFS_VIEWER_LEN = 511;
struct MIMEPrefs {
	char type[MIMEPREFS_TYPE_LEN+1];
	char viewer[MIMEPREFS_VIEWER_LEN+1];
	char spare[64];
};

const int READHEADER_LEN = 511;

class Account {
public:
	char name[64];
	char email[64];
	char nntp[64]; // see later for additional newsservers
	int nntp_auth;
	char user[64];
	char password[64];
	int usesig;
	int sentID;
	int timezone;
	int bst;
	int killfiles;
	char smtp[64];
	char domain[64];
	int use_sizekill; // use size killfile?
	int sizekill; // max size allowed if so
	int dateformat; // 0=dd-mmm-yy, 1=dd-mm-yy
	MUI_PenSpec pen_acc_text_norm;
	MUI_PenSpec pen_acc_text_quote1;
	MUI_PenSpec pen_acc_text_quote2;
	MUI_PenSpec pen_acc_text_col;
	short readheader_type;
	short readheader_first; // 0 means initialise, then set this to 1
	char readheaders[READHEADER_LEN + 1];
	// max of 16 flags only!
	static short SNIPSIG,LOGGING,LOGDEL,NODELONLINE,QUIETDL,NOCONFIRMQUIT,CHECKFORDUPS;
	short flags;
		// bit field: 0=snipsig, 1=logging, 2=delete log, 3=online autodelete,
		// 4=quiet statusbar when downloading, 5=force quit,
		// 6=always check for duplicates?
	char lastGotGroups[14];
	char nntppost[64];
	int defexpiretype; // default for killfiles
	int defexpire;
	short sorttype; //0=subject, 1=date, 2=from
	short sorttypedir; //1=down,-1=up
	short grouplistType; //0=NList, 1=Listtree, 2=Listtree w/ hierarchy
	short mdata_view; // 0 means NLIST, 1 means threaded
	int uselocaletz; // use locale prefs for timezone 0=no,1=yes
	ULONG displayFormat; // 0=email, 1=plain text (when viewing messages)
	static short LISTUPDATE, LISTFLAGS, LISTFROMGROUP, LISTDATE, LISTSUBJECT, LISTSIZE, LISTLINES;
	int listflags;
		// 0='update', 1=flags, 2=from/newsgroup, 3=date, 4=subject, 5=size,
		// 6=lines
	char buffer[2528]; // was 2528
	char charset_read[100]; // currently unused
	char charset_write[100];
	int port;
	char buffer2[28];
	float version; // what version of NC was used to create this .prefs file?
	char followup_text[128];
	int noconfirmdel; // don't confirm deletions? (non-permenant)
	int vgroupdl; // view group when downloading?
	int weightGLIST,weightMLIST;
	int weightHD,weightMESS;
	char org[64];
	char realemail[64];
	int multiplevws;
	int mimeprefsno;
	MIMEPrefs * mimeprefs[256]; //mimeprefs
	char defviewer[300];
	int linelength;
	int rewrap; //0=none, 1=replies
	int xno; // 0=none at all, 1=always, 2=only if on replied
	int xnews; // short x-newsreader header?
	int h_weightHD,weightATT;
	int spare[10]; // for updating!
	void save_data();
	void save_tofile(char * filename);};
extern Account account;

class GroupData_v2 {
public:
	int ID; // Folder ID; -1=outgoing, -2=sent
	char desc[256]; // descriptive name
	char name[256];  // usenet name
	int nummess; // number of messages
	int nextmID; // next message ID
	int s; // subscribed?
	int defsig; // default sig
	struct DateStamp lastdlds; // temporary store for below
	//char lastdl[32]; // date last downloaded (YYMMDD HHMMSS)
	char old[32];
	int max_dl; // max messages to download (-1=unlimited)
	int flags[8];	// 0=downloaded for this group in this session?, 1=do we need to update index?, 1=has index been written?, 2=max. lines to read (0=unlimited)
						// 3=first download (0=first, -1=not first), 4=next article number to download (-1=do last 100)
						// 5=online reading?
						// 6=discard type (0=none,1=age,2=number), 7=discard limit (days/messages)
	short mflags[16]; // 0=currently downloading for this group?, 1=newsserver to use (0-7)

	/*GroupData_v2() {
		int i = 0;
		ID = 0;
		for(i=0;i<256;i++) {
			desc[i] = 0;
			name[i] = 0;
		}
		nummess = 0;
		nextmID = 0;
		s = 0;
		defsig = 0;
		for(i=0;i<32;i++)
			lastdl[i] = '\0';
		max_dl = 0;
		for(i=0;i<8;i++)
			flags[i] = 0;
		for(i=0;i<16;i++)
			mflags[i] = 0;
	}*/
	/*GroupData_v2() {
		memset(this,0,sizeof(GroupData_v2));
	}*/
};

const int GroupData_UNUSED = 824; // was
class GroupData : public GroupData_v2 {
public:
	char appv_hd[64]; // insert Approved header for this group?
	int moreflags[16];	// 0=use Approved header?, 1=use alternative name?,
								// 2=use alternative email?
								// 3=up to date with newsgroup (and thus don't need to check for duplicates)?
								// 4=post using the specified newsserver (if not default)
	char alt_name[32];
	char alt_email[32];
	int serverID; // 0==use default
	int num_unread;
	char unused[GroupData_UNUSED];
	/*GroupData() : GroupData_v2() {
		int i = 0;
		for(i=0;i<64;i++)
			appv_hd[i] = '\0';
		for(i=0;i<16;i++)
			moreflags[i] = 0;
		for(i=0;i<32;i++) {
			alt_name[i] = '\0';
			alt_email[i] = '\0';
		}
		num_unread = 0;
		for(i=0;i<GroupData_UNUSED;i++)
			unused[i] = '\0';
	}*/
	/*GroupData() {
		memset(this,0,sizeof(GroupData));
	}*/
};

const int SERVER_LEN = 63;
class Server {
public:
	char nntp[SERVER_LEN+1];
	char user[SERVER_LEN+1];
	char password[SERVER_LEN+1];
	int nntp_auth;
	int port;
	int def;
	int post;
	int ID;
	char lastGotGroups[15];
	char spare[255];
	Server() {
		int i;
		for(i=0;i<(SERVER_LEN+1);i++)
		{
			nntp[i] = '\0';
			user[i] = '\0';
			password[i] = '\0';
		}
		nntp_auth = FALSE;
		port=119;
		def = FALSE;
		post = FALSE;
		ID = 0; // must be set!
		*lastGotGroups = '\0';
		for(i=0;i<255;i++)
			spare[i] = 0;
	}
};

const int NMFLAGS=16; // no. of messagelistdata flags
const int MAXATT=64;
const int IHEADENDLONG=127;
const int IHEADENDSUBJECT = 95;
const int IHEADENDMID=63;
const int IHEADEND=63;
const int IHEADENDSHORT=31;

class MessageListData {
public:
	int ID; // internal version
	char from[IHEADENDSHORT+1];
	char newsgroups[IHEADENDSHORT+1];
	char subject[IHEADENDSUBJECT+1];
	//char subject[IHEADEND+1];
	//char xxdate[IHEADENDSHORT+1]; // original format
	char datec[32]; // Date in displayable format
	struct DateStamp ds; // AmigaDOS date
	char c_date[IHEADENDSHORT+1],c_time[IHEADENDSHORT+1];
	char messageID[IHEADENDMID+1]; // the usenet version
	char lastref[IHEADENDMID+1]; // the last reference, ie, the messageID of the message this is replying to ("" if new message)
	//char type[IHEADENDSHORT+1-8]; // eg, text/plain
	char unused[IHEADENDSHORT+1-8];
	int mIDhash;// a hash for the MessageID
	int size; // the size of the post
	int flags[NMFLAGS]; // 0=hold,1=unread,2=post immediately,3=temp store for previous (2)
								// 4=is newsgroup?,5=is email?,6=gdata ID this belongs to, for searching and those in deleted, only
								// 7=index pos(temporary), 8=LINES in article
								// 9=level of article for threading,
								// 10=has been replied to
								// 11=article number when downloaded (backup if by messageID returns BODY not found)
								// 12=do we still need to download body?
								// 13=importance (0=normal, 1=highlight)
	MessageListData();
	void init();
};

const int USER_LEN = 8;
class User {
	char username[USER_LEN+1];
	char password[USER_LEN+1]; // stored in encrypted format (0 length means no password required)
	int flags; // bitfield: 0=supervisor?
	void encode(char * pass) {
		if(pass==NULL)
			return;
		int i=0;
		int len=strlen(pass);
		for(i=0;i<len;i++) {
			if(i==0)
				pass[i] = ( pass[i] * (i+2) ) % 256;
			else
				pass[i] = ( pass[i] * (i+1) + pass[i-1] * i ) % 256;
		}
		if(len<USER_LEN) {
			for(i=len+1;i<USER_LEN+1;i++)
				pass[i]=0;
		}
	}
public:
	char dataLocation[300];
	User() {
		//memset(this,0,sizeof(User));
	}
	User(char * name,char * pass) {
		//memset(this,0,sizeof(User));

		strcpy(username,name);
		if(pass!=NULL)
			strcpy(password,pass);
		else
			strcpy(password,"");
		encode(password);
		flags=0;
	}
	BOOL isUser(char * name) {
		if(strcmp(username,name)==0)
			return TRUE;
		return FALSE;
	}
	BOOL isValid(char * name,char * passptr) {
		BOOL okay=FALSE;
		if(strcmp(username,name)==0) {
			char pass[USER_LEN+1] = "";
			strncpy(pass,passptr,USER_LEN);
			pass[USER_LEN] = '\0';
			encode(pass);
			if(password[0]==0)
				okay=TRUE;
			else {
				okay=TRUE;
				for(int i=0;i<USER_LEN+1;i++) {
					if(password[i]!=pass[i]) {
						okay=FALSE;
						break;
					}
				}
			}
			for(int i=0;i<USER_LEN+1;i++)
				pass[i]=0;
		}
		unsigned int i=0;
		for(i=0;i<strlen(passptr);i++)
			passptr[i]=0;
		for(i=0;i<strlen(name);i++)
			name[i]=0;
		return okay;
	}
	void setSupervisor(BOOL sup) {
		flags = flags | 1;
		if(!sup)
			flags -= 1;
	}
	BOOL isSupervisor() {
		return ( (flags & 1)>0 );
	}
	BOOL requiresPassword() {
		return (password[0]!=0);
	}
	char * getName() {
		return username;
	}
	void setPassword(char * pass) {
		if(pass!=NULL)
			strcpy(password,pass);
		else
			strcpy(password,"");
		encode(password);
	}
};

class KillFile {
public:
	char header[64]; // exact match
	char text[256]; // substring search
	char ngroups[64]; // substring search
	struct DateStamp ds; // creation date
	struct DateStamp lastused; // last usage (==creation date to begin with)
	int expiretype; // 0=never, 1=from creation, 2=since last used
	int expire; // days to go before expiring (0 == never)
	//short type; // 0=kill, 1=keep, 2=mark important
	short match; // 0 = contains, 1 = doesn't contain;
	short action; // 0 = kill, 1 = mark important
	char carryon; // carry onto other killfiles? (0/1)
	char spare[91]; // was 94
	KillFile();
};

#if defined(__MORPHOS__) || defined(__amigaos4__)
#pragma pack()
#endif

const int NEWMESS_shortshort = 64;
const int NEWMESS_short = 256;
const int NEWMESS_long = 1024;
const int MAX_REFS = 512;

/*class NewMessage {
public:*/
struct NewMessage {
	char from[NEWMESS_long+1];
	char replyto[NEWMESS_long+1];
	char to[NEWMESS_long+1];
	char newsgroups[NEWMESS_long+1];
	char followup[NEWMESS_long+1];
	char subject[NEWMESS_long+1];
	char *references[MAX_REFS];
	int nrefs;
	BOOL edit; // is this an edited message?
	BOOL reply; // are we replying to another message (replying means same as followingup here!)
	int replied_gdataID; // replied group ID
	int replied_mdataID; // replied message ID (internal version ID)
	char messageID[NEWMESS_short+1]; // ID of edited message if so
	int mIDhash;// a hash for the MessageID (currently unused)
	int xno; // was original message xno?
	BOOL sent; // TRUE if this is a sent message when being edited
	char supersedeID[NEWMESS_short+1]; // ID of message to supersede; "" if not superseding
	char xnewsreader[NEWMESS_long+1];
	char date[NEWMESS_shortshort+1];
	char type[64]; // eg text/plain
	char dummyHeader[NEWMESS_long+1];
	char getThisHeader[NEWMESS_shortshort+1]; // if this is of length>0, it specifies a header to seek, and place the value in dummyHeader (include ':', eg, 'From:')
	BOOL online; // a guess at whether we still need to download body - will always be FALSE for messages downloaded before v1.26 !
	int lines;
	void copyToMessageListData(MessageListData * mdata);
	/*NewMessage() {
		memset(this,0,sizeof(NewMessage));
	}*/
};

const int MIMETYPE_FILE_LEN = 299;
const int MIMETYPE_SHORTNAME_LEN = 63;
const int MIMETYPE_TYPE_LEN = 63;
struct MIMEType {
	char file[MIMETYPE_FILE_LEN+1];
	char shortname[MIMETYPE_SHORTNAME_LEN+1];
	char type[MIMETYPE_TYPE_LEN+1];
	int size;
};

struct StatsList {
	char value[256];
	int freq;
};

#if defined(__amigaos4__)

#define HOOKCL1 HOOK1
#define HOOKCL2 HOOK2
#define HOOKCL3 HOOK3

#define HOOK1( type0, funname, type1, arg1, reg1 ) \
	type0 funname( REG(reg1, type1 arg1) ) \
{

#define HOOK2( type0, funname, type1, arg1, reg1, type2, arg2, reg2 ) \
	type0 funname( CPPHook* hook, REG(reg1, type1 arg1), REG(reg2, type2 arg2) ) \
{

#define HOOK3( type0, funname, type1, arg1, reg1, type2, arg2, reg2, type3, arg3, reg3 ) \
	type0 funname( REG(reg1, type1 arg1), REG(reg2, type2 arg2), REG(reg3, type3 arg3) ) \
{

#define DECL_FUNCHOOK1( type0, funname, type1, arg1, reg1 ) \
	static type0 funname( REG(reg1, type1 arg1) );

#define DECL_FUNCHOOK2( type0, funname, type1, arg1, reg1, type2, arg2, reg2 ) \
        static type0 funname( CPPHook* hook, REG(reg1, type1 arg1), REG(reg2, type2 arg2) );

#define DECL_FUNCHOOK3( type0, funname, type1, arg1, reg1, type2, arg2, reg2, type3, arg3 , reg3 ) \
	static type0 funname( REG(reg1, type1 arg1), REG(reg2, type2 arg2), REG(reg3, type3 arg3));

#elif !defined( __GNUC__ )

#define HOOKCL1 HOOK1
#define HOOKCL2 HOOK2
#define HOOKCL3 HOOK3

#define HOOK1( type0, funname, type1, arg1, reg1 ) \
	type0 funname( REG(reg1) type1 arg1 ) \
{

#define HOOK2( type0, funname, type1, arg1, reg1, type2, arg2, reg2 ) \
	type0 funname( REG(reg1) type1 arg1, REG(reg2) type2 arg2 ) \
{

#define HOOK3( type0, funname, type1, arg1, reg1, type2, arg2, reg2, type3, arg3, reg3 ) \
	type0 funname( REG(reg1) type1 arg1, REG(reg2) type2 arg2, REG(reg3) type3 arg3 ) \
{

#define DECL_FUNCHOOK1( type0, funname, type1, arg1, reg1 ) \
	static type0 funname( REG(reg1) type1 arg1 );

#define DECL_FUNCHOOK2( type0, funname, type1, arg1, reg1, type2, arg2, reg2 ) \
	static type0 funname( REG(reg1) type1 arg1, REG(reg2) type2 arg2 );

#define DECL_FUNCHOOK3( type0, funname, type1, arg1, reg1, type2, arg2, reg2, type3, arg3, reg3 ) \
	static type0 funname( REG(reg1) type1 arg1, REG(reg2) type2 arg2, REG(reg3) type3 arg3 );

// Unfortunately STORM C does not support #elseif :-(((
#else
#if !defined(__MORPHOS__)

#define HOOKCL1 HOOK1
#define HOOKCL2 HOOK2
#define HOOKCL3 HOOK3

#define HOOK1( type0, funname, type1, arg1, reg1 ) \
	type0 funname(VOID)                     \
{                                    \
	register type1 reg1 __asm(#reg1); \
	type1 arg1 = reg1;

#define HOOK2( type0, funname, type1, arg1, reg1, type2, arg2, reg2 ) \
	type0 funname(VOID)                     \
{                                    \
	register type1 reg1 __asm(#reg1); \
	type1 arg1 = reg1;                \
	register type2 reg2 __asm(#reg2); \
	type2 arg2 = reg2;

#define HOOK3( type0, funname, type1, arg1, reg1, type2, arg2, reg2, type3, arg3, reg3 ) \
	type0 funname(VOID)                     \
{                                    \
	register type1 reg1 __asm(#reg1); \
	type1 arg1 = reg1;                \
	register type2 reg2 __asm(#reg2); \
	type2 arg2 = reg2;                \
	register type3 reg3 __asm(#reg3); \
	type3 arg3 = reg3;


#define DECL_FUNCHOOK1( type0, funname, type1, arg1, reg1 ) \
	static type0 ASM funname();

#define DECL_FUNCHOOK2( type0, funname, type1, arg1, reg1, type2, arg2, reg2 ) \
	static type0 ASM funname();

#define DECL_FUNCHOOK3( type0, funname, type1, arg1, reg1, type2, arg2, reg2, type3, arg3, reg3 ) \
	static type0 ASM funname();

#else

#define HOOKCL1( type0, funname, type1, arg1, reg1 ) \
    struct EmulLibEntry funname = \
	{ \
		TRAP_LIB, 0, (void (*)(void))funname##_GATE \
	}; \
	\
	type0 funname##_GATE(void) \
	{ \
		return (funname##_GATE2((type1)REG_##reg1)); \
	} \
	\
	type0 funname##_GATE2(type1 arg1) \
{

#define HOOKCL2( type0, funname, type1, arg1, reg1, type2, arg2, reg2 ) \
    struct EmulLibEntry funname = \
	{ \
		TRAP_LIB, 0, (void (*)(void))funname##_GATE \
	}; \
	\
	type0 funname##_GATE(void) \
	{ \
		return (funname##_GATE2((type1)REG_##reg1, (type2)REG_##reg2)); \
	} \
	\
	type0 funname##_GATE2(type1 arg1, type2 arg2) \
{

#define HOOKCL3( type0, funname, type1, arg1, reg1, type2, arg2, reg2, type3, arg3, reg3 ) \
    struct EmulLibEntry funname = \
	{ \
		TRAP_LIB, 0, (void (*)(void))funname##_GATE \
	}; \
	\
	type0 funname##_GATE(void) \
	{ \
		return (funname##_GATE2((type1)REG_##reg1, (type2)REG_##reg2, (type3)REG_##reg3)); \
	} \
	\
	type0 funname##_GATE2(type1 arg1, type2 arg2, type3 arg3) \
{

#define HOOK1( type0, funname, type1, arg1, reg1 ) \
	static type0 funname##_GATE(void); \
	static type0 funname##_GATE2(type1 arg1); \
	HOOKCL1( type0, funname, type1, arg1, reg1 )

#define HOOK2( type0, funname, type1, arg1, reg1, type2, arg2, reg2 ) \
	static type0 funname##_GATE(void); \
	static type0 funname##_GATE2(type1 arg1, type2 arg2); \
	HOOKCL2( type0, funname, type1, arg1, reg1, type2, arg2, reg2 )

#define HOOK3( type0, funname, type1, arg1, reg1, type2, arg2, reg2, type3, arg3, reg3 ) \
	static type0 funname##_GATE(void); \
	static type0 funname##_GATE2(type1 arg1, type2 arg2, type3 arg3); \
	HOOKCL3( type0, funname, type1, arg1, reg1, type2, arg2, reg2, type3, arg3, reg3 )

#define DECL_FUNCHOOK1( type0, funname, type1, arg1, reg1 ) \
	static type0 funname##_GATE( void ); \
	static type0 funname##_GATE2( type1 arg1 ); \
	static struct EmulLibEntry funname;

#define DECL_FUNCHOOK2( type0, funname, type1, arg1, reg1, type2, arg2, reg2 ) \
	static type0 funname##_GATE( void ); \
	static type0 funname##_GATE2( type1 arg1, type2 arg2 ); \
	static struct EmulLibEntry funname;

#define DECL_FUNCHOOK3( type0, funname, type1, arg1, reg1, type2, arg2, reg2, type3, arg3, reg3 ) \
	static type0 funname##_GATE( void ); \
	static type0 funname##_GATE2( type1 arg1, type2 arg2, type3 arg3 ); \
	static struct EmulLibEntry funname;

#endif
#endif

extern struct CPPHook hook_standard;
#endif
