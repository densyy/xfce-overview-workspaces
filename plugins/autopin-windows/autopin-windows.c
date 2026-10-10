/*
 * autopin-windows: Pins or unpins windows depending on the monitor
 *                  it is located
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

#include "autopin-windows.h"

#include <libxfce-overview-workspaces/libxfce-overview-workspaces.h>
#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>
#include <math.h>


/* Define this class in GObject system */
struct _XfceOverviewWorkspacesAutopinWindowsPrivate
{
	/* Instance related */
	XfceOverviewWorkspacesWindowTracker				*windowTracker;
	guint									windowOpenedSignaledID;
	guint									windowClosedSignaledID;
	guint									windowMonitorChangedSignalID;

	gboolean								unpinOnDispose;
	GSList									*pinnedWindows;
};

G_DEFINE_DYNAMIC_TYPE_EXTENDED(XfceOverviewWorkspacesAutopinWindows,
								xfce_overview_workspaces_autopin_windows,
								G_TYPE_OBJECT,
								0,
								G_ADD_PRIVATE_DYNAMIC(XfceOverviewWorkspacesAutopinWindows))

/* Define this class in this plugin */
XFCE_OVERVIEW_WORKSPACES_DEFINE_PLUGIN_TYPE(xfce_overview_workspaces_autopin_windows);


/* IMPLEMENTATION: Private variables and methods */

/* Update pin state of window depending on monitor it is located on */
static void _xfce_overview_workspaces_autopin_windows_update_window_pin_state(XfceOverviewWorkspacesAutopinWindows *self, XfceOverviewWorkspacesWindowTrackerWindow *inWindow)
{
	XfceOverviewWorkspacesAutopinWindowsPrivate		*priv;
	XfceOverviewWorkspacesWindowTrackerMonitor			*currentMonitor;
	gboolean								isPrimary;
	XfceOverviewWorkspacesWindowTrackerWindowState		windowState;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_AUTOPIN_WINDOWS(self));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_WINDOW(inWindow));

	priv=self->priv;

	/* Get monitor the window is located on */
	currentMonitor=xfce_overview_workspaces_window_tracker_window_get_monitor(inWindow);
	if(!currentMonitor)
	{
		XFCE_OVERVIEW_WORKSPACES_DEBUG(self, PLUGINS,
							"Skipping window '%s' because we could not get monitor",
							xfce_overview_workspaces_window_tracker_window_get_name(inWindow));
		return;
	}

	/* Get primary state of new monitor and window state */
	isPrimary=xfce_overview_workspaces_window_tracker_monitor_is_primary(currentMonitor);
	windowState=xfce_overview_workspaces_window_tracker_window_get_state(inWindow);
	XFCE_OVERVIEW_WORKSPACES_DEBUG(self, PLUGINS,
						"Window '%s' is on %s monitor with state %u (%s)",
						xfce_overview_workspaces_window_tracker_window_get_name(inWindow),
						isPrimary ? "primary" : "non-primary",
						windowState,
						(windowState & XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW_STATE_PINNED) ? "pinned" : "unpinned");

	/* Check if window is a "normal" window which could be pinned by user or
	 * this plugin. This depends on its state and role (meaning it is not a
	 * stage window).
	 */
	if(windowState &
		(XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW_STATE_SKIP_PAGER | XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW_STATE_SKIP_TASKLIST))
	{
		XFCE_OVERVIEW_WORKSPACES_DEBUG(self, PLUGINS,
							"Skipping window '%s' because it is skipped from pager and/or tasklist",
							xfce_overview_workspaces_window_tracker_window_get_name(inWindow));
		return;
	}

	if(xfce_overview_workspaces_window_tracker_window_is_stage(inWindow))
	{
		XFCE_OVERVIEW_WORKSPACES_DEBUG(self, PLUGINS,
							"Skipping window '%s' because it is the stage window",
							xfce_overview_workspaces_window_tracker_window_get_name(inWindow));
		return;
	}

	/* Pin window if moved to non-primary monitor and is not pinned yet or unpin
	 * window if moved to primary monitor and is not unpinned yet. Otherwise
	 * keep window untouched.
	 */
	if(isPrimary &&
		(windowState & XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW_STATE_PINNED))
	{
		XFCE_OVERVIEW_WORKSPACES_DEBUG(self, PLUGINS,
							"Unpinning window '%s' as it is located on primary monitor",
							xfce_overview_workspaces_window_tracker_window_get_name(inWindow));

		windowState=windowState & ~XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW_STATE_PINNED;
		xfce_overview_workspaces_window_tracker_window_set_state(inWindow, windowState);

		/* Forget this window as it is unpinned if we remebered it */
		priv->pinnedWindows=g_slist_remove(priv->pinnedWindows, inWindow);
	}
		else if(!isPrimary &&
				!(windowState & XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW_STATE_PINNED))
		{
			XFCE_OVERVIEW_WORKSPACES_DEBUG(self, PLUGINS,
								"Pinning window '%s' as it is located on non-primary monitor",
								xfce_overview_workspaces_window_tracker_window_get_name(inWindow));

			windowState=windowState | XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW_STATE_PINNED;
			xfce_overview_workspaces_window_tracker_window_set_state(inWindow, windowState);

			/* Remember that we pinned this window, so we can unpin it on dispose of
			 * this object instance.
			 */
			priv->pinnedWindows=g_slist_prepend(priv->pinnedWindows, inWindow);
		}
}

