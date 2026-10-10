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

#include <libxfdashboard/x11/window-tracker-backend-x11.h>

#include <glib/gi18n-lib.h>
#include <clutter/x11/clutter-x11.h>
#include <gdk/gdkx.h>

#include <libxfdashboard/x11/window-tracker-x11.h>
#include <libxfdashboard/x11/window-tracker-window-x11.h>
#include <libxfdashboard/marshal.h>
#include <libxfdashboard/debug.h>


/* Define this class in GObject system */
static void _xfdashboard_window_tracker_backend_x11_window_tracker_backend_iface_init(XfdashboardWindowTrackerBackendInterface *iface);

struct _XfdashboardWindowTrackerBackendX11Private
{
	/* Instance related */
	XfdashboardWindowTrackerX11		*windowTracker;
};

G_DEFINE_TYPE_WITH_CODE(XfdashboardWindowTrackerBackendX11,
						xfdashboard_window_tracker_backend_x11,
						G_TYPE_OBJECT,
						G_ADD_PRIVATE(XfdashboardWindowTrackerBackendX11)
						G_IMPLEMENT_INTERFACE(XFDASHBOARD_TYPE_WINDOW_TRACKER_BACKEND, _xfdashboard_window_tracker_backend_x11_window_tracker_backend_iface_init))

/* IMPLEMENTATION: Private variables and methods */

/* State of stage window changed */
static void _xfdashboard_window_tracker_backend_x11_on_stage_state_changed(WnckWindow *inWindow,
																			WnckWindowState inChangedMask,
																			WnckWindowState inNewValue,
																			gpointer inUserData)
{
	G_GNUC_UNUSED XfdashboardWindowTrackerWindowX11			*stageWindow;

	g_return_if_fail(WNCK_IS_WINDOW(inWindow));
	g_return_if_fail(XFDASHBOARD_IS_WINDOW_TRACKER_WINDOW_X11(inUserData));

	stageWindow=XFDASHBOARD_WINDOW_TRACKER_WINDOW_X11(inUserData);

	/* Set 'skip-tasklist' if changed */
	if((inChangedMask & WNCK_WINDOW_STATE_SKIP_TASKLIST) &&
		!(inNewValue & WNCK_WINDOW_STATE_SKIP_TASKLIST))
	{
		wnck_window_set_skip_tasklist(WNCK_WINDOW(inWindow), TRUE);
		XFDASHBOARD_DEBUG(inWindow, WINDOWS,
							"State 'skip-tasklist' for stage window %p (wnck-window=%p) needs reset",
							stageWindow,
							inWindow);
	}

	/* Set 'skip-pager' if changed */
	if((inChangedMask & WNCK_WINDOW_STATE_SKIP_PAGER) &&
		!(inNewValue & WNCK_WINDOW_STATE_SKIP_PAGER))
	{
		wnck_window_set_skip_pager(WNCK_WINDOW(inWindow), TRUE);
		XFDASHBOARD_DEBUG(inWindow, WINDOWS,
							"State 'skip-pager' for stage window %p (wnck-window=%p) needs reset",
							stageWindow,
							inWindow);
	}

	/* Set 'make-above' if changed */
	if((inChangedMask & WNCK_WINDOW_STATE_ABOVE) &&
		!(inNewValue & WNCK_WINDOW_STATE_ABOVE))
	{
		wnck_window_make_above(WNCK_WINDOW(inWindow));
		XFDASHBOARD_DEBUG(inWindow, WINDOWS,
							"State 'make-above' for stage window %p (wnck-window=%p) needs reset",
							stageWindow,
							inWindow);
	}
}

