/*
 * live-workspace: An actor showing the content of a workspace which will
 *                 be updated if changed.
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

#include <libxfce-overview-workspaces/live-workspace.h>

#include <glib/gi18n-lib.h>
#include <math.h>

#include <libxfce-overview-workspaces/button.h>
#include <libxfce-overview-workspaces/live-window-simple.h>
#include <libxfce-overview-workspaces/window-tracker.h>
#include <libxfce-overview-workspaces/click-action.h>
#include <libxfce-overview-workspaces/drag-action.h>
#include <libxfce-overview-workspaces/image-content.h>
#include <libxfce-overview-workspaces/enums.h>
#include <libxfce-overview-workspaces/stylable.h>
#include <libxfce-overview-workspaces/core.h>
#include <libxfce-overview-workspaces/debug.h>


/* Define this class in GObject system */
struct _XfceOverviewWorkspacesLiveWorkspacePrivate
{
	/* Properties related */
	XfceOverviewWorkspacesWindowTrackerWorkspace		*workspace;
	XfceOverviewWorkspacesWindowTrackerMonitor			*monitor;
	XfceOverviewWorkspacesStageBackgroundImageType		backgroundType;
	gboolean								showWorkspaceName;
	gfloat									workspaceNamePadding;

	/* Instance related */
	XfceOverviewWorkspacesWindowTracker				*windowTracker;
	ClutterActor							*backgroundImageLayer;
	ClutterActor							*actorTitle;
	ClutterAction							*clickAction;
};

G_DEFINE_TYPE_WITH_PRIVATE(XfceOverviewWorkspacesLiveWorkspace,
							xfce_overview_workspaces_live_workspace,
							XFCE_OVERVIEW_WORKSPACES_TYPE_BACKGROUND)

/* Properties */
enum
{
	PROP_0,

	PROP_WORKSPACE,
	PROP_MONITOR,
	PROP_BACKGROUND_IMAGE_TYPE,
	PROP_SHOW_WORKSPACE_NAME,
	PROP_WORKSPACE_NAME_PADDING,

	PROP_LAST
};

static GParamSpec* XfceOverviewWorkspacesLiveWorkspaceProperties[PROP_LAST]={ 0, };

/* Signals */
enum
{
	SIGNAL_CLICKED,

	SIGNAL_LAST
};

static guint XfceOverviewWorkspacesLiveWorkspaceSignals[SIGNAL_LAST]={ 0, };

/* IMPLEMENTATION: Private variables and methods */
#define DEFAULT_DRAG_HANDLE_SIZE						32.0f

/* Check if window should be shown */
static gboolean _xfce_overview_workspaces_live_workspace_is_visible_window(XfceOverviewWorkspacesLiveWorkspace *self,
																XfceOverviewWorkspacesWindowTrackerWindow *inWindow)
{
	XfceOverviewWorkspacesLiveWorkspacePrivate			*priv;
	XfceOverviewWorkspacesWindowTrackerWindowState		state;
	gboolean								visible;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WORKSPACE(self), FALSE);
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_WINDOW(inWindow), FALSE);

	priv=self->priv;

	/* Determine if windows should be shown at workspace depending on its state.
	 * NOTE: Minimized windows are intentionally NOT excluded here. A minimized
	 * window of this workspace must still get an actor showing its last known
	 * (frozen) live image instead of disappearing or falling back to an icon.
	 */
	visible=TRUE;

	state=xfce_overview_workspaces_window_tracker_window_get_state(inWindow);
	if(visible && (state & XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW_STATE_SKIP_PAGER))
	{
		XFCE_OVERVIEW_WORKSPACES_DEBUG(self, ACTOR,
							"Window '%s' is invisible due to 'skip-pager' window state",
							xfce_overview_workspaces_window_tracker_window_get_name(inWindow));
		visible=FALSE;
	}

	if(visible && (state & XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW_STATE_SKIP_TASKLIST))
	{
		XFCE_OVERVIEW_WORKSPACES_DEBUG(self, ACTOR,
							"Window '%s' is invisible due to 'skip-tasklist' window state",
							xfce_overview_workspaces_window_tracker_window_get_name(inWindow));
		visible=FALSE;
	}

	/* NOTE: Do NOT filter via xfce_overview_workspaces_window_tracker_window_is_visible()
	 * here. That helper returns FALSE for HIDDEN windows and libwnck sets
	 * WNCK_WINDOW_STATE_HIDDEN for every window which is not on the active
	 * workspace. Filtering on it would leave every inactive workspace empty
	 * (or with only an icon fallback stretched over the thumbnail) while
	 * only the active workspace shows live thumbnails. Workspace membership
	 * below (is_on_workspace / pinned) already decides if a window belongs
	 * to this LiveWorkspace; minimized/hidden windows must still keep their
	 * actor with the last known (frozen) live image.
	 */

	if(visible &&
		(!priv->workspace && !(state & XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW_STATE_PINNED)))
	{
		XFCE_OVERVIEW_WORKSPACES_DEBUG(self, ACTOR,
							"Window '%s' is invisible because no workspace was set and window is not pinned",
							xfce_overview_workspaces_window_tracker_window_get_name(inWindow));
		visible=FALSE;
	}

	if(visible &&
		(priv->workspace && !xfce_overview_workspaces_window_tracker_window_is_on_workspace(inWindow, priv->workspace)))
	{
		XFCE_OVERVIEW_WORKSPACES_DEBUG(self, ACTOR,
							"Window '%s' is invisible because window is not on workspace '%s'",
							xfce_overview_workspaces_window_tracker_window_get_name(inWindow),
							xfce_overview_workspaces_window_tracker_workspace_get_name(priv->workspace));
		visible=FALSE;
	}

	if(visible &&
		xfce_overview_workspaces_window_tracker_window_is_stage(inWindow))
	{
		XFCE_OVERVIEW_WORKSPACES_DEBUG(self, ACTOR,
							"Window '%s' is invisible because it is the stage window",
							xfce_overview_workspaces_window_tracker_window_get_name(inWindow));
		visible=FALSE;
	}

	/* Return visibility result */
	return(visible);
}

/* Find live window actor by window */
static ClutterActor* _xfce_overview_workspaces_live_workspace_find_by_window(XfceOverviewWorkspacesLiveWorkspace *self,
																XfceOverviewWorkspacesWindowTrackerWindow *inWindow)
{
	ClutterActorIter				iter;
	ClutterActor					*child;
	XfceOverviewWorkspacesLiveWindowSimple		*windowActor;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WORKSPACE(self), NULL);
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_WINDOW(inWindow), NULL);

	/* Iterate through list of current actors and find the one for requested window */
	clutter_actor_iter_init(&iter, CLUTTER_ACTOR(self));
	while(clutter_actor_iter_next(&iter, &child))
	{
		/* Check if it is really a window actor */
		if(!XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WINDOW_SIMPLE(child)) continue;

		windowActor=XFCE_OVERVIEW_WORKSPACES_LIVE_WINDOW_SIMPLE(child);

		/* Check if this is the actor showing requested window */
		if(xfce_overview_workspaces_live_window_simple_get_window(windowActor)==inWindow) return(child);
	}

	/* If we get here we did not find the window and we return NULL */
	return(NULL);
}

