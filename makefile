#OBJS specifies which files to compile as part of the project
OBJS = main.c

COMPILER_FLAGS = -Wall -Wextra -pedantic

#LINKER_FLAGS specifies the libraries we're linking against
LINKER_FLAGS = -lSDL2 -lSDL2_image -lSDL2_ttf -lm
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

$(EXEC) : $(OBJS) jeu.c jeu.h ia.c ia.h background.c
	gcc -o $@ $(OBJS) jeu.c ia.c background.c $(LINKER_FLAGS) $(COMPILER_FLAGS)

clean :
	rm *.o

mrproper : clean
	rm $(EXEC)