/* The active window changed. Reselect stage window as active one if it is visible */
static void _xfdashboard_window_tracker_backend_x11_on_stage_active_window_changed(WnckScreen *inScreen,
																					WnckWindow *inPreviousWindow,
																					gpointer inUserData)
{
	XfdashboardWindowTrackerWindowX11			*stageWindow;
	XfdashboardWindowTrackerWindowState			stageWindowState;
	WnckWindow									*stageWnckWindow;
	WnckWindow									*activeWindow;
	gboolean									reselect;

	g_return_if_fail(WNCK_IS_SCREEN(inScreen));
	g_return_if_fail(inPreviousWindow==NULL || WNCK_IS_WINDOW(inPreviousWindow));
	g_return_if_fail(XFDASHBOARD_IS_WINDOW_TRACKER_WINDOW_X11(inUserData));

	stageWindow=XFDASHBOARD_WINDOW_TRACKER_WINDOW_X11(inUserData);
	reselect=FALSE;

	/* Get wnck window of stage window */
	stageWnckWindow=xfdashboard_window_tracker_window_x11_get_window(stageWindow);
	if(!stageWnckWindow)
	{
		g_critical("Could not get real stage window to handle signal 'active-window-changed'");
		return;
	}

	/* Get stage of stage window */
	stageWindowState=xfdashboard_window_tracker_window_get_state(XFDASHBOARD_WINDOW_TRACKER_WINDOW(stageWindow));

	/* Reactive stage window if not hidden */
	activeWindow=wnck_screen_get_active_window(inScreen);

	if(inPreviousWindow && inPreviousWindow==stageWnckWindow) reselect=TRUE;
	if(!activeWindow || activeWindow!=stageWnckWindow) reselect=TRUE;
	if(!(stageWindowState & (XFDASHBOARD_WINDOW_TRACKER_WINDOW_STATE_MINIMIZED | XFDASHBOARD_WINDOW_TRACKER_WINDOW_STATE_HIDDEN))) reselect=TRUE;

	if(reselect)
	{
		wnck_window_activate_transient(stageWnckWindow, xfdashboard_window_tracker_x11_get_time());
		XFDASHBOARD_DEBUG(stageWindow, WINDOWS,
							"Active window changed from %p (%s) to %p (%s) but stage window %p (wnck-window=%p) is visible and should be active one",
							inPreviousWindow, inPreviousWindow ? wnck_window_get_name(inPreviousWindow) : "<nil>",
							activeWindow, activeWindow ? wnck_window_get_name(activeWindow) : "<nil>",
							stageWindow,
							stageWnckWindow);
	}
}

