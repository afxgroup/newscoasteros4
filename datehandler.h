class DateHandler {
	static const char monthnames[12][4];
	static const char daynames[7][4];

public:
	static int getMonthIndex(const char * monthname);
	static void get_datenow(char * out,int timezone,BOOL bst);
	static void get_date(char * out,int timezone,BOOL bst,struct DateStamp ds);
	static void get_date_mailbox(char * out,struct DateStamp ds);
	static void read_date(struct DateStamp * ds,char * str,char * c_date,char * c_time);
};
