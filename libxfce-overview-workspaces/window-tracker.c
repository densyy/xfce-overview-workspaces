/*
 * window-tracker: Tracks windows, workspaces, monitors and
 *                 listens for changes. It also bundles libwnck into one
 *                 class.
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

#include <libxfce-overview-workspaces/window-tracker.h>

#include <glib/gi18n-lib.h>

#include <libxfce-overview-workspaces/window-tracker-backend.h>
#include <libxfce-overview-workspaces/marshal.h>
#include <libxfce-overview-workspaces/core.h>
#include <libxfce-overview-workspaces/debug.h>


/* Define this class in GObject system */
G_DEFINE_INTERFACE(XfceOverviewWorkspacesWindowTracker,
					xfce_overview_workspaces_window_tracker,
					G_TYPE_OBJECT)


/* Signals */
enum
{
	SIGNAL_WINDOW_STACKING_CHANGED,

	SIGNAL_ACTIVE_WINDOW_CHANGED,
	SIGNAL_WINDOW_OPENED,
	SIGNAL_WINDOW_CLOSED,
	SIGNAL_WINDOW_GEOMETRY_CHANGED,
	SIGNAL_WINDOW_ACTIONS_CHANGED,
	SIGNAL_WINDOW_STATE_CHANGED,
	SIGNAL_WINDOW_ICON_CHANGED,
	SIGNAL_WINDOW_NAME_CHANGED,
	SIGNAL_WINDOW_WORKSPACE_CHANGED,
	SIGNAL_WINDOW_MONITOR_CHANGED,

	SIGNAL_ACTIVE_WORKSPACE_CHANGED,
	SIGNAL_WORKSPACE_ADDED,
	SIGNAL_WORKSPACE_REMOVED,
	SIGNAL_WORKSPACE_NAME_CHANGED,

	SIGNAL_PRIMARY_MONITOR_CHANGED,
	SIGNAL_MONITOR_ADDED,
	SIGNAL_MONITOR_REMOVED,
	SIGNAL_MONITOR_GEOMETRY_CHANGED,
	SIGNAL_SCREEN_SIZE_CHANGED,

	SIGNAL_WINDOW_MANAGER_CHANGED,

	SIGNAL_LAST
};

static guint XfceOverviewWorkspacesWindowTrackerSignals[SIGNAL_LAST]={ 0, };


/* IMPLEMENTATION: Private variables and methods */
#define XFCE_OVERVIEW_WORKSPACES_WINDOWS_TRACKER_WARN_NOT_IMPLEMENTED(self, vfunc)  \
	g_warning("Object of type %s does not implement required virtual function XfceOverviewWorkspacesWindowTracker::%s",\
				G_OBJECT_TYPE_NAME(self), \
				vfunc);

/* Default signal handler for signal "window_closed" */
static void _xfce_overview_workspaces_window_tracker_real_window_closed(XfceOverviewWorkspacesWindowTracker *self,
															XfceOverviewWorkspacesWindowTrackerWindow *inWindow)
{
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER(self));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_WINDOW(inWindow));

	/* By default (if not overidden) emit "closed" signal at window */
	g_signal_emit_by_name(inWindow, "closed");
}


/* IMPLEMENTATION: GObject */

/* Interface initialization
 * Set up default functions
 */
