/*
 * live-window-simple: An actor showing the content of a window which will
 *                     be updated if changed and visible on active workspace.
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

#include <libxfce-overview-workspaces/live-window-simple.h>
#include <libxfce-overview-workspaces/live-window-snapshot.h>

#include <math.h>
#include <glib/gi18n-lib.h>

#include <libxfce-overview-workspaces/click-action.h>
#include <libxfce-overview-workspaces/window-content.h>
#include <libxfce-overview-workspaces/image-content.h>
#include <libxfce-overview-workspaces/stylable.h>
#include <libxfce-overview-workspaces/enums.h>
#include <libxfce-overview-workspaces/debug.h>

#include <gdk-pixbuf/gdk-pixbuf.h>


/* Define this class in GObject system */
struct _XfceOverviewWorkspacesLiveWindowSimplePrivate
{
	/* Properties related */
	XfceOverviewWorkspacesWindowTrackerWindow			*window;
	XfceOverviewWorkspacesLiveWindowSimpleDisplayType	displayType;

	/* Instance related */
	gboolean								isVisible;
	ClutterActor							*actorWindow;
	gboolean								destroyOnClose;
	gboolean								isFrozen;
};

G_DEFINE_TYPE_WITH_PRIVATE(XfceOverviewWorkspacesLiveWindowSimple,
							xfce_overview_workspaces_live_window_simple,
							XFCE_OVERVIEW_WORKSPACES_TYPE_BACKGROUND)

/* Properties */
enum
{
	PROP_0,

	PROP_WINDOW,
	PROP_DISPLAY_TYPE,
	PROP_DESTROY_ON_CLOSE,

	PROP_LAST
};

static GParamSpec* XfceOverviewWorkspacesLiveWindowSimpleProperties[PROP_LAST]={ 0, };

/* Signals */
enum
{
	SIGNAL_GEOMETRY_CHANGED,
	SIGNAL_VISIBILITY_CHANGED,
	SIGNAL_WORKSPACE_CHANGED,

	SIGNAL_LAST
};

static guint XfceOverviewWorkspacesLiveWindowSimpleSignals[SIGNAL_LAST]={ 0, };

/* IMPLEMENTATION: Private variables and methods */

/* Check if window should be shown */
static gboolean _xfce_overview_workspaces_live_window_simple_is_visible_window(XfceOverviewWorkspacesLiveWindowSimple *self, XfceOverviewWorkspacesWindowTrackerWindow *inWindow)
{
	XfceOverviewWorkspacesWindowTrackerWindowState		state;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_WINDOW(inWindow), FALSE);

	/* Determine if windows should be shown depending on its state */
	state=xfce_overview_workspaces_window_tracker_window_get_state(inWindow);
	if((state & XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW_STATE_SKIP_PAGER) ||
		(state & XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW_STATE_SKIP_TASKLIST))
	{
		return(FALSE);
	}

	/* If we get here the window should be shown */
	return(TRUE);
}

/* Position and/or size of window has changed */
static void _xfce_overview_workspaces_live_window_simple_on_geometry_changed(XfceOverviewWorkspacesLiveWindowSimple *self,
																gpointer inUserData)
{
	XfceOverviewWorkspacesLiveWindowSimplePrivate	*priv;
	XfceOverviewWorkspacesWindowTrackerWindow		*window;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WINDOW_SIMPLE(self));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_WINDOW(inUserData));

	priv=self->priv;
	window=XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW(inUserData);

	/* Check if signal is for this window */
	if(window!=priv->window) return;

	/* Actor's allocation may change because of new geometry so relayout */
	clutter_actor_queue_relayout(CLUTTER_ACTOR(self));

	/* Emit "geometry-changed" signal */
	g_signal_emit(self, XfceOverviewWorkspacesLiveWindowSimpleSignals[SIGNAL_GEOMETRY_CHANGED], 0);
}

