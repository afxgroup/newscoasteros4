class Connection {
	int s;
	static int connections;

public:
	int serverID;

	Connection();
	~Connection();
	BOOL init();
	BOOL call_socket(Server *server);
	BOOL call_socket(char *hostname, unsigned short portnum);
	void close();
	BOOL connected() {
		return (s!=-1);
	}
	void doOnlineBlocking(BOOL online,BOOL getbody);
	//int read_data_reply(char *buffer,int length);
	int read_data_chunk(char *buffer,int length);
	int read_data(char *buffer,int len);
	int read_data(char *buffer);
	int read_data_line(char *buffer);
	//int read_some_data(char *buffer,int max);
	void send_data(const char *buffer);
};
