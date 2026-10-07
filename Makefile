PREFIX=/usr/local
INSTALL_DIR=$(PREFIX)/bin
PERL_PREFIX=/usr/lib64/perl5/5.44
PERL_INSTALL_DIR=$(PERL_PREFIX)/CGVirt
CC = clang
CFLAGS = -std=c99 -Wall -Wextra -Wpedantic -Wunused-value -I/usr/include/libxml2
LDFLAGS = -lvirt -lxml2
SOURCES = src/cgvt.c src/xml.c
OBJECTS = $(SOURCES:.c=.o)
TARGET = cgvt

$(TARGET): $(OBJECTS)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

clean:
	rm -v cgvt src/*.o

install:
	mkdir -p $(PERL_INSTALL_DIR)
	install -m 0644 src/modules/CloudInit.pm $(PERL_INSTALL_DIR)
	install -m 0644 src/modules/IMGDownload.pm $(PERL_INSTALL_DIR)
	install -m 0644 src/modules/VMConfig.pm $(PERL_INSTALL_DIR)
	install -m 0755 src/cgv.pl $(INSTALL_DIR)/cgv
	install -m 0755 cgvt $(INSTALL_DIR)

uninstall:
	rm -vf $(INSTALL_DIR)/cgv
	rm -vrf $(PERL_INSTALL_DIR)
