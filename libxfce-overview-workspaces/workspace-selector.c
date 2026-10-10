/*
 * workspace-selector: Workspace selector box
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

#include <libxfce-overview-workspaces/workspace-selector.h>

#include <glib/gi18n-lib.h>
#include <math.h>

#include <libxfce-overview-workspaces/enums.h>
#include <libxfce-overview-workspaces/window-tracker.h>
#include <libxfce-overview-workspaces/live-workspace.h>
#include <libxfce-overview-workspaces/core.h>
#include <libxfce-overview-workspaces/drop-action.h>
#include <libxfce-overview-workspaces/windows-view.h>
#include <libxfce-overview-workspaces/live-window.h>
#include <libxfce-overview-workspaces/application-button.h>
#include <libxfce-overview-workspaces/utils.h>
#include <libxfce-overview-workspaces/stylable.h>
#include <libxfce-overview-workspaces/focusable.h>
#include <libxfce-overview-workspaces/stage-interface.h>
#include <libxfce-overview-workspaces/debug.h>


/* Define this class in GObject system */
static void _xfce_overview_workspaces_workspace_selector_focusable_iface_init(XfceOverviewWorkspacesFocusableInterface *iface);

struct _XfceOverviewWorkspacesWorkspaceSelectorPrivate
{
	/* Properties related */
	gfloat								spacing;
	ClutterOrientation					orientation;
	gfloat								maxSize;
	gfloat								maxFraction;
	gboolean							usingFraction;
	gboolean							showCurrentMonitorOnly;

	/* Instance related */
	XfceOverviewWorkspacesWindowTracker			*windowTracker;
	XfceOverviewWorkspacesWindowTrackerWorkspace	*activeWorkspace;
};

G_DEFINE_TYPE_WITH_CODE(XfceOverviewWorkspacesWorkspaceSelector,
						xfce_overview_workspaces_workspace_selector,
						XFCE_OVERVIEW_WORKSPACES_TYPE_BACKGROUND,
						G_ADD_PRIVATE(XfceOverviewWorkspacesWorkspaceSelector)
						G_IMPLEMENT_INTERFACE(XFCE_OVERVIEW_WORKSPACES_TYPE_FOCUSABLE, _xfce_overview_workspaces_workspace_selector_focusable_iface_init))

/* Properties */
enum
{
	PROP_0,

	PROP_SPACING,

	PROP_ORIENTATION,

	PROP_MAX_SIZE,
	PROP_MAX_FRACTION,
	PROP_USING_FRACTION,

	PROP_SHOW_CURRENT_MONITOR_ONLY,

	PROP_LAST
};

static GParamSpec* XfceOverviewWorkspacesWorkspaceSelectorProperties[PROP_LAST]={ 0, };

/* IMPLEMENTATION: Private variables and methods */
#define DEFAULT_MAX_SIZE			256.0f
#define DEFAULT_MAX_FRACTION		0.25f
#define DEFAULT_USING_FRACTION		TRUE
#define DEFAULT_ORIENTATION			CLUTTER_ORIENTATION_VERTICAL

/* Get maximum (horizontal or vertical) size either by static size or fraction */
static gfloat _xfce_overview_workspaces_workspace_selector_get_max_size_internal(XfceOverviewWorkspacesWorkspaceSelector *self)
{
	XfceOverviewWorkspacesWorkspaceSelectorPrivate		*priv;
	XfceOverviewWorkspacesStageInterface				*stageInterface;
	gfloat									w, h;
	gfloat									size, fraction;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WORKSPACE_SELECTOR(self), 0.0f);

	priv=self->priv;

	/* Get size of monitor of stage interface where this actor is shown at
	 * to determine maximum size by fraction or to update maximum size or
	 * fraction and send notifications.
	 */
	stageInterface=xfce_overview_workspaces_get_stage_of_actor(CLUTTER_ACTOR(self));
	if(!stageInterface) return(0.0f);

	clutter_actor_get_size(CLUTTER_ACTOR(stageInterface), &w, &h);

	/* If fraction should be used to determine maximum size get width or height
	 * of stage depending on orientation and calculate size by fraction
	 */
	if(priv->usingFraction)
	{
		/* Calculate size by fraction */
		if(priv->orientation==CLUTTER_ORIENTATION_HORIZONTAL) size=h*priv->maxFraction;
			else size=w*priv->maxFraction;

		/* Update maximum size if it has changed */
		if(priv->maxSize!=size)
		{
			priv->maxSize=size;

			/* Notify about property change */
			g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesWorkspaceSelectorProperties[PROP_MAX_SIZE]);
		}

		return(size);
	}

	/* Calculate fraction from size */
	if(priv->orientation==CLUTTER_ORIENTATION_HORIZONTAL) fraction=priv->maxSize/h;
		else fraction=priv->maxSize/w;

	/* Update maximum fraction if it has changed */
	if(priv->maxFraction!=fraction)
	{
		priv->maxFraction=fraction;

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesWorkspaceSelectorProperties[PROP_MAX_FRACTION]);
	}

	/* Otherwise return static maximum size configured */
	return(priv->maxSize);
}

/* Find live workspace actor for native workspace */
static XfceOverviewWorkspacesLiveWorkspace* _xfce_overview_workspaces_workspace_selector_find_actor_for_workspace(XfceOverviewWorkspacesWorkspaceSelector *self,
																							XfceOverviewWorkspacesWindowTrackerWorkspace *inWorkspace)
{
	ClutterActorIter				iter;
	ClutterActor					*child;
	XfceOverviewWorkspacesLiveWorkspace		*liveWorkspace;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WORKSPACE_SELECTOR(self), NULL);
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_WORKSPACE(inWorkspace), NULL);

	/* Iterate through workspace actor and lookup the one handling requesting workspace */
	clutter_actor_iter_init(&iter, CLUTTER_ACTOR(self));
	while(clutter_actor_iter_next(&iter, &child))
	{
		if(XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WORKSPACE(child))
		{
			liveWorkspace=XFCE_OVERVIEW_WORKSPACES_LIVE_WORKSPACE(child);
			if(xfce_overview_workspaces_live_workspace_get_workspace(liveWorkspace)==inWorkspace)
			{
				return(liveWorkspace);
			}
		}
	}

	return(NULL);
}

/* Get height of a child of this actor */
static void _xfce_overview_workspaces_workspace_selector_get_preferred_height_for_child(XfceOverviewWorkspacesWorkspaceSelector *self,
																			ClutterActor *inChild,
																			gfloat inForWidth,
																			gfloat *outMinHeight,
																			gfloat *outNaturalHeight)
{
	XfceOverviewWorkspacesWorkspaceSelectorPrivate		*priv;
	gfloat									minHeight, naturalHeight;
	gfloat									maxSize;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WORKSPACE_SELECTOR(self));

	priv=self->priv;

	/* Set up default values */
	minHeight=naturalHeight=0.0f;

	/* Determine height for horizontal orientation ... */
	if(priv->orientation==CLUTTER_ORIENTATION_HORIZONTAL)
	{
		/* Get height of child */
		clutter_actor_get_preferred_height(inChild, inForWidth, &minHeight, &naturalHeight);

		/* Adjust child's height to maximum height */
		maxSize=_xfce_overview_workspaces_workspace_selector_get_max_size_internal(self)-(2*priv->spacing);
		if(maxSize>=0.0)
		{
			/* Adjust minimum width if it exceed limit */
			if(minHeight>maxSize) minHeight=maxSize;

			/* Adjust natural width if it exceed limit */
			if(naturalHeight>maxSize) naturalHeight=maxSize;
		}
	}
		/* ... otherwise determine height for vertical orientation */
		else
		{
			/* Adjust requested width to maximum width */
			maxSize=_xfce_overview_workspaces_workspace_selector_get_max_size_internal(self)-(2*priv->spacing);
			if(maxSize>=0.0f && inForWidth>maxSize) inForWidth=maxSize;

			/* Get height of child */
			clutter_actor_get_preferred_height(inChild, inForWidth, &minHeight, &naturalHeight);
		}

	/* Store sizes computed */
	if(outMinHeight) *outMinHeight=minHeight;
	if(outNaturalHeight) *outNaturalHeight=naturalHeight;
}

