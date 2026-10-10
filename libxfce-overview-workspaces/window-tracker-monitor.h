/*
 * window-tracker-monitor: A monitor object tracked by window tracker.
 *                         It provides information about position and
 *                         size of monitor within screen and also a flag
 *                         if this monitor is the primary one.
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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_MONITOR__
#define __LIBXFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_MONITOR__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <glib-object.h>

G_BEGIN_DECLS

#define XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_TRACKER_MONITOR					(xfce_overview_workspaces_window_tracker_monitor_get_type())
#define XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_MONITOR(obj)					(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_TRACKER_MONITOR, XfceOverviewWorkspacesWindowTrackerMonitor))
#define XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_MONITOR(obj)				(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_TRACKER_MONITOR))
#define XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_MONITOR_GET_IFACE(obj)		(G_TYPE_INSTANCE_GET_INTERFACE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_TRACKER_MONITOR, XfceOverviewWorkspacesWindowTrackerMonitorInterface))

typedef struct _XfceOverviewWorkspacesWindowTrackerMonitor					XfceOverviewWorkspacesWindowTrackerMonitor;
typedef struct _XfceOverviewWorkspacesWindowTrackerMonitorInterface		XfceOverviewWorkspacesWindowTrackerMonitorInterface;

struct _XfceOverviewWorkspacesWindowTrackerMonitorInterface
{
	/*< private >*/
	/* Parent class */
	GTypeInterface						parent_interface;

	/*< public >*/
	/* Virtual functions */
	gboolean (*is_equal)(XfceOverviewWorkspacesWindowTrackerMonitor *inLeft, XfceOverviewWorkspacesWindowTrackerMonitor *inRight);

	gboolean (*is_primary)(XfceOverviewWorkspacesWindowTrackerMonitor *self);
	gint (*get_number)(XfceOverviewWorkspacesWindowTrackerMonitor *self);

	void (*get_geometry)(XfceOverviewWorkspacesWindowTrackerMonitor *self, gint *outX, gint *outY, gint *outWidth, gint *outHeight);

	/* Signals */
	void (*primary_changed)(XfceOverviewWorkspacesWindowTrackerMonitor *self);
	void (*geometry_changed)(XfceOverviewWorkspacesWindowTrackerMonitor *self);
};

/* Public API */
GType xfce_overview_workspaces_window_tracker_monitor_get_type(void) G_GNUC_CONST;

gboolean xfce_overview_workspaces_window_tracker_monitor_is_equal(XfceOverviewWorkspacesWindowTrackerMonitor *inLeft,
														XfceOverviewWorkspacesWindowTrackerMonitor *inRight);

gint xfce_overview_workspaces_window_tracker_monitor_get_number(XfceOverviewWorkspacesWindowTrackerMonitor *self);

gboolean xfce_overview_workspaces_window_tracker_monitor_is_primary(XfceOverviewWorkspacesWindowTrackerMonitor *self);

void xfce_overview_workspaces_window_tracker_monitor_get_geometry(XfceOverviewWorkspacesWindowTrackerMonitor *self,
														gint *outX,
														gint *outY,
														gint *outWidth,
														gint *outHeight);
gboolean xfce_overview_workspaces_window_tracker_monitor_contains(XfceOverviewWorkspacesWindowTrackerMonitor *self,
														gint inX,
														gint inY);

G_END_DECLS

#endif	/* __LIBXFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_MONITOR__ */
