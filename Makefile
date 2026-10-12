PREFIX=/usr/local
INSTALL_DIR=$(PREFIX)/bin
PERL_PREFIX=/usr/lib64/perl5/5.44
PERL_INSTALL_DIR=$(PERL_PREFIX)/Cobia
CC = clang
CFLAGS = -std=c99 -Wall -Wextra -Wpedantic -Wunused-value -I/usr/include/libxml2
LDFLAGS = -lvirt -lxml2 -lyaml
SOURCES = src/cobia.c src/xml.c src/virt.c
OBJECTS = $(SOURCES:.c=.o)
TARGET = cobia

$(TARGET): $(OBJECTS)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

clean:
	rm -v cobia src/*.o

install:
	mkdir -p $(PERL_INSTALL_DIR)
	install -m 0644 src/modules/*.pm $(PERL_INSTALL_DIR)
	install -m 0755 src/cobia-cfg.pl $(INSTALL_DIR)/cobia-cfg
	install -m 0755 cobia $(INSTALL_DIR)

uninstall:
	rm -vf $(INSTALL_DIR)/cobia-cfg
	rm -vf $(INSTALL_DIR)/cobia
	rm -vrf $(PERL_INSTALL_DIR)