/* Get width of a child of this actor */
static void _xfce_overview_workspaces_workspace_selector_get_preferred_width_for_child(XfceOverviewWorkspacesWorkspaceSelector *self,
																			ClutterActor *inChild,
																			gfloat inForHeight,
																			gfloat *outMinWidth,
																			gfloat *outNaturalWidth)
{
	XfceOverviewWorkspacesWorkspaceSelectorPrivate		*priv;
	gfloat									minWidth, naturalWidth;
	gfloat									maxSize;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WORKSPACE_SELECTOR(self));

	priv=self->priv;

	/* Set up default values */
	minWidth=naturalWidth=0.0f;

	/* Determine width for horizontal orientation ... */
	if(priv->orientation==CLUTTER_ORIENTATION_HORIZONTAL)
	{
		/* Adjust requested height to maximum height */
		maxSize=_xfce_overview_workspaces_workspace_selector_get_max_size_internal(self)-(2*priv->spacing);
		if(maxSize>=0.0f && inForHeight>maxSize) inForHeight=maxSize;

		/* Get width of child */
		clutter_actor_get_preferred_width(inChild, inForHeight, &minWidth, &naturalWidth);
	}
		/* ... otherwise determine height for vertical orientation */
		else
		{
			/* Get width of child */
			clutter_actor_get_preferred_width(inChild, inForHeight, &minWidth, &naturalWidth);

			/* Adjust child's width to maximum width */
			maxSize=_xfce_overview_workspaces_workspace_selector_get_max_size_internal(self)-(2*priv->spacing);
			if(maxSize>=0.0)
			{
				/* Adjust minimum width if it exceed limit */
				if(minWidth>maxSize) minWidth=maxSize;

				/* Adjust natural width if it exceed limit */
				if(naturalWidth>maxSize) naturalWidth=maxSize;
			}
		}

	/* Store sizes computed */
	if(outMinWidth) *outMinWidth=minWidth;
	if(outNaturalWidth) *outNaturalWidth=naturalWidth;
}

/* Drag of an actor to this view as drop target begins */
static gboolean _xfce_overview_workspaces_workspace_selector_on_drop_begin(XfceOverviewWorkspacesLiveWorkspace *self,
																XfceOverviewWorkspacesDragAction *inDragAction,
																gpointer inUserData)
{
	ClutterActor					*dragSource;
	ClutterActor					*draggedActor;
	gboolean						canHandle;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WORKSPACE(self), FALSE);
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

	/* We can handle dragged actor if it is an application button */
	if(XFCE_OVERVIEW_WORKSPACES_IS_APPLICATION_BUTTON(draggedActor))
	{
		canHandle=TRUE;
	}

	/* Return TRUE if we can handle dragged actor in this drop target
	 * otherwise FALSE
	 */
	return(canHandle);
}

/* Dragged actor was dropped on this drop target */
static void _xfce_overview_workspaces_workspace_selector_on_drop_drop(XfceOverviewWorkspacesLiveWorkspace *self,
															XfceOverviewWorkspacesDragAction *inDragAction,
															gfloat inX,
															gfloat inY,
															gpointer inUserData)
{
	ClutterActor						*draggedActor;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WORKSPACE(self));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_DRAG_ACTION(inDragAction));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_DROP_ACTION(inUserData));

	/* Get dragged actor */
	draggedActor=xfce_overview_workspaces_drag_action_get_actor(inDragAction);

	/* Check if dragged actor is a window so move window to workspace */
	if(XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WINDOW_SIMPLE(draggedActor))
	{
		XfceOverviewWorkspacesWindowTrackerWindow	*window;

		/* Get window */
		window=xfce_overview_workspaces_live_window_simple_get_window(XFCE_OVERVIEW_WORKSPACES_LIVE_WINDOW_SIMPLE(draggedActor));
		g_return_if_fail(window);

		/* Move window to workspace */
		xfce_overview_workspaces_window_tracker_window_move_to_workspace(window, xfce_overview_workspaces_live_workspace_get_workspace(self));
	}

	/* Check if dragged actor is a application button so launch app at workspace */
	if(XFCE_OVERVIEW_WORKSPACES_IS_APPLICATION_BUTTON(draggedActor))
	{
		XfceOverviewWorkspacesApplicationButton	*button;
		GAppLaunchContext				*context;

		/* Get application button */
		button=XFCE_OVERVIEW_WORKSPACES_APPLICATION_BUTTON(draggedActor);

		/* Launch application at workspace where application button was dropped */
		context=xfce_overview_workspaces_create_app_context(xfce_overview_workspaces_live_workspace_get_workspace(self));
		xfce_overview_workspaces_application_button_execute(button, context);
		g_object_unref(context);
	}
}

/* A live workspace was clicked */
static void _xfce_overview_workspaces_workspace_selector_on_workspace_clicked(XfceOverviewWorkspacesWorkspaceSelector *self,
																	gpointer inUserData)
{
	XfceOverviewWorkspacesLiveWorkspace	*liveWorkspace;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WORKSPACE_SELECTOR(self));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WORKSPACE(inUserData));

	liveWorkspace=XFCE_OVERVIEW_WORKSPACES_LIVE_WORKSPACE(inUserData);

	/* Active workspace */
	xfce_overview_workspaces_window_tracker_workspace_activate(xfce_overview_workspaces_live_workspace_get_workspace(liveWorkspace));

	/* Request core to quit */
	xfce_overview_workspaces_core_quit(NULL);
}

/* A workspace was destroyed */
static void _xfce_overview_workspaces_workspace_selector_on_workspace_removed(XfceOverviewWorkspacesWorkspaceSelector *self,
																	XfceOverviewWorkspacesWindowTrackerWorkspace *inWorkspace,
																	gpointer inUserData)
{
	XfceOverviewWorkspacesLiveWorkspace		*liveWorkspace;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WORKSPACE_SELECTOR(self));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_WORKSPACE(inWorkspace));

	/* Iterate through children and find workspace to destroy */
	liveWorkspace=_xfce_overview_workspaces_workspace_selector_find_actor_for_workspace(self, inWorkspace);
	if(liveWorkspace) xfce_overview_workspaces_actor_destroy(CLUTTER_ACTOR(liveWorkspace));
}

/* A workspace was created */
static void _xfce_overview_workspaces_workspace_selector_on_workspace_added(XfceOverviewWorkspacesWorkspaceSelector *self,
																XfceOverviewWorkspacesWindowTrackerWorkspace *inWorkspace,
																gpointer inUserData)
{
	XfceOverviewWorkspacesWorkspaceSelectorPrivate		*priv;
	ClutterActor							*actor;
	gint									index;
	ClutterAction							*action;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WORKSPACE_SELECTOR(self));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_WORKSPACE(inWorkspace));

	priv=self->priv;

	/* Get index of workspace for insertion */
	index=xfce_overview_workspaces_window_tracker_workspace_get_number(inWorkspace);

	/* Create new live workspace actor and insert at index */
	actor=xfce_overview_workspaces_live_workspace_new_for_workspace(inWorkspace);
	if(priv->showCurrentMonitorOnly)
	{
		XfceOverviewWorkspacesStageInterface			*stageInterface;
		XfceOverviewWorkspacesWindowTrackerMonitor		*monitor;

		/* Get parent stage interface */
		stageInterface=xfce_overview_workspaces_get_stage_of_actor(CLUTTER_ACTOR(self));

		/* Get monitor of stage interface if available */
		monitor=NULL;
		if(stageInterface) monitor=xfce_overview_workspaces_stage_interface_get_monitor(stageInterface);

		/* Set monitor at newly created live workspace actor */
		xfce_overview_workspaces_live_workspace_set_monitor(XFCE_OVERVIEW_WORKSPACES_LIVE_WORKSPACE(actor), monitor);
	}
	g_signal_connect_swapped(actor, "clicked", G_CALLBACK(_xfce_overview_workspaces_workspace_selector_on_workspace_clicked), self);
	clutter_actor_insert_child_at_index(CLUTTER_ACTOR(self), actor, index);

	action=xfce_overview_workspaces_drop_action_new();
	clutter_actor_add_action(actor, action);
	g_signal_connect_swapped(action, "begin", G_CALLBACK(_xfce_overview_workspaces_workspace_selector_on_drop_begin), actor);
	g_signal_connect_swapped(action, "drop", G_CALLBACK(_xfce_overview_workspaces_workspace_selector_on_drop_drop), actor);
}

