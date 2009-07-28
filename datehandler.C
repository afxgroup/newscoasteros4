/*
    NewsCoaster
    Copyright (C) 1999-2002 Mark Harman

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    NewsCoaster Homepage :
        http://newscoaster.tripod.com/

    Mailing List to hear about announcements of new releases etc at:
        http://groups.yahoo.com/group/newscoaster-announce/
*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <proto/dos.h>
#include <proto/utility.h>
#include <libraries/mui.h>

#include "vector.h"
#include "various.h"
#include "misc.h"
#include "datehandler.h"
#include "strings.h"

const char DateHandler::monthnames[12][4] = { "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec" };
const char DateHandler::daynames[7][4] = { "Sun","Mon","Tue","Wed","Thu","Fri","Sat" };

int DateHandler::getMonthIndex(const char * monthname) {
	int m=-1;
	for(int k=0;k<12;k++) {
		if(STRNICMP(monthnames[k],monthname,3)==0) {
			m=k;
			break;
		}
	}
	return m;
}

/* Copies a string for the current date into 'out' in a format for putting
 * on the Date: header. 'timezone' is the number of hours to add.
 */
void DateHandler::get_datenow(char * out,int timezone,BOOL bst) {
	struct DateStamp ds;
	DateStamp(&ds);
	get_date(out,timezone,bst,ds);
}

/* Copies a string for date 'ds' into 'out' in a format for putting on the
 * Date: header. 'timezone' is the number of hours to add.
 */
void DateHandler::get_date(char * out,int timezone,BOOL bst,struct DateStamp ds) {
	nLog("get_date((char *)%d,(int)%d,(BOOL)%d,(DateStamp)%d,%d,%d) called\n",out,timezone,bst,ds.ds_Days,ds.ds_Minute,ds.ds_Tick);

	DateTime dt;
	char c_date[128]="";
	char c_datetemp[128]="";
	char c_time[128]="";
	char gmt_diff[64]="";
	char buffer[64]="";
	if(bst)
		timezone++;
	sprintf(buffer,"%+d",timezone);
	if(strlen(buffer)==2)
	{
		strcpy(gmt_diff," 0 00");
		gmt_diff[0]=buffer[0];
		gmt_diff[2]=buffer[1];
	}
	else
		sprintf(gmt_diff,"%s00",buffer);
	dt.dat_Stamp=ds;
	dt.dat_Format=FORMAT_CDN;
	dt.dat_Flags=NULL;
	dt.dat_StrDay=NULL;
	dt.dat_StrDate=(STRPTR)c_datetemp; // puts format dd-mm-yy
	dt.dat_StrTime=(STRPTR)c_time;
	DateToStr(&dt);
	int month = atoi(&c_datetemp[3]);
	// y2k friendly version
	strncpy(&c_date[0],&c_datetemp[0],3); // dd-
	const int ml = 3;
	strncpy(&c_date[3],DateHandler::monthnames[month-1],ml); // mmm
	strncpy(&c_date[3 + ml],&c_datetemp[5],1); // -
	// get century
	int days=ds.ds_Days;
	if(days>8034 || days<0)
		strncpy(&c_date[4 + ml],"20",2);
	else
		strncpy(&c_date[4 + ml],"19",2);

	strncpy(&c_date[6 + ml],&c_datetemp[6],2); // yy
	c_date[8 + ml]=0;

	for(unsigned int k=0;k<strlen(c_date);k++) {
		if(c_date[k]=='-')
			c_date[k]=' ';
	}
	sprintf(out,"%s %s %s",c_date,c_time,gmt_diff);
	nLog("  get_date() finished: %s\n",out);
}

/* Copies a string for date 'ds' into 'out' in a format for putting into
 * a standard mailbox file.
 */
