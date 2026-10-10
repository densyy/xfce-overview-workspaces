/*
 * stage: Global stage of application
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

#include <libxfdashboard/stage.h>

#include <glib/gi18n-lib.h>
#include <clutter/clutter.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <math.h>

#include <libxfdashboard/core.h>
#include <libxfdashboard/viewpad.h>
#include <libxfdashboard/text-box.h>
#include <libxfdashboard/workspace-selector.h>
#include <libxfdashboard/tooltip-action.h>
#include <libxfdashboard/utils.h>
#include <libxfdashboard/focus-manager.h>
#include <libxfdashboard/enums.h>
#include <libxfdashboard/window-tracker.h>
#include <libxfdashboard/window-content.h>
#include <libxfdashboard/stage-interface.h>
#include <libxfdashboard/settings.h>
#include <libxfdashboard/debug.h>


/* Define this class in GObject system */
struct _XfdashboardStagePrivate
{
	/* Properties related */
	XfdashboardStageBackgroundImageType		backgroundType;

	ClutterColor							*backgroundColor;

	/* Actors */
	ClutterActor							*backgroundImageLayer;
	ClutterActor							*backgroundColorLayer;

	gpointer								primaryInterface;
	gpointer								workspaces;
	gpointer								viewpad;
	gpointer								notification;
	gpointer								tooltip;

	/* Instance related */
	XfdashboardWindowTracker				*windowTracker;
	XfdashboardWindowTrackerWindow			*stageWindow;

	gchar									*switchToView;
	gpointer								focusActorOnShow;

	guint									notificationTimeoutID;

	XfdashboardFocusManager					*focusManager;

	XfdashboardSettings						*settings;

	/* Last active-workspace switch (monotonic usec). Focus changes right
	 * after a workspace switch (e.g. scrolling over the strip to navigate
	 * workspaces) must not hide the stage as "clicked outside".
	 */
	gint64									lastWorkspaceSwitchUsec;
};

G_DEFINE_TYPE_WITH_PRIVATE(XfdashboardStage,
							xfdashboard_stage,
							CLUTTER_TYPE_STAGE)

/* Properties */
enum
{
	PROP_0,

	PROP_BACKGROUND_IMAGE_TYPE,
	PROP_BACKGROUND_COLOR,

	PROP_SWITCH_TO_VIEW,

	PROP_LAST
};

static GParamSpec* XfdashboardStageProperties[PROP_LAST]={ 0, };

/* Signals */
enum
{
	SIGNAL_ACTOR_CREATED,

	SIGNAL_SHOW_TOOLTIP,
	SIGNAL_HIDE_TOOLTIP,

	SIGNAL_LAST
};

static guint XfdashboardStageSignals[SIGNAL_LAST]={ 0, };


/* Forward declaration */
static void _xfdashboard_stage_on_window_opened(XfdashboardStage *self,
													XfdashboardWindowTrackerWindow *inWindow,
													gpointer inUserData);


/* IMPLEMENTATION: Private variables and methods */

#define XFDASHBOARD_THEME_LAYOUT_PRIMARY				"primary"
#define XFDASHBOARD_THEME_LAYOUT_SECONDARY				"secondary"

typedef struct _XfdashboardStageThemeInterfaceData		XfdashboardStageThemeInterfaceData;
struct _XfdashboardStageThemeInterfaceData
{
	ClutterActor		*actor;
	GPtrArray			*focusables;
	ClutterActor		*focus;
};

/* Handle an event */
static gboolean _xfdashboard_stage_event(ClutterActor *inActor, ClutterEvent *inEvent)
{
	XfdashboardStage			*self;
	XfdashboardStagePrivate		*priv;
	gboolean					result;

	g_return_val_if_fail(XFDASHBOARD_IS_STAGE(inActor), CLUTTER_EVENT_PROPAGATE);

	self=XFDASHBOARD_STAGE(inActor);
	priv=self->priv;

	/* Do only intercept any event if a focus manager is available */
	if(!priv->focusManager) return(CLUTTER_EVENT_PROPAGATE);

	/* Do only intercept "key-press" and "key-release" events */
	if(clutter_event_type(inEvent)!=CLUTTER_KEY_PRESS &&
		clutter_event_type(inEvent)!=CLUTTER_KEY_RELEASE)
	{
		return(CLUTTER_EVENT_PROPAGATE);
	}

	/* Handle key release event */
	if(clutter_event_type(inEvent)==CLUTTER_KEY_RELEASE)
	{
		/* Handle key */
		switch(inEvent->key.keyval)
		{
			/* Handle ESC key to quit/suspend application */
			case CLUTTER_KEY_Escape:
			{
				xfdashboard_core_quit(NULL);
				return(CLUTTER_EVENT_STOP);
			}
			break;

			default:
				/* Fallthrough */
				break;
		}
	}

	/* Ask focus manager to handle this event */
	result=xfdashboard_focus_manager_handle_key_event(priv->focusManager, inEvent, NULL);
	if(result==CLUTTER_EVENT_STOP) return(result);

	return(CLUTTER_EVENT_STOP);
}

/* Get view to switch to by first looking upr temporary view ID set via command-line
 * and if not found or not set then looking up view ID configured via settings.
 */
static XfdashboardView* _xfdashboard_stage_get_view_to_switch_to(XfdashboardStage *self)
{
	XfdashboardStagePrivate				*priv;
	XfdashboardView						*view;

	g_return_val_if_fail(XFDASHBOARD_IS_STAGE(self), NULL);

	priv=self->priv;
	view=NULL;

	/* No viewpad (sidebar-only mode): no views to switch to */
	if(!priv->viewpad) return(NULL);

	/* First lookup view at private variable 'switchToView' which has higher
	 * priority as it is a temporary value and is usually set via command-line.
	 */
	if(priv->switchToView)
	{
		view=xfdashboard_viewpad_find_view_by_id(XFDASHBOARD_VIEWPAD(priv->viewpad), priv->switchToView);
		if(!view) g_warning("Will not switch to unknown view '%s'", priv->switchToView);

		/* Regardless if we could find view by its internal name or not
		 * reset variable because the switch should happen once only.
		 */
		g_free(priv->switchToView);
		priv->switchToView=NULL;

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfdashboardStageProperties[PROP_SWITCH_TO_VIEW]);
	}

	/* If we have not to switch to a specific view or if this view cannot be found
	 * then lookup the configured view in settings by its internal name
	 */
	if(!view)
	{
		const gchar						*resumeViewID;

		/* Get view ID from settings and look up view */
		resumeViewID=xfdashboard_settings_get_switch_to_view_on_resume(priv->settings);
		if(resumeViewID)
		{
			/* Lookup view by its ID set configured settings */
			view=xfdashboard_viewpad_find_view_by_id(XFDASHBOARD_VIEWPAD(priv->viewpad), resumeViewID);
			if(!view) g_warning("Cannot switch to unknown view '%s'", resumeViewID);
		}
	}

	/* Return view found */
	return(view);
}

/* Set focus in stage */
static void _xfdashboard_stage_set_focus(XfdashboardStage *self)
{
	XfdashboardStagePrivate		*priv;
	XfdashboardFocusable		*actor;

	g_return_if_fail(XFDASHBOARD_IS_STAGE(self));

	priv=self->priv;

	/* Set focus if no focus is set */
	actor=xfdashboard_focus_manager_get_focus(priv->focusManager);
	if(!actor)
	{
		XfdashboardFocusable	*focusable;

		/* Lookup first focusable actor */
		focusable=xfdashboard_focus_manager_get_next_focusable(priv->focusManager, NULL);
		if(focusable) xfdashboard_focus_manager_set_focus(priv->focusManager, focusable);
	}
}

/* Stage got signal to show a tooltip */
static void _xfdashboard_stage_show_tooltip(XfdashboardStage *self, ClutterAction *inAction)
{
	XfdashboardStagePrivate		*priv;
	XfdashboardTooltipAction	*tooltipAction;
	const gchar					*tooltipText;
	gfloat						tooltipX, tooltipY;
	gfloat						tooltipWidth, tooltipHeight;
	guint						cursorSize;
	gfloat						x, y;
	gfloat						stageWidth, stageHeight;

	g_return_if_fail(XFDASHBOARD_IS_STAGE(self));
	g_return_if_fail(XFDASHBOARD_IS_TOOLTIP_ACTION(inAction));
	g_return_if_fail(self->priv->tooltip);

	priv=self->priv;
	tooltipAction=XFDASHBOARD_TOOLTIP_ACTION(inAction);

	/* Hide tooltip while setup to avoid flicker */
	clutter_actor_hide(priv->tooltip);

	/* Get tooltip text and update text in tooltip actor */
	tooltipText=xfdashboard_tooltip_action_get_text(tooltipAction);
	xfdashboard_text_box_set_text(XFDASHBOARD_TEXT_BOX(priv->tooltip), tooltipText);

	/* Determine coordinates where to show tooltip at */
	xfdashboard_tooltip_action_get_position(tooltipAction, &tooltipX, &tooltipY);
	clutter_actor_get_size(priv->tooltip, &tooltipWidth, &tooltipHeight);

	cursorSize=gdk_display_get_default_cursor_size(gdk_display_get_default());

	clutter_actor_get_size(CLUTTER_ACTOR(self), &stageWidth, &stageHeight);

	x=tooltipX+cursorSize;
	y=tooltipY+cursorSize;
	if((x+tooltipWidth)>stageWidth) x=tooltipX-tooltipWidth;
	if((y+tooltipHeight)>stageHeight) y=tooltipY-tooltipHeight;

	clutter_actor_set_position(priv->tooltip, floor(x), floor(y));

	/* Show tooltip */
	clutter_actor_show(priv->tooltip);
}