/* The active workspace has changed */
static void _xfce_overview_workspaces_workspace_selector_on_active_workspace_changed(XfceOverviewWorkspacesWorkspaceSelector *self,
																		XfceOverviewWorkspacesWindowTrackerWorkspace *inPrevWorkspace,
																		gpointer inUserData)
{
	XfceOverviewWorkspacesWorkspaceSelectorPrivate		*priv;
	XfceOverviewWorkspacesLiveWorkspace				*liveWorkspace;
	XfceOverviewWorkspacesWindowTrackerWorkspace		*workspace;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WORKSPACE_SELECTOR(self));

	priv=self->priv;

	/* Unmark previous workspace */
	if(inPrevWorkspace)
	{
		liveWorkspace=_xfce_overview_workspaces_workspace_selector_find_actor_for_workspace(self, inPrevWorkspace);
		if(liveWorkspace) xfce_overview_workspaces_stylable_remove_pseudo_class(XFCE_OVERVIEW_WORKSPACES_STYLABLE(liveWorkspace), "active");

		priv->activeWorkspace=NULL;
	}

	/* Mark new active workspace */
	workspace=xfce_overview_workspaces_window_tracker_get_active_workspace(priv->windowTracker);
	if(workspace)
	{
		priv->activeWorkspace=workspace;

		liveWorkspace=_xfce_overview_workspaces_workspace_selector_find_actor_for_workspace(self, priv->activeWorkspace);
		if(liveWorkspace) xfce_overview_workspaces_stylable_add_pseudo_class(XFCE_OVERVIEW_WORKSPACES_STYLABLE(liveWorkspace), "active");
	}
}

/* A scroll event occured in workspace selector (e.g. by mouse-wheel) */
static gboolean _xfce_overview_workspaces_workspace_selector_on_scroll_event(ClutterActor *inActor,
																ClutterEvent *inEvent,
																gpointer inUserData)
{
	XfceOverviewWorkspacesWorkspaceSelector			*self;
	XfceOverviewWorkspacesWorkspaceSelectorPrivate		*priv;
	gint									direction;
	gint									currentWorkspace;
	gint									maxWorkspace;
	XfceOverviewWorkspacesWindowTrackerWorkspace		*workspace;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WORKSPACE_SELECTOR(inActor), CLUTTER_EVENT_PROPAGATE);
	g_return_val_if_fail(inEvent, CLUTTER_EVENT_PROPAGATE);

	self=XFCE_OVERVIEW_WORKSPACES_WORKSPACE_SELECTOR(inActor);
	priv=self->priv;

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
	currentWorkspace=xfce_overview_workspaces_window_tracker_workspace_get_number(priv->activeWorkspace);
	maxWorkspace=xfce_overview_workspaces_window_tracker_get_workspaces_count(priv->windowTracker);

	currentWorkspace+=direction;
	if(currentWorkspace<0 || currentWorkspace>=maxWorkspace) return(CLUTTER_EVENT_STOP);

	/* Activate new workspace */
	workspace=xfce_overview_workspaces_window_tracker_get_workspace_by_number(priv->windowTracker, currentWorkspace);
	xfce_overview_workspaces_window_tracker_workspace_activate(workspace);

	return(CLUTTER_EVENT_STOP);
}

/* IMPLEMENTATION: ClutterActor */

/* Get preferred width/height */
static void _xfce_overview_workspaces_workspace_selector_get_preferred_height(ClutterActor *inActor,
																	gfloat inForWidth,
																	gfloat *outMinHeight,
																	gfloat *outNaturalHeight)
{
	XfceOverviewWorkspacesWorkspaceSelector			*self=XFCE_OVERVIEW_WORKSPACES_WORKSPACE_SELECTOR(inActor);
	XfceOverviewWorkspacesWorkspaceSelectorPrivate		*priv=self->priv;
	gfloat									minHeight, naturalHeight;
	ClutterActor							*child;
	ClutterActorIter						iter;
	gfloat									childMinHeight, childNaturalHeight;
	gint									numberChildren;
	gfloat									requestChildSize;
	gfloat									maxSize;

	/* Set up default values */
	minHeight=naturalHeight=0.0f;

	/* Determine width for horizontal orientation ... */
	if(priv->orientation==CLUTTER_ORIENTATION_HORIZONTAL)
	{
		/* Determine maximum size */
		maxSize=_xfce_overview_workspaces_workspace_selector_get_max_size_internal(self)-(2*priv->spacing);

		/* Count visible children */
		numberChildren=0;
		clutter_actor_iter_init(&iter, CLUTTER_ACTOR(inActor));
		while(clutter_actor_iter_next(&iter, &child))
		{
			/* Only check visible children */
			if(clutter_actor_is_visible(child)) numberChildren++;
		}

		/* All workspace actors should have the same size because they
		 * reside on the same screen and share its size. That's why
		 * we can devide the requested size (reduced by spacings) by
		 * the number of visible actors to get the preferred size for
		 * the "requested size" of each actor.
		 */
		requestChildSize=-1.0f;
		if(numberChildren>0 && inForWidth>=0.0f)
		{
			requestChildSize=inForWidth;
			requestChildSize-=(numberChildren+1)*priv->spacing;
			requestChildSize/=numberChildren;
		}

		clutter_actor_iter_init(&iter, CLUTTER_ACTOR(inActor));
		while(clutter_actor_iter_next(&iter, &child))
		{
			/* Only handle visible children */
			if(!clutter_actor_is_visible(child)) continue;

			/* Get child's size */
			_xfce_overview_workspaces_workspace_selector_get_preferred_height_for_child(self,
																			child,
																			requestChildSize,
																			&childMinHeight,
																			&childNaturalHeight);

			/* Adjust size to maximal size allowed */
			childMinHeight=MIN(maxSize, childMinHeight);
			childNaturalHeight=MIN(maxSize, childNaturalHeight);

			/* Determine heights */
			minHeight=MAX(minHeight, childMinHeight);
			naturalHeight=MAX(naturalHeight, childNaturalHeight);
		}

		/* Add spacing between children and spacing as padding */
		if(numberChildren>0)
		{
			minHeight+=2*priv->spacing;
			naturalHeight+=2*priv->spacing;
		}
	}
		/* ... otherwise determine height for vertical orientation */
		else
		{
			/* Reduce requested width by spacing */
			if(inForWidth>=0.0f) inForWidth-=2*priv->spacing;

			/* Calculate children heights */
			numberChildren=0;
			clutter_actor_iter_init(&iter, CLUTTER_ACTOR(inActor));
			while(clutter_actor_iter_next(&iter, &child))
			{
				/* Only handle visible children */
				if(!clutter_actor_is_visible(child)) continue;

				/* Get child's size */
				_xfce_overview_workspaces_workspace_selector_get_preferred_height_for_child(self,
																				child,
																				inForWidth,
																				&childMinHeight,
																				&childNaturalHeight);

				/* Sum heights */
				minHeight+=childMinHeight;
				naturalHeight+=childNaturalHeight;

				/* Count visible children */
				numberChildren++;
			}

			/* Add spacing between children and spacing as padding */
			if(numberChildren>0)
			{
				minHeight+=(numberChildren+1)*priv->spacing;
				naturalHeight+=(numberChildren+1)*priv->spacing;
			}
		}

	/* Store sizes computed */
	if(outMinHeight) *outMinHeight=minHeight;
	if(outNaturalHeight) *outNaturalHeight=naturalHeight;
}

