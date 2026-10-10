/*
 * windows-view: A view showing visible windows
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

#include <libxfce-overview-workspaces/windows-view.h>

#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>

#include <libxfce-overview-workspaces/live-window.h>
#include <libxfce-overview-workspaces/scaled-table-layout.h>
#include <libxfce-overview-workspaces/stage.h>
#include <libxfce-overview-workspaces/core.h>
#include <libxfce-overview-workspaces/view.h>
#include <libxfce-overview-workspaces/drop-action.h>
#include <libxfce-overview-workspaces/window-tracker.h>
#include <libxfce-overview-workspaces/image-content.h>
#include <libxfce-overview-workspaces/utils.h>
#include <libxfce-overview-workspaces/marshal.h>
#include <libxfce-overview-workspaces/enums.h>
#include <libxfce-overview-workspaces/stage-interface.h>
#include <libxfce-overview-workspaces/live-workspace.h>
#include <libxfce-overview-workspaces/debug.h>


/* Define this class in GObject system */
static void _xfce_overview_workspaces_windows_view_focusable_iface_init(XfceOverviewWorkspacesFocusableInterface *iface);

struct _XfceOverviewWorkspacesWindowsViewPrivate
{
	/* Properties related */
	XfceOverviewWorkspacesWindowTrackerWorkspace	*workspace;
	gfloat								spacing;
	gboolean							preventUpscaling;
	gboolean							isScrollEventChangingWorkspace;

	/* Instance related */
	XfceOverviewWorkspacesWindowTracker			*windowTracker;
	ClutterLayoutManager				*layout;
	gpointer							selectedItem;

	GBinding							*settingsScrollEventChangingWorkspaceBinding;
	XfceOverviewWorkspacesStageInterface			*scrollEventChangingWorkspaceStage;
	guint								scrollEventChangingWorkspaceStageSignalID;

	gboolean							isWindowsNumberShown;

	gboolean							filterMonitorWindows;
	gboolean							filterWorkspaceWindows;
	XfceOverviewWorkspacesStageInterface			*currentStage;
	XfceOverviewWorkspacesWindowTrackerMonitor		*currentMonitor;
	guint								currentStageMonitorBindingID;
};

G_DEFINE_TYPE_WITH_CODE(XfceOverviewWorkspacesWindowsView,
						xfce_overview_workspaces_windows_view,
						XFCE_OVERVIEW_WORKSPACES_TYPE_VIEW,
						G_ADD_PRIVATE(XfceOverviewWorkspacesWindowsView)
						G_IMPLEMENT_INTERFACE(XFCE_OVERVIEW_WORKSPACES_TYPE_FOCUSABLE, _xfce_overview_workspaces_windows_view_focusable_iface_init))

/* Properties */
enum
{
	PROP_0,

	PROP_WORKSPACE,
	PROP_SPACING,
	PROP_PREVENT_UPSCALING,
	PROP_SCROLL_EVENT_CHANGES_WORKSPACE,
	PROP_FILTER_MONITOR_WINDOWS,
	PROP_FILTER_WORKSPACE_WINDOWS,

	PROP_LAST
};

static GParamSpec* XfceOverviewWorkspacesWindowsViewProperties[PROP_LAST]={ 0, };

/* Signals */
enum
{
	ACTION_WINDOW_CLOSE,
	ACTION_WINDOWS_SHOW_NUMBERS,
	ACTION_WINDOWS_HIDE_NUMBERS,
	ACTION_WINDOWS_ACTIVATE_WINDOW_ONE,
	ACTION_WINDOWS_ACTIVATE_WINDOW_TWO,
	ACTION_WINDOWS_ACTIVATE_WINDOW_THREE,
	ACTION_WINDOWS_ACTIVATE_WINDOW_FOUR,
	ACTION_WINDOWS_ACTIVATE_WINDOW_FIVE,
	ACTION_WINDOWS_ACTIVATE_WINDOW_SIX,
	ACTION_WINDOWS_ACTIVATE_WINDOW_SEVEN,
	ACTION_WINDOWS_ACTIVATE_WINDOW_EIGHT,
	ACTION_WINDOWS_ACTIVATE_WINDOW_NINE,
	ACTION_WINDOWS_ACTIVATE_WINDOW_TEN,

	SIGNAL_LAST
};

static guint XfceOverviewWorkspacesWindowsViewSignals[SIGNAL_LAST]={ 0, };

/* Forward declaration */
static void _xfce_overview_workspaces_windows_view_recreate_window_actors(XfceOverviewWorkspacesWindowsView *self);
static XfceOverviewWorkspacesLiveWindow* _xfce_overview_workspaces_windows_view_create_actor(XfceOverviewWorkspacesWindowsView *self, XfceOverviewWorkspacesWindowTrackerWindow *inWindow);
static void _xfce_overview_workspaces_windows_view_set_active_workspace(XfceOverviewWorkspacesWindowsView *self, XfceOverviewWorkspacesWindowTrackerWorkspace *inWorkspace);

/* IMPLEMENTATION: Private variables and methods */
#define DEFAULT_VIEW_ICON								"view-fullscreen"
#define DEFAULT_DRAG_HANDLE_SIZE						32.0f

/* Stage interface has changed monitor */
static void _xfce_overview_workspaces_windows_view_update_on_stage_monitor_changed(XfceOverviewWorkspacesWindowsView *self,
																		GParamSpec *inSpec,
																		gpointer inUserData)
{
	XfceOverviewWorkspacesWindowsViewPrivate		*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOWS_VIEW(self));

	priv=self->priv;

	/* Get new reference to new monitor of stage */
	priv->currentMonitor=xfce_overview_workspaces_stage_interface_get_monitor(priv->currentStage);

	/* Recreate all window actors */
	_xfce_overview_workspaces_windows_view_recreate_window_actors(self);
}

/* Update reference to stage interface and monitor where this view is on */
static gboolean _xfce_overview_workspaces_windows_view_update_stage_and_monitor(XfceOverviewWorkspacesWindowsView *self)
{
	XfceOverviewWorkspacesWindowsViewPrivate		*priv;
	XfceOverviewWorkspacesStageInterface			*newStage;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOWS_VIEW(self), FALSE);

	priv=self->priv;

	/* Iterate through parent actors until stage interface is reached */
	newStage=xfce_overview_workspaces_get_stage_of_actor(CLUTTER_ACTOR(self));

	/* If stage did not change return immediately */
	if(newStage==priv->currentStage) return(FALSE);

	/* Release old references to stage and monitor */
	priv->currentMonitor=NULL;

	if(priv->currentStage)
	{
		if(priv->currentStageMonitorBindingID)
		{
			g_signal_handler_disconnect(priv->currentStage, priv->currentStageMonitorBindingID);
			priv->currentStageMonitorBindingID=0;
		}

		priv->currentStage=NULL;
	}

	/* Get new references to new stage and monitor and connect signal to get notified
	 * if stage changes monitor.
	 */
	if(newStage)
	{
		priv->currentStage=newStage;
		priv->currentStageMonitorBindingID=g_signal_connect_swapped(priv->currentStage,
																	"notify::monitor",
																	G_CALLBACK(_xfce_overview_workspaces_windows_view_update_on_stage_monitor_changed),
																	self);

		/* Get new reference to current monitor of new stage */
		priv->currentMonitor=xfce_overview_workspaces_stage_interface_get_monitor(priv->currentStage);
	}

	/* Stage and monitor changed and references were renewed and setup */
	return(TRUE);
}

/* Check if window should be shown */
static gboolean _xfce_overview_workspaces_windows_view_is_visible_window(XfceOverviewWorkspacesWindowsView *self,
																XfceOverviewWorkspacesWindowTrackerWindow *inWindow)
{
	XfceOverviewWorkspacesWindowsViewPrivate			*priv;
	XfceOverviewWorkspacesWindowTrackerWindowState		state;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOWS_VIEW(self), FALSE);
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_WINDOW(inWindow), FALSE);

	priv=self->priv;

	/* Get state of window */
	state=xfce_overview_workspaces_window_tracker_window_get_state(inWindow);

	/* Determine if windows should be shown depending on its state, size and position */
	if(state & XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW_STATE_SKIP_PAGER)
	{
		return(FALSE);
	}

	if(state & XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW_STATE_SKIP_TASKLIST)
	{
		return(FALSE);
	}

	if(xfce_overview_workspaces_window_tracker_window_is_stage(inWindow))
	{
		return(FALSE);
	}

	if(!priv->workspace)
	{
		return(FALSE);
	}

	if(!xfce_overview_workspaces_window_tracker_window_is_visible(inWindow) ||
		(priv->filterWorkspaceWindows && !xfce_overview_workspaces_window_tracker_window_is_on_workspace(inWindow, priv->workspace)))
	{
		return(FALSE);
	}

	if(priv->filterMonitorWindows &&
		xfce_overview_workspaces_window_tracker_supports_multiple_monitors(priv->windowTracker) &&
		(!priv->currentMonitor || !xfce_overview_workspaces_window_tracker_window_is_on_monitor(inWindow, priv->currentMonitor)))
	{
		return(FALSE);
	}

	/* If we get here the window should be shown */
	return(TRUE);
}

/* Find live window actor by window */
static XfceOverviewWorkspacesLiveWindow* _xfce_overview_workspaces_windows_view_find_by_window(XfceOverviewWorkspacesWindowsView *self,
																		XfceOverviewWorkspacesWindowTrackerWindow *inWindow)
{
	XfceOverviewWorkspacesLiveWindow		*liveWindow;
	ClutterActor				*child;
	ClutterActorIter			iter;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOWS_VIEW(self), NULL);
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_WINDOW(inWindow), NULL);

	/* Iterate through list of current actors and find the one for requested window */
	clutter_actor_iter_init(&iter, CLUTTER_ACTOR(self));
	while(clutter_actor_iter_next(&iter, &child))
	{
		if(!XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WINDOW(child)) continue;

		liveWindow=XFCE_OVERVIEW_WORKSPACES_LIVE_WINDOW(child);
		if(xfce_overview_workspaces_live_window_simple_get_window(XFCE_OVERVIEW_WORKSPACES_LIVE_WINDOW_SIMPLE(liveWindow))==inWindow)
		{
			return(liveWindow);
		}
	}

	/* If we get here we did not find the window and we return NULL */
	return(NULL);
}

/* Update window number in close button of each window actor */
static void _xfce_overview_workspaces_windows_view_update_window_number_in_actors(XfceOverviewWorkspacesWindowsView *self)
{
	XfceOverviewWorkspacesWindowsViewPrivate		*priv;
	ClutterActor						*child;
	ClutterActorIter					iter;
	gint								index;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOWS_VIEW(self));

	priv=self->priv;

	/* Iterate through list of current actors and for the first ten actors
	 * change the close button to window number and the rest will still be
	 * close buttons.
	 */
	index=1;
	clutter_actor_iter_init(&iter, CLUTTER_ACTOR(self));
	while(clutter_actor_iter_next(&iter, &child))
	{
		/* Only live window actors can be handled */
		if(!XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WINDOW(child)) continue;

		/* If this is one of the first ten window actors change close button
		 * to window number and set number.
		 */
		if(priv->isWindowsNumberShown && index<=10)
		{
			g_object_set(child, "window-number", index, NULL);
			index++;
		}
			else
			{
				g_object_set(child, "window-number", 0, NULL);
			}
	}
}

/* Recreate all window actors in this view */
static void _xfce_overview_workspaces_windows_view_recreate_window_actors(XfceOverviewWorkspacesWindowsView *self)
{
	XfceOverviewWorkspacesWindowsViewPrivate			*priv;
	GList									*windowsList;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOWS_VIEW(self));

	priv=self->priv;

	/* Remove weak reference at current selection and unset selection */
	if(priv->selectedItem)
	{
		g_object_remove_weak_pointer(G_OBJECT(priv->selectedItem), &priv->selectedItem);
		priv->selectedItem=NULL;
	}

	/* Destroy all actors */
	xfce_overview_workspaces_actor_destroy_all_children(CLUTTER_ACTOR(self));

	/* Create live window actors for new workspace */
	if(priv->workspace!=NULL)
	{
		/* Get list of all windows open */
		windowsList=xfce_overview_workspaces_window_tracker_get_windows(priv->windowTracker);

		/* Iterate through list of window (from last to first), check if window
		 * is visible and create actor for it if it is.
		 */
		windowsList=g_list_last(windowsList);
		while(windowsList)
		{
			XfceOverviewWorkspacesWindowTrackerWindow	*window=XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW(windowsList->data);
			XfceOverviewWorkspacesLiveWindow			*liveWindow;

			/* Check if window is visible on this workspace */
			if(_xfce_overview_workspaces_windows_view_is_visible_window(self, window))
			{
				/* Create actor */
				liveWindow=_xfce_overview_workspaces_windows_view_create_actor(XFCE_OVERVIEW_WORKSPACES_WINDOWS_VIEW(self), window);
				if(liveWindow)
				{
					clutter_actor_add_child(CLUTTER_ACTOR(self), CLUTTER_ACTOR(liveWindow));
					_xfce_overview_workspaces_windows_view_update_window_number_in_actors(self);
				}
			}

			/* Next window */
			windowsList=g_list_previous(windowsList);
		}
	}
}