/* Drag of a live window begins */
static void _xfce_overview_workspaces_live_workspace_on_window_drag_begin(ClutterDragAction *inAction,
																ClutterActor *inActor,
																gfloat inStageX,
																gfloat inStageY,
																ClutterModifierType inModifiers,
																gpointer inUserData)
{
	XfceOverviewWorkspacesLiveWorkspace			*self;
	XfceOverviewWorkspacesLiveWorkspacePrivate		*priv;
	ClutterActor						*dragHandle;
	ClutterStage						*stage;
	GdkPixbuf							*windowIcon;
	ClutterContent						*image;
	XfceOverviewWorkspacesLiveWindowSimple			*liveWindow;

	g_return_if_fail(CLUTTER_IS_DRAG_ACTION(inAction));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WINDOW_SIMPLE(inActor));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WORKSPACE(inUserData));

	self=XFCE_OVERVIEW_WORKSPACES_LIVE_WORKSPACE(inUserData);
	priv=self->priv;
	liveWindow=XFCE_OVERVIEW_WORKSPACES_LIVE_WINDOW_SIMPLE(inActor);

	/* Prevent signal "clicked" from being emitted on live workspace actor */
	clutter_actor_meta_set_enabled(CLUTTER_ACTOR_META(priv->clickAction), FALSE);

	/* Get stage */
	stage=CLUTTER_STAGE(clutter_actor_get_stage(inActor));

	/* Create a application icon for drag handle */
	windowIcon=xfce_overview_workspaces_window_tracker_window_get_icon(xfce_overview_workspaces_live_window_simple_get_window(liveWindow));
	image=xfce_overview_workspaces_image_content_new_for_pixbuf(windowIcon);

	dragHandle=xfce_overview_workspaces_background_new();
	clutter_actor_set_position(dragHandle, inStageX, inStageY);
	clutter_actor_set_size(dragHandle, DEFAULT_DRAG_HANDLE_SIZE, DEFAULT_DRAG_HANDLE_SIZE);
	xfce_overview_workspaces_background_set_image(XFCE_OVERVIEW_WORKSPACES_BACKGROUND(dragHandle), CLUTTER_IMAGE(image));
	clutter_actor_add_child(CLUTTER_ACTOR(stage), dragHandle);

	clutter_drag_action_set_drag_handle(inAction, dragHandle);

	g_object_unref(image);
}

/* Drag of a live window ends */
static void _xfce_overview_workspaces_live_workspace_on_window_on_drag_end(ClutterDragAction *inAction,
																ClutterActor *inActor,
																gfloat inStageX,
																gfloat inStageY,
																ClutterModifierType inModifiers,
																gpointer inUserData)
{
	XfceOverviewWorkspacesLiveWorkspace			*self;
	XfceOverviewWorkspacesLiveWorkspacePrivate		*priv;
	ClutterActor						*dragHandle;

	g_return_if_fail(CLUTTER_IS_DRAG_ACTION(inAction));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WINDOW_SIMPLE(inActor));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WORKSPACE(inUserData));

	self=XFCE_OVERVIEW_WORKSPACES_LIVE_WORKSPACE(inUserData);
	priv=self->priv;

	/* Destroy clone of application icon used as drag handle */
	dragHandle=clutter_drag_action_get_drag_handle(inAction);
	if(dragHandle)
	{
		clutter_drag_action_set_drag_handle(inAction, NULL);
		clutter_actor_destroy(dragHandle);
	}

	/* Allow signal "clicked" from being emitted again */
	clutter_actor_meta_set_enabled(CLUTTER_ACTOR_META(priv->clickAction), TRUE);
}

/* Create actor for window but respect window stacking when adding */
static ClutterActor* _xfce_overview_workspaces_live_workspace_create_and_add_window_actor(XfceOverviewWorkspacesLiveWorkspace *self,
																				XfceOverviewWorkspacesWindowTrackerWindow *inWindow)
{
	XfceOverviewWorkspacesLiveWorkspacePrivate		*priv;
	ClutterActor						*actor;
	GList								*windows;
	ClutterActor						*lastWindowActor;
	ClutterActor						*existingActor;
	XfceOverviewWorkspacesWindowTrackerWindow		*window;
	ClutterAction						*action;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WORKSPACE(self), NULL);
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_WINDOW(inWindow), NULL);

	priv=self->priv;

	/* We cannot assume that each window newly opened or moved to this workspace
	 * will be on top of all other windows. We need to respect window stacking
	 * (get_windows_stacked() returns bottom-to-top, so children are painted in
	 * that order with the top window last, just below the title actor).
	 * Therefore we iterate through the stacked list from bottom to top and
	 * remember the last window we already have an actor for *below* the
	 * requested window. No culling is done: overlapping windows simply
	 * overdraw, so only the top one remains visible.
	 */
	lastWindowActor=NULL;
	windows=xfce_overview_workspaces_window_tracker_get_windows_stacked(priv->windowTracker);
	for( ; windows; windows=g_list_next(windows))
	{
		/* Get window from list */
		window=XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW(windows->data);
		if(!window) continue;

		/* Stop at requested window: only actors below it are predecessors */
		if(window==inWindow) break;

		/* We do not need to check if window would be visible on this workspace
		 * as it should not have been created if it is not visible.
		 */
		existingActor=_xfce_overview_workspaces_live_workspace_find_by_window(self, window);
		if(existingActor) lastWindowActor=existingActor;
	}

	/* Check if we have to "move" an existing window actor or if we have to create
	 * a new actor for window
	 */
	actor=_xfce_overview_workspaces_live_workspace_find_by_window(self, inWindow);
	if(actor)
	{
		/* Move existing window actor to new stacking position */
		g_object_ref(actor);
		clutter_actor_remove_child(CLUTTER_ACTOR(self), actor);
		if(lastWindowActor && lastWindowActor!=actor)
		{
			clutter_actor_insert_child_above(CLUTTER_ACTOR(self), actor, lastWindowActor);
			XFCE_OVERVIEW_WORKSPACES_DEBUG(self, ACTOR,
								"Moved existing actor for window '%s' above actor for window '%s' at live workspace '%s'",
								xfce_overview_workspaces_window_tracker_window_get_name(xfce_overview_workspaces_live_window_simple_get_window(XFCE_OVERVIEW_WORKSPACES_LIVE_WINDOW_SIMPLE(actor))),
								xfce_overview_workspaces_window_tracker_window_get_name(xfce_overview_workspaces_live_window_simple_get_window(XFCE_OVERVIEW_WORKSPACES_LIVE_WINDOW_SIMPLE(lastWindowActor))),
								xfce_overview_workspaces_window_tracker_workspace_get_name(priv->workspace));
		}
			else if(priv->backgroundImageLayer)
			{
				/* No predecessor: window is bottommost, so place it right
				 * above the background layer (bottom of window stack).
				 */
				clutter_actor_insert_child_above(CLUTTER_ACTOR(self), actor, priv->backgroundImageLayer);
				XFCE_OVERVIEW_WORKSPACES_DEBUG(self, ACTOR,
									"Moved existing actor for window '%s' to bottom at live workspace '%s'",
									xfce_overview_workspaces_window_tracker_window_get_name(xfce_overview_workspaces_live_window_simple_get_window(XFCE_OVERVIEW_WORKSPACES_LIVE_WINDOW_SIMPLE(actor))),
									xfce_overview_workspaces_window_tracker_workspace_get_name(priv->workspace));
			}
			else
			{
				clutter_actor_insert_child_below(CLUTTER_ACTOR(self), actor, priv->actorTitle);
				XFCE_OVERVIEW_WORKSPACES_DEBUG(self, ACTOR,
									"Moved existing actor for window '%s' to bottom at live workspace '%s'",
									xfce_overview_workspaces_window_tracker_window_get_name(xfce_overview_workspaces_live_window_simple_get_window(XFCE_OVERVIEW_WORKSPACES_LIVE_WINDOW_SIMPLE(actor))),
									xfce_overview_workspaces_window_tracker_workspace_get_name(priv->workspace));
			}
		g_object_unref(actor);

	}
		else
		{
			/* Create actor */
			actor=xfce_overview_workspaces_live_window_simple_new_for_window(inWindow);

			/* Add drag action to actor */
			action=xfce_overview_workspaces_drag_action_new_with_source(CLUTTER_ACTOR(self));
			clutter_drag_action_set_drag_threshold(CLUTTER_DRAG_ACTION(action), -1, -1);
			clutter_actor_add_action(actor, action);
			g_signal_connect(action, "drag-begin", G_CALLBACK(_xfce_overview_workspaces_live_workspace_on_window_drag_begin), self);
			g_signal_connect(action, "drag-end", G_CALLBACK(_xfce_overview_workspaces_live_workspace_on_window_on_drag_end), self);

			/* Add new actor at right stacking position:
		 * above its predecessor or, if it has none (bottommost window),
		 * right above the background layer (bottom of window stack).
		 */
			if(lastWindowActor)
			{
				clutter_actor_insert_child_above(CLUTTER_ACTOR(self), actor, lastWindowActor);
				XFCE_OVERVIEW_WORKSPACES_DEBUG(self, ACTOR,
									"Created new actor for window '%s' above actor for window '%s' at live workspace '%s'",
									xfce_overview_workspaces_window_tracker_window_get_name(xfce_overview_workspaces_live_window_simple_get_window(XFCE_OVERVIEW_WORKSPACES_LIVE_WINDOW_SIMPLE(actor))),
									xfce_overview_workspaces_window_tracker_window_get_name(xfce_overview_workspaces_live_window_simple_get_window(XFCE_OVERVIEW_WORKSPACES_LIVE_WINDOW_SIMPLE(lastWindowActor))),
									xfce_overview_workspaces_window_tracker_workspace_get_name(priv->workspace));
			}
				else if(priv->backgroundImageLayer)
				{
					clutter_actor_insert_child_above(CLUTTER_ACTOR(self), actor, priv->backgroundImageLayer);
					XFCE_OVERVIEW_WORKSPACES_DEBUG(self, ACTOR,
										"Created new actor for window '%s' to bottom at live workspace '%s'",
										xfce_overview_workspaces_window_tracker_window_get_name(xfce_overview_workspaces_live_window_simple_get_window(XFCE_OVERVIEW_WORKSPACES_LIVE_WINDOW_SIMPLE(actor))),
										xfce_overview_workspaces_window_tracker_workspace_get_name(priv->workspace));
				}
				else
				{
					clutter_actor_insert_child_below(CLUTTER_ACTOR(self), actor, priv->actorTitle);
					XFCE_OVERVIEW_WORKSPACES_DEBUG(self, ACTOR,
										"Created new actor for window '%s' to bottom at live workspace '%s'",
										xfce_overview_workspaces_window_tracker_window_get_name(xfce_overview_workspaces_live_window_simple_get_window(XFCE_OVERVIEW_WORKSPACES_LIVE_WINDOW_SIMPLE(actor))),
										xfce_overview_workspaces_window_tracker_workspace_get_name(priv->workspace));
				}
		}

	return(actor);
}

