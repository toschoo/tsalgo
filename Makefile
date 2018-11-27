# =========================================================================
# Makefile for TSALGO
# (c) Tobias Schoofs, 1998 -- 2017
# =========================================================================
CC = @gcc
CXX = @g++
AR = @ar

LNKMSG = @printf "linking   $@\n"
CMPMSG = @printf "compiling $@\n"

FLGMSG = @printf "CFLAGS: $(CFLAGS)\nLDFLAGS: $(LDFLAGS)\n"

INSMSG = @printf ". setenv.sh"

CFLAGS = -O3 -g -Wall -std=c99 -fPIC -D_GNU_SOURCE -D_POSIX_C_SOURCE=200809L
LDFLAGS = -L./lib

INC = -I./include -I./test
LIB = lib

SRC = src/tsalgo
HDR = include/tsalgo
TST = test
TOOLS = tools
RSC = rsc
OUTLIB = lib

OBJ = $(SRC)/tree.o \
      $(SRC)/list.o \
      $(SRC)/bufsort.o \
      $(SRC)/listsort.o \
      $(SRC)/filesort.o \
      $(SRC)/lru.o 

DEP = $(SRC)/tree.c $(HDR)/tree.h \
      $(SRC)/lru.c $(HDR)/lru.h \
      $(SRC)/list.c $(HDR)/list.h $(SRC)/listsort.c \
      $(SRC)/bufsort.c $(HDR)/bufsort.h \
      $(SRC)/filesort.c $(HDR)/filesort.h

default:	lib \
		treerandom \
		treesmoke  \
		lrurandom  \
		treebench  \
		listrandom \
		sortrandom \
		fsortsmoke \
		fsortrandom 

all:	default tools

tools:	treei \
	shuffle \
	readkeys

debug:	CFLAGS += -g
debug:	default
debug:	tools

install:	$(OUTLIB)/libtsalgo.so
		cp $(OUTLIB)/libtsalgo.so /usr/local/lib/
		cp -r include/tsalgo /usr/local/include/

run:	treerandom treebench treesmoke \
	listrandom lrurandom           \
	sortrandom fsortrandom fsortsmoke
	$(TST)/listrandom
	$(TST)/treerandom
	$(TST)/treebench
	$(TST)/treesmoke
	$(TST)/lrurandom
	$(TST)/sortrandom
	$(TST)/fsortsmoke
	$(TST)/fsortrandom

flags:
	$(FLGMSG)

.SUFFIXES: .c .o

.c.o:	$(DEP)
	$(CMPMSG)
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<

.cpp.o:	$(DEP)
	$(CMPMSG)
	$(CXX) $(CFLAGS) $(INC) -c -o $@ $<

# Tests and demos
binomtree:	$(TST)/binomtree
treesmoke:	$(TST)/treesmoke
treerandom:	$(TST)/treerandom
treebench:	$(TST)/treebench
lrurandom:	$(TST)/lrurandom
listrandom:	$(TST)/listrandom
sortrandom:	$(TST)/sortrandom
fsortsmoke:	$(TST)/fsortsmoke
fsortrandom:	$(TST)/fsortrandom
tstprogress:	$(TST)/tstprogress
random:		$(TST)/random

# Tools
readkeys:	$(TOOLS)/readkeys
treei:		$(TOOLS)/treei
shuffle:	$(TOOLS)/shuffle

# Library
lib:	lib/libtsalgo.so

lib/libtsalgo.so:	$(OBJ) $(DEP)
			$(LNKMSG)
			$(CC) -shared \
			      -o $(OUTLIB)/libtsalgo.so \
			         $(SRC)/list.o     \
			         $(SRC)/bufsort.o  \
			         $(SRC)/filesort.o \
			         $(SRC)/listsort.o \
			         $(SRC)/tree.o \
			         $(SRC)/lru.o \
			         -lm
			
# Tests and demos
$(TST)/treesmoke:	$(OBJ) $(DEP) lib $(TST)/treesmoke.o
			$(LNKMSG)
			$(CC) $(LDFLAGS) -o $(TST)/treesmoke $(TST)/treesmoke.o -lm -ltsalgo

$(TST)/binomtree:	$(OBJ) $(DEP) lib $(TST)/binomtree.o $(SRC)/random.o
			$(LNKMSG)
			$(CC) $(LDFLAGS) -o $(TST)/binomtree \
			                     $(SRC)/random.o \
					     $(TST)/binomtree.o -lm -ltsalgo

$(TST)/fsortsmoke:	$(OBJ) $(DEP) lib $(SRC)/random.o $(TST)/fsortsmoke.o
			$(LNKMSG)
			$(CC) $(LDFLAGS) -o $(TST)/fsortsmoke \
			                    $(SRC)/random.o   \
			                    $(TST)/fsortsmoke.o -lm -ltsalgo