/* Stage got signal to hide tooltip */
static void _xfdashboard_stage_hide_tooltip(XfdashboardStage *self, ClutterAction *inAction)
{
	XfdashboardStagePrivate		*priv;

	g_return_if_fail(XFDASHBOARD_IS_STAGE(self));

	priv=self->priv;

	/* Hide tooltip */
	clutter_actor_hide(priv->tooltip);
}

/* Notification timeout has been reached */
static void _xfdashboard_stage_on_notification_timeout_destroyed(gpointer inUserData)
{
	XfdashboardStage			*self;
	XfdashboardStagePrivate		*priv;

	g_return_if_fail(XFDASHBOARD_IS_STAGE(inUserData));

	self=XFDASHBOARD_STAGE(inUserData);
	priv=self->priv;

	/* Timeout source was destroy so just reset ID to 0 */
	priv->notificationTimeoutID=0;
}

static gboolean _xfdashboard_stage_on_notification_timeout(gpointer inUserData)
{
	XfdashboardStage			*self;
	XfdashboardStagePrivate		*priv;

	g_return_val_if_fail(XFDASHBOARD_IS_STAGE(inUserData), G_SOURCE_REMOVE);

	self=XFDASHBOARD_STAGE(inUserData);
	priv=self->priv;

	/* Timeout reached so hide notification */
	clutter_actor_hide(priv->notification);

	/* Tell main context to remove this source */
	return(G_SOURCE_REMOVE);
}

/* A window was closed
 * Check if stage window was closed then unset up window properties and reinstall
 * signal handler to find new stage window.
 */
static void _xfdashboard_stage_on_window_closed(XfdashboardStage *self,
												gpointer inUserData)
{
	XfdashboardStagePrivate				*priv;
	XfdashboardWindowTrackerWindow		*window;

	g_return_if_fail(XFDASHBOARD_IS_STAGE(self));
	g_return_if_fail(XFDASHBOARD_IS_WINDOW_TRACKER_WINDOW(inUserData));

	priv=self->priv;
	window=XFDASHBOARD_WINDOW_TRACKER_WINDOW(inUserData);

	/* Check if window closed is this stage window */
	if(priv->stageWindow!=window) return;

	/* Disconnect this signal handler as this stage window was closed*/
	XFDASHBOARD_DEBUG(self, ACTOR, "Stage window was closed. Removing signal handler");
	g_signal_handlers_disconnect_by_func(priv->stageWindow, G_CALLBACK(_xfdashboard_stage_on_window_closed), self);

	/* Forget stage window as it was closed */
	priv->stageWindow=NULL;

	/* Instead reconnect signal handler to find new stage window */
	XFDASHBOARD_DEBUG(self, ACTOR, "Reconnecting signal to find new stage window as this one as closed");
	g_signal_connect_swapped(priv->windowTracker, "window-opened", G_CALLBACK(_xfdashboard_stage_on_window_opened), self);

	/* Set focus */
	_xfdashboard_stage_set_focus(self);
}


/* A window was created
 * Check for stage window and set up window properties
 */
static void _xfdashboard_stage_on_window_opened(XfdashboardStage *self,
													XfdashboardWindowTrackerWindow *inWindow,
													gpointer inUserData)
{
	XfdashboardStagePrivate				*priv;
	XfdashboardWindowTrackerWindow		*stageWindow;

	g_return_if_fail(XFDASHBOARD_IS_STAGE(self));
	g_return_if_fail(XFDASHBOARD_IS_WINDOW_TRACKER_WINDOW(inWindow));

	priv=self->priv;

	/* Check if window opened is this stage window */
	stageWindow=xfdashboard_window_tracker_get_stage_window(priv->windowTracker, CLUTTER_STAGE(self));
	if(stageWindow!=inWindow) return;

	/* Set up window for use as stage window */
	priv->stageWindow=inWindow;
	xfdashboard_window_tracker_window_show_stage(priv->stageWindow);

	/* Disconnect this signal handler as this is a one-time setup of stage window */
	XFDASHBOARD_DEBUG(self, ACTOR, "Stage window was opened and set up. Removing signal handler");
	g_signal_handlers_disconnect_by_func(priv->windowTracker, G_CALLBACK(_xfdashboard_stage_on_window_opened), self);

	/* Instead connect signal handler to get notified when this stage window was
	 * destroyed as we need to forget this window and to reinstall this signal
	 * handler again.
	 */
	XFDASHBOARD_DEBUG(self, ACTOR, "Connecting signal signal handler to get notified about destruction of stage window");
	g_signal_connect_swapped(priv->stageWindow,
								"closed",
								G_CALLBACK(_xfdashboard_stage_on_window_closed),
								self);

	/* Set focus */
	_xfdashboard_stage_set_focus(self);
}

/* A window was created
 * Check if window opened is desktop background window
 */
static void _xfdashboard_stage_on_desktop_window_opened(XfdashboardStage *self,
														XfdashboardWindowTrackerWindow *inWindow,
														gpointer inUserData)
{
	XfdashboardStagePrivate				*priv;
	XfdashboardWindowTrackerWindow		*desktopWindow;
	ClutterContent						*windowContent;

	g_return_if_fail(XFDASHBOARD_IS_STAGE(self));
	g_return_if_fail(XFDASHBOARD_IS_WINDOW_TRACKER_WINDOW(inWindow));

	priv=self->priv;

	/* Get desktop background window and check if it is the new window opened */
	desktopWindow=xfdashboard_window_tracker_get_root_window(priv->windowTracker);
	if(desktopWindow)
	{
		windowContent=xfdashboard_window_tracker_window_get_content(desktopWindow);
		clutter_actor_set_content(priv->backgroundImageLayer, windowContent);
		clutter_actor_show(priv->backgroundImageLayer);
		g_object_unref(windowContent);

		g_signal_handlers_disconnect_by_func(priv->windowTracker, G_CALLBACK(_xfdashboard_stage_on_desktop_window_opened), self);
		XFDASHBOARD_DEBUG(self, ACTOR, "Found desktop window with signal 'window-opened', so disconnecting signal handler");
	}
}

/* The active workspace changed: remember when, so focus changes caused
 * by workspace switches (e.g. scrolling over the strip) do not hide the
 * stage as "clicked outside".
 */
static void _xfdashboard_stage_on_active_workspace_changed(XfdashboardStage *self,
															XfdashboardWindowTrackerWorkspace *inOldWorkspace,
															XfdashboardWindowTrackerWorkspace *inNewWorkspace,
															gpointer inUserData)
{
	XfdashboardStagePrivate				*priv;

	g_return_if_fail(XFDASHBOARD_IS_STAGE(self));

	priv=self->priv;
	priv->lastWorkspaceSwitchUsec=g_get_monotonic_time();
}

/* The active window changed: hide when focus leaves the stage, i.e. the
 * user clicked outside. Only reacts when the stage itself was active to
 * avoid hiding right after resume before the stage took focus, and does
 * nothing while suspended.
 */
static void _xfdashboard_stage_on_active_window_changed(XfdashboardStage *self,
															XfdashboardWindowTrackerWindow *inOldWindow,
															XfdashboardWindowTrackerWindow *inNewWindow,
															gpointer inUserData)
{
	XfdashboardStagePrivate				*priv;
	XfdashboardCore						*core;

	g_return_if_fail(XFDASHBOARD_IS_STAGE(self));

	priv=self->priv;

	/* Only hide when focus actually left the stage window */
	if(!priv->stageWindow) return;
	if(inOldWindow!=priv->stageWindow) return;
	if(inNewWindow==priv->stageWindow) return;

	/* Do nothing while suspended (already hidden) */
	core=xfdashboard_core_get_default();
	if(!core || xfdashboard_core_is_suspended(core)) return;

	/* Ignore focus changes right after a workspace switch: scrolling
	 * over the strip navigates workspaces and the window manager may
	 * move focus (or clear it on empty workspaces) as a result. That
	 * is navigation, not "clicked outside".
	 */
	if(priv->lastWorkspaceSwitchUsec!=0 &&
		(g_get_monotonic_time()-priv->lastWorkspaceSwitchUsec)<G_USEC_PER_SEC)
	{
		return;
	}

	XFDASHBOARD_DEBUG(self, ACTOR, "Active window left stage window, hiding");
	xfdashboard_core_quit(NULL);
}

