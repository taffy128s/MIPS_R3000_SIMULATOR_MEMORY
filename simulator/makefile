CC = gcc -Wall
SRCS = ./*.c
OBS = ./*.o

# The following line means that do SRC first.
CMP: SRC
	$(CC) -o $@ $(OBS)

SRC: $(SRCS)
	$(CC) -c $(SRCS)

clean: $(OBS)
	rm $(OBS) CMP