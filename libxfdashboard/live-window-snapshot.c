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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <libxfdashboard/live-window-snapshot.h>

#include <libxfdashboard/image-content.h>
#include <libxfdashboard/x11/window-tracker-window-x11.h>
#include <libxfdashboard/x11/window-tracker-x11.h>

#include <clutter/x11/clutter-x11.h>
#include <X11/Xlib.h>
#include <gtk/gtk.h>

/* Fallback icon: Wnck icon if available, otherwise a themed default so the
 * actor is never left empty (which renders as an invisible 0x0 actor).
 */
GdkPixbuf* xfdashboard_live_window_snapshot_get_icon(XfdashboardWindowTrackerWindow *inWindow)
{
	GdkPixbuf					*icon;

	g_return_val_if_fail(XFDASHBOARD_IS_WINDOW_TRACKER_WINDOW(inWindow), NULL);

	icon=xfdashboard_window_tracker_window_get_icon(inWindow);
	if(icon)
	{
		/* Wnck icon is borrowed - take our own reference for uniform cleanup */
		g_object_ref(icon);
		return(icon);
	}

	/* Wnck has no icon (common for never-mapped windows): use themed default */
	{
		GtkIconTheme			*theme;
		GError					*error;

		theme=gtk_icon_theme_get_default();
		if(!theme) return(NULL);

		error=NULL;
		icon=gtk_icon_theme_load_icon(theme,
										"application-x-executable",
										48,
										GTK_ICON_LOOKUP_USE_BUILTIN,
										&error);
		if(error) g_error_free(error);
	}

	return(icon);
}

/* Load frozen snapshot from /tmp/xfdashboard/win-<XID>.jpg if available.
 * Returns a new ClutterContent (to be unrefed) or NULL when no file/error.
 */
ClutterContent* xfdashboard_live_window_snapshot_load_frozen(XfdashboardWindowTrackerWindow *inWindow)
{
	gulong									xid;
	gchar									*path;
	GdkPixbuf								*pix;
	GError									*error;
	ClutterContent							*content;

	g_return_val_if_fail(XFDASHBOARD_IS_WINDOW_TRACKER_WINDOW(inWindow), NULL);

	/* Only X11 windows have an XID - safe cast with type check */
	if(!XFDASHBOARD_IS_WINDOW_TRACKER_WINDOW_X11(inWindow)) return(NULL);

	xid=xfdashboard_window_tracker_window_x11_get_xid(XFDASHBOARD_WINDOW_TRACKER_WINDOW_X11(inWindow));
	if(xid==0) return(NULL);

	path=g_strdup_printf("/tmp/xfdashboard/win-%lu.jpg", xid);
	if(!g_file_test(path, G_FILE_TEST_EXISTS))
	{
		g_free(path);
		return(NULL);
	}

	error=NULL;
	pix=gdk_pixbuf_new_from_file(path, &error);
	g_free(path);

	if(!pix)
	{
		if(error) g_error_free(error);
		return(NULL);
	}

	content=xfdashboard_image_content_new_for_pixbuf(pix);
	g_object_unref(pix);

	return(content);
}

/* Check if window is currently Viewable via XGetWindowAttributes.
 * Non-X11 windows are assumed viewable (keep live path).
 */
gboolean xfdashboard_live_window_snapshot_is_viewable(XfdashboardWindowTrackerWindow *inWindow)
{
	gulong									xid;
	Display									*display;
	XWindowAttributes						attrs;
	Status									status;
	gint									trapError;

	g_return_val_if_fail(XFDASHBOARD_IS_WINDOW_TRACKER_WINDOW(inWindow), FALSE);

	if(!XFDASHBOARD_IS_WINDOW_TRACKER_WINDOW_X11(inWindow)) return(TRUE);

	xid=xfdashboard_window_tracker_window_x11_get_xid(XFDASHBOARD_WINDOW_TRACKER_WINDOW_X11(inWindow));
	if(xid==0) return(FALSE);

	display=xfdashboard_window_tracker_x11_get_display();
	if(!display) return(FALSE);

	clutter_x11_trap_x_errors();
	status=XGetWindowAttributes(display, (Window)xid, &attrs);
	XSync(display, False);
	trapError=clutter_x11_untrap_x_errors();

	if(status==0 || trapError!=0) return(FALSE);

	return(attrs.map_state==IsViewable);
}
