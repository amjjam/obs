CPPFLAGS=-Wall -g -I/opt/amjCom/include -L/opt/amjCom/lib
CPP=g++ $(CPPFLAGS)

SIMULATOR_SRC=simulator.C DelaylineSimulator.C PhasorsSim.C \
	../../shared/src/Help.C \
	../../shared/src/Wavelengths.C \
	../../shared/src/Phasors.C
SIMULATOR_OBJ=$(SIMULATOR_SRC:.C=.o)

all: simulator

simulator: $(SIMULATOR_OBJ)
	$(CPP) -o $@ $^ -lamjCom

clean:
	- rm simulator $(SIMULATOR_OBJ)