/* The application will be suspended */
static void _xfdashboard_stage_on_application_suspend(XfdashboardStage *self, gpointer inUserData)
{
	XfdashboardStagePrivate				*priv;

	g_return_if_fail(XFDASHBOARD_IS_STAGE(self));
	g_return_if_fail(XFDASHBOARD_IS_CORE(inUserData));

	priv=self->priv;

	/* Instead of hiding stage actor just hide stage's window. It should be safe
	 * to just hide the window as it should be listed on any task list and is not
	 * selectable by user. The advantage should be that the window is already mapped
	 * and its state is already set up like fullscreen, sticky and so on. This
	 * prevents that window will not be shown in fullscreen again (just maximized
	 * and flickers) if we use clutter_actor_show to show stage actor and its window
	 * again. But we can only do this if the window is known and set up ;)
	 */
	if(priv->stageWindow)
	{
		xfdashboard_window_tracker_window_hide_stage(priv->stageWindow);
	}

	/* Hide tooltip */
	if(priv->tooltip) clutter_actor_hide(priv->tooltip);
}

/* The application will be resumed */
static void _xfdashboard_stage_on_application_resume(XfdashboardStage *self, gpointer inUserData)
{
	XfdashboardStagePrivate				*priv;

	g_return_if_fail(XFDASHBOARD_IS_STAGE(self));
	g_return_if_fail(XFDASHBOARD_IS_CORE(inUserData));

	priv=self->priv;

	/* If stage window is known just show it again ... */
	if(priv->stageWindow)
	{
		XfdashboardView					*resumeView;

		/* Find view to switch to if requested */
		resumeView=_xfdashboard_stage_get_view_to_switch_to(self);

		/* Switch to view if requested (no-op in sidebar-only mode) */
		if(resumeView && priv->viewpad)
		{
			xfdashboard_viewpad_set_active_view(XFDASHBOARD_VIEWPAD(priv->viewpad), resumeView);
		}

		/* Now move focus to actor if user requested to refocus preselected actor
		 * as specified by theme.
		 */
		if(priv->focusActorOnShow)
		{
			gboolean				reselectFocusOnResume;

			/* Determine if user (also) requests to reselect focus on resume */
			reselectFocusOnResume=xfdashboard_settings_get_reselect_theme_focus_on_resume(priv->settings);
			if(reselectFocusOnResume)
			{
				/* Move focus to actor */
				xfdashboard_focus_manager_set_focus(priv->focusManager, XFDASHBOARD_FOCUSABLE(priv->focusActorOnShow));

				XFDASHBOARD_DEBUG(self, ACTOR,
									"Moved focus to actor %s because it should be reselected on resume",
									G_OBJECT_TYPE_NAME(priv->focusActorOnShow));
			}
				else
				{
					/* Forget actor to focus now the user did not requested to reselect
					 * this focus again and again when stage window is shown ;)
					 */
					g_object_remove_weak_pointer(G_OBJECT(priv->focusActorOnShow), &priv->focusActorOnShow);
					priv->focusActorOnShow=NULL;
				}
		}

		/* Set up stage and show it */
		xfdashboard_window_tracker_window_show_stage(priv->stageWindow);
	}
		/* ... otherwise set it up by calling clutter_actor_show() etc. */
		else
		{
			/* Show stage and force window creation. It will also handle
			 * the switch to a specific view.
			 */
			clutter_actor_show(CLUTTER_ACTOR(self));
		}

	/* In any case force a redraw */
	clutter_actor_queue_redraw(CLUTTER_ACTOR(self));
}

/* Theme in application has changed */
static void _xfdashboard_stage_theme_interface_data_free(XfdashboardStageThemeInterfaceData *inData)
{
	g_return_if_fail(inData);

	/* Release each data in data structure but do not unref the interface actor
	 * as it might be used at stage. The stage is responsible to destroy the
	 * interface actor in *any* case.
	 */
	if(inData->focusables) g_ptr_array_unref(inData->focusables);
	if(inData->focus) g_object_unref(inData->focus);

	/* Release allocated memory */
	g_free(inData);
}

static XfdashboardStageThemeInterfaceData* _xfdashboard_stage_theme_interface_data_new(void)
{
	XfdashboardStageThemeInterfaceData   *data;

	/* Allocate memory for data structure */
	data=g_new0(XfdashboardStageThemeInterfaceData, 1);
	if(!data) return(NULL);

	/* Return newly create and initialized data structure */
	return(data);
}

/* Wire primary children (viewpad, workspace-selector, notification, tooltip) */
static void _wire_primary_children(XfdashboardStage *self, XfdashboardStageThemeInterfaceData *interface)
{
	XfdashboardStagePrivate		*priv;
	ClutterActor					*child;

	g_return_if_fail(XFDASHBOARD_IS_STAGE(self));
	g_return_if_fail(interface!=NULL);

	priv=self->priv;

	/* Get children from built stage and set up pointer variables */
	priv->viewpad=NULL;
	child=xfdashboard_find_actor_by_name(CLUTTER_ACTOR(self), "viewpad");
	if(child && XFDASHBOARD_IS_VIEWPAD(child))
	{
		priv->viewpad=child;
		g_object_add_weak_pointer(G_OBJECT(priv->viewpad), &priv->viewpad);

		/* Register this focusable actor if it is focusable */
		if(!interface->focusables && XFDASHBOARD_IS_FOCUSABLE(priv->viewpad))
		{
			xfdashboard_focus_manager_register(priv->focusManager,
												XFDASHBOARD_FOCUSABLE(priv->viewpad));

			/* Check if viewpad can be focused to enforce all focusable views
			 * will be registered too. We need to do it now to get all focusable
			 * views registered before first use of any function of focus manager.
			 */
			xfdashboard_focusable_can_focus(XFDASHBOARD_FOCUSABLE(priv->viewpad));
		}
	}

	priv->workspaces=NULL;
	child=xfdashboard_find_actor_by_name(CLUTTER_ACTOR(self), "workspace-selector");
	if(child && XFDASHBOARD_IS_WORKSPACE_SELECTOR(child))
	{
		priv->workspaces=child;
		g_object_add_weak_pointer(G_OBJECT(priv->workspaces), &priv->workspaces);

		/* Register this focusable actor if it is focusable */
		if(!interface->focusables && XFDASHBOARD_IS_FOCUSABLE(priv->workspaces))
		{
			xfdashboard_focus_manager_register(priv->focusManager,
												XFDASHBOARD_FOCUSABLE(priv->workspaces));
		}
	}

	priv->notification=NULL;
	child=xfdashboard_find_actor_by_name(CLUTTER_ACTOR(self), "notification");
	if(child && XFDASHBOARD_IS_TEXT_BOX(child))
	{
		priv->notification=child;
		g_object_add_weak_pointer(G_OBJECT(priv->notification), &priv->notification);

		/* Register this focusable actor if it is focusable */
		if(!interface->focusables && XFDASHBOARD_IS_FOCUSABLE(priv->notification))
		{
			xfdashboard_focus_manager_register(priv->focusManager,
												XFDASHBOARD_FOCUSABLE(priv->notification));
		}

		/* Hide notification by default */
		clutter_actor_hide(priv->notification);
		clutter_actor_set_reactive(priv->notification, FALSE);
	}

	priv->tooltip=NULL;
	child=xfdashboard_find_actor_by_name(CLUTTER_ACTOR(self), "tooltip");
	if(child && XFDASHBOARD_IS_TEXT_BOX(child))
	{
		priv->tooltip=child;
		g_object_add_weak_pointer(G_OBJECT(priv->tooltip), &priv->tooltip);

		/* Register this focusable actor if it is focusable */
		if(!interface->focusables && XFDASHBOARD_IS_FOCUSABLE(priv->tooltip))
		{
			xfdashboard_focus_manager_register(priv->focusManager,
												XFDASHBOARD_FOCUSABLE(priv->tooltip));
		}

		/* Hide tooltip by default */
		clutter_actor_hide(priv->tooltip);
		clutter_actor_set_reactive(priv->tooltip, FALSE);
	}
}

