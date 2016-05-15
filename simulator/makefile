CC = g++ -Wall
SRCS = ./*.c
SRCSS = ./*.cpp
OBS = ./*.o

# The following line means that do SRC first.
CMP: SRC SRCC
	$(CC) -o $@ $(OBS)

SRC: $(SRCS)
	$(CC) -c $(SRCS)

SRCC: $(SRCSS)
	$(CC) -c $(SRCSS)

clean: $(OBS)
	rm $(OBS) CMP