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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <libxfce-overview-workspaces/gdk/window-tracker-backend-gdk.h>

#include <glib/gi18n-lib.h>
#include <clutter/gdk/clutter-gdk.h>
#include <clutter/x11/clutter-x11.h>
#include <gdk/gdkx.h>
#ifdef HAVE_XINERAMA
#include <X11/extensions/Xinerama.h>
#endif

#include <libxfce-overview-workspaces/x11/window-tracker-x11.h>
#include <libxfce-overview-workspaces/x11/window-tracker-window-x11.h>
#include <libxfce-overview-workspaces/marshal.h>
#include <libxfce-overview-workspaces/debug.h>


/* Define this class in GObject system */
static void _xfce_overview_workspaces_window_tracker_backend_gdk_window_tracker_backend_iface_init(XfceOverviewWorkspacesWindowTrackerBackendInterface *iface);

struct _XfceOverviewWorkspacesWindowTrackerBackendGDKPrivate
{
	/* Instance related */
	XfceOverviewWorkspacesWindowTrackerX11		*windowTracker;
};

G_DEFINE_TYPE_WITH_CODE(XfceOverviewWorkspacesWindowTrackerBackendGDK,
						xfce_overview_workspaces_window_tracker_backend_gdk,
						G_TYPE_OBJECT,
						G_ADD_PRIVATE(XfceOverviewWorkspacesWindowTrackerBackendGDK)
						G_IMPLEMENT_INTERFACE(XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_TRACKER_BACKEND, _xfce_overview_workspaces_window_tracker_backend_gdk_window_tracker_backend_iface_init))

/* IMPLEMENTATION: Private variables and methods */

/* State of stage window changed */
static void _xfce_overview_workspaces_window_tracker_backend_gdk_on_stage_state_changed(WnckWindow *inWindow,
																			WnckWindowState inChangedMask,
																			WnckWindowState inNewValue,
																			gpointer inUserData)
{
	G_GNUC_UNUSED XfceOverviewWorkspacesWindowTrackerWindowX11			*stageWindow;

	g_return_if_fail(WNCK_IS_WINDOW(inWindow));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_WINDOW_X11(inUserData));

	stageWindow=XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW_X11(inUserData);

	/* Set 'skip-tasklist' if changed */
	if((inChangedMask & WNCK_WINDOW_STATE_SKIP_TASKLIST) &&
		!(inNewValue & WNCK_WINDOW_STATE_SKIP_TASKLIST))
	{
		wnck_window_set_skip_tasklist(WNCK_WINDOW(inWindow), TRUE);
		XFCE_OVERVIEW_WORKSPACES_DEBUG(inWindow, WINDOWS,
							"State 'skip-tasklist' for stage window %p (wnck-window=%p) needs reset",
							stageWindow,
							inWindow);
	}

	/* Set 'skip-pager' if changed */
	if((inChangedMask & WNCK_WINDOW_STATE_SKIP_PAGER) &&
		!(inNewValue & WNCK_WINDOW_STATE_SKIP_PAGER))
	{
		wnck_window_set_skip_pager(WNCK_WINDOW(inWindow), TRUE);
		XFCE_OVERVIEW_WORKSPACES_DEBUG(inWindow, WINDOWS,
							"State 'skip-pager' for stage window %p (wnck-window=%p) needs reset",
							stageWindow,
							inWindow);
	}

	/* Set 'make-above' if changed */
	if((inChangedMask & WNCK_WINDOW_STATE_ABOVE) &&
		!(inNewValue & WNCK_WINDOW_STATE_ABOVE))
	{
		wnck_window_make_above(WNCK_WINDOW(inWindow));
		XFCE_OVERVIEW_WORKSPACES_DEBUG(inWindow, WINDOWS,
							"State 'make-above' for stage window %p (wnck-window=%p) needs reset",
							stageWindow,
							inWindow);
	}
}

