/*
 * hot-corner: Activates application when pointer is move to a corner
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

#include "hot-corner.h"

#include <libxfce-overview-workspaces/libxfce-overview-workspaces.h>
#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>
#include <math.h>

#include "hot-corner-settings.h"


/* Define this class in GObject system */
struct _XfceOverviewWorkspacesHotCornerPrivate
{
	/* Instance related */
	XfceOverviewWorkspacesCore							*core;
	XfceOverviewWorkspacesWindowTracker				*windowTracker;
	GdkWindow								*rootWindow;
	GdkSeat									*seat;

	guint									timeoutID;
	GDateTime								*enteredTime;
	gboolean								wasHandledRecently;

	XfceOverviewWorkspacesHotCornerSettings			*settings;
};

G_DEFINE_DYNAMIC_TYPE_EXTENDED(XfceOverviewWorkspacesHotCorner,
								xfce_overview_workspaces_hot_corner,
								G_TYPE_OBJECT,
								0,
								G_ADD_PRIVATE_DYNAMIC(XfceOverviewWorkspacesHotCorner))

/* Define this class in this plugin */
XFCE_OVERVIEW_WORKSPACES_DEFINE_PLUGIN_TYPE(xfce_overview_workspaces_hot_corner);

/* IMPLEMENTATION: Enum XFCE_OVERVIEW_WORKSPACES_TYPE_HOT_CORNER_ACTIVATION_CORNER */

GType xfce_overview_workspaces_hot_corner_activation_corner_get_type(void)
{
	static gsize	static_g_define_type_id=0;

	if(g_once_init_enter(&static_g_define_type_id))
	{
		static const GEnumValue values[]=
		{
			{ XFCE_OVERVIEW_WORKSPACES_HOT_CORNER_ACTIVATION_CORNER_TOP_LEFT, "XFCE_OVERVIEW_WORKSPACES_HOT_CORNER_ACTIVATION_CORNER_TOP_LEFT", "top-left" },
			{ XFCE_OVERVIEW_WORKSPACES_HOT_CORNER_ACTIVATION_CORNER_TOP_RIGHT, "XFCE_OVERVIEW_WORKSPACES_HOT_CORNER_ACTIVATION_CORNER_TOP_RIGHT", "top-right" },
			{ XFCE_OVERVIEW_WORKSPACES_HOT_CORNER_ACTIVATION_CORNER_BOTTOM_LEFT, "XFCE_OVERVIEW_WORKSPACES_HOT_CORNER_ACTIVATION_CORNER_BOTTOM_LEFT", "bottom-left" },
			{ XFCE_OVERVIEW_WORKSPACES_HOT_CORNER_ACTIVATION_CORNER_BOTTOM_RIGHT, "XFCE_OVERVIEW_WORKSPACES_HOT_CORNER_ACTIVATION_CORNER_BOTTOM_RIGHT", "bottom-right" },
			{ 0, NULL, NULL }
		};

		GType	g_define_type_id=g_enum_register_static(g_intern_static_string("XfceOverviewWorkspacesHotCornerActivationCorner"), values);
		g_once_init_leave(&static_g_define_type_id, g_define_type_id);
	}

	return(static_g_define_type_id);
}


/* IMPLEMENTATION: Private variables and methods */
#define POLL_POINTER_POSITION_INTERVAL			100

typedef struct _XfceOverviewWorkspacesHotCornerBox		XfceOverviewWorkspacesHotCornerBox;
struct _XfceOverviewWorkspacesHotCornerBox
{
	gint		x1, y1;
	gint		x2, y2;
};

/* Timeout callback to check for activation or suspend via hot corner */
static gboolean _is_fullscreen_blocked(XfceOverviewWorkspacesWindowTracker *inTracker)
{
	XfceOverviewWorkspacesWindowTrackerWindow		*activeWindow;
	XfceOverviewWorkspacesWindowTrackerWindowState	activeWindowState;

	g_return_val_if_fail(!inTracker || XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER(inTracker), FALSE);

	if(!inTracker) return(FALSE);

	activeWindow=xfce_overview_workspaces_window_tracker_get_active_window(inTracker);
	if(!activeWindow) return(FALSE);

	activeWindowState=xfce_overview_workspaces_window_tracker_window_get_state(activeWindow);
	if((activeWindowState & XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW_STATE_FULLSCREEN) &&
		!xfce_overview_workspaces_window_tracker_window_is_stage(activeWindow))
	{
		return(TRUE);
	}

	return(FALSE);
}