void DateHandler::get_date_mailbox(char * out,struct DateStamp ds) {
	DateTime dt;
	char c_datetemp[128]="";
	char c_time[128]="";

	dt.dat_Stamp=ds;
	dt.dat_Format=FORMAT_CDN;
	dt.dat_Flags=NULL;
	dt.dat_StrDay=NULL;
	dt.dat_StrDate=(STRPTR)c_datetemp; // puts format dd-mm-yy
	dt.dat_StrTime=(STRPTR)c_time;
	DateToStr(&dt);
	int month = atoi(&c_datetemp[3]);
	strncpy(&out[0],DateHandler::daynames[ds.ds_Days % 7],3); // day
	out[3] = ' ';
	strncpy(&out[4],DateHandler::monthnames[month-1],3); // mmm
	out[7] = ' ';
	strncpy(&out[8],&c_datetemp[0],2); // dd
	out[10] = ' ';
	strncpy(&out[11],c_time,8);
	out[19] = ' ';
	// get century
	int days=ds.ds_Days;
	if(days>8034 || days<0)
		strncpy(&out[20],"20",2);
	else
		strncpy(&out[20],"19",2);
	strncpy(&out[22],&c_datetemp[6],2); // yy
	out[24]=0;
}

/* Reads the date in 'str' (which should be from a general Date: header),
 * and parse it into a standard consistent format. the DateStamp for the
 * date will be returned in 'ds', and the date and time returned in a
 * standard format in 'c_date' and 'c_time' respectively.
 */
void DateHandler::read_date(struct DateStamp * ds,char * str,char * c_date,char * c_time) {
	nLog("read_date((DateStamp *)%d,(char *)%s,(char *)%d,(char *)%d) called\n",ds,str,c_date,c_time);

	int day = 0;
	int month = 0;
	int year = 1978;
	int h = 0,m = 0,s = 0;
	int gmt = 0;
	char *ptr = strchr(str,',');
	if(ptr)
		ptr++;
	else
		ptr = str;
	while(*ptr == ' ')
		ptr++;

	if(*ptr != '\0') {
		day = atoi(ptr);
		ptr = strchr(ptr,' ');
		ptr++;
		if(*ptr != '\0') {
			month = DateHandler::getMonthIndex(ptr) +1;
			ptr = strchr(ptr,' '); ptr++;
			if(*ptr!=NULL) {
				year = atoi(ptr);
				if(year < 100) {
					// guess century
					if(year < 78)
						year += 2000;
					else
						year += 1900;
				}
				else if(year < 1000) {
					// stupid y2k bug
					// Fix for silly clients that rolled over on a 2 digit year,
					// eg, giving 101 instead of 2001
					year += 1900;
				}
				ptr = strchr(ptr,' '); ptr++;
				if(*ptr!=NULL) {
					if(sscanf(ptr,"%d:%d:%d",&h,&m,&s) != 3) {
						sscanf(ptr,"%d:%d",&h,&m);
						s = 0;
					}
				}
			}
		}
	}

	sprintf(c_date,"%02d-%02d-%02d",day,month,year%100);
	sprintf(c_time,"%02d:%02d:%02d",h,m,s);

	DateTime dt;
	dt.dat_Format=FORMAT_CDN;
	dt.dat_Flags=NULL;
	dt.dat_StrDay=NULL;
	dt.dat_StrDate=(STRPTR)c_date;
	dt.dat_StrTime=(STRPTR)c_time;
	StrToDate(&dt);

	char *tz = strchr(ptr,'+');
	if(tz==NULL)
		tz=strchr(ptr,'-');
	if(tz) {
		char buffer[4];
		strncpy(buffer,tz,3);
		buffer[3] = '\0';
		gmt = atoi(buffer);
		dt.dat_Stamp.ds_Minute -= gmt*60;
		while(dt.dat_Stamp.ds_Minute>=1440) {
			dt.dat_Stamp.ds_Minute -= 1440;
			dt.dat_Stamp.ds_Days++;
		}
		while(dt.dat_Stamp.ds_Minute<0) {
			dt.dat_Stamp.ds_Minute += 1440;
			dt.dat_Stamp.ds_Days--;
		}
	}

	if(account.dateformat==0)
		dt.dat_Format=FORMAT_DOS;
	else
		dt.dat_Format=FORMAT_CDN;
	DateToStr(&dt);

	*ds=dt.dat_Stamp;
	nLog("  finished read_date()\n");
}