/* Size of screen has changed so resize stage window */
static void _xfce_overview_workspaces_window_tracker_backend_gdk_on_stage_screen_size_changed(XfceOverviewWorkspacesWindowTracker *inWindowTracker,
																					gint inWidth,
																					gint inHeight,
																					gpointer inUserData)
{
#ifdef HAVE_XINERAMA
	XfceOverviewWorkspacesWindowTrackerWindowX11	*realStageWindow;
	WnckWindow							*stageWindow;
	GdkDisplay							*display;
	GdkScreen							*screen;
	XineramaScreenInfo					*monitors;
	int									monitorsCount;
	gint								top, bottom, left, right;
	gint								topIndex, bottomIndex, leftIndex, rightIndex;
	gint								i;
	Atom								atomFullscreenMonitors;
	XEvent								xEvent;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER(inWindowTracker));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_WINDOW_X11(inUserData));

	realStageWindow=XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW_X11(inUserData);

	XFCE_OVERVIEW_WORKSPACES_DEBUG(inWindowTracker, WINDOWS, "Set fullscreen across all monitors using Xinerama");

	/* Get wnck window for stage window object as it is needed a lot from this
	 * point on.
	 */
	stageWindow=xfce_overview_workspaces_window_tracker_window_x11_get_window(realStageWindow);

	/* If window manager does not support fullscreen across all monitors
	 * return here.
	 */
	if(!wnck_screen_net_wm_supports(wnck_window_get_screen(stageWindow), "_NET_WM_FULLSCREEN_MONITORS"))
	{
		g_warning("Keep window fullscreen on primary monitor because window manager does not support _NET_WM_FULLSCREEN_MONITORS.");
		return;
	}

	/* Get display */
	display=gdk_display_get_default();

	/* Get screen */
	screen=gdk_screen_get_default();

	/* Check if Xinerama is active on display. If not try to move and resize
	 * stage window to primary monitor.
	 */
	if(!XineramaIsActive(gdk_x11_display_get_xdisplay(display)))
	{
		GdkMonitor			*primaryMonitor;
		GdkRectangle		geometry;

		/* Get position and size of primary monitor and try to move and resize
		 * stage window to its position and size. Even if it fails it should
		 * resize the stage to the size of current monitor this window is
		 * fullscreened to. Tested with xfwm4.
		 */
		primaryMonitor=gdk_display_get_primary_monitor(gdk_screen_get_display(screen));
		gdk_monitor_get_geometry(primaryMonitor, &geometry);
		wnck_window_set_geometry(stageWindow,
									WNCK_WINDOW_GRAVITY_STATIC,
									WNCK_WINDOW_CHANGE_X | WNCK_WINDOW_CHANGE_Y | WNCK_WINDOW_CHANGE_WIDTH | WNCK_WINDOW_CHANGE_HEIGHT,
									geometry.x, geometry.y, geometry.width, geometry.height);
		return;
	}

	/* Get monitors from Xinerama */
	monitors=XineramaQueryScreens(GDK_DISPLAY_XDISPLAY(display), &monitorsCount);
	if(monitorsCount<=0 || !monitors)
	{
		if(monitors) XFree(monitors);
		return;
	}

	/* Get monitor indices for each corner of screen */
	xfce_overview_workspaces_window_tracker_get_screen_size(inWindowTracker, &left, &top);
	bottom=0;
	right=0;
	topIndex=bottomIndex=leftIndex=rightIndex=0;
	for(i=0; i<monitorsCount; i++)
	{
		XFCE_OVERVIEW_WORKSPACES_DEBUG(inWindowTracker, WINDOWS,
							"Checking edges at monitor %d with upper-left at %d,%d and lower-right at %d,%d [size: %dx%d]",
							i,
							monitors[i].x_org,
							monitors[i].y_org,
							monitors[i].x_org+monitors[i].width, monitors[i].y_org+monitors[i].height,
							monitors[i].width, monitors[i].height);

		if(left>monitors[i].x_org)
		{
			left=monitors[i].x_org;
			leftIndex=i;
		}

		if(right<(monitors[i].x_org+monitors[i].width))
		{
			right=(monitors[i].x_org+monitors[i].width);
			rightIndex=i;
		}

		if(top>monitors[i].y_org)
		{
			top=monitors[i].y_org;
			topIndex=i;
		}

		if(bottom<(monitors[i].y_org+monitors[i].height))
		{
			bottom=(monitors[i].y_org+monitors[i].height);
			bottomIndex=i;
		}
	}
	XFCE_OVERVIEW_WORKSPACES_DEBUG(inWindowTracker, WINDOWS,
						"Found edge monitors: left=%d (monitor %d), right=%d (monitor %d), top=%d (monitor %d), bottom=%d (monitor %d)",
						left, leftIndex,
						right, rightIndex,
						top, topIndex,
						bottom, bottomIndex);

	/* Get X atom for fullscreen-across-all-monitors */
	atomFullscreenMonitors=XInternAtom(gdk_x11_display_get_xdisplay(display),
										"_NET_WM_FULLSCREEN_MONITORS",
										False);

	/* Send event to X to set window to fullscreen over all monitors */
	memset(&xEvent, 0, sizeof(xEvent));
	xEvent.type=ClientMessage;
	xEvent.xclient.window=wnck_window_get_xid(stageWindow);
	xEvent.xclient.display=GDK_DISPLAY_XDISPLAY(display);
	xEvent.xclient.message_type=atomFullscreenMonitors;
	xEvent.xclient.format=32;
	xEvent.xclient.data.l[0]=topIndex;
	xEvent.xclient.data.l[1]=bottomIndex;
	xEvent.xclient.data.l[2]=leftIndex;
	xEvent.xclient.data.l[3]=rightIndex;
	xEvent.xclient.data.l[4]=0;
	XSendEvent(GDK_DISPLAY_XDISPLAY(display),
				DefaultRootWindow(GDK_DISPLAY_XDISPLAY(display)),
				False,
				SubstructureRedirectMask | SubstructureNotifyMask,
				&xEvent);

	/* Release allocated resources */
	if(monitors) XFree(monitors);
