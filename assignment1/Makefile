TARGET = embarrassed
OBJECTS = $(SOURCE:.c=.o)
LIBS = -lpthread -lrt -lm
CFLAGS = -Wall

.PHONY: default all clean
HEADERS = work_distribution.h
SOURCE  = embarrasingly_parrallel.c \
	work_distribution.c

default: $(TARGET) 
all: default

%.o: %.c 
	gcc $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJECTS)
	gcc $(OBJECTS) -Wall $(LIBS) -o $@

clean:
	-rm -f *.o
	-rm -f $(TARGET)
