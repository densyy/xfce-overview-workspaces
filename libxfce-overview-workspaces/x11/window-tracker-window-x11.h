/*
 * window-tracker-window: A window tracked by window tracker and also
 *                        a wrapper class around WnckWindow.
 *                        By wrapping libwnck objects we can use a virtual
 *                        stable API while the API in libwnck changes
 *                        within versions. We only need to use #ifdefs in
 *                        window tracker object and nowhere else in the code.
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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW_X11__
#define __LIBXFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW_X11__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <glib-object.h>
#include <clutter/x11/clutter-x11.h>

#define WNCK_I_KNOW_THIS_IS_UNSTABLE
#include <libwnck/libwnck.h>

G_BEGIN_DECLS

#define XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_TRACKER_WINDOW_X11				(xfce_overview_workspaces_window_tracker_window_x11_get_type())
#define XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW_X11(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_TRACKER_WINDOW_X11, XfceOverviewWorkspacesWindowTrackerWindowX11))
#define XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_WINDOW_X11(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_TRACKER_WINDOW_X11))
#define XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW_X11_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_TRACKER_WINDOW_X11, XfceOverviewWorkspacesWindowTrackerWindowX11Class))
#define XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_WINDOW_X11_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_TRACKER_WINDOW_X11))
#define XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW_X11_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_TRACKER_WINDOW_X11, XfceOverviewWorkspacesWindowTrackerWindowX11Class))

typedef struct _XfceOverviewWorkspacesWindowTrackerWindowX11				XfceOverviewWorkspacesWindowTrackerWindowX11;
typedef struct _XfceOverviewWorkspacesWindowTrackerWindowX11Class			XfceOverviewWorkspacesWindowTrackerWindowX11Class;
typedef struct _XfceOverviewWorkspacesWindowTrackerWindowX11Private		XfceOverviewWorkspacesWindowTrackerWindowX11Private;

struct _XfceOverviewWorkspacesWindowTrackerWindowX11
{
	/*< private >*/
	/* Parent instance */
	GObject										parent_instance;

	/* Private structure */
	XfceOverviewWorkspacesWindowTrackerWindowX11Private	*priv;
};

struct _XfceOverviewWorkspacesWindowTrackerWindowX11Class
{
	/*< private >*/
	/* Parent class */
	GObjectClass								parent_class;

	/*< public >*/
	/* Virtual functions */
};

/* Public API */
GType xfce_overview_workspaces_window_tracker_window_x11_get_type(void) G_GNUC_CONST;

WnckWindow* xfce_overview_workspaces_window_tracker_window_x11_get_window(XfceOverviewWorkspacesWindowTrackerWindowX11 *self);
gulong xfce_overview_workspaces_window_tracker_window_x11_get_xid(XfceOverviewWorkspacesWindowTrackerWindowX11 *self);

G_END_DECLS

#endif	/* __LIBXFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW_X11__ */