#else
	XfceOverviewWorkspacesWindowTrackerWindowX11	*realStageWindow;
	WnckWindow							*stageWindow;
	GdkScreen							*screen;
	gint								primaryMonitor;
	GdkRectangle						geometry;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER(inWindowTracker));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_WINDOW_X11(inUserData));

	realStageWindow=XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW_X11(inUserData);

	XFCE_OVERVIEW_WORKSPACES_DEBUG(inWindowTracker, WINDOWS, "No support for multiple monitor: Setting fullscreen on primary monitor");

	/* Get wnck window for stage window object as it is needed a lot from this
	 * point on.
	 */
	stageWindow=xfce_overview_workspaces_window_tracker_window_gdk_get_window(realStageWindow);

	/* Get screen */
	screen=gdk_screen_get_default();

	/* Get position and size of primary monitor and try to move and resize
	 * stage window to its position and size. Even if it fails it should
	 * resize the stage to the size of current monitor this window is
	 * fullscreened to. Tested with xfwm4.
	 */
	primaryMonitor=gdk_screen_get_primary_monitor(screen);
	gdk_screen_get_monitor_geometry(screen, primaryMonitor, &geometry);
	wnck_window_set_geometry(stageWindow,
								WNCK_WINDOW_GRAVITY_STATIC,
								WNCK_WINDOW_CHANGE_X | WNCK_WINDOW_CHANGE_Y | WNCK_WINDOW_CHANGE_WIDTH | WNCK_WINDOW_CHANGE_HEIGHT,
								geometry.x, geometry.y, geometry.width, geometry.height);

	XFCE_OVERVIEW_WORKSPACES_DEBUG(inWindowTracker, WINDOWS,
						"Moving stage window to %d,%d and resize to %dx%d",
						geometry.x, geometry.y,
						geometry.width, geometry.height);
#endif
}


/* IMPLEMENTATION: XfceOverviewWorkspacesWindowTrackerBackend */

/* Get name of backend */
static const gchar* _xfce_overview_workspaces_window_tracker_backend_gdk_window_tracker_backend_get_name(XfceOverviewWorkspacesWindowTrackerBackend *inBackend)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_BACKEND_GDK(inBackend), NULL);

	/* Return name of backend */
	return("GDK-X11");
}

/* Get window tracker instance used by this backend */
static XfceOverviewWorkspacesWindowTracker* _xfce_overview_workspaces_window_tracker_backend_gdk_window_tracker_backend_get_window_tracker(XfceOverviewWorkspacesWindowTrackerBackend *inBackend)
{
	XfceOverviewWorkspacesWindowTrackerBackendGDK				*self;
	XfceOverviewWorkspacesWindowTrackerBackendGDKPrivate		*priv;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_BACKEND_GDK(inBackend), NULL);

	self=XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_BACKEND_GDK(inBackend);
	priv=self->priv;

	/* Return window tracker instance used by this instance but do not take a
	 * reference on it as it will be done by the backend interface when
	 * xfce_overview_workspaces_window_tracker_backend_get_default() is called.
	 */
	return(XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER(priv->windowTracker));
}

