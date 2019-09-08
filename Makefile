INCLUDE_DIR = ./include
SOURCE_DIR = ./source
OBJECT_DIR = ./object

CC = gcc
CFLAGS=-g -I$(INCLUDE_DIR) -lcrypto -lssl
OUT = hw2

_DEPS = ErrorCodes.h Globals.h InQueue.h OutQueue.h ProcessC.h ProcessP.h SharedMem.h
#pattern matching from  _DEPS to include directory
DEPS = $(patsubst %,$(INCLUDE_DIR)/%,$(_DEPS))

_OBJ = InQueue.o main.o OutQueue.o ProcessC.o ProcessP.o SharedMem.o
#same pattern matching principe
OBJ = $(patsubst %,$(OBJECT_DIR)/%,$(_OBJ))

.PHONY: clean

############goals#######################

#general rule for all object files
$(OBJECT_DIR)/%.o: $(SOURCE_DIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

#default goal
$(OUT): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

count:
	wc $(SOURCE_DIR)/*.c $(DEPS)

clean:
	rm -f $(OBJECT_DIR)/*.o ./hw2
