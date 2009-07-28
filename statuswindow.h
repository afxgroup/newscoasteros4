const int TXTLEN = 256;

class StatusWindow {
	Object *wnd;
	Object *TXT_INFO;
	Object *GROUP;
	BOOL aborted;
	char text[TXTLEN + 1];
	void init(const char * title,BOOL quiet);

	static void disposeFunc(StatusWindow **stsW);
	static void abortFunc(StatusWindow **stsW);

public:
	Object *BT_ABORT;
	BOOL is_subthread;

	StatusWindow(Object *app,const char * title);
	StatusWindow(Object *app,const char * title,BOOL quiet);
	~StatusWindow();
	void setTitle(const char * t);
	void setText(const char * t);
	void resize();
	void setVisible(BOOL v);
	void setAborted(BOOL aborted);
	BOOL isAborted();
};