void xfce_overview_workspaces_window_tracker_default_init(XfceOverviewWorkspacesWindowTrackerInterface *iface)
{
	static gboolean		initialized=FALSE;
	GParamSpec			*property;

	/* The following virtual functions should be overriden if default
	 * implementation does not fit.
	 */
	iface->window_closed=_xfce_overview_workspaces_window_tracker_real_window_closed;

	/* Define properties, signals and actions */
	if(!initialized)
	{
		/* Define properties */
		property=g_param_spec_object("active-window",
										"Active window",
										"The current active window",
										XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_TRACKER_WINDOW,
										G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
		g_object_interface_install_property(iface, property);

		property=g_param_spec_object("active-workspace",
										"Active workspace",
										"The current active workspace",
										XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_TRACKER_WORKSPACE,
										G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
		g_object_interface_install_property(iface, property);

		property=g_param_spec_object("primary-monitor",
										"Primary monitor",
										"The current primary monitor",
										XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_TRACKER_MONITOR,
										G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
		g_object_interface_install_property(iface, property);

		/* Define signals */
		/**
		 * XfceOverviewWorkspacesWindowTracker::window-tacking-changed:
		 * @self: The window tracker
		 *
		 * The ::window-tacking-changed signal is emitted whenever the stacking
		 * order of the windows at the desktop environment has changed.
		 */
		XfceOverviewWorkspacesWindowTrackerSignals[SIGNAL_WINDOW_STACKING_CHANGED]=
			g_signal_new("window-stacking-changed",
							G_TYPE_FROM_INTERFACE(iface),
							G_SIGNAL_RUN_LAST,
							G_STRUCT_OFFSET(XfceOverviewWorkspacesWindowTrackerInterface, window_stacking_changed),
							NULL,
							NULL,
							g_cclosure_marshal_VOID__VOID,
							G_TYPE_NONE,
							0);

		/**
		 * XfceOverviewWorkspacesWindowTracker::active-window-changed:
		 * @self: The window tracker
		 * @inPreviousActiveWindow: The #XfceOverviewWorkspacesWindowTrackerWindow which
		 *    was the active window before this change
		 * @inCurrentActiveWindow: The #XfceOverviewWorkspacesWindowTrackerWindow which is
		 *    the active window now
		 *
		 * The ::active-window-changed is emitted when the active window has
		 * changed.
		 */
		XfceOverviewWorkspacesWindowTrackerSignals[SIGNAL_ACTIVE_WINDOW_CHANGED]=
			g_signal_new("active-window-changed",
							G_TYPE_FROM_INTERFACE(iface),
							G_SIGNAL_RUN_LAST,
							G_STRUCT_OFFSET(XfceOverviewWorkspacesWindowTrackerInterface, active_window_changed),
							NULL,
							NULL,
							_xfce_overview_workspaces_marshal_VOID__OBJECT_OBJECT,
							G_TYPE_NONE,
							2,
							XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_TRACKER_WINDOW,
							XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_TRACKER_WINDOW);

		/**
		 * XfceOverviewWorkspacesWindowTracker::window-opened:
		 * @self: The window tracker
		 * @inWindow: The #XfceOverviewWorkspacesWindowTrackerWindow opened
		 *
		 * The ::window-opened signal is emitted whenever a new window was opened
		 * at the desktop environment.
		 */
		XfceOverviewWorkspacesWindowTrackerSignals[SIGNAL_WINDOW_OPENED]=
			g_signal_new("window-opened",
							G_TYPE_FROM_INTERFACE(iface),
							G_SIGNAL_RUN_LAST,
							G_STRUCT_OFFSET(XfceOverviewWorkspacesWindowTrackerInterface, window_opened),
							NULL,
							NULL,
							g_cclosure_marshal_VOID__OBJECT,
							G_TYPE_NONE,
							1,
							XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_TRACKER_WINDOW);

		/**
		 * XfceOverviewWorkspacesWindowTracker::window-closed:
		 * @self: The window tracker
		 * @inWindow: The #XfceOverviewWorkspacesWindowTrackerWindow closed
		 *
		 * The ::window-closed signal is emitted when a window was closed and is
		 * not available anymore.
		 */
		XfceOverviewWorkspacesWindowTrackerSignals[SIGNAL_WINDOW_CLOSED]=
			g_signal_new("window-closed",
							G_TYPE_FROM_INTERFACE(iface),
							G_SIGNAL_RUN_LAST,
							G_STRUCT_OFFSET(XfceOverviewWorkspacesWindowTrackerInterface, window_closed),
							NULL,
							NULL,
							g_cclosure_marshal_VOID__OBJECT,
							G_TYPE_NONE,
							1,
							XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_TRACKER_WINDOW);

		/**
		 * XfceOverviewWorkspacesWindowTracker::window-geometry-changed:
		 * @self: The window tracker
		 * @inWindow: The #XfceOverviewWorkspacesWindowTrackerWindow which changed its size or position
		 *
		 * The ::window-geometry-changed signal is emitted when the size of a
		 * window or its position at screen of the desktop environment has changed.
		 */
		XfceOverviewWorkspacesWindowTrackerSignals[SIGNAL_WINDOW_GEOMETRY_CHANGED]=
			g_signal_new("window-geometry-changed",
							G_TYPE_FROM_INTERFACE(iface),
							G_SIGNAL_RUN_LAST,
							G_STRUCT_OFFSET(XfceOverviewWorkspacesWindowTrackerInterface, window_geometry_changed),
							NULL,
							NULL,
							g_cclosure_marshal_VOID__OBJECT,
							G_TYPE_NONE,
							1,
							XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_TRACKER_WINDOW);

		/**
		 * XfceOverviewWorkspacesWindowTracker::window-actions-changed:
		 * @self: The window tracker
		 * @inWindow: The #XfceOverviewWorkspacesWindowTrackerWindow changed the availability
		 *    of actions
		 *
		 * The ::window-actions-changed signal is emitted whenever the availability
		 * of actions of a window changes.
		 */
		XfceOverviewWorkspacesWindowTrackerSignals[SIGNAL_WINDOW_ACTIONS_CHANGED]=
			g_signal_new("window-actions-changed",
							G_TYPE_FROM_INTERFACE(iface),
							G_SIGNAL_RUN_LAST,
							G_STRUCT_OFFSET(XfceOverviewWorkspacesWindowTrackerInterface, window_actions_changed),
							NULL,
							NULL,
							g_cclosure_marshal_VOID__OBJECT,
							G_TYPE_NONE,
							1,
							XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_TRACKER_WINDOW);

		/**
		 * XfceOverviewWorkspacesWindowTracker::window-state-changed:
		 * @self: The window tracker
		 * @inWindow: The #XfceOverviewWorkspacesWindowTrackerWindow changed its state
		 *
		 * The ::window-state-changed signal is emitted whenever a window
		 * changes its state. This can happen when @inWindow is (un)minimized,
		 * (un)maximized, (un)pinned, (un)set fullscreen etc. See
		 * #XfceOverviewWorkspacesWindowTrackerWindowState for the complete list of states
		 * that might have changed.
		 */
		XfceOverviewWorkspacesWindowTrackerSignals[SIGNAL_WINDOW_STATE_CHANGED]=
			g_signal_new("window-state-changed",
							G_TYPE_FROM_INTERFACE(iface),
							G_SIGNAL_RUN_LAST,
							G_STRUCT_OFFSET(XfceOverviewWorkspacesWindowTrackerInterface, window_state_changed),
							NULL,
							NULL,
							g_cclosure_marshal_VOID__OBJECT,
							G_TYPE_NONE,
							1,
							XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_TRACKER_WINDOW);

		/**
		 * XfceOverviewWorkspacesWindowTracker::window-icon-changed:
		 * @self: The window tracker
		 * @inWindow: The #XfceOverviewWorkspacesWindowTrackerWindow changed its icon
		 *
		 * The ::window-icon-changed signal is emitted whenever a window
		 * changes its icon
		 */
		XfceOverviewWorkspacesWindowTrackerSignals[SIGNAL_WINDOW_ICON_CHANGED]=
			g_signal_new("window-icon-changed",
							G_TYPE_FROM_INTERFACE(iface),
							G_SIGNAL_RUN_LAST,
							G_STRUCT_OFFSET(XfceOverviewWorkspacesWindowTrackerInterface, window_icon_changed),
							NULL,
							NULL,
							g_cclosure_marshal_VOID__OBJECT,
							G_TYPE_NONE,
							1,
							XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_TRACKER_WINDOW);

		/**
		 * XfceOverviewWorkspacesWindowTracker::window-name-changed:
		 * @self: The window tracker
		 * @inWindow: The #XfceOverviewWorkspacesWindowTrackerWindow changed its name
		 *
		 * The ::window-name-changed signal is emitted whenever a window
		 * changes its name, known as window title.
		 */
		XfceOverviewWorkspacesWindowTrackerSignals[SIGNAL_WINDOW_NAME_CHANGED]=
			g_signal_new("window-name-changed",
							G_TYPE_FROM_INTERFACE(iface),
							G_SIGNAL_RUN_LAST,
							G_STRUCT_OFFSET(XfceOverviewWorkspacesWindowTrackerInterface, window_name_changed),
							NULL,
							NULL,
							g_cclosure_marshal_VOID__OBJECT,
							G_TYPE_NONE,
							1,
							XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_TRACKER_WINDOW);

		/**
		 * XfceOverviewWorkspacesWindowTracker::window-workspace-changed:
		 * @self: The window tracker
		 * @inWindow: The #XfceOverviewWorkspacesWindowTrackerWindow moved to another workspace
		 * @inWorkspace: The #XfceOverviewWorkspacesWindowTrackerWorkspace where the window
		 *    @inWindow was moved to
		 *
		 * The ::window-workspace-changed signal is emitted whenever a window
		 * moves to another workspace.
		 */
		XfceOverviewWorkspacesWindowTrackerSignals[SIGNAL_WINDOW_WORKSPACE_CHANGED]=
			g_signal_new("window-workspace-changed",
							G_TYPE_FROM_INTERFACE(iface),
							G_SIGNAL_RUN_LAST,
							G_STRUCT_OFFSET(XfceOverviewWorkspacesWindowTrackerInterface, window_workspace_changed),
							NULL,
							NULL,
							_xfce_overview_workspaces_marshal_VOID__OBJECT_OBJECT,
							G_TYPE_NONE,
							2,
							XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_TRACKER_WINDOW,
							XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_TRACKER_WORKSPACE);

		/**
		 * XfceOverviewWorkspacesWindowTracker::window-monitor-changed:
		 * @self: The window tracker
		 * @inWindow: The #XfceOverviewWorkspacesWindowTrackerWindow moved to another monitor
		 * @inPreviousMonitor: The #XfceOverviewWorkspacesWindowTrackerMonitor where the window
		 *    @inWindow was located at before this change
		 * @inCurrentMonitor: The #XfceOverviewWorkspacesWindowTrackerMonitor where the window
		 *    @inWindow is located at now
		 *
		 * The ::window-monitor-changed signal is emitted whenever a window
		 * moves to another monitor.
		 */
		XfceOverviewWorkspacesWindowTrackerSignals[SIGNAL_WINDOW_MONITOR_CHANGED]=
			g_signal_new("window-monitor-changed",
							G_TYPE_FROM_INTERFACE(iface),
							G_SIGNAL_RUN_LAST,
							G_STRUCT_OFFSET(XfceOverviewWorkspacesWindowTrackerInterface, window_monitor_changed),
							NULL,
							NULL,
							_xfce_overview_workspaces_marshal_VOID__OBJECT_OBJECT_OBJECT,
							G_TYPE_NONE,
							3,
							XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_TRACKER_WINDOW,
							XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_TRACKER_MONITOR,
							XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_TRACKER_MONITOR);

		/**
		 * XfceOverviewWorkspacesWindowTracker::active-workspace-changed:
		 * @self: The window tracker
		 * @inPreviousActiveWorkspace: The #XfceOverviewWorkspacesWindowTrackerWorkspace which
		 *    was the active workspace before this change
		 * @inCurrentActiveWorkspace: The #XfceOverviewWorkspacesWindowTrackerWorkspace which
		 *    is the active workspace now
		 *
		 * The ::active-workspace-changed signal is emitted when the active workspace
		 * has changed.
		 */
		XfceOverviewWorkspacesWindowTrackerSignals[SIGNAL_ACTIVE_WORKSPACE_CHANGED]=
			g_signal_new("active-workspace-changed",
							G_TYPE_FROM_INTERFACE(iface),
							G_SIGNAL_RUN_LAST,
							G_STRUCT_OFFSET(XfceOverviewWorkspacesWindowTrackerInterface, active_workspace_changed),
							NULL,
							NULL,
							_xfce_overview_workspaces_marshal_VOID__OBJECT_OBJECT,
							G_TYPE_NONE,
							2,
							XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_TRACKER_WORKSPACE,
							XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_TRACKER_WORKSPACE);

		/**
		 * XfceOverviewWorkspacesWindowTracker::workspace-added:
		 * @self: The window tracker
		 * @inWorkspace: The #XfceOverviewWorkspacesWindowTrackerWorkspace added
		 *
		 * The ::workspace-added signal is emitted whenever a new workspace was added.
		 */
		XfceOverviewWorkspacesWindowTrackerSignals[SIGNAL_WORKSPACE_ADDED]=
			g_signal_new("workspace-added",
							G_TYPE_FROM_INTERFACE(iface),
							G_SIGNAL_RUN_LAST,
							G_STRUCT_OFFSET(XfceOverviewWorkspacesWindowTrackerInterface, workspace_added),
							NULL,
							NULL,
							g_cclosure_marshal_VOID__OBJECT,
							G_TYPE_NONE,
							1,
							XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_TRACKER_WORKSPACE);

		/**
		 * XfceOverviewWorkspacesWindowTracker::workspace-removed:
		 * @self: The window tracker
		 * @inWorkspace: The #XfceOverviewWorkspacesWindowTrackerWorkspace removed
		 *
		 * The ::workspace-removed signal is emitted whenever a workspace was removed.
		 */
		XfceOverviewWorkspacesWindowTrackerSignals[SIGNAL_WORKSPACE_REMOVED]=
			g_signal_new("workspace-removed",
							G_TYPE_FROM_INTERFACE(iface),
							G_SIGNAL_RUN_LAST,
							G_STRUCT_OFFSET(XfceOverviewWorkspacesWindowTrackerInterface, workspace_removed),
							NULL,
							NULL,
							g_cclosure_marshal_VOID__OBJECT,
							G_TYPE_NONE,
							1,
							XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_TRACKER_WORKSPACE);

		/**
		 * XfceOverviewWorkspacesWindowTracker::workspace-name-changed:
		 * @self: The window tracker
		 * @inWorkspace: The #XfceOverviewWorkspacesWindowTrackerWorkspace changed its name
		 *
		 * The ::workspace-name-changed signal is emitted whenever a workspace
		 * changes its name.
		 */
		XfceOverviewWorkspacesWindowTrackerSignals[SIGNAL_WORKSPACE_NAME_CHANGED]=
			g_signal_new("workspace-name-changed",
							G_TYPE_FROM_INTERFACE(iface),
							G_SIGNAL_RUN_LAST,
							G_STRUCT_OFFSET(XfceOverviewWorkspacesWindowTrackerInterface, workspace_name_changed),
							NULL,
							NULL,
							g_cclosure_marshal_VOID__OBJECT,
							G_TYPE_NONE,
							1,
							XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_TRACKER_WORKSPACE);

		/**
		 * XfceOverviewWorkspacesWindowTracker::primary-monitor-changed:
		 * @self: The window tracker
		 * @inPreviousPrimaryMonitor: The #XfceOverviewWorkspacesWindowTrackerMonitor which
		 *    was the primary monitor before this change
		 * @inCurrentPrimaryMonitor: The #XfceOverviewWorkspacesWindowTrackerMonitor which
		 *    is the new primary monitor now
		 *
		 * The ::primary-monitor-changed signal is emitted when another monitor
		 * was configured to be the primary monitor.
		 */
		XfceOverviewWorkspacesWindowTrackerSignals[SIGNAL_PRIMARY_MONITOR_CHANGED]=
			g_signal_new("primary-monitor-changed",
							G_TYPE_FROM_INTERFACE(iface),
							G_SIGNAL_RUN_LAST,
							G_STRUCT_OFFSET(XfceOverviewWorkspacesWindowTrackerInterface, primary_monitor_changed),
							NULL,
							NULL,
							_xfce_overview_workspaces_marshal_VOID__OBJECT_OBJECT,
							G_TYPE_NONE,
							2,
							XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_TRACKER_MONITOR,
							XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_TRACKER_MONITOR);

		/**
		 * XfceOverviewWorkspacesWindowTracker::monitor-added:
		 * @self: The window tracker
		 * @inMonitor: The #XfceOverviewWorkspacesWindowTrackerMonitor added
		 *
		 * The ::monitor-added signal is emitted whenever a new monitor was added.
		 */
		XfceOverviewWorkspacesWindowTrackerSignals[SIGNAL_MONITOR_ADDED]=
			g_signal_new("monitor-added",
							G_TYPE_FROM_INTERFACE(iface),
							G_SIGNAL_RUN_LAST,
							G_STRUCT_OFFSET(XfceOverviewWorkspacesWindowTrackerInterface, monitor_added),
							NULL,
							NULL,
							g_cclosure_marshal_VOID__OBJECT,
							G_TYPE_NONE,
							1,
							XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_TRACKER_MONITOR);

		/**
		 * XfceOverviewWorkspacesWindowTracker::monitor-removed:
		 * @self: The window tracker
		 * @inMonitor: The #XfceOverviewWorkspacesWindowTrackerMonitor removed
		 *
		 * The ::monitor-removed signal is emitted whenever a monitor was removed.
		 */
		XfceOverviewWorkspacesWindowTrackerSignals[SIGNAL_MONITOR_REMOVED]=
			g_signal_new("monitor-removed",
							G_TYPE_FROM_INTERFACE(iface),
							G_SIGNAL_RUN_LAST,
							G_STRUCT_OFFSET(XfceOverviewWorkspacesWindowTrackerInterface, monitor_removed),
							NULL,
							NULL,
							g_cclosure_marshal_VOID__OBJECT,
							G_TYPE_NONE,
							1,
							XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_TRACKER_MONITOR);

		/**
		 * XfceOverviewWorkspacesWindowTracker::monitor-geometry-changed:
		 * @self: The window tracker
		 * @inMonitor: The #XfceOverviewWorkspacesWindowTrackerMonitor which changed its size or position
		 *
		 * The ::monitor-geometry-changed signal is emitted when the size of a
		 * monitor or its position at screen of the desktop environment has changed.
		 */
		XfceOverviewWorkspacesWindowTrackerSignals[SIGNAL_MONITOR_GEOMETRY_CHANGED]=
			g_signal_new("monitor-geometry-changed",
							G_TYPE_FROM_INTERFACE(iface),
							G_SIGNAL_RUN_LAST,
							G_STRUCT_OFFSET(XfceOverviewWorkspacesWindowTrackerInterface, monitor_geometry_changed),
							NULL,
							NULL,
							g_cclosure_marshal_VOID__OBJECT,
							G_TYPE_NONE,
							1,
							XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_TRACKER_MONITOR);

		/**
		 * XfceOverviewWorkspacesWindowTracker::screen-size-changed:
		 * @self: The window tracker
		 *
		 * The ::screen-size-changed signal is emitted when the screen size of
		 * the desktop environment has been changed, e.g. one monitor changed its
		 * resolution.
		 */
		XfceOverviewWorkspacesWindowTrackerSignals[SIGNAL_SCREEN_SIZE_CHANGED]=
			g_signal_new("screen-size-changed",
							G_TYPE_FROM_INTERFACE(iface),
							G_SIGNAL_RUN_LAST,
							G_STRUCT_OFFSET(XfceOverviewWorkspacesWindowTrackerInterface, screen_size_changed),
							NULL,
							NULL,
							g_cclosure_marshal_VOID__VOID,
							G_TYPE_NONE,
							0);

		/**
		 * XfceOverviewWorkspacesWindowTracker::window-manager-changed:
		 * @self: The window tracker
		 *
		 * The ::window-manager-changed signal is emitted when the window manager
		 * of the desktop environment has been replaced with a new one.
		 */
		XfceOverviewWorkspacesWindowTrackerSignals[SIGNAL_WINDOW_MANAGER_CHANGED]=
			g_signal_new("window-manager-changed",
							G_TYPE_FROM_INTERFACE(iface),
							G_SIGNAL_RUN_LAST,
							G_STRUCT_OFFSET(XfceOverviewWorkspacesWindowTrackerInterface, window_manager_changed),
							NULL,
							NULL,
							g_cclosure_marshal_VOID__VOID,
							G_TYPE_NONE,
							0);

		/* Set flag that base initialization was done for this interface */
		initialized=TRUE;
	}
}


/* IMPLEMENTATION: Public API */

/**
 * xfce_overview_workspaces_window_tracker_get_windows:
 * @self: A #XfceOverviewWorkspacesWindowTracker
 *
 * Retrieves the list of #XfceOverviewWorkspacesWindowTrackerWindow tracked by @self.
 * The list is ordered: the first element in the list is the first
 * #XfceOverviewWorkspacesWindowTrackerWindow, etc..
 *
 * Return value: (element-type XfceOverviewWorkspacesWindowTrackerWindow) (transfer none):
 *   The list of #XfceOverviewWorkspacesWindowTrackerWindow. The list should not be modified
 *   nor freed, as it is owned by XfceOverviewWorkspaces.
 */
GList* xfce_overview_workspaces_window_tracker_get_windows(XfceOverviewWorkspacesWindowTracker *self)
{
	XfceOverviewWorkspacesWindowTrackerInterface		*iface;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER(self), NULL);

	iface=XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_GET_IFACE(self);

	/* Call virtual function */
	if(iface->get_windows)
	{
		return(iface->get_windows(self));
	}

	/* If we get here the virtual function was not overridden */
	XFCE_OVERVIEW_WORKSPACES_WINDOWS_TRACKER_WARN_NOT_IMPLEMENTED(self, "get_windows");
	return(NULL);
}

/**
 * xfce_overview_workspaces_window_tracker_get_windows_stacked:
 * @self: A #XfceOverviewWorkspacesWindowTracker
 *
 * Retrieves the list of #XfceOverviewWorkspacesWindowTrackerWindow tracked by @self in
 * stacked order from bottom to top. The list is ordered: the first element in
 * the list is the most bottom #XfceOverviewWorkspacesWindowTrackerWindow, etc..
 *
 * Return value: (element-type XfceOverviewWorkspacesWindowTrackerWindow) (transfer none):
 *   The list of #XfceOverviewWorkspacesWindowTrackerWindow in stacked order. The list should
 *   not be modified nor freed, as it is owned by XfceOverviewWorkspaces.
 */
GList* xfce_overview_workspaces_window_tracker_get_windows_stacked(XfceOverviewWorkspacesWindowTracker *self)
{
	XfceOverviewWorkspacesWindowTrackerInterface		*iface;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER(self), NULL);

	iface=XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_GET_IFACE(self);

	/* Call virtual function */
	if(iface->get_windows_stacked)
	{
		return(iface->get_windows_stacked(self));
	}

	/* If we get here the virtual function was not overridden */
	XFCE_OVERVIEW_WORKSPACES_WINDOWS_TRACKER_WARN_NOT_IMPLEMENTED(self, "get_windows_stacked");
	return(NULL);
}

/**
 * xfce_overview_workspaces_window_tracker_get_active_window:
 * @self: A #XfceOverviewWorkspacesWindowTracker
 *
 * Retrieves the currently active #XfceOverviewWorkspacesWindowTrackerWindow.
 *
 * Return value: (transfer none): The #XfceOverviewWorkspacesWindowTrackerWindow currently
 *   active or %NULL if not determinable. The returned object is owned by XfceOverviewWorkspaces
 *   and it should not be referenced or unreferenced.
 */
XfceOverviewWorkspacesWindowTrackerWindow* xfce_overview_workspaces_window_tracker_get_active_window(XfceOverviewWorkspacesWindowTracker *self)
{
	XfceOverviewWorkspacesWindowTrackerInterface		*iface;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER(self), NULL);

	iface=XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_GET_IFACE(self);

	/* Call virtual function */
	if(iface->get_active_window)
	{
		return(iface->get_active_window(self));
	}

	/* If we get here the virtual function was not overridden */
	XFCE_OVERVIEW_WORKSPACES_WINDOWS_TRACKER_WARN_NOT_IMPLEMENTED(self, "get_active_window");
	return(NULL);
}

