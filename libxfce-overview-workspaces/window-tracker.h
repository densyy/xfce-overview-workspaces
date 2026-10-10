/*
 * window-tracker: Tracks windows, workspaces, monitors and
 *                 listens for changes.
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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER__
#define __LIBXFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <glib-object.h>

#include <libxfce-overview-workspaces/window-tracker-window.h>
#include <libxfce-overview-workspaces/window-tracker-workspace.h>
#include <libxfce-overview-workspaces/window-tracker-monitor.h>

G_BEGIN_DECLS

/* Object declaration */
#define XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_TRACKER				(xfce_overview_workspaces_window_tracker_get_type())
#define XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_TRACKER, XfceOverviewWorkspacesWindowTracker))
#define XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_TRACKER))
#define XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_GET_IFACE(obj)	(G_TYPE_INSTANCE_GET_INTERFACE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_TRACKER, XfceOverviewWorkspacesWindowTrackerInterface))

typedef struct _XfceOverviewWorkspacesWindowTracker			XfceOverviewWorkspacesWindowTracker;
typedef struct _XfceOverviewWorkspacesWindowTrackerInterface	XfceOverviewWorkspacesWindowTrackerInterface;

/**
 * XfceOverviewWorkspacesWindowTrackerInterface:
 * @get_windows: Get list of windows tracked
 * @get_windows_stacked: Get list of windows tracked in stacked order
 *    (from bottom to top)
 * @get_active_window: Get the current active window
 * @get_workspaces_count: Get number of workspaces
 * @get_workspaces: Get list of workspaces tracked
 * @get_active_workspace: Get the current active workspace
 * @get_workspace_by_number: Get workspace by its index
 * @supports_multiple_monitors: Whether the window trackers supports and tracks
 *    multiple monitors or not
 * @get_monitors_count: Get number of monitors
 * @get_monitors: Get list of monitors tracked
 * @get_primary_monitor: Get the current primary monitor
 * @get_monitor_by_number: Get monitor by its index
 * @get_monitor_by_position: Get monitor by looking up if it contains the
 *    requested position
 * @get_screen_size: Get total size of screen (size over all connected monitors)
 * @get_window_manager_name: Get name of window manager at desktop environment
 * @get_root_window: Get root window (usually the desktop at background)
 * @window_stacking_changed: Signal emitted when the stacking order of windows
 *    has changed
 * @active_window_changed: Signal emitted when the active window has changed,
 *    e.g. focus moved to another window
 * @window_opened: Signal emitted when a new window was opened
 * @window_closed: Signal emitted when a window was closed
 * @window_geometry_changed: Signal emitted when a window has changed its
 *    position or size or both
 * @window_actions_changed: Signal emitted when the available action of a window
 *    has changed
 * @window_state_changed: Signal emitted when the state of a window has changed
 * @window_icon_changed: Signal emitted when the icon of a window has changed
 * @window_name_changed: Signal emitted when the title of a window has changed
 * @window_workspace_changed: Signal emitted when a window was moved to another
 *    workspace
 * @window_monitor_changed: Signal emitted when a window was moved to another
 *    monitor
 * @active_workspace_changed: Signal emitted when the active workspace has changed
 * @workspace_added: Signal emitted when a new workspace was added
 * @workspace_removed: Signal emitted when a workspace was removed
 * @workspace_name_changed: Signal emitted when the title of a workspace has
 *    changed
 * @primary_monitor_changed: Signal emitted when the primary monitor has changed
 * @monitor_added: Signal emitted when a new monitor was connected
 * @monitor_removed: Signal emitted when a monitor was disconnected or turned off
 * @monitor_geometry_changed: Signal emitted when the resolution of a monitor
 *    has chnaged
 * @screen_size_changed: Signal emitted when the total screen size over all
 *    connected monitors has changed
 * @window_manager_changed: Signal emitted when the window manager was replaced
 */
struct _XfceOverviewWorkspacesWindowTrackerInterface
{
	/*< private >*/
	/* Parent interface */
	GTypeInterface						parent_interface;

	/*< public >*/
	/* Virtual functions */
	GList* (*get_windows)(XfceOverviewWorkspacesWindowTracker *self);
	GList* (*get_windows_stacked)(XfceOverviewWorkspacesWindowTracker *self);
	XfceOverviewWorkspacesWindowTrackerWindow* (*get_active_window)(XfceOverviewWorkspacesWindowTracker *self);

