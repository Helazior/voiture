CC = gcc
LD = gcc

CFLAGS = -Wall -Wextra -pedantic -Iinclude
#LINKER_FLAGS specifies the libraries we're linking against
LDFLAGS = -lSDL2 -lSDL2_image -lSDL2_ttf -lm
SRC_FILES=$(wildcard src/*.c)
# Par défaut, la compilation de src/toto.c génère le fichier objet obj/toto.o 
OBJ_FILES=$(patsubst src/%.c,obj/%.o,$(SRC_FILES))

#OBJ_NAME specifies the name of our exectuable
EXEC = exec

all : $(EXEC)

run : $(EXEC)
run : ;./exec

debug : CFLAGS += -DDEBUG -g3
debug : $(EXEC)
debug : ;gdb exec

opti : CFLAGS += -O3
opti : $(EXEC)

# to use gproftime : CFLAGS += -O3
gprof : CFLAGS += -pg
gprof : LDFLAGS += -pg
gprof : $(EXEC)

valgrind : CFLAGS += -g
valgrind : $(EXEC)
valgrind : ;valgrind ./exec

$(EXEC) : $(OBJ_FILES)
	$(LD) $(OBJ_FILES) $(LDFLAGS) -o $@

obj/%.o: src/%.c
	$(CC) -c $(CFLAGS) $< -o $@

.PHONY: clean
clean :
	rm -rf $(EXEC) $(OBJ_FILES)