/**
 * xfce_overview_workspaces_window_tracker_get_workspaces_count:
 * @self: A #XfceOverviewWorkspacesWindowTracker
 *
 * Retrieves the number of #XfceOverviewWorkspacesWindowTrackerWorkspace tracked by @self.
 *
 * Return value: The number of #XfceOverviewWorkspacesWindowTrackerWorkspace
 */
gint xfce_overview_workspaces_window_tracker_get_workspaces_count(XfceOverviewWorkspacesWindowTracker *self)
{
	XfceOverviewWorkspacesWindowTrackerInterface		*iface;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER(self), 0);

	iface=XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_GET_IFACE(self);

	/* Call virtual function */
	if(iface->get_workspaces_count)
	{
		return(iface->get_workspaces_count(self));
	}

	/* If we get here the virtual function was not overridden */
	XFCE_OVERVIEW_WORKSPACES_WINDOWS_TRACKER_WARN_NOT_IMPLEMENTED(self, "get_workspaces_count");
	return(0);
}

/**
 * xfce_overview_workspaces_window_tracker_get_workspaces:
 * @self: A #XfceOverviewWorkspacesWindowTracker
 *
 * Retrieves the list of #XfceOverviewWorkspacesWindowTrackerWorkspace tracked by @self.
 * The list is ordered: the first element in the list is the first
 * #XfceOverviewWorkspacesWindowTrackerWorkspace, etc..
 *
 * Return value: (element-type XfceOverviewWorkspacesWindowTrackerWorkspace) (transfer none):
 *   The list of #XfceOverviewWorkspacesWindowTrackerWorkspace. The list should not be modified
 *   nor freed, as it is owned by XfceOverviewWorkspaces.
 */
