ifndef prefix
	prefix=/opt/obs
endif
ifndef INC
	INC=-I/opt/amj/include -I/opt/amj/include/amjCom
endif
ifndef LIB
	LIB=-L/opt/amj/lib
endif

LIBS=-lamjCom -lamjTime -lamjArg -lamjInterferometry
LDFLAGS= $(LIB)

FLAGS=-Wall -g $(INC) -std=c++20
CPPFLAGS=$(FLAGS)

CPP=g++ $(CPPFLAGS)

TRACKER_SRC=tracker.C 	StateMachine.C \
	../../protocol/src/FringeTrackerProtocol.C

TRACKER_OBJ=$(TRACKER_SRC:.C=.o)

all: FringeTracker

FringeTracker: $(TRACKER_OBJ)
	$(CPP) -o $@ $^ $(LDFLAGS) $(LIBS)

install: all
	- mkdir -p $(prefix)/bin
	cp FringeTracker $(prefix)/bin

uninstall:
	- rm $(prefix)/bin/FringeTracker
	- rmdir $(prefix)/bin
	- rmdir $(prefix)

clean:
	- rm FringeTracker
	- rm $(TRACKER_OBJ)

distclean: clean
