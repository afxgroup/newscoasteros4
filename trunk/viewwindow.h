#ifndef __VIEWWINDOW_H__
/****************************************************************
 *  *  Checkin History
 *  +---------------------------------------------------------------
 *  * $Revision: 1.6 $
 *  * $Date: 2004/05/13 09:51:50 $
 *  * $Log: viewwindow.h,v $
 *  * Revision 1.6  2004/05/13 09:51:50  sonic_amiga
 *  *
 *  *  Improved GCC compatiblity
 *  *
 *  * Revision 1.5  2004/03/29 02:18:23  mdwh2
 *  * Remove return IDs.
 *  * Change Password window should have password box disabled initially if user has not set a password.
 *  *
 *  * Revision 1.4  2004/03/28 03:52:32  mdwh2
 *  * Simplify some hook functions; replace returnIDs with functions; remove unused code.
 *  *
 *  * Revision 1.3  2003/11/15 06:51:18  scoulson
 *  * Added RCS header.
 *  * Added preprocessor stuff to preventmultiple inclusion.
 *  * (Also cecking that CVS is updating properly!!)
 *  *
 *  +---------------------------------------------------------------
 ****************************************************************/

#define __VIEWWINDOW_H__

const int MIMESECTION_LEN = 256;

class MimeSection {
public:
	char *start;
	int length;
	char name[MIMESECTION_LEN+1];
	char mimetype[MIMESECTION_LEN+1];
	char encoding[MIMESECTION_LEN+1];
	char boundary[MIMESECTION_LEN+1];
	MimeSection(char *start,int length,char *name,char *mimetype,char *encoding,char *boundary);
	void saveToFile(char *filename);
};

class ViewWindow {
	enum {
		MENVIEW_MESSAGE, MENVIEW_EDIT, MENVIEW_ATTACH,
		VIEW_HEADERS, VIEW_O, VIEW_PREV, VIEW_PREVT, VIEW_KILL, VIEW_DELNEXT, VIEW_NEXT, VIEW_APP, VIEW_EXP, VIEW_EXPADD, VIEW_REPLY,
		VIEW_MARK, VIEW_MARKNORMAL, VIEW_MARKIMPORTANT,
		VIEW_FIND, VIEW_FINDNEXT, VIEW_COPY, VIEW_ROT13,
		VIEW_ATTACHVIEW, VIEW_ATTACHSAVE, VIEW_UUVIEW, VIEW_UUSAVE,
	};

	BOOL noclose;
	int readheader_type; // local store of account.readheader_type

	static CPPHook closeHook;
	static CPPHook dblHook;
	static CPPHook dispattachHook;

	static int count;
	static MUI_CustomClass *editor_mcc_view;

	static void disposeFunc(ViewWindow **W);
	static void killFunc(ViewWindow **W);
	static void prevFunc(ViewWindow **W);
	static void prevtFunc(ViewWindow **W);
	static void delNextFunc(ViewWindow **W);
	static void nextFunc(ViewWindow **W);
	static void replyFunc(ViewWindow **W);
	static void expFunc(ViewWindow **W);
	static void expaddFunc(ViewWindow **W);
	static void appFunc(ViewWindow **W);
	static void findFunc(ViewWindow **W);
	static void menusFunc(void **args);

/*#ifndef __GNUC__
	static ULONG ASM TextEditor_Dispatcher_view(REG(a0) IClass *cl,REG(a2) Object *obj,REG(a1) Msg msg);
	static LONG ASM closeFunc(REG(a0) CPPHook * hook,REG(a2) Object * object,REG(a1) ViewWindow ** W);
	static LONG ASM dblFunc(REG(a1) struct ClickMessage *clickmsg);
	static LONG ASM dispattachFunc(REG(a2) char **array,REG(a1) MimeSection * data);
#else
	DECL_FUNCHOOK3( ULONG, TextEditor_Dispatcher_view, IClass *, cl, Object *, obj, Msg, msg );
	DECL_FUNCHOOK3( LONG, closeFunc, CPPHook *, hook, Object *, object, ViewWindow **, W );
	DECL_FUNCHOOK1( LONG, dblFunc, struct ClickMessage *, clickmsg );
	DECL_FUNCHOOK2( LONG, dispattachFunc, char **, array, MimeSection *, data );
#endif*/
	DECL_FUNCHOOK3( ULONG, TextEditor_Dispatcher_view, IClass *, cl, a0, Object *, obj, a2, Msg, msg, a1 );
	DECL_FUNCHOOK3( LONG, closeFunc, CPPHook *, hook, a0, Object *, object, a2, ViewWindow **, W, a1 );
	DECL_FUNCHOOK1( LONG, dblFunc, struct ClickMessage *, clickmsg, a1 );
	DECL_FUNCHOOK2( LONG, dispattachFunc, char **, array, a2, MimeSection *, data, a1 );

	void createMcc();
	void resetAppType();
	void readInIDs();
	void setID(int gID,int mID);
	void setCurrentID();

	BOOL insertMessage(MessageListData *mdata,char *enc,char *sep,char *charset);
	void parse(char *text,int length,char *name,char *mimetype,char *enc,char *sep,char *charset);
	void clearAttachmentList();
	void addAttachment(MimeSection *section);

	char *body;
	ViewWindow();
public:
	char title[256];
	int gID;
	int *mIDs;
	int nIDs;
	int cmptr;
	static Vector *ptrs;
	Object *wnd;
	Object *ED_view_HD;
	Object *SLD_view_HD;
	Object *ED_view_MESS;
	Object *SLD_view_MESS;
	Object *BT_view_REPLY;
	//Object *BT_view_EXP;
	Object *CY_view_APP;
	Object *BT_view_PREV;
	Object *BT_view_PREVT;
	Object *BT_view_KILL;
	Object *BT_view_DELNEXT;
	Object *BT_view_NEXT;
	Object *GROUP_view_HD;
	Object *GROUP_view_MESS;
	Object *GROUP_view_ATTACH;
	Object *NLIST_view_ATTACH;
	Object *BT_view_ATTVIEW;
	Object *BT_view_ATTSAVE;
	Object *menustrip;
	Object *menuitem_VIEWEDIT;
	Object *pages;

	Object *wnd_find;
	Object *STR_find_TEXT;
	Object *BT_find_OKAY;
	Object *CM_find_CASE;

	//ViewWindow(Object *app,char *scrtitle);
	ViewWindow(const char *scrtitle);
	~ViewWindow();
	static ViewWindow *createExportWindow(Object *app,const char *scrtitle,int gID,int mID);
	static void checkDeleted(int d_gID,int d_mID);
	static void sleepAll(BOOL sleep);
	static void freeStatics();
	void reset();
	void resetSingle(int gID,int mID);
	BOOL read(MessageListData *mdata);
	BOOL read();
};

struct ViewData {
	ViewWindow * vw;
};
#endif //__VIEWWINDOW_H__
