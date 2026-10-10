/*
 * window-tracker-window: A window tracked by window tracker.
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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW__
#define __LIBXFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <clutter/clutter.h>
#include <glib-object.h>
#include <gdk/gdk.h>

#include <libxfce-overview-workspaces/window-tracker-workspace.h>
#include <libxfce-overview-workspaces/window-tracker-monitor.h>

G_BEGIN_DECLS

/* Public definitions */
/**
 * XfceOverviewWorkspacesWindowTrackerWindowState:
 * @XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW_STATE_HIDDEN: The window is not visible on its
 *                                                  #XfceOverviewWorkspacesWindowTrackerWorkspace,
 *                                                  e.g. when minimized.
 * @XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW_STATE_MINIMIZED: The window is minimized.
 * @XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW_STATE_MAXIMIZED: The window is maximized.
 * @XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW_STATE_FULLSCREEN: The window is fullscreen.
 * @XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW_STATE_SKIP_PAGER: The window should not be included on pagers.
 * @XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW_STATE_SKIP_TASKLIST: The window should not be included on tasklists.
 * @XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW_STATE_PINNED: The window is on all workspaces.
 * @XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW_STATE_URGENT: The window requires a response from the user.
 *
 * Type used as a bitmask to describe the state of a #XfceOverviewWorkspacesWindowTrackerWindow.
 */
typedef enum /*< flags,prefix=XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW_STATE >*/
{
	XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW_STATE_HIDDEN=1 << 0,
	XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW_STATE_MINIMIZED=1 << 1,
	XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW_STATE_MAXIMIZED=1 << 2,
	XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW_STATE_FULLSCREEN=1 << 3,
	XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW_STATE_SKIP_PAGER=1 << 4,
	XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW_STATE_SKIP_TASKLIST=1 << 5,
	XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW_STATE_PINNED=1 << 6,
	XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW_STATE_URGENT=1 << 7,
} XfceOverviewWorkspacesWindowTrackerWindowState;

/**
 * XfceOverviewWorkspacesWindowTrackerWindowAction:
 * @XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW_ACTION_CLOSE: The window may be closed.
 *
 * Type used as a bitmask to describe the actions that can be done for a #XfceOverviewWorkspacesWindowTrackerWindow.
 */
typedef enum /*< flags,prefix=XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW_ACTION >*/
{
	XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW_ACTION_CLOSE=1 << 0,
} XfceOverviewWorkspacesWindowTrackerWindowAction;


/* Object declaration */
#define XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_TRACKER_WINDOW				(xfce_overview_workspaces_window_tracker_window_get_type())
#define XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_TRACKER_WINDOW, XfceOverviewWorkspacesWindowTrackerWindow))
#define XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_WINDOW(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_TRACKER_WINDOW))
#define XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW_GET_IFACE(obj)	(G_TYPE_INSTANCE_GET_INTERFACE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_TRACKER_WINDOW, XfceOverviewWorkspacesWindowTrackerWindowInterface))

typedef struct _XfceOverviewWorkspacesWindowTrackerWindow				XfceOverviewWorkspacesWindowTrackerWindow;
typedef struct _XfceOverviewWorkspacesWindowTrackerWindowInterface		XfceOverviewWorkspacesWindowTrackerWindowInterface;

struct _XfceOverviewWorkspacesWindowTrackerWindowInterface
{
	/*< private >*/
	/* Parent interface */
	GTypeInterface						parent_interface;

	/*< public >*/
	/* Virtual functions */
	gboolean (*is_equal)(XfceOverviewWorkspacesWindowTrackerWindow *inLeft, XfceOverviewWorkspacesWindowTrackerWindow *inRight);

	gboolean (*is_visible)(XfceOverviewWorkspacesWindowTrackerWindow *self);
	void (*show)(XfceOverviewWorkspacesWindowTrackerWindow *self);
	void (*hide)(XfceOverviewWorkspacesWindowTrackerWindow *self);

	XfceOverviewWorkspacesWindowTrackerWindow* (*get_parent)(XfceOverviewWorkspacesWindowTrackerWindow *self);

	XfceOverviewWorkspacesWindowTrackerWindowState (*get_state)(XfceOverviewWorkspacesWindowTrackerWindow *self);
	void (*set_state)(XfceOverviewWorkspacesWindowTrackerWindow *self, XfceOverviewWorkspacesWindowTrackerWindowState inState);
	XfceOverviewWorkspacesWindowTrackerWindowAction (*get_actions)(XfceOverviewWorkspacesWindowTrackerWindow *self);