GList* xfce_overview_workspaces_window_tracker_get_workspaces(XfceOverviewWorkspacesWindowTracker *self)
{
	XfceOverviewWorkspacesWindowTrackerInterface		*iface;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER(self), NULL);

	iface=XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_GET_IFACE(self);

	/* Call virtual function */
	if(iface->get_workspaces)
	{
		return(iface->get_workspaces(self));
	}

	/* If we get here the virtual function was not overridden */
	XFCE_OVERVIEW_WORKSPACES_WINDOWS_TRACKER_WARN_NOT_IMPLEMENTED(self, "get_workspaces");
	return(NULL);
}

/**
 * xfce_overview_workspaces_window_tracker_get_active_workspace:
 * @self: A #XfceOverviewWorkspacesWindowTracker
 *
 * Retrieves the currently active #XfceOverviewWorkspacesWindowTrackerWorkspace.
 *
 * Return value: (transfer none): The #XfceOverviewWorkspacesWindowTrackerWorkspace currently
 *   active or %NULL if not determinable. The returned object is owned by XfceOverviewWorkspaces
 *   and it should not be referenced or unreferenced.
 */
XfceOverviewWorkspacesWindowTrackerWorkspace* xfce_overview_workspaces_window_tracker_get_active_workspace(XfceOverviewWorkspacesWindowTracker *self)
{
	XfceOverviewWorkspacesWindowTrackerInterface		*iface;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER(self), NULL);

	iface=XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_GET_IFACE(self);

	/* Call virtual function */
	if(iface->get_active_workspace)
	{
		return(iface->get_active_workspace(self));
	}

	/* If we get here the virtual function was not overridden */
	XFCE_OVERVIEW_WORKSPACES_WINDOWS_TRACKER_WARN_NOT_IMPLEMENTED(self, "get_active_workspace");
	return(NULL);
}

