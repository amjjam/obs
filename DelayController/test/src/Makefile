
FLAGS=-Wall -g -I/opt/amj/include
CPPFLAGS=$(FLAGS)
CPP=g++ $(CPPFLAGS)

TESTMROIDL_SRC=testmroidl.C ../../src/DelaylineInterfaceMROI.C
TESTMROIDL_OBJ=$(TESTMROIDL_SRC:.C=.o)

testmroidl: $(TESTMROIDL_OBJ)
	$(CPP) -o $@ $^ -L/opt/amj/lib -lamjCom

clean:
	- rm testmroidl
	- rm $(TESTMROIDL_OBJ)