static void _hot_rect(const XfceOverviewWorkspacesHotCornerBox *inMonitorRect, gint inCorner, gint inRadius, XfceOverviewWorkspacesHotCornerBox *outHotRect)
{
	g_return_if_fail(inMonitorRect);
	g_return_if_fail(outHotRect);

	/* Get rectangle where pointer must be inside to activate hot corner */
	switch(inCorner)
	{
		case XFCE_OVERVIEW_WORKSPACES_HOT_CORNER_ACTIVATION_CORNER_TOP_RIGHT:
			outHotRect->x2=inMonitorRect->x2;
			outHotRect->x1=MAX(inMonitorRect->x2-inRadius, inMonitorRect->x1);
			outHotRect->y1=inMonitorRect->y1;
			outHotRect->y2=MIN(inMonitorRect->y1+inRadius, inMonitorRect->y2);
			break;

		case XFCE_OVERVIEW_WORKSPACES_HOT_CORNER_ACTIVATION_CORNER_BOTTOM_LEFT:
			outHotRect->x1=inMonitorRect->x1;
			outHotRect->x2=MIN(inMonitorRect->x1+inRadius, inMonitorRect->x2);
			outHotRect->y2=inMonitorRect->y2;
			outHotRect->y1=MAX(inMonitorRect->y2-inRadius, inMonitorRect->y1);
			break;

		case XFCE_OVERVIEW_WORKSPACES_HOT_CORNER_ACTIVATION_CORNER_BOTTOM_RIGHT:
			outHotRect->x2=inMonitorRect->x2;
			outHotRect->x1=MAX(inMonitorRect->x2-inRadius, inMonitorRect->x1);
			outHotRect->y2=inMonitorRect->y2;
			outHotRect->y1=MAX(inMonitorRect->y2-inRadius, inMonitorRect->y1);
			break;

		case XFCE_OVERVIEW_WORKSPACES_HOT_CORNER_ACTIVATION_CORNER_TOP_LEFT:
		default:
			outHotRect->x1=inMonitorRect->x1;
			outHotRect->x2=MIN(inMonitorRect->x1+inRadius, inMonitorRect->x2);
			outHotRect->y1=inMonitorRect->y1;
			outHotRect->y2=MIN(inMonitorRect->y1+inRadius, inMonitorRect->y2);
			break;
	}
}