/**
 * xfce_overview_workspaces_window_tracker_get_workspace_by_number:
 * @self: A #XfceOverviewWorkspacesWindowTracker
 * @inNumber: The workspace index, starting from 0
 *
 * Retrieves the #XfceOverviewWorkspacesWindowTrackerWorkspace at index @inNumber.
 *
 * Return value: (transfer none): The #XfceOverviewWorkspacesWindowTrackerWorkspace at the
 *   index or %NULL if no such workspace exists. The returned object is owned by
 *   XfceOverviewWorkspaces and it should not be referenced or unreferenced.
 */
XfceOverviewWorkspacesWindowTrackerWorkspace* xfce_overview_workspaces_window_tracker_get_workspace_by_number(XfceOverviewWorkspacesWindowTracker *self,
																						gint inNumber)
{
	XfceOverviewWorkspacesWindowTrackerInterface		*iface;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER(self), NULL);
	g_return_val_if_fail(inNumber>=0, NULL);
	g_return_val_if_fail(inNumber<xfce_overview_workspaces_window_tracker_get_workspaces_count(self), NULL);

	iface=XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_GET_IFACE(self);

	/* Call virtual function */
	if(iface->get_workspace_by_number)
	{
		return(iface->get_workspace_by_number(self, inNumber));
	}

	/* If we get here the virtual function was not overridden */
	XFCE_OVERVIEW_WORKSPACES_WINDOWS_TRACKER_WARN_NOT_IMPLEMENTED(self, "get_workspace_by_number");
	return(NULL);
}