/* Move window to monitor of this window view */
static void _xfce_overview_workspaces_windows_view_move_live_to_view(XfceOverviewWorkspacesWindowsView *self,
														XfceOverviewWorkspacesLiveWindow *inWindowActor)
{
	XfceOverviewWorkspacesWindowsViewPrivate			*priv;
	XfceOverviewWorkspacesWindowTrackerWindow			*window;
	XfceOverviewWorkspacesWindowTrackerWorkspace		*sourceWorkspace;
	XfceOverviewWorkspacesWindowTrackerWorkspace		*targetWorkspace;
	XfceOverviewWorkspacesWindowTrackerMonitor			*sourceMonitor;
	XfceOverviewWorkspacesWindowTrackerMonitor			*targetMonitor;
	gint									oldWindowX, oldWindowY, oldWindowWidth, oldWindowHeight;
	gint									newWindowX, newWindowY;
	gint									oldMonitorX, oldMonitorY, oldMonitorWidth, oldMonitorHeight;
	gint									newMonitorX, newMonitorY, newMonitorWidth, newMonitorHeight;
	gfloat									relativeX, relativeY;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOWS_VIEW(self));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WINDOW(inWindowActor));

	priv=self->priv;

	/* Get window from window actor */
	window=xfce_overview_workspaces_live_window_simple_get_window(XFCE_OVERVIEW_WORKSPACES_LIVE_WINDOW_SIMPLE(inWindowActor));

	/* Get source and target workspace */
	sourceWorkspace=xfce_overview_workspaces_window_tracker_window_get_workspace(window);
	targetWorkspace=priv->workspace;

	/* Get source and target monitor */
	sourceMonitor=xfce_overview_workspaces_window_tracker_window_get_monitor(window);
	targetMonitor=priv->currentMonitor;

	XFCE_OVERVIEW_WORKSPACES_DEBUG(self, ACTOR,
						"Moving window '%s' from %s-monitor %d to %s-monitor %d and from workspace '%s' (%d) to '%s' (%d)",
						xfce_overview_workspaces_window_tracker_window_get_name(window),
						xfce_overview_workspaces_window_tracker_monitor_is_primary(sourceMonitor) ? "primary" : "secondary",
						xfce_overview_workspaces_window_tracker_monitor_get_number(sourceMonitor),
						xfce_overview_workspaces_window_tracker_monitor_is_primary(targetMonitor) ? "primary" : "secondary",
						xfce_overview_workspaces_window_tracker_monitor_get_number(targetMonitor),
						xfce_overview_workspaces_window_tracker_workspace_get_name(sourceWorkspace),
						xfce_overview_workspaces_window_tracker_workspace_get_number(sourceWorkspace),
						xfce_overview_workspaces_window_tracker_workspace_get_name(targetWorkspace),
						xfce_overview_workspaces_window_tracker_workspace_get_number(targetWorkspace));

	/* Get position and size of window to move */
	xfce_overview_workspaces_window_tracker_window_get_geometry(window,
													&oldWindowX,
													&oldWindowY,
													&oldWindowWidth,
													&oldWindowHeight);

	/* Calculate source x and y coordinate relative to monitor size in percent */
	xfce_overview_workspaces_window_tracker_monitor_get_geometry(sourceMonitor,
													&oldMonitorX,
													&oldMonitorY,
													&oldMonitorWidth,
													&oldMonitorHeight);
	relativeX=((gfloat)(oldWindowX-oldMonitorX)) / ((gfloat)oldMonitorWidth);
	relativeY=((gfloat)(oldWindowY-oldMonitorY)) / ((gfloat)oldMonitorHeight);

	/* Calculate target x and y coordinate from relative size in percent to monitor size */
	xfce_overview_workspaces_window_tracker_monitor_get_geometry(targetMonitor,
													&newMonitorX,
													&newMonitorY,
													&newMonitorWidth,
													&newMonitorHeight);
	newWindowX=newMonitorX+((gint)(relativeX*newMonitorWidth));
	newWindowY=newMonitorY+((gint)(relativeY*newMonitorHeight));

	/* Move window to workspace if they are not the same */
	if(!xfce_overview_workspaces_window_tracker_workspace_is_equal(sourceWorkspace, targetWorkspace))
	{
		xfce_overview_workspaces_window_tracker_window_move_to_workspace(window, targetWorkspace);
		XFCE_OVERVIEW_WORKSPACES_DEBUG(self, ACTOR,
							"Moved window '%s' from workspace '%s' (%d) to '%s' (%d)",
							xfce_overview_workspaces_window_tracker_window_get_name(window),
							xfce_overview_workspaces_window_tracker_workspace_get_name(sourceWorkspace),
							xfce_overview_workspaces_window_tracker_workspace_get_number(sourceWorkspace),
							xfce_overview_workspaces_window_tracker_workspace_get_name(targetWorkspace),
							xfce_overview_workspaces_window_tracker_workspace_get_number(targetWorkspace));
	}

	/* Move window to new position */
	xfce_overview_workspaces_window_tracker_window_move(window, newWindowX, newWindowY);
	XFCE_OVERVIEW_WORKSPACES_DEBUG(self, ACTOR,
						"Moved window '%s' from [%d,%d] at monitor [%d,%d x %d,%d] to [%d,%d] at monitor [%d,%d x %d,%d] (relative x=%.2f, y=%.2f)",
						xfce_overview_workspaces_window_tracker_window_get_name(window),
						oldWindowX, oldWindowY,
						oldMonitorX, oldMonitorY, oldMonitorWidth, oldMonitorHeight,
						newWindowX, newWindowY,
						newMonitorX, newMonitorY, newMonitorWidth, newMonitorHeight,
						relativeX, relativeY);
}

/* Drag of an actor to this view as drop target begins */
static gboolean _xfce_overview_workspaces_windows_view_on_drop_begin(XfceOverviewWorkspacesWindowsView *self,
														XfceOverviewWorkspacesDragAction *inDragAction,
														gpointer inUserData)
{
	ClutterActor					*dragSource;
	ClutterActor					*draggedActor;
	gboolean						canHandle;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOWS_VIEW(self), FALSE);
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_DRAG_ACTION(inDragAction), FALSE);
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_DROP_ACTION(inUserData), FALSE);

	canHandle=FALSE;

	/* Get source where dragging started and actor being dragged */
	dragSource=xfce_overview_workspaces_drag_action_get_source(inDragAction);
	draggedActor=xfce_overview_workspaces_drag_action_get_actor(inDragAction);

	/* We can handle dragged actor if it is a live window and its source
	 * is windows view.
	 */
	if(XFCE_OVERVIEW_WORKSPACES_IS_WINDOWS_VIEW(dragSource) &&
		XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WINDOW(draggedActor))
	{
		canHandle=TRUE;
	}

	/* We can handle dragged actor if it is a live window and its source
	 * is a live workspace
	 */
	if(XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WORKSPACE(dragSource) &&
		XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WINDOW_SIMPLE(draggedActor))
	{
		canHandle=TRUE;
	}

	/* Return TRUE if we can handle dragged actor in this drop target
	 * otherwise FALSE
	 */
	return(canHandle);
}

/* Dragged actor was dropped on this drop target */
static void _xfce_overview_workspaces_windows_view_on_drop_drop(XfceOverviewWorkspacesWindowsView *self,
													XfceOverviewWorkspacesDragAction *inDragAction,
													gfloat inX,
													gfloat inY,
													gpointer inUserData)
{
	XfceOverviewWorkspacesWindowsViewPrivate		*priv;
	ClutterActor						*dragSource;
	ClutterActor						*draggedActor;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOWS_VIEW(self));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_DRAG_ACTION(inDragAction));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_DROP_ACTION(inUserData));

	priv=self->priv;

	/* Get source where dragging started and actor being dragged */
	dragSource=xfce_overview_workspaces_drag_action_get_source(inDragAction);
	draggedActor=xfce_overview_workspaces_drag_action_get_actor(inDragAction);

	/* Handle drop of an window from another windows view */
	if(XFCE_OVERVIEW_WORKSPACES_IS_WINDOWS_VIEW(dragSource) &&
		XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WINDOW(draggedActor))
	{
		XfceOverviewWorkspacesWindowsView			*sourceWindowsView;
		XfceOverviewWorkspacesLiveWindow			*liveWindowActor;

		/* Get source windows view */
		sourceWindowsView=XFCE_OVERVIEW_WORKSPACES_WINDOWS_VIEW(dragSource);

		/* Do nothing if source windows view is the same as target windows view
		 * that means this one.
		 */
		if(sourceWindowsView==self)
		{
			XFCE_OVERVIEW_WORKSPACES_DEBUG(self, ACTOR,
						"Will not handle drop of %s at %s because source and target are the same.",
						G_OBJECT_TYPE_NAME(draggedActor),
						G_OBJECT_TYPE_NAME(dragSource));
			return;
		}

		/* Get dragged window */
		liveWindowActor=XFCE_OVERVIEW_WORKSPACES_LIVE_WINDOW(draggedActor);

		/* Move dragged window to monitor of this window view */
		_xfce_overview_workspaces_windows_view_move_live_to_view(self, liveWindowActor);

		/* Drop action handled so return here */
		return;
	}

	/* Handle drop of an window from a live workspace */
	if(XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WORKSPACE(dragSource) &&
		XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WINDOW_SIMPLE(draggedActor))
	{
		XfceOverviewWorkspacesLiveWorkspace			*sourceLiveWorkspace;
		XfceOverviewWorkspacesWindowTrackerWorkspace*	sourceWorkspace;
		XfceOverviewWorkspacesLiveWindowSimple			*liveWindowActor;
		XfceOverviewWorkspacesWindowTrackerWindow		*window;

		/* Get source live workspace and its workspace */
		sourceLiveWorkspace=XFCE_OVERVIEW_WORKSPACES_LIVE_WORKSPACE(dragSource);
		sourceWorkspace=xfce_overview_workspaces_live_workspace_get_workspace(sourceLiveWorkspace);

		/* Do nothing if source and destination workspaces are the same as nothing
		 * is to do in this case.
		 */
		if(xfce_overview_workspaces_window_tracker_workspace_is_equal(sourceWorkspace, priv->workspace))
		{
			XFCE_OVERVIEW_WORKSPACES_DEBUG(self, ACTOR,
						"Will not handle drop of %s at %s because source and target workspaces are the same.",
						G_OBJECT_TYPE_NAME(draggedActor),
						G_OBJECT_TYPE_NAME(dragSource));
			return;
		}

		/* Get dragged window */
		liveWindowActor=XFCE_OVERVIEW_WORKSPACES_LIVE_WINDOW_SIMPLE(draggedActor);
		window=xfce_overview_workspaces_live_window_simple_get_window(liveWindowActor);

		/* Move dragged window to workspace of this window view */
		xfce_overview_workspaces_window_tracker_window_move_to_workspace(window, priv->workspace);

		/* Drop action handled so return here */
		return;
	}

	/* If we get here we did not handle drop action properly
	 * and this should never happen.
	 */
	g_critical("Did not handle drop action for dragged actor %s of source %s at target %s",
				G_OBJECT_TYPE_NAME(draggedActor),
				G_OBJECT_TYPE_NAME(dragSource),
				G_OBJECT_TYPE_NAME(self));
}

/* A child actor was added to view */
static void _xfce_overview_workspaces_windows_view_on_child_added(ClutterContainer *inContainer,
														ClutterActor *inChild,
														gpointer inUserData)
{
	ClutterActorIter			iter;
	ClutterActor				*child;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOWS_VIEW(inContainer));

	/* Iterate through list of current actors and enable allocation animation */
	clutter_actor_iter_init(&iter, CLUTTER_ACTOR(inContainer));
	while(clutter_actor_iter_next(&iter, &child))
	{
		if(XFCE_OVERVIEW_WORKSPACES_IS_ACTOR(child))
		{
			xfce_overview_workspaces_actor_enable_allocation_animation_once(XFCE_OVERVIEW_WORKSPACES_ACTOR(child));
		}
	}
}

