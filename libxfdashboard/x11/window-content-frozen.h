/*
 * window-content-frozen: Save last live frame to on-disk frozen cache
 *
 * Single responsibility: persist a live window texture as JPEG via
 * the thumbnail cache (best effort). Never saves fallback icon frames.
 *
 * Copyright 2025
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef __LIBXFDASHBOARD_WINDOW_CONTENT_FROZEN__
#define __LIBXFDASHBOARD_WINDOW_CONTENT_FROZEN__

#if !defined(__LIBXFDASHBOARD_H_INSIDE__) && !defined(LIBXFDASHBOARD_COMPILATION)
#error "Only <libxfdashboard/libxfdashboard.h> can be included directly."
#endif

#include <glib.h>
#include <cogl/cogl.h>
#include <X11/X.h>

typedef struct _XfdashboardWindowTrackerWindowX11				XfdashboardWindowTrackerWindowX11;

G_BEGIN_DECLS

void xfdashboard_window_content_frozen_pixbuf_free_data(guchar *inPixels, gpointer inUserData);

void xfdashboard_window_content_frozen_save(gboolean isFallback,
											CoglTexture *texture,
											Window xWindowID,
											XfdashboardWindowTrackerWindowX11 *window,
											Window *inoutFrozenCacheXID);

gboolean xfdashboard_window_content_frozen_should_save_damage(gint64 *lastUsec);

G_END_DECLS

#endif	/* __LIBXFDASHBOARD_WINDOW_CONTENT_FROZEN__ */