/**
 * xfce_overview_workspaces_window_tracker_supports_multiple_monitors:
 * @self: A #XfceOverviewWorkspacesWindowTracker
 *
 * Determines if window tracker at @self supports multiple monitors.
 *
 * If multiple monitors are supported, %TRUE will be returned and the number
 * of monitors can be determined by calling xfce_overview_workspaces_window_tracker_get_monitors_count().
 * Also each monitor can be accessed xfce_overview_workspaces_window_tracker_get_monitor_by_number()
 * and other monitor related functions.
 *
 * If multiple monitors are not supported or the desktop environment cannot provide
 * this kind of information, %FALSE will be returned.
 *
 * Return value: %TRUE if @self supports multiple monitors, otherwise %FALSE.
 */
gboolean xfce_overview_workspaces_window_tracker_supports_multiple_monitors(XfceOverviewWorkspacesWindowTracker *self)
{
	XfceOverviewWorkspacesWindowTrackerInterface		*iface;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER(self), FALSE);

	iface=XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_GET_IFACE(self);

	/* Call virtual function */
	if(iface->supports_multiple_monitors)
	{
		return(iface->supports_multiple_monitors(self));
	}

	/* If we get here the virtual function was not overridden */
	XFCE_OVERVIEW_WORKSPACES_WINDOWS_TRACKER_WARN_NOT_IMPLEMENTED(self, "supports_multiple_monitors");
	return(FALSE);
}