/* Callback for windows moved to another monitor */
static void _xfce_overview_workspaces_autopin_windows_on_window_monitor_changed(XfceOverviewWorkspacesAutopinWindows *self,
																	XfceOverviewWorkspacesWindowTrackerWindow *inWindow,
																	XfceOverviewWorkspacesWindowTrackerMonitor *inOldMonitor,
																	XfceOverviewWorkspacesWindowTrackerMonitor *inNewMonitor,
																	gpointer inUserData)
{

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_AUTOPIN_WINDOWS(self));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_WINDOW(inWindow));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_MONITOR(inNewMonitor));

	/* Check moved window if it needs to be pinned or unpinned */
	XFCE_OVERVIEW_WORKSPACES_DEBUG(self, PLUGINS,
						"Window '%s' with state %u (%s) moved from monitor %d (%s) to %d (%s) and needs to be %s",
						xfce_overview_workspaces_window_tracker_window_get_name(inWindow),
						xfce_overview_workspaces_window_tracker_window_get_state(inWindow),
						(xfce_overview_workspaces_window_tracker_window_get_state(inWindow) & XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW_STATE_PINNED) ? "pinned" : "unpinned",
						inOldMonitor ? xfce_overview_workspaces_window_tracker_monitor_get_number(inOldMonitor) : -1,
						(inOldMonitor && xfce_overview_workspaces_window_tracker_monitor_is_primary(inOldMonitor)) ? "primary" : "non-primary",
						inNewMonitor ? xfce_overview_workspaces_window_tracker_monitor_get_number(inNewMonitor) : -1,
						(inNewMonitor && xfce_overview_workspaces_window_tracker_monitor_is_primary(inNewMonitor)) ? "primary" : "non-primary",
						(inNewMonitor && xfce_overview_workspaces_window_tracker_monitor_is_primary(inNewMonitor)) ? "unpinned" : "pinned");

	_xfce_overview_workspaces_autopin_windows_update_window_pin_state(self, inWindow);
}

/* Callback for new windows opened */
static void _xfce_overview_workspaces_autopin_windows_on_window_opened(XfceOverviewWorkspacesAutopinWindows *self,
															XfceOverviewWorkspacesWindowTrackerWindow *inWindow,
															gpointer inUserData)
{
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_AUTOPIN_WINDOWS(self));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_WINDOW(inWindow));

	/* Check window if newly opened window needs to be pinned or unpinned */
	XFCE_OVERVIEW_WORKSPACES_DEBUG(self, PLUGINS,
						"Window '%s' was opened, checking pin state",
						xfce_overview_workspaces_window_tracker_window_get_name(inWindow));
	_xfce_overview_workspaces_autopin_windows_update_window_pin_state(self, inWindow);
}