/* This actor was clicked */
static void _xfce_overview_workspaces_live_workspace_on_clicked(XfceOverviewWorkspacesLiveWorkspace *self, ClutterActor *inActor, gpointer inUserData)
{
	XfceOverviewWorkspacesClickAction			*action;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WORKSPACE(self));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_CLICK_ACTION(inUserData));

	action=XFCE_OVERVIEW_WORKSPACES_CLICK_ACTION(inUserData);

	/* Only emit any of these signals if click was perform with left button 
	 * or is a short touchscreen touch event.
	 */
	if(xfce_overview_workspaces_click_action_is_left_button_or_tap(action))
	{
		/* Emit "clicked" signal */
		g_signal_emit(self, XfceOverviewWorkspacesLiveWorkspaceSignals[SIGNAL_CLICKED], 0);
	}
}

/* A window was closed */
static void _xfce_overview_workspaces_live_workspace_on_window_closed(XfceOverviewWorkspacesLiveWorkspace *self,
															XfceOverviewWorkspacesWindowTrackerWindow *inWindow,
															gpointer inUserData)
{
	ClutterActor		*windowActor;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WORKSPACE(self));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_WINDOW(inWindow));

	/* Find and destroy actor */
	windowActor=_xfce_overview_workspaces_live_workspace_find_by_window(self, inWindow);
	if(windowActor) clutter_actor_destroy(windowActor);
}

/* A window was opened */
static void _xfce_overview_workspaces_live_workspace_on_window_opened(XfceOverviewWorkspacesLiveWorkspace *self,
															XfceOverviewWorkspacesWindowTrackerWindow *inWindow,
															gpointer inUserData)
{
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WORKSPACE(self));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_WINDOW(inWindow));

	/* Check if window is visible on this workspace */
	if(!_xfce_overview_workspaces_live_workspace_is_visible_window(self, inWindow)) return;

	/* Create actor for window */
	_xfce_overview_workspaces_live_workspace_create_and_add_window_actor(self, inWindow);
}

/* A window's position and/or size has changed */
static void _xfce_overview_workspaces_live_workspace_on_window_geometry_changed(XfceOverviewWorkspacesLiveWorkspace *self,
																	XfceOverviewWorkspacesWindowTrackerWindow *inWindow,
																	gpointer inUserData)
{
	ClutterActor		*windowActor;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WORKSPACE(self));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_WINDOW(inWindow));

	/* Actor's allocation may change because of new geometry so relayout
	 * and redraw immediately to update the live photo without throttling.
	 */
	clutter_actor_queue_relayout(CLUTTER_ACTOR(self));
	clutter_actor_queue_redraw(CLUTTER_ACTOR(self));

	windowActor=_xfce_overview_workspaces_live_workspace_find_by_window(self, inWindow);
	if(windowActor)
	{
		clutter_actor_queue_relayout(windowActor);
		clutter_actor_queue_redraw(windowActor);
	}
}

/* Window stacking has changed */
static void _xfce_overview_workspaces_live_workspace_on_window_stacking_changed(XfceOverviewWorkspacesLiveWorkspace *self,
																	gpointer inUserData)
{
	XfceOverviewWorkspacesLiveWorkspacePrivate		*priv;
	GList								*windows;
	XfceOverviewWorkspacesWindowTrackerWindow		*window;
	ClutterActor						*actor;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WORKSPACE(self));

	priv=self->priv;

	/* Get stacked order of windows */
	windows=xfce_overview_workspaces_window_tracker_get_windows_stacked(priv->windowTracker);

	/* Iterate through list of stacked window from beginning (bottom) to end
	 * (top) and reinsert each window found just below the title actor, so the
	 * last painted window actor is the top window. No culling is done:
	 * overlapping windows overdraw and only the top one stays visible.
	 */
	for( ; windows; windows=g_list_next(windows))
	{
		/* Get window and find corresponding actor */
		window=XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW(windows->data);
		if(!window) continue;

		actor=_xfce_overview_workspaces_live_workspace_find_by_window(self, window);
		if(!actor) continue;

		/* If we get here the window actor was found so move it just below
		 * the title actor, preserving bottom-to-top paint order.
		 */
		g_object_ref(actor);
		clutter_actor_remove_child(CLUTTER_ACTOR(self), actor);
		clutter_actor_insert_child_below(CLUTTER_ACTOR(self), actor, priv->actorTitle);
		g_object_unref(actor);
	}
}