/* Size of screen has changed so resize stage window */
static void _xfdashboard_window_tracker_backend_x11_on_stage_screen_size_changed(XfdashboardWindowTracker *inWindowTracker,
																					gint inWidth,
																					gint inHeight,
																					gpointer inUserData)
{
#ifdef HAVE_XINERAMA
	XfdashboardWindowTrackerWindowX11	*realStageWindow;

	g_return_if_fail(XFDASHBOARD_IS_WINDOW_TRACKER(inWindowTracker));
	g_return_if_fail(XFDASHBOARD_IS_WINDOW_TRACKER_WINDOW_X11(inUserData));

	realStageWindow=XFDASHBOARD_WINDOW_TRACKER_WINDOW_X11(inUserData);

	/* Top-strip mode: never fullscreen-span. The stage window is a top bar
	 * (full width, ~22% height at y=0). Skip _NET_WM_FULLSCREEN_MONITORS.
	 */
	{
		GdkDisplay			*topDisplay;
		GdkScreen			*topScreen;
		GdkMonitor			*primaryMonitor;
		GdkRectangle		geometry;
		gint				stripHeight;
		WnckWindow			*topStageWindow;

		topDisplay=gdk_display_get_default();
		topScreen=gdk_screen_get_default();
		if(topDisplay==NULL || topScreen==NULL) return;

		primaryMonitor=gdk_display_get_primary_monitor(gdk_screen_get_display(topScreen));
		gdk_monitor_get_geometry(primaryMonitor, &geometry);
		stripHeight=(gint)(geometry.height*0.22f);
		if(stripHeight<1) stripHeight=1;

		topStageWindow=xfdashboard_window_tracker_window_x11_get_window(realStageWindow);
		if(topStageWindow)
		{
			wnck_window_set_geometry(topStageWindow,
										WNCK_WINDOW_GRAVITY_STATIC,
										WNCK_WINDOW_CHANGE_X | WNCK_WINDOW_CHANGE_Y | WNCK_WINDOW_CHANGE_WIDTH | WNCK_WINDOW_CHANGE_HEIGHT,
										geometry.x, geometry.y, geometry.width, stripHeight);
		}
		return;
	}

#else
	XfdashboardWindowTrackerWindowX11	*realStageWindow;
	WnckWindow							*stageWindow;
	GdkScreen							*screen;
	gint								primaryMonitor;
	GdkRectangle						geometry;

	g_return_if_fail(XFDASHBOARD_IS_WINDOW_TRACKER(inWindowTracker));
	g_return_if_fail(XFDASHBOARD_IS_WINDOW_TRACKER_WINDOW_X11(inUserData));

	realStageWindow=XFDASHBOARD_WINDOW_TRACKER_WINDOW_X11(inUserData);

	XFDASHBOARD_DEBUG(inWindowTracker, WINDOWS, "No support for multiple monitor: Setting fullscreen on primary monitor");

	/* Get wnck window for stage window object as it is needed a lot from this
	 * point on.
	 */
	stageWindow=xfdashboard_window_tracker_window_x11_get_window(realStageWindow);

	/* Get screen */
	screen=gdk_screen_get_default();

	/* Get position and size of primary monitor: top strip (full width,
	 * ~22% height at y=0). Tested with xfwm4.
	 */
	primaryMonitor=gdk_screen_get_primary_monitor(screen);
	gdk_screen_get_monitor_geometry(screen, primaryMonitor, &geometry);
	geometry.height=(gint)(geometry.height*0.22f);
	if(geometry.height<1) geometry.height=1;
	wnck_window_set_geometry(stageWindow,
								WNCK_WINDOW_GRAVITY_STATIC,
								WNCK_WINDOW_CHANGE_X | WNCK_WINDOW_CHANGE_Y | WNCK_WINDOW_CHANGE_WIDTH | WNCK_WINDOW_CHANGE_HEIGHT,
								geometry.x, geometry.y, geometry.width, geometry.height);

	XFDASHBOARD_DEBUG(inWindowTracker, WINDOWS,
						"Moving stage window to %d,%d and resize to %dx%d",
						geometry.x, geometry.y,
						geometry.width, geometry.height);
#endif
}


/* IMPLEMENTATION: XfdashboardWindowTrackerBackend */

/* Get name of backend */
static const gchar* _xfdashboard_window_tracker_backend_x11_window_tracker_backend_get_name(XfdashboardWindowTrackerBackend *inBackend)
{
	g_return_val_if_fail(XFDASHBOARD_IS_WINDOW_TRACKER_BACKEND_X11(inBackend), NULL);

	/* Return name of backend */
	return("X11");
}

/* Get window tracker instance used by this backend */
static XfdashboardWindowTracker* _xfdashboard_window_tracker_backend_x11_window_tracker_backend_get_window_tracker(XfdashboardWindowTrackerBackend *inBackend)
{
	XfdashboardWindowTrackerBackendX11				*self;
	XfdashboardWindowTrackerBackendX11Private		*priv;

	g_return_val_if_fail(XFDASHBOARD_IS_WINDOW_TRACKER_BACKEND_X11(inBackend), NULL);

	self=XFDASHBOARD_WINDOW_TRACKER_BACKEND_X11(inBackend);
	priv=self->priv;

	/* Return window tracker instance used by this instance but do not take a
	 * reference on it as it will be done by the backend interface when
	 * xfdashboard_window_tracker_backend_get_default() is called.
	 */
	return(XFDASHBOARD_WINDOW_TRACKER(priv->windowTracker));
}