static void _xfce_overview_workspaces_workspace_selector_get_preferred_width(ClutterActor *inActor,
																gfloat inForHeight,
																gfloat *outMinWidth,
																gfloat *outNaturalWidth)
{
	XfceOverviewWorkspacesWorkspaceSelector			*self=XFCE_OVERVIEW_WORKSPACES_WORKSPACE_SELECTOR(inActor);
	XfceOverviewWorkspacesWorkspaceSelectorPrivate		*priv=self->priv;
	gfloat									minWidth, naturalWidth;
	ClutterActor							*child;
	ClutterActorIter						iter;
	gfloat									childMinWidth, childNaturalWidth;
	gint									numberChildren;
	gfloat									requestChildSize;
	gfloat									maxSize;

	/* Set up default values */
	minWidth=naturalWidth=0.0f;

	/* Determine width for horizontal orientation ... */
	if(priv->orientation==CLUTTER_ORIENTATION_HORIZONTAL)
	{
		/* Reduce requested height by spacing */
		if(inForHeight>=0.0f) inForHeight-=2*priv->spacing;

		/* Calculate children widths */
		numberChildren=0;
		clutter_actor_iter_init(&iter, CLUTTER_ACTOR(inActor));
		while(clutter_actor_iter_next(&iter, &child))
		{
			/* Only handle visible children */
			if(!clutter_actor_is_visible(child)) continue;

			/* Get child's size */
			_xfce_overview_workspaces_workspace_selector_get_preferred_width_for_child(self,
																			child,
																			inForHeight,
																			&childMinWidth,
																			&childNaturalWidth);

			/* Sum widths */
			minWidth+=childMinWidth;
			naturalWidth+=childNaturalWidth;

			/* Count visible children */
			numberChildren++;
		}

		/* Add spacing between children and spacing as padding */
		if(numberChildren>0)
		{
			minWidth+=(numberChildren+1)*priv->spacing;
			naturalWidth+=(numberChildren+1)*priv->spacing;
		}
	}
		/* ... otherwise determine height for vertical orientation */
		else
		{
			/* Determine maximum size */
			maxSize=_xfce_overview_workspaces_workspace_selector_get_max_size_internal(self)-(2*priv->spacing);

			/* Count visible children */
			numberChildren=0;
			clutter_actor_iter_init(&iter, CLUTTER_ACTOR(inActor));
			while(clutter_actor_iter_next(&iter, &child))
			{
				/* Only check visible children */
				if(clutter_actor_is_visible(child)) numberChildren++;
			}

			/* All workspace actors should have the same size because they
			 * reside on the same screen and share its size. That's why
			 * we can devide the requested size (reduced by spacings) by
			 * the number of visible actors to get the preferred size for
			 * the "requested size" of each actor.
			 */
			requestChildSize=-1.0f;
			if(numberChildren>0 && inForHeight>=0.0f)
			{
				requestChildSize=inForHeight;
				requestChildSize-=(numberChildren+1)*priv->spacing;
				requestChildSize/=numberChildren;
			}

			clutter_actor_iter_init(&iter, CLUTTER_ACTOR(inActor));
			while(clutter_actor_iter_next(&iter, &child))
			{
				/* Only handle visible children */
				if(!clutter_actor_is_visible(child)) continue;

				/* Get child's size */
				_xfce_overview_workspaces_workspace_selector_get_preferred_width_for_child(self,
																				child,
																				requestChildSize,
																				&childMinWidth,
																				&childNaturalWidth);

				/* Adjust size to maximal size allowed */
				childMinWidth=MIN(maxSize, childMinWidth);
				childNaturalWidth=MIN(maxSize, childNaturalWidth);

				/* Determine widths */
				minWidth=MAX(minWidth, childMinWidth);
				naturalWidth=MAX(naturalWidth, childNaturalWidth);
			}

			/* Add spacing between children and spacing as padding */
			if(numberChildren>0)
			{
				minWidth+=2*priv->spacing;
				naturalWidth+=2*priv->spacing;
			}
		}

	/* Store sizes computed */
	if(outMinWidth) *outMinWidth=minWidth;
	if(outNaturalWidth) *outNaturalWidth=naturalWidth;
}

/* Allocate position and size of actor and its children */
static void _xfce_overview_workspaces_workspace_selector_allocate(ClutterActor *inActor,
														const ClutterActorBox *inBox,
														ClutterAllocationFlags inFlags)
{
	XfceOverviewWorkspacesWorkspaceSelector			*self=XFCE_OVERVIEW_WORKSPACES_WORKSPACE_SELECTOR(inActor);
	XfceOverviewWorkspacesWorkspaceSelectorPrivate		*priv=self->priv;
	gfloat									availableWidth, availableHeight;
	gfloat									childWidth, childHeight;
	ClutterActor							*child;
	ClutterActorIter						iter;
	ClutterActorBox							childAllocation={ 0, };
	gfloat									totalNeeded;
	gfloat									scale;
	gint									numberChildren;
	gfloat									baseChildSize;
	gfloat									tmpSize;

	/* Chain up to store the allocation of the actor */
	CLUTTER_ACTOR_CLASS(xfce_overview_workspaces_workspace_selector_parent_class)->allocate(inActor, inBox, inFlags);

	/* Get available size */
	clutter_actor_box_get_size(inBox, &availableWidth, &availableHeight);

	/* Pre-pass: compute total needed space (sum of preferred sizes + spacings)
	 * and a proportional shrink factor when it overflows the available box.
	 * This keeps max-fraction/clamp logic untouched and avoids any scrollbar.
	 */
	scale=1.0f;
	totalNeeded=0.0f;
	numberChildren=0;
	if(priv->orientation==CLUTTER_ORIENTATION_HORIZONTAL)
	{
		baseChildSize=availableHeight-(2*priv->spacing);
		baseChildSize=MAX(0.0f, MIN(baseChildSize, availableHeight-(2*priv->spacing)));

		clutter_actor_iter_init(&iter, CLUTTER_ACTOR(inActor));
		while(clutter_actor_iter_next(&iter, &child))
		{
			if(!clutter_actor_is_visible(child)) continue;

			tmpSize=0.0f;
			clutter_actor_get_preferred_width(child, baseChildSize, NULL, &tmpSize);
			tmpSize=MAX(0.0f, MIN(tmpSize, availableWidth));
			totalNeeded+=tmpSize;
			numberChildren++;
		}

		if(numberChildren>0) totalNeeded+=(numberChildren+1)*priv->spacing;
			else totalNeeded=0.0f;

		if(totalNeeded>availableWidth && totalNeeded>0.0f) scale=availableWidth/totalNeeded;
	}
		else
		{
			baseChildSize=availableWidth-(2*priv->spacing);
			baseChildSize=MAX(0.0f, MIN(baseChildSize, availableWidth-(2*priv->spacing)));

			clutter_actor_iter_init(&iter, CLUTTER_ACTOR(inActor));
			while(clutter_actor_iter_next(&iter, &child))
			{
				if(!clutter_actor_is_visible(child)) continue;

				tmpSize=0.0f;
				clutter_actor_get_preferred_height(child, baseChildSize, NULL, &tmpSize);
				tmpSize=MAX(0.0f, MIN(tmpSize, availableHeight));
				totalNeeded+=tmpSize;
				numberChildren++;
			}

			if(numberChildren>0) totalNeeded+=(numberChildren+1)*priv->spacing;
				else totalNeeded=0.0f;

			if(totalNeeded>availableHeight && totalNeeded>0.0f) scale=availableHeight/totalNeeded;
		}

	/* Calculate new position and size of visible children.
	 * Each child gets exactly its slice (x1 starts at spacing, y chained)
	 * and the frame only shrinks proportionally: aspect inside is preserved
	 * because the other dimension is re-derived via get_preferred_* so the
	 * internal geometry transform is never distorted. Never allocate beyond
	 * the box: clamp each child to the remaining space.
	 */
	childAllocation.x1=childAllocation.y1=priv->spacing;
	clutter_actor_iter_init(&iter, CLUTTER_ACTOR(inActor));
	while(clutter_actor_iter_next(&iter, &child))
	{
		/* Is child visible? */
		if(!clutter_actor_is_visible(child)) continue;

		/* Calculate new position and size of child */
		if(priv->orientation==CLUTTER_ORIENTATION_HORIZONTAL)
		{
			childHeight=availableHeight-(2*priv->spacing);
			childHeight=MAX(0.0f, MIN(childHeight, availableHeight-(2*priv->spacing)));
			clutter_actor_get_preferred_width(child, childHeight, NULL, &childWidth);
			childWidth=MAX(0.0f, MIN(childWidth, availableWidth));

			/* Proportional shrink when overflowing */
			childWidth*=scale;
			if(scale<1.0f)
			{
				gfloat aspectHeight=0.0f;

				/* Keep aspect: derive height from shrunk width, only shrink frame */
				clutter_actor_get_preferred_height(child, childWidth, NULL, &aspectHeight);
				aspectHeight=MAX(0.0f, MIN(aspectHeight, availableHeight-(2*priv->spacing)));
				childHeight=MIN(childHeight, aspectHeight);
			}

			/* Never allocate beyond the box: clamp to remaining width */
			{
				gfloat remainingWidth=availableWidth-childAllocation.x1-priv->spacing;
				remainingWidth=MAX(0.0f, remainingWidth);
				childWidth=MIN(childWidth, remainingWidth);
				childHeight=MAX(0.0f, MIN(childHeight, availableHeight-(2*priv->spacing)));
			}

			childAllocation.y1=ceil(MAX(((availableHeight-childHeight))/2.0f, priv->spacing));
			childAllocation.y2=floor(MIN(childAllocation.y1+childHeight, availableHeight-priv->spacing));
			childAllocation.y2=MAX(childAllocation.y2, childAllocation.y1);
			childAllocation.x2=floor(MIN(childAllocation.x1+childWidth, availableWidth-priv->spacing));
			childAllocation.x2=MAX(childAllocation.x2, childAllocation.x1);
		}
			else
			{
				childWidth=availableWidth-(2*priv->spacing);
				childWidth=MAX(0.0f, MIN(childWidth, availableWidth-(2*priv->spacing)));
				clutter_actor_get_preferred_height(child, childWidth, NULL, &childHeight);
				childHeight=MAX(0.0f, MIN(childHeight, availableHeight));

				/* Proportional shrink when overflowing */
				childHeight*=scale;
				if(scale<1.0f)
				{
					gfloat aspectWidth=0.0f;

					/* Keep aspect: derive width from shrunk height, only shrink frame */
					clutter_actor_get_preferred_width(child, childHeight, NULL, &aspectWidth);
					aspectWidth=MAX(0.0f, MIN(aspectWidth, availableWidth-(2*priv->spacing)));
					childWidth=MIN(childWidth, aspectWidth);
				}

				/* Never allocate beyond the box: clamp to remaining height */
				{
					gfloat remainingHeight=availableHeight-childAllocation.y1-priv->spacing;
					remainingHeight=MAX(0.0f, remainingHeight);
					childHeight=MIN(childHeight, remainingHeight);
					childWidth=MAX(0.0f, MIN(childWidth, availableWidth-(2*priv->spacing)));
				}

				childAllocation.x1=ceil(MAX(((availableWidth-childWidth))/2.0f, priv->spacing));
				childAllocation.x2=floor(MIN(childAllocation.x1+childWidth, availableWidth-priv->spacing));
				childAllocation.x2=MAX(childAllocation.x2, childAllocation.x1);
				childAllocation.y2=floor(childAllocation.y1+childHeight);
			}

		clutter_actor_allocate(child, &childAllocation, inFlags);

		/* Set up for next child: y chained (vertical) / x chained (horizontal) */
		if(priv->orientation==CLUTTER_ORIENTATION_HORIZONTAL) childAllocation.x1=floor(childAllocation.x1+childWidth+priv->spacing);
			else childAllocation.y1=floor(childAllocation.y1+childHeight+priv->spacing);
	}
}

