[![License](https://img.shields.io/badge/License-GPL%20v2-blue.svg)](https://gitlab.xfce.org/apps/xfdashboard/-/blob/master/COPYING)

# xfdashboard

> Fork "workspace-overview": em vez do dashboard central estilo Exposé, este
> branch mostra apenas uma **faixa no topo com 6 quadradinhos** (um por
> workspace), cada um com **live preview / foto congelada / ícone** das janelas
> dele. Arrastar um programa de um quadradinho para outro **move a janela de
> workspace**. A visão central (`viewpad`) foi removida dos 7 temas; o palco
> nunca é fullscreen (largura da tela × ~22% da altura).

## Arquitetura (responsabilidade única por arquivo)

Novos módulos extraídos (cada um com um escopo):

- `libxfdashboard/live-window-snapshot.[hc]` — obter snapshot de janela X11:
  `is_viewable` (XGetWindowAttributes), `load_frozen` (`/tmp/xfdashboard/`),
  `get_icon` (Wnck + fallback `application-x-executable`).
- `libxfdashboard/live-window-simple.c` — só orquestra
  live → frozen → ícone → conteúdo (nunca deixa o ator vazio).
- `libxfdashboard/x11/thumbnail-cache.[hc]` — paths e JPG q75 via GdkPixbuf
  em `/tmp/xfdashboard/win-<XID>.jpg` (chave = XID cliente), `0700`, sem purge
  no boot; invalida no close/resize.
- `libxfdashboard/x11/window-content-frozen.[hc]` — persistir o frame frozen
  (resume + damage com throttle de 5s).
- `libxfdashboard/x11/window-content-x11.c` — só live textures
  (composite/damage/suspend-resume). No resume (`core` resumido) retoma
  **todas** as janelas, não só `isMapped` — janelas pré-existentes (sem
  `MapNotify`) também aparecem.
- `xfdashboard/restart.[hc]` — restart `-r`/`-q` via dbus; `xfdashboard/toolkit-init.[hc]`
  — init do toolkit; `xfdashboard/main.c` — só backend-env + run.

## Guia: instalar, buildar, rodar, toggle

Tudo passa pelo `GNUmakefile` (`make help` lista os alvos).

### Instalar (PC zerado, Debian/Ubuntu com Xfce)

    % make deps       # dependências de build (apt)
    % sudo make install   # compila + instala em /usr/local + ldconfig
    % make autostart  # daemon no login (~/.config/autostart)

### Buildar ajustes que eu fizer

    % make build      # recompila o que mudou (ou: meson compile -C build)

Depois de rebuildar, o daemon continua rodando o binário velho — reinicie:

    % xfdashboard -q  # desliga o daemon atual
    % xfdashboard -d  # sobe o novo instalado

(Se rodar sem install, use o binário do build: `./build/xfdashboard/xfdashboard -d`.
Nesse caso os temas do fonte valem via `XFDASHBOARD_THEME_PATH=data/themes`
ou copiando `data/themes/*/` para `~/.local/share/themes/*/xfdashboard-1.0/`
— que sombreia o instalado.)

### Rodar o daemon

    % xfdashboard -d  # bifurca para segundo plano (uma vez por login; o autostart já faz)

### Toggle (mostra / esconde a faixa)

    % xfdashboard -t  # alterna a visibilidade da faixa dos 6 workspaces

A faixa fica no topo (largura da tela × ~22% da altura). Arrastar um programa
de um quadradinho para outro move a janela de workspace.

---

Xfdashboard is maybe a Gnome shell and macOS Expose like dashboard for Xfce.

**Xfdashboard** provides a GNOME shell dashboard and macOS Mission Control
(e.g. Exposé and Spaces) like interface for use with the Xfce desktop. It can
be configured to run with any keyboard shortcut and when executed provides an
overview of applications currently open enabling the user to switch between
different applications.

----

### Homepage

[Xfdashboard documentation](https://docs.xfce.org/apps/xfdashboard/start)

### Changelog

See [NEWS](https://gitlab.xfce.org/apps/xfdashboard/-/blob/master/NEWS) for details on changes and fixes made in the current release.

### Source Code Repository

[xfdashboard source code](https://gitlab.xfce.org/apps/xfdashboard)

### Download a Release Tarball

[Xfdashboard archive](https://archive.xfce.org/src/apps/xfdashboard)
    or
[Xfdashboard tags](https://gitlab.xfce.org/apps/xfdashboard/-/tags)

### Installation

Use o `GNUmakefile` deste fork (seção "Guia" acima):

    % make deps && sudo make install && make autostart

Equivalente manual: `meson setup build`, `meson compile -C build`,
`sudo meson install -C build`.

### Uninstallation

    % sudo ninja uninstall -C build   # ou: make uninstall

### Reporting Bugs

Visit the [reporting bugs](https://docs.xfce.org/apps/xfdashboard/bugs) page to view currently open bug reports and instructions on reporting new bugs or submitting bugfixes.
