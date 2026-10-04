PREFIX=/usr/local
INSTALL_DIR=$(PREFIX)/bin
PERL_PREFIX=/usr/lib64/perl5/5.44
PERL_INSTALL_DIR=$(PERL_PREFIX)/CGVirt

install:
	mkdir -p $(PERL_INSTALL_DIR)
	install -m 0644 src/modules/CloudInit.pm $(PERL_INSTALL_DIR)
	install -m 0644 src/modules/IMGDownload.pm $(PERL_INSTALL_DIR)
	install -m 0644 src/modules/VMConfig.pm $(PERL_INSTALL_DIR)
	install -m 0755 src/cgv.pl $(INSTALL_DIR)/cgv

uninstall:
	rm -vf $(INSTALL_DIR)/cgv
	rm -vrf $(PERL_INSTALL_DIR)
