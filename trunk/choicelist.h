/*  This class creates an MUI window with an NLIST,
      and provides buttons to press
*/

class ChoiceList {
private:
	Object *wnd;
	Object *group;
	Object *NLIST_choice;
	Object *app;
	Object *parent;
	char * title;
	char ** entries;
	int count;
	char ** buttons;
	int nb;
	Object * extraGroup;
	ULONG selected;

	void init(Object * app,Object * parent,char * title,char ** entries,int count,char ** buttons,int nb, Object * extraGroup) {
		this->app=app;
		this->parent=parent;
		this->title=title;
		this->entries=entries;
		this->count=count;
		this->buttons=buttons;
		this->nb=nb;
		this->extraGroup=extraGroup;
		selected=-1;
	}

public:
	ChoiceList(Object * app,Object * parent,char * title,char ** entries,int count,char ** buttons,int nb)
	{ init(app,parent,title,entries,count,buttons,nb,NULL); }

	ChoiceList(Object * app,Object * parent,char * title,char ** entries,int count,char ** buttons,int nb, Object * extraGroup)
	{ init(app,parent,title,entries,count,buttons,nb,extraGroup); }

	int show();
	int getSelected();
};
