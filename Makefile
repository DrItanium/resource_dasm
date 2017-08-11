COMMON_OBJECTS=resource_fork.o util.o
REALMZ_DASM_OBJECTS=realmz_dasm.o realmz_lib.o $(COMMON_OBJECTS)
RESOURCE_DUMP_OBJECTS=resource_dump.o $(COMMON_OBJECTS)
CXXFLAGS=-I/opt/local/include -g -Wall -std=c++14
LDFLAGS=-L/opt/local/lib -lphosg
EXECUTABLES=realmz_dasm

all: realmz_dasm resource_dump

realmz_dasm: $(REALMZ_DASM_OBJECTS)
	g++ $(LDFLAGS) -o realmz_dasm $^

resource_dump: $(RESOURCE_DUMP_OBJECTS)
	g++ $(LDFLAGS) -o resource_dump $^

clean:
	-rm -f *.o $(EXECUTABLES)

.PHONY: clean
