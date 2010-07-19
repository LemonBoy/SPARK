OBJECTS = doap_arc.o arc_paa.o arc_par.o common.o
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
