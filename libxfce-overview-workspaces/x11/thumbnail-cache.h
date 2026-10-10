/*
 * thumbnail-cache: On-disk frozen cache of live window thumbnails
 *
 * Saves last live frame per X window ID as JPEG under /tmp/xfce-overview-workspaces
 * using GdkPixbuf (no extra libjpeg dependency).
 *
 * Copyright 2025
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_THUMBNAIL_CACHE__
#define __LIBXFCE_OVERVIEW_WORKSPACES_THUMBNAIL_CACHE__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <glib.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

G_BEGIN_DECLS

#define XFCE_OVERVIEW_WORKSPACES_THUMBNAIL_CACHE_DIR		"/tmp/xfce-overview-workspaces"

void xfce_overview_workspaces_thumbnail_cache_ensure_dir_0700(void);

char* xfce_overview_workspaces_thumbnail_cache_path_for_xid(gulong xid);

gboolean xfce_overview_workspaces_thumbnail_cache_save_pixbuf(GdkPixbuf *pix, gulong xid);

void xfce_overview_workspaces_thumbnail_cache_invalidate_xid(gulong xid);

G_END_DECLS

#endif