/* Callback for windows closed */
static void _xfce_overview_workspaces_autopin_windows_on_window_closed(XfceOverviewWorkspacesAutopinWindows *self,
															XfceOverviewWorkspacesWindowTrackerWindow *inWindow,
															gpointer inUserData)
{
	XfceOverviewWorkspacesAutopinWindowsPrivate		*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_AUTOPIN_WINDOWS(self));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_WINDOW(inWindow));

	priv=self->priv;

	/* Forget this window as it was closed so we cannot unping it when this
	 * object will be destroyed.
	 */
	XFCE_OVERVIEW_WORKSPACES_DEBUG(self, PLUGINS,
						"Forget window '%s' which was closed",
						xfce_overview_workspaces_window_tracker_window_get_name(inWindow));
	priv->pinnedWindows=g_slist_remove(priv->pinnedWindows, inWindow);
}


/* IMPLEMENTATION: GObject */

/* Dispose this object */
static void _xfce_overview_workspaces_autopin_windows_dispose(GObject *inObject)
{
	XfceOverviewWorkspacesAutopinWindows				*self=XFCE_OVERVIEW_WORKSPACES_AUTOPIN_WINDOWS(inObject);
	XfceOverviewWorkspacesAutopinWindowsPrivate		*priv=self->priv;
	GSList									*windowIter;
	XfceOverviewWorkspacesWindowTrackerWindow			*window;
	XfceOverviewWorkspacesWindowTrackerWindowState		windowState;

	/* Iterate through all windows and unpin them as this plugin will be disposed */
	if(priv->unpinOnDispose && priv->pinnedWindows)
	{
		for(windowIter=priv->pinnedWindows; windowIter; windowIter=g_slist_next(windowIter))
		{
			/* Get window from iterator */
			window=XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW(windowIter->data);
			if(!window) continue;

			/* Unpinned window */
			windowState=xfce_overview_workspaces_window_tracker_window_get_state(window);
			xfce_overview_workspaces_window_tracker_window_set_state(window, windowState & ~XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW_STATE_PINNED);
			XFCE_OVERVIEW_WORKSPACES_DEBUG(self, PLUGINS,
								"Unpinned window '%s' because it was pinned by us and this plugin is shut down",
								xfce_overview_workspaces_window_tracker_window_get_name(window));
		}

		/* Free and release list */
		g_slist_free(priv->pinnedWindows);
		priv->pinnedWindows=NULL;
	}

	/* Release allocated resources */
	if(priv->windowTracker)
	{
		/* Disconnect signals from window tracker */
		if(priv->windowMonitorChangedSignalID)
		{
			g_signal_handler_disconnect(priv->windowTracker, priv->windowMonitorChangedSignalID);
			priv->windowMonitorChangedSignalID=0;
		}

		if(priv->windowOpenedSignaledID)
		{
			g_signal_handler_disconnect(priv->windowTracker, priv->windowOpenedSignaledID);
			priv->windowOpenedSignaledID=0;
		}

		if(priv->windowClosedSignaledID)
		{
			g_signal_handler_disconnect(priv->windowTracker, priv->windowClosedSignaledID);
			priv->windowClosedSignaledID=0;
		}

		/* Release window tracker */
		g_object_unref(priv->windowTracker);
		priv->windowTracker=NULL;
	}

	/* Call parent's class dispose method */
	G_OBJECT_CLASS(xfce_overview_workspaces_autopin_windows_parent_class)->dispose(inObject);
}

/* Class initialization
 * Override functions in parent classes and define properties
 * and signals
 */