/**
 * xfce_overview_workspaces_window_tracker_get_monitors_count:
 * @self: A #XfceOverviewWorkspacesWindowTracker
 *
 * Retrieves the number of #XfceOverviewWorkspacesWindowTrackerMonitor tracked by @self.
 *
 * Return value: The number of #XfceOverviewWorkspacesWindowTrackerMonitor
 */
gint xfce_overview_workspaces_window_tracker_get_monitors_count(XfceOverviewWorkspacesWindowTracker *self)
{
	XfceOverviewWorkspacesWindowTrackerInterface		*iface;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER(self), 0);

	iface=XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_GET_IFACE(self);

	/* Call virtual function */
	if(iface->get_monitors_count)
	{
		return(iface->get_monitors_count(self));
	}

	/* If we get here the virtual function was not overridden */
	XFCE_OVERVIEW_WORKSPACES_WINDOWS_TRACKER_WARN_NOT_IMPLEMENTED(self, "get_monitors_count");
	return(0);
}

/**
 * xfce_overview_workspaces_window_tracker_get_monitors:
 * @self: A #XfceOverviewWorkspacesWindowTracker
 *
 * Retrieves the list of #XfceOverviewWorkspacesWindowTrackerMonitor tracked by @self.
 * The list is ordered: the first element in the list is the first #XfceOverviewWorkspacesWindowTrackerMonitor,
 * etc..
 *
 * Return value: (element-type XfceOverviewWorkspacesWindowTrackerMonitor) (transfer none):
 *   The list of #XfceOverviewWorkspacesWindowTrackerMonitor. The list should not be modified
 *   nor freed, as it is owned by XfceOverviewWorkspaces.
 */
GList* xfce_overview_workspaces_window_tracker_get_monitors(XfceOverviewWorkspacesWindowTracker *self)
{
	XfceOverviewWorkspacesWindowTrackerInterface		*iface;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER(self), NULL);

	iface=XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_GET_IFACE(self);

	/* Call virtual function */
	if(iface->get_monitors)
	{
		return(iface->get_monitors(self));
	}

	/* If we get here the virtual function was not overridden */
	XFCE_OVERVIEW_WORKSPACES_WINDOWS_TRACKER_WARN_NOT_IMPLEMENTED(self, "get_monitors");
	return(NULL);
}

/**
 * xfce_overview_workspaces_window_tracker_get_primary_monitor:
 * @self: A #XfceOverviewWorkspacesWindowTracker
 *
 * Retrieves the primary #XfceOverviewWorkspacesWindowTrackerMonitor the user configured
 * at its desktop environment.
 *
 * Return value: (transfer none): The #XfceOverviewWorkspacesWindowTrackerMonitor configured
 *   as primary or %NULL if no primary monitor exists. The returned object is
 *   owned by XfceOverviewWorkspaces and it should not be referenced or unreferenced.
 */
XfceOverviewWorkspacesWindowTrackerMonitor* xfce_overview_workspaces_window_tracker_get_primary_monitor(XfceOverviewWorkspacesWindowTracker *self)
{
	XfceOverviewWorkspacesWindowTrackerInterface		*iface;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER(self), NULL);

	iface=XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_GET_IFACE(self);

	/* Call virtual function */
	if(iface->get_primary_monitor)
	{
		return(iface->get_primary_monitor(self));
	}

	/* If we get here the virtual function was not overridden */
	XFCE_OVERVIEW_WORKSPACES_WINDOWS_TRACKER_WARN_NOT_IMPLEMENTED(self, "get_primary_monitor");
	return(NULL);
}

/**
 * xfce_overview_workspaces_window_tracker_get_monitor_by_number:
 * @self: A #XfceOverviewWorkspacesWindowTracker
 * @inNumber: The monitor index, starting from 0
 *
 * Retrieves the #XfceOverviewWorkspacesWindowTrackerMonitor at index @inNumber.
 *
 * Return value: (transfer none): The #XfceOverviewWorkspacesWindowTrackerMonitor at the
 *   index or %NULL if no such monitor exists. The returned object is owned by
 *   XfceOverviewWorkspaces and it should not be referenced or unreferenced.
 */
XfceOverviewWorkspacesWindowTrackerMonitor* xfce_overview_workspaces_window_tracker_get_monitor_by_number(XfceOverviewWorkspacesWindowTracker *self,
																					gint inNumber)
{
	XfceOverviewWorkspacesWindowTrackerInterface		*iface;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER(self), NULL);
	g_return_val_if_fail(inNumber>=0, NULL);
	g_return_val_if_fail(inNumber<xfce_overview_workspaces_window_tracker_get_monitors_count(self), NULL);

	iface=XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_GET_IFACE(self);

	/* Call virtual function */
	if(iface->get_monitor_by_number)
	{
		return(iface->get_monitor_by_number(self, inNumber));
	}

	/* If we get here the virtual function was not overridden */
	XFCE_OVERVIEW_WORKSPACES_WINDOWS_TRACKER_WARN_NOT_IMPLEMENTED(self, "get_monitor_by_number");
	return(NULL);
}

/**
 * xfce_overview_workspaces_window_tracker_get_monitor_by_position:
 * @self: A #XfceOverviewWorkspacesWindowTracker
 * @inX: The X coordinate of position at screen
 * @inY: The Y coordinate of position at screen
 *
 * Retrieves the monitor containing the position at @inX,@inY at screen.
 *
 * Return value: (transfer none): The #XfceOverviewWorkspacesWindowTrackerMonitor for the
 *   requested position or %NULL if no monitor could be found containing the
 *   position. The returned object is owned by XfceOverviewWorkspaces and it should not be
 *   referenced or unreferenced.
 */
