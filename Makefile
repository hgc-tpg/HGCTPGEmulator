### DUMMY Makefile

#HDRDIR_EMUL = L1Trigger/L1THGCal/interface/backend_emulator/
#HDRDIR_TEST = HGCTPGEmulator/interface/
#HDRSUF = .h
##HDR =  $(foreachd, $(HDRDIR), $(wildcard $(HDRDIR)*$(HDRSUF)))
#HDR =  $(wildcard $(HDRDIR_EMUL)*$(HDRSUF)) $(wildcard $(HDRDIR_TEST)*$(HDRSUF))

SRCDIR_EMUL = L1Trigger/L1THGCal/src/backend_emulator/
SRCDIR_TEST = HGCTPGEmulator/src/
SRCSUF = .cc
#SRC = HGCTPGEmulatorTester_S1.cc $(foreachd, $(SRCDIR), $(wildcard $(SRCDIR)*$(SRCSUF)))
SRC = HGCTPGEmulatorTester_S1.cc $(wildcard $(SRCDIR_EMUL)*$(SRCSUF)) $(wildcard $(SRCDIR_TEST)*$(SRCSUF))

OBJSUF = .o
OBJDIR = obj
OBJ = HGCTPGEmulatorTester_S1.o $(patsubst %$(SRCSUF), %$(OBJSUF), $(SRC))

CC = g++
CCFLAGS = -std=c++1y -I./ -I../ -I$(HDRDIR_EMUL) -I$(HDRDIR_TEST)

##########################################################

%.o: %.cc #$(HDR)
	$(CC) -c -o $@ $< $(CCFLAGS)

test/HGCTPGEmulatorTester_S1: $(OBJ)
	$(CC) -o $@ $^ $(CCFLAGS)
	rm -f $(OBJ)

clean:
	rm -f $(OBJ)
	rm -f *.exe
	rm test/HGCTPGEmulatorTester_S1