/* A child actor was removed from view */
static void _xfce_overview_workspaces_windows_view_on_child_removed(ClutterContainer *inContainer,
														ClutterActor *inChild,
														gpointer inUserData)
{
	ClutterActorIter			iter;
	ClutterActor				*child;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOWS_VIEW(inContainer));

	/* Iterate through list of current actors and enable allocation animation */
	clutter_actor_iter_init(&iter, CLUTTER_ACTOR(inContainer));
	while(clutter_actor_iter_next(&iter, &child))
	{
		if(XFCE_OVERVIEW_WORKSPACES_IS_ACTOR(child))
		{
			xfce_overview_workspaces_actor_enable_allocation_animation_once(XFCE_OVERVIEW_WORKSPACES_ACTOR(child));
		}
	}
}

/* Active workspace was changed */
static void _xfce_overview_workspaces_windows_view_on_active_workspace_changed(XfceOverviewWorkspacesWindowsView *self,
																	XfceOverviewWorkspacesWindowTrackerWorkspace *inPrevWorkspace,
																	XfceOverviewWorkspacesWindowTrackerWorkspace *inNewWorkspace,
																	gpointer inUserData)
{
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOWS_VIEW(self));

	/* Update window list */
	_xfce_overview_workspaces_windows_view_set_active_workspace(self, inNewWorkspace);
}

/* A window was opened */
static void _xfce_overview_workspaces_windows_view_on_window_opened(XfceOverviewWorkspacesWindowsView *self,
														XfceOverviewWorkspacesWindowTrackerWindow *inWindow,
														gpointer inUserData)
{
	XfceOverviewWorkspacesLiveWindow				*liveWindow;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOWS_VIEW(self));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_WINDOW(inWindow));

	/* Check if parent stage interface changed. If not just add window actor.
	 * Otherwise recreate all window actors for changed stage interface and
	 * monitor.
	 */
	if(!_xfce_overview_workspaces_windows_view_update_stage_and_monitor(self))
	{
		/* Check if window is visible on this workspace */
		if(!_xfce_overview_workspaces_windows_view_is_visible_window(self, inWindow)) return;

		/* Create actor if it does not exist already */
		liveWindow=_xfce_overview_workspaces_windows_view_find_by_window(self, inWindow);
		if(G_LIKELY(!liveWindow))
		{
			liveWindow=_xfce_overview_workspaces_windows_view_create_actor(self, inWindow);
			if(liveWindow)
			{
				clutter_actor_insert_child_below(CLUTTER_ACTOR(self), CLUTTER_ACTOR(liveWindow), NULL);
				_xfce_overview_workspaces_windows_view_update_window_number_in_actors(self);
			}
		}
	}
		else
		{
			/* Recreate all window actors because parent stage interface changed */
			_xfce_overview_workspaces_windows_view_recreate_window_actors(self);
		}
}

/* A window has changed monitor */
static void _xfce_overview_workspaces_windows_view_on_window_monitor_changed(XfceOverviewWorkspacesWindowsView *self,
																XfceOverviewWorkspacesWindowTrackerWindow *inWindow,
																XfceOverviewWorkspacesWindowTrackerMonitor *inOldMonitor,
																XfceOverviewWorkspacesWindowTrackerMonitor *inNewMonitor,
																gpointer inUserData)
{
	XfceOverviewWorkspacesWindowsViewPrivate		*priv;
	XfceOverviewWorkspacesLiveWindow				*liveWindow;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOWS_VIEW(self));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_WINDOW(inWindow));
	g_return_if_fail(inOldMonitor==NULL || XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_MONITOR(inOldMonitor));
	g_return_if_fail(inNewMonitor==NULL || XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_MONITOR(inNewMonitor));

	priv=self->priv;

	/* Check if parent stage interface changed. If not check if window has
	 * moved away from this view and destroy it or it has moved to this view
	 * and create it. Otherwise recreate all window actors for changed stage
	 * interface and monitor.
	 */
	if(!_xfce_overview_workspaces_windows_view_update_stage_and_monitor(self) &&
		G_LIKELY(!inOldMonitor) &&
		G_LIKELY(!inNewMonitor))
	{
		/* Check if window moved away from this view */
		if(priv->currentMonitor==inOldMonitor &&
			!_xfce_overview_workspaces_windows_view_is_visible_window(self, inWindow))
		{
			/* Find live window for window to destroy it */
			liveWindow=_xfce_overview_workspaces_windows_view_find_by_window(self, inWindow);
			if(G_LIKELY(liveWindow))
			{
				/* Destroy actor */
				xfce_overview_workspaces_actor_destroy(CLUTTER_ACTOR(liveWindow));
			}
		}

		/* Check if window moved to this view */
		if(priv->currentMonitor==inNewMonitor &&
			_xfce_overview_workspaces_windows_view_is_visible_window(self, inWindow))
		{
			/* Create actor if it does not exist already */
			liveWindow=_xfce_overview_workspaces_windows_view_find_by_window(self, inWindow);
			if(G_LIKELY(!liveWindow))
			{
				liveWindow=_xfce_overview_workspaces_windows_view_create_actor(self, inWindow);
				if(liveWindow)
				{
					clutter_actor_insert_child_below(CLUTTER_ACTOR(self), CLUTTER_ACTOR(liveWindow), NULL);
					_xfce_overview_workspaces_windows_view_update_window_number_in_actors(self);
				}
			}
		}
	}
		else
		{
			/* Recreate all window actors because parent stage interface changed */
			_xfce_overview_workspaces_windows_view_recreate_window_actors(self);
		}
}

/* A live window was clicked */
static void _xfce_overview_workspaces_windows_view_on_window_clicked(XfceOverviewWorkspacesWindowsView *self,
														gpointer inUserData)
{
	XfceOverviewWorkspacesWindowsViewPrivate		*priv;
	XfceOverviewWorkspacesLiveWindowSimple			*liveWindow;
	XfceOverviewWorkspacesWindowTrackerWindow		*window;
	XfceOverviewWorkspacesWindowTrackerWorkspace	*activeWorkspace;
	XfceOverviewWorkspacesWindowTrackerWorkspace	*windowWorkspace;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOWS_VIEW(self));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WINDOW_SIMPLE(inUserData));

	priv=self->priv;
	liveWindow=XFCE_OVERVIEW_WORKSPACES_LIVE_WINDOW_SIMPLE(inUserData);

	/* Get window to activate */
	window=xfce_overview_workspaces_live_window_simple_get_window(liveWindow);

	/* Move to workspace if window to active is on a different one than the active one */
	activeWorkspace=xfce_overview_workspaces_window_tracker_get_active_workspace(priv->windowTracker);
	if(!xfce_overview_workspaces_window_tracker_window_is_on_workspace(window, activeWorkspace))
	{
		windowWorkspace=xfce_overview_workspaces_window_tracker_window_get_workspace(window);
		xfce_overview_workspaces_window_tracker_workspace_activate(windowWorkspace);
	}

	/* Activate window */
	xfce_overview_workspaces_window_tracker_window_activate(window);

	/* Request core to quit */
	xfce_overview_workspaces_core_quit(NULL);
}

/* The close button of a live window was clicked */
static void _xfce_overview_workspaces_windows_view_on_window_close_clicked(XfceOverviewWorkspacesWindowsView *self,
																gpointer inUserData)
{
	XfceOverviewWorkspacesLiveWindowSimple			*liveWindow;
	XfceOverviewWorkspacesWindowTrackerWindow		*window;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOWS_VIEW(self));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WINDOW_SIMPLE(inUserData));

	liveWindow=XFCE_OVERVIEW_WORKSPACES_LIVE_WINDOW_SIMPLE(inUserData);

	/* Close clicked window */
	window=XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW(xfce_overview_workspaces_live_window_simple_get_window(liveWindow));
	xfce_overview_workspaces_window_tracker_window_close(window);
}

/* A window was moved or resized */
static void _xfce_overview_workspaces_windows_view_on_window_geometry_changed(XfceOverviewWorkspacesWindowsView *self,
																	gpointer inUserData)
{
	XfceOverviewWorkspacesLiveWindow				*liveWindow;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOWS_VIEW(self));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WINDOW(inUserData));

	liveWindow=XFCE_OVERVIEW_WORKSPACES_LIVE_WINDOW(inUserData);

	/* Force a relayout to reflect new size of window */
	clutter_actor_queue_relayout(CLUTTER_ACTOR(liveWindow));
}

/* A window was hidden or shown */
static void _xfce_overview_workspaces_windows_view_on_window_visibility_changed(XfceOverviewWorkspacesWindowsView *self,
																	gboolean inIsVisible,
																	gpointer inUserData)
{
	XfceOverviewWorkspacesLiveWindow				*liveWindow;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOWS_VIEW(self));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WINDOW(inUserData));

	liveWindow=XFCE_OVERVIEW_WORKSPACES_LIVE_WINDOW(inUserData);

	/* If window is shown, show it in window list - otherwise hide it.
	 * We should not destroy the live window actor as the window might
	 * get visible again.
	 */
	if(inIsVisible) clutter_actor_show(CLUTTER_ACTOR(liveWindow));
		else
		{
			/* Hide actor */
			clutter_actor_hide(CLUTTER_ACTOR(liveWindow));
		}
}

/* A window changed workspace or was pinned to all workspaces */
static void _xfce_overview_workspaces_windows_view_on_window_workspace_changed(XfceOverviewWorkspacesWindowsView *self,
																	XfceOverviewWorkspacesWindowTrackerWindow *inWindow,
																	XfceOverviewWorkspacesWindowTrackerWorkspace *inWorkspace,
																	gpointer inUserData)
{
	XfceOverviewWorkspacesWindowsViewPrivate		*priv;
	XfceOverviewWorkspacesLiveWindow				*liveWindow;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOWS_VIEW(self));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_WINDOW(inWindow));
	g_return_if_fail(!inWorkspace || XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_WORKSPACE(inWorkspace));

	priv=self->priv;

	/* Check if parent stage interface changed. If not check if window has
	 * moved away from this view and destroy it or it has moved to this view
	 * and create it. Otherwise recreate all window actors for changed stage
	 * interface and monitor.
	 */
	if(!_xfce_overview_workspaces_windows_view_update_stage_and_monitor(self))
	{
		/* Check if window moved away from this view*/
		if(priv->workspace!=inWorkspace &&
			!_xfce_overview_workspaces_windows_view_is_visible_window(self, inWindow))
		{
			/* Find live window for window to destroy it */
			liveWindow=_xfce_overview_workspaces_windows_view_find_by_window(self, inWindow);
			if(G_LIKELY(liveWindow))
			{
				/* Destroy actor */
				xfce_overview_workspaces_actor_destroy(CLUTTER_ACTOR(liveWindow));
			}
		}

		/* Check if window moved to this view */
		if(priv->workspace==inWorkspace &&
			_xfce_overview_workspaces_windows_view_is_visible_window(self, inWindow))
		{
			/* Create actor if it does not exist already */
			liveWindow=_xfce_overview_workspaces_windows_view_find_by_window(self, inWindow);
			if(G_LIKELY(!liveWindow))
			{
				liveWindow=_xfce_overview_workspaces_windows_view_create_actor(self, inWindow);
				if(liveWindow)
				{
					clutter_actor_insert_child_below(CLUTTER_ACTOR(self), CLUTTER_ACTOR(liveWindow), NULL);
					_xfce_overview_workspaces_windows_view_update_window_number_in_actors(self);
				}
			}
		}
	}
		else
		{
			/* Recreate all window actors because parent stage interface changed */
			_xfce_overview_workspaces_windows_view_recreate_window_actors(self);
		}
}