/* Get window of stage */
static XfdashboardWindowTrackerWindow* _xfdashboard_window_tracker_backend_x11_window_tracker_backend_get_window_for_stage(XfdashboardWindowTrackerBackend *inBackend,
																															ClutterStage *inStage)
{
	XfdashboardWindowTrackerBackendX11			*self;
	XfdashboardWindowTrackerBackendX11Private	*priv;
	Window										stageXWindow;
	WnckWindow									*wnckWindow;
	XfdashboardWindowTrackerWindow				*window;

	g_return_val_if_fail(XFDASHBOARD_IS_WINDOW_TRACKER_BACKEND_X11(inBackend), NULL);
	g_return_val_if_fail(CLUTTER_IS_STAGE(inStage), NULL);

	self=XFDASHBOARD_WINDOW_TRACKER_BACKEND_X11(inBackend);
	priv=self->priv;

	/* Get stage X window and translate to needed window type */
	stageXWindow=clutter_x11_get_stage_window(inStage);
	wnckWindow=wnck_window_get(stageXWindow);

	/* Get or create window object for wnck background window */
	window=xfdashboard_window_tracker_x11_get_window_for_wnck(priv->windowTracker, wnckWindow);
	XFDASHBOARD_DEBUG(self, WINDOWS,
						"Resolved stage wnck window %s@%p of stage %s@%p to window object %s@%p",
						G_OBJECT_TYPE_NAME(wnckWindow), wnckWindow,
						G_OBJECT_TYPE_NAME(inStage), inStage,
						G_OBJECT_TYPE_NAME(window), window);

	return(window);
}

/* Get associated stage of window */
static ClutterStage* _xfdashboard_window_tracker_backend_x11_window_tracker_backend_get_stage_from_window(XfdashboardWindowTrackerBackend *inBackend,
																											XfdashboardWindowTrackerWindow *inStageWindow)
{
	G_GNUC_UNUSED XfdashboardWindowTrackerBackendX11			*self;
	XfdashboardWindowTrackerWindowX11			*stageWindow;
	WnckWindow									*stageWnckWindow;
	Window										stageXWindow;
	ClutterStage								*foundStage;
	GSList										*stages;
	GSList										*iter;
	Window										iterXWindow;
	ClutterStage								*stage;

	g_return_val_if_fail(XFDASHBOARD_IS_WINDOW_TRACKER_BACKEND_X11(inBackend), NULL);
	g_return_val_if_fail(XFDASHBOARD_IS_WINDOW_TRACKER_WINDOW_X11(inStageWindow), NULL);

	self=XFDASHBOARD_WINDOW_TRACKER_BACKEND_X11(inBackend);
	stageWindow=XFDASHBOARD_WINDOW_TRACKER_WINDOW_X11(inStageWindow);

	/* Get wnck and X window of stage window */
	stageWnckWindow=xfdashboard_window_tracker_window_x11_get_window(stageWindow);
	if(!stageWnckWindow)
	{
		XFDASHBOARD_DEBUG(self, WINDOWS,
							"Could not get wnck window for window %s@%p",
							G_OBJECT_TYPE_NAME(stageWindow),
							stageWindow);
		g_critical("Could not get real stage window to find stage");
		return(NULL);
	}

	stageXWindow=wnck_window_get_xid(stageWnckWindow);
	if(stageXWindow==None)
	{
		XFDASHBOARD_DEBUG(self, WINDOWS,
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
			iterXWindow=clutter_x11_get_stage_window(stage);
			if(iterXWindow==stageXWindow) foundStage=stage;
		}
	}
	g_slist_free(stages);

	/* Return stage found */
	return(foundStage);
}