/* IMPLEMENTATION: Interface XfceOverviewWorkspacesFocusable */

/* Determine if this actor supports selection */
static gboolean _xfce_overview_workspaces_workspace_selector_focusable_supports_selection(XfceOverviewWorkspacesFocusable *inFocusable)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_FOCUSABLE(inFocusable), FALSE);
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WORKSPACE_SELECTOR(inFocusable), FALSE);

	/* This actor supports selection */
	return(TRUE);
}

/* Get current selection */
static ClutterActor* _xfce_overview_workspaces_workspace_selector_focusable_get_selection(XfceOverviewWorkspacesFocusable *inFocusable)
{
	XfceOverviewWorkspacesWorkspaceSelector			*self;
	XfceOverviewWorkspacesWorkspaceSelectorPrivate		*priv;
	XfceOverviewWorkspacesLiveWorkspace				*actor;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_FOCUSABLE(inFocusable), NULL);
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WORKSPACE_SELECTOR(inFocusable), NULL);

	self=XFCE_OVERVIEW_WORKSPACES_WORKSPACE_SELECTOR(inFocusable);
	priv=self->priv;
	actor=NULL;

	/* Find actor for current active workspace which is also the current selection */
	if(priv->activeWorkspace)
	{
		actor=_xfce_overview_workspaces_workspace_selector_find_actor_for_workspace(self, priv->activeWorkspace);
	}
	if(!actor) return(NULL);

	/* Return current selection */
	return(CLUTTER_ACTOR(actor));
}

/* Set new selection */
static gboolean _xfce_overview_workspaces_workspace_selector_focusable_set_selection(XfceOverviewWorkspacesFocusable *inFocusable,
																		ClutterActor *inSelection)
{
	XfceOverviewWorkspacesWorkspaceSelector			*self;
	XfceOverviewWorkspacesLiveWorkspace				*actor;
	XfceOverviewWorkspacesWindowTrackerWorkspace		*workspace;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_FOCUSABLE(inFocusable), FALSE);
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WORKSPACE_SELECTOR(inFocusable), FALSE);
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WORKSPACE(inSelection), FALSE);

	self=XFCE_OVERVIEW_WORKSPACES_WORKSPACE_SELECTOR(inFocusable);
	actor=XFCE_OVERVIEW_WORKSPACES_LIVE_WORKSPACE(inSelection);
	workspace=NULL;

	/* Check that selection is a child of this actor */
	if(!clutter_actor_contains(CLUTTER_ACTOR(self), inSelection))
	{
		ClutterActor						*parent;

		parent=clutter_actor_get_parent(inSelection);
		g_warning("%s is a child of %s and cannot be selected at %s",
					G_OBJECT_TYPE_NAME(inSelection),
					parent ? G_OBJECT_TYPE_NAME(parent) : "<nil>",
					G_OBJECT_TYPE_NAME(self));
	}

	/* Get workspace of new selection and set new selection*/
	workspace=xfce_overview_workspaces_live_workspace_get_workspace(actor);
	if(workspace)
	{
		/* Setting new selection means also to set selected workspace active */
		xfce_overview_workspaces_window_tracker_workspace_activate(workspace);

		/* New selection was set successfully */
		return(TRUE);
	}

	/* Setting new selection was unsuccessful if we get here */
	g_warning("Could not determine workspace of %s to set selection at %s",
				G_OBJECT_TYPE_NAME(actor),
				G_OBJECT_TYPE_NAME(self));

	return(FALSE);
}

