AM_CPPFLAGS = \
	-I$(top_srcdir) \
	-DLIBXFCE_OVERVIEW_WORKSPACES_COMPILATION=1 \
	-DG_LOG_DOMAIN=\"xfce-overview-workspaces\" \
	-DPACKAGE_DATADIR=\"$(datadir)\" \
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\" \
	-DPACKAGE_LIBDIR=\"$(libdir)\" \
	$(XFCE_OVERVIEW_WORKSPACES_DEBUG_CFLAGS) \
	$(PLATFORM_CPPFLAGS)

bin_PROGRAMS = \
	xfce-overview-workspaces

xfce_overview_workspaces_headers = \
	application.h

xfce_overview_workspaces_SOURCES = \
	$(xfce_overview_workspaces_headers) \
	application.c \
	main.c

xfce_overview_workspaces_CFLAGS = \
	$(CLUTTER_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(LIBXFCONF_CFLAGS) \
	$(GARCON_CFLAGS) \
	$(LIBXFCE4UTIL_CFLAGS) \
	$(LIBXFCE4UI_CFLAGS) \
	$(GTK_CFLAGS) \
	$(PLATFORM_CFLAGS)

xfce_overview_workspaces_LDADD = \
	$(CLUTTER_LIBS) \
	$(GLIB_LIBS) \
	$(LIBXFCONF_LIBS) \
	$(GARCON_LIBS) \
	$(LIBXFCE4UTIL_LIBS) \
	$(LIBXFCE4UI_LIBS) \
	$(GTK_LIBS) \
	$(top_builddir)/common/libxfconf-settings.la \
	$(top_builddir)/libxfce-overview-workspaces/libxfce-overview-workspaces.la

xfce_overview_workspaces_LDFLAGS = \
	$(PLATFORM_LDFLAGS) \
	-export-dynamic \
	-no-undefined

EXTRA_DIST = \
	meson.build \
	$(NULL)