void xfce_overview_workspaces_autopin_windows_class_init(XfceOverviewWorkspacesAutopinWindowsClass *klass)
{
	GObjectClass			*gobjectClass=G_OBJECT_CLASS(klass);

	/* Override functions */
	gobjectClass->dispose=_xfce_overview_workspaces_autopin_windows_dispose;
}

/* Class finalization */
void xfce_overview_workspaces_autopin_windows_class_finalize(XfceOverviewWorkspacesAutopinWindowsClass *klass)
{
}

/* Object initialization
 * Create private structure and set up default values
 */
void xfce_overview_workspaces_autopin_windows_init(XfceOverviewWorkspacesAutopinWindows *self)
{
	XfceOverviewWorkspacesAutopinWindowsPrivate		*priv;
	XfceOverviewWorkspacesCore							*core;
	GList									*windowList;
	XfceOverviewWorkspacesWindowTrackerWindow			*window;

	self->priv=priv=xfce_overview_workspaces_autopin_windows_get_instance_private(self);

	/* Set up default values */
	priv->windowTracker=xfce_overview_workspaces_core_get_window_tracker(NULL);
	priv->windowMonitorChangedSignalID=0;
	priv->windowOpenedSignaledID=0;
	priv->windowClosedSignaledID=0;
	priv->unpinOnDispose=TRUE;
	priv->pinnedWindows=NULL;

	/* Check if application can suspend, i.e. it is running in daemon mode */
	core=xfce_overview_workspaces_core_get_default();
	if(!xfce_overview_workspaces_core_can_suspend(core))
	{
		g_warning("Disabling autopin-windows plugin because core cannot suspend.");
		return;
	}

	/* Iterate through all windows and pin or unpin them depending on which
	 * monitor they are located at.
	 */
	XFCE_OVERVIEW_WORKSPACES_DEBUG(self, PLUGINS,
						"Initializing plugin class %s so iterate through active window list",
						G_OBJECT_TYPE_NAME(self));
	windowList=xfce_overview_workspaces_window_tracker_get_windows(priv->windowTracker);
	for(; windowList; windowList=g_list_next(windowList))
	{
		/* Get window from iterator */
		window=XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW(windowList->data);
		if(!window) continue;

		/* Check window if it needs to be pinned or unpinned */
		_xfce_overview_workspaces_autopin_windows_update_window_pin_state(self, window);
	}
	XFCE_OVERVIEW_WORKSPACES_DEBUG(self, PLUGINS,
						"Initialization of plugin class %s completed",
						G_OBJECT_TYPE_NAME(self));

	/* Connect signal to get notified about actor creations  and filter out
	 * and set up the ones we are interested in.
	 */
	priv->windowMonitorChangedSignalID=
		g_signal_connect_swapped(priv->windowTracker,
									"window-monitor-changed",
									G_CALLBACK(_xfce_overview_workspaces_autopin_windows_on_window_monitor_changed),
									self);

	priv->windowOpenedSignaledID=
		g_signal_connect_swapped(priv->windowTracker,
									"window-opened",
									G_CALLBACK(_xfce_overview_workspaces_autopin_windows_on_window_opened),
									self);

	priv->windowClosedSignaledID=
		g_signal_connect_swapped(priv->windowTracker,
									"window-closed",
									G_CALLBACK(_xfce_overview_workspaces_autopin_windows_on_window_closed),
									self);
}


/* IMPLEMENTATION: Public API */

/* Create new instance */
XfceOverviewWorkspacesAutopinWindows* xfce_overview_workspaces_autopin_windows_new(void)
{
	GObject		*autopinWindows;

	autopinWindows=g_object_new(XFCE_OVERVIEW_WORKSPACES_TYPE_AUTOPIN_WINDOWS, NULL);
	if(!autopinWindows) return(NULL);

	return(XFCE_OVERVIEW_WORKSPACES_AUTOPIN_WINDOWS(autopinWindows));
}