static gboolean _xfce_overview_workspaces_hot_corner_check_hot_corner(gpointer inUserData)
{
	XfceOverviewWorkspacesHotCorner							*self;
	XfceOverviewWorkspacesHotCornerPrivate						*priv;
	GdkDevice										*pointerDevice;
	gint											pointerX, pointerY;
	XfceOverviewWorkspacesHotCornerSettingsActivationCorner	activationCorner;
	gint											activationRadius;
	gint64											activationDuration;
	gboolean										primaryMonitorOnly;
	XfceOverviewWorkspacesWindowTrackerMonitor					*monitor;
	XfceOverviewWorkspacesHotCornerBox							monitorRect;
	XfceOverviewWorkspacesHotCornerBox							hotCornerRect;
	GDateTime										*currentTime;
	GTimeSpan										timeDiff;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_HOT_CORNER(inUserData), G_SOURCE_CONTINUE);

	self=XFCE_OVERVIEW_WORKSPACES_HOT_CORNER(inUserData);
	priv=self->priv;

	/* Get all settings now which are used within this function */
	activationCorner=xfce_overview_workspaces_hot_corner_settings_get_activation_corner(priv->settings);
	activationRadius=xfce_overview_workspaces_hot_corner_settings_get_activation_radius(priv->settings);
	activationDuration=xfce_overview_workspaces_hot_corner_settings_get_activation_duration(priv->settings);
	primaryMonitorOnly=xfce_overview_workspaces_hot_corner_settings_get_primary_monitor_only(priv->settings);

	/* Do nothing if current window is fullscreen but not this application */
	if(_is_fullscreen_blocked(priv->windowTracker))
	{
		return(G_SOURCE_CONTINUE);
	}

	/* Get current position of pointer */
	pointerDevice=gdk_seat_get_pointer(priv->seat);
	if(!pointerDevice)
	{
		g_critical("Could not get pointer to determine pointer position");
		return(G_SOURCE_CONTINUE);
	}

	gdk_window_get_device_position(priv->rootWindow, pointerDevice, &pointerX, &pointerY, NULL);

	/* Get monitor and its position and size at pointer position */
	monitor=xfce_overview_workspaces_window_tracker_get_monitor_by_position(priv->windowTracker, pointerX, pointerY);
	if(monitor)
	{
		gint										monitorWidth, monitorHeight;

		xfce_overview_workspaces_window_tracker_monitor_get_geometry(monitor,
														&monitorRect.x1,
														&monitorRect.y1,
														&monitorWidth,
														&monitorHeight);
		monitorRect.x2=monitorRect.x1+monitorWidth;
		monitorRect.y2=monitorRect.y1+monitorHeight;
	}
		else
		{
			/* Set position to 0,0 and size to screen size */
			monitorRect.x1=monitorRect.y1=0;
			xfce_overview_workspaces_window_tracker_get_screen_size(priv->windowTracker, &monitorRect.x2, &monitorRect.y2);
		}

	/* Check pointer in currently iterated monitor should be checked */
	if(primaryMonitorOnly &&
		monitor &&
		!xfce_overview_workspaces_window_tracker_monitor_is_primary(monitor))
	{
		return(G_SOURCE_CONTINUE);
	}

	/* Get rectangle where pointer must be inside to activate hot corner */
	_hot_rect(&monitorRect, activationCorner, activationRadius, &hotCornerRect);

	/* Check if pointer is in configured hot corner for a configured interval.
	 * If it is not reset entered time and return immediately without doing anything.
	 */
	if(pointerX<hotCornerRect.x1 || pointerX>=hotCornerRect.x2 ||
		pointerY<hotCornerRect.y1 || pointerY>=hotCornerRect.y2)
	{
		/* Reset entered time */
		if(priv->enteredTime)
		{
			g_date_time_unref(priv->enteredTime);
			priv->enteredTime=NULL;
		}

		return(G_SOURCE_CONTINUE);
	}

	/* If no entered time was registered yet we assume the pointer is in hot corner
	 * for the first time. So remember entered time for next polling interval.
	 */
	if(!priv->enteredTime)
	{
		/* Remember entered time */
		priv->enteredTime=g_date_time_new_now_local();

		/* Reset handled flag to get duration checked next time */
		priv->wasHandledRecently=FALSE;

		return(G_SOURCE_CONTINUE);
	}

	/* If handled flag is set then do nothing to avoid flapping between activation
	 * and suspending application once the activation duration was reached.
	 */
	if(priv->wasHandledRecently) return(G_SOURCE_CONTINUE);

	/* We know the time the pointer entered hot corner. Check if pointer have stayed
	 * in hot corner for the duration to activate/suspend application. If duration
	 * was not reached yet, just return immediately.
	 */
	currentTime=g_date_time_new_now_local();
	timeDiff=g_date_time_difference(currentTime, priv->enteredTime);
	g_date_time_unref(currentTime);

	if(timeDiff<(activationDuration*G_TIME_SPAN_MILLISECOND)) return(G_SOURCE_CONTINUE);

	/* Activation duration reached so resume core if suspended or suspend it
	 * if active currently.
	 */
	if(!xfce_overview_workspaces_core_is_suspended(priv->core))
	{
		xfce_overview_workspaces_core_suspend(priv->core);
	}
		else
		{
			xfce_overview_workspaces_core_resume(priv->core);
		}

	/* Set flag that activation was handled recently */
	priv->wasHandledRecently=TRUE;

	return(G_SOURCE_CONTINUE);
}