/* Drag of a live window begins */
static void _xfce_overview_workspaces_windows_view_on_drag_begin(ClutterDragAction *inAction,
													ClutterActor *inActor,
													gfloat inStageX,
													gfloat inStageY,
													ClutterModifierType inModifiers,
													gpointer inUserData)
{
	ClutterActor					*dragHandle;
	ClutterStage					*stage;
	GdkPixbuf						*windowIcon;
	ClutterContent					*image;
	XfceOverviewWorkspacesLiveWindowSimple		*liveWindow;

	g_return_if_fail(CLUTTER_IS_DRAG_ACTION(inAction));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WINDOW_SIMPLE(inActor));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOWS_VIEW(inUserData));

	liveWindow=XFCE_OVERVIEW_WORKSPACES_LIVE_WINDOW_SIMPLE(inActor);

	/* Prevent signal "clicked" from being emitted on dragged icon */
	g_signal_handlers_block_by_func(inActor, _xfce_overview_workspaces_windows_view_on_window_clicked, inUserData);

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
static void _xfce_overview_workspaces_windows_view_on_drag_end(ClutterDragAction *inAction,
													ClutterActor *inActor,
													gfloat inStageX,
													gfloat inStageY,
													ClutterModifierType inModifiers,
													gpointer inUserData)
{
	ClutterActor					*dragHandle;

	g_return_if_fail(CLUTTER_IS_DRAG_ACTION(inAction));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WINDOW(inActor));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOWS_VIEW(inUserData));

	/* Destroy clone of application icon used as drag handle */
	dragHandle=clutter_drag_action_get_drag_handle(inAction);
	if(dragHandle)
	{
		clutter_drag_action_set_drag_handle(inAction, NULL);
		xfce_overview_workspaces_actor_destroy(dragHandle);
	}

	/* Allow signal "clicked" from being emitted again */
	g_signal_handlers_unblock_by_func(inActor, _xfce_overview_workspaces_windows_view_on_window_clicked, inUserData);
}

/* Create actor for wnck-window and connect signals */
static XfceOverviewWorkspacesLiveWindow* _xfce_overview_workspaces_windows_view_create_actor(XfceOverviewWorkspacesWindowsView *self,
																		XfceOverviewWorkspacesWindowTrackerWindow *inWindow)
{
	ClutterActor	*actor;
	ClutterAction	*dragAction;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOWS_VIEW(self), NULL);
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_WINDOW(inWindow), NULL);

	/* Check if window opened is a stage window */
	if(xfce_overview_workspaces_window_tracker_window_is_stage(inWindow))
	{
		XFCE_OVERVIEW_WORKSPACES_DEBUG(self, ACTOR, "Will not create live-window actor for stage window.");
		return(NULL);
	}

	/* Create actor and connect signals */
	actor=xfce_overview_workspaces_live_window_new();
	g_signal_connect_swapped(actor, "clicked", G_CALLBACK(_xfce_overview_workspaces_windows_view_on_window_clicked), self);
	g_signal_connect_swapped(actor, "close", G_CALLBACK(_xfce_overview_workspaces_windows_view_on_window_close_clicked), self);
	g_signal_connect_swapped(actor, "geometry-changed", G_CALLBACK(_xfce_overview_workspaces_windows_view_on_window_geometry_changed), self);
	g_signal_connect_swapped(actor, "visibility-changed", G_CALLBACK(_xfce_overview_workspaces_windows_view_on_window_visibility_changed), self);
	xfce_overview_workspaces_live_window_simple_set_window(XFCE_OVERVIEW_WORKSPACES_LIVE_WINDOW_SIMPLE(actor), inWindow);

	dragAction=xfce_overview_workspaces_drag_action_new_with_source(CLUTTER_ACTOR(self));
	clutter_drag_action_set_drag_threshold(CLUTTER_DRAG_ACTION(dragAction), -1, -1);
	clutter_actor_add_action(actor, dragAction);
	g_signal_connect(dragAction, "drag-begin", G_CALLBACK(_xfce_overview_workspaces_windows_view_on_drag_begin), self);
	g_signal_connect(dragAction, "drag-end", G_CALLBACK(_xfce_overview_workspaces_windows_view_on_drag_end), self);

	return(XFCE_OVERVIEW_WORKSPACES_LIVE_WINDOW(actor));
}

/* Set active screen */
static void _xfce_overview_workspaces_windows_view_set_active_workspace(XfceOverviewWorkspacesWindowsView *self,
															XfceOverviewWorkspacesWindowTrackerWorkspace *inWorkspace)
{
	XfceOverviewWorkspacesWindowsViewPrivate			*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOWS_VIEW(self));
	g_return_if_fail(inWorkspace==NULL || XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_WORKSPACE(inWorkspace));

	priv=XFCE_OVERVIEW_WORKSPACES_WINDOWS_VIEW(self)->priv;

	/* Check if parent stage interface or workspace changed. If both have not
	 * changed do nothing and return immediately.
	 */
	if(!_xfce_overview_workspaces_windows_view_update_stage_and_monitor(self) &&
		inWorkspace==priv->workspace)
	{
		return;
	}

	/* Freeze notification */
	g_object_freeze_notify(G_OBJECT(self));

	/* Set new workspace if changed */
	if(priv->workspace!=inWorkspace)
	{
		/* Set new workspace */
		priv->workspace=inWorkspace;

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesWindowsViewProperties[PROP_WORKSPACE]);
	}

	/* Recreate all window actors */
	_xfce_overview_workspaces_windows_view_recreate_window_actors(self);

	/* Thaw notification */
	g_object_thaw_notify(G_OBJECT(self));
}

/* A scroll event occured in workspace selector (e.g. by mouse-wheel) */
static gboolean _xfce_overview_workspaces_windows_view_on_scroll_event(ClutterActor *inActor,
															ClutterEvent *inEvent,
															gpointer inUserData)
{
	XfceOverviewWorkspacesWindowsView					*self;
	XfceOverviewWorkspacesWindowsViewPrivate			*priv;
	gint									direction;
	gint									workspace;
	gint									maxWorkspace;
	XfceOverviewWorkspacesWindowTrackerWorkspace		*activeWorkspace;
	XfceOverviewWorkspacesWindowTrackerWorkspace		*newWorkspace;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOWS_VIEW(inActor), CLUTTER_EVENT_PROPAGATE);
	g_return_val_if_fail(inEvent, CLUTTER_EVENT_PROPAGATE);

	self=XFCE_OVERVIEW_WORKSPACES_WINDOWS_VIEW(inActor);
	priv=self->priv;

	/* Do not handle event if scroll event of mouse-wheel should not
	 * change workspace. In this case propagate event to get it handled
	 * by next actor in chain.
	 */
	if(!priv->isScrollEventChangingWorkspace) return(CLUTTER_EVENT_PROPAGATE);

	/* Get direction of scroll event */
	switch(clutter_event_get_scroll_direction(inEvent))
	{
		case CLUTTER_SCROLL_UP:
		case CLUTTER_SCROLL_LEFT:
			direction=-1;
			break;

		case CLUTTER_SCROLL_DOWN:
		case CLUTTER_SCROLL_RIGHT:
			direction=1;
			break;

		/* Unhandled directions */
		default:
			XFCE_OVERVIEW_WORKSPACES_DEBUG(self, ACTOR,
								"Cannot handle scroll direction %d in %s",
								clutter_event_get_scroll_direction(inEvent),
								G_OBJECT_TYPE_NAME(self));
			return(CLUTTER_EVENT_PROPAGATE);
	}

	/* Get next workspace in scroll direction */
	activeWorkspace=xfce_overview_workspaces_window_tracker_get_active_workspace(priv->windowTracker);
	maxWorkspace=xfce_overview_workspaces_window_tracker_get_workspaces_count(priv->windowTracker);

	workspace=xfce_overview_workspaces_window_tracker_workspace_get_number(activeWorkspace)+direction;
	if(workspace<0 || workspace>=maxWorkspace) return(CLUTTER_EVENT_STOP);

	/* Activate new workspace */
	newWorkspace=xfce_overview_workspaces_window_tracker_get_workspace_by_number(priv->windowTracker, workspace);
	xfce_overview_workspaces_window_tracker_workspace_activate(newWorkspace);

	return(CLUTTER_EVENT_STOP);
}

/* Set flag if scroll events (e.g. mouse-wheel up or down) should change active workspace
 * and set up scroll event listener or remove an existing one.
 */
static void _xfce_overview_workspaces_windows_view_set_scroll_event_changes_workspace(XfceOverviewWorkspacesWindowsView *self, gboolean inMouseWheelChangingWorkspace)
{
	XfceOverviewWorkspacesWindowsViewPrivate		*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOWS_VIEW(self));

	priv=self->priv;

	/* Set value if changed */
	if(priv->isScrollEventChangingWorkspace!=inMouseWheelChangingWorkspace)
	{
		/* Set value */
		priv->isScrollEventChangingWorkspace=inMouseWheelChangingWorkspace;

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesWindowsViewProperties[PROP_SCROLL_EVENT_CHANGES_WORKSPACE]);
	}
}

/* Set flag if this view should show all windows of all monitors or only the windows
 * which are at the monitor where this view is placed at.
 */
static void _xfce_overview_workspaces_windows_view_set_filter_monitor_windows(XfceOverviewWorkspacesWindowsView *self, gboolean inFilterMonitorWindows)
{
	XfceOverviewWorkspacesWindowsViewPrivate		*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOWS_VIEW(self));

	priv=self->priv;

	/* Set value if changed */
	if(priv->filterMonitorWindows!=inFilterMonitorWindows)
	{
		/* Set value */
		priv->filterMonitorWindows=inFilterMonitorWindows;

		/* Recreate all window actors */
		_xfce_overview_workspaces_windows_view_recreate_window_actors(self);

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesWindowsViewProperties[PROP_FILTER_MONITOR_WINDOWS]);
	}
}

/* Set flag if this view should show all windows of all workspaces or only the windows
 * which are at current workspace.
 */
static void _xfce_overview_workspaces_windows_view_set_filter_workspace_windows(XfceOverviewWorkspacesWindowsView *self, gboolean inFilterWorkspaceWindows)
{
	XfceOverviewWorkspacesWindowsViewPrivate		*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOWS_VIEW(self));

	priv=self->priv;

	/* Set value if changed */
	if(priv->filterWorkspaceWindows!=inFilterWorkspaceWindows)
	{
		/* Set value */
		priv->filterWorkspaceWindows=inFilterWorkspaceWindows;

		/* Recreate all window actors */
		_xfce_overview_workspaces_windows_view_recreate_window_actors(self);

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesWindowsViewProperties[PROP_FILTER_WORKSPACE_WINDOWS]);
	}
}

/* Action signal to close currently selected window was emitted */
static gboolean _xfce_overview_workspaces_windows_view_window_close(XfceOverviewWorkspacesWindowsView *self,
														XfceOverviewWorkspacesFocusable *inSource,
														const gchar *inAction,
														ClutterEvent *inEvent)
{
	XfceOverviewWorkspacesWindowsViewPrivate			*priv;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOWS_VIEW(self), CLUTTER_EVENT_PROPAGATE);
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOWS_VIEW(inSource), CLUTTER_EVENT_PROPAGATE);

	priv=self->priv;

	/* Check if a window is currenly selected */
	if(!priv->selectedItem)
	{
		XFCE_OVERVIEW_WORKSPACES_DEBUG(self, ACTOR, "No window to close is selected.");
		return(CLUTTER_EVENT_STOP);
	}

	/* Close selected window */
	_xfce_overview_workspaces_windows_view_on_window_close_clicked(self, XFCE_OVERVIEW_WORKSPACES_LIVE_WINDOW(priv->selectedItem));

	/* We handled this event */
	return(CLUTTER_EVENT_STOP);
}

/* Action signal to show window numbers was emitted */
static gboolean _xfce_overview_workspaces_windows_view_windows_show_numbers(XfceOverviewWorkspacesWindowsView *self,
																XfceOverviewWorkspacesFocusable *inSource,
																const gchar *inAction,
																ClutterEvent *inEvent)
{
	XfceOverviewWorkspacesWindowsViewPrivate			*priv;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOWS_VIEW(self), CLUTTER_EVENT_PROPAGATE);
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOWS_VIEW(inSource), CLUTTER_EVENT_PROPAGATE);

	priv=self->priv;

	/* If window numbers are already shown do nothing */
	if(priv->isWindowsNumberShown) return(CLUTTER_EVENT_PROPAGATE);

	/* Set flag that window numbers are shown already
	 * to prevent do it twice concurrently.
	 */
	priv->isWindowsNumberShown=TRUE;

	/* Show window numbers */
	_xfce_overview_workspaces_windows_view_update_window_number_in_actors(self);

	/* Action handled but do not prevent further processing */
	return(CLUTTER_EVENT_PROPAGATE);
}