	const gchar* (*get_name)(XfceOverviewWorkspacesWindowTrackerWindow *self);

	GdkPixbuf* (*get_icon)(XfceOverviewWorkspacesWindowTrackerWindow *self);
	const gchar* (*get_icon_name)(XfceOverviewWorkspacesWindowTrackerWindow *self);

	XfceOverviewWorkspacesWindowTrackerWorkspace* (*get_workspace)(XfceOverviewWorkspacesWindowTrackerWindow *self);
	gboolean (*is_on_workspace)(XfceOverviewWorkspacesWindowTrackerWindow *self, XfceOverviewWorkspacesWindowTrackerWorkspace *inWorkspace);

	XfceOverviewWorkspacesWindowTrackerMonitor* (*get_monitor)(XfceOverviewWorkspacesWindowTrackerWindow *self);
	gboolean (*is_on_monitor)(XfceOverviewWorkspacesWindowTrackerWindow *self, XfceOverviewWorkspacesWindowTrackerMonitor *inMonitor);

	void (*get_geometry)(XfceOverviewWorkspacesWindowTrackerWindow *self, gint *outX, gint *outY, gint *outWidth, gint *outHeight);
	void (*set_geometry)(XfceOverviewWorkspacesWindowTrackerWindow *self, gint inX, gint inY, gint inWidth, gint inHeight);
	void (*move)(XfceOverviewWorkspacesWindowTrackerWindow *self, gint inX, gint inY);
	void (*resize)(XfceOverviewWorkspacesWindowTrackerWindow *self, gint inWidth, gint inHeight);
	void (*move_to_workspace)(XfceOverviewWorkspacesWindowTrackerWindow *self, XfceOverviewWorkspacesWindowTrackerWorkspace *inWorkspace);
	void (*activate)(XfceOverviewWorkspacesWindowTrackerWindow *self);
	void (*close)(XfceOverviewWorkspacesWindowTrackerWindow *self);

	gint (*get_pid)(XfceOverviewWorkspacesWindowTrackerWindow *self);
	GAppInfo* (*get_appinfo)(XfceOverviewWorkspacesWindowTrackerWindow *self);

	ClutterContent* (*get_content)(XfceOverviewWorkspacesWindowTrackerWindow *self);

	/* Signals */
	void (*name_changed)(XfceOverviewWorkspacesWindowTrackerWindow *self);
	void (*state_changed)(XfceOverviewWorkspacesWindowTrackerWindow *self,
							XfceOverviewWorkspacesWindowTrackerWindowState inOldStates);
	void (*actions_changed)(XfceOverviewWorkspacesWindowTrackerWindow *self,
							XfceOverviewWorkspacesWindowTrackerWindowAction inOldActions);
	void (*icon_changed)(XfceOverviewWorkspacesWindowTrackerWindow *self);
	void (*workspace_changed)(XfceOverviewWorkspacesWindowTrackerWindow *self,
								XfceOverviewWorkspacesWindowTrackerWorkspace *inOldWorkspace);
	void (*monitor_changed)(XfceOverviewWorkspacesWindowTrackerWindow *self,
							XfceOverviewWorkspacesWindowTrackerMonitor *inOldMonitor);
	void (*geometry_changed)(XfceOverviewWorkspacesWindowTrackerWindow *self);
	void (*closed)(XfceOverviewWorkspacesWindowTrackerWindow *self);
};

/* Public API */
GType xfce_overview_workspaces_window_tracker_window_get_type(void) G_GNUC_CONST;

gboolean xfce_overview_workspaces_window_tracker_window_is_equal(XfceOverviewWorkspacesWindowTrackerWindow *inLeft,
													XfceOverviewWorkspacesWindowTrackerWindow *inRight);

gboolean xfce_overview_workspaces_window_tracker_window_is_visible(XfceOverviewWorkspacesWindowTrackerWindow *self);
gboolean xfce_overview_workspaces_window_tracker_window_is_visible_on_workspace(XfceOverviewWorkspacesWindowTrackerWindow *self,
																	XfceOverviewWorkspacesWindowTrackerWorkspace *inWorkspace);
gboolean xfce_overview_workspaces_window_tracker_window_is_visible_on_monitor(XfceOverviewWorkspacesWindowTrackerWindow *self,
																	XfceOverviewWorkspacesWindowTrackerMonitor *inMonitor);
void xfce_overview_workspaces_window_tracker_window_show(XfceOverviewWorkspacesWindowTrackerWindow *self);
void xfce_overview_workspaces_window_tracker_window_hide(XfceOverviewWorkspacesWindowTrackerWindow *self);