/* Window's state has changed */
static void _xfce_overview_workspaces_live_window_simple_on_state_changed(XfceOverviewWorkspacesLiveWindowSimple *self,
																XfceOverviewWorkspacesWindowTrackerWindowState inOldState,
																gpointer inUserData)
{
	XfceOverviewWorkspacesLiveWindowSimplePrivate		*priv;
	XfceOverviewWorkspacesWindowTrackerWindow			*window;
	gboolean								isVisible;
	XfceOverviewWorkspacesWindowTrackerWindowState		state;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WINDOW_SIMPLE(self));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_WINDOW(inUserData));

	priv=self->priv;
	window=XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW(inUserData);

	/* Check if signal is for this window */
	if(window!=priv->window) return;

	/* Check if window's visibility has changed */
	isVisible=_xfce_overview_workspaces_live_window_simple_is_visible_window(self, window);
	if(priv->isVisible!=isVisible)
	{
		priv->isVisible=isVisible;
		g_signal_emit(self, XfceOverviewWorkspacesLiveWindowSimpleSignals[SIGNAL_VISIBILITY_CHANGED], 0);
	}

	/* Add or remove class depending on 'pinned' window state */
	state=xfce_overview_workspaces_window_tracker_window_get_state(window);
	if(state & XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW_STATE_PINNED)
	{
		xfce_overview_workspaces_stylable_add_class(XFCE_OVERVIEW_WORKSPACES_STYLABLE(self), "window-state-pinned");
	}
		else
		{
			xfce_overview_workspaces_stylable_remove_class(XFCE_OVERVIEW_WORKSPACES_STYLABLE(self), "window-state-pinned");
		}

	/* Add or remove class depending on 'minimized' window state */
	if(state & XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW_STATE_MINIMIZED)
	{
		xfce_overview_workspaces_stylable_add_class(XFCE_OVERVIEW_WORKSPACES_STYLABLE(self), "window-state-minimized");
	}
		else
		{
			xfce_overview_workspaces_stylable_remove_class(XFCE_OVERVIEW_WORKSPACES_STYLABLE(self), "window-state-minimized");
		}

	/* Add or remove class depending on 'maximized' window state */
	if(state & XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW_STATE_MAXIMIZED)
	{
		xfce_overview_workspaces_stylable_add_class(XFCE_OVERVIEW_WORKSPACES_STYLABLE(self), "window-state-maximized");
	}
		else
		{
			xfce_overview_workspaces_stylable_remove_class(XFCE_OVERVIEW_WORKSPACES_STYLABLE(self), "window-state-maximized");
		}

	/* Add or remove class depending on 'urgent' window state */
	if(state & XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW_STATE_URGENT)
	{
		xfce_overview_workspaces_stylable_add_class(XFCE_OVERVIEW_WORKSPACES_STYLABLE(self), "window-state-urgent");
	}
		else
		{
			xfce_overview_workspaces_stylable_remove_class(XFCE_OVERVIEW_WORKSPACES_STYLABLE(self), "window-state-urgent");
		}
}

/* Window's workspace has changed */
static void _xfce_overview_workspaces_live_window_simple_on_workspace_changed(XfceOverviewWorkspacesLiveWindowSimple *self,
																	XfceOverviewWorkspacesWindowTrackerWorkspace *inWorkspace,
																	gpointer inUserData)
{
	XfceOverviewWorkspacesLiveWindowSimplePrivate	*priv;
	XfceOverviewWorkspacesWindowTrackerWindow		*window;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WINDOW_SIMPLE(self));
	g_return_if_fail(!inWorkspace || XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_WORKSPACE(inWorkspace));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_WINDOW(inUserData));

	priv=self->priv;
	window=XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW(inUserData);

	/* Check if signal is for this window */
	if(window!=priv->window) return;

	/* Emit "workspace-changed" signal */
	g_signal_emit(self, XfceOverviewWorkspacesLiveWindowSimpleSignals[SIGNAL_WORKSPACE_CHANGED], 0);
}

/* Window's was closed */
static void _xfce_overview_workspaces_live_window_simple_on_closed(XfceOverviewWorkspacesLiveWindowSimple *self,
														gpointer inUserData)
{
	XfceOverviewWorkspacesLiveWindowSimplePrivate	*priv;
	XfceOverviewWorkspacesWindowTrackerWindow		*window;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WINDOW_SIMPLE(self));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_WINDOW(inUserData));

	priv=self->priv;
	window=XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW(inUserData);

	/* Check if signal is for this window */
	if(window!=priv->window) return;

	/* Check if actor should be destroy when window was closed */
	if(priv->destroyOnClose)
	{
		XFCE_OVERVIEW_WORKSPACES_DEBUG(self, WINDOWS,
							"Window '%s' was closed and auto-destruction of actor was requested",
							xfce_overview_workspaces_window_tracker_window_get_name(priv->window));

		if(xfce_overview_workspaces_actor_destroy(CLUTTER_ACTOR(self)))
		{
			/* Release allocated resources early, before the dispose function
			 * is called, if an animation was started as the window is now gone!
			 */
			if(priv->window)
			{
				g_signal_handlers_disconnect_by_data(priv->window, self);
				priv->window=NULL;
			}
		}
	}
}