/* Set up and show window for use as stage */
static void _xfdashboard_window_tracker_backend_x11_window_tracker_backend_show_stage_window(XfdashboardWindowTrackerBackend *inBackend,
																								XfdashboardWindowTrackerWindow *inStageWindow)
{
	XfdashboardWindowTrackerBackendX11			*self;
	XfdashboardWindowTrackerBackendX11Private	*priv;
	XfdashboardWindowTrackerWindowX11			*stageWindow;
	WnckWindow									*stageWnckWindow;
	WnckScreen									*screen;
	guint										signalID;
	gulong										handlerID;
	gint										width, height;

	g_return_if_fail(XFDASHBOARD_IS_WINDOW_TRACKER_BACKEND_X11(inBackend));
	g_return_if_fail(XFDASHBOARD_IS_WINDOW_TRACKER_WINDOW_X11(inStageWindow));

	self=XFDASHBOARD_WINDOW_TRACKER_BACKEND_X11(inBackend);
	priv=self->priv;
	stageWindow=XFDASHBOARD_WINDOW_TRACKER_WINDOW_X11(inStageWindow);

	/* Get wnck window of stage window */
	stageWnckWindow=xfdashboard_window_tracker_window_x11_get_window(stageWindow);
	if(!stageWnckWindow)
	{
		g_critical("Could not get real stage window to show");
		return;
	}

	/* Window of stage is a top strip: above all other windows, pinned to
	 * all workspaces, not listed in pager/tasklist.
	 */
	if(!wnck_window_is_skip_tasklist(stageWnckWindow)) wnck_window_set_skip_tasklist(stageWnckWindow, TRUE);
	if(!wnck_window_is_skip_pager(stageWnckWindow)) wnck_window_set_skip_pager(stageWnckWindow, TRUE);
	if(!wnck_window_is_above(stageWnckWindow)) wnck_window_make_above(stageWnckWindow);
	if(!wnck_window_is_pinned(stageWnckWindow)) wnck_window_pin(stageWnckWindow);

	/* Get screen of window */
	screen=wnck_window_get_screen(stageWnckWindow);

	/* Connect signals if not already connected */
	signalID=g_signal_lookup("state-changed", WNCK_TYPE_WINDOW);
	handlerID=g_signal_handler_find(stageWnckWindow,
									G_SIGNAL_MATCH_ID | G_SIGNAL_MATCH_FUNC | G_SIGNAL_MATCH_DATA,
									signalID,
									0,
									NULL,
									G_CALLBACK(_xfdashboard_window_tracker_backend_x11_on_stage_state_changed),
									stageWindow);
	if(!handlerID)
	{
		handlerID=g_signal_connect(stageWnckWindow,
									"state-changed",
									G_CALLBACK(_xfdashboard_window_tracker_backend_x11_on_stage_state_changed),
									stageWindow);
		XFDASHBOARD_DEBUG(self, WINDOWS,
							"Connecting signal handler %lu to 'state-changed' at window %p (wnck-window=%p)",
							handlerID,
							stageWindow,
							stageWnckWindow);
	}

	signalID=g_signal_lookup("active-window-changed", WNCK_TYPE_SCREEN);
	handlerID=g_signal_handler_find(screen,
									G_SIGNAL_MATCH_ID | G_SIGNAL_MATCH_FUNC | G_SIGNAL_MATCH_DATA,
									signalID,
									0,
									NULL,
									G_CALLBACK(_xfdashboard_window_tracker_backend_x11_on_stage_active_window_changed),
									stageWindow);
	if(!handlerID)
	{
		handlerID=g_signal_connect(screen,
									"active-window-changed",
									G_CALLBACK(_xfdashboard_window_tracker_backend_x11_on_stage_active_window_changed),
									stageWindow);
		XFDASHBOARD_DEBUG(self, WINDOWS,
							"Connecting signal handler %lu to 'active-window-changed' at screen %p of window %p (wnck-window=%p)",
							handlerID,
							screen,
							stageWindow,
							stageWnckWindow);
	}

	signalID=g_signal_lookup("screen-size-changed", XFDASHBOARD_TYPE_WINDOW_TRACKER);
	handlerID=g_signal_handler_find(priv->windowTracker,
									G_SIGNAL_MATCH_ID | G_SIGNAL_MATCH_FUNC | G_SIGNAL_MATCH_DATA,
									signalID,
									0,
									NULL,
									G_CALLBACK(_xfdashboard_window_tracker_backend_x11_on_stage_screen_size_changed),
									stageWindow);
	if(!handlerID)
	{
		handlerID=g_signal_connect(priv->windowTracker,
									"screen-size-changed",
									G_CALLBACK(_xfdashboard_window_tracker_backend_x11_on_stage_screen_size_changed),
									stageWindow);
		XFDASHBOARD_DEBUG(self, WINDOWS,
							"Connecting signal handler %lu to 'screen-size-changed' at window %p (wnck-window=%p)",
							handlerID,
							stageWindow,
							stageWnckWindow);
	}
	xfdashboard_window_tracker_get_screen_size(XFDASHBOARD_WINDOW_TRACKER(priv->windowTracker), &width, &height);
	_xfdashboard_window_tracker_backend_x11_on_stage_screen_size_changed(XFDASHBOARD_WINDOW_TRACKER(priv->windowTracker),
																			width,
																			height,
																			inStageWindow);

	/* Now the window is set up and we can show it */
	xfdashboard_window_tracker_window_show(inStageWindow);
}

