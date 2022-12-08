### HGCTPGEmulatorTester_S1 Makefile

SRCDIR_EMUL = L1Trigger/L1THGCal/src/backend_emulator/
SRCDIR_TEST = HGCTPGStage1Emulator/src/
BINDIR = HGCTPGStage1Emulator/bin/
TESTDIR = HGCTPGStage1Emulator/test/
INCLUDEDIR = HGCTPGStage1Emulator/include/

SRCSUF = .cc
SRC = $(TESTDIR)HGCTPGStage1EmulatorTester.cc $(wildcard $(SRCDIR_EMUL)*$(SRCSUF)) $(wildcard $(SRCDIR_TEST)*$(SRCSUF))

OBJSUF = .o
OBJDIR = obj
OBJ = $(TESTDIR)HGCTPGStage1EmulatorTester.o $(patsubst %$(SRCSUF), %$(OBJSUF), $(SRC))

CC = g++
CCFLAGS = -std=c++17 -I./ -I../ -I./$(INCLUDEDIR)

##########################################################

%.o: %.cc
	$(CC) -c -o $@ $< $(CCFLAGS)

$(BINDIR)HGCTPGStage1EmulatorTester.exe: $(OBJ)
	echo $(CCFLAGS)
	$(CC) -o $@ $^ $(CCFLAGS)
	rm -f $(OBJ)

clean:
	rm -f $(OBJ)
	rm -f $(BINDIR)*.exe

