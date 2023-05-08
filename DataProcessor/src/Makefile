FLAGS=-Wall -g -I/opt/amjCom/include -L/opt/amjCom/lib
CPPFLAGS=$(FLAGS)

CPP=g++ $(CPPFLAGS)

DATAPROCESSOR_SRC=processor.C DataProcessorBaselineSim.C \
	../../shared/src/Phasors.C 
DATAPROCESSOR_OBJ=$(DATAPROCESSOR_SRC:.C=.o)

all: DataProcessor

DataProcessor: $(DATAPROCESSOR_OBJ)
	$(CPP) -o $@ $^ -lamjCom

clean:
	- rm DataProcessor
	- rm $(DATAPROCESSOR_OBJ)
