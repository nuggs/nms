CC          = gcc
RM	    = rm
EXE         = merc
NATIVEFLAG  = -march=native
OPTIMIZE    = -O2
PROF        = -g

C_FLAGS = -Wall $(NATIVEFLAG) $(OPTIMIZE) $(PROF)
L_FLAGS = $(NATIVEFLAG) $(OPTIMIZE) $(PROF) -lcrypt

C_FILES := $(wildcard *.c)
O_FILES = act_comm.o act_info.o act_move.o act_obj.o act_wiz.o comm.o const.o \
          db.o fight.o handler.o interp.o magic.o protocol.o parson.o save.o \
	  special.o update.o

merc: $(O_FILES)
	$(RM) -f $(EXE)
	$(CC) -o $(EXE) $(O_FILES) $(L_FLAGS)

.c.o: merc.h
	$(CC) -c $(C_FLAGS) $<

clean:
	$(RM) -f *.o $(EXE)
