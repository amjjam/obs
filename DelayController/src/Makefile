FLAGS=-Wall -g -I/opt/amjCom/include -I/opt/amj/include -L/opt/amjCom/lib -L/opt/amj/lib
CPPFLAGS=$(FLAGS)

CPP=g++ $(CPPFLAGS)

DELAY_SRC=delay.C DelaylineInterfaceSimulator.C ../../shared/src/Help.C
DELAY_OBJ=$(DELAY_SRC:.C=.o)

DelayController: $(DELAY_OBJ)
	$(CPP) -o $@ $^ -lamjCom -lamjTime

clean:
	- rm DelayController $(DELAY_OBJ)
