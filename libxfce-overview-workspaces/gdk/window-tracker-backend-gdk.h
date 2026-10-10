/*
 * window-tracker-backend: Window tracker backend providing special functions
 *                         for different windowing and clutter backends.
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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_BACKEND_GDK__
#define __LIBXFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_BACKEND_GDK__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <glib-object.h>

#include <libxfce-overview-workspaces/window-tracker-backend.h>

G_BEGIN_DECLS

#define XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_TRACKER_BACKEND_GDK				(xfce_overview_workspaces_window_tracker_backend_gdk_get_type())
#define XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_BACKEND_GDK(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_TRACKER_BACKEND_GDK, XfceOverviewWorkspacesWindowTrackerBackendGDK))
#define XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_BACKEND_GDK(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_TRACKER_BACKEND_GDK))
#define XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_BACKEND_GDK_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_TRACKER_BACKEND_GDK, XfceOverviewWorkspacesWindowTrackerBackendGDKClass))
#define XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_BACKEND_GDK_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_TRACKER_BACKEND_GDK))
#define XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_BACKEND_GDK_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_TRACKER_BACKEND_GDK, XfceOverviewWorkspacesWindowTrackerBackendGDKClass))

typedef struct _XfceOverviewWorkspacesWindowTrackerBackendGDK				XfceOverviewWorkspacesWindowTrackerBackendGDK;
typedef struct _XfceOverviewWorkspacesWindowTrackerBackendGDKClass			XfceOverviewWorkspacesWindowTrackerBackendGDKClass;
typedef struct _XfceOverviewWorkspacesWindowTrackerBackendGDKPrivate		XfceOverviewWorkspacesWindowTrackerBackendGDKPrivate;

struct _XfceOverviewWorkspacesWindowTrackerBackendGDK
{
	/*< private >*/
	/* Parent instance */
	GObject											parent_instance;

	/* Private structure */
	XfceOverviewWorkspacesWindowTrackerBackendGDKPrivate		*priv;
};

struct _XfceOverviewWorkspacesWindowTrackerBackendGDKClass
{
	/*< private >*/
	/* Parent class */
	GObjectClass									parent_class;

	/*< public >*/
	/* Virtual functions */
};

/* Public API */
GType xfce_overview_workspaces_window_tracker_backend_gdk_get_type(void) G_GNUC_CONST;

XfceOverviewWorkspacesWindowTrackerBackend* xfce_overview_workspaces_window_tracker_backend_gdk_new(void);

G_END_DECLS

#endif	/* __LIBXFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_BACKEND_GDK__ */