XfceOverviewWorkspacesWindowTrackerMonitor* xfce_overview_workspaces_window_tracker_get_monitor_by_position(XfceOverviewWorkspacesWindowTracker *self,
																						gint inX,
																						gint inY)
{
	XfceOverviewWorkspacesWindowTrackerInterface		*iface;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER(self), NULL);

	iface=XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_GET_IFACE(self);

	/* Call virtual function */
	if(iface->get_monitor_by_position)
	{
		return(iface->get_monitor_by_position(self, inX, inY));
	}

	/* If we get here the virtual function was not overridden */
	XFCE_OVERVIEW_WORKSPACES_WINDOWS_TRACKER_WARN_NOT_IMPLEMENTED(self, "get_monitor_by_position");
	return(NULL);
}

/**
 * xfce_overview_workspaces_window_tracker_get_screen_size:
 * @self: A #XfceOverviewWorkspacesWindowTracker
 * @outWidth: (out): Return location for width of screen.
 * @outHeight: (out): Return location for height of screen.
 *
 * Retrieves width and height of screen of the desktop environment. The screen
 * contains all connected monitors so it the total size of the desktop environment.
 */
void xfce_overview_workspaces_window_tracker_get_screen_size(XfceOverviewWorkspacesWindowTracker *self, gint *outWidth, gint *outHeight)
{
	XfceOverviewWorkspacesWindowTrackerInterface		*iface;
	gint									width, height;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER(self));

	iface=XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_GET_IFACE(self);

	/* Call virtual function */
	if(iface->get_screen_size)
	{
		/* Get screen size */
		iface->get_screen_size(self, &width, &height);

		/* Store result where possible */
		if(outWidth) *outWidth=width;
		if(outHeight) *outHeight=height;

		return;
	}

	/* If we get here the virtual function was not overridden */
	XFCE_OVERVIEW_WORKSPACES_WINDOWS_TRACKER_WARN_NOT_IMPLEMENTED(self, "get_screen_width");
}

/**
 * xfce_overview_workspaces_window_tracker_get_window_manager_name:
 * @self: A #XfceOverviewWorkspacesWindowTracker
 *
 * Retrieves the name of window manager managing the desktop environment, i.e.
 * windows, workspaces etc.
 *
 * Return value: A string with name of the window manager
 */
const gchar* xfce_overview_workspaces_window_tracker_get_window_manager_name(XfceOverviewWorkspacesWindowTracker *self)
{
	XfceOverviewWorkspacesWindowTrackerInterface		*iface;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER(self), NULL);

	iface=XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_GET_IFACE(self);

	/* Call virtual function */
	if(iface->get_window_manager_name)
	{
		return(iface->get_window_manager_name(self));
	}

	/* If we get here the virtual function was not overridden */
	XFCE_OVERVIEW_WORKSPACES_WINDOWS_TRACKER_WARN_NOT_IMPLEMENTED(self, "get_window_manager_name");
	return(NULL);
}

/**
 * xfce_overview_workspaces_window_tracker_get_root_window:
 * @self: A #XfceOverviewWorkspacesWindowTracker
 *
 * Retrieves the root window of the desktop environment. The root window is
 * usually the desktop seen at the background of the desktop environment.
 *
 * Return value: (transfer none): The #XfceOverviewWorkspacesWindowTrackerWindow representing
 *   the root window or %NULL if not available. The returned object is owned by
 *   XfceOverviewWorkspaces and it should not be referenced or unreferenced.
 */
XfceOverviewWorkspacesWindowTrackerWindow* xfce_overview_workspaces_window_tracker_get_root_window(XfceOverviewWorkspacesWindowTracker *self)
{
	XfceOverviewWorkspacesWindowTrackerInterface		*iface;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER(self), NULL);

	iface=XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_GET_IFACE(self);

	/* Call virtual function */
	if(iface->get_root_window)
	{
		return(iface->get_root_window(self));
	}

	/* If we get here the virtual function was not overridden */
	XFCE_OVERVIEW_WORKSPACES_WINDOWS_TRACKER_WARN_NOT_IMPLEMENTED(self, "get_root_window");
	return(NULL);
}

/**
 * xfce_overview_workspaces_window_tracker_get_stage_window:
 * @self: A #XfceOverviewWorkspacesWindowTracker
 * @inStage: A #ClutterStage
 *
 * Retrieves the window created for the requested stage @inStage.
 *
 * This function is the logical equivalent of:
 *
 * |[<!-- language="C" -->
 *   XfceOverviewWorkspacesWindowTrackerBackend *backend;
 *   XfceOverviewWorkspacesWindowTrackerWindow  *stageWindow;
 *
 *   backend=xfce_overview_workspaces_core_get_window_tracker_backend(core);
 *   stageWindow=xfce_overview_workspaces_window_tracker_backend_get_window_for_stage(backend, inStage);
 *   g_object_unref(backend);
 * ]|
 *
 * Return value: (transfer none): The #XfceOverviewWorkspacesWindowTrackerWindow representing
 *   the window of requested stage or %NULL if not available. The returned object
 *   is owned by XfceOverviewWorkspaces and it should not be referenced or unreferenced.
 */
XfceOverviewWorkspacesWindowTrackerWindow* xfce_overview_workspaces_window_tracker_get_stage_window(XfceOverviewWorkspacesWindowTracker *self,
																			ClutterStage *inStage)
{
	XfceOverviewWorkspacesWindowTrackerBackend		*backend;
	XfceOverviewWorkspacesWindowTrackerWindow		*stageWindow;

	/* Get default window tracker backend */
	backend=xfce_overview_workspaces_core_get_window_tracker_backend(NULL);
	if(!backend)
	{
		g_critical("Could not get default window tracker backend");
		return(NULL);
	}

	/* Get window for requested stage from backend */
	stageWindow=xfce_overview_workspaces_window_tracker_backend_get_window_for_stage(backend, inStage);

	/* Release allocated resources */
	if(backend) g_object_unref(backend);

	/* Return window object instance */
	return(stageWindow);
}