/* Register interface focusables at focus manager */
static void _register_interface_focusables(XfdashboardStage *self, XfdashboardStageThemeInterfaceData *interface)
{
	XfdashboardStagePrivate		*priv;
	GObject						*focusObject;
	guint						i;

	g_return_if_fail(XFDASHBOARD_IS_STAGE(self));
	g_return_if_fail(interface!=NULL);

	priv=self->priv;

	/* Register focusable actors at focus manager */
	if(interface->focusables)
	{
		for(i=0; i<interface->focusables->len; i++)
		{
			/* Get actor to register at focus manager */
			focusObject=G_OBJECT(g_ptr_array_index(interface->focusables, i));
			if(!focusObject) continue;

			/* Check that actor is focusable */
			if(!XFDASHBOARD_IS_FOCUSABLE(focusObject))
			{
				g_warning("Object %s is not focusable and cannot be registered.",
							G_OBJECT_TYPE_NAME(focusObject));
				continue;
			}

			/* Register actor at focus manager */
			xfdashboard_focus_manager_register(priv->focusManager,
												XFDASHBOARD_FOCUSABLE(focusObject));
			XFDASHBOARD_DEBUG(self, ACTOR,
								"Registering actor %s of interface with ID '%s' at focus manager",
								G_OBJECT_TYPE_NAME(focusObject),
								clutter_actor_get_name(interface->actor));
		}
	}
}

static void _xfdashboard_stage_on_application_theme_changed(XfdashboardStage *self,
															XfdashboardTheme *inTheme,
															gpointer inUserData)
{
	XfdashboardStagePrivate				*priv;
	XfdashboardThemeLayout				*themeLayout;
	GList								*interfaces;
	XfdashboardStageThemeInterfaceData	*interface;
	GList								*iter;
	GList								*monitors;
	XfdashboardWindowTrackerMonitor		*monitor;
	ClutterActorIter					childIter;
	ClutterActor						*child;
	GObject								*focusObject;
	guint								i;
	gboolean							reselectFocusOnResume;

	g_return_if_fail(XFDASHBOARD_IS_STAGE(self));
	g_return_if_fail(XFDASHBOARD_IS_THEME(inTheme));
	g_return_if_fail(XFDASHBOARD_IS_CORE(inUserData));

	priv=self->priv;

	/* Get theme layout */
	themeLayout=xfdashboard_theme_get_layout(inTheme);

	/* Create interface for each monitor if multiple monitors are supported */
	interfaces=NULL;
	if(xfdashboard_window_tracker_supports_multiple_monitors(priv->windowTracker))
	{
		monitors=xfdashboard_window_tracker_get_monitors(priv->windowTracker);
		for(iter=monitors; iter; iter=g_list_next(iter))
		{
			/* Get monitor */
			monitor=XFDASHBOARD_WINDOW_TRACKER_MONITOR(iter->data);

			/* Get interface  */
			if(xfdashboard_window_tracker_monitor_is_primary(monitor))
			{
				/* Get interface for primary monitor */
				interface=_xfdashboard_stage_theme_interface_data_new();
				interface->actor=xfdashboard_theme_layout_build_interface(themeLayout,
																			XFDASHBOARD_THEME_LAYOUT_PRIMARY,
																			XFDASHBOARD_THEME_LAYOUT_BUILD_GET_FOCUSABLES, &interface->focusables,
																			XFDASHBOARD_THEME_LAYOUT_BUILD_GET_SELECTED_FOCUS, &interface->focus,
																			-1);
				if(!interface->actor)
				{
					g_critical("Could not build interface '%s' from theme '%s'",
								XFDASHBOARD_THEME_LAYOUT_PRIMARY,
								xfdashboard_theme_get_theme_name(inTheme));

					/* Release allocated resources */
					_xfdashboard_stage_theme_interface_data_free(interface);
					g_list_free_full(interfaces, (GDestroyNotify)_xfdashboard_stage_theme_interface_data_free);

					return;
				}

				if(!XFDASHBOARD_IS_STAGE_INTERFACE(interface->actor))
				{
					g_critical("Interface '%s' from theme '%s' must be an actor of type %s",
								XFDASHBOARD_THEME_LAYOUT_PRIMARY,
								xfdashboard_theme_get_theme_name(inTheme),
								g_type_name(XFDASHBOARD_TYPE_STAGE_INTERFACE));

					/* Release allocated resources */
					_xfdashboard_stage_theme_interface_data_free(interface);
					g_list_free_full(interfaces, (GDestroyNotify)_xfdashboard_stage_theme_interface_data_free);

					return;
				}
			}
				else
				{
					/* Get interface for non-primary monitors. If no interface
					 * is defined in theme then create an empty interface.
					 */
					interface=_xfdashboard_stage_theme_interface_data_new();
					interface->actor=xfdashboard_theme_layout_build_interface(themeLayout,
																				XFDASHBOARD_THEME_LAYOUT_SECONDARY,
																				XFDASHBOARD_THEME_LAYOUT_BUILD_GET_FOCUSABLES, &interface->focusables,
																				XFDASHBOARD_THEME_LAYOUT_BUILD_GET_SELECTED_FOCUS, &interface->focus,
																				-1);
					if(!interface->actor)
					{
						interface->actor=xfdashboard_stage_interface_new();
					}

					if(!XFDASHBOARD_IS_STAGE_INTERFACE(interface->actor))
					{
						g_critical("Interface '%s' from theme '%s' must be an actor of type %s",
									XFDASHBOARD_THEME_LAYOUT_SECONDARY,
									xfdashboard_theme_get_theme_name(inTheme),
									g_type_name(XFDASHBOARD_TYPE_STAGE_INTERFACE));

						/* Release allocated resources */
						_xfdashboard_stage_theme_interface_data_free(interface);
						g_list_free_full(interfaces, (GDestroyNotify)_xfdashboard_stage_theme_interface_data_free);

						return;
					}
				}

			/* Set monitor at interface */
			xfdashboard_stage_interface_set_monitor(XFDASHBOARD_STAGE_INTERFACE(interface->actor), monitor);

			/* Add interface to list of interfaces */
			interfaces=g_list_prepend(interfaces, interface);
		}
	}
		/* Otherwise create only a primary stage interface and set no monitor
		 * because no one is available if multiple monitors are not supported.
		 */
		else
		{
			/* Get interface for primary monitor */
			interface=_xfdashboard_stage_theme_interface_data_new();
			interface->actor=xfdashboard_theme_layout_build_interface(themeLayout,
																		XFDASHBOARD_THEME_LAYOUT_PRIMARY,
																		XFDASHBOARD_THEME_LAYOUT_BUILD_GET_FOCUSABLES, &interface->focusables,
																		XFDASHBOARD_THEME_LAYOUT_BUILD_GET_SELECTED_FOCUS, &interface->focus,
																		-1);
			if(!interface->actor)
			{
				g_critical("Could not build interface '%s' from theme '%s'",
							XFDASHBOARD_THEME_LAYOUT_PRIMARY,
							xfdashboard_theme_get_theme_name(inTheme));

				/* Release allocated resources */
				_xfdashboard_stage_theme_interface_data_free(interface);
				g_list_free_full(interfaces, (GDestroyNotify)_xfdashboard_stage_theme_interface_data_free);

				return;
			}

			if(!XFDASHBOARD_IS_STAGE_INTERFACE(interface->actor))
			{
				g_critical("Interface '%s' from theme '%s' must be an actor of type %s",
							XFDASHBOARD_THEME_LAYOUT_PRIMARY,
							xfdashboard_theme_get_theme_name(inTheme),
							g_type_name(XFDASHBOARD_TYPE_STAGE_INTERFACE));

				/* Release allocated resources */
				_xfdashboard_stage_theme_interface_data_free(interface);
				g_list_free_full(interfaces, (GDestroyNotify)_xfdashboard_stage_theme_interface_data_free);

				return;
			}

			/* Add interface to list of interfaces */
			interfaces=g_list_prepend(interfaces, interface);

			XFDASHBOARD_DEBUG(self, ACTOR, "Creating primary interface only because of no support for multiple monitors");
		}

	/* Destroy all interfaces from stage.
	 * There is no need to reset pointer variables
	 * because they should be set NULL to by calling _xfdashboard_stage_reset_reference_on_destroy
	 * when stage actor was destroyed.
	 */
	clutter_actor_iter_init(&childIter, CLUTTER_ACTOR(self));
	while(clutter_actor_iter_next(&childIter, &child))
	{
		if(XFDASHBOARD_IS_STAGE_INTERFACE(child))
		{
			clutter_actor_iter_destroy(&childIter);
		}
	}

	/* Add all new interfaces to stage */
	for(iter=interfaces; iter; iter=g_list_next(iter))
	{
		/* Get interface to add to stage */
		interface=(XfdashboardStageThemeInterfaceData*)(iter->data);
		if(!interface) continue;

		/* Check for interface actor to add to stage */
		if(!interface->actor) continue;

		/* Add interface to stage */
		clutter_actor_add_child(CLUTTER_ACTOR(self), interface->actor);

		/* Only check children and set up pointer variables
		 * for primary monitor.
		 */
		monitor=xfdashboard_stage_interface_get_monitor(XFDASHBOARD_STAGE_INTERFACE(interface->actor));
		if(!monitor || xfdashboard_window_tracker_monitor_is_primary(monitor))
		{
			/* Remember primary interface */
			if(!priv->primaryInterface)
			{
				priv->primaryInterface=interface->actor;
				g_object_add_weak_pointer(G_OBJECT(priv->primaryInterface), &priv->primaryInterface);
			}
				else g_critical("Invalid multiple stages for primary monitor");

			/* Get children from built stage and set up pointer variables */
			_wire_primary_children(self, interface);
		}

		/* Register focusable actors at focus manager */
		_register_interface_focusables(self, interface);

		/* Move focus to selected actor or remember actor focus to set it later
		 * but only if selected actor is a focusable actor and is registered
		 * to focus manager.
		 */
		if(interface->focus &&
			XFDASHBOARD_IS_FOCUSABLE(interface->focus) &&
			xfdashboard_focus_manager_is_registered(priv->focusManager, XFDASHBOARD_FOCUSABLE(interface->focus)))
		{
			/* If actor can be focused then move focus to actor ... */
			if(xfdashboard_focusable_can_focus(XFDASHBOARD_FOCUSABLE(interface->focus)))
			{
				xfdashboard_focus_manager_set_focus(priv->focusManager, XFDASHBOARD_FOCUSABLE(interface->focus));
				XFDASHBOARD_DEBUG(self, ACTOR,
									"Moved focus to actor %s of interface with ID '%s'",
									G_OBJECT_TYPE_NAME(interface->focus),
									clutter_actor_get_name(interface->actor));

				/* Determine if user (also) requests to reselect focus on resume
				 * because then remember the actor to focus, to move the focus
				 * each time the stage window gets shown after it was hidden.
				 */
				reselectFocusOnResume=xfdashboard_settings_get_reselect_theme_focus_on_resume(priv->settings);
				if(reselectFocusOnResume)
				{
					priv->focusActorOnShow=XFDASHBOARD_FOCUSABLE(interface->focus);
					g_object_add_weak_pointer(G_OBJECT(priv->focusActorOnShow), &priv->focusActorOnShow);

					XFDASHBOARD_DEBUG(self, ACTOR,
										"Will move focus to actor %s of interface with ID '%s' any time the stage gets visible",
										G_OBJECT_TYPE_NAME(interface->focus),
										clutter_actor_get_name(interface->actor));
				}
			}
				/* ... otherwise if stage is not visible, remember the actor
				 * to focus to move the focus to it as soon as stage is
				 * visible ...
				 */
				else if(!clutter_actor_is_visible(CLUTTER_ACTOR(self)))
				{
					priv->focusActorOnShow=XFDASHBOARD_FOCUSABLE(interface->focus);
					g_object_add_weak_pointer(G_OBJECT(priv->focusActorOnShow), &priv->focusActorOnShow);

					XFDASHBOARD_DEBUG(self, ACTOR,
										"Cannot move focus to actor %s of interface with ID '%s' but will try again when stage is visible",
										G_OBJECT_TYPE_NAME(interface->focus),
										clutter_actor_get_name(interface->actor));
				}
				/* ... otherwise just show a debug message */
				else
				{
					XFDASHBOARD_DEBUG(self, ACTOR,
										"Cannot move focus to actor %s of interface with ID '%s' because actor cannot be focused",
										G_OBJECT_TYPE_NAME(interface->focus),
										clutter_actor_get_name(interface->actor));
				}
		}
			else
			{
				XFDASHBOARD_DEBUG(self, ACTOR, "Cannot move focus to any actor because no one was selected in theme");
			}
	}

	/* Release allocated resources */
	g_list_free_full(interfaces, (GDestroyNotify)_xfdashboard_stage_theme_interface_data_free);

	/* Set focus */
	_xfdashboard_stage_set_focus(self);
}

