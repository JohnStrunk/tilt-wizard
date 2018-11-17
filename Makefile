CCFLAGS= -Wall -Werror -ggdb -D_FORTIFY_SOURCE=2 -D_GLIBCXX_ASSERTIONS \
         -fasynchronous-unwind-tables -fexceptions -fpic \
         -fstack-clash-protection -grecord-gcc-switches -fcf-protection \
         -O2 -pipe

SOURCES= EMStat.cc tilt-wizard.cc
OBJECTS= $(SOURCES:.cc=.o)
LIBS= -ldinput8 -ldxguid -lole32

.PHONY: clean

tilt-wizard.exe: $(OBJECTS)
	g++ -static -o $@ $(CCFLAGS) $^ -mconsole $(LIBS)

%.o: %.cc Makefile
	g++ $(CCFLAGS) -c $<

clean:
	-del $(OBJECTS) tilt-wizard.exe
