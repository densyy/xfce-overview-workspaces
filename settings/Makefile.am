AM_CPPFLAGS = \
	-I$(top_srcdir) \
	-DLIBXFCE_OVERVIEW_WORKSPACES_COMPILATION=1 \
	-DG_LOG_DOMAIN=\"xfce-overview-workspaces-settings\" \
	-DPACKAGE_DATADIR=\"$(datadir)\" \
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\" \
	-DPACKAGE_LIBDIR=\"$(libdir)\" \
	$(XFCE_OVERVIEW_WORKSPACES_DEBUG_CFLAGS) \
	$(PLATFORM_CPPFLAGS)

bin_PROGRAMS = \
	xfce-overview-workspaces-settings

xfce_overview_workspaces_settings_headers = \
	general.h \
	plugins.h \
	settings.h \
	themes.h

xfce_overview_workspaces_settings_SOURCES = \
	$(xfce_overview_workspaces_settings_headers) \
	general.c \
	main.c \
	plugins.c \
	settings.c \
	themes.c

xfce_overview_workspaces_settings_CFLAGS = \
	$(GTK_CFLAGS) \
	$(CLUTTER_CFLAGS) \
	$(LIBXFCONF_CFLAGS) \
	$(LIBXFCE4UTIL_CFLAGS) \
	$(LIBXFCE4UI_CFLAGS) \
	$(PLATFORM_CFLAGS)

xfce_overview_workspaces_settings_LDADD = \
	$(GTK_LIBS) \
	$(CLUTTER_LIBS) \
	$(LIBXFCONF_LIBS) \
	$(LIBXFCE4UTIL_LIBS) \
	$(LIBXFCE4UI_LIBS) \
	$(LIBM) \
	$(top_builddir)/common/libxfconf-settings.la \
	$(top_builddir)/libxfce-overview-workspaces/libxfce-overview-workspaces.la

xfce_overview_workspaces_settings_LDFLAGS = \
	-export-dynamic \
	-no-undefined \
	$(PLATFORM_LDFLAGS)

EXTRA_DIST = \
	meson.build \
	$(NULL)