	gint (*get_workspaces_count)(XfceOverviewWorkspacesWindowTracker *self);
	GList* (*get_workspaces)(XfceOverviewWorkspacesWindowTracker *self);
	XfceOverviewWorkspacesWindowTrackerWorkspace* (*get_active_workspace)(XfceOverviewWorkspacesWindowTracker *self);
	XfceOverviewWorkspacesWindowTrackerWorkspace* (*get_workspace_by_number)(XfceOverviewWorkspacesWindowTracker *self, gint inNumber);

	gboolean (*supports_multiple_monitors)(XfceOverviewWorkspacesWindowTracker *self);
	gint (*get_monitors_count)(XfceOverviewWorkspacesWindowTracker *self);
	GList* (*get_monitors)(XfceOverviewWorkspacesWindowTracker *self);
	XfceOverviewWorkspacesWindowTrackerMonitor* (*get_primary_monitor)(XfceOverviewWorkspacesWindowTracker *self);
	XfceOverviewWorkspacesWindowTrackerMonitor* (*get_monitor_by_number)(XfceOverviewWorkspacesWindowTracker *self, gint inNumber);
	XfceOverviewWorkspacesWindowTrackerMonitor* (*get_monitor_by_position)(XfceOverviewWorkspacesWindowTracker *self, gint inX, gint inY);

	void (*get_screen_size)(XfceOverviewWorkspacesWindowTracker *self, gint *outWidth, gint *outHeight);

	const gchar* (*get_window_manager_name)(XfceOverviewWorkspacesWindowTracker *self);

	XfceOverviewWorkspacesWindowTrackerWindow* (*get_root_window)(XfceOverviewWorkspacesWindowTracker *self);

	/* Signals */
	void (*window_stacking_changed)(XfceOverviewWorkspacesWindowTracker *self);

	void (*active_window_changed)(XfceOverviewWorkspacesWindowTracker *self,
									XfceOverviewWorkspacesWindowTrackerWindow *inOldWindow,
									XfceOverviewWorkspacesWindowTrackerWindow *inNewWindow);
	void (*window_opened)(XfceOverviewWorkspacesWindowTracker *self, XfceOverviewWorkspacesWindowTrackerWindow *inWindow);
	void (*window_closed)(XfceOverviewWorkspacesWindowTracker *self, XfceOverviewWorkspacesWindowTrackerWindow *inWindow);
	void (*window_geometry_changed)(XfceOverviewWorkspacesWindowTracker *self,XfceOverviewWorkspacesWindowTrackerWindow *inWindow);
	void (*window_actions_changed)(XfceOverviewWorkspacesWindowTracker *self, XfceOverviewWorkspacesWindowTrackerWindow *inWindow);
	void (*window_state_changed)(XfceOverviewWorkspacesWindowTracker *self, XfceOverviewWorkspacesWindowTrackerWindow *inWindow);
	void (*window_icon_changed)(XfceOverviewWorkspacesWindowTracker *self, XfceOverviewWorkspacesWindowTrackerWindow *inWindow);
	void (*window_name_changed)(XfceOverviewWorkspacesWindowTracker *self, XfceOverviewWorkspacesWindowTrackerWindow *inWindow);
	void (*window_workspace_changed)(XfceOverviewWorkspacesWindowTracker *self,
										XfceOverviewWorkspacesWindowTrackerWindow *inWindow,
										XfceOverviewWorkspacesWindowTrackerWorkspace *inWorkspace);
	void (*window_monitor_changed)(XfceOverviewWorkspacesWindowTracker *self,
										XfceOverviewWorkspacesWindowTrackerWindow *inWindow,
										XfceOverviewWorkspacesWindowTrackerMonitor *inOldMonitor,
										XfceOverviewWorkspacesWindowTrackerMonitor *inNewMonitor);

	void (*active_workspace_changed)(XfceOverviewWorkspacesWindowTracker *self,
										XfceOverviewWorkspacesWindowTrackerWorkspace *inOldWorkspace,
										XfceOverviewWorkspacesWindowTrackerWorkspace *inNewWorkspace);
	void (*workspace_added)(XfceOverviewWorkspacesWindowTracker *self, XfceOverviewWorkspacesWindowTrackerWorkspace *inWorkspace);
	void (*workspace_removed)(XfceOverviewWorkspacesWindowTracker *self, XfceOverviewWorkspacesWindowTrackerWorkspace *inWorkspace);
	void (*workspace_name_changed)(XfceOverviewWorkspacesWindowTracker *self, XfceOverviewWorkspacesWindowTrackerWorkspace *inWorkspace);