/* Check if actor is in icon/fallback mode: LIVE_PREVIEW without a real
 * window content (content==NULL or plain image from icon pixbuf). In this
 * mode natural size must be icon size, never window geometry, and the icon
 * must stay small and centered, never stretched. Full window geometry is
 * only valid for LIVE_PREVIEW with a real XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_CONTENT
 * texture.
 */
static gboolean _xfce_overview_workspaces_live_window_simple_is_icon_fallback(XfceOverviewWorkspacesLiveWindowSimple *self)
{
	XfceOverviewWorkspacesLiveWindowSimplePrivate	*priv;
	ClutterContent						*content;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WINDOW_SIMPLE(self), FALSE);

	priv=self->priv;

	/* Frozen snapshot uses window geometry like live preview, never icon size.
	 * It is an image content but must fill full allocation and stay interactive.
	 */
	if(priv->isFrozen) return(FALSE);

	/* Without child actor we cannot decide - assume live preview */
	if(!priv->actorWindow) return(FALSE);

	/* No content (photo not arrived yet) or non-window content
	 * (icon pixbuf image) means fallback.
	 */
	content=clutter_actor_get_content(priv->actorWindow);
	if(!content) return(TRUE);
	if(!XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_CONTENT(content)) return(TRUE);

	return(FALSE);
}

/* Set up actor's content depending on display. If no window is set the current
 * content of this actor is destroyed and a new one is not set up. The actor will
 * be displayed empty.
 */
static void _xfce_overview_workspaces_live_window_simple_setup_content(XfceOverviewWorkspacesLiveWindowSimple *self)
{
	XfceOverviewWorkspacesLiveWindowSimplePrivate	*priv;
	ClutterContent						*content;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WINDOW_SIMPLE(self));

	priv=self->priv;

	clutter_actor_set_content(priv->actorWindow, NULL);
	priv->isFrozen=FALSE;
	content=NULL;

	if(!priv->window) return;

	/* NOTE: order live->frozen->icon must be kept: live texture when viewable,
	 * otherwise frozen snapshot, otherwise window icon.
	 */
	if(xfce_overview_workspaces_live_window_snapshot_is_viewable(priv->window))
	{
		content=xfce_overview_workspaces_window_tracker_window_get_content(priv->window);
		if(!content)
		{
			GdkPixbuf			*icon;

			icon=xfce_overview_workspaces_live_window_snapshot_get_icon(priv->window);
			if(icon)
			{
				content=xfce_overview_workspaces_image_content_new_for_pixbuf(icon);
				g_object_unref(icon);
			}
		}
		if(content)
		{
			clutter_actor_set_content(priv->actorWindow, content);
			g_object_unref(content);
		}
	}
		else
		{
			content=xfce_overview_workspaces_live_window_snapshot_load_frozen(priv->window);
			if(content)
			{
				clutter_actor_set_content(priv->actorWindow, content);
				g_object_unref(content);
				priv->isFrozen=TRUE;
			}
				else
				{
					GdkPixbuf *icon;

					icon=xfce_overview_workspaces_live_window_snapshot_get_icon(priv->window);
					if(icon)
					{
						content=xfce_overview_workspaces_image_content_new_for_pixbuf(icon);
						g_object_unref(icon);
					}
					/* NOTE: never leave the actor empty: fall back to the
					 * live window content object which always exists
					 * and carries its fallback texture.
					 */
					if(!content) content=xfce_overview_workspaces_window_tracker_window_get_content(priv->window);
					if(content)
					{
						clutter_actor_set_content(priv->actorWindow, content);
						g_object_unref(content);
					}
				}
		}

	/* Content (live texture vs. small fallback icon) changed so preferred
	 * size and allocation must be recomputed. This is especially important
	 * when falling back to the icon until the live photo arrives.
	 */
	clutter_actor_queue_relayout(CLUTTER_ACTOR(self));
}

