metainfodir = $(datadir)/metainfo
metainfo_DATA = $(metainfo_in_files:.xml.in=.xml)
metainfo_in_files = org.xfce.xfce-overview-workspaces.metainfo.xml.in

org.xfce.xfce-overview-workspaces.metainfo.xml: org.xfce.xfce-overview-workspaces.metainfo.xml.in
	$(AM_V_GEN)$(MSGFMT) --xml --template $< -d $(top_srcdir)/po -o $@

EXTRA_DIST = \
        $(metainfo_DATA) \
        meson.build \
        org.xfce.xfce-overview-workspaces.metainfo.xml.in

DISTCLEANFILES = \
        $(metainfo_in_files:.xml.in=.xml)
