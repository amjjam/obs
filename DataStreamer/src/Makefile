ifndef prefix
	prefix=/opt/obs
endif
ifndef INC
	INC=-I/opt/amj/include
endif
ifndef LIB
	LIB=-L/opt/amj/lib
endif

LIBS=-lamjCom -lamjFourierFITS -lamjTime -lamjArg
LDFLAGS=$(LIB)

FLAGS=-Wall -g $(INC)
CPPFLAGS=$(FLAGS)

CPP=g++ $(CPPFLAGS)

FOURIERSTREAMER_SRC=streamer.C
FOURIERSTREAMER_OBJ=$(FOURIERSTREAMER_SRC:.C=.o)

all: FourierStreamer

FourierStreamer: $(FOURIERSTREAMER_OBJ)
	$(CPP) -o $@ $^ $(LDFLAGS) $(LIBS)

install: all
	- mkdir -p $(prefix)/bin
	cp FourierStreamer $(prefix)/bin

uninstall:
	- rm $(prefix)/bin/FourierStreamer
	- rmdir $(prefix)/bin
	- rmdir $(prefix)

clean:
	- rm FourierStreamer
	- rm $(FOURIERSTREAMER_OBJ)

