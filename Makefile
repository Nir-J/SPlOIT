SRCDIR = src
BINDIR = bin
INCLUDES = include

CC = g++
CFLAGS = -fno-stack-protector -z execstack -pthread -std=c++11 -I $(INCLUDES)
DEPS = $(wildcard $(INCLUDES)/%.h)

all: dir $(BINDIR)/client $(BINDIR)/server $(DEPS)

#$(BINDIR)/sploit: $(SRCDIR)/sploit.cpp
#	$(CC) $(CFLAGS) $< -o $@

${BINDIR}:
	mkdir -p ${BINDIR}

dir: ${BINDIR}

$(BINDIR)/client: $(SRCDIR)/client.cpp 
	$(CC) $(CFLAGS) $< $(SRCDIR)/sploit.cpp -o $@

$(BINDIR)/server: $(SRCDIR)/server.cpp
	$(CC) $(CFLAGS) $< $(SRCDIR)/sploit.cpp -o $@

.PHONY: dir clean
clean:
	rm -rf $(BINDIR)

