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

DRSOURCES:= Device.cc direader.cc
DRRESOURCES:= direader.rc
DROBJECTS:= $(DRSOURCES:.cc=.o) $(DRRESOURCES:.rc=-rc.o)

TWSOURCES:= Device.cc EMStat.cc tilt-wizard.cc
TWRESOURCES:= tilt-wizard.rc
TWOBJECTS:= $(TWSOURCES:.cc=.o) $(TWRESOURCES:.rc=-rc.o)
LIBS:= -ldinput8 -ldxguid -lole32


.PHONY: all clean doc release

all: tilt-wizard.exe direader.exe

tilt-wizard.exe: $(TWOBJECTS)
	g++ -static -o $@ $(CCFLAGS) $^ -mconsole $(LIBS)

direader.exe: $(DROBJECTS)
	g++ -static -o $@ $(CCFLAGS) $^ -mconsole $(LIBS)

%.asc: %
	-del $@
	$(GPG) --detach-sign --armor --local-user $(SIGNING_KEY) $<

%.o: %.cc Makefile
	g++ $(CCFLAGS) $(DEFINES) -MMD -c $<

%-rc.o: %.rc Makefile
	windres "-DGIT_VERSION=\"$(GIT_VERSION)\"" $< $@

clean:
	-del $(TWOBJECTS) $(TWSOURCES:.cc=.d) tilt-wizard.exe tilt-wizard.exe.asc
	-del $(DROBJECTS) $(DRSOURCES:.cc=.d) direader.exe direader.exe.asc
	-rmdir /s /q "docs\html"

doc:
	doxygen

release:
	$(MAKE) clean
	$(MAKE) tilt-wizard.exe tilt-wizard.exe.asc direader.exe direader.exe.asc
	$(GPG) --verify tilt-wizard.exe.asc
	$(GPG) --verify direader.exe.asc

include $(wildcard *.d)