/* Action signal to hide window numbers was emitted */
static gboolean _xfce_overview_workspaces_windows_view_windows_hide_numbers(XfceOverviewWorkspacesWindowsView *self,
																XfceOverviewWorkspacesFocusable *inSource,
																const gchar *inAction,
																ClutterEvent *inEvent)
{
	XfceOverviewWorkspacesWindowsViewPrivate			*priv;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOWS_VIEW(self), CLUTTER_EVENT_PROPAGATE);
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOWS_VIEW(inSource), CLUTTER_EVENT_PROPAGATE);

	priv=self->priv;

	/* If no window numbers are shown do nothing */
	if(!priv->isWindowsNumberShown) return(CLUTTER_EVENT_PROPAGATE);

	/* Set flag that window numbers are hidden already
	 * to prevent do it twice concurrently.
	 */
	priv->isWindowsNumberShown=FALSE;

	/* Hide window numbers */
	_xfce_overview_workspaces_windows_view_update_window_number_in_actors(self);

	/* Action handled but do not prevent further processing */
	return(CLUTTER_EVENT_PROPAGATE);
}

/* Action signal to hide window numbers was emitted */
static gboolean _xfce_overview_workspaces_windows_view_windows_activate_window_by_number(XfceOverviewWorkspacesWindowsView *self,
																				guint inWindowNumber)
{
	ClutterActor						*child;
	ClutterActorIter					iter;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOWS_VIEW(self), CLUTTER_EVENT_PROPAGATE);

	/* Iterate through list of current actors and at each live window actor
	 * check if its window number matches the requested one. If it does
	 * activate this window.
	 */
	clutter_actor_iter_init(&iter, CLUTTER_ACTOR(self));
	while(clutter_actor_iter_next(&iter, &child))
	{
		guint							windowNumber;

		/* Only live window actors can be handled */
		if(!XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WINDOW(child)) continue;

		/* Get window number set at live window actor */
		windowNumber=0;
		g_object_get(child, "window-number", &windowNumber, NULL);

		/* If window number at live window actor matches requested one
		 * activate this window.
		 */
		if(windowNumber==inWindowNumber)
		{
			/* Activate window */
			_xfce_overview_workspaces_windows_view_on_window_clicked(self, XFCE_OVERVIEW_WORKSPACES_LIVE_WINDOW(child));

			/* Action was handled */
			return(CLUTTER_EVENT_STOP);
		}
	}

	/* If we get here the requested window was not found
	 * so this action could not be handled by this actor.
	 */
	return(CLUTTER_EVENT_PROPAGATE);
}

static gboolean _xfce_overview_workspaces_windows_view_windows_activate_window_one(XfceOverviewWorkspacesWindowsView *self,
																		XfceOverviewWorkspacesFocusable *inSource,
																		const gchar *inAction,
																		ClutterEvent *inEvent)
{
	return(_xfce_overview_workspaces_windows_view_windows_activate_window_by_number(self, 1));
}

static gboolean _xfce_overview_workspaces_windows_view_windows_activate_window_two(XfceOverviewWorkspacesWindowsView *self,
																		XfceOverviewWorkspacesFocusable *inSource,
																		const gchar *inAction,
																		ClutterEvent *inEvent)
{
	return(_xfce_overview_workspaces_windows_view_windows_activate_window_by_number(self, 2));
}

static gboolean _xfce_overview_workspaces_windows_view_windows_activate_window_three(XfceOverviewWorkspacesWindowsView *self,
																		XfceOverviewWorkspacesFocusable *inSource,
																		const gchar *inAction,
																		ClutterEvent *inEvent)
{
	return(_xfce_overview_workspaces_windows_view_windows_activate_window_by_number(self, 3));
}

static gboolean _xfce_overview_workspaces_windows_view_windows_activate_window_four(XfceOverviewWorkspacesWindowsView *self,
																		XfceOverviewWorkspacesFocusable *inSource,
																		const gchar *inAction,
																		ClutterEvent *inEvent)
{
	return(_xfce_overview_workspaces_windows_view_windows_activate_window_by_number(self, 4));
}

static gboolean _xfce_overview_workspaces_windows_view_windows_activate_window_five(XfceOverviewWorkspacesWindowsView *self,
																		XfceOverviewWorkspacesFocusable *inSource,
																		const gchar *inAction,
																		ClutterEvent *inEvent)
{
	return(_xfce_overview_workspaces_windows_view_windows_activate_window_by_number(self, 5));
}

static gboolean _xfce_overview_workspaces_windows_view_windows_activate_window_six(XfceOverviewWorkspacesWindowsView *self,
																		XfceOverviewWorkspacesFocusable *inSource,
																		const gchar *inAction,
																		ClutterEvent *inEvent)
{
	return(_xfce_overview_workspaces_windows_view_windows_activate_window_by_number(self, 6));
}

static gboolean _xfce_overview_workspaces_windows_view_windows_activate_window_seven(XfceOverviewWorkspacesWindowsView *self,
																		XfceOverviewWorkspacesFocusable *inSource,
																		const gchar *inAction,
																		ClutterEvent *inEvent)
{
	return(_xfce_overview_workspaces_windows_view_windows_activate_window_by_number(self, 7));
}

static gboolean _xfce_overview_workspaces_windows_view_windows_activate_window_eight(XfceOverviewWorkspacesWindowsView *self,
																		XfceOverviewWorkspacesFocusable *inSource,
																		const gchar *inAction,
																		ClutterEvent *inEvent)
{
	return(_xfce_overview_workspaces_windows_view_windows_activate_window_by_number(self, 8));
}

static gboolean _xfce_overview_workspaces_windows_view_windows_activate_window_nine(XfceOverviewWorkspacesWindowsView *self,
																		XfceOverviewWorkspacesFocusable *inSource,
																		const gchar *inAction,
																		ClutterEvent *inEvent)
{
	return(_xfce_overview_workspaces_windows_view_windows_activate_window_by_number(self, 9));
}

static gboolean _xfce_overview_workspaces_windows_view_windows_activate_window_ten(XfceOverviewWorkspacesWindowsView *self,
																		XfceOverviewWorkspacesFocusable *inSource,
																		const gchar *inAction,
																		ClutterEvent *inEvent)
{
	return(_xfce_overview_workspaces_windows_view_windows_activate_window_by_number(self, 10));
}

/* IMPLEMENTATION: Interface XfceOverviewWorkspacesFocusable */

/* Determine if actor can get the focus */
static gboolean _xfce_overview_workspaces_windows_view_focusable_can_focus(XfceOverviewWorkspacesFocusable *inFocusable)
{
	XfceOverviewWorkspacesWindowsView			*self;
	XfceOverviewWorkspacesFocusableInterface	*selfIface;
	XfceOverviewWorkspacesFocusableInterface	*parentIface;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_FOCUSABLE(inFocusable), FALSE);
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOWS_VIEW(inFocusable), FALSE);

	self=XFCE_OVERVIEW_WORKSPACES_WINDOWS_VIEW(inFocusable);

	/* Call parent class interface function */
	selfIface=XFCE_OVERVIEW_WORKSPACES_FOCUSABLE_GET_IFACE(inFocusable);
	parentIface=g_type_interface_peek_parent(selfIface);

	if(parentIface && parentIface->can_focus)
	{
		if(!parentIface->can_focus(inFocusable)) return(FALSE);
	}

	/* If this view is not enabled it is not focusable */
	if(!xfce_overview_workspaces_view_get_enabled(XFCE_OVERVIEW_WORKSPACES_VIEW(self))) return(FALSE);

	/* If we get here this actor can be focused */
	return(TRUE);
}

/* Actor lost focus */
static void _xfce_overview_workspaces_windows_view_focusable_unset_focus(XfceOverviewWorkspacesFocusable *inFocusable)
{
	XfceOverviewWorkspacesWindowsView			*self;
	XfceOverviewWorkspacesFocusableInterface	*selfIface;
	XfceOverviewWorkspacesFocusableInterface	*parentIface;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_FOCUSABLE(inFocusable));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOWS_VIEW(inFocusable));

	self=XFCE_OVERVIEW_WORKSPACES_WINDOWS_VIEW(inFocusable);

	/* Call parent class interface function */
	selfIface=XFCE_OVERVIEW_WORKSPACES_FOCUSABLE_GET_IFACE(inFocusable);
	parentIface=g_type_interface_peek_parent(selfIface);

	if(parentIface && parentIface->unset_focus)
	{
		parentIface->unset_focus(inFocusable);
	}

	/* Actor lost focus so ensure window numbers are hiding again */
	_xfce_overview_workspaces_windows_view_windows_hide_numbers(self, XFCE_OVERVIEW_WORKSPACES_FOCUSABLE(self), NULL, NULL);
}

/* Determine if this actor supports selection */
static gboolean _xfce_overview_workspaces_windows_view_focusable_supports_selection(XfceOverviewWorkspacesFocusable *inFocusable)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_FOCUSABLE(inFocusable), FALSE);
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOWS_VIEW(inFocusable), FALSE);

	/* This actor supports selection */
	return(TRUE);
}

/* Get current selection */
static ClutterActor* _xfce_overview_workspaces_windows_view_focusable_get_selection(XfceOverviewWorkspacesFocusable *inFocusable)
{
	XfceOverviewWorkspacesWindowsView					*self;
	XfceOverviewWorkspacesWindowsViewPrivate			*priv;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_FOCUSABLE(inFocusable), NULL);
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOWS_VIEW(inFocusable), NULL);

	self=XFCE_OVERVIEW_WORKSPACES_WINDOWS_VIEW(inFocusable);
	priv=self->priv;

	/* Return current selection */
	return(priv->selectedItem);
}

/* Set new selection */
static gboolean _xfce_overview_workspaces_windows_view_focusable_set_selection(XfceOverviewWorkspacesFocusable *inFocusable,
																	ClutterActor *inSelection)
{
	XfceOverviewWorkspacesWindowsView					*self;
	XfceOverviewWorkspacesWindowsViewPrivate			*priv;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_FOCUSABLE(inFocusable), FALSE);
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOWS_VIEW(inFocusable), FALSE);
	g_return_val_if_fail(!inSelection || CLUTTER_IS_ACTOR(inSelection), FALSE);

	self=XFCE_OVERVIEW_WORKSPACES_WINDOWS_VIEW(inFocusable);
	priv=self->priv;

	/* Check that selection is a child of this actor */
	if(inSelection &&
		!clutter_actor_contains(CLUTTER_ACTOR(self), inSelection))
	{
		g_warning("%s is not a child of %s and cannot be selected",
					G_OBJECT_TYPE_NAME(inSelection),
					G_OBJECT_TYPE_NAME(self));

		return(FALSE);
	}

	/* Remove weak reference at current selection */
	if(priv->selectedItem)
	{
		g_object_remove_weak_pointer(G_OBJECT(priv->selectedItem), &priv->selectedItem);
	}

	/* Set new selection */
	priv->selectedItem=inSelection;

	/* Add weak reference at new selection */
	if(priv->selectedItem) g_object_add_weak_pointer(G_OBJECT(priv->selectedItem), &priv->selectedItem);

	/* New selection was set successfully */
	return(TRUE);
}

/* Compute grid index for selection direction (extracted from find_selection) */
static gint _grid_index(gint inRows, gint inCols, gint inCurRow, gint inCurCol, XfceOverviewWorkspacesSelectionTarget inDir, gint inNChildren)
{
	gint					curIndex;
	gint					newIndex;
	gint					curRow;
	gint					curCol;

	g_return_val_if_fail(inCols>0, 0);
	g_return_val_if_fail(inNChildren>=0, 0);

	curIndex=(inCurRow*inCols)+inCurCol;
	curRow=inCurRow;
	curCol=inCurCol;
	newIndex=curIndex;

	/* Find target index */
	switch(inDir)
	{
		case XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_LEFT:
			curCol--;
			if(curCol<0)
			{
				curRow++;
				newIndex=(curRow*inCols)-1;
			}
				else newIndex=curIndex-1;
			newIndex=MIN(newIndex, inNChildren-1);
			break;

		case XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_RIGHT:
			curCol++;
			if(curCol==inCols ||
				curIndex==inNChildren)
			{
				newIndex=(curRow*inCols);
			}
				else newIndex=curIndex+1;
			newIndex=MIN(newIndex, inNChildren-1);
			break;

		case XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_UP:
			curRow--;
			if(curRow<0) curRow=inRows-1;
			newIndex=(curRow*inCols)+curCol;
			newIndex=MIN(newIndex, inNChildren-1);
			break;

		case XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_DOWN:
			curRow++;
			if(curRow>=inRows) curRow=0;
			newIndex=(curRow*inCols)+curCol;
			newIndex=MIN(newIndex, inNChildren-1);
			break;

		case XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_FIRST:
			newIndex=0;
			break;

		case XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_LAST:
			newIndex=MAX(inNChildren-1, 0);
			break;

		case XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_NEXT:
			if(curIndex+1<inNChildren) newIndex=curIndex+1;
				else if(curIndex>0) newIndex=curIndex-1;
				else newIndex=0;
			break;

		case XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_PAGE_LEFT:
			newIndex=(curRow*inCols);
			newIndex=MIN(newIndex, inNChildren-1);
			break;

		case XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_PAGE_RIGHT:
			newIndex=((curRow+1)*inCols)-1;
			newIndex=MIN(newIndex, inNChildren-1);
			break;

		case XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_PAGE_UP:
			newIndex=curCol;
			newIndex=MIN(newIndex, inNChildren-1);
			break;

		case XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_PAGE_DOWN:
			newIndex=((inRows-1)*inCols)+curCol;
			newIndex=MIN(newIndex, inNChildren-1);
			break;

		default:
			newIndex=curIndex;
			break;
	}

	return(newIndex);
}

