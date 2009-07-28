class StatusWindow;

void initNetstuff();
int auth(int s,char * buffer,StatusWindow * statusWindow);
void getgrouplist(Server *server);
void getnewgroups(Server *server);
void postnews(int type);
void getnews(int thisgroup,BOOL quiet = FALSE);
BOOL getBody(BOOL *available,GroupData * gdata,MessageListData * mdata,BOOL quiet = FALSE);
BOOL getBody(BOOL *available,GroupData * gdata,MessageListData * mdata,StatusWindow * statusWindow,int index,int max,BOOL first, BOOL delmess);
void closeSockets();

static char onlineflag[] = "X-NewsCoaster-Flag-Online: yes\n";