$(TST)/treerandom:	$(OBJ) $(DEP) lib $(TST)/progress.o \
			                  $(TST)/treerandom.o $(SRC)/random.o
			$(LNKMSG)
			$(CC) $(LDFLAGS) -o $(TST)/treerandom \
			                    $(SRC)/random.o   \
			                    $(TST)/progress.o \
			                    $(TST)/treerandom.o -lm -ltsalgo

$(TST)/lrurandom:	$(OBJ) $(DEP) lib $(TST)/progress.o \
			                  $(TST)/lrurandom.o $(SRC)/random.o
			$(LNKMSG)
			$(CC) $(LDFLAGS) -o $(TST)/lrurandom \
			                    $(SRC)/random.o   \
			                    $(TST)/progress.o \
			                    $(TST)/lrurandom.o -lm -ltsalgo

$(TST)/random:		$(OBJ) $(DEP) lib $(TST)/progress.o \
			                  $(SRC)/random.o   \
			                  $(TST)/random.o
			$(LNKMSG)
			$(CC) $(LDFLAGS) -o $(TST)/random     \
			                    $(TST)/progress.o \
			                    $(SRC)/random.o   \
			                    $(TST)/random.o -lm

$(TST)/treebench:	$(OBJ) $(DEP) lib $(TST)/treebench.o $(TST)/progress.o \
			                  $(SRC)/random.o
			$(LNKMSG)
			$(CC) $(LDFLAGS) -o $(TST)/treebench  \
			                    $(SRC)/random.o   \
			                    $(TST)/progress.o \
			                    $(TST)/treebench.o -ltsalgo -lm


$(TST)/listrandom:	$(OBJ) $(DEP) lib $(TST)/listrandom.o $(SRC)/random.o
			$(LNKMSG)
			$(CC) $(LDFLAGS) -o $(TST)/listrandom \
			                    $(SRC)/random.o   \
			                    $(TST)/listrandom.o -lm -ltsalgo

$(TST)/sortrandom:	$(OBJ) $(DEP) lib $(TST)/sortrandom.o \
			                  $(TST)/progress.o $(SRC)/random.o
			$(LNKMSG)
			$(CC) $(LDFLAGS) -o $(TST)/sortrandom \
			                    $(SRC)/random.o   \
			                    $(TST)/progress.o \
			                    $(TST)/sortrandom.o -lm -ltsalgo

$(TST)/fsortrandom:	$(OBJ) $(DEP) lib $(TST)/fsortrandom.o $(SRC)/random.o
			$(LNKMSG)
			$(CC) $(LDFLAGS) -o $(TST)/fsortrandom \
			                    $(SRC)/random.o    \
			                    $(TST)/fsortrandom.o -lm -ltsalgo

$(TST)/tstprogress:	$(TST)/tstprogress.o $(TST)/progress.o
			$(LNKMSG)
			$(CC) $(LDFLAGS) -o $(TST)/tstprogress \
			                    $(TST)/progress.o  \
			                    $(TST)/tstprogress.o

# Tools
$(TOOLS)/readkeys:	$(DEP) lib $(TOOLS)/readkeys.o
			$(LNKMSG)
			$(CC) $(LDFLAGS) -o $(TOOLS)/readkeys $(TOOLS)/readkeys.o

$(TOOLS)/treei:		$(OBJ) lib $(DEP) $(TOOLS)/treei.o
			$(LNKMSG)
			$(CC) $(LDFLAGS) -o $(TOOLS)/treei $(TOOLS)/treei.o -lm -ltsalgo

$(TOOLS)/shuffle:	$(TOOLS)/shuffle.o $(SRC)/random.o
			$(LNKMSG)
			$(CC) $(LDFLAGS) -o $(TOOLS)/shuffle \
			                    $(SRC)/random.o  \
			                    $(TOOLS)/shuffle.o -lm

# Clean up
clean:
	rm -f $(SRC)/*.o
	rm -f $(TST)/*.o
	rm -f $(TOOLS)/*.o
	rm -f $(TST)/treesmoke
	rm -f $(TST)/treerandom
	rm -f $(TST)/treebench
	rm -f $(TST)/lrurandom
	rm -f $(TST)/binomtree
	rm -f $(TST)/listrandom
	rm -f $(TST)/sortrandom
	rm -f $(TST)/fsortrandom
	rm -f $(TST)/fsortsmoke
	rm -f $(TST)/tstprogress
	rm -f $(TST)/radnom
	rm -f $(TOOLS)/readkeys
	rm -f $(TOOLS)/shuffle
	rm -f $(TOOLS)/treei
	rm -f $(RSC)/*.bin
	rm -f $(OUTLIB)/libtsalgo.so