/* Find requested selection target depending of current selection */
static ClutterActor* _xfce_overview_workspaces_windows_view_focusable_find_selection(XfceOverviewWorkspacesFocusable *inFocusable,
																			ClutterActor *inSelection,
																			XfceOverviewWorkspacesSelectionTarget inDirection)
{
	XfceOverviewWorkspacesWindowsView					*self;
	XfceOverviewWorkspacesWindowsViewPrivate			*priv;
	ClutterActor							*selection;
	ClutterActor							*newSelection;
	gint									numberChildren;
	gint									rows;
	gint									columns;
	gint									currentSelectionIndex;
	gint									currentSelectionRow;
	gint									currentSelectionColumn;
	gint									newSelectionIndex;
	ClutterActorIter						iter;
	ClutterActor							*child;
	gchar									*valueName;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_FOCUSABLE(inFocusable), NULL);
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOWS_VIEW(inFocusable), NULL);
	g_return_val_if_fail(!inSelection || CLUTTER_IS_ACTOR(inSelection), NULL);
	g_return_val_if_fail(inDirection>=0 && inDirection<=XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_NEXT, NULL);

	self=XFCE_OVERVIEW_WORKSPACES_WINDOWS_VIEW(inFocusable);
	priv=self->priv;
	selection=inSelection;
	newSelection=NULL;

	/* If there is nothing selected, select first actor and return */
	if(!inSelection)
	{
		newSelection=clutter_actor_get_first_child(CLUTTER_ACTOR(self));

		valueName=xfce_overview_workspaces_get_enum_value_name(XFCE_OVERVIEW_WORKSPACES_TYPE_SELECTION_TARGET, inDirection);
		XFCE_OVERVIEW_WORKSPACES_DEBUG(self, ACTOR,
							"No selection at %s, so select first child %s for direction %s",
							G_OBJECT_TYPE_NAME(self),
							newSelection ? G_OBJECT_TYPE_NAME(newSelection) : "<nil>",
							valueName);
		g_free(valueName);

		return(newSelection);
	}

	/* Check that selection is a child of this actor otherwise return NULL */
	if(!clutter_actor_contains(CLUTTER_ACTOR(self), inSelection))
	{
		ClutterActor						*parent;

		parent=clutter_actor_get_parent(inSelection);
		g_warning("Cannot lookup selection target at %s because %s is a child of %s",
					G_OBJECT_TYPE_NAME(self),
					G_OBJECT_TYPE_NAME(inSelection),
					parent ? G_OBJECT_TYPE_NAME(parent) : "<nil>");

		return(NULL);
	}

	/* Get number of rows and columns and also get number of children
	 * of layout manager.
	 */
	numberChildren=xfce_overview_workspaces_scaled_table_layout_get_number_children(XFCE_OVERVIEW_WORKSPACES_SCALED_TABLE_LAYOUT(priv->layout));
	rows=xfce_overview_workspaces_scaled_table_layout_get_rows(XFCE_OVERVIEW_WORKSPACES_SCALED_TABLE_LAYOUT(priv->layout));
	columns=xfce_overview_workspaces_scaled_table_layout_get_columns(XFCE_OVERVIEW_WORKSPACES_SCALED_TABLE_LAYOUT(priv->layout));

	/* Get index of current selection */
	currentSelectionIndex=0;
	clutter_actor_iter_init(&iter, CLUTTER_ACTOR(self));
	while(clutter_actor_iter_next(&iter, &child) &&
			child!=inSelection)
	{
		currentSelectionIndex++;
	}

	currentSelectionRow=(currentSelectionIndex / columns);
	currentSelectionColumn=(currentSelectionIndex % columns);

	/* Find target selection via grid index helper (covers all 11 directions) */
	switch(inDirection)
	{
		case XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_LEFT:
		case XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_RIGHT:
		case XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_UP:
		case XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_DOWN:
		case XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_FIRST:
		case XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_LAST:
		case XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_NEXT:
		case XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_PAGE_LEFT:
		case XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_PAGE_RIGHT:
		case XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_PAGE_UP:
		case XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_PAGE_DOWN:
			newSelectionIndex=_grid_index(rows, columns, currentSelectionRow, currentSelectionColumn, inDirection, numberChildren);
			newSelection=clutter_actor_get_child_at_index(CLUTTER_ACTOR(self), newSelectionIndex);
			break;

		default:
			{
				valueName=xfce_overview_workspaces_get_enum_value_name(XFCE_OVERVIEW_WORKSPACES_TYPE_SELECTION_TARGET, inDirection);
				g_critical("Focusable object %s does not handle selection direction of type %s.",
							G_OBJECT_TYPE_NAME(self),
							valueName);
				g_free(valueName);
			}
			break;
	}

	/* If new selection could be found override current selection with it */
	if(newSelection) selection=newSelection;

	/* Return new selection found */
	XFCE_OVERVIEW_WORKSPACES_DEBUG(self, ACTOR,
						"Selecting %s at %s for current selection %s in direction %u",
						selection ? G_OBJECT_TYPE_NAME(selection) : "<nil>",
						G_OBJECT_TYPE_NAME(self),
						inSelection ? G_OBJECT_TYPE_NAME(inSelection) : "<nil>",
						inDirection);

	return(selection);
}

/* Activate selection */
static gboolean _xfce_overview_workspaces_windows_view_focusable_activate_selection(XfceOverviewWorkspacesFocusable *inFocusable,
																		ClutterActor *inSelection)
{
	XfceOverviewWorkspacesWindowsView					*self;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_FOCUSABLE(inFocusable), FALSE);
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOWS_VIEW(inFocusable), FALSE);
	g_return_val_if_fail(CLUTTER_IS_ACTOR(inSelection), FALSE);

	self=XFCE_OVERVIEW_WORKSPACES_WINDOWS_VIEW(inFocusable);

	/* Check that selection is a child of this actor */
	if(!clutter_actor_contains(CLUTTER_ACTOR(self), inSelection))
	{
		ClutterActor						*parent;

		parent=clutter_actor_get_parent(inSelection);
		g_warning("%s is a child of %s and cannot be activated at %s",
					G_OBJECT_TYPE_NAME(inSelection),
					parent ? G_OBJECT_TYPE_NAME(parent) : "<nil>",
					G_OBJECT_TYPE_NAME(self));

		return(FALSE);
	}

	/* Check that child is a live window */
	if(!XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WINDOW(inSelection))
	{
		g_warning("Cannot activate selection of type %s at %s because expecting type %s",
					G_OBJECT_TYPE_NAME(inSelection),
					G_OBJECT_TYPE_NAME(self),
					g_type_name(XFCE_OVERVIEW_WORKSPACES_TYPE_LIVE_WINDOW));

		return(FALSE);
	}

	/* Activate selection means clicking on window */
	_xfce_overview_workspaces_windows_view_on_window_clicked(self, XFCE_OVERVIEW_WORKSPACES_LIVE_WINDOW(inSelection));

	return(TRUE);
}

/* Interface initialization
 * Set up default functions
 */
void _xfce_overview_workspaces_windows_view_focusable_iface_init(XfceOverviewWorkspacesFocusableInterface *iface)
{
	iface->can_focus=_xfce_overview_workspaces_windows_view_focusable_can_focus;
	iface->unset_focus=_xfce_overview_workspaces_windows_view_focusable_unset_focus;

	iface->supports_selection=_xfce_overview_workspaces_windows_view_focusable_supports_selection;
	iface->get_selection=_xfce_overview_workspaces_windows_view_focusable_get_selection;
	iface->set_selection=_xfce_overview_workspaces_windows_view_focusable_set_selection;
	iface->find_selection=_xfce_overview_workspaces_windows_view_focusable_find_selection;
	iface->activate_selection=_xfce_overview_workspaces_windows_view_focusable_activate_selection;
}

/* IMPLEMENTATION: ClutterActor */

/* Actor will be mapped */
static void _xfce_overview_workspaces_windows_view_map(ClutterActor *inActor)
{
	XfceOverviewWorkspacesWindowsView			*self;
	XfceOverviewWorkspacesWindowsViewPrivate	*priv;
	ClutterActorClass				*clutterActorClass;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOWS_VIEW(inActor));

	self=XFCE_OVERVIEW_WORKSPACES_WINDOWS_VIEW(inActor);
	priv=self->priv;

	/* Call parent's virtual function */
	clutterActorClass=CLUTTER_ACTOR_CLASS(xfce_overview_workspaces_windows_view_parent_class);
	if(clutterActorClass->map) clutterActorClass->map(inActor);

	/* Disconnect signal handler if available */
	if(priv->scrollEventChangingWorkspaceStage)
	{
		if(priv->scrollEventChangingWorkspaceStageSignalID)
		{
			g_signal_handler_disconnect(priv->scrollEventChangingWorkspaceStage, priv->scrollEventChangingWorkspaceStageSignalID);
			priv->scrollEventChangingWorkspaceStageSignalID=0;
		}

		priv->scrollEventChangingWorkspaceStage=NULL;
	}

	/* Get stage interface where this actor belongs to and connect
	 * signal handler if found.
	 */
	priv->scrollEventChangingWorkspaceStage=xfce_overview_workspaces_get_stage_of_actor(CLUTTER_ACTOR(self));
	if(priv->scrollEventChangingWorkspaceStage)
	{
		priv->scrollEventChangingWorkspaceStageSignalID=g_signal_connect_swapped(priv->scrollEventChangingWorkspaceStage,
																					"scroll-event",
																					G_CALLBACK(_xfce_overview_workspaces_windows_view_on_scroll_event),
																					self);
	}
}

/* Actor will be unmapped */
static void _xfce_overview_workspaces_windows_view_unmap(ClutterActor *inActor)
{
	XfceOverviewWorkspacesWindowsView			*self;
	XfceOverviewWorkspacesWindowsViewPrivate	*priv;
	ClutterActorClass				*clutterActorClass;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOWS_VIEW(inActor));

	self=XFCE_OVERVIEW_WORKSPACES_WINDOWS_VIEW(inActor);
	priv=self->priv;

	/* Call parent's virtual function */
	clutterActorClass=CLUTTER_ACTOR_CLASS(xfce_overview_workspaces_windows_view_parent_class);
	if(clutterActorClass->unmap) clutterActorClass->unmap(inActor);

	/* Disconnect signal handler if available */
	if(priv->scrollEventChangingWorkspaceStage)
	{
		if(priv->scrollEventChangingWorkspaceStageSignalID)
		{
			g_signal_handler_disconnect(priv->scrollEventChangingWorkspaceStage, priv->scrollEventChangingWorkspaceStageSignalID);
			priv->scrollEventChangingWorkspaceStageSignalID=0;
		}

		priv->scrollEventChangingWorkspaceStage=NULL;
	}
}

/* IMPLEMENTATION: GObject */

