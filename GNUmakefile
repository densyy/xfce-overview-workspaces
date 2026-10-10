# Fork xfdashboard — wrapper Meson para build/install/autostart rápido
# Uso num PC zerado (Debian/Ubuntu com Xfce):
#   make deps      # dependências de build (precisa sudo/apt)
#   make install   # compila + instala em $(PREFIX) + ldconfig
#   make autostart # daemon no login
#   make on        # mostra a faixa agora
# Uso: make [deps|build|install|autostart|on|off|uninstall|clean|help]
#
# Por que GNUmakefile e não Makefile?
# `make` lê GNUmakefile primeiro. `Makefile` é gerado pelo autotools
# (configure.ac) e está ignorado no .gitignore, então seria sobrescrito.

BUILDDIR ?= build
PREFIX   ?= /usr/local
SUDO     ?= sudo
AUTOSTART_DIR ?= $(HOME)/.config/autostart

MESON   ?= meson
NINJA   ?= ninja

.PHONY: help deps setup build all install autostart on off uninstall clean distclean reconfig

help:
	@echo "Alvos:"
	@echo "  make deps      - instala dependências de build (Debian/Ubuntu)"
	@echo "  make build     - configura (se preciso) e compila"
	@echo "  make install   - compila + instala em $(PREFIX) + ldconfig"
	@echo "  make autostart - registra o daemon no login"
	@echo "  make on        - sobe o daemon e mostra a faixa"
	@echo "  make off       - desliga o daemon"
	@echo "  make uninstall - desinstala + ldconfig"
	@echo "  make clean     - limpa objetos (mantém pasta $(BUILDDIR))"
	@echo "  make distclean - apaga pasta $(BUILDDIR)"
	@echo "  make reconfig  - reconfigura prefixo"
	@echo "Vars: BUILDDIR=$(BUILDDIR) PREFIX=$(PREFIX) SUDO=$(SUDO)"
	@echo "Ex:  make install PREFIX=/usr SUDO=sudo"

deps:
	sudo apt install git gcc meson ninja-build pkg-config \
	  libwnck-3-dev libgtk-3-dev libgarcon-1-0-dev libxfconf-0-dev \
	  libxfce4ui-2-dev libxfce4util-dev libcogl-dev libclutter-1.0-dev \
	  libclutter-gtk-1.0-dev libx11-dev libxcomposite-dev libxdamage-dev \
	  libxinerama-dev

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

autostart:
	mkdir -p $(AUTOSTART_DIR)
	printf '%s\n' \
	  '[Desktop Entry]' \
	  'Type=Application' \
	  'Name=xfdashboard' \
	  'Comment=Faixa de workspaces no topo (daemon)' \
	  'Exec=$(PREFIX)/bin/xfdashboard -d' \
	  'Hidden=false' \
	  'X-GNOME-Autostart-enabled=true' \
	  > $(AUTOSTART_DIR)/xfdashboard.desktop
	@echo "autostart em $(AUTOSTART_DIR)/xfdashboard.desktop"

on:
	-$(PREFIX)/bin/xfdashboard -d
	$(PREFIX)/bin/xfdashboard -t

off:
	-$(PREFIX)/bin/xfdashboard -q

uninstall:
	$(SUDO) $(NINJA) uninstall -C $(BUILDDIR) || $(SUDO) $(MESON) --internal uninstall -C $(BUILDDIR)
	$(SUDO) ldconfig
	@echo "OK desinstalado."

clean:
	$(NINJA) -C $(BUILDDIR) clean || true

distclean:
	rm -rf $(BUILDDIR)