/* Primary monitor changed */
static void _xfdashboard_stage_on_primary_monitor_changed(XfdashboardStage *self,
															XfdashboardWindowTrackerMonitor *inOldMonitor,
															XfdashboardWindowTrackerMonitor *inNewMonitor,
															gpointer inUserData)
{
	XfdashboardStagePrivate					*priv;
	XfdashboardStageInterface				*oldStageInterface;
	XfdashboardWindowTrackerMonitor			*oldPrimaryStageInterfaceMonitor;
	ClutterActorIter						childIter;
	ClutterActor							*child;

	g_return_if_fail(XFDASHBOARD_IS_STAGE(self));
	g_return_if_fail(!inOldMonitor || XFDASHBOARD_IS_WINDOW_TRACKER_MONITOR(inOldMonitor));
	g_return_if_fail(XFDASHBOARD_IS_WINDOW_TRACKER_MONITOR(inNewMonitor));
	g_return_if_fail(XFDASHBOARD_IS_WINDOW_TRACKER(inUserData));

	priv=self->priv;

	/* If we do not have a primary stage interface yet do nothing */
	if(!priv->primaryInterface) return;

	/* If primary stage interface has already new monitor set do nothing */
	oldPrimaryStageInterfaceMonitor=xfdashboard_stage_interface_get_monitor(XFDASHBOARD_STAGE_INTERFACE(priv->primaryInterface));
	if(oldPrimaryStageInterfaceMonitor==inNewMonitor) return;

	/* Find stage interface currently using the new primary monitor */
	oldStageInterface=NULL;

	clutter_actor_iter_init(&childIter, CLUTTER_ACTOR(self));
	while(!oldStageInterface && clutter_actor_iter_next(&childIter, &child))
	{
		XfdashboardStageInterface			*interface;

		/* Check for stage interface */
		if(!XFDASHBOARD_IS_STAGE_INTERFACE(child)) continue;

		/* Get stage interface */
		interface=XFDASHBOARD_STAGE_INTERFACE(child);

		/* Check if stage interface is using new primary monitor then remember it */
		if(xfdashboard_stage_interface_get_monitor(interface)==inNewMonitor)
		{
			oldStageInterface=interface;
		}
	}

	/* Set old primary monitor at stage interface which is using new primary monitor */
	if(oldStageInterface)
	{
		/* Set old monitor at found stage interface */
		xfdashboard_stage_interface_set_monitor(oldStageInterface, oldPrimaryStageInterfaceMonitor);
	}

	/* Set new primary monitor at primary stage interface */
	xfdashboard_stage_interface_set_monitor(XFDASHBOARD_STAGE_INTERFACE(priv->primaryInterface), inNewMonitor);
	XFDASHBOARD_DEBUG(self, ACTOR,
						"Primary monitor changed from %d to %d",
						xfdashboard_window_tracker_monitor_get_number(oldPrimaryStageInterfaceMonitor),
						xfdashboard_window_tracker_monitor_get_number(inNewMonitor));
}

/* A monitor was added */
static void _xfdashboard_stage_on_monitor_added(XfdashboardStage *self,
												XfdashboardWindowTrackerMonitor *inMonitor,
												gpointer inUserData)
{
	ClutterActor							*interface;
	XfdashboardTheme						*theme;
	XfdashboardThemeLayout					*themeLayout;

	g_return_if_fail(XFDASHBOARD_IS_STAGE(self));
	g_return_if_fail(XFDASHBOARD_IS_WINDOW_TRACKER_MONITOR(inMonitor));
	g_return_if_fail(XFDASHBOARD_IS_WINDOW_TRACKER(inUserData));

	/* Get theme and theme layout */
	theme=xfdashboard_core_get_theme(NULL);
	themeLayout=xfdashboard_theme_get_layout(theme);

	/* Create interface for non-primary monitors. If no interface is defined in theme
	 * then create an empty interface.
	 */
	interface=xfdashboard_theme_layout_build_interface(themeLayout, XFDASHBOARD_THEME_LAYOUT_SECONDARY);
	if(!interface)
	{
		interface=xfdashboard_stage_interface_new();
	}

	if(!XFDASHBOARD_IS_STAGE_INTERFACE(interface))
	{
		g_critical("Interface '%s' from theme '%s' must be an actor of type %s",
					XFDASHBOARD_THEME_LAYOUT_SECONDARY,
					xfdashboard_theme_get_theme_name(theme),
					g_type_name(XFDASHBOARD_TYPE_STAGE_INTERFACE));
		return;
	}

	/* Set monitor at interface */
	xfdashboard_stage_interface_set_monitor(XFDASHBOARD_STAGE_INTERFACE(interface), inMonitor);

	/* Add interface to stage */
	clutter_actor_add_child(CLUTTER_ACTOR(self), interface);
	XFDASHBOARD_DEBUG(self, ACTOR,
						"Added stage interface for new monitor %d",
						xfdashboard_window_tracker_monitor_get_number(inMonitor));

	/* If monitor added is the primary monitor then swap now the stage interfaces */
	if(xfdashboard_window_tracker_monitor_is_primary(inMonitor))
	{
		_xfdashboard_stage_on_primary_monitor_changed(self, NULL, inMonitor, inUserData);
	}
}

