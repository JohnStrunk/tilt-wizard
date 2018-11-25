HAS_GIT:= $(shell git --version)
ifdef HAS_GIT
GIT_VERSION:= $(shell git describe --dirty --always --tags)
GIT_DATE:= $(shell git log -1 --format=%cd)
else
GIT_VERSION:= (unknown)
GIT_DATE:= (unknown)
endif

GPG:= gpg
SIGNING_KEY:= BD1FF508

CCFLAGS:= -Wall -Werror -ggdb -D_FORTIFY_SOURCE=2 -D_GLIBCXX_ASSERTIONS \
          -fasynchronous-unwind-tables -fexceptions -fpic \
          -fstack-clash-protection -grecord-gcc-switches -fcf-protection \
          -O2 -pipe
DEFINES:= "-DGIT_VERSION=$(GIT_VERSION)" "-DGIT_DATE=$(GIT_DATE)"

SOURCES:= EMStat.cc tilt-wizard.cc
RESOURCES:= tilt-wizard.rc
OBJECTS:= $(SOURCES:.cc=.o) $(RESOURCES:.rc=-rc.o)
LIBS:= -ldinput8 -ldxguid -lole32


.PHONY: all clean doc release

all: tilt-wizard.exe

tilt-wizard.exe: $(OBJECTS)
	g++ -static -o $@ $(CCFLAGS) $^ -mconsole $(LIBS)

%.asc: %
	-del $@
	$(GPG) --detach-sign --armor --local-user $(SIGNING_KEY) $<

%.o: %.cc Makefile
	g++ $(CCFLAGS) $(DEFINES) -c $<

%-rc.o: %.rc Makefile
	windres "-DGIT_VERSION=\"$(GIT_VERSION)\"" $< $@

clean:
	-del $(OBJECTS) tilt-wizard.exe tilt-wizard.exe.asc
	-rmdir /s /q "docs\html"

doc:
	doxygen

release:
	$(MAKE) clean
	$(MAKE) tilt-wizard.exe tilt-wizard.exe.asc
	$(GPG) --verify tilt-wizard.exe.asc
