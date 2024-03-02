BINARY = aws

CFLAGS = -std=gnu11
CFLAGS += -Og -g
CFLAGS += -Wall -Wextra -Wmissing-prototypes -Wstrict-prototypes
CFLAGS += -Wshadow

SOURCES = \
	src/main.c

OBJECTS = $(patsubst %.S,%.o,$(patsubst %.c,%.o,$(SOURCES)))

$(BINARY): $(OBJECTS)
	$(CC) -o $@ $(LDFLAGS) $^ $(LIBS)

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(BINARY) $(OBJECTS)

.PHONY: clean $(SOURCES)