/* A monitor was removed */
static void _xfdashboard_stage_on_monitor_removed(XfdashboardStage *self,
													XfdashboardWindowTrackerMonitor *inMonitor,
													gpointer inUserData)
{
	XfdashboardStagePrivate					*priv;
	ClutterActorIter						childIter;
	ClutterActor							*child;

	g_return_if_fail(XFDASHBOARD_IS_STAGE(self));
	g_return_if_fail(XFDASHBOARD_IS_WINDOW_TRACKER_MONITOR(inMonitor));
	g_return_if_fail(XFDASHBOARD_IS_WINDOW_TRACKER(inUserData));

	priv=self->priv;

	/* If monitor removed is the primary monitor swap primary interface with first
	 * stage interface to keep it alive. We should afterward receive a signal that
	 * primary monitor has changed, then the primary interface will be set to its
	 * right place.
	 */
	if(xfdashboard_window_tracker_monitor_is_primary(inMonitor))
	{
		XfdashboardWindowTrackerMonitor*	firstMonitor;

		/* Get first monitor */
		firstMonitor=xfdashboard_window_tracker_get_monitor_by_number(priv->windowTracker, 0);

		/* Swp stage interfaces */
		_xfdashboard_stage_on_primary_monitor_changed(self, inMonitor, firstMonitor, inUserData);
	}

	/* Look up stage interface for removed monitor and destroy it */
	clutter_actor_iter_init(&childIter, CLUTTER_ACTOR(self));
	while(clutter_actor_iter_next(&childIter, &child))
	{
		XfdashboardStageInterface			*interface;

		/* Only check stage interfaces */
		if(!XFDASHBOARD_IS_STAGE_INTERFACE(child)) continue;

		/* If stage interface is the one for this monitor then destroy it */
		interface=XFDASHBOARD_STAGE_INTERFACE(child);
		if(xfdashboard_stage_interface_get_monitor(interface)==inMonitor)
		{
			clutter_actor_iter_destroy(&childIter);
			XFDASHBOARD_DEBUG(self, ACTOR,
								"Removed stage interface for removed monitor %d",
								xfdashboard_window_tracker_monitor_get_number(inMonitor));
		}
	}
}

/* Screen size has changed */
static void _xfdashboard_stage_on_screen_size_changed(XfdashboardStage *self,
														gpointer inUserData)
{
	XfdashboardWindowTracker	*windowTracker;
	gint						screenWidth, screenHeight;
	gfloat						stageWidth, stageHeight;

	g_return_if_fail(XFDASHBOARD_IS_STAGE(self));
	g_return_if_fail(XFDASHBOARD_IS_WINDOW_TRACKER(inUserData));

	windowTracker=XFDASHBOARD_WINDOW_TRACKER(inUserData);

	/* Get screen size */
	xfdashboard_window_tracker_get_screen_size(windowTracker, &screenWidth, &screenHeight);

	/* Top-strip mode: stage is as wide as the screen but only as tall as
	 * the workspace selector strip (~22% of screen height). Never force
	 * fullscreen height.
	 */
	{
		gint stripHeight=(gint)(screenHeight*0.22f);
		if(stripHeight<1) stripHeight=1;

		clutter_actor_get_size(CLUTTER_ACTOR(self), &stageWidth, &stageHeight);

		if((gint)stageWidth!=screenWidth ||
			(gint)stageHeight!=stripHeight)
		{
			XFDASHBOARD_DEBUG(self, ACTOR,
								"Screen is %dx%d, resizing top-strip stage to %dx%d",
								screenWidth, screenHeight,
								screenWidth, stripHeight);

			clutter_actor_set_size(CLUTTER_ACTOR(self), screenWidth, stripHeight);
		}
		return;
	}
}

/* IMPLEMENTATION: ClutterActor */

/* The stage actor should be shown */
static void _xfdashboard_stage_show(ClutterActor *inActor)
{
	XfdashboardStage			*self;
	XfdashboardStagePrivate		*priv;
	XfdashboardView				*switchView;

	g_return_if_fail(XFDASHBOARD_IS_STAGE(inActor));

	self=XFDASHBOARD_STAGE(inActor);
	priv=self->priv;

	/* Find view to switch to if requested and switch to this view */
	switchView=_xfdashboard_stage_get_view_to_switch_to(self);
	if(switchView && priv->viewpad)
	{
		xfdashboard_viewpad_set_active_view(XFDASHBOARD_VIEWPAD(priv->viewpad), switchView);
	}

	/* Set stage to non-fullscreen top strip (may be a newly created window) */
	clutter_stage_set_fullscreen(CLUTTER_STAGE(self), FALSE);

	/* If we do not know the stage window connect signal to find it */
	if(!priv->stageWindow)
	{
		/* Connect signals */
		XFDASHBOARD_DEBUG(self, ACTOR, "Connecting signal to find stage window");
		g_signal_connect_swapped(priv->windowTracker, "window-opened", G_CALLBACK(_xfdashboard_stage_on_window_opened), self);
	}

	/* Call parent's show method */
	if(CLUTTER_ACTOR_CLASS(xfdashboard_stage_parent_class)->show)
	{
		CLUTTER_ACTOR_CLASS(xfdashboard_stage_parent_class)->show(inActor);
	}

	/* Now move focus to actor is one was remembered when theme was loaded */
	if(priv->focusActorOnShow)
	{
		gboolean				reselectFocusOnResume;

		/* Determine if user (also) requests to reselect focus on resume */
		reselectFocusOnResume=xfdashboard_settings_get_reselect_theme_focus_on_resume(priv->settings);

		/* Move focus to actor */
		xfdashboard_focus_manager_set_focus(priv->focusManager, XFDASHBOARD_FOCUSABLE(priv->focusActorOnShow));

		XFDASHBOARD_DEBUG(self, ACTOR,
							"Moved focus to actor %s %s",
							G_OBJECT_TYPE_NAME(priv->focusActorOnShow),
							!reselectFocusOnResume ? "now as it was delayed to when stage is visible" : "because it should be reselected on resume");

		/* Forget actor to focus now if user did not requested to reselect
		 * this focus again and again when stage window is shown ;)
		 */
		if(!reselectFocusOnResume)
		{
			g_object_remove_weak_pointer(G_OBJECT(priv->focusActorOnShow), &priv->focusActorOnShow);
			priv->focusActorOnShow=NULL;
		}
	}
}

/* IMPLEMENTATION: GObject */

/* Dispose this object */
static void _xfdashboard_stage_dispose(GObject *inObject)
{
	XfdashboardStage			*self=XFDASHBOARD_STAGE(inObject);
	XfdashboardStagePrivate		*priv=self->priv;

	/* Release allocated resources */
	if(priv->stageWindow)
	{
		g_signal_handlers_disconnect_by_func(priv->stageWindow, G_CALLBACK(_xfdashboard_stage_on_window_closed), self);
		xfdashboard_window_tracker_window_hide_stage(priv->stageWindow);
		priv->stageWindow=NULL;
	}

	if(priv->focusManager)
	{
		g_object_unref(priv->focusManager);
		priv->focusManager=NULL;
	}

	if(priv->notificationTimeoutID)
	{
		g_source_remove(priv->notificationTimeoutID);
		priv->notificationTimeoutID=0;
	}

	if(priv->windowTracker)
	{
		g_signal_handlers_disconnect_by_data(priv->windowTracker, self);
		g_object_unref(priv->windowTracker);
		priv->windowTracker=NULL;
	}

	if(priv->backgroundColor)
	{
		clutter_color_free(priv->backgroundColor);
		priv->backgroundColor=NULL;
	}

	if(priv->notification)
	{
		clutter_actor_destroy(CLUTTER_ACTOR(priv->notification));
		priv->notification=NULL;
	}

	if(priv->tooltip)
	{
		clutter_actor_destroy(CLUTTER_ACTOR(priv->tooltip));
		priv->tooltip=NULL;
	}

	if(priv->workspaces)
	{
		clutter_actor_destroy(priv->workspaces);
		priv->workspaces=NULL;
	}

	if(priv->viewpad)
	{
		clutter_actor_destroy(priv->viewpad);
		priv->viewpad=NULL;
	}

	if(priv->primaryInterface)
	{
		clutter_actor_destroy(priv->primaryInterface);
		priv->primaryInterface=NULL;
	}

	if(priv->backgroundImageLayer)
	{
		clutter_actor_destroy(priv->backgroundImageLayer);
		priv->backgroundImageLayer=NULL;
	}

	if(priv->backgroundColorLayer)
	{
		clutter_actor_destroy(priv->backgroundColorLayer);
		priv->backgroundColorLayer=NULL;
	}

	if(priv->switchToView)
	{
		g_free(priv->switchToView);
		priv->switchToView=NULL;
	}

	if(priv->settings)
	{
		g_object_unref(priv->settings);
		priv->settings=NULL;
	}

	/* Call parent's class dispose method */
	G_OBJECT_CLASS(xfdashboard_stage_parent_class)->dispose(inObject);
}

