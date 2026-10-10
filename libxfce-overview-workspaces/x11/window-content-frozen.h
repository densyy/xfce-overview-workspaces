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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_WINDOW_CONTENT_FROZEN__
#define __LIBXFCE_OVERVIEW_WORKSPACES_WINDOW_CONTENT_FROZEN__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <glib.h>
#include <cogl/cogl.h>
#include <X11/X.h>

typedef struct _XfceOverviewWorkspacesWindowTrackerWindowX11				XfceOverviewWorkspacesWindowTrackerWindowX11;

G_BEGIN_DECLS

void xfce_overview_workspaces_window_content_frozen_pixbuf_free_data(guchar *inPixels, gpointer inUserData);

void xfce_overview_workspaces_window_content_frozen_save(gboolean isFallback,
											CoglTexture *texture,
											Window xWindowID,
											XfceOverviewWorkspacesWindowTrackerWindowX11 *window,
											Window *inoutFrozenCacheXID);

gboolean xfce_overview_workspaces_window_content_frozen_should_save_damage(gint64 *lastUsec);

G_END_DECLS

#endif	/* __LIBXFCE_OVERVIEW_WORKSPACES_WINDOW_CONTENT_FROZEN__ */