/* Get window of stage */
static XfceOverviewWorkspacesWindowTrackerWindow* _xfce_overview_workspaces_window_tracker_backend_gdk_window_tracker_backend_get_window_for_stage(XfceOverviewWorkspacesWindowTrackerBackend *inBackend,
																															ClutterStage *inStage)
{
	XfceOverviewWorkspacesWindowTrackerBackendGDK			*self;
	XfceOverviewWorkspacesWindowTrackerBackendGDKPrivate	*priv;
	GdkWindow									*stageGdkWindow;
	Window										stageXWindow;
	WnckWindow									*wnckWindow;
	XfceOverviewWorkspacesWindowTrackerWindow				*window;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_BACKEND_GDK(inBackend), NULL);
	g_return_val_if_fail(CLUTTER_IS_STAGE(inStage), NULL);

	self=XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_BACKEND_GDK(inBackend);
	priv=self->priv;

	/* Get stage window and translate to needed window type */
	stageGdkWindow=clutter_gdk_get_stage_window(inStage);
	stageXWindow=gdk_x11_window_get_xid(stageGdkWindow);
	wnckWindow=wnck_window_get(stageXWindow);

	/* Get or create window object for wnck background window */
	window=xfce_overview_workspaces_window_tracker_x11_get_window_for_wnck(priv->windowTracker, wnckWindow);
	XFCE_OVERVIEW_WORKSPACES_DEBUG(self, WINDOWS,
						"Resolved stage wnck window %s@%p of stage %s@%p to window object %s@%p",
						G_OBJECT_TYPE_NAME(wnckWindow), wnckWindow,
						G_OBJECT_TYPE_NAME(inStage), inStage,
						G_OBJECT_TYPE_NAME(window), window);

	return(window);
}

/* Get associated stage of window */
static ClutterStage* _xfce_overview_workspaces_window_tracker_backend_gdk_window_tracker_backend_get_stage_from_window(XfceOverviewWorkspacesWindowTrackerBackend *inBackend,
																											XfceOverviewWorkspacesWindowTrackerWindow *inStageWindow)
{
	G_GNUC_UNUSED XfceOverviewWorkspacesWindowTrackerBackendGDK			*self;
	XfceOverviewWorkspacesWindowTrackerWindowX11			*stageWindow;
	WnckWindow									*stageWnckWindow;
	Window										stageXWindow;
	ClutterStage								*foundStage;
	GSList										*stages;
	GSList										*iter;
	GdkWindow									*iterGdkWindow;
	Window										iterXWindow;
	ClutterStage								*stage;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_BACKEND_GDK(inBackend), NULL);
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_WINDOW_X11(inStageWindow), NULL);

	self=XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_BACKEND_GDK(inBackend);
	stageWindow=XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW_X11(inStageWindow);

	/* Get wnck and X window of stage window */
	stageWnckWindow=xfce_overview_workspaces_window_tracker_window_x11_get_window(stageWindow);
	if(!stageWnckWindow)
	{
		XFCE_OVERVIEW_WORKSPACES_DEBUG(self, WINDOWS,
							"Could not get wnck window for window %s@%p",
							G_OBJECT_TYPE_NAME(stageWindow),
							stageWindow);
		g_critical("Could not get real stage window to find stage");
		return(NULL);
	}

	stageXWindow=wnck_window_get_xid(stageWnckWindow);
	if(stageXWindow==None)
	{
		XFCE_OVERVIEW_WORKSPACES_DEBUG(self, WINDOWS,
							"Could not get X server window from wnck window %s@%p for window %s@%p",
							G_OBJECT_TYPE_NAME(stageWnckWindow),
							stageWnckWindow,
							G_OBJECT_TYPE_NAME(stageWindow),
							stageWindow);
		g_critical("Could not get real stage window to find stage");
		return(NULL);
	}

	/* Iterate through stages and check if stage window matches requested one */
	foundStage=NULL;
	stages=clutter_stage_manager_list_stages(clutter_stage_manager_get_default());
	for(iter=stages; !foundStage && iter; iter=g_slist_next(iter))
	{
		stage=CLUTTER_STAGE(iter->data);
		if(stage)
		{
			iterGdkWindow=clutter_gdk_get_stage_window(stage);
			if(!iterGdkWindow)
			{
				XFCE_OVERVIEW_WORKSPACES_DEBUG(self, WINDOWS,
									"Could not get GDK window from stage %s@%p",
									G_OBJECT_TYPE_NAME(stage),
									stage);
				continue;
			}

			iterXWindow=gdk_x11_window_get_xid(iterGdkWindow);
			if(iterXWindow==None)
			{
				XFCE_OVERVIEW_WORKSPACES_DEBUG(self, WINDOWS,
									"Could not get X server window for GDK window %s@%p of stage %s@%p",
									G_OBJECT_TYPE_NAME(iterGdkWindow),
									iterGdkWindow,
									G_OBJECT_TYPE_NAME(stage),
									stage);
				continue;
			}

			if(stageXWindow==iterXWindow) foundStage=stage;
		}
	}
	g_slist_free(stages);

	return(foundStage);
}

