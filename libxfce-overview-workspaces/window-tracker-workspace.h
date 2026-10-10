/*
 * window-tracker-workspace: A workspace tracked by window tracker.
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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WORKSPACE__
#define __LIBXFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WORKSPACE__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <glib-object.h>

G_BEGIN_DECLS

#define XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_TRACKER_WORKSPACE				(xfce_overview_workspaces_window_tracker_workspace_get_type())
#define XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WORKSPACE(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_TRACKER_WORKSPACE, XfceOverviewWorkspacesWindowTrackerWorkspace))
#define XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_WORKSPACE(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_TRACKER_WORKSPACE))
#define XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WORKSPACE_GET_IFACE(obj)		(G_TYPE_INSTANCE_GET_INTERFACE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_TRACKER_WORKSPACE, XfceOverviewWorkspacesWindowTrackerWorkspaceInterface))

typedef struct _XfceOverviewWorkspacesWindowTrackerWorkspace				XfceOverviewWorkspacesWindowTrackerWorkspace;
typedef struct _XfceOverviewWorkspacesWindowTrackerWorkspaceInterface		XfceOverviewWorkspacesWindowTrackerWorkspaceInterface;

struct _XfceOverviewWorkspacesWindowTrackerWorkspaceInterface
{
	/*< private >*/
	/* Parent interface */
	GTypeInterface						parent_interface;

	/*< public >*/
	/* Virtual functions */
	gboolean (*is_equal)(XfceOverviewWorkspacesWindowTrackerWorkspace *inLeft, XfceOverviewWorkspacesWindowTrackerWorkspace *inRight);

	gint (*get_number)(XfceOverviewWorkspacesWindowTrackerWorkspace *self);
	const gchar* (*get_name)(XfceOverviewWorkspacesWindowTrackerWorkspace *self);

	void (*get_size)(XfceOverviewWorkspacesWindowTrackerWorkspace *self, gint *outWidth, gint *outHeight);

	gboolean (*is_active)(XfceOverviewWorkspacesWindowTrackerWorkspace *self);
	void (*activate)(XfceOverviewWorkspacesWindowTrackerWorkspace *self);

	/* Signals */
	void (*name_changed)(XfceOverviewWorkspacesWindowTrackerWorkspace *self);
};

/* Public API */
GType xfce_overview_workspaces_window_tracker_workspace_get_type(void) G_GNUC_CONST;

gboolean xfce_overview_workspaces_window_tracker_workspace_is_equal(XfceOverviewWorkspacesWindowTrackerWorkspace *inLeft,
														XfceOverviewWorkspacesWindowTrackerWorkspace *inRight);

gint xfce_overview_workspaces_window_tracker_workspace_get_number(XfceOverviewWorkspacesWindowTrackerWorkspace *self);
const gchar* xfce_overview_workspaces_window_tracker_workspace_get_name(XfceOverviewWorkspacesWindowTrackerWorkspace *self);

void xfce_overview_workspaces_window_tracker_workspace_get_size(XfceOverviewWorkspacesWindowTrackerWorkspace *self,
													gint *outWidth,
													gint *outHeight);

gboolean xfce_overview_workspaces_window_tracker_workspace_is_active(XfceOverviewWorkspacesWindowTrackerWorkspace *self);
void xfce_overview_workspaces_window_tracker_workspace_activate(XfceOverviewWorkspacesWindowTrackerWorkspace *self);

G_END_DECLS

#endif	/* __LIBXFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WORKSPACE__ */