/* Unset up and hide stage window */
static void _xfdashboard_window_tracker_backend_x11_window_tracker_backend_hide_stage_window(XfdashboardWindowTrackerBackend *inBackend,
																								XfdashboardWindowTrackerWindow *inStageWindow)
{
	XfdashboardWindowTrackerBackendX11			*self;
	XfdashboardWindowTrackerBackendX11Private	*priv;
	XfdashboardWindowTrackerWindowX11			*stageWindow;
	WnckWindow									*stageWnckWindow;
	WnckScreen									*screen;
	guint										signalID;
	gulong										handlerID;

	g_return_if_fail(XFDASHBOARD_IS_WINDOW_TRACKER_BACKEND_X11(inBackend));
	g_return_if_fail(XFDASHBOARD_IS_WINDOW_TRACKER_WINDOW_X11(inStageWindow));

	self=XFDASHBOARD_WINDOW_TRACKER_BACKEND_X11(inBackend);
	priv=self->priv;
	stageWindow=XFDASHBOARD_WINDOW_TRACKER_WINDOW_X11(inStageWindow);

	/* Get wnck window of stage window */
	stageWnckWindow=xfdashboard_window_tracker_window_x11_get_window(stageWindow);
	if(!stageWnckWindow)
	{
		g_critical("Could not get real stage window to hide");
		return;
	}

	/* First hide window before removing signals etc. */
	xfdashboard_window_tracker_window_hide(inStageWindow);

	/* Get screen of window */
	screen=wnck_window_get_screen(stageWnckWindow);

	/* Disconnect signals */
	signalID=g_signal_lookup("state-changed", WNCK_TYPE_WINDOW);
	handlerID=g_signal_handler_find(stageWnckWindow,
									G_SIGNAL_MATCH_ID | G_SIGNAL_MATCH_FUNC | G_SIGNAL_MATCH_DATA,
									signalID,
									0,
									NULL,
									G_CALLBACK(_xfdashboard_window_tracker_backend_x11_on_stage_state_changed),
									stageWindow);
	if(handlerID)
	{
		g_signal_handler_disconnect(stageWnckWindow, handlerID);
		XFDASHBOARD_DEBUG(self, WINDOWS,
							"Disconnecting handler %lu for signal 'state-changed' at window %p (wnck-window=%p)",
							handlerID,
							stageWindow,
							stageWnckWindow);
	}

	signalID=g_signal_lookup("active-window-changed", WNCK_TYPE_SCREEN);
	handlerID=g_signal_handler_find(screen,
									G_SIGNAL_MATCH_ID | G_SIGNAL_MATCH_FUNC | G_SIGNAL_MATCH_DATA,
									signalID,
									0,
									NULL,
									G_CALLBACK(_xfdashboard_window_tracker_backend_x11_on_stage_active_window_changed),
									stageWindow);
	if(handlerID)
	{
		g_signal_handler_disconnect(screen, handlerID);
		XFDASHBOARD_DEBUG(self, WINDOWS,
							"Disconnecting handler %lu for signal 'active-window-changed' at screen %p of window %p (wnck-window=%p)",
							handlerID,
							screen,
							stageWindow,
							stageWnckWindow);
	}

	signalID=g_signal_lookup("screen-size-changed", XFDASHBOARD_TYPE_WINDOW_TRACKER);
	handlerID=g_signal_handler_find(priv->windowTracker,
									G_SIGNAL_MATCH_ID | G_SIGNAL_MATCH_FUNC | G_SIGNAL_MATCH_DATA,
									signalID,
									0,
									NULL,
									G_CALLBACK(_xfdashboard_window_tracker_backend_x11_on_stage_screen_size_changed),
									stageWindow);
	if(handlerID)
	{
		g_signal_handler_disconnect(priv->windowTracker, handlerID);
		XFDASHBOARD_DEBUG(self, WINDOWS,
							"Disconnecting handler %lu for signal 'screen-size-changed' at window %p (wnck-window=%p)",
							handlerID,
							stageWindow,
							stageWnckWindow);
	}
}