/* Set up and show window for use as stage */
static void _xfce_overview_workspaces_window_tracker_backend_gdk_window_tracker_backend_show_stage_window(XfceOverviewWorkspacesWindowTrackerBackend *inBackend,
																								XfceOverviewWorkspacesWindowTrackerWindow *inStageWindow)
{
	XfceOverviewWorkspacesWindowTrackerBackendGDK			*self;
	XfceOverviewWorkspacesWindowTrackerBackendGDKPrivate	*priv;
	XfceOverviewWorkspacesWindowTrackerWindowX11			*stageWindow;
	GdkWindow									*stageGdkWindow;
	WnckWindow									*stageWnckWindow;
	guint										signalID;
	gulong										handlerID;
	gint										width, height;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_BACKEND_GDK(inBackend));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_WINDOW_X11(inStageWindow));

	self=XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_BACKEND_GDK(inBackend);
	priv=self->priv;
	stageWindow=XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW_X11(inStageWindow);

	/* Get GDK stage window */
	stageGdkWindow=clutter_gdk_get_stage_window(xfce_overview_workspaces_window_tracker_window_get_stage(inStageWindow));
	if(!stageGdkWindow)
	{
		g_critical("Could not get real stage window to show");
		return;
	}

	/* Window of stage should always be above all other windows, pinned to all
	 * workspaces, not be listed in window pager and set to fullscreen
	 */
	gdk_window_set_skip_taskbar_hint(stageGdkWindow, TRUE);
	gdk_window_set_skip_pager_hint(stageGdkWindow, TRUE);
	gdk_window_set_keep_above(stageGdkWindow, TRUE);
	gdk_window_stick(stageGdkWindow);
	gdk_window_set_fullscreen_mode(stageGdkWindow, GDK_FULLSCREEN_ON_ALL_MONITORS);
	gdk_window_fullscreen(stageGdkWindow);

	/* Get wnck window of window */
	stageWnckWindow=wnck_window_get(gdk_x11_window_get_xid(stageGdkWindow));

	/* Connect signals if not already connected */
	signalID=g_signal_lookup("state-changed", WNCK_TYPE_WINDOW);
	handlerID=g_signal_handler_find(stageWnckWindow,
									G_SIGNAL_MATCH_ID | G_SIGNAL_MATCH_FUNC | G_SIGNAL_MATCH_DATA,
									signalID,
									0,
									NULL,
									G_CALLBACK(_xfce_overview_workspaces_window_tracker_backend_gdk_on_stage_state_changed),
									stageWindow);
	if(!handlerID)
	{
		handlerID=g_signal_connect(stageWnckWindow,
									"state-changed",
									G_CALLBACK(_xfce_overview_workspaces_window_tracker_backend_gdk_on_stage_state_changed),
									stageWindow);
		XFCE_OVERVIEW_WORKSPACES_DEBUG(self, WINDOWS,
							"Connecting signal handler %lu to 'state-changed' at window %p (wnck-window=%p)",
							handlerID,
							stageWindow,
							stageWnckWindow);
	}

	signalID=g_signal_lookup("screen-size-changed", XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_TRACKER);
	handlerID=g_signal_handler_find(priv->windowTracker,
									G_SIGNAL_MATCH_ID | G_SIGNAL_MATCH_FUNC | G_SIGNAL_MATCH_DATA,
									signalID,
									0,
									NULL,
									G_CALLBACK(_xfce_overview_workspaces_window_tracker_backend_gdk_on_stage_screen_size_changed),
									stageWindow);
	if(!handlerID)
	{
		handlerID=g_signal_connect(priv->windowTracker,
									"screen-size-changed",
									G_CALLBACK(_xfce_overview_workspaces_window_tracker_backend_gdk_on_stage_screen_size_changed),
									stageWindow);
		XFCE_OVERVIEW_WORKSPACES_DEBUG(self, WINDOWS,
							"Connecting signal handler %lu to 'screen-size-changed' at window %p (wnck-window=%p)",
							handlerID,
							stageWindow,
							stageWnckWindow);
	}
	xfce_overview_workspaces_window_tracker_get_screen_size(XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER(priv->windowTracker), &width, &height);
	_xfce_overview_workspaces_window_tracker_backend_gdk_on_stage_screen_size_changed(XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER(priv->windowTracker),
																			width,
																			height,
																			inStageWindow);

	/* Now the window is set up and we can show it. Take focus once so
	 * ESC and key handling work. Focus is not forced back afterwards:
	 * if the user clicks outside, the stage hides instead (see
	 * XfceOverviewWorkspacesStage active-window-changed handler).
	 */
	xfce_overview_workspaces_window_tracker_window_show(inStageWindow);
	wnck_window_activate_transient(stageWnckWindow, xfce_overview_workspaces_window_tracker_x11_get_time());
}

