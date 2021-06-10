CC = gcc
LD = gcc

#OBJS specifies which files to compile as part of the project
OBJS = main.c

COMPILER_FLAGS = -Wall -Wextra -std=c99 -Iinclude -O0

#LINKER_FLAGS specifies the libraries we're linking against
LINKER_FLAGS = -lSDL2 -lSDL2_image -lSDL2_ttf -lm
SRC_FILES=$(wildcard src/*.c)
# Par défaut, la compilation de src/toto.c génère le fichier objet obj/toto.o 
OBJ_FILES=$(patsubst src/%.c,obj/%.o,$(SRC_FILES))

#OBJ_NAME specifies the name of our exectuable
EXEC = exec

all : $(EXEC)

debug : COMPILER_FLAGS += -DDEBUG -g3
debug : $(EXEC)

opti : COMPILER_FLAGS += -O3
opti : $(EXEC)

# to use gproftime : COMPILER_FLAGS += -O3
gprof : COMPILER_FLAGS += -pg
#gprof : LINKER_FLAGS += -pg
gprof : $(EXEC)

valgrind : COMPILER_FLAGS += -g
valgrind : $(EXEC)

$(EXEC) : $(OBJ_FILES)
	$(LD) $(OBJ_FILES) $(LDFLAGS) -o $@

obj/%.o: src/%.c 	$(CC) -c $(CFLAGS) $< -o $@

.PHONY: clean
clean :
	rm -rf $(EXEC) $(OBJ_FILES)
