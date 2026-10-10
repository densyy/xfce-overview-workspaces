/*
 * live-window-snapshot: Single-responsibility helper to obtain an X11
 *                       window snapshot (live check, frozen file, icon).
 *
 * Copyright 2012-2021 Stephan Haller <nomad@froevel.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 *
 */

#ifndef __LIBXFDASHBOARD_LIVE_WINDOW_SNAPSHOT__
#define __LIBXFDASHBOARD_LIVE_WINDOW_SNAPSHOT__

#if !defined(__LIBXFDASHBOARD_H_INSIDE__) && !defined(LIBXFDASHBOARD_COMPILATION)
#error "Only <libxfdashboard/libxfdashboard.h> can be included directly."
#endif

#include <clutter/clutter.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include <libxfdashboard/window-tracker.h>

G_BEGIN_DECLS

ClutterContent* xfdashboard_live_window_snapshot_load_frozen(XfdashboardWindowTrackerWindow *inWindow);
gboolean xfdashboard_live_window_snapshot_is_viewable(XfdashboardWindowTrackerWindow *inWindow);
GdkPixbuf* xfdashboard_live_window_snapshot_get_icon(XfdashboardWindowTrackerWindow *inWindow);

G_END_DECLS

#endif	/* __LIBXFDASHBOARD_LIVE_WINDOW_SNAPSHOT__ */