/* Set/get properties */
static void _xfdashboard_stage_set_property(GObject *inObject,
											guint inPropID,
											const GValue *inValue,
											GParamSpec *inSpec)
{
	XfdashboardStage			*self=XFDASHBOARD_STAGE(inObject);

	switch(inPropID)
	{
		case PROP_BACKGROUND_IMAGE_TYPE:
			xfdashboard_stage_set_background_image_type(self, g_value_get_enum(inValue));
			break;

		case PROP_BACKGROUND_COLOR:
			xfdashboard_stage_set_background_color(self, clutter_value_get_color(inValue));
			break;

		case PROP_SWITCH_TO_VIEW:
			xfdashboard_stage_set_switch_to_view(self, g_value_get_string(inValue));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(inObject, inPropID, inSpec);
			break;
	}
}

static void _xfdashboard_stage_get_property(GObject *inObject,
											guint inPropID,
											GValue *outValue,
											GParamSpec *inSpec)
{
	XfdashboardStage			*self=XFDASHBOARD_STAGE(inObject);
	XfdashboardStagePrivate		*priv=self->priv;

	switch(inPropID)
	{
		case PROP_BACKGROUND_IMAGE_TYPE:
			g_value_set_enum(outValue, priv->backgroundType);
			break;

		case PROP_BACKGROUND_COLOR:
			clutter_value_set_color(outValue, priv->backgroundColor);
			break;

		case PROP_SWITCH_TO_VIEW:
			g_value_set_string(outValue, priv->switchToView);
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(inObject, inPropID, inSpec);
			break;
	}
}

/* Class initialization
 * Override functions in parent classes and define properties
 * and signals
 */
