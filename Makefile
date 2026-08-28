CC = gcc
CFLAGS = -Iinclude -Ilibs/raylib -Ilibs/raylib/external/glfw/include -DPLATFORM_DESKTOP -D_GLFW_X11
SRC = src/*.c libs/raylib/*.c
LDFLAGS = -lm -ldl -lpthread -lX11
TARGET = lyroll

all: $(TARGET)

$(TARGET):
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
