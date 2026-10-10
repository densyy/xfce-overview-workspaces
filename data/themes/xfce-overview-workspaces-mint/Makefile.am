themedir = $(datadir)/themes/xfce-overview-workspaces-mint/xfce-overview-workspaces-1.0

theme_in_files = \
	xfce-overview-workspaces.theme.desktop.in

theme_DATA = \
	$(theme_in_files:.theme.desktop.in=.theme) \
	xfce-overview-workspaces-mint.css \
	xfce-overview-workspaces.css \
	xfce-overview-workspaces.xml \
	xfce-overview-workspaces-secondary.xml \
	animations.xml \
	effects.xml \
	screenshot-xfce-overview-workspaces-mint.png

# Substitute bindir so it works when built in a non-system prefix
xfce-overview-workspaces.theme.desktop.in:xfce-overview-workspaces.theme.desktop.in.in
	$(AM_V_GEN) sed -e "s|@PACKAGE_VERSION[@]|$(PACKAGE_VERSION)|" \
	    $< > $@ || rm $@

xfce-overview-workspaces.theme: xfce-overview-workspaces.theme.desktop.in
	$(AM_V_GEN)$(MSGFMT) --desktop --template $< -d $(top_srcdir)/po -o $@

CLEANFILES = \
	$(theme_in_files) \
	$(theme_in_files:.theme.desktop.in=.theme)

EXTRA_DIST = \
	meson.build \
	$(theme_in_files:.theme.desktop.in=.theme.desktop.in.in) \
	$(theme_DATA)

DISTCLEANFILES = \
	$(theme_in_files) \
	$(theme_in_files:.theme.desktop.in=.theme)
