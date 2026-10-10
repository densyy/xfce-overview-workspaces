# Fork xfce-overview-workspaces — wrapper Meson para build/install/autostart rápido
# Uso num PC zerado (Arch ou Debian/Ubuntu com Xfce):
#   make deps      # dependências de build (precisa sudo; detecta pacman ou apt)
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
	@echo "  make deps      - instala dependências (detecta pacman/Arch ou apt/Debian)"
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
	@if [ -f /etc/arch-release ] || grep -qs '^ID=arch' /etc/os-release 2>/dev/null; then \
		sudo pacman -S --needed git gcc meson ninja pkgconf \
		  libwnck3 gtk3 garcon xfconf \
		  libxfce4ui libxfce4util libx11 libxcomposite libxdamage \
		  libxinerama; \
		if command -v yay >/dev/null 2>&1; then \
			yay -S --needed cogl clutter clutter-gtk; \
		elif command -v paru >/dev/null 2>&1; then \
			paru -S --needed cogl clutter clutter-gtk; \
		else \
			echo "No Arch, cogl/clutter/clutter-gtk sao AUR: instale yay ou paru e rode: yay -S cogl clutter clutter-gtk"; \
		fi; \
	else \
		sudo apt install git gcc meson ninja-build pkg-config \
		  libwnck-3-dev libgtk-3-dev libgarcon-1-0-dev libxfconf-0-dev \
		  libxfce4ui-2-dev libxfce4util-dev libcogl-dev libclutter-1.0-dev \
		  libclutter-gtk-1.0-dev libx11-dev libxcomposite-dev libxdamage-dev \
		  libxinerama-dev; \
	fi

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
	@echo "OK. Teste: LD_LIBRARY_PATH=$(PREFIX)/lib/x86_64-linux-gnu xfce-overview-workspaces --version"

autostart:
	mkdir -p $(AUTOSTART_DIR)
	printf '%s\n' \
	  '[Desktop Entry]' \
	  'Type=Application' \
	  'Name=xfce-overview-workspaces' \
	  'Comment=Faixa de workspaces no topo (daemon)' \
	  'Exec=$(PREFIX)/bin/xfce-overview-workspaces -d' \
	  'Hidden=false' \
	  'X-GNOME-Autostart-enabled=true' \
	  > $(AUTOSTART_DIR)/xfce-overview-workspaces.desktop
	@echo "autostart em $(AUTOSTART_DIR)/xfce-overview-workspaces.desktop"

on:
	-$(PREFIX)/bin/xfce-overview-workspaces -d
	$(PREFIX)/bin/xfce-overview-workspaces -t

off:
	-$(PREFIX)/bin/xfce-overview-workspaces -q

uninstall:
	$(SUDO) $(NINJA) uninstall -C $(BUILDDIR) || $(SUDO) $(MESON) --internal uninstall -C $(BUILDDIR)
	$(SUDO) ldconfig
	@echo "OK desinstalado."

clean:
	$(NINJA) -C $(BUILDDIR) clean || true

distclean:
	rm -rf $(BUILDDIR)
