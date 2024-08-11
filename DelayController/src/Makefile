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
LDFLAGS=$(LIB)

FLAGS=-Wall -g $(INC)
CPPFLAGS=$(FLAGS)

CPP=g++ $(CPPFLAGS)

DELAY_SRC=delay.C DelaylineInterfaceSimulator.C DelaylineInterfaceMROI.C \
	../../shared/src/Help.C
DELAY_OBJ=$(DELAY_SRC:.C=.o)

all: DelayController

DelayController: $(DELAY_OBJ)
	$(CPP) -o $@ $^ $(LDFLAGS) $(LIBS)

install: all
	- mkdir -p $(prefix)/bin
	cp DelayController $(prefix)/bin

uninstall:
	- rm $(prefix)/bin/DelayController
	- rmdir $(prefix)/bin
	- rmdir $(prefix)

clean:
	- rm DelayController $(DELAY_OBJ)

distclean: clean