/* IMPLEMENTATION: GObject */

/* Dispose this object */
static void _xfce_overview_workspaces_hot_corner_dispose(GObject *inObject)
{
	XfceOverviewWorkspacesHotCorner			*self=XFCE_OVERVIEW_WORKSPACES_HOT_CORNER(inObject);
	XfceOverviewWorkspacesHotCornerPrivate		*priv=self->priv;

	/* Release allocated resources */
	if(priv->enteredTime)
	{
		g_date_time_unref(priv->enteredTime);
		priv->enteredTime=NULL;
	}

	if(priv->windowTracker)
	{
		g_object_unref(priv->windowTracker);
		priv->windowTracker=NULL;
	}

	if(priv->timeoutID)
	{
		g_source_remove(priv->timeoutID);
		priv->timeoutID=0;
	}

	if(priv->settings)
	{
		g_object_unref(priv->settings);
		priv->settings=NULL;
	}

	if(priv->core)
	{
		priv->core=NULL;
	}

	/* Call parent's class dispose method */
	G_OBJECT_CLASS(xfce_overview_workspaces_hot_corner_parent_class)->dispose(inObject);
}

/* Class initialization
 * Override functions in parent classes and define properties
 * and signals
 */
void xfce_overview_workspaces_hot_corner_class_init(XfceOverviewWorkspacesHotCornerClass *klass)
{
	GObjectClass			*gobjectClass=G_OBJECT_CLASS(klass);

	/* Override functions */
	gobjectClass->dispose=_xfce_overview_workspaces_hot_corner_dispose;
}

/* Class finalization */
void xfce_overview_workspaces_hot_corner_class_finalize(XfceOverviewWorkspacesHotCornerClass *klass)
{
}

/* Object initialization
 * Create private structure and set up default values
 */
void xfce_overview_workspaces_hot_corner_init(XfceOverviewWorkspacesHotCorner *self)
{
	XfceOverviewWorkspacesHotCornerPrivate		*priv;
	GdkScreen						*screen;
	GdkDisplay						*display;

	self->priv=priv=xfce_overview_workspaces_hot_corner_get_instance_private(self);

	/* Set up default values */
	priv->core=xfce_overview_workspaces_core_get_default();
	priv->windowTracker=xfce_overview_workspaces_core_get_window_tracker(priv->core);
	priv->rootWindow=NULL;
	priv->seat=NULL;

	priv->timeoutID=0;
	priv->enteredTime=NULL;
	priv->wasHandledRecently=FALSE;

	/* Set up settings */
	priv->settings=xfce_overview_workspaces_hot_corner_settings_new();

	/* Get device manager for polling pointer position */
	if(xfce_overview_workspaces_core_can_suspend(priv->core))
	{
		screen=gdk_screen_get_default();
		priv->rootWindow=gdk_screen_get_root_window(screen);
		if(priv->rootWindow)
		{
			display=gdk_window_get_display(priv->rootWindow);
			priv->seat=gdk_display_get_default_seat(display);
		}
			else
			{
				g_critical("Disabling hot-corner plugin because the root window to determine pointer position could not be found.");
			}

		if(priv->seat)
		{
			/* Start polling pointer position */
			priv->timeoutID=g_timeout_add(POLL_POINTER_POSITION_INTERVAL,
											(GSourceFunc)_xfce_overview_workspaces_hot_corner_check_hot_corner,
											self);
		}
			else
			{
				g_critical("Disabling hot-corner plugin because the device manager to determine pointer position could not be found.");
			}
	}
		else
		{
			g_warning("Disabling hot-corner plugin because core cannot suspend.");
		}
}


/* IMPLEMENTATION: Public API */

/* Create new instance */
XfceOverviewWorkspacesHotCorner* xfce_overview_workspaces_hot_corner_new(void)
{
	GObject		*hotCorner;

	hotCorner=g_object_new(XFCE_OVERVIEW_WORKSPACES_TYPE_HOT_CORNER, NULL);
	if(!hotCorner) return(NULL);

	return(XFCE_OVERVIEW_WORKSPACES_HOT_CORNER(hotCorner));
}