/* IMPLEMENTATION: ClutterActor */

/* Get preferred width/height */
static void _xfce_overview_workspaces_live_window_simple_get_preferred_height(ClutterActor *self,
																	gfloat inForWidth,
																	gfloat *outMinHeight,
																	gfloat *outNaturalHeight)
{
	XfceOverviewWorkspacesLiveWindowSimplePrivate	*priv=XFCE_OVERVIEW_WORKSPACES_LIVE_WINDOW_SIMPLE(self)->priv;
	gfloat								minHeight, naturalHeight;

	minHeight=naturalHeight=0.0f;

	/* Fallback/icon mode: natural size is icon size, never window geometry.
	 * Full geometry is only valid for LIVE_PREVIEW with a real window content.
	 */
	if(_xfce_overview_workspaces_live_window_simple_is_icon_fallback(XFCE_OVERVIEW_WORKSPACES_LIVE_WINDOW_SIMPLE(self)))
	{
		if(priv->actorWindow)
		{
			ClutterContent				*content;

			content=clutter_actor_get_content(priv->actorWindow);
			if(content)
			{
				gfloat					childNaturalHeight;

				if(clutter_content_get_preferred_size(content, NULL, &childNaturalHeight))
				{
					minHeight=naturalHeight=childNaturalHeight;
				}
			}
		}

		/* Store sizes computed */
		if(outMinHeight) *outMinHeight=minHeight;
		if(outNaturalHeight) *outNaturalHeight=naturalHeight;
		return;
	}

	/* Determine size of window if available and visible (should usually be the largest actor) */
	if(priv->actorWindow &&
		clutter_actor_is_visible(priv->actorWindow) &&
		priv->window)
	{
		gint							windowHeight;

		xfce_overview_workspaces_window_tracker_window_get_geometry(priv->window, NULL, NULL, NULL, &windowHeight);
		if(windowHeight<=0.0)
		{
			ClutterContent				*content;
			gfloat						childNaturalHeight;

			/* If we get here getting window size failed so fallback to old
			 * behaviour by getting size of window content associated to actor.
			 */
			content=clutter_actor_get_content(priv->actorWindow);
			if(content &&
				XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_CONTENT(content))
			{
				if(clutter_content_get_preferred_size(content, NULL, &childNaturalHeight)) windowHeight=childNaturalHeight;
				XFCE_OVERVIEW_WORKSPACES_DEBUG(self, WINDOWS,
									"Using fallback method to determine preferred height for window '%s'",
									xfce_overview_workspaces_window_tracker_window_get_name(priv->window));
			}
		}

		if(windowHeight>minHeight) minHeight=windowHeight;
		if(windowHeight>naturalHeight) naturalHeight=windowHeight;
	}

	/* Store sizes computed */
	if(outMinHeight) *outMinHeight=minHeight;
	if(outNaturalHeight) *outNaturalHeight=naturalHeight;
}

static void _xfce_overview_workspaces_live_window_simple_get_preferred_width(ClutterActor *self,
																gfloat inForHeight,
																gfloat *outMinWidth,
																gfloat *outNaturalWidth)
{
	XfceOverviewWorkspacesLiveWindowSimplePrivate	*priv=XFCE_OVERVIEW_WORKSPACES_LIVE_WINDOW_SIMPLE(self)->priv;
	gfloat								minWidth, naturalWidth;

	minWidth=naturalWidth=0.0f;

	/* Fallback/icon mode: natural size is icon size, never window geometry.
	 * Full geometry is only valid for LIVE_PREVIEW with a real window content.
	 */
	if(_xfce_overview_workspaces_live_window_simple_is_icon_fallback(XFCE_OVERVIEW_WORKSPACES_LIVE_WINDOW_SIMPLE(self)))
	{
		if(priv->actorWindow)
		{
			ClutterContent				*content;

			content=clutter_actor_get_content(priv->actorWindow);
			if(content)
			{
				gfloat					childNaturalWidth;

				if(clutter_content_get_preferred_size(content, &childNaturalWidth, NULL))
				{
					minWidth=naturalWidth=childNaturalWidth;
				}
			}
		}

		/* Store sizes computed */
		if(outMinWidth) *outMinWidth=minWidth;
		if(outNaturalWidth) *outNaturalWidth=naturalWidth;
		return;
	}

	/* Determine size of window if available and visible (should usually be the largest actor) */
	if(priv->actorWindow &&
		clutter_actor_is_visible(priv->actorWindow) &&
		priv->window)
	{
		gint							windowWidth;

		xfce_overview_workspaces_window_tracker_window_get_geometry(priv->window, NULL, NULL, &windowWidth, NULL);
		if(windowWidth<=0.0)
		{
			ClutterContent				*content;
			gfloat						childNaturalWidth;

			/* If we get here getting window size failed so fallback to old
			 * behaviour by getting size of window content associated to actor.
			 */
			content=clutter_actor_get_content(priv->actorWindow);
			if(content &&
				XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_CONTENT(content))
			{
				if(clutter_content_get_preferred_size(content, &childNaturalWidth, NULL)) windowWidth=childNaturalWidth;
				XFCE_OVERVIEW_WORKSPACES_DEBUG(self, WINDOWS,
									"Using fallback method to determine preferred width for window '%s'",
									xfce_overview_workspaces_window_tracker_window_get_name(priv->window));
			}
		}

		if(windowWidth>minWidth) minWidth=windowWidth;
		if(windowWidth>naturalWidth) naturalWidth=windowWidth;
	}

	/* Store sizes computed */
	if(outMinWidth) *outMinWidth=minWidth;
	if(outNaturalWidth) *outNaturalWidth=naturalWidth;
}