/* Unset up and hide stage window */
static void _xfce_overview_workspaces_window_tracker_backend_gdk_window_tracker_backend_hide_stage_window(XfceOverviewWorkspacesWindowTrackerBackend *inBackend,
																								XfceOverviewWorkspacesWindowTrackerWindow *inStageWindow)
{
	XfceOverviewWorkspacesWindowTrackerBackendGDK			*self;
	XfceOverviewWorkspacesWindowTrackerBackendGDKPrivate	*priv;
	XfceOverviewWorkspacesWindowTrackerWindowX11			*stageWindow;
	GdkWindow									*stageGdkWindow;
	WnckWindow									*stageWnckWindow;
	guint										signalID;
	gulong										handlerID;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_BACKEND_GDK(inBackend));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_WINDOW_X11(inStageWindow));

	self=XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_BACKEND_GDK(inBackend);
	priv=self->priv;
	stageWindow=XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW_X11(inStageWindow);

	/* Get GDK stage window */
	stageGdkWindow=clutter_gdk_get_stage_window(xfce_overview_workspaces_window_tracker_window_get_stage(inStageWindow));
	if(!stageGdkWindow)
	{
		g_critical("Could not get real stage window to show");
		return;
	}

	/* First hide window before removing signals etc. */
	xfce_overview_workspaces_window_tracker_window_hide(inStageWindow);

	/* Get wnck window of window */
	stageWnckWindow=wnck_window_get(gdk_x11_window_get_xid(stageGdkWindow));

	/* Disconnect signals */
	signalID=g_signal_lookup("state-changed", WNCK_TYPE_WINDOW);
	handlerID=g_signal_handler_find(stageWnckWindow,
									G_SIGNAL_MATCH_ID | G_SIGNAL_MATCH_FUNC | G_SIGNAL_MATCH_DATA,
									signalID,
									0,
									NULL,
									G_CALLBACK(_xfce_overview_workspaces_window_tracker_backend_gdk_on_stage_state_changed),
									stageWindow);
	if(handlerID)
	{
		g_signal_handler_disconnect(stageWnckWindow, handlerID);
		XFCE_OVERVIEW_WORKSPACES_DEBUG(self, WINDOWS,
							"Disconnecting handler %lu for signal 'state-changed' at window %p (wnck-window=%p)",
							handlerID,
							stageWindow,
							stageWnckWindow);
	}

	signalID=g_signal_lookup("screen-size-changed", XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_TRACKER);
	handlerID=g_signal_handler_find(priv->windowTracker,
									G_SIGNAL_MATCH_ID | G_SIGNAL_MATCH_FUNC | G_SIGNAL_MATCH_DATA,
									signalID,
									0,
									NULL,
									G_CALLBACK(_xfce_overview_workspaces_window_tracker_backend_gdk_on_stage_screen_size_changed),
									stageWindow);
	if(handlerID)
	{
		g_signal_handler_disconnect(priv->windowTracker, handlerID);
		XFCE_OVERVIEW_WORKSPACES_DEBUG(self, WINDOWS,
							"Disconnecting handler %lu for signal 'screen-size-changed' at window %p (wnck-window=%p)",
							handlerID,
							stageWindow,
							stageWnckWindow);
	}
}

/* Interface initialization
 * Set up default functions
 */
