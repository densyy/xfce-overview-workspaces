SUBDIRS = \
	metainfo \
	data \
	icons \
	themes

desktopdir = $(datadir)/applications
desktop_in_files = org.xfce.xfce-overview-workspaces.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)

autostartdir = $(sysconfdir)/xdg/autostart
autostart_in_files = org.xfce.xfce-overview-workspaces-autostart.desktop.in
autostart_DATA = $(autostart_in_files:.desktop.in=.desktop)

settingsdir = $(datadir)/applications
settings_in_files = org.xfce.xfce-overview-workspaces-settings.desktop.in
settings_DATA = $(settings_in_files:.desktop.in=.desktop)

# Substitute bindir so it works when built in a non-system prefix
org.xfce.xfce-overview-workspaces.desktop.in:org.xfce.xfce-overview-workspaces.desktop.in.in
	$(AM_V_GEN) sed -e "s|@bindir[@]|$(bindir)|" \
	    -e "s|@VERSION[@]|$(VERSION)|" \
	    $< > $@ || rm $@

org.xfce.xfce-overview-workspaces-autostart.desktop.in:org.xfce.xfce-overview-workspaces-autostart.desktop.in.in
	$(AM_V_GEN) sed -e "s|@bindir[@]|$(bindir)|" \
	    -e "s|@VERSION[@]|$(VERSION)|" \
	    $< > $@ || rm $@

org.xfce.xfce-overview-workspaces-settings.desktop.in:org.xfce.xfce-overview-workspaces-settings.desktop.in.in
	$(AM_V_GEN) sed -e "s|@bindir[@]|$(bindir)|" \
	    -e "s|@VERSION[@]|$(VERSION)|" \
	    $< > $@ || rm $@

%.desktop: %.desktop.in
	$(AM_V_GEN)$(MSGFMT) --desktop --template $< -d $(top_srcdir)/po -o $@

EXTRA_DIST = \
	meson.build \
	$(desktop_in_files:.desktop.in=.desktop.in.in) \
	$(autostart_in_files:.desktop.in=.desktop.in.in) \
	$(settings_in_files:.desktop.in=.desktop.in.in)

CLEANFILES = \
	$(desktop_in_files) \
	$(desktop_DATA) \
	$(autostart_in_files) \
	$(autostart_DATA) \
	$(settings_in_files) \
	$(settings_DATA)