/* A window's state has changed */
static void _xfce_overview_workspaces_live_workspace_on_window_state_changed(XfceOverviewWorkspacesLiveWorkspace *self,
																XfceOverviewWorkspacesWindowTrackerWindow *inWindow,
																gpointer inUserData)
{
	/* We need to see it from the point of view of a workspace.
	 * If a window is visible on the workspace but we have no actor
	 * for this window then create it. If a window is not visible anymore
	 * on this workspace then destroy the corresponding actor.
	 * That is why initially we set any window to invisible because if
	 * changed window is not visible on this workspace it will do nothing.
	 */

	ClutterActor		*windowActor;
	gboolean			newVisible;
	gboolean			currentVisible;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WORKSPACE(self));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_WINDOW(inWindow));

	currentVisible=FALSE;

	/* Find window and get current visibility state */
	windowActor=_xfce_overview_workspaces_live_workspace_find_by_window(self, inWindow);
	if(windowActor)
	{
		currentVisible=clutter_actor_is_visible(windowActor);
	}

	/* Check if window's visibility has changed */
	newVisible=_xfce_overview_workspaces_live_workspace_is_visible_window(self, inWindow);
	if(newVisible!=currentVisible)
	{
		if(newVisible) windowActor=_xfce_overview_workspaces_live_workspace_create_and_add_window_actor(self, inWindow);
			else if(windowActor)
			{
				clutter_actor_destroy(windowActor);
				windowActor=NULL;
			}
	}

	/* Refresh immediately (no new throttle): state changes such as
	 * minimize/unminimize keep the actor but freeze/resume its live photo,
	 * so relayout and redraw right away.
	 */
	clutter_actor_queue_relayout(CLUTTER_ACTOR(self));
	clutter_actor_queue_redraw(CLUTTER_ACTOR(self));
	if(windowActor) clutter_actor_queue_redraw(windowActor);
}

/* A window's workspace has changed */
static void _xfce_overview_workspaces_live_workspace_on_window_workspace_changed(XfceOverviewWorkspacesLiveWorkspace *self,
																	XfceOverviewWorkspacesWindowTrackerWindow *inWindow,
																	XfceOverviewWorkspacesWindowTrackerWorkspace *inWorkspace,
																	gpointer inUserData)
{
	ClutterActor						*windowActor;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WORKSPACE(self));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_WINDOW(inWindow));

	(void)inWorkspace;
	(void)inUserData;

	/* Decide by current visibility on this workspace (is_on_workspace /
	 * pinned), not by pointer equality with the destination workspace.
	 * Pointer comparison breaks pinned (on-all-workspaces) windows and
	 * would drop them everywhere except the destination. Each LiveWorkspace
	 * instance (even for an inactive/invisible workspace) must destroy its
	 * actor if the window left, or create/move it if the window joined.
	 */
	if(_xfce_overview_workspaces_live_workspace_is_visible_window(self, inWindow))
	{
		/* Add (or restack, if already present) window actor */
		_xfce_overview_workspaces_live_workspace_create_and_add_window_actor(self, inWindow);
	}
		else
		{
			/* Find and destroy actor for window if available */
			windowActor=_xfce_overview_workspaces_live_workspace_find_by_window(self, inWindow);
			if(windowActor) clutter_actor_destroy(windowActor);
		}
}

/* A monitor's position and/or size has changed */
static void _xfce_overview_workspaces_live_workspace_on_monitor_geometry_changed(XfceOverviewWorkspacesLiveWorkspace *self,
																	gpointer inUserData)
{
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WORKSPACE(self));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_MONITOR(inUserData));

	/* Actor's allocation may change because of new geometry so relayout */
	clutter_actor_queue_relayout(CLUTTER_ACTOR(self));
}

/* A workspace's name has changed */
static void _xfce_overview_workspaces_live_workspace_on_workspace_name_changed(XfceOverviewWorkspacesLiveWorkspace *self,
																	XfceOverviewWorkspacesWindowTrackerWorkspace *inWorkspace,
																	gpointer inUserData)
{
	XfceOverviewWorkspacesLiveWorkspacePrivate		*priv;
	gchar								*workspaceName;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WORKSPACE(self));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_WORKSPACE(inWorkspace));

	priv=self->priv;

	/* Check if signal is for this workspace */
	if(inWorkspace!=priv->workspace) return;

	/* Set new name in title actor */
	workspaceName=g_markup_printf_escaped("%s", xfce_overview_workspaces_window_tracker_workspace_get_name(inWorkspace));
	xfce_overview_workspaces_label_set_text(XFCE_OVERVIEW_WORKSPACES_LABEL(priv->actorTitle), workspaceName);
	g_free(workspaceName);
}

/* A window was created
 * Check if window opened is desktop background window
 */
static void _xfce_overview_workspaces_live_workspace_on_desktop_window_opened(XfceOverviewWorkspacesLiveWorkspace *self,
																	XfceOverviewWorkspacesWindowTrackerWindow *inWindow,
																	gpointer inUserData)
{
	XfceOverviewWorkspacesLiveWorkspacePrivate		*priv;
	XfceOverviewWorkspacesWindowTrackerWindow		*desktopWindow;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WORKSPACE(self));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_WINDOW(inWindow));

	priv=self->priv;

	/* Get desktop background window and check if it is the new window opened */
	desktopWindow=xfce_overview_workspaces_window_tracker_get_root_window(priv->windowTracker);
	if(desktopWindow)
	{
		xfce_overview_workspaces_live_window_simple_set_window(XFCE_OVERVIEW_WORKSPACES_LIVE_WINDOW_SIMPLE(priv->backgroundImageLayer), desktopWindow);
		clutter_actor_show(priv->backgroundImageLayer);

		g_signal_handlers_disconnect_by_func(priv->windowTracker,
												G_CALLBACK(_xfce_overview_workspaces_live_workspace_on_desktop_window_opened),
												self);
		XFCE_OVERVIEW_WORKSPACES_DEBUG(self, ACTOR,
							"Found desktop window for workspace '%s' with signal 'window-opened', so disconnecting signal handler",
							xfce_overview_workspaces_window_tracker_workspace_get_name(priv->workspace));
	}
}

/* IMPLEMENTATION: ClutterActor */

/* Get preferred width/height */
static void _xfce_overview_workspaces_live_workspace_get_preferred_height(ClutterActor *self,
															gfloat inForWidth,
															gfloat *outMinHeight,
															gfloat *outNaturalHeight)
{
	XfceOverviewWorkspacesLiveWorkspacePrivate		*priv=XFCE_OVERVIEW_WORKSPACES_LIVE_WORKSPACE(self)->priv;
	gfloat								minHeight, naturalHeight;
	gfloat								childWidth, childHeight;
	gint								w, h;

	minHeight=naturalHeight=0.0f;

	/* Determine size of workspace if available (should usually be the largest actor) */
	if(priv->workspace)
	{
		if(priv->monitor)
		{
			xfce_overview_workspaces_window_tracker_monitor_get_geometry(priv->monitor, NULL, NULL, &w, &h);
			childWidth=(gfloat)w;
			childHeight=(gfloat)h;
		}
			else
			{
				xfce_overview_workspaces_window_tracker_workspace_get_size(priv->workspace, &w, &h);
				childWidth=(gfloat)w;
				childHeight=(gfloat)h;
			}

		if(inForWidth<0.0f) naturalHeight=childHeight;
			else naturalHeight=(childHeight/childWidth)*inForWidth;
	}

	/* Store sizes computed */
	if(outMinHeight) *outMinHeight=minHeight;
	if(outNaturalHeight) *outNaturalHeight=naturalHeight;
}