static void xfdashboard_stage_class_init(XfdashboardStageClass *klass)
{
	ClutterActorClass				*actorClass=CLUTTER_ACTOR_CLASS(klass);
	GObjectClass					*gobjectClass=G_OBJECT_CLASS(klass);

	/* Override functions */
	klass->show_tooltip=_xfdashboard_stage_show_tooltip;
	klass->hide_tooltip=_xfdashboard_stage_hide_tooltip;

	actorClass->show=_xfdashboard_stage_show;
	actorClass->event=_xfdashboard_stage_event;

	gobjectClass->dispose=_xfdashboard_stage_dispose;
	gobjectClass->set_property=_xfdashboard_stage_set_property;
	gobjectClass->get_property=_xfdashboard_stage_get_property;

	/* Define properties */
	XfdashboardStageProperties[PROP_BACKGROUND_IMAGE_TYPE]=
		g_param_spec_enum("background-image-type",
							"Background image type",
							"Background image type",
							XFDASHBOARD_TYPE_STAGE_BACKGROUND_IMAGE_TYPE,
							XFDASHBOARD_STAGE_BACKGROUND_IMAGE_TYPE_NONE,
							G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	XfdashboardStageProperties[PROP_BACKGROUND_COLOR]=
		clutter_param_spec_color("background-color",
									"Background color",
									"Color of stage's background",
									NULL,
									G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	XfdashboardStageProperties[PROP_SWITCH_TO_VIEW]=
		g_param_spec_string("switch-to-view",
							"Switch to view",
							"Switch to this named view as soon as stage gets visible",
							NULL,
							G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(gobjectClass, PROP_LAST, XfdashboardStageProperties);

	/* Define signals */
	XfdashboardStageSignals[SIGNAL_ACTOR_CREATED]=
		g_signal_new("actor-created",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST,
						G_STRUCT_OFFSET(XfdashboardStageClass, actor_created),
						NULL,
						NULL,
						g_cclosure_marshal_VOID__OBJECT,
						G_TYPE_NONE,
						1,
						CLUTTER_TYPE_ACTOR);

	XfdashboardStageSignals[SIGNAL_SHOW_TOOLTIP]=
		g_signal_new("show-tooltip",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
						G_STRUCT_OFFSET(XfdashboardStageClass, show_tooltip),
						NULL,
						NULL,
						g_cclosure_marshal_VOID__OBJECT,
						G_TYPE_NONE,
						1,
						CLUTTER_TYPE_ACTION);

	XfdashboardStageSignals[SIGNAL_HIDE_TOOLTIP]=
		g_signal_new("hide-tooltip",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
						G_STRUCT_OFFSET(XfdashboardStageClass, hide_tooltip),
						NULL,
						NULL,
						g_cclosure_marshal_VOID__OBJECT,
						G_TYPE_NONE,
						1,
						CLUTTER_TYPE_ACTION);
}

/* Object initialization
 * Create private structure and set up default values
 */
static void xfdashboard_stage_init(XfdashboardStage *self)
{
	XfdashboardStagePrivate		*priv;
	XfdashboardCore				*core;
	ClutterConstraint			*widthConstraint;
	ClutterConstraint			*heightConstraint;
	ClutterColor				transparent;

	priv=self->priv=xfdashboard_stage_get_instance_private(self);

	/* Set default values */
	priv->focusManager=xfdashboard_core_get_focus_manager(NULL);
	priv->windowTracker=xfdashboard_core_get_window_tracker(NULL);
	priv->stageWindow=NULL;
	priv->primaryInterface=NULL;
	priv->workspaces=NULL;
	priv->viewpad=NULL;
	priv->notification=NULL;
	priv->tooltip=NULL;
	priv->notificationTimeoutID=0;
	priv->backgroundType=XFDASHBOARD_STAGE_BACKGROUND_IMAGE_TYPE_NONE;
	priv->backgroundColor=NULL;
	priv->backgroundColorLayer=NULL;
	priv->backgroundImageLayer=NULL;
	priv->switchToView=NULL;
	priv->focusActorOnShow=NULL;
	priv->settings=g_object_ref(xfdashboard_core_get_settings(NULL));

	/* Create background actors but order of adding background children is important */
	widthConstraint=clutter_bind_constraint_new(CLUTTER_ACTOR(self), CLUTTER_BIND_WIDTH, 0.0f);
	heightConstraint=clutter_bind_constraint_new(CLUTTER_ACTOR(self), CLUTTER_BIND_HEIGHT, 0.0f);
	priv->backgroundImageLayer=clutter_actor_new();
	clutter_actor_hide(priv->backgroundImageLayer);
	clutter_actor_add_constraint(priv->backgroundImageLayer, widthConstraint);
	clutter_actor_add_constraint(priv->backgroundImageLayer, heightConstraint);
	clutter_actor_add_child(CLUTTER_ACTOR(self), priv->backgroundImageLayer);

	widthConstraint=clutter_bind_constraint_new(CLUTTER_ACTOR(self), CLUTTER_BIND_WIDTH, 0.0f);
	heightConstraint=clutter_bind_constraint_new(CLUTTER_ACTOR(self), CLUTTER_BIND_HEIGHT, 0.0f);
	priv->backgroundColorLayer=clutter_actor_new();
	clutter_actor_hide(priv->backgroundColorLayer);
	clutter_actor_add_constraint(priv->backgroundColorLayer, widthConstraint);
	clutter_actor_add_constraint(priv->backgroundColorLayer, heightConstraint);
	clutter_actor_add_child(CLUTTER_ACTOR(self), priv->backgroundColorLayer);

	/* Set up stage and style it */
	clutter_color_init(&transparent, 0, 0, 0, 0);
	clutter_actor_set_background_color(CLUTTER_ACTOR(self), &transparent);

	clutter_stage_set_use_alpha(CLUTTER_STAGE(self), TRUE);
	clutter_stage_set_user_resizable(CLUTTER_STAGE(self), TRUE);
	/* Top-strip mode: only the 6 workspace squares at the top, never
	 * fullscreen. The window is sized in _on_screen_size_changed.
	 */
	clutter_stage_set_fullscreen(CLUTTER_STAGE(self), FALSE);

	/* Connect signals to window tracker */
	g_signal_connect_swapped(priv->windowTracker,
								"monitor-added",
								G_CALLBACK(_xfdashboard_stage_on_monitor_added),
								self);
	g_signal_connect_swapped(priv->windowTracker,
								"monitor-removed",
								G_CALLBACK(_xfdashboard_stage_on_monitor_removed),
								self);
	g_signal_connect_swapped(priv->windowTracker,
								"primary-monitor-changed",
								G_CALLBACK(_xfdashboard_stage_on_primary_monitor_changed),
								self);
	g_signal_connect_swapped(priv->windowTracker,
								"active-window-changed",
								G_CALLBACK(_xfdashboard_stage_on_active_window_changed),
								self);
	g_signal_connect_swapped(priv->windowTracker,
								"active-workspace-changed",
								G_CALLBACK(_xfdashboard_stage_on_active_workspace_changed),
								self);

	/* Connect signal to core */
	core=xfdashboard_core_get_default();
	g_signal_connect_swapped(core,
								"suspend",
								G_CALLBACK(_xfdashboard_stage_on_application_suspend),
								self);

	g_signal_connect_swapped(core,
								"resume",
								G_CALLBACK(_xfdashboard_stage_on_application_resume),
								self);

	g_signal_connect_swapped(core,
								"theme-changed",
								G_CALLBACK(_xfdashboard_stage_on_application_theme_changed),
								self);

	/* Resize stage to match screen size and listen for futher screen size changes
	 * to resize stage again.
	 * This should only be needed when compiled against Clutter prior to 0.17.2
	 * because this version or newer ones seem to handle window resizes correctly.
	 */
	if(clutter_major_version<1 ||
		(clutter_major_version==1 && clutter_minor_version<17) ||
		(clutter_major_version==1 && clutter_minor_version==17 && clutter_micro_version<2))
	{
		_xfdashboard_stage_on_screen_size_changed(self, priv->windowTracker);

		g_signal_connect_swapped(priv->windowTracker,
									"screen-size-changed",
									G_CALLBACK(_xfdashboard_stage_on_screen_size_changed),
									self);

		XFDASHBOARD_DEBUG(self, ACTOR, "Tracking screen resizes to resize stage");
	}
}

/* IMPLEMENTATION: Public API */

/* Create new instance */
ClutterActor* xfdashboard_stage_new(void)
{
	return(CLUTTER_ACTOR(g_object_new(XFDASHBOARD_TYPE_STAGE, NULL)));
}

/* Get/set background type */
XfdashboardStageBackgroundImageType xfdashboard_stage_get_background_image_type(XfdashboardStage *self)
{
	g_return_val_if_fail(XFDASHBOARD_IS_STAGE(self), XFDASHBOARD_STAGE_BACKGROUND_IMAGE_TYPE_NONE);

	return(self->priv->backgroundType);
}

void xfdashboard_stage_set_background_image_type(XfdashboardStage *self, XfdashboardStageBackgroundImageType inType)
{
	XfdashboardStagePrivate		*priv;

	g_return_if_fail(XFDASHBOARD_IS_STAGE(self));
	g_return_if_fail(inType<=XFDASHBOARD_STAGE_BACKGROUND_IMAGE_TYPE_DESKTOP);

	priv=self->priv;


	/* Set value if changed */
	if(priv->backgroundType!=inType)
	{
		/* Set value */
		priv->backgroundType=inType;

		/* Set up background actor depending on type */
		if(priv->backgroundImageLayer)
		{
			switch(priv->backgroundType)
			{
				case XFDASHBOARD_STAGE_BACKGROUND_IMAGE_TYPE_DESKTOP:
					{
						XfdashboardWindowTrackerWindow	*backgroundWindow;

						backgroundWindow=xfdashboard_window_tracker_get_root_window(priv->windowTracker);
						if(backgroundWindow)
						{
							ClutterContent				*backgroundContent;

							backgroundContent=xfdashboard_window_tracker_window_get_content(backgroundWindow);
							clutter_actor_show(priv->backgroundImageLayer);
							clutter_actor_set_content(priv->backgroundImageLayer, backgroundContent);
							g_object_unref(backgroundContent);

							XFDASHBOARD_DEBUG(self, ACTOR, "Desktop window was found and set up as background image for stage");
						}
							else
							{
								g_signal_connect_swapped(priv->windowTracker,
															"window-opened",
															G_CALLBACK(_xfdashboard_stage_on_desktop_window_opened),
															self);
								XFDASHBOARD_DEBUG(self, ACTOR, "Desktop window was not found. Setting up signal to get notified when desktop window might be opened.");
							}
					}
					break;

				default:
					clutter_actor_hide(priv->backgroundImageLayer);
					clutter_actor_set_content(priv->backgroundImageLayer, NULL);
					break;
			}
		}

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfdashboardStageProperties[PROP_BACKGROUND_IMAGE_TYPE]);
	}
}

/* Get/set background color */
ClutterColor* xfdashboard_stage_get_background_color(XfdashboardStage *self)
{
	g_return_val_if_fail(XFDASHBOARD_IS_STAGE(self), NULL);

	return(self->priv->backgroundColor);
}

void xfdashboard_stage_set_background_color(XfdashboardStage *self, const ClutterColor *inColor)
{
	XfdashboardStagePrivate		*priv;

	g_return_if_fail(XFDASHBOARD_IS_STAGE(self));

	priv=self->priv;

	/* Set value if changed */
	if((priv->backgroundColor && !inColor) ||
		(!priv->backgroundColor && inColor) ||
		(inColor && clutter_color_equal(inColor, priv->backgroundColor)==FALSE))
	{
		/* Set value */
		if(priv->backgroundColor)
		{
			clutter_color_free(priv->backgroundColor);
			priv->backgroundColor=NULL;
		}

		if(inColor) priv->backgroundColor=clutter_color_copy(inColor);

		/* If a color is provided set background color and show background actor
		 * otherwise hide background actor
		 */
		if(priv->backgroundColorLayer)
		{
			if(priv->backgroundColor)
			{
				clutter_actor_set_background_color(priv->backgroundColorLayer,
													priv->backgroundColor);
				clutter_actor_show(priv->backgroundColorLayer);
			}
				else clutter_actor_hide(priv->backgroundColorLayer);
		}

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfdashboardStageProperties[PROP_BACKGROUND_COLOR]);
	}
}

/* Set name of view to switch to at next resume */
const gchar* xfdashboard_stage_get_switch_to_view(XfdashboardStage *self)
{
	g_return_val_if_fail(XFDASHBOARD_IS_STAGE(self), NULL);

	return(self->priv->switchToView);
}

void xfdashboard_stage_set_switch_to_view(XfdashboardStage *self, const gchar *inViewInternalName)
{
	XfdashboardStagePrivate		*priv;

	g_return_if_fail(XFDASHBOARD_IS_STAGE(self));

	priv=self->priv;

	/* Set value if changed */
	if(g_strcmp0(priv->switchToView, inViewInternalName)!=0)
	{
		if(priv->switchToView)
		{
			g_free(priv->switchToView);
			priv->switchToView=NULL;
		}

		if(inViewInternalName) priv->switchToView=g_strdup(inViewInternalName);

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfdashboardStageProperties[PROP_SWITCH_TO_VIEW]);
	}
}

/* Show a notification on stage */
void xfdashboard_stage_show_notification(XfdashboardStage *self, const gchar *inIconName, const gchar *inText)
{
	XfdashboardStagePrivate		*priv;
	gint						interval;

	g_return_if_fail(XFDASHBOARD_IS_STAGE(self));

	priv=self->priv;

	/* Stop current running timeout source because it would hide this
	 * new notification to soon.
	 */
	if(priv->notificationTimeoutID)
	{
		g_source_remove(priv->notificationTimeoutID);
		priv->notificationTimeoutID=0;
	}

	/* Only show notification if a notification box is known where the notification
	 * could be shown at.
	 */
	if(!priv->notification)
	{
		XFDASHBOARD_DEBUG(self, ACTOR, "Cannot show notification because no notification box is available");
		return;
	}

	/* Show notification on stage */
	xfdashboard_text_box_set_text(XFDASHBOARD_TEXT_BOX(priv->notification), inText);
	xfdashboard_text_box_set_primary_icon(XFDASHBOARD_TEXT_BOX(priv->notification), inIconName);
	clutter_actor_show(CLUTTER_ACTOR(priv->notification));

	/* Set up timeout source. The timeout interval differs and depends on the length
	 * of the notification text to show but never drops below the minimum timeout configured.
	 * The interval is calculated by one second for 30 characters.
	 */
	interval=xfdashboard_settings_get_notification_timeout(priv->settings);
	interval=MAX((gint)((strlen(inText)/30.0f)*1000.0f), interval);

	priv->notificationTimeoutID=clutter_threads_add_timeout_full(G_PRIORITY_DEFAULT,
																	interval,
																	_xfdashboard_stage_on_notification_timeout,
																	self,
																	_xfdashboard_stage_on_notification_timeout_destroyed);
}