/* Find requested selection target depending of current selection */
static ClutterActor* _xfce_overview_workspaces_workspace_selector_focusable_find_selection(XfceOverviewWorkspacesFocusable *inFocusable,
																				ClutterActor *inSelection,
																				XfceOverviewWorkspacesSelectionTarget inDirection)
{
	XfceOverviewWorkspacesWorkspaceSelector			*self;
	XfceOverviewWorkspacesWorkspaceSelectorPrivate		*priv;
	XfceOverviewWorkspacesLiveWorkspace				*selection;
	ClutterActor							*newSelection;
	gchar									*valueName;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_FOCUSABLE(inFocusable), NULL);
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WORKSPACE_SELECTOR(inFocusable), NULL);
	g_return_val_if_fail(!inSelection || XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WORKSPACE(inSelection), NULL);

	self=XFCE_OVERVIEW_WORKSPACES_WORKSPACE_SELECTOR(inFocusable);
	priv=self->priv;
	newSelection=NULL;
	selection=NULL;

	/* Find actor for current active workspace which is also the current selection */
	if(priv->activeWorkspace)
	{
		selection=_xfce_overview_workspaces_workspace_selector_find_actor_for_workspace(self, priv->activeWorkspace);
	}
	if(!selection) return(NULL);

	/* If there is nothing selected return currently determined actor which is
	 * the current active workspace.
	 */
	if(!inSelection)
	{
		valueName=xfce_overview_workspaces_get_enum_value_name(XFCE_OVERVIEW_WORKSPACES_TYPE_SELECTION_TARGET, inDirection);
		XFCE_OVERVIEW_WORKSPACES_DEBUG(self, ACTOR,
							"No selection at %s, so select first child %s for direction %s",
							G_OBJECT_TYPE_NAME(self),
							selection ? G_OBJECT_TYPE_NAME(selection) : "<nil>",
							valueName);
		g_free(valueName);

		return(CLUTTER_ACTOR(selection));
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

	/* Find target selection */
	switch(inDirection)
	{
		case XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_LEFT:
			if(priv->orientation==CLUTTER_ORIENTATION_HORIZONTAL)
			{
				newSelection=clutter_actor_get_previous_sibling(CLUTTER_ACTOR(selection));
			}
			break;

		case XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_UP:
			if(priv->orientation==CLUTTER_ORIENTATION_VERTICAL)
			{
				newSelection=clutter_actor_get_previous_sibling(CLUTTER_ACTOR(selection));
			}
			break;

		case XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_RIGHT:
			if(priv->orientation==CLUTTER_ORIENTATION_HORIZONTAL)
			{
				newSelection=clutter_actor_get_next_sibling(CLUTTER_ACTOR(selection));
			}
			break;

		case XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_DOWN:
			if(priv->orientation==CLUTTER_ORIENTATION_VERTICAL)
			{
				newSelection=clutter_actor_get_next_sibling(CLUTTER_ACTOR(selection));
			}
			break;

		case XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_FIRST:
		case XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_PAGE_UP:
		case XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_PAGE_LEFT:
			if(inDirection==XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_FIRST ||
				(inDirection==XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_PAGE_UP && priv->orientation==CLUTTER_ORIENTATION_VERTICAL) ||
				(inDirection==XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_PAGE_LEFT && priv->orientation==CLUTTER_ORIENTATION_HORIZONTAL))
			{
				newSelection=clutter_actor_get_first_child(CLUTTER_ACTOR(self));
			}
			break;

		case XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_LAST:
		case XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_PAGE_DOWN:
		case XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_PAGE_RIGHT:
			if(inDirection==XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_LAST ||
				(inDirection==XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_PAGE_DOWN && priv->orientation==CLUTTER_ORIENTATION_VERTICAL) ||
				(inDirection==XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_PAGE_RIGHT && priv->orientation==CLUTTER_ORIENTATION_HORIZONTAL))
			{
				newSelection=clutter_actor_get_last_child(CLUTTER_ACTOR(self));
			}
			break;

		case XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_NEXT:
			newSelection=clutter_actor_get_next_sibling(CLUTTER_ACTOR(selection));
			if(!newSelection) newSelection=clutter_actor_get_previous_sibling(CLUTTER_ACTOR(selection));
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
	if(newSelection && XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WORKSPACE(newSelection))
	{
		selection=XFCE_OVERVIEW_WORKSPACES_LIVE_WORKSPACE(newSelection);
	}

	/* Return new selection found */
	XFCE_OVERVIEW_WORKSPACES_DEBUG(self, ACTOR,
						"Selecting %s at %s for current selection %s in direction %u",
						selection ? G_OBJECT_TYPE_NAME(selection) : "<nil>",
						G_OBJECT_TYPE_NAME(self),
						inSelection ? G_OBJECT_TYPE_NAME(inSelection) : "<nil>",
						inDirection);

	return(CLUTTER_ACTOR(selection));
}

/* Activate selection */
static gboolean _xfce_overview_workspaces_workspace_selector_focusable_activate_selection(XfceOverviewWorkspacesFocusable *inFocusable,
																				ClutterActor *inSelection)
{
	XfceOverviewWorkspacesWorkspaceSelector			*self;
	XfceOverviewWorkspacesLiveWorkspace				*actor;
	XfceOverviewWorkspacesWindowTrackerWorkspace		*workspace;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_FOCUSABLE(inFocusable), FALSE);
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WORKSPACE_SELECTOR(inFocusable), FALSE);
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WORKSPACE(inSelection), FALSE);

	self=XFCE_OVERVIEW_WORKSPACES_WORKSPACE_SELECTOR(inFocusable);
	actor=XFCE_OVERVIEW_WORKSPACES_LIVE_WORKSPACE(inSelection);
	workspace=NULL;

	/* Check that selection is a child of this actor */
	if(!clutter_actor_contains(CLUTTER_ACTOR(self), inSelection))
	{
		ClutterActor						*parent;

		parent=clutter_actor_get_parent(inSelection);
		g_warning("%s is a child of %s and cannot be selected at %s",
					G_OBJECT_TYPE_NAME(inSelection),
					parent ? G_OBJECT_TYPE_NAME(parent) : "<nil>",
					G_OBJECT_TYPE_NAME(self));
	}

	/* Get workspace of new selection and set new selection*/
	workspace=xfce_overview_workspaces_live_workspace_get_workspace(actor);
	if(workspace)
	{
		/* Activate workspace */
		xfce_overview_workspaces_window_tracker_workspace_activate(workspace);

		/* Request core to quit */
		xfce_overview_workspaces_core_quit(NULL);

		/* Activation was successful */
		return(TRUE);
	}

	/* Activation was unsuccessful if we get here */
	g_warning("Could not determine workspace of %s to set selection at %s",
				G_OBJECT_TYPE_NAME(actor),
				G_OBJECT_TYPE_NAME(self));
	return(FALSE);
}

/* Interface initialization
 * Set up default functions
 */
void _xfce_overview_workspaces_workspace_selector_focusable_iface_init(XfceOverviewWorkspacesFocusableInterface *iface)
{
	iface->supports_selection=_xfce_overview_workspaces_workspace_selector_focusable_supports_selection;
	iface->get_selection=_xfce_overview_workspaces_workspace_selector_focusable_get_selection;
	iface->set_selection=_xfce_overview_workspaces_workspace_selector_focusable_set_selection;
	iface->find_selection=_xfce_overview_workspaces_workspace_selector_focusable_find_selection;
	iface->activate_selection=_xfce_overview_workspaces_workspace_selector_focusable_activate_selection;
}

/* IMPLEMENTATION: GObject */

/* Dispose this object */
static void _xfce_overview_workspaces_workspace_selector_dispose(GObject *inObject)
{
	XfceOverviewWorkspacesWorkspaceSelector			*self=XFCE_OVERVIEW_WORKSPACES_WORKSPACE_SELECTOR(inObject);
	XfceOverviewWorkspacesWorkspaceSelectorPrivate		*priv=self->priv;

	/* Release allocated resources */
	if(priv->activeWorkspace)
	{
		_xfce_overview_workspaces_workspace_selector_on_active_workspace_changed(self, NULL, priv->activeWorkspace);
		priv->activeWorkspace=NULL;
	}

	if(priv->windowTracker)
	{
		g_signal_handlers_disconnect_by_data(priv->windowTracker, self);
		g_object_unref(priv->windowTracker);
		priv->windowTracker=NULL;
	}

	/* Call parent's class dispose method */
	G_OBJECT_CLASS(xfce_overview_workspaces_workspace_selector_parent_class)->dispose(inObject);
}

/* Set/get properties */
static void _xfce_overview_workspaces_workspace_selector_set_property(GObject *inObject,
															guint inPropID,
															const GValue *inValue,
															GParamSpec *inSpec)
{
	XfceOverviewWorkspacesWorkspaceSelector			*self=XFCE_OVERVIEW_WORKSPACES_WORKSPACE_SELECTOR(inObject);

	switch(inPropID)
	{
		case PROP_SPACING:
			xfce_overview_workspaces_workspace_selector_set_spacing(self, g_value_get_float(inValue));
			break;

		case PROP_ORIENTATION:
			xfce_overview_workspaces_workspace_selector_set_orientation(self, g_value_get_enum(inValue));
			break;

		case PROP_MAX_SIZE:
			xfce_overview_workspaces_workspace_selector_set_maximum_size(self, g_value_get_float(inValue));
			break;

		case PROP_MAX_FRACTION:
			xfce_overview_workspaces_workspace_selector_set_maximum_fraction(self, g_value_get_float(inValue));
			break;

		case PROP_SHOW_CURRENT_MONITOR_ONLY:
			xfce_overview_workspaces_workspace_selector_set_show_current_monitor_only(self, g_value_get_boolean(inValue));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(inObject, inPropID, inSpec);
			break;
	}
}