static void _xfce_overview_workspaces_live_workspace_get_preferred_width(ClutterActor *self,
															gfloat inForHeight,
															gfloat *outMinWidth,
															gfloat *outNaturalWidth)
{
	XfceOverviewWorkspacesLiveWorkspacePrivate		*priv=XFCE_OVERVIEW_WORKSPACES_LIVE_WORKSPACE(self)->priv;
	gfloat								minWidth, naturalWidth;
	gfloat								childWidth, childHeight;
	gint								w, h;

	minWidth=naturalWidth=0.0f;

	/* Determine size of workspace if available (should usually be the largest actor) */
	if(priv->workspace)
	{
		if(priv->monitor)
		{
			xfce_overview_workspaces_window_tracker_monitor_get_geometry(priv->monitor, NULL, NULL, &w, &h);
			childWidth=(gfloat)w;
			childHeight=(gfloat)h;
		}
			else
			{
				xfce_overview_workspaces_window_tracker_workspace_get_size(priv->workspace, &w, &h);
				childWidth=(gfloat)w;
				childHeight=(gfloat)h;
			}

		if(inForHeight<0.0f) naturalWidth=childWidth;
			else naturalWidth=(childWidth/childHeight)*inForHeight;
	}

	/* Store sizes computed */
	if(outMinWidth) *outMinWidth=minWidth;
	if(outNaturalWidth) *outNaturalWidth=naturalWidth;
}

/* Allocate position and size of actor and its children */
static void _xfce_overview_workspaces_live_workspace_transform_allocation(ClutterActorBox *ioBox,
																const ClutterActorBox *inTotalArea,
																const ClutterActorBox *inVisibleArea,
																const ClutterActorBox *inAllocation)
{
	ClutterActorBox		result;
	gfloat				totalWidth, totalHeight;
	gfloat				visibleWidth, visibleHeight;
	gfloat				allocationWidth, allocationHeight;

	/* Get sizes */
	totalWidth=clutter_actor_box_get_width(inTotalArea);
	totalHeight=clutter_actor_box_get_height(inTotalArea);

	visibleWidth=clutter_actor_box_get_width(inVisibleArea);
	visibleHeight=clutter_actor_box_get_height(inVisibleArea);

	allocationWidth=clutter_actor_box_get_width(inAllocation);
	allocationHeight=clutter_actor_box_get_height(inAllocation);

	/* Transform positions */
	result.x1=((ioBox->x1/totalWidth)*allocationWidth)*(totalWidth/visibleWidth);
	result.x2=((ioBox->x2/totalWidth)*allocationWidth)*(totalWidth/visibleWidth);

	result.y1=((ioBox->y1/totalHeight)*allocationHeight)*(totalHeight/visibleHeight);
	result.y2=((ioBox->y2/totalHeight)*allocationHeight)*(totalHeight/visibleHeight);

	/* Set result */
	ioBox->x1=result.x1;
	ioBox->y1=result.y1;
	ioBox->x2=result.x2;
	ioBox->y2=result.y2;
}

static void _xfce_overview_workspaces_live_workspace_allocate(ClutterActor *self,
												const ClutterActorBox *inBox,
												ClutterAllocationFlags inFlags)
{
	XfceOverviewWorkspacesLiveWorkspacePrivate		*priv=XFCE_OVERVIEW_WORKSPACES_LIVE_WORKSPACE(self)->priv;
	XfceOverviewWorkspacesWindowTrackerWindow		*window;
	gint								x, y, w, h;
	ClutterActor						*child;
	ClutterActorIter					iter;
	ClutterActorBox						childAllocation={ 0, };
	ClutterActorBox						visibleArea={ 0, };
	ClutterActorBox						workspaceArea={ 0, };

	/* Chain up to store the allocation of the actor */
	CLUTTER_ACTOR_CLASS(xfce_overview_workspaces_live_workspace_parent_class)->allocate(self, inBox, inFlags);

	/* Get size of workspace as it is needed to calculate translated position
	 * and size but fallback to size of screen if no workspace is set.
	 */
	if(priv->workspace)
	{
		xfce_overview_workspaces_window_tracker_workspace_get_size(priv->workspace, &w, &h);
		workspaceArea.x1=0.0f;
		workspaceArea.y1=0.0f;
		workspaceArea.x2=w;
		workspaceArea.y2=h;
	}
		else
		{
			xfce_overview_workspaces_window_tracker_get_screen_size(priv->windowTracker, &w, &h);
			workspaceArea.x1=0.0f;
			workspaceArea.y1=0.0f;
			workspaceArea.x2=w;
			workspaceArea.y2=h;
		}

	/* Get visible area of workspace */
	if(priv->monitor)
	{
		xfce_overview_workspaces_window_tracker_monitor_get_geometry(priv->monitor, &x, &y, &w, &h);
		visibleArea.x1=x;
		visibleArea.x2=x+w;
		visibleArea.y1=y;
		visibleArea.y2=y+h;
	}
		else
		{
			visibleArea.x1=0;
			visibleArea.y1=0;
			visibleArea.x2=clutter_actor_box_get_width(&workspaceArea);
			visibleArea.y2=clutter_actor_box_get_height(&workspaceArea);
		}

	/* Resize background image layer to allocation even if it is hidden */
	childAllocation.x1=-visibleArea.x1;
	childAllocation.y1=-visibleArea.y1;
	childAllocation.x2=childAllocation.x1+clutter_actor_box_get_width(&workspaceArea);
	childAllocation.y2=childAllocation.y1+clutter_actor_box_get_height(&workspaceArea);
	_xfce_overview_workspaces_live_workspace_transform_allocation(&childAllocation, &workspaceArea, &visibleArea, inBox);
	clutter_actor_allocate(priv->backgroundImageLayer, &childAllocation, inFlags);

	/* Set allocation of title actor */
	if(priv->actorTitle)
	{
		gfloat							left, right, top, bottom;
		gfloat							titleWidth, titleHeight;
		gfloat							maxWidth;

		clutter_actor_get_preferred_size(priv->actorTitle,
											NULL, NULL,
											&titleWidth, &titleHeight);

		maxWidth=clutter_actor_box_get_width(inBox)-(2*priv->workspaceNamePadding);
		if(titleWidth>maxWidth) titleWidth=maxWidth;

		left=(clutter_actor_box_get_width(inBox)-titleWidth)/2.0f;
		right=left+titleWidth;
		bottom=clutter_actor_box_get_height(inBox)-priv->workspaceNamePadding;
		top=bottom-titleHeight;
		if(left>right) left=right-1.0f;

		right=MAX(left, right);
		bottom=MAX(top, bottom);

		childAllocation.x1=floor(left);
		childAllocation.y1=floor(top);
		childAllocation.x2=floor(right);
		childAllocation.y2=floor(bottom);
		clutter_actor_allocate(priv->actorTitle, &childAllocation, inFlags);
	}

	/* If we handle no workspace to not set allocation of children */
	if(!priv->workspace) return;

	/* Iterate through window actors, calculate translated allocation of
	 * position and size to available size of this actor
	 */
	clutter_actor_iter_init(&iter, self);
	while(clutter_actor_iter_next(&iter, &child))
	{
		/* Get window actor */
		if(!XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WINDOW_SIMPLE(child)) continue;

		/* Get associated window */
		window=xfce_overview_workspaces_live_window_simple_get_window(XFCE_OVERVIEW_WORKSPACES_LIVE_WINDOW_SIMPLE(child));
		if(!window || !XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_WINDOW(window)) continue;

		/* Get real size of child */
		xfce_overview_workspaces_window_tracker_window_get_geometry(window, &x, &y, &w, &h);

		/* Calculate translated position and size of child from the exact
		 * client geometry (no special-casing for fullscreen/split: a
		 * fullscreen app naturally covers 100% of the thumbnail, two
		 * side-by-side apps 50/50 each, and stacked apps overdraw with
		 * only the top one visible).
		 */
		childAllocation.x1=x-visibleArea.x1;
		childAllocation.x2=childAllocation.x1+w;
		childAllocation.y1=y-visibleArea.y1;
		childAllocation.y2=childAllocation.y1+h;
		_xfce_overview_workspaces_live_workspace_transform_allocation(&childAllocation, &workspaceArea, &visibleArea, inBox);

		/* Set allocation of child */
		clutter_actor_allocate(child, &childAllocation, inFlags);

		/* Clip each child to its own box so its live image cannot bleed
		 * over a neighbouring window actor.
		 */
		clutter_actor_set_clip(child,
								0.0f,
								0.0f,
								MAX(0.0f, clutter_actor_box_get_width(&childAllocation)),
								MAX(0.0f, clutter_actor_box_get_height(&childAllocation)));
	}

	/* Set clip if a specific monitor should be shown only otherwise remove clip */
	if(priv->monitor)
	{
		clutter_actor_set_clip(self,
								0.0f,
								0.0f,
								clutter_actor_box_get_width(inBox),
								clutter_actor_box_get_height(inBox));
	}
		else
		{
			clutter_actor_remove_clip(self);
		}
}