XfceOverviewWorkspacesWindowTrackerWindow* xfce_overview_workspaces_window_tracker_window_get_parent(XfceOverviewWorkspacesWindowTrackerWindow *self);

XfceOverviewWorkspacesWindowTrackerWindowState xfce_overview_workspaces_window_tracker_window_get_state(XfceOverviewWorkspacesWindowTrackerWindow *self);
void xfce_overview_workspaces_window_tracker_window_set_state(XfceOverviewWorkspacesWindowTrackerWindow *self, XfceOverviewWorkspacesWindowTrackerWindowState inState);

XfceOverviewWorkspacesWindowTrackerWindowAction xfce_overview_workspaces_window_tracker_window_get_actions(XfceOverviewWorkspacesWindowTrackerWindow *self);

const gchar* xfce_overview_workspaces_window_tracker_window_get_name(XfceOverviewWorkspacesWindowTrackerWindow *self);

GdkPixbuf* xfce_overview_workspaces_window_tracker_window_get_icon(XfceOverviewWorkspacesWindowTrackerWindow *self);
const gchar* xfce_overview_workspaces_window_tracker_window_get_icon_name(XfceOverviewWorkspacesWindowTrackerWindow *self);

XfceOverviewWorkspacesWindowTrackerWorkspace* xfce_overview_workspaces_window_tracker_window_get_workspace(XfceOverviewWorkspacesWindowTrackerWindow *self);
gboolean xfce_overview_workspaces_window_tracker_window_is_on_workspace(XfceOverviewWorkspacesWindowTrackerWindow *self,
															XfceOverviewWorkspacesWindowTrackerWorkspace *inWorkspace);
void xfce_overview_workspaces_window_tracker_window_move_to_workspace(XfceOverviewWorkspacesWindowTrackerWindow *self,
															XfceOverviewWorkspacesWindowTrackerWorkspace *inWorkspace);

XfceOverviewWorkspacesWindowTrackerMonitor* xfce_overview_workspaces_window_tracker_window_get_monitor(XfceOverviewWorkspacesWindowTrackerWindow *self);
gboolean xfce_overview_workspaces_window_tracker_window_is_on_monitor(XfceOverviewWorkspacesWindowTrackerWindow *self,
															XfceOverviewWorkspacesWindowTrackerMonitor *inMonitor);

void xfce_overview_workspaces_window_tracker_window_get_geometry(XfceOverviewWorkspacesWindowTrackerWindow *self,
															gint *outX,
															gint *outY,
															gint *outWidth,
															gint *outHeight);
void xfce_overview_workspaces_window_tracker_window_set_geometry(XfceOverviewWorkspacesWindowTrackerWindow *self,
															gint inX,
															gint inY,
															gint inWidth,
															gint inHeight);
void xfce_overview_workspaces_window_tracker_window_move(XfceOverviewWorkspacesWindowTrackerWindow *self,
											gint inX,
											gint inY);
void xfce_overview_workspaces_window_tracker_window_resize(XfceOverviewWorkspacesWindowTrackerWindow *self,
												gint inWidth,
												gint inHeight);

void xfce_overview_workspaces_window_tracker_window_activate(XfceOverviewWorkspacesWindowTrackerWindow *self);
void xfce_overview_workspaces_window_tracker_window_close(XfceOverviewWorkspacesWindowTrackerWindow *self);

gboolean xfce_overview_workspaces_window_tracker_window_is_stage(XfceOverviewWorkspacesWindowTrackerWindow *self);
ClutterStage* xfce_overview_workspaces_window_tracker_window_get_stage(XfceOverviewWorkspacesWindowTrackerWindow *self);
void xfce_overview_workspaces_window_tracker_window_show_stage(XfceOverviewWorkspacesWindowTrackerWindow *self);
void xfce_overview_workspaces_window_tracker_window_hide_stage(XfceOverviewWorkspacesWindowTrackerWindow *self);

gint xfce_overview_workspaces_window_tracker_window_get_pid(XfceOverviewWorkspacesWindowTrackerWindow *self);
GAppInfo* xfce_overview_workspaces_window_tracker_window_get_appinfo(XfceOverviewWorkspacesWindowTrackerWindow *self);

ClutterContent* xfce_overview_workspaces_window_tracker_window_get_content(XfceOverviewWorkspacesWindowTrackerWindow *self);

G_END_DECLS

#endif	/* __LIBXFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW__ */