/* Allocate position and size of actor and its children */
static void _xfce_overview_workspaces_live_window_simple_allocate(ClutterActor *self,
														const ClutterActorBox *inBox,
														ClutterAllocationFlags inFlags)
{
	XfceOverviewWorkspacesLiveWindowSimplePrivate	*priv=XFCE_OVERVIEW_WORKSPACES_LIVE_WINDOW_SIMPLE(self)->priv;
	ClutterActorBox						*boxActorWindow=NULL;

	/* Chain up to store the allocation of the actor */
	CLUTTER_ACTOR_CLASS(xfce_overview_workspaces_live_window_simple_parent_class)->allocate(self, inBox, inFlags);

	if(!priv->actorWindow) return;

	/* Fallback/icon mode: keep icon small and centered, never stretched to
	 * window geometry. Even if the parent stretches this actor to window
	 * size, the inner icon actor stays at its natural icon size.
	 */
	if(_xfce_overview_workspaces_live_window_simple_is_icon_fallback(XFCE_OVERVIEW_WORKSPACES_LIVE_WINDOW_SIMPLE(self)))
	{
		gfloat							allocWidth, allocHeight;
		gfloat							iconWidth, iconHeight;
		gfloat							left, top, right, bottom;

		allocWidth=clutter_actor_box_get_width(inBox);
		allocHeight=clutter_actor_box_get_height(inBox);

		clutter_actor_get_preferred_size(priv->actorWindow, NULL, NULL, &iconWidth, &iconHeight);

		/* Clamp icon to allocation so it never overflows */
		if(iconWidth>allocWidth) iconWidth=allocWidth;
		if(iconHeight>allocHeight) iconHeight=allocHeight;

		/* Center icon within allocation */
		left=(allocWidth-iconWidth)/2.0f;
		top=(allocHeight-iconHeight)/2.0f;
		right=left+iconWidth;
		bottom=top+iconHeight;

		boxActorWindow=clutter_actor_box_new(floor(left), floor(top), floor(right), floor(bottom));
		clutter_actor_allocate(priv->actorWindow, boxActorWindow, inFlags);
	}
		else
		{
			/* Live preview with real texture: fill full window geometry */
			boxActorWindow=clutter_actor_box_copy(inBox);
			clutter_actor_box_set_origin(boxActorWindow, 0.0f, 0.0f);
			clutter_actor_allocate(priv->actorWindow, boxActorWindow, inFlags);
		}

	/* Release allocated resources */
	if(boxActorWindow) clutter_actor_box_free(boxActorWindow);
}

/* IMPLEMENTATION: GObject */