/* IMPLEMENTATION: GObject */

/* Dispose this object */
static void _xfce_overview_workspaces_live_workspace_dispose(GObject *inObject)
{
	XfceOverviewWorkspacesLiveWorkspace			*self=XFCE_OVERVIEW_WORKSPACES_LIVE_WORKSPACE(inObject);
	XfceOverviewWorkspacesLiveWorkspacePrivate		*priv=self->priv;

	/* Dispose allocated resources */
	if(priv->actorTitle)
	{
		clutter_actor_destroy(priv->actorTitle);
		priv->actorTitle=NULL;
	}

	if(priv->backgroundImageLayer)
	{
		clutter_actor_destroy(priv->backgroundImageLayer);
		priv->backgroundImageLayer=NULL;
	}

	if(priv->windowTracker)
	{
		g_signal_handlers_disconnect_by_data(priv->windowTracker, self);
		g_object_unref(priv->windowTracker);
		priv->windowTracker=NULL;
	}

	if(priv->monitor)
	{
		g_signal_handlers_disconnect_by_data(priv->monitor, self);
		priv->monitor=NULL;
	}

	if(priv->workspace)
	{
		g_signal_handlers_disconnect_by_data(priv->workspace, self);
		priv->workspace=NULL;
	}

	if(priv->clickAction)
	{
		clutter_actor_remove_action(CLUTTER_ACTOR(self), priv->clickAction);
		priv->clickAction=NULL;
	}

	/* Call parent's class dispose method */
	G_OBJECT_CLASS(xfce_overview_workspaces_live_workspace_parent_class)->dispose(inObject);
}

/* Set/get properties */
static void _xfce_overview_workspaces_live_workspace_set_property(GObject *inObject,
													guint inPropID,
													const GValue *inValue,
													GParamSpec *inSpec)
{
	XfceOverviewWorkspacesLiveWorkspace			*self=XFCE_OVERVIEW_WORKSPACES_LIVE_WORKSPACE(inObject);
	
	switch(inPropID)
	{
		case PROP_WORKSPACE:
			xfce_overview_workspaces_live_workspace_set_workspace(self, g_value_get_object(inValue));
			break;

		case PROP_MONITOR:
			xfce_overview_workspaces_live_workspace_set_monitor(self, g_value_get_object(inValue));
			break;

		case PROP_BACKGROUND_IMAGE_TYPE:
			xfce_overview_workspaces_live_workspace_set_background_image_type(self, g_value_get_enum(inValue));
			break;

		case PROP_SHOW_WORKSPACE_NAME:
			xfce_overview_workspaces_live_workspace_set_show_workspace_name(self, g_value_get_boolean(inValue));
			break;

		case PROP_WORKSPACE_NAME_PADDING:
			xfce_overview_workspaces_live_workspace_set_workspace_name_padding(self, g_value_get_float(inValue));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(inObject, inPropID, inSpec);
			break;
	}
}

