ifndef prefix
	prefix=/opt/obs
endif
ifndef INC
	INC=-I/opt/amj/include
endif
ifndef LIB
	LIB=-L/opt/amj/lib
endif

LIBS=-lamjCom -lamjTime
LDFLAGS= $(LIB)

FLAGS=-Wall -g $(INC)
CPPFLAGS=$(FLAGS)


#CPPFLAGS=-Wall -g -I/opt/amjCom/include -L/opt/amjCom/lib -I/opt/amj/include -L/opt/amj/lib
CPP=g++ $(CPPFLAGS)

TRACKER_SRC=tracker.C DelayMachine.C DelayMachineGD.C \
	FringeTrackerStateMachine.C \
	Baseline2Delayline.C Baseline2DelaylineLinear.C \
	../../shared/src/Phasors.C \
	../../shared/src/PowerSpectrum.C \
	../../shared/src/Help.C \
	../../shared/src/Wavelengths.C
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