/* Dispose this object */
static void _xfce_overview_workspaces_live_window_simple_dispose(GObject *inObject)
{
	XfceOverviewWorkspacesLiveWindowSimple			*self=XFCE_OVERVIEW_WORKSPACES_LIVE_WINDOW_SIMPLE(inObject);
	XfceOverviewWorkspacesLiveWindowSimplePrivate	*priv=self->priv;

	/* Dispose allocated resources */
	if(priv->window)
	{
		g_signal_handlers_disconnect_by_data(priv->window, self);
		priv->window=NULL;
	}

	if(priv->actorWindow)
	{
		clutter_actor_destroy(priv->actorWindow);
		priv->actorWindow=NULL;
	}

	/* Call parent's class dispose method */
	G_OBJECT_CLASS(xfce_overview_workspaces_live_window_simple_parent_class)->dispose(inObject);
}

/* Set/get properties */
static void _xfce_overview_workspaces_live_window_simple_set_property(GObject *inObject,
													guint inPropID,
													const GValue *inValue,
													GParamSpec *inSpec)
{
	XfceOverviewWorkspacesLiveWindowSimple			*self=XFCE_OVERVIEW_WORKSPACES_LIVE_WINDOW_SIMPLE(inObject);
	
	switch(inPropID)
	{
		case PROP_WINDOW:
			xfce_overview_workspaces_live_window_simple_set_window(self, g_value_get_object(inValue));
			break;

		case PROP_DISPLAY_TYPE:
			xfce_overview_workspaces_live_window_simple_set_display_type(self, g_value_get_enum(inValue));
			break;

		case PROP_DESTROY_ON_CLOSE:
			xfce_overview_workspaces_live_window_simple_set_destroy_on_close(self, g_value_get_boolean(inValue));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(inObject, inPropID, inSpec);
			break;
	}
}

