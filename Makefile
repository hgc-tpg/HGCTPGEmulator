### HGCTPGEmulatorTester_S1 Makefile

SRCDIR_EMUL = L1Trigger/L1THGCal/src/backend_emulator/
SRCDIR_TEST = HGCTPGEmulator/src/
SRCSUF = .cc
SRC = HGCTPGEmulatorTester_S1.cc $(wildcard $(SRCDIR_EMUL)*$(SRCSUF)) $(wildcard $(SRCDIR_TEST)*$(SRCSUF))

OBJSUF = .o
OBJDIR = obj
OBJ = HGCTPGEmulatorTester_S1.o $(patsubst %$(SRCSUF), %$(OBJSUF), $(SRC))

CC = g++
CCFLAGS = -std=c++1y -I./ -I../ $$(pkg-config nlohmann_json --cflags)

##########################################################

%.o: %.cc
	$(CC) -c -o $@ $< $(CCFLAGS)

test/HGCTPGEmulatorTester_S1: $(OBJ)
	echo $(CCFLAGS)
	$(CC) -o $@ $^ $(CCFLAGS)
	rm -f $(OBJ)

clean:
	rm -f $(OBJ)
	rm -f *.exe
	rm test/HGCTPGEmulatorTester_S1
