TARGET = embarrassed
OBJECTS = embarrassed.o
LIBS = -lpthread -lrt -lm
CFLAGS = -Wall

.PHONY: default all clean

SOURCE  = embarrasingly_parrallel.c

default: $(TARGET) 
all: default

$(OBJECTS): $(SOURCE) 
	gcc $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJECTS)
	gcc $(OBJECTS) -Wall $(LIBS) -o $@

clean:
	-rm -f *.o
	-rm -f $(TARGET)