static void _xfce_overview_workspaces_live_window_simple_get_property(GObject *inObject,
													guint inPropID,
													GValue *outValue,
													GParamSpec *inSpec)
{
	XfceOverviewWorkspacesLiveWindowSimple	*self=XFCE_OVERVIEW_WORKSPACES_LIVE_WINDOW_SIMPLE(inObject);

	switch(inPropID)
	{
		case PROP_WINDOW:
			g_value_set_object(outValue, self->priv->window);
			break;

		case PROP_DISPLAY_TYPE:
			g_value_set_enum(outValue, self->priv->displayType);
			break;

		case PROP_DESTROY_ON_CLOSE:
			g_value_set_boolean(outValue, self->priv->destroyOnClose);
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
static void xfce_overview_workspaces_live_window_simple_class_init(XfceOverviewWorkspacesLiveWindowSimpleClass *klass)
{
	XfceOverviewWorkspacesActorClass	*actorClass=XFCE_OVERVIEW_WORKSPACES_ACTOR_CLASS(klass);
	ClutterActorClass		*clutterActorClass=CLUTTER_ACTOR_CLASS(klass);
	GObjectClass			*gobjectClass=G_OBJECT_CLASS(klass);

	/* Override functions */
	clutterActorClass->get_preferred_width=_xfce_overview_workspaces_live_window_simple_get_preferred_width;
	clutterActorClass->get_preferred_height=_xfce_overview_workspaces_live_window_simple_get_preferred_height;
	clutterActorClass->allocate=_xfce_overview_workspaces_live_window_simple_allocate;

	gobjectClass->dispose=_xfce_overview_workspaces_live_window_simple_dispose;
	gobjectClass->set_property=_xfce_overview_workspaces_live_window_simple_set_property;
	gobjectClass->get_property=_xfce_overview_workspaces_live_window_simple_get_property;

	/* Define properties */
	XfceOverviewWorkspacesLiveWindowSimpleProperties[PROP_WINDOW]=
		g_param_spec_object("window",
								"Window",
								"The window to show",
								XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_TRACKER_WINDOW,
								G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	XfceOverviewWorkspacesLiveWindowSimpleProperties[PROP_DISPLAY_TYPE]=
		g_param_spec_enum("display-type",
							"Display type",
							"How to display the window",
							XFCE_OVERVIEW_WORKSPACES_TYPE_LIVE_WINDOW_SIMPLE_DISPLAY_TYPE,
							XFCE_OVERVIEW_WORKSPACES_LIVE_WINDOW_SIMPLE_DISPLAY_TYPE_LIVE_PREVIEW,
							G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	XfceOverviewWorkspacesLiveWindowSimpleProperties[PROP_DESTROY_ON_CLOSE]=
		g_param_spec_boolean("destroy-on-close",
								"Destroy on close",
								"If this actor should be destroy when window was closed",
								TRUE,
								G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(gobjectClass, PROP_LAST, XfceOverviewWorkspacesLiveWindowSimpleProperties);

	/* Define stylable properties */
	xfce_overview_workspaces_actor_install_stylable_property(actorClass, XfceOverviewWorkspacesLiveWindowSimpleProperties[PROP_DISPLAY_TYPE]);

	/* Define signals */
	XfceOverviewWorkspacesLiveWindowSimpleSignals[SIGNAL_GEOMETRY_CHANGED]=
		g_signal_new("geometry-changed",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST,
						G_STRUCT_OFFSET(XfceOverviewWorkspacesLiveWindowSimpleClass, geometry_changed),
						NULL,
						NULL,
						g_cclosure_marshal_VOID__VOID,
						G_TYPE_NONE,
						0);

	XfceOverviewWorkspacesLiveWindowSimpleSignals[SIGNAL_VISIBILITY_CHANGED]=
		g_signal_new("visibility-changed",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST,
						G_STRUCT_OFFSET(XfceOverviewWorkspacesLiveWindowSimpleClass, visibility_changed),
						NULL,
						NULL,
						g_cclosure_marshal_VOID__BOOLEAN,
						G_TYPE_NONE,
						1,
						G_TYPE_BOOLEAN);

	XfceOverviewWorkspacesLiveWindowSimpleSignals[SIGNAL_WORKSPACE_CHANGED]=
		g_signal_new("workspace-changed",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST,
						G_STRUCT_OFFSET(XfceOverviewWorkspacesLiveWindowSimpleClass, workspace_changed),
						NULL,
						NULL,
						g_cclosure_marshal_VOID__VOID,
						G_TYPE_NONE,
						0);
}

/* Object initialization
 * Create private structure and set up default values
 */
static void xfce_overview_workspaces_live_window_simple_init(XfceOverviewWorkspacesLiveWindowSimple *self)
{
	XfceOverviewWorkspacesLiveWindowSimplePrivate	*priv;

	priv=self->priv=xfce_overview_workspaces_live_window_simple_get_instance_private(self);

	/* This actor reacts on events */
	clutter_actor_set_reactive(CLUTTER_ACTOR(self), TRUE);

	/* Set default values */
	priv->window=NULL;
	priv->displayType=XFCE_OVERVIEW_WORKSPACES_LIVE_WINDOW_SIMPLE_DISPLAY_TYPE_LIVE_PREVIEW;
	priv->destroyOnClose=TRUE;
	priv->isFrozen=FALSE;
	

	/* Set up child actors (order is important) */
	priv->actorWindow=clutter_actor_new();
	clutter_actor_show(priv->actorWindow);
	clutter_actor_add_child(CLUTTER_ACTOR(self), priv->actorWindow);
}

/* IMPLEMENTATION: Public API */

/* Create new instance */
ClutterActor* xfce_overview_workspaces_live_window_simple_new(void)
{
	return(CLUTTER_ACTOR(g_object_new(XFCE_OVERVIEW_WORKSPACES_TYPE_LIVE_WINDOW_SIMPLE, NULL)));
}

ClutterActor* xfce_overview_workspaces_live_window_simple_new_for_window(XfceOverviewWorkspacesWindowTrackerWindow *inWindow)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_WINDOW(inWindow), NULL);

	return(CLUTTER_ACTOR(g_object_new(XFCE_OVERVIEW_WORKSPACES_TYPE_LIVE_WINDOW_SIMPLE,
										"window", inWindow,
										NULL)));
}

/* Get/set window to show */
XfceOverviewWorkspacesWindowTrackerWindow* xfce_overview_workspaces_live_window_simple_get_window(XfceOverviewWorkspacesLiveWindowSimple *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WINDOW_SIMPLE(self), NULL);

	return(self->priv->window);
}

void xfce_overview_workspaces_live_window_simple_set_window(XfceOverviewWorkspacesLiveWindowSimple *self, XfceOverviewWorkspacesWindowTrackerWindow *inWindow)
{
	XfceOverviewWorkspacesLiveWindowSimplePrivate	*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WINDOW_SIMPLE(self));
	g_return_if_fail(!inWindow || XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_WINDOW(inWindow));

	priv=self->priv;

	/* Only set value if it changes */
	if(inWindow==priv->window) return;

	/* Release old value */
	if(priv->window)
	{
		g_signal_handlers_disconnect_by_data(priv->window, self);
		priv->window=NULL;
	}

	/* Set new value
	 * Window tracker objects should never be refed or unrefed, so just set new value
	 */
	priv->window=inWindow;
	if(priv->window)
	{
		/* Get visibility state of window */
		priv->isVisible=_xfce_overview_workspaces_live_window_simple_is_visible_window(self, priv->window);

		/* Setup window actor content */
		_xfce_overview_workspaces_live_window_simple_setup_content(self);

		/* Set up this actor and child actor by calling each signal handler now */
		_xfce_overview_workspaces_live_window_simple_on_geometry_changed(self, priv->window);
		_xfce_overview_workspaces_live_window_simple_on_state_changed(self, 0, priv->window);
		_xfce_overview_workspaces_live_window_simple_on_workspace_changed(self, NULL, priv->window);

		/* Connect signal handlers */
		g_signal_connect_swapped(priv->window, "geometry-changed", G_CALLBACK(_xfce_overview_workspaces_live_window_simple_on_geometry_changed), self);
		g_signal_connect_swapped(priv->window, "state-changed", G_CALLBACK(_xfce_overview_workspaces_live_window_simple_on_state_changed), self);
		g_signal_connect_swapped(priv->window, "workspace-changed", G_CALLBACK(_xfce_overview_workspaces_live_window_simple_on_workspace_changed), self);
		g_signal_connect_swapped(priv->window, "closed", G_CALLBACK(_xfce_overview_workspaces_live_window_simple_on_closed), self);
	}
		else
		{
			/* Clean window actor */
			clutter_actor_set_content(priv->actorWindow, NULL);
			priv->isFrozen=FALSE;

			/* Set window to invisible as NULL window is no window */
			priv->isVisible=FALSE;
		}

	/* Notify about property change */
	g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesLiveWindowSimpleProperties[PROP_WINDOW]);
}

