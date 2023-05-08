FLAGS=-Wall -g -I/opt/amjCom/include -L/opt/amjCom/lib
CPPFLAGS=$(FLAGS)

CPP=g++ $(CPPFLAGS)

DELAY_SRC=delay.C ../../shared/src/Help.C
DELAY_OBJ=$(DELAY_SRC:.C=.o)

DelayController: $(DELAY_OBJ)
	$(CPP) -o $@ $^ -lamjCom

clean:
	- rm DelayController $(DELAY_OBJ)
