class WriteWindow {
	enum {
		MENWRITE_MESSAGE,
		WRITE_SAVE
	};

	static CPPHook completeHook;
	static CPPHook attObjStrHook;
	static int count;
	static MUI_CustomClass *editor_mcc_write;

	static void rot13Func(WriteWindow **W);
	static void readFunc(WriteWindow **W);
	static void cysigFunc(WriteWindow **W);
	static void mimeTypeFunc(WriteWindow **W);
	static void disposeFunc(WriteWindow **W);
	static void addNGFunc(void **args);
	static void attFunc(void **args);
	static void menusFunc(void **args);

/*#ifndef __GNUC__
	static ULONG ASM TextEditor_Dispatcher_write(REG(a0) IClass *cl,REG(a2) Object *obj,REG(a1) struct MUIP_TextEditor_HandleError *msg);
	static LONG ASM completeFunc(REG(a0) CPPHook * hook,REG(a2) Object * object,REG(a1) WriteWindow ** W);
	static VOID ASM attObjStrFunc(REG(a2) Object *pop,REG(a1) Object *str);
	//static LONG ASM menusFunc(REG(a0) CPPHook * hook,REG(a2) Object * object,REG(a1) APTR * param);
	//static VOID ASM attFunc(REG(a0) CPPHook * hook,REG(a2) Object * object,REG(a1) WriteWindow ** W);
#else
	DECL_FUNCHOOK3( ULONG, TextEditor_Dispatcher_write, IClass *, cl, Object *, obj, struct MUIP_TextEditor_HandleError *, msg );
	DECL_FUNCHOOK3( LONG, completeFunc, CPPHook *, hook, Object *, object, WriteWindow **, W );
	DECL_FUNCHOOK2( VOID, attObjStrFunc, Object *, pop, Object *, str );
	//DECL_FUNCHOOK3( LONG, menusFunc, CPPHook *, hook, Object *, object, APTR *, param );
	//DECL_FUNCHOOK3( VOID, attFunc, CPPHook *, hook, Object *, object, WriteWindow **, W );
#endif*/
	DECL_FUNCHOOK3( ULONG, TextEditor_Dispatcher_write, IClass *, cl, a0, Object *, obj, a2, struct MUIP_TextEditor_HandleError *, msg, a1 );
	DECL_FUNCHOOK3( LONG, completeFunc, CPPHook *, hook, a0, Object *, object, a2, WriteWindow **, W, a1 );
	DECL_FUNCHOOK2( VOID, attObjStrFunc, Object *, pop, a2, Object *, str, a1 );

	char title[256];
	void updateMimeType(const char *mimetype);
	MessageListData *write_message(BOOL hold,BOOL online,BOOL force);
	static void create_mID(char * str,const char * email,const char * nntp);
	static BOOL createMessage(GroupData *gdata,MessageListData *mdata,const char *message);
	static BOOL replyedit_mess(BOOL reply,WriteWindow * ww);
	static BOOL exfile(MIMEType * mime);

public:
	int cmptr;
	static Vector *ptrs;
	Object *wnd;
	Object *BT_write_NG;
	Object *BT_write_FU;
	Object *BT_write_P;
	Object *BT_write_PL;
	Object *BT_write_H;
	Object *BT_write_C;
	Object *BT_write_ROT13;
	Object *BT_write_READ;
	Object *CY_write_SIG;
	Object *ED_write_MESS;
	Object *SLD_write_MESS;
	Object *STR_write_NG;
	Object *STR_write_TO;
	Object *STR_write_SUBJECT;
	Object *STR_write_FROM;
	Object *STR_write_FOLLOWUP;
	Object *NLIST_write_ATT;
	Object *BT_write_ADDATT;
	Object *BT_write_DELATT;
	Object *POP_write_MIME;
	Object *LIST_write_MIME;
	Object *STR_write_MIME;
	NewMessage newmessage;

	WriteWindow(const char *scrtitle);
	~WriteWindow();
	BOOL save(BOOL force);
	static void editMess(GroupData *gdata,MessageListData *mdata,BOOL super);
	static void cancelMess(MessageListData * mdata2);
	static BOOL replyedit_mess(BOOL reply,WriteWindow * ww,GroupData * gdata,MessageListData * mdata,void *source);
	static void reply(BOOL followup,BOOL reply);
	static void reply(GroupData * gdata,MessageListData * mdata,void *source,BOOL followup,BOOL reply);
	static void sleepAll(BOOL sleep);
	static void freeStatics();
};
