all: lispy
clean:
	rm -f lispy *.o
.PHONY: all clean

# -c is implicit, you don't need it (it *shouldn't* be there)
# CC is also implicit, you don't need it
CFLAGS := -std=c99 -g -Wall -Wextra
LIBS = -ledit -lm

lispy: prompt.o mpc.o lval.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

# Make will automatically generate the necessary commands
# You just need to name which headers each file depends on
# (You can make the dependencies automatic but this is simpler)
# prompt.o: prompt.c #header.h
