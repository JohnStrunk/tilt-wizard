CCFLAGS= -Wall -Werror -ggdb

SOURCES= EMStat.cc tilt-wizard.cc
OBJECTS= $(SOURCES:.cc=.o)
LIBS= -ldinput8 -ldxguid

.PHONY: clean

tilt-wizard.exe: $(OBJECTS)
	g++ -static -o $@ $(CCFLAGS) $^ -mconsole $(LIBS)

%.o: %.cc
	g++ $(CCFLAGS) -c $^

clean:
	del $(OBJECTS) tilt-wizard.exe