/* Dispose this object */
static void _xfce_overview_workspaces_windows_view_dispose(GObject *inObject)
{
	XfceOverviewWorkspacesWindowsView			*self=XFCE_OVERVIEW_WORKSPACES_WINDOWS_VIEW(inObject);
	XfceOverviewWorkspacesWindowsViewPrivate	*priv=XFCE_OVERVIEW_WORKSPACES_WINDOWS_VIEW(self)->priv;

	/* Release allocated resources */
	if(priv->selectedItem)
	{
		g_object_remove_weak_pointer(G_OBJECT(priv->selectedItem), &priv->selectedItem);
		priv->selectedItem=NULL;
	}

	if(priv->scrollEventChangingWorkspaceStage)
	{
		if(priv->scrollEventChangingWorkspaceStageSignalID)
		{
			g_signal_handler_disconnect(priv->scrollEventChangingWorkspaceStage, priv->scrollEventChangingWorkspaceStageSignalID);
			priv->scrollEventChangingWorkspaceStageSignalID=0;
		}

		priv->scrollEventChangingWorkspaceStage=NULL;
	}


	if(priv->settingsScrollEventChangingWorkspaceBinding)
	{
		g_object_unref(priv->settingsScrollEventChangingWorkspaceBinding);
		priv->settingsScrollEventChangingWorkspaceBinding=NULL;
	}

	if(priv->workspace)
	{
		_xfce_overview_workspaces_windows_view_set_active_workspace(self, NULL);
	}

	if(priv->layout)
	{
		priv->layout=NULL;
	}

	if(priv->currentMonitor)
	{
		priv->currentMonitor=NULL;
	}

	if(priv->currentStage)
	{
		if(priv->currentStageMonitorBindingID)
		{
			g_signal_handler_disconnect(priv->currentStage, priv->currentStageMonitorBindingID);
			priv->currentStageMonitorBindingID=0;
		}

		priv->currentStage=NULL;
	}

	if(priv->windowTracker)
	{
		g_signal_handlers_disconnect_by_data(priv->windowTracker, self);
		g_object_unref(priv->windowTracker);
		priv->windowTracker=NULL;
	}

	/* Call parent's class dispose method */
	G_OBJECT_CLASS(xfce_overview_workspaces_windows_view_parent_class)->dispose(inObject);
}

/* Set/get properties */
static void _xfce_overview_workspaces_windows_view_set_property(GObject *inObject,
													guint inPropID,
													const GValue *inValue,
													GParamSpec *inSpec)
{
	XfceOverviewWorkspacesWindowsView		*self=XFCE_OVERVIEW_WORKSPACES_WINDOWS_VIEW(inObject);
	
	switch(inPropID)
	{
		case PROP_WORKSPACE:
			_xfce_overview_workspaces_windows_view_set_active_workspace(self, g_value_get_object(inValue));
			break;

		case PROP_SPACING:
			xfce_overview_workspaces_windows_view_set_spacing(self, g_value_get_float(inValue));
			break;

		case PROP_PREVENT_UPSCALING:
			xfce_overview_workspaces_windows_view_set_prevent_upscaling(self, g_value_get_boolean(inValue));
			break;

		case PROP_SCROLL_EVENT_CHANGES_WORKSPACE:
			_xfce_overview_workspaces_windows_view_set_scroll_event_changes_workspace(self, g_value_get_boolean(inValue));
			break;

		case PROP_FILTER_MONITOR_WINDOWS:
			_xfce_overview_workspaces_windows_view_set_filter_monitor_windows(self, g_value_get_boolean(inValue));
			break;

		case PROP_FILTER_WORKSPACE_WINDOWS:
			_xfce_overview_workspaces_windows_view_set_filter_workspace_windows(self, g_value_get_boolean(inValue));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(inObject, inPropID, inSpec);
			break;
	}
}