static void _xfce_overview_workspaces_live_workspace_get_property(GObject *inObject,
													guint inPropID,
													GValue *outValue,
													GParamSpec *inSpec)
{
	XfceOverviewWorkspacesLiveWorkspace	*self=XFCE_OVERVIEW_WORKSPACES_LIVE_WORKSPACE(inObject);

	switch(inPropID)
	{
		case PROP_WORKSPACE:
			g_value_set_object(outValue, self->priv->workspace);
			break;

		case PROP_MONITOR:
			g_value_set_object(outValue, self->priv->monitor);
			break;

		case PROP_BACKGROUND_IMAGE_TYPE:
			g_value_set_enum(outValue, self->priv->backgroundType);
			break;

		case PROP_SHOW_WORKSPACE_NAME:
			g_value_set_boolean(outValue, self->priv->showWorkspaceName);
			break;

		case PROP_WORKSPACE_NAME_PADDING:
			g_value_set_float(outValue, self->priv->workspaceNamePadding);
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
static void xfce_overview_workspaces_live_workspace_class_init(XfceOverviewWorkspacesLiveWorkspaceClass *klass)
{
	XfceOverviewWorkspacesActorClass	*actorClass=XFCE_OVERVIEW_WORKSPACES_ACTOR_CLASS(klass);
	ClutterActorClass		*clutterActorClass=CLUTTER_ACTOR_CLASS(klass);
	GObjectClass			*gobjectClass=G_OBJECT_CLASS(klass);

	/* Override functions */
	clutterActorClass->get_preferred_width=_xfce_overview_workspaces_live_workspace_get_preferred_width;
	clutterActorClass->get_preferred_height=_xfce_overview_workspaces_live_workspace_get_preferred_height;
	clutterActorClass->allocate=_xfce_overview_workspaces_live_workspace_allocate;

	gobjectClass->dispose=_xfce_overview_workspaces_live_workspace_dispose;
	gobjectClass->set_property=_xfce_overview_workspaces_live_workspace_set_property;
	gobjectClass->get_property=_xfce_overview_workspaces_live_workspace_get_property;

	/* Define properties */
	XfceOverviewWorkspacesLiveWorkspaceProperties[PROP_WORKSPACE]=
		g_param_spec_object("workspace",
								"Workspace",
								"The workspace to show",
								XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_TRACKER_WORKSPACE,
								G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	XfceOverviewWorkspacesLiveWorkspaceProperties[PROP_MONITOR]=
		g_param_spec_object("monitor",
								"Monitor",
								"The monitor whose window to show only",
								XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_TRACKER_MONITOR,
								G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	XfceOverviewWorkspacesLiveWorkspaceProperties[PROP_BACKGROUND_IMAGE_TYPE]=
		g_param_spec_enum("background-image-type",
							"Background image type",
							"Background image type",
							XFCE_OVERVIEW_WORKSPACES_TYPE_STAGE_BACKGROUND_IMAGE_TYPE,
							XFCE_OVERVIEW_WORKSPACES_STAGE_BACKGROUND_IMAGE_TYPE_NONE,
							G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	XfceOverviewWorkspacesLiveWorkspaceProperties[PROP_SHOW_WORKSPACE_NAME]=
		g_param_spec_boolean("show-workspace-name",
								"Show workspace name",
								"If TRUE the name of workspace should be shown",
								FALSE,
								G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	XfceOverviewWorkspacesLiveWorkspaceProperties[PROP_WORKSPACE_NAME_PADDING]=
		g_param_spec_float("workspace-name-padding",
							"Workspace name padding",
							"Padding of workspace name actor in pixels",
							0.0f, G_MAXFLOAT,
							0.0f,
							G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(gobjectClass, PROP_LAST, XfceOverviewWorkspacesLiveWorkspaceProperties);

	/* Define stylable properties */
	xfce_overview_workspaces_actor_install_stylable_property(actorClass, XfceOverviewWorkspacesLiveWorkspaceProperties[PROP_BACKGROUND_IMAGE_TYPE]);
	xfce_overview_workspaces_actor_install_stylable_property(actorClass, XfceOverviewWorkspacesLiveWorkspaceProperties[PROP_SHOW_WORKSPACE_NAME]);
	xfce_overview_workspaces_actor_install_stylable_property(actorClass, XfceOverviewWorkspacesLiveWorkspaceProperties[PROP_WORKSPACE_NAME_PADDING]);

	/* Define signals */
	XfceOverviewWorkspacesLiveWorkspaceSignals[SIGNAL_CLICKED]=
		g_signal_new("clicked",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST,
						G_STRUCT_OFFSET(XfceOverviewWorkspacesLiveWorkspaceClass, clicked),
						NULL,
						NULL,
						g_cclosure_marshal_VOID__VOID,
						G_TYPE_NONE,
						0);
}

/* Object initialization
 * Create private structure and set up default values
 */
static void xfce_overview_workspaces_live_workspace_init(XfceOverviewWorkspacesLiveWorkspace *self)
{
	XfceOverviewWorkspacesLiveWorkspacePrivate		*priv;

	priv=self->priv=xfce_overview_workspaces_live_workspace_get_instance_private(self);

	/* Set default values */
	priv->windowTracker=xfce_overview_workspaces_core_get_window_tracker(NULL);
	priv->workspace=NULL;
	priv->backgroundType=XFCE_OVERVIEW_WORKSPACES_STAGE_BACKGROUND_IMAGE_TYPE_NONE;
	priv->monitor=NULL;
	priv->showWorkspaceName=FALSE;
	priv->workspaceNamePadding=0.0f;

	/* Set up this actor */
	clutter_actor_set_reactive(CLUTTER_ACTOR(self), TRUE);

	/* Connect signals */
	priv->clickAction=xfce_overview_workspaces_click_action_new();
	clutter_actor_add_action(CLUTTER_ACTOR(self), priv->clickAction);
	g_signal_connect_swapped(priv->clickAction, "clicked", G_CALLBACK(_xfce_overview_workspaces_live_workspace_on_clicked), self);

	/* Create background actors but order of adding background children is important */
	priv->backgroundImageLayer=xfce_overview_workspaces_live_window_simple_new();
	clutter_actor_hide(priv->backgroundImageLayer);
	clutter_actor_add_child(CLUTTER_ACTOR(self), priv->backgroundImageLayer);

	/* Create title actor */
	priv->actorTitle=xfce_overview_workspaces_button_new();
	xfce_overview_workspaces_stylable_add_class(XFCE_OVERVIEW_WORKSPACES_STYLABLE(priv->actorTitle), "title");
	clutter_actor_set_reactive(priv->actorTitle, FALSE);
	clutter_actor_hide(priv->actorTitle);
	clutter_actor_add_child(CLUTTER_ACTOR(self), priv->actorTitle);

	/* Connect signals to window tracker */
	g_signal_connect_swapped(priv->windowTracker,
								"window-opened",
								G_CALLBACK(_xfce_overview_workspaces_live_workspace_on_window_opened),
								self);
	g_signal_connect_swapped(priv->windowTracker,
								"window-closed",
								G_CALLBACK(_xfce_overview_workspaces_live_workspace_on_window_closed),
								self);
	g_signal_connect_swapped(priv->windowTracker,
								"window-geometry-changed",
								G_CALLBACK(_xfce_overview_workspaces_live_workspace_on_window_geometry_changed),
								self);
	g_signal_connect_swapped(priv->windowTracker,
								"window-state-changed",
								G_CALLBACK(_xfce_overview_workspaces_live_workspace_on_window_state_changed),
								self);
	g_signal_connect_swapped(priv->windowTracker,
								"window-workspace-changed",
								G_CALLBACK(_xfce_overview_workspaces_live_workspace_on_window_workspace_changed),
								self);
	g_signal_connect_swapped(priv->windowTracker,
								"window-stacking-changed",
								G_CALLBACK(_xfce_overview_workspaces_live_workspace_on_window_stacking_changed),
								self);
	g_signal_connect_swapped(priv->windowTracker,
								"workspace-name-changed",
								G_CALLBACK(_xfce_overview_workspaces_live_workspace_on_workspace_name_changed),
								self);
}

/* IMPLEMENTATION: Public API */

/* Create new instance */
ClutterActor* xfce_overview_workspaces_live_workspace_new(void)
{
	return(CLUTTER_ACTOR(g_object_new(XFCE_OVERVIEW_WORKSPACES_TYPE_LIVE_WORKSPACE, NULL)));
}

ClutterActor* xfce_overview_workspaces_live_workspace_new_for_workspace(XfceOverviewWorkspacesWindowTrackerWorkspace *inWorkspace)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_WORKSPACE(inWorkspace), NULL);

	return(CLUTTER_ACTOR(g_object_new(XFCE_OVERVIEW_WORKSPACES_TYPE_LIVE_WORKSPACE,
										"workspace", inWorkspace,
										NULL)));
}

/* Get/set workspace to show */
XfceOverviewWorkspacesWindowTrackerWorkspace* xfce_overview_workspaces_live_workspace_get_workspace(XfceOverviewWorkspacesLiveWorkspace *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WORKSPACE(self), NULL);

	return(self->priv->workspace);
}

void xfce_overview_workspaces_live_workspace_set_workspace(XfceOverviewWorkspacesLiveWorkspace *self, XfceOverviewWorkspacesWindowTrackerWorkspace *inWorkspace)
{
	XfceOverviewWorkspacesLiveWorkspacePrivate		*priv;
	XfceOverviewWorkspacesWindowTrackerWindow		*window;
	ClutterActor						*child;
	ClutterActorIter					iter;
	GList								*windows;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WORKSPACE(self));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_WORKSPACE(inWorkspace));

	priv=self->priv;

	/* Only set value if it changes */
	if(inWorkspace==priv->workspace) return;

	/* Release old value */
	if(priv->workspace)
	{
		g_signal_handlers_disconnect_by_data(priv->workspace, self);
		priv->workspace=NULL;
	}

	/* Set new value
	 * Window tracker objects should never be refed or unrefed, so just set new value
	 */
	priv->workspace=inWorkspace;

	/* Destroy all window actors */
	clutter_actor_iter_init(&iter, CLUTTER_ACTOR(self));
	while(clutter_actor_iter_next(&iter, &child))
	{
		/* Get window actor */
		if(!XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WINDOW_SIMPLE(child)) continue;

		/* Check if it is really a window actor by retrieving associated window */
		window=xfce_overview_workspaces_live_window_simple_get_window(XFCE_OVERVIEW_WORKSPACES_LIVE_WINDOW_SIMPLE(child));
		if(!window || !XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_WINDOW(window)) continue;

		/* Do not destroy background image layer */
		if(child==priv->backgroundImageLayer) continue;

		/* Destroy window actor */
		clutter_actor_destroy(child);
	}

	/* Create windows for new workspace in stacked order */
	windows=xfce_overview_workspaces_window_tracker_get_windows_stacked(priv->windowTracker);
	for( ; windows; windows=g_list_next(windows))
	{
		/* Get window */
		window=XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW(windows->data);
		if(!window) continue;

		/* Create window actor only if window is visible */
		if(!_xfce_overview_workspaces_live_workspace_is_visible_window(self, window)) continue;

		/* Create actor for window */
		_xfce_overview_workspaces_live_workspace_create_and_add_window_actor(self, window);
	}

	/* Set name of workspace at title actor */
	_xfce_overview_workspaces_live_workspace_on_workspace_name_changed(self, inWorkspace, NULL);

	/* Notify about property change */
	g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesLiveWorkspaceProperties[PROP_WORKSPACE]);
}

/* Get/set monitor whose window to show only */
XfceOverviewWorkspacesWindowTrackerMonitor* xfce_overview_workspaces_live_workspace_get_monitor(XfceOverviewWorkspacesLiveWorkspace *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WORKSPACE(self), NULL);

	return(self->priv->monitor);
}