static void _xfce_overview_workspaces_workspace_selector_get_property(GObject *inObject,
															guint inPropID,
															GValue *outValue,
															GParamSpec *inSpec)
{
	XfceOverviewWorkspacesWorkspaceSelector			*self=XFCE_OVERVIEW_WORKSPACES_WORKSPACE_SELECTOR(inObject);
	XfceOverviewWorkspacesWorkspaceSelectorPrivate		*priv=self->priv;

	switch(inPropID)
	{
		case PROP_SPACING:
			g_value_set_float(outValue, priv->spacing);
			break;

		case PROP_ORIENTATION:
			g_value_set_enum(outValue, priv->orientation);
			break;

		case PROP_MAX_SIZE:
			g_value_set_float(outValue, priv->maxSize);
			break;

		case PROP_MAX_FRACTION:
			g_value_set_float(outValue, priv->maxFraction);
			break;

		case PROP_USING_FRACTION:
			g_value_set_boolean(outValue, priv->usingFraction);
			break;

		case PROP_SHOW_CURRENT_MONITOR_ONLY:
			g_value_set_boolean(outValue, priv->showCurrentMonitorOnly);
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
static void xfce_overview_workspaces_workspace_selector_class_init(XfceOverviewWorkspacesWorkspaceSelectorClass *klass)
{
	XfceOverviewWorkspacesActorClass	*actorClass=XFCE_OVERVIEW_WORKSPACES_ACTOR_CLASS(klass);
	ClutterActorClass		*clutterActorClass=CLUTTER_ACTOR_CLASS(klass);
	GObjectClass			*gobjectClass=G_OBJECT_CLASS(klass);

	/* Override functions */
	gobjectClass->dispose=_xfce_overview_workspaces_workspace_selector_dispose;
	gobjectClass->set_property=_xfce_overview_workspaces_workspace_selector_set_property;
	gobjectClass->get_property=_xfce_overview_workspaces_workspace_selector_get_property;

	clutterActorClass->get_preferred_width=_xfce_overview_workspaces_workspace_selector_get_preferred_width;
	clutterActorClass->get_preferred_height=_xfce_overview_workspaces_workspace_selector_get_preferred_height;
	clutterActorClass->allocate=_xfce_overview_workspaces_workspace_selector_allocate;

	/* Define properties */
	XfceOverviewWorkspacesWorkspaceSelectorProperties[PROP_SPACING]=
		g_param_spec_float("spacing",
								"Spacing",
								"The spacing between children",
								0.0, G_MAXFLOAT,
								0.0,
								G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	XfceOverviewWorkspacesWorkspaceSelectorProperties[PROP_ORIENTATION]=
		g_param_spec_enum("orientation",
							"Orientation",
							"The orientation to layout children",
							CLUTTER_TYPE_ORIENTATION,
							DEFAULT_ORIENTATION,
							G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	XfceOverviewWorkspacesWorkspaceSelectorProperties[PROP_MAX_SIZE]=
		g_param_spec_float("max-size",
								"Maximum size",
								"The maximum size of this actor for opposite direction of orientation",
								0.0, G_MAXFLOAT,
								DEFAULT_MAX_SIZE,
								G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	XfceOverviewWorkspacesWorkspaceSelectorProperties[PROP_MAX_FRACTION]=
		g_param_spec_float("max-fraction",
								"Maximum fraction",
								"The maximum size of this actor for opposite direction of orientation defined by fraction between 0.0 and 1.0",
								0.0, G_MAXFLOAT,
								DEFAULT_MAX_FRACTION,
								G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	XfceOverviewWorkspacesWorkspaceSelectorProperties[PROP_USING_FRACTION]=
		g_param_spec_boolean("using-fraction",
								"Using fraction",
								"Flag indicating if maximum size is static or defined by fraction between 0.0 and 1.0",
								DEFAULT_USING_FRACTION,
								G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

	XfceOverviewWorkspacesWorkspaceSelectorProperties[PROP_SHOW_CURRENT_MONITOR_ONLY]=
		g_param_spec_boolean("show-current-monitor-only",
								"Show current monitor only",
								"Show only windows of the monitor where this actor is placed",
								FALSE,
								G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(gobjectClass, PROP_LAST, XfceOverviewWorkspacesWorkspaceSelectorProperties);

	/* Define stylable properties */
	xfce_overview_workspaces_actor_install_stylable_property(actorClass, XfceOverviewWorkspacesWorkspaceSelectorProperties[PROP_SPACING]);
	xfce_overview_workspaces_actor_install_stylable_property(actorClass, XfceOverviewWorkspacesWorkspaceSelectorProperties[PROP_ORIENTATION]);
	xfce_overview_workspaces_actor_install_stylable_property(actorClass, XfceOverviewWorkspacesWorkspaceSelectorProperties[PROP_MAX_SIZE]);
	xfce_overview_workspaces_actor_install_stylable_property(actorClass, XfceOverviewWorkspacesWorkspaceSelectorProperties[PROP_MAX_FRACTION]);
}

/* Object initialization
 * Create private structure and set up default values
 */
static void xfce_overview_workspaces_workspace_selector_init(XfceOverviewWorkspacesWorkspaceSelector *self)
{
	XfceOverviewWorkspacesWorkspaceSelectorPrivate		*priv;
	ClutterRequestMode						requestMode;
	GList									*workspaces;
	XfceOverviewWorkspacesWindowTrackerWorkspace		*workspace;

	priv=self->priv=xfce_overview_workspaces_workspace_selector_get_instance_private(self);

	/* Set up default values */
	priv->windowTracker=xfce_overview_workspaces_core_get_window_tracker(NULL);
	priv->activeWorkspace=NULL;
	priv->spacing=0.0f;
	priv->orientation=DEFAULT_ORIENTATION;
	priv->maxSize=DEFAULT_MAX_SIZE;
	priv->maxFraction=DEFAULT_MAX_FRACTION;
	priv->usingFraction=DEFAULT_USING_FRACTION;
	priv->showCurrentMonitorOnly=FALSE;

	/* Set up this actor */
	clutter_actor_set_reactive(CLUTTER_ACTOR(self), TRUE);
	clutter_actor_set_clip_to_allocation(CLUTTER_ACTOR(self), TRUE);

	requestMode=(priv->orientation==CLUTTER_ORIENTATION_HORIZONTAL ? CLUTTER_REQUEST_HEIGHT_FOR_WIDTH : CLUTTER_REQUEST_WIDTH_FOR_HEIGHT);
	clutter_actor_set_request_mode(CLUTTER_ACTOR(self), requestMode);

	/* Connect signals */
	g_signal_connect(self, "scroll-event", G_CALLBACK(_xfce_overview_workspaces_workspace_selector_on_scroll_event), NULL);

	g_signal_connect_swapped(priv->windowTracker,
								"workspace-added",
								G_CALLBACK(_xfce_overview_workspaces_workspace_selector_on_workspace_added),
								self);

	g_signal_connect_swapped(priv->windowTracker,
								"workspace-removed",
								G_CALLBACK(_xfce_overview_workspaces_workspace_selector_on_workspace_removed),
								self);

	g_signal_connect_swapped(priv->windowTracker,
								"active-workspace-changed",
								G_CALLBACK(_xfce_overview_workspaces_workspace_selector_on_active_workspace_changed),
								self);

	/* If we there are already workspace known add them to this actor */
	workspaces=xfce_overview_workspaces_window_tracker_get_workspaces(priv->windowTracker);
	if(workspaces)
	{
		for(; workspaces; workspaces=g_list_next(workspaces))
		{
			workspace=(XfceOverviewWorkspacesWindowTrackerWorkspace*)workspaces->data;

			_xfce_overview_workspaces_workspace_selector_on_workspace_added(self, workspace, NULL);
		}
	}

	/* If active workspace is already available then set it in this actor */
	workspace=xfce_overview_workspaces_window_tracker_get_active_workspace(priv->windowTracker);
	if(workspace)
	{
		_xfce_overview_workspaces_workspace_selector_on_active_workspace_changed(self, NULL, NULL);
	}
}

/* IMPLEMENTATION: Public API */

/* Create new actor */
ClutterActor* xfce_overview_workspaces_workspace_selector_new(void)
{
	return(g_object_new(XFCE_OVERVIEW_WORKSPACES_TYPE_WORKSPACE_SELECTOR, NULL));
}

ClutterActor* xfce_overview_workspaces_workspace_selector_new_with_orientation(ClutterOrientation inOrientation)
{
	g_return_val_if_fail(inOrientation==CLUTTER_ORIENTATION_HORIZONTAL || inOrientation==CLUTTER_ORIENTATION_VERTICAL, NULL);

	return(g_object_new(XFCE_OVERVIEW_WORKSPACES_TYPE_WORKSPACE_SELECTOR,
						"orientation", inOrientation,
						NULL));
}

/* Get/set spacing between children */
gfloat xfce_overview_workspaces_workspace_selector_get_spacing(XfceOverviewWorkspacesWorkspaceSelector *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WORKSPACE_SELECTOR(self), 0.0f);

	return(self->priv->spacing);
}

void xfce_overview_workspaces_workspace_selector_set_spacing(XfceOverviewWorkspacesWorkspaceSelector *self, const gfloat inSpacing)
{
	XfceOverviewWorkspacesWorkspaceSelectorPrivate		*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WORKSPACE_SELECTOR(self));
	g_return_if_fail(inSpacing>=0.0f);

	priv=self->priv;

	/* Set value if changed */
	if(priv->spacing!=inSpacing)
	{
		/* Set value */
		priv->spacing=inSpacing;
		clutter_actor_queue_relayout(CLUTTER_ACTOR(self));

		xfce_overview_workspaces_background_set_corner_radius(XFCE_OVERVIEW_WORKSPACES_BACKGROUND(self), priv->spacing);

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesWorkspaceSelectorProperties[PROP_SPACING]);
	}
}

/* Get/set orientation */
ClutterOrientation xfce_overview_workspaces_workspace_selector_get_orientation(XfceOverviewWorkspacesWorkspaceSelector *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WORKSPACE_SELECTOR(self), DEFAULT_ORIENTATION);

	return(self->priv->orientation);
}

void xfce_overview_workspaces_workspace_selector_set_orientation(XfceOverviewWorkspacesWorkspaceSelector *self, ClutterOrientation inOrientation)
{
	XfceOverviewWorkspacesWorkspaceSelectorPrivate		*priv;
	ClutterRequestMode						requestMode;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WORKSPACE_SELECTOR(self));
	g_return_if_fail(inOrientation==CLUTTER_ORIENTATION_HORIZONTAL ||
						inOrientation==CLUTTER_ORIENTATION_VERTICAL);

	priv=self->priv;

	/* Set value if changed */
	if(priv->orientation!=inOrientation)
	{
		/* Set value */
		priv->orientation=inOrientation;

		requestMode=(priv->orientation==CLUTTER_ORIENTATION_HORIZONTAL ? CLUTTER_REQUEST_HEIGHT_FOR_WIDTH : CLUTTER_REQUEST_WIDTH_FOR_HEIGHT);
		clutter_actor_set_request_mode(CLUTTER_ACTOR(self), requestMode);

		clutter_actor_queue_relayout(CLUTTER_ACTOR(self));

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesWorkspaceSelectorProperties[PROP_ORIENTATION]);
	}
}

/* Get/set static maximum size of children */
gfloat xfce_overview_workspaces_workspace_selector_get_maximum_size(XfceOverviewWorkspacesWorkspaceSelector *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WORKSPACE_SELECTOR(self), 0.0f);

	return(self->priv->maxSize);
}

void xfce_overview_workspaces_workspace_selector_set_maximum_size(XfceOverviewWorkspacesWorkspaceSelector *self, const gfloat inSize)
{
	XfceOverviewWorkspacesWorkspaceSelectorPrivate		*priv;
	gboolean								needRelayout;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WORKSPACE_SELECTOR(self));
	g_return_if_fail(inSize>=0.0f);

	priv=self->priv;
	needRelayout=FALSE;

	/* Freeze notification */
	g_object_freeze_notify(G_OBJECT(self));

	/* Set values if changed */
	if(priv->usingFraction)
	{
		/* Set value */
		priv->usingFraction=FALSE;
		needRelayout=TRUE;

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesWorkspaceSelectorProperties[PROP_USING_FRACTION]);
	}

	if(priv->maxSize!=inSize)
	{
		/* Set value */
		priv->maxSize=inSize;
		needRelayout=TRUE;

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesWorkspaceSelectorProperties[PROP_MAX_SIZE]);
	}

	/* Queue a relayout if needed */
	if(needRelayout) clutter_actor_queue_relayout(CLUTTER_ACTOR(self));

	/* Thaw notification */
	g_object_thaw_notify(G_OBJECT(self));
}

