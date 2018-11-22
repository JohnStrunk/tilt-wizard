HAS_GIT:= $(shell git --version)
ifdef HAS_GIT
GIT_VERSION:= $(shell git describe --dirty --always --tags)
GIT_DATE:= $(shell git log -1 --format=%cd)
else
GIT_VERSION:= (unknown)
GIT_DATE:= (unknown)
endif

CCFLAGS:= -Wall -Werror -ggdb -D_FORTIFY_SOURCE=2 -D_GLIBCXX_ASSERTIONS \
          -fasynchronous-unwind-tables -fexceptions -fpic \
          -fstack-clash-protection -grecord-gcc-switches -fcf-protection \
          -O2 -pipe
DEFINES:= "-DGIT_VERSION=$(GIT_VERSION)" "-DGIT_DATE=$(GIT_DATE)"

SOURCES:= EMStat.cc tilt-wizard.cc
RESOURCES:= tilt-wizard.rc
OBJECTS:= $(SOURCES:.cc=.o) $(RESOURCES:.rc=-rc.o)
LIBS:= -ldinput8 -ldxguid -lole32


.PHONY: clean

tilt-wizard.exe: $(OBJECTS)
	g++ -static -o $@ $(CCFLAGS) $^ -mconsole $(LIBS)

%.o: %.cc Makefile
	g++ $(CCFLAGS) $(DEFINES) -c $<

%-rc.o: %.rc Makefile
	windres "-DGIT_VERSION=\"$(GIT_VERSION)\"" $< $@

clean:
	-del $(OBJECTS) tilt-wizard.exe
