CFLAGS=-g -Wall -Wextra -I/usr/include/mysql

OBJS=util.o decpcap.o Packet.o connection.o connection_hashMap.o cursor_mem_pool.o shared_queue.o

lime-capture: lime_capture.c $(OBJS)
	$(CXX) $(CFLAGS) lime_capture.c $(OBJS) -o lime-capture -lpcap -lm -L/usr/lib64/mysql -lmysqlclient -lrt

util.o: util.c util.h lime_capture.h
	$(CXX) $(CFLAGS) -c util.c
decpcap.o: decpcap.c decpcap.h lime_capture.h util.h Packet.h connection.h
	$(CXX) $(CFLAGS) -c decpcap.c
Packet.o: Packet.cpp Packet.h lime_capture.h util.h
	$(CXX) $(CFLAGS) -c Packet.cpp
connection.o: connection.cpp connection.h Packet.h lime_capture.h util.h connection_hashMap.h cursor_mem_pool.h shared_queue.h
	$(CXX) $(CFLAGS) -c connection.cpp
connection_hashMap.o: connection_hashMap.c connection_hashMap.h cursor_mem_pool.h lime_capture.h
	$(CXX) $(CFLAGS) -c connection_hashMap.c
cursor_mem_pool.o: cursor_mem_pool.c cursor_mem_pool.h connection.h lime_capture.h
	$(CXX) $(CFLAGS) -c cursor_mem_pool.c 
shared_queue.o: shared_queue.c shared_queue.h
	$(CXX) $(CFLAGS) -c shared_queue.c

.PHONY: clean
clean:
	rm -f $(OBJS)
	rm -f lime-capture
