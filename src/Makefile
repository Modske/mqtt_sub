LDFLAGS:=-lmosquitto -luci -largp -lsqlite3 -ljson-c -lcurl
CFLAGS:=-ggdb3
SRCS:=$(wildcard ./*.c)
OBJS:=$(SRCS:.c=.o)
BIN:=mqtt_sub

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	$(RM) $(OBJS) $(BIN)