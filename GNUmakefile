# Fork xfdashboard — wrapper Meson para install/uninstall rápido
# Uso: make [build|install|uninstall|clean|help]
#
# Por que GNUmakefile e não Makefile?
# `make` lê GNUmakefile primeiro. `Makefile` é gerado pelo autotools
# (configure.ac) e está ignorado no .gitignore, então seria sobrescrito.

BUILDDIR ?= build
PREFIX   ?= /usr/local
SUDO     ?= sudo

MESON   ?= meson
NINJA   ?= ninja

.PHONY: help setup build all install uninstall clean distclean reconfig

help:
	@echo "Alvos:"
	@echo "  make build     - configura (se preciso) e compila"
	@echo "  make install   - compila + instala em $(PREFIX) + ldconfig"
	@echo "  make uninstall - desinstala + ldconfig"
	@echo "  make clean     - limpa objetos (mantém pasta $(BUILDDIR))"
	@echo "  make distclean - apaga pasta $(BUILDDIR)"
	@echo "  make reconfig  - reconfigura prefixo"
	@echo "Vars: BUILDDIR=$(BUILDDIR) PREFIX=$(PREFIX) SUDO=$(SUDO)"
	@echo "Ex:  make install PREFIX=/usr SUDO=sudo"

setup:
	@if [ ! -f "$(BUILDDIR)/build.ninja" ]; then \
		$(MESON) setup $(BUILDDIR) --prefix=$(PREFIX); \
	else \
		echo "$(BUILDDIR)/ já configurado. Use 'make reconfig' para trocar prefixo."; \
	fi

reconfig:
	$(MESON) setup --reconfigure $(BUILDDIR) --prefix=$(PREFIX)

build all: setup
	$(MESON) compile -C $(BUILDDIR)

install: build
	$(SUDO) $(MESON) install -C $(BUILDDIR)
	$(SUDO) ldconfig
	@echo "OK. Teste: LD_LIBRARY_PATH=$(PREFIX)/lib/x86_64-linux-gnu xfdashboard --version"

uninstall:
	$(SUDO) $(NINJA) uninstall -C $(BUILDDIR) || $(SUDO) $(MESON) --internal uninstall -C $(BUILDDIR)
	$(SUDO) ldconfig
	@echo "OK desinstalado."

clean:
	$(NINJA) -C $(BUILDDIR) clean || true

distclean:
	rm -rf $(BUILDDIR)