static void _xfce_overview_workspaces_windows_view_get_property(GObject *inObject,
													guint inPropID,
													GValue *outValue,
													GParamSpec *inSpec)
{
	XfceOverviewWorkspacesWindowsView			*self=XFCE_OVERVIEW_WORKSPACES_WINDOWS_VIEW(inObject);
	XfceOverviewWorkspacesWindowsViewPrivate	*priv=self->priv;

	switch(inPropID)
	{
		case PROP_WORKSPACE:
			g_value_set_object(outValue, priv->workspace);
			break;

		case PROP_SPACING:
			g_value_set_float(outValue, priv->spacing);
			break;

		case PROP_PREVENT_UPSCALING:
			g_value_set_boolean(outValue, self->priv->preventUpscaling);
			break;

		case PROP_SCROLL_EVENT_CHANGES_WORKSPACE:
			g_value_set_boolean(outValue, self->priv->isScrollEventChangingWorkspace);
			break;

		case PROP_FILTER_MONITOR_WINDOWS:
			g_value_set_boolean(outValue, self->priv->filterMonitorWindows);
			break;

		case PROP_FILTER_WORKSPACE_WINDOWS:
			g_value_set_boolean(outValue, self->priv->filterWorkspaceWindows);
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
static void xfce_overview_workspaces_windows_view_class_init(XfceOverviewWorkspacesWindowsViewClass *klass)
{
	XfceOverviewWorkspacesActorClass	*actorClass=XFCE_OVERVIEW_WORKSPACES_ACTOR_CLASS(klass);
	ClutterActorClass		*clutterActorClass=CLUTTER_ACTOR_CLASS(klass);
	GObjectClass			*gobjectClass=G_OBJECT_CLASS(klass);

	/* Override functions */
	gobjectClass->dispose=_xfce_overview_workspaces_windows_view_dispose;
	gobjectClass->set_property=_xfce_overview_workspaces_windows_view_set_property;
	gobjectClass->get_property=_xfce_overview_workspaces_windows_view_get_property;

	clutterActorClass->map=_xfce_overview_workspaces_windows_view_map;
	clutterActorClass->unmap=_xfce_overview_workspaces_windows_view_unmap;

	klass->window_close=_xfce_overview_workspaces_windows_view_window_close;
	klass->windows_show_numbers=_xfce_overview_workspaces_windows_view_windows_show_numbers;
	klass->windows_hide_numbers=_xfce_overview_workspaces_windows_view_windows_hide_numbers;
	klass->windows_activate_window_one=_xfce_overview_workspaces_windows_view_windows_activate_window_one;
	klass->windows_activate_window_two=_xfce_overview_workspaces_windows_view_windows_activate_window_two;
	klass->windows_activate_window_three=_xfce_overview_workspaces_windows_view_windows_activate_window_three;
	klass->windows_activate_window_four=_xfce_overview_workspaces_windows_view_windows_activate_window_four;
	klass->windows_activate_window_five=_xfce_overview_workspaces_windows_view_windows_activate_window_five;
	klass->windows_activate_window_six=_xfce_overview_workspaces_windows_view_windows_activate_window_six;
	klass->windows_activate_window_seven=_xfce_overview_workspaces_windows_view_windows_activate_window_seven;
	klass->windows_activate_window_eight=_xfce_overview_workspaces_windows_view_windows_activate_window_eight;
	klass->windows_activate_window_nine=_xfce_overview_workspaces_windows_view_windows_activate_window_nine;
	klass->windows_activate_window_ten=_xfce_overview_workspaces_windows_view_windows_activate_window_ten;

	/* Define properties */
	XfceOverviewWorkspacesWindowsViewProperties[PROP_WORKSPACE]=
		g_param_spec_object("workspace",
							"Current workspace",
							"The current workspace whose windows are shown",
							XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_TRACKER_WORKSPACE,
							G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

	XfceOverviewWorkspacesWindowsViewProperties[PROP_SPACING]=
		g_param_spec_float("spacing",
							"Spacing",
							"Spacing between each element in view",
							0.0f, G_MAXFLOAT,
							0.0f,
							G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	XfceOverviewWorkspacesWindowsViewProperties[PROP_PREVENT_UPSCALING]=
		g_param_spec_boolean("prevent-upscaling",
								"Prevent upscaling",
								"Whether this view should prevent upsclaing any window beyond its real size",
								FALSE,
								G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	XfceOverviewWorkspacesWindowsViewProperties[PROP_SCROLL_EVENT_CHANGES_WORKSPACE]=
		g_param_spec_boolean("scroll-event-changes-workspace",
								"Scroll event changes workspace",
								"Whether this view should change active workspace on scroll events",
								FALSE,
								G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	XfceOverviewWorkspacesWindowsViewProperties[PROP_FILTER_MONITOR_WINDOWS]=
		g_param_spec_boolean("filter-monitor-windows",
								"Filter monitor windows",
								"Whether this view should only show windows of monitor where it placed at",
								FALSE,
								G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	XfceOverviewWorkspacesWindowsViewProperties[PROP_FILTER_WORKSPACE_WINDOWS]=
		g_param_spec_boolean("filter-workspace-windows",
								"Filter workspace windows",
								"Whether this view should only show windows of active workspace",
								TRUE,
								G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(gobjectClass, PROP_LAST, XfceOverviewWorkspacesWindowsViewProperties);

	/* Define stylable properties */
	xfce_overview_workspaces_actor_install_stylable_property(actorClass, XfceOverviewWorkspacesWindowsViewProperties[PROP_SPACING]);
	xfce_overview_workspaces_actor_install_stylable_property(actorClass, XfceOverviewWorkspacesWindowsViewProperties[PROP_PREVENT_UPSCALING]);
	xfce_overview_workspaces_actor_install_stylable_property(actorClass, XfceOverviewWorkspacesWindowsViewProperties[PROP_FILTER_MONITOR_WINDOWS]);
	xfce_overview_workspaces_actor_install_stylable_property(actorClass, XfceOverviewWorkspacesWindowsViewProperties[PROP_FILTER_WORKSPACE_WINDOWS]);

	/* Define actions */
	XfceOverviewWorkspacesWindowsViewSignals[ACTION_WINDOW_CLOSE]=
		g_signal_new("window-close",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
						G_STRUCT_OFFSET(XfceOverviewWorkspacesWindowsViewClass, window_close),
						g_signal_accumulator_true_handled,
						NULL,
						_xfce_overview_workspaces_marshal_BOOLEAN__OBJECT_STRING_BOXED,
						G_TYPE_BOOLEAN,
						3,
						XFCE_OVERVIEW_WORKSPACES_TYPE_FOCUSABLE,
						G_TYPE_STRING,
						CLUTTER_TYPE_EVENT);

	XfceOverviewWorkspacesWindowsViewSignals[ACTION_WINDOWS_SHOW_NUMBERS]=
		g_signal_new("windows-show-numbers",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
						G_STRUCT_OFFSET(XfceOverviewWorkspacesWindowsViewClass, windows_show_numbers),
						g_signal_accumulator_true_handled,
						NULL,
						_xfce_overview_workspaces_marshal_BOOLEAN__OBJECT_STRING_BOXED,
						G_TYPE_BOOLEAN,
						3,
						XFCE_OVERVIEW_WORKSPACES_TYPE_FOCUSABLE,
						G_TYPE_STRING,
						CLUTTER_TYPE_EVENT);

	XfceOverviewWorkspacesWindowsViewSignals[ACTION_WINDOWS_HIDE_NUMBERS]=
		g_signal_new("windows-hide-numbers",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
						G_STRUCT_OFFSET(XfceOverviewWorkspacesWindowsViewClass, windows_hide_numbers),
						g_signal_accumulator_true_handled,
						NULL,
						_xfce_overview_workspaces_marshal_BOOLEAN__OBJECT_STRING_BOXED,
						G_TYPE_BOOLEAN,
						3,
						XFCE_OVERVIEW_WORKSPACES_TYPE_FOCUSABLE,
						G_TYPE_STRING,
						CLUTTER_TYPE_EVENT);

	XfceOverviewWorkspacesWindowsViewSignals[ACTION_WINDOWS_ACTIVATE_WINDOW_ONE]=
		g_signal_new("windows-activate-window-one",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
						G_STRUCT_OFFSET(XfceOverviewWorkspacesWindowsViewClass, windows_activate_window_one),
						g_signal_accumulator_true_handled,
						NULL,
						_xfce_overview_workspaces_marshal_BOOLEAN__OBJECT_STRING_BOXED,
						G_TYPE_BOOLEAN,
						3,
						XFCE_OVERVIEW_WORKSPACES_TYPE_FOCUSABLE,
						G_TYPE_STRING,
						CLUTTER_TYPE_EVENT);

	XfceOverviewWorkspacesWindowsViewSignals[ACTION_WINDOWS_ACTIVATE_WINDOW_TWO]=
		g_signal_new("windows-activate-window-two",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
						G_STRUCT_OFFSET(XfceOverviewWorkspacesWindowsViewClass, windows_activate_window_two),
						g_signal_accumulator_true_handled,
						NULL,
						_xfce_overview_workspaces_marshal_BOOLEAN__OBJECT_STRING_BOXED,
						G_TYPE_BOOLEAN,
						3,
						XFCE_OVERVIEW_WORKSPACES_TYPE_FOCUSABLE,
						G_TYPE_STRING,
						CLUTTER_TYPE_EVENT);

	XfceOverviewWorkspacesWindowsViewSignals[ACTION_WINDOWS_ACTIVATE_WINDOW_THREE]=
		g_signal_new("windows-activate-window-three",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
						G_STRUCT_OFFSET(XfceOverviewWorkspacesWindowsViewClass, windows_activate_window_three),
						g_signal_accumulator_true_handled,
						NULL,
						_xfce_overview_workspaces_marshal_BOOLEAN__OBJECT_STRING_BOXED,
						G_TYPE_BOOLEAN,
						3,
						XFCE_OVERVIEW_WORKSPACES_TYPE_FOCUSABLE,
						G_TYPE_STRING,
						CLUTTER_TYPE_EVENT);

	XfceOverviewWorkspacesWindowsViewSignals[ACTION_WINDOWS_ACTIVATE_WINDOW_FOUR]=
		g_signal_new("windows-activate-window-four",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
						G_STRUCT_OFFSET(XfceOverviewWorkspacesWindowsViewClass, windows_activate_window_four),
						g_signal_accumulator_true_handled,
						NULL,
						_xfce_overview_workspaces_marshal_BOOLEAN__OBJECT_STRING_BOXED,
						G_TYPE_BOOLEAN,
						3,
						XFCE_OVERVIEW_WORKSPACES_TYPE_FOCUSABLE,
						G_TYPE_STRING,
						CLUTTER_TYPE_EVENT);

	XfceOverviewWorkspacesWindowsViewSignals[ACTION_WINDOWS_ACTIVATE_WINDOW_FIVE]=
		g_signal_new("windows-activate-window-five",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
						G_STRUCT_OFFSET(XfceOverviewWorkspacesWindowsViewClass, windows_activate_window_five),
						g_signal_accumulator_true_handled,
						NULL,
						_xfce_overview_workspaces_marshal_BOOLEAN__OBJECT_STRING_BOXED,
						G_TYPE_BOOLEAN,
						3,
						XFCE_OVERVIEW_WORKSPACES_TYPE_FOCUSABLE,
						G_TYPE_STRING,
						CLUTTER_TYPE_EVENT);

	XfceOverviewWorkspacesWindowsViewSignals[ACTION_WINDOWS_ACTIVATE_WINDOW_SIX]=
		g_signal_new("windows-activate-window-six",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
						G_STRUCT_OFFSET(XfceOverviewWorkspacesWindowsViewClass, windows_activate_window_six),
						g_signal_accumulator_true_handled,
						NULL,
						_xfce_overview_workspaces_marshal_BOOLEAN__OBJECT_STRING_BOXED,
						G_TYPE_BOOLEAN,
						3,
						XFCE_OVERVIEW_WORKSPACES_TYPE_FOCUSABLE,
						G_TYPE_STRING,
						CLUTTER_TYPE_EVENT);

	XfceOverviewWorkspacesWindowsViewSignals[ACTION_WINDOWS_ACTIVATE_WINDOW_SEVEN]=
		g_signal_new("windows-activate-window-seven",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
						G_STRUCT_OFFSET(XfceOverviewWorkspacesWindowsViewClass, windows_activate_window_seven),
						g_signal_accumulator_true_handled,
						NULL,
						_xfce_overview_workspaces_marshal_BOOLEAN__OBJECT_STRING_BOXED,
						G_TYPE_BOOLEAN,
						3,
						XFCE_OVERVIEW_WORKSPACES_TYPE_FOCUSABLE,
						G_TYPE_STRING,
						CLUTTER_TYPE_EVENT);

	XfceOverviewWorkspacesWindowsViewSignals[ACTION_WINDOWS_ACTIVATE_WINDOW_EIGHT]=
		g_signal_new("windows-activate-window-eight",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
						G_STRUCT_OFFSET(XfceOverviewWorkspacesWindowsViewClass, windows_activate_window_eight),
						g_signal_accumulator_true_handled,
						NULL,
						_xfce_overview_workspaces_marshal_BOOLEAN__OBJECT_STRING_BOXED,
						G_TYPE_BOOLEAN,
						3,
						XFCE_OVERVIEW_WORKSPACES_TYPE_FOCUSABLE,
						G_TYPE_STRING,
						CLUTTER_TYPE_EVENT);

	XfceOverviewWorkspacesWindowsViewSignals[ACTION_WINDOWS_ACTIVATE_WINDOW_NINE]=
		g_signal_new("windows-activate-window-nine",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
						G_STRUCT_OFFSET(XfceOverviewWorkspacesWindowsViewClass, windows_activate_window_nine),
						g_signal_accumulator_true_handled,
						NULL,
						_xfce_overview_workspaces_marshal_BOOLEAN__OBJECT_STRING_BOXED,
						G_TYPE_BOOLEAN,
						3,
						XFCE_OVERVIEW_WORKSPACES_TYPE_FOCUSABLE,
						G_TYPE_STRING,
						CLUTTER_TYPE_EVENT);

	XfceOverviewWorkspacesWindowsViewSignals[ACTION_WINDOWS_ACTIVATE_WINDOW_TEN]=
		g_signal_new("windows-activate-window-ten",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
						G_STRUCT_OFFSET(XfceOverviewWorkspacesWindowsViewClass, windows_activate_window_ten),
						g_signal_accumulator_true_handled,
						NULL,
						_xfce_overview_workspaces_marshal_BOOLEAN__OBJECT_STRING_BOXED,
						G_TYPE_BOOLEAN,
						3,
						XFCE_OVERVIEW_WORKSPACES_TYPE_FOCUSABLE,
						G_TYPE_STRING,
						CLUTTER_TYPE_EVENT);
}

/* Object initialization
 * Create private structure and set up default values
 */
static void xfce_overview_workspaces_windows_view_init(XfceOverviewWorkspacesWindowsView *self)
{
	XfceOverviewWorkspacesWindowsViewPrivate		*priv;
	ClutterAction						*action;
	XfceOverviewWorkspacesWindowTrackerWorkspace	*activeWorkspace;
	XfceOverviewWorkspacesSettings					*settings;

	self->priv=priv=xfce_overview_workspaces_windows_view_get_instance_private(self);

	/* Set up default values */
	priv->windowTracker=xfce_overview_workspaces_core_get_window_tracker(NULL);
	priv->workspace=NULL;
	priv->spacing=0.0f;
	priv->preventUpscaling=FALSE;
	priv->selectedItem=NULL;
	priv->isWindowsNumberShown=FALSE;
	priv->isScrollEventChangingWorkspace=FALSE;
	priv->scrollEventChangingWorkspaceStage=NULL;
	priv->scrollEventChangingWorkspaceStageSignalID=0;
	priv->filterMonitorWindows=FALSE;
	priv->filterWorkspaceWindows=TRUE;
	priv->currentStage=NULL;
	priv->currentMonitor=NULL;
	priv->currentStageMonitorBindingID=0;

	/* Set up view */
	xfce_overview_workspaces_view_set_name(XFCE_OVERVIEW_WORKSPACES_VIEW(self), _("Windows"));
	xfce_overview_workspaces_view_set_icon(XFCE_OVERVIEW_WORKSPACES_VIEW(self), DEFAULT_VIEW_ICON);
	xfce_overview_workspaces_view_set_view_fit_mode(XFCE_OVERVIEW_WORKSPACES_VIEW(self), XFCE_OVERVIEW_WORKSPACES_VIEW_FIT_MODE_BOTH);

	/* Setup actor */
	xfce_overview_workspaces_actor_set_can_focus(XFCE_OVERVIEW_WORKSPACES_ACTOR(self), TRUE);

	priv->layout=xfce_overview_workspaces_scaled_table_layout_new();
	xfce_overview_workspaces_scaled_table_layout_set_relative_scale(XFCE_OVERVIEW_WORKSPACES_SCALED_TABLE_LAYOUT(priv->layout), TRUE);
	xfce_overview_workspaces_scaled_table_layout_set_prevent_upscaling(XFCE_OVERVIEW_WORKSPACES_SCALED_TABLE_LAYOUT(priv->layout), priv->preventUpscaling);
	clutter_actor_set_layout_manager(CLUTTER_ACTOR(self), priv->layout);

	action=xfce_overview_workspaces_drop_action_new();
	clutter_actor_add_action(CLUTTER_ACTOR(self), action);
	g_signal_connect_swapped(action, "begin", G_CALLBACK(_xfce_overview_workspaces_windows_view_on_drop_begin), self);
	g_signal_connect_swapped(action, "drop", G_CALLBACK(_xfce_overview_workspaces_windows_view_on_drop_drop), self);

	/* Bind to settings to react on changes */
	settings=xfce_overview_workspaces_core_get_settings(NULL);
	priv->settingsScrollEventChangingWorkspaceBinding=
		g_object_bind_property(settings,
								"scroll-event-changes-workspace",
								self,
								"scroll-event-changes-workspace",
								G_BINDING_SYNC_CREATE);

	/* Connect signals */
	g_signal_connect(self,
						"actor-added",
						G_CALLBACK(_xfce_overview_workspaces_windows_view_on_child_added),
						NULL);

	g_signal_connect(self,
						"actor-removed",
						G_CALLBACK(_xfce_overview_workspaces_windows_view_on_child_removed),
						NULL);

	g_signal_connect_swapped(priv->windowTracker,
								"active-workspace-changed",
								G_CALLBACK(_xfce_overview_workspaces_windows_view_on_active_workspace_changed),
								self);

	g_signal_connect_swapped(priv->windowTracker,
								"window-workspace-changed",
								G_CALLBACK(_xfce_overview_workspaces_windows_view_on_window_workspace_changed),
								self);

	g_signal_connect_swapped(priv->windowTracker,
								"window-opened",
								G_CALLBACK(_xfce_overview_workspaces_windows_view_on_window_opened),
								self);

	g_signal_connect_swapped(priv->windowTracker,
								"window-monitor-changed",
								G_CALLBACK(_xfce_overview_workspaces_windows_view_on_window_monitor_changed),
								self);

	/* If active workspace is already available then set up this view */
	activeWorkspace=xfce_overview_workspaces_window_tracker_get_active_workspace(priv->windowTracker);
	if(activeWorkspace)
	{
		_xfce_overview_workspaces_windows_view_set_active_workspace(self, activeWorkspace);
	}
}

/* IMPLEMENTATION: Public API */

/* Get/set spacing between elements */
gfloat xfce_overview_workspaces_windows_view_get_spacing(XfceOverviewWorkspacesWindowsView *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOWS_VIEW(self), 0.0f);

	return(self->priv->spacing);
}

void xfce_overview_workspaces_windows_view_set_spacing(XfceOverviewWorkspacesWindowsView *self, const gfloat inSpacing)
{
	XfceOverviewWorkspacesWindowsViewPrivate		*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOWS_VIEW(self));
	g_return_if_fail(inSpacing>=0.0f);

	priv=self->priv;

	/* Set value if changed */
	if(priv->spacing!=inSpacing)
	{
		/* Set value */
		priv->spacing=inSpacing;

		/* Update layout manager */
		if(priv->layout)
		{
			xfce_overview_workspaces_scaled_table_layout_set_spacing(XFCE_OVERVIEW_WORKSPACES_SCALED_TABLE_LAYOUT(priv->layout), priv->spacing);
		}

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesWindowsViewProperties[PROP_SPACING]);
	}
}

/* Get/set if layout manager should prevent to size any child larger than its real size */
gboolean xfce_overview_workspaces_windows_view_get_prevent_upscaling(XfceOverviewWorkspacesWindowsView *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOWS_VIEW(self), FALSE);

	return(self->priv->preventUpscaling);
}

void xfce_overview_workspaces_windows_view_set_prevent_upscaling(XfceOverviewWorkspacesWindowsView *self, gboolean inPreventUpscaling)
{
	XfceOverviewWorkspacesWindowsViewPrivate		*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOWS_VIEW(self));

	priv=self->priv;

	/* Set value if changed */
	if(priv->preventUpscaling!=inPreventUpscaling)
	{
		/* Set value */
		priv->preventUpscaling=inPreventUpscaling;

		/* Update layout manager */
		if(priv->layout)
		{
			xfce_overview_workspaces_scaled_table_layout_set_prevent_upscaling(XFCE_OVERVIEW_WORKSPACES_SCALED_TABLE_LAYOUT(priv->layout), priv->preventUpscaling);
		}

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesWindowsViewProperties[PROP_PREVENT_UPSCALING]);
	}
}
