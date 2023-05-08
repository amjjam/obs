CPPFLAGS=-Wall -g -I/opt/amjCom/include -L/opt/amjCom/lib
CPP=g++ $(CPPFLAGS)

TRACKER_SRC=tracker.C DelayMachine.C DelayMachineGD.C \
	FringeTrackerStateMachine.C \
	Baseline2Delayline.C Baseline2DelaylineLinear.C \
	../../shared/src/Phasors.C \
	../../shared/src/PowerSpectrum.C \
	../../shared/src/Help.C \
	../../shared/src/Wavelengths.C
TRACKER_OBJ=$(TRACKER_SRC:.C=.o)

all: tracker

tracker: $(TRACKER_OBJ)
	$(CPP) -o $@ $^ -lamjCom

clean:
	- rm tracker $(TRACKER_OBJ)