/* Get/set maximum size of children by fraction */
gfloat xfce_overview_workspaces_workspace_selector_get_maximum_fraction(XfceOverviewWorkspacesWorkspaceSelector *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WORKSPACE_SELECTOR(self), 0.0f);

	return(self->priv->maxFraction);
}

void xfce_overview_workspaces_workspace_selector_set_maximum_fraction(XfceOverviewWorkspacesWorkspaceSelector *self, const gfloat inFraction)
{
	XfceOverviewWorkspacesWorkspaceSelectorPrivate		*priv;
	gboolean								needRelayout;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WORKSPACE_SELECTOR(self));
	g_return_if_fail(inFraction>0.0f && inFraction<=1.0f);

	priv=self->priv;
	needRelayout=FALSE;

	/* Freeze notification */
	g_object_freeze_notify(G_OBJECT(self));

	/* Set values if changed */
	if(!priv->usingFraction)
	{
		/* Set value */
		priv->usingFraction=TRUE;
		needRelayout=TRUE;

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesWorkspaceSelectorProperties[PROP_USING_FRACTION]);
	}

	if(priv->maxFraction!=inFraction)
	{
		/* Set value */
		priv->maxFraction=inFraction;
		needRelayout=TRUE;

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesWorkspaceSelectorProperties[PROP_MAX_FRACTION]);
	}

	/* Queue a relayout if needed */
	if(needRelayout) clutter_actor_queue_relayout(CLUTTER_ACTOR(self));

	/* Thaw notification */
	g_object_thaw_notify(G_OBJECT(self));
}

/* Get state if maximum size is static or calculated by fraction dynamically */
gboolean xfce_overview_workspaces_workspace_selector_is_using_fraction(XfceOverviewWorkspacesWorkspaceSelector *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WORKSPACE_SELECTOR(self), FALSE);

	return(self->priv->usingFraction);
}

/* Get/set orientation */
gboolean xfce_overview_workspaces_workspace_selector_get_show_current_monitor_only(XfceOverviewWorkspacesWorkspaceSelector *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WORKSPACE_SELECTOR(self), FALSE);

	return(self->priv->showCurrentMonitorOnly);
}

void xfce_overview_workspaces_workspace_selector_set_show_current_monitor_only(XfceOverviewWorkspacesWorkspaceSelector *self, gboolean inShowCurrentMonitorOnly)
{
	XfceOverviewWorkspacesWorkspaceSelectorPrivate		*priv;
	ClutterActorIter						iter;
	ClutterActor							*child;
	XfceOverviewWorkspacesStageInterface				*stageInterface;
	XfceOverviewWorkspacesWindowTrackerMonitor			*monitor;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WORKSPACE_SELECTOR(self));

	priv=self->priv;

	/* Set value if changed */
	if(priv->showCurrentMonitorOnly!=inShowCurrentMonitorOnly)
	{
		/* Set value */
		priv->showCurrentMonitorOnly=inShowCurrentMonitorOnly;

		/* Get parent stage interface */
		stageInterface=xfce_overview_workspaces_get_stage_of_actor(CLUTTER_ACTOR(self));

		/* Get monitor of stage interface if available and if only windows
		 * of current monitor should be shown.
		 */
		monitor=NULL;
		if(stageInterface && priv->showCurrentMonitorOnly) monitor=xfce_overview_workspaces_stage_interface_get_monitor(stageInterface);

		/* Iterate through workspace actors and update monitor */
		clutter_actor_iter_init(&iter, CLUTTER_ACTOR(self));
		while(clutter_actor_iter_next(&iter, &child))
		{
			if(XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WORKSPACE(child))
			{
				xfce_overview_workspaces_live_workspace_set_monitor(XFCE_OVERVIEW_WORKSPACES_LIVE_WORKSPACE(child), monitor);
			}
		}

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesWorkspaceSelectorProperties[PROP_SHOW_CURRENT_MONITOR_ONLY]);
	}
}
