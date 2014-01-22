INCDIR=
LIBDIR=
LIBS=-ltbbmalloc -ltbb
CC=g++
#DEFINES=-DTBB_USE_CAPTURED_EXCEPTION=1

edsl: edsl.h main.cpp
	$(CC) -std=c++0x -g $(DEFINES) $(INCDIR) $(LIBDIR) main.cpp $(LIBS) -o edsl 

