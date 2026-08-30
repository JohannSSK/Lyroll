CC = gcc
CFLAGS = -Wall -O2 -Iinclude -Ilibs/cJSON
LDFLAGS = -lraylib -lm -lpthread -ldl -lrt -lX11 -lcurl

SRCS = src/main.c src/generalgui.c src/menugui.c src/songgui.c src/audioNlyrics.c src/parsing.c src/timing.c libs/cJSON/cJSON.c
OBJS = $(SRCS:.c=.o)
TARGET = lyroll

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

install: $(TARGET)
	sudo mkdir -p /opt/Lyroll
	sudo cp -r . /opt/Lyroll/
	sudo ln -sf /opt/Lyroll/$(TARGET) /usr/local/bin/lyroll
	@echo "Installed to /opt/Lyroll"
	@echo "Run with: lyroll"

uninstall:
	sudo rm -rf /opt/Lyroll
	sudo rm -f /usr/local/bin/lyroll
	@echo "Uninstalled"

clean:
	rm -f $(OBJS) $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run install uninstall
