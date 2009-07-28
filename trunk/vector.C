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

#include "mui_headers.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vector.h"
#include "various.h"
#include "misc.h"

Vector::Vector(int capacity) {
	//nLog("Vector::Vector((int)%d) called\n",capacity);
	if(capacity <= 0)
		capacity = 1;
	this->capacity=capacity;
	this->size=0;
	this->data = new void *[capacity];
	nLog("  capacity is %d\n",capacity);
}

Vector::Vector()
{
	//nLog("Vector::Vector() called\n");
	this->capacity=256;
	this->size=0;
	this->data = new void *[capacity];
	//nLog("  capacity is %d\n",capacity);
}

Vector::~Vector() {
	//nLog("Vector::~Vector() called\n");
	/*for(int i=0;i<size;i++) {
		if(data[i])
			delete data[i];
	}*/
	if(data)
		delete [] this->data;
}

int Vector::getSize() {
	return size;
}

void Vector::add(void * element) {
	if(size==capacity) {
		//nLog("  capacity was %d\n",capacity);
		capacity *= 2;
		//nLog("  Vector::add() :  need more Vector space, capacity is now %d\n",capacity);
		void ** newdata = new void *[capacity];
		if(newdata) {
			for(int i=0;i<size;i++)
				newdata[i]=data[i];
			if(data)
				delete [] this->data;
			data = newdata;
		}
		else {
			nLog("  ERROR - Failed to allocate new space in Vector::add()!!!\n");
			return;
		}
		//nLog("  done\n");
	}
	data[size++]=element;
}

void *Vector::getElementAt(int i) {
	return data[i];
}

void Vector::removeElement(void * element) {
	for(int i=0;i<size;i++) {
		if(data[i] == element) {
			removeElementAt(i);
			// no break - if multiple copies of the same object, we remove all (although multiple copies in a vector is not advised!)
		}
	}
}

void Vector::removeElementAt(int i) {
	/*void **newdata = new void *[capacity];
	int k=0;
	for(int j=0;j<size;j++) {
		if(j!=i)
			newdata[k++]=data[j];
	}
	newdata[size]=0;*/
	for(int j=i+1;j<size;j++) {
		data[j-1] = data[j];
	}
	data[size-1] = 0;
	size--;
	/*if(data)
		delete [] this->data;
	data = newdata;*/
}

void Vector::flush() {
	for(int i=0;i<size;i++) {
		delete data[i];
		data[i] = NULL;
	}
	size = 0;
}

#ifdef _DEBUG

Vector AllocInfo::allocs;

AllocInfo::AllocInfo(unsigned int address,unsigned int size, const char *file, int line) {
        this->address = address;
        this->size = size;
        strcpy(this->file,file);
        this->line = line;
}

BOOL RemoveTrack(unsigned int ptr,int size) {
	AllocInfo **allocInfo = (AllocInfo **)AllocInfo::allocs.getData();
        for(int i=0;i<AllocInfo::allocs.getSize();i++) {
                if(allocInfo[i]->address == ptr) {
                        AllocInfo::allocs.removeElementAt(i);
                        return TRUE;
                }
        }
        // trying to delete something not allocated / already deleted
	printf("### Tried to delete or free something of size %d already deleted or never allocated!!!\n",size);
	//nLog("### Tried to delete or free something of size already deleted or never allocated!!!\n",size);
        return FALSE;
}

void dumpAllocs() {
        unsigned int total = 0;
        FILE *file = fopen("NewsCoaster:nc_mem_log.txt","a+");
        fprintf(file,"\nVision Dump..\n\n");
	AllocInfo **allocInfo = (AllocInfo **)AllocInfo::allocs.getData();
        for(int i=0;i<AllocInfo::allocs.getSize();i++) {
                fprintf(file,"%-50s:\t\tLINE %d,\t\tADDRESS %d\t%d unfreed\n",
                        allocInfo[i]->file,allocInfo[i]->line,allocInfo[i]->address,allocInfo[i]->size);
                total += allocInfo[i]->size;
        }
        fprintf(file, "-----------------------------------------------------------\n");
        fprintf(file, "Total Unfreed: %d bytes\n", total);
        fclose(file);
};


#endif