/* Interface initialization
 * Set up default functions
 */
static void _xfdashboard_window_tracker_backend_x11_window_tracker_backend_iface_init(XfdashboardWindowTrackerBackendInterface *iface)
{
	iface->get_name=_xfdashboard_window_tracker_backend_x11_window_tracker_backend_get_name;

	iface->get_window_tracker=_xfdashboard_window_tracker_backend_x11_window_tracker_backend_get_window_tracker;

	iface->get_window_for_stage=_xfdashboard_window_tracker_backend_x11_window_tracker_backend_get_window_for_stage;
	iface->get_stage_from_window=_xfdashboard_window_tracker_backend_x11_window_tracker_backend_get_stage_from_window;
	iface->show_stage_window=_xfdashboard_window_tracker_backend_x11_window_tracker_backend_show_stage_window;
	iface->hide_stage_window=_xfdashboard_window_tracker_backend_x11_window_tracker_backend_hide_stage_window;
}


/* IMPLEMENTATION: GObject */

/* Dispose this object */
static void _xfdashboard_window_tracker_backend_x11_dispose(GObject *inObject)
{
	XfdashboardWindowTrackerBackendX11				*self=XFDASHBOARD_WINDOW_TRACKER_BACKEND_X11(inObject);
	XfdashboardWindowTrackerBackendX11Private		*priv=self->priv;

	/* Dispose allocated resources */
	if(priv->windowTracker)
	{
		g_object_unref(priv->windowTracker);
		priv->windowTracker=NULL;
	}

	/* Call parent's class dispose method */
	G_OBJECT_CLASS(xfdashboard_window_tracker_backend_x11_parent_class)->dispose(inObject);
}

/* Class initialization
 * Override functions in parent classes and define properties
 * and signals
 */
void xfdashboard_window_tracker_backend_x11_class_init(XfdashboardWindowTrackerBackendX11Class *klass)
{
	GObjectClass						*gobjectClass=G_OBJECT_CLASS(klass);

	/* Override functions */
	gobjectClass->dispose=_xfdashboard_window_tracker_backend_x11_dispose;
}

/* Object initialization
 * Create private structure and set up default values
 */
void xfdashboard_window_tracker_backend_x11_init(XfdashboardWindowTrackerBackendX11 *self)
{
	XfdashboardWindowTrackerBackendX11Private		*priv;

	priv=self->priv=xfdashboard_window_tracker_backend_x11_get_instance_private(self);

	XFDASHBOARD_DEBUG(self, WINDOWS, "Initializing X11 window tracker backend");

	/* Create window tracker instance */
	priv->windowTracker=g_object_new(XFDASHBOARD_TYPE_WINDOW_TRACKER_X11, NULL);
}


/* IMPLEMENTATION: Public API */

/**
 * xfdashboard_window_tracker_backend_x11_new:
 *
 * Creates a new #XfdashboardWindowTrackerBackendX11 backend for use with
 * Clutter's X11 backend.
 *
 * Return value: The newly created #XfdashboardWindowTrackerBackend
 */
XfdashboardWindowTrackerBackend* xfdashboard_window_tracker_backend_x11_new(void)
{
	return(g_object_new(XFDASHBOARD_TYPE_WINDOW_TRACKER_BACKEND_X11, NULL));
}