static void _xfce_overview_workspaces_window_tracker_backend_gdk_window_tracker_backend_iface_init(XfceOverviewWorkspacesWindowTrackerBackendInterface *iface)
{
	iface->get_name=_xfce_overview_workspaces_window_tracker_backend_gdk_window_tracker_backend_get_name;

	iface->get_window_tracker=_xfce_overview_workspaces_window_tracker_backend_gdk_window_tracker_backend_get_window_tracker;

	iface->get_window_for_stage=_xfce_overview_workspaces_window_tracker_backend_gdk_window_tracker_backend_get_window_for_stage;
	iface->get_stage_from_window=_xfce_overview_workspaces_window_tracker_backend_gdk_window_tracker_backend_get_stage_from_window;
	iface->show_stage_window=_xfce_overview_workspaces_window_tracker_backend_gdk_window_tracker_backend_show_stage_window;
	iface->hide_stage_window=_xfce_overview_workspaces_window_tracker_backend_gdk_window_tracker_backend_hide_stage_window;
}


/* IMPLEMENTATION: GObject */

/* Dispose this object */
static void _xfce_overview_workspaces_window_tracker_backend_gdk_dispose(GObject *inObject)
{
	XfceOverviewWorkspacesWindowTrackerBackendGDK				*self=XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_BACKEND_GDK(inObject);
	XfceOverviewWorkspacesWindowTrackerBackendGDKPrivate		*priv=self->priv;

	/* Dispose allocated resources */
	if(priv->windowTracker)
	{
		g_object_unref(priv->windowTracker);
		priv->windowTracker=NULL;
	}

	/* Call parent's class dispose method */
	G_OBJECT_CLASS(xfce_overview_workspaces_window_tracker_backend_gdk_parent_class)->dispose(inObject);
}

/* Class initialization
 * Override functions in parent classes and define properties
 * and signals
 */
void xfce_overview_workspaces_window_tracker_backend_gdk_class_init(XfceOverviewWorkspacesWindowTrackerBackendGDKClass *klass)
{
	GObjectClass						*gobjectClass=G_OBJECT_CLASS(klass);

	/* Override functions */
	gobjectClass->dispose=_xfce_overview_workspaces_window_tracker_backend_gdk_dispose;
}

/* Object initialization
 * Create private structure and set up default values
 */
void xfce_overview_workspaces_window_tracker_backend_gdk_init(XfceOverviewWorkspacesWindowTrackerBackendGDK *self)
{
	XfceOverviewWorkspacesWindowTrackerBackendGDKPrivate		*priv;

	priv=self->priv=xfce_overview_workspaces_window_tracker_backend_gdk_get_instance_private(self);

	XFCE_OVERVIEW_WORKSPACES_DEBUG(self, WINDOWS, "Initializing GDK-X11 window tracker backend");

	/* Create window tracker instance */
	priv->windowTracker=g_object_new(XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_TRACKER_X11, NULL);
}

/* IMPLEMENTATION: Public API */

/**
 * xfce_overview_workspaces_window_tracker_backend_gdk_new:
 *
 * Creates a new #XfceOverviewWorkspacesWindowTrackerBackendGDK backend for use with
 * Clutter's GDK backend.
 *
 * Currently only the X11 backend of GDK is supported.
 *
 * Return value: The newly created #XfceOverviewWorkspacesWindowTrackerBackend
 */
XfceOverviewWorkspacesWindowTrackerBackend* xfce_overview_workspaces_window_tracker_backend_gdk_new(void)
{
	GdkDisplay						*display;
	gboolean						supported;

	/* Check for supported windowing system */
	supported=FALSE;

	display=gdk_display_manager_get_default_display(gdk_display_manager_get());
#ifdef GDK_WINDOWING_X11
	if(!supported && GDK_IS_X11_DISPLAY(display))
	{
		XFCE_OVERVIEW_WORKSPACES_DEBUG(NULL, WINDOWS, "GDK windowing system is X11");
		supported=TRUE;
	}
#endif

	/* If no supported windowing system was found, print a message and return
	 * NULL to indicate an error.
	 */
	if(!supported)
	{
		XFCE_OVERVIEW_WORKSPACES_DEBUG(NULL, WINDOWS, "Cannot create GDK window tracker backend as no supported backend found");
		return(NULL);
	}

	/* Create window tracker backend and return it */
	return(g_object_new(XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_TRACKER_BACKEND_GDK, NULL));
}
