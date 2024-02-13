ifndef prefix
	prefix=/opt/obs
endif
ifndef INC
	INC=-I/opt/amj/include
endif
ifndef LIB
	LIB=-L/opt/amj/lib
endif

LIBS = -lamjCom -lamjFourier -lamjInterferometry -lamjTime
LDFLAGS = $(LIB)

FLAGS=-Wall -g $(INC)
CPPFLAGS=$(FLAGS)

CPP=g++ $(CPPFLAGS)

SIMULATOR_SRC=simulator.C DelaylineSimulator.C PhasorsSim.C \
	ExternalDelaySimulator.C ../../shared/src/Help.C \
	../../shared/src/Wavelengths.C 
SIMULATOR_OBJ=$(SIMULATOR_SRC:.C=.o)

all: Simulator

Simulator: $(SIMULATOR_OBJ)
	$(CPP) -o $@ $^ $(LDFLAGS) $(LIBS)

install: all
	- mkdir -p $(prefix)/bin
	cp Simulator $(prefix)/bin

uninstall:
	- rm $(prefix)/bin/Simulator
	- rmdir $(prefix)/bin
	- rmdir $(prefix)

clean:
	- rm Simulator
	- rm $(SIMULATOR_OBJ)
