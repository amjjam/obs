ifndef prefix
	prefix=/opt/obs
endif
ifndef INC
	INC=-I/opt/amj/include
endif
ifndef LIB
	LIB=-L/opt/amj/lib
endif

LIBS=-lamjCom -lamjFourier -lamjInterferometry -lamjTime
LDFLAGS= $(LIB)

FLAGS=-Wall -g $(INC)
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
