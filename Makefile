CCFLAGS= -Wall -Werror -ggdb

SOURCES= EMStat.cc tilt-wizard.cc
OBJECTS= $(SOURCES:.cc=.o)

.PHONY: clean

tilt-wizard.exe: $(OBJECTS)
	g++ -static -o $@ $(CCFLAGS) $^

%.o: %.cc
	g++ $(CCFLAGS) -c $^

clean:
	rm -f $(OBJECTS) tilt-wizard
