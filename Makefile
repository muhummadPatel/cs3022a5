CPP=g++
CPPFLAGS=-std=c++11 -g
EXE_NAME=samp

$(EXE_NAME):
	$(CPP) -o $(EXE_NAME) main.cpp audio.h $(CPPFLAGS)

test:
	$(CPP) -o testsamp test.cpp audio.h $(CPPFLAGS)
	./testsamp

clean:
	rm -f *.o $(EXE_NAME)
