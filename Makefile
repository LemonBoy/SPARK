OBJECTS = doap_arc.o
OUTPUT = doap_arc
CFLAGS = -Wall -pedantic -g
LDFLAGS = -lz

all: $(OUTPUT)
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
$(OUTPUT): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $(OUTPUT) $(OBJECTS)
clean:
	$(RM) $(OUTPUT) $(OBJECTS)
