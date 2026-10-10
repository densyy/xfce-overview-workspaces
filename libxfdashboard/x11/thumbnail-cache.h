/*
 * thumbnail-cache: On-disk frozen cache of live window thumbnails
 *
 * Saves last live frame per X window ID as JPEG under /tmp/xfdashboard
 * using GdkPixbuf (no extra libjpeg dependency).
 *
 * Copyright 2025
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef __LIBXFDASHBOARD_THUMBNAIL_CACHE__
#define __LIBXFDASHBOARD_THUMBNAIL_CACHE__

#if !defined(__LIBXFDASHBOARD_H_INSIDE__) && !defined(LIBXFDASHBOARD_COMPILATION)
#error "Only <libxfdashboard/libxfdashboard.h> can be included directly."
#endif

#include <glib.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

G_BEGIN_DECLS

#define XFDASHBOARD_THUMBNAIL_CACHE_DIR		"/tmp/xfdashboard"

void xfdashboard_thumbnail_cache_ensure_dir_0700(void);

char* xfdashboard_thumbnail_cache_path_for_xid(gulong xid);

gboolean xfdashboard_thumbnail_cache_save_pixbuf(GdkPixbuf *pix, gulong xid);

void xfdashboard_thumbnail_cache_invalidate_xid(gulong xid);

G_END_DECLS

#endif
