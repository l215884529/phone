CC = gcc
CFLAGS = -Wall
LDLIBS = -lm -pthread
OBJS = i1i2i3_phone_tcp.o die.o fft.o sound.o

i1i2i3_phone_tcp: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDLIBS)

clean:
	rm -f *~ *.o i1i2i3_phone_tcp