void xfce_overview_workspaces_live_workspace_set_monitor(XfceOverviewWorkspacesLiveWorkspace *self, XfceOverviewWorkspacesWindowTrackerMonitor *inMonitor)
{
	XfceOverviewWorkspacesLiveWorkspacePrivate		*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WORKSPACE(self));
	g_return_if_fail(!inMonitor || XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_MONITOR(inMonitor));

	priv=self->priv;

	/* Set value if changed */
	if(priv->monitor!=inMonitor)
	{
		/* Set value */
		if(priv->monitor)
		{
			g_signal_handlers_disconnect_by_data(priv->monitor, self);
			priv->monitor=NULL;
		}

		if(inMonitor)
		{
			priv->monitor=inMonitor;
			g_signal_connect_swapped(priv->monitor,
										"geometry-changed",
										G_CALLBACK(_xfce_overview_workspaces_live_workspace_on_monitor_geometry_changed),
										self);
		}

		/* Force a relayout of this actor to update appearance */
		clutter_actor_queue_relayout(CLUTTER_ACTOR(self));

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesLiveWorkspaceProperties[PROP_MONITOR]);
	}
}

/* Get/set background type */
XfceOverviewWorkspacesStageBackgroundImageType xfce_overview_workspaces_live_workspace_get_background_image_type(XfceOverviewWorkspacesLiveWorkspace *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WORKSPACE(self), XFCE_OVERVIEW_WORKSPACES_STAGE_BACKGROUND_IMAGE_TYPE_NONE);

	return(self->priv->backgroundType);
}

void xfce_overview_workspaces_live_workspace_set_background_image_type(XfceOverviewWorkspacesLiveWorkspace *self, XfceOverviewWorkspacesStageBackgroundImageType inType)
{
	XfceOverviewWorkspacesLiveWorkspacePrivate		*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WORKSPACE(self));
	g_return_if_fail(inType<=XFCE_OVERVIEW_WORKSPACES_STAGE_BACKGROUND_IMAGE_TYPE_DESKTOP);

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
				case XFCE_OVERVIEW_WORKSPACES_STAGE_BACKGROUND_IMAGE_TYPE_DESKTOP:
					{
						XfceOverviewWorkspacesWindowTrackerWindow	*backgroundWindow;

						backgroundWindow=xfce_overview_workspaces_window_tracker_get_root_window(priv->windowTracker);
						if(backgroundWindow)
						{
							xfce_overview_workspaces_live_window_simple_set_window(XFCE_OVERVIEW_WORKSPACES_LIVE_WINDOW_SIMPLE(priv->backgroundImageLayer), backgroundWindow);
							clutter_actor_show(priv->backgroundImageLayer);
							XFCE_OVERVIEW_WORKSPACES_DEBUG(self, ACTOR,
												"Desktop window was found and set up as background image for workspace '%s'",
												xfce_overview_workspaces_window_tracker_workspace_get_name(priv->workspace));
						}
							else
							{
								g_signal_connect_swapped(priv->windowTracker,
															"window-opened",
															G_CALLBACK(_xfce_overview_workspaces_live_workspace_on_desktop_window_opened),
															self);
								XFCE_OVERVIEW_WORKSPACES_DEBUG(self, ACTOR,
													"Desktop window was not found. Setting up signal to get notified when desktop window might be opened for workspace '%s'",
													xfce_overview_workspaces_window_tracker_workspace_get_name(priv->workspace));
							}
					}
					break;

				default:
					xfce_overview_workspaces_live_window_simple_set_window(XFCE_OVERVIEW_WORKSPACES_LIVE_WINDOW_SIMPLE(priv->backgroundImageLayer), NULL);
					clutter_actor_hide(priv->backgroundImageLayer);
					break;
			}
		}

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesLiveWorkspaceProperties[PROP_BACKGROUND_IMAGE_TYPE]);
	}
}

/* Get/set if workspace's name should be shown */
gboolean xfce_overview_workspaces_live_workspace_get_show_workspace_name(XfceOverviewWorkspacesLiveWorkspace *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WORKSPACE(self), TRUE);

	return(self->priv->showWorkspaceName);
}

void xfce_overview_workspaces_live_workspace_set_show_workspace_name(XfceOverviewWorkspacesLiveWorkspace *self, gboolean inIsVisible)
{
	XfceOverviewWorkspacesLiveWorkspacePrivate		*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WORKSPACE(self));

	priv=self->priv;

	/* Set value if changed */
	if(priv->showWorkspaceName!=inIsVisible)
	{
		/* Set value */
		priv->showWorkspaceName=inIsVisible;

		/* Depending on value show or hide workspace name actor */
		if(priv->showWorkspaceName) clutter_actor_show(priv->actorTitle);
			else clutter_actor_hide(priv->actorTitle);

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesLiveWorkspaceProperties[PROP_SHOW_WORKSPACE_NAME]);
	}
}

/* Get/set padding of close button actor */
gfloat xfce_overview_workspaces_live_workspace_get_workspace_name_padding(XfceOverviewWorkspacesLiveWorkspace *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WORKSPACE(self), 0.0f);

	return(self->priv->workspaceNamePadding);
}

void xfce_overview_workspaces_live_workspace_set_workspace_name_padding(XfceOverviewWorkspacesLiveWorkspace *self, gfloat inPadding)
{
	XfceOverviewWorkspacesLiveWorkspacePrivate		*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WORKSPACE(self));
	g_return_if_fail(inPadding>=0.0f);

	priv=self->priv;

	/* Set value if changed */
	if(priv->workspaceNamePadding!=inPadding)
	{
		/* Set value */
		priv->workspaceNamePadding=inPadding;

		/* Enfore a reallocation of this actor */
		clutter_actor_queue_relayout(CLUTTER_ACTOR(self));

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesLiveWorkspaceProperties[PROP_WORKSPACE_NAME_PADDING]);
	}
}
