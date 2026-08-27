#   compiler we are using
#   obviously gcc
CC = gcc
#   flags for gcc itself
CFLAGS =
#   flags we would need for libraries
#   only lraylib is that important here
LDFLAGS = -lraylib -lm -lpthread -ldl -lrt -lX11

#   these are the .c files we are expecting
#   this is just a variable
SRCS = main.c audio.c parse.c timing.c gui.c
#   object files we are expecting, same ones of course
#   so we use a quick prompt that adds prefix build/
#   and replaces .c with .o
OBJS = $(addprefix build/, $(SRCS:.c=.o))
#   where our executable will be
TARGET = bin/lyroll

#   all depends on target
#   needed for "make " to work
all: $(TARGET)



#   main building block
#   % just mean any file, same on both sides
#   so this line means, all files called xyz.c in src/
#   will be made as xyz.o in build/
#   syntax just means, build/xyz.o depends on src/xyz.c
#   when all good (all files are made as .o) enter the block
build/%.o: src/%.c
#   in the block
#   make build/ if it doesn't exist
	mkdir -p build
#   $< means the first dependency
#   $@ means the other side of dependencies
#   gcc -flags -needed -for -gcc    -c  main.c  -o xyz.o...
	$(CC) $(CFLAGS) -c $< -o $@
	@echo "Built $@ from $<. Prepared object files in /build."




#   target depends on object
#   meaning, if objects don't exist, make them with build
#   when all is good, enter the block
$(TARGET): $(OBJS)
#   if all if good, we are here
#   make new bin directory
#   -p is for protection if it already exists
	mkdir -p bin
#   gcc objectFile.o    -o     target/path      -flags -i - wanted
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)
	@echo "Built $(TARGET) executable. Go to path to run it."




#   for make clean
#   deleted build and bin directory
#   never touch src
clean:
	rm -rf build bin
	@echo "Cleaned up unneeded directories."

#   required for compiler to know that all and clean are arguments, not file names
.PHONY: all clean