/* Get/set display type of window */
XfceOverviewWorkspacesLiveWindowSimpleDisplayType xfce_overview_workspaces_live_window_simple_get_display_type(XfceOverviewWorkspacesLiveWindowSimple *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WINDOW_SIMPLE(self), XFCE_OVERVIEW_WORKSPACES_LIVE_WINDOW_SIMPLE_DISPLAY_TYPE_LIVE_PREVIEW);

	return(self->priv->displayType);
}

void xfce_overview_workspaces_live_window_simple_set_display_type(XfceOverviewWorkspacesLiveWindowSimple *self, XfceOverviewWorkspacesLiveWindowSimpleDisplayType inType)
{
	XfceOverviewWorkspacesLiveWindowSimplePrivate	*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WINDOW_SIMPLE(self));
	g_return_if_fail(inType>=XFCE_OVERVIEW_WORKSPACES_LIVE_WINDOW_SIMPLE_DISPLAY_TYPE_LIVE_PREVIEW && inType<=XFCE_OVERVIEW_WORKSPACES_LIVE_WINDOW_SIMPLE_DISPLAY_TYPE_ICON);

	priv=self->priv;

	/* Only set value if it changes */
	if(priv->displayType!=inType)
	{
		/* Set value */
		priv->displayType=inType;

		/* Setup window actor content */
		_xfce_overview_workspaces_live_window_simple_setup_content(self);

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesLiveWindowSimpleProperties[PROP_DISPLAY_TYPE]);
	}
}

/* Get/set flag for destruction on window close */
gboolean xfce_overview_workspaces_live_window_simple_get_destroy_on_close(XfceOverviewWorkspacesLiveWindowSimple *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WINDOW_SIMPLE(self), FALSE);

	return(self->priv->destroyOnClose);
}

void xfce_overview_workspaces_live_window_simple_set_destroy_on_close(XfceOverviewWorkspacesLiveWindowSimple *self, gboolean inDestroyOnClose)
{
	XfceOverviewWorkspacesLiveWindowSimplePrivate	*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WINDOW_SIMPLE(self));

	priv=self->priv;

	/* Only set value if it changes */
	if(priv->destroyOnClose!=inDestroyOnClose)
	{
		/* Set value */
		priv->destroyOnClose=inDestroyOnClose;

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesLiveWindowSimpleProperties[PROP_DESTROY_ON_CLOSE]);
	}
}
