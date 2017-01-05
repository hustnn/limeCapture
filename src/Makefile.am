bin_PROGRAMS = lime_capture
lime_capture_SOURCES = Packet.cpp connection.cpp connection_hashMap.c \
	conninode.cpp cursor_mem_pool.c decpcap.c decpcap.cpp inode2prog.cpp lime_capture.c \
	process.cpp shared_queue.c util.c

# set the include path found by configure
INCLUDES= $(all_includes)

# the library search path.
lime_capture_LDFLAGS = $(all_libraries) 
noinst_HEADERS = Connection.h Packet.h connection.h connection_hashMap.h \
	conninode.h cursor_mem_pool.h decpcap.h inode2prog.h lime_capture.h process.h \
	shared_queue.h util.h
