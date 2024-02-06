prefix=/opt/obs

LIBS=-lamjCom -lamjFourier -lamjInterferometry -lamjTime
LDFLAGS= -L/opt/amjCom/lib -L/opt/amj/lib

FLAGS=-Wall -g -I/opt/amjCom/include -I/opt/amj/include
CPPFLAGS=$(FLAGS)

CPP=g++ $(CPPFLAGS)

DATAPROCESSOR_SRC=processor.C DataProcessorBaselineSim.C \
	../../shared/src/Phasors.C 
DATAPROCESSOR_OBJ=$(DATAPROCESSOR_SRC:.C=.o)

all: DataProcessor

DataProcessor: $(DATAPROCESSOR_OBJ)
	$(CPP) -o $@ $^ $(LDFLAGS) $(LIBS)

install: all
	- mkdir -p $(prefix)/bin
	cp DataProcessor $(prefix)/bin

uninstall:
	- rm $(prefix)/bin/DataProcessor
	- rmdir $(prefix)/bin
	- rmdir $(prefix)

clean:
	- rm DataProcessor
	- rm $(DATAPROCESSOR_OBJ)