	void (*primary_monitor_changed)(XfceOverviewWorkspacesWindowTracker *self,
									XfceOverviewWorkspacesWindowTrackerMonitor *inOldMonitor,
									XfceOverviewWorkspacesWindowTrackerMonitor *inNewMonitor);
	void (*monitor_added)(XfceOverviewWorkspacesWindowTracker *self, XfceOverviewWorkspacesWindowTrackerMonitor *inMonitor);
	void (*monitor_removed)(XfceOverviewWorkspacesWindowTracker *self, XfceOverviewWorkspacesWindowTrackerMonitor *inMonitor);
	void (*monitor_geometry_changed)(XfceOverviewWorkspacesWindowTracker *self, XfceOverviewWorkspacesWindowTrackerMonitor *inMonitor);

	void (*screen_size_changed)(XfceOverviewWorkspacesWindowTracker *self);

	void (*window_manager_changed)(XfceOverviewWorkspacesWindowTracker *self);
};


/* Public API */
GType xfce_overview_workspaces_window_tracker_get_type(void) G_GNUC_CONST;

GList* xfce_overview_workspaces_window_tracker_get_windows(XfceOverviewWorkspacesWindowTracker *self);
GList* xfce_overview_workspaces_window_tracker_get_windows_stacked(XfceOverviewWorkspacesWindowTracker *self);
XfceOverviewWorkspacesWindowTrackerWindow* xfce_overview_workspaces_window_tracker_get_active_window(XfceOverviewWorkspacesWindowTracker *self);

gint xfce_overview_workspaces_window_tracker_get_workspaces_count(XfceOverviewWorkspacesWindowTracker *self);
GList* xfce_overview_workspaces_window_tracker_get_workspaces(XfceOverviewWorkspacesWindowTracker *self);
XfceOverviewWorkspacesWindowTrackerWorkspace* xfce_overview_workspaces_window_tracker_get_active_workspace(XfceOverviewWorkspacesWindowTracker *self);
XfceOverviewWorkspacesWindowTrackerWorkspace* xfce_overview_workspaces_window_tracker_get_workspace_by_number(XfceOverviewWorkspacesWindowTracker *self,
																						gint inNumber);

gboolean xfce_overview_workspaces_window_tracker_supports_multiple_monitors(XfceOverviewWorkspacesWindowTracker *self);
gint xfce_overview_workspaces_window_tracker_get_monitors_count(XfceOverviewWorkspacesWindowTracker *self);
GList* xfce_overview_workspaces_window_tracker_get_monitors(XfceOverviewWorkspacesWindowTracker *self);
XfceOverviewWorkspacesWindowTrackerMonitor* xfce_overview_workspaces_window_tracker_get_primary_monitor(XfceOverviewWorkspacesWindowTracker *self);
XfceOverviewWorkspacesWindowTrackerMonitor* xfce_overview_workspaces_window_tracker_get_monitor_by_number(XfceOverviewWorkspacesWindowTracker *self,
																					gint inNumber);
XfceOverviewWorkspacesWindowTrackerMonitor* xfce_overview_workspaces_window_tracker_get_monitor_by_position(XfceOverviewWorkspacesWindowTracker *self,
																						gint inX,
																						gint inY);

void xfce_overview_workspaces_window_tracker_get_screen_size(XfceOverviewWorkspacesWindowTracker *self,
													gint *outWidth,
													gint *outHeight);

const gchar* xfce_overview_workspaces_window_tracker_get_window_manager_name(XfceOverviewWorkspacesWindowTracker *self);

XfceOverviewWorkspacesWindowTrackerWindow* xfce_overview_workspaces_window_tracker_get_root_window(XfceOverviewWorkspacesWindowTracker *self);
XfceOverviewWorkspacesWindowTrackerWindow* xfce_overview_workspaces_window_tracker_get_stage_window(XfceOverviewWorkspacesWindowTracker *self,
																			ClutterStage *inStage);

G_END_DECLS

#endif	/* __LIBXFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER__ */
