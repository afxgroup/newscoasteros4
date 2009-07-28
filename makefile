REDEFINE = -DCoerceMethod=ICoerceMethod -DDoMethod=IDoMethod \
			-DDoSuperMethod=IDoSuperMethod -DDoSuperMethodA=IDoSuperMethodA

CFLAGS 	= -gstabs -c -mstrict-align -O3 -D__USE_INLINE__ -Wwrite-strings -Wno-unused $(REDEFINE) #-Wall 
CC		= ppc-amigaos-g++
LDLIBS 	= #-lm #-lauto
OBJ 	= obj/

all: prepare NewsCoaster

NewsCoaster: $(OBJ)choicelist.o $(OBJ)connection.o $(OBJ)datehandler.o $(OBJ)general.o $(OBJ)indices.o \
	$(OBJ)list.o $(OBJ)lists.o $(OBJ)misc.o $(OBJ)netstuff.o $(OBJ)statuswindow.o $(OBJ)strings.o \
	$(OBJ)subthreads.o $(OBJ)various.o $(OBJ)vector.o $(OBJ)viewwindow.o $(OBJ)writewindow.o $(OBJ)main.o
	$(CC) $^ -o Release/NewsCoaster $(LDLIBS)

$(OBJ)choicelist.o: choicelist.c mui_headers.h vector.h various.h main.h \
	choicelist.h
	$(CC) $(CFLAGS) -c choicelist.c -o $(OBJ)choicelist.o

$(OBJ)connection.o: connection.c mui_headers.h vector.h various.h main.h \
	misc.h connection.h
	$(CC) $(CFLAGS) -c connection.c -o $(OBJ)connection.o

$(OBJ)datehandler.o: datehandler.c vector.h various.h misc.h datehandler.h \
	strings.h
	$(CC) $(CFLAGS) -c datehandler.c -o $(OBJ)datehandler.o

$(OBJ)general.o: general.c vector.h general.h
	$(CC) $(CFLAGS) -c  general.c -o $(OBJ)general.o

$(OBJ)indices.o: indices.c mui_headers.h vector.h various.h main.h misc.h \
	indices.h lists.h statuswindow.h newscoaster_catalog.h
	$(CC) $(CFLAGS) -c indices.c -o $(OBJ)indices.o

$(OBJ)list.o: list.c list.h
	$(CC) $(CFLAGS) -c list.c -o $(OBJ)list.o

$(OBJ)lists.o: lists.c mui_headers.h vector.h various.h main.h lists.h \
	misc.h strings.h indices.h
	$(CC) $(CFLAGS) -c lists.c -o $(OBJ)lists.o

$(OBJ)main.o: main.c mui_headers.h vector.h various.h viewwindow.h \
	writewindow.h main.h indices.h misc.h statuswindow.h netstuff.h \
	general.h choicelist.h datehandler.h strings.h lists.h \
	newscoaster_catalog.h
	$(CC) $(CFLAGS) -c main.c -o $(OBJ)main.o

$(OBJ)misc.o: misc.c vector.h various.h misc.h datehandler.h strings.h \
	main.h
	$(CC) $(CFLAGS) -c misc.c -o $(OBJ)misc.o

$(OBJ)netstuff.o: netstuff.c mui_headers.h vector.h main.h various.h \
	writewindow.h indices.h misc.h datehandler.h statuswindow.h \
	connection.h netstuff.h strings.h viewwindow.h lists.h \
	newscoaster_catalog.h
	$(CC) $(CFLAGS) -c netstuff.c -o $(OBJ)netstuff.o

$(OBJ)statuswindow.o: statuswindow.c mui_headers.h vector.h various.h main.h \
	misc.h statuswindow.h newscoaster_catalog.h
	$(CC) $(CFLAGS) -c statuswindow.c -o $(OBJ)statuswindow.o

$(OBJ)strings.o: strings.c mui_headers.h vector.h various.h misc.h strings.h
	$(CC) $(CFLAGS) -c strings.c -o $(OBJ)strings.o

$(OBJ)subthreads.o: subthreads.c list.h mui_headers.h subthreads.h \
	subthreads_amiga.h
	$(CC) $(CFLAGS) -c subthreads.c -o $(OBJ)subthreads.o

$(OBJ)various.o: various.c mui_headers.h vector.h various.h misc.h strings.h \
	datehandler.h
	$(CC) $(CFLAGS) -c various.c -o $(OBJ)various.o

$(OBJ)vector.o: vector.c mui_headers.h vector.h various.h misc.h
	$(CC) $(CFLAGS) -c vector.c -o $(OBJ)vector.o

$(OBJ)viewwindow.o: viewwindow.c mui_headers.h vector.h various.h \
	viewwindow.h writewindow.h misc.h main.h indices.h statuswindow.h \
	netstuff.h general.h strings.h lists.h newscoaster_catalog.h
	$(CC) $(CFLAGS) -c viewwindow.c -o $(OBJ)viewwindow.o

$(OBJ)writewindow.o: writewindow.c mui_headers.h vector.h various.h \
	writewindow.h main.h misc.h indices.h datehandler.h general.h \
	strings.h netstuff.h lists.h newscoaster_catalog.h
	$(CC) $(CFLAGS) -c writewindow.c -o $(OBJ)writewindow.o

#newscoaster_catalog.h: newscoaster.cd
#	catcomp newscoaster.cd CFILE newscoaster_catalog.h NOARRAY NOBLOCK NOCODE

prepare:
	@mkdir -p $(OBJ)
	
clean:
	rm -f release/NewsCoaster
	rm -f $(OBJ)*.o
