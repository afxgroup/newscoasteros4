void write_index_single(GroupData * gdata,MessageListData * mdata);
void write_index_multi(GroupData * gdata,MessageListData ** mdatalist,int count);
void write_index_update(GroupData * gdata,MessageListData * mdata,void *source);
void write_index_update_multi(GroupData * gdata,MessageListData ** mdatalist,int count,void *source);
void write_index_delete(GroupData * gdata,MessageListData * mdata);
void write_index_delete_multi(GroupData * gdata,MessageListData ** mdatalist,int count);
void write_index(int type);
void write_index(GroupData * gdata,Vector * vector);
void writeEmptyIndex(int gID);
void read_index();
BOOL read_index(GroupData * gdata,Vector * vector);
BOOL get_mdata_from_index(MessageListData **mdataptr,GroupData *gdata,int mID);

class index_handler {
	BPTR file;
	BPTR lock;
public:
	index_handler(GroupData * gdata);
	~index_handler();
	void write(MessageListData * mdata);
};

static char indexversion[]="i100";
