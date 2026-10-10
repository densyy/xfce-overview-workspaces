/*
 * view: Abstract class for views, optional with scrollbars
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

#include <libxfce-overview-workspaces/view.h>

#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>

#include <libxfce-overview-workspaces/marshal.h>
#include <libxfce-overview-workspaces/image-content.h>
#include <libxfce-overview-workspaces/utils.h>
#include <libxfce-overview-workspaces/focus-manager.h>
#include <libxfce-overview-workspaces/viewpad.h>
#include <libxfce-overview-workspaces/core.h>
#include <libxfce-overview-workspaces/enums.h>


/* Define this class in GObject system */
struct _XfceOverviewWorkspacesViewPrivate
{
	/* Properties related */
	gchar					*viewID;

	gchar					*viewName;

	gchar					*viewIcon;
	ClutterContent			*viewIconImage;

	XfceOverviewWorkspacesViewFitMode	fitMode;

	gboolean				isEnabled;

	/* Layout manager */
	guint					signalChangedID;
};

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE(XfceOverviewWorkspacesView,
									xfce_overview_workspaces_view,
									XFCE_OVERVIEW_WORKSPACES_TYPE_ACTOR)

/* Properties */
enum
{
	PROP_0,

	PROP_VIEW_ID,
	PROP_VIEW_NAME,
	PROP_VIEW_ICON,

	PROP_VIEW_FIT_MODE,

	PROP_ENABLED,

	PROP_LAST
};

static GParamSpec* XfceOverviewWorkspacesViewProperties[PROP_LAST]={ 0, };

/* Signals */
enum
{
	SIGNAL_ACTIVATING,
	SIGNAL_ACTIVATED,
	SIGNAL_DEACTIVATING,
	SIGNAL_DEACTIVATED,

	SIGNAL_ENABLING,
	SIGNAL_ENABLED,
	SIGNAL_DISABLING,
	SIGNAL_DISABLED,

	SIGNAL_NAME_CHANGED,
	SIGNAL_ICON_CHANGED,

	SIGNAL_SCROLL_TO,
	SIGNAL_CHILD_NEEDS_SCROLL,
	SIGNAL_CHILD_ENSURE_VISIBLE,

	ACTION_VIEW_ACTIVATE,

	SIGNAL_LAST
};

static guint XfceOverviewWorkspacesViewSignals[SIGNAL_LAST]={ 0, };

/* IMPLEMENTATION: Private variables and methods */

/* Find viewpad which contains this view */
static XfceOverviewWorkspacesViewpad* _xfce_overview_workspaces_view_find_viewpad(XfceOverviewWorkspacesView *self)
{
	ClutterActor				*viewpad;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_VIEW(self), NULL);

	/* Iterate through parent actors and for each viewpad found
	 * check if it contains this view.
	 */
	viewpad=clutter_actor_get_parent(CLUTTER_ACTOR(self));
	while(viewpad)
	{
		/* Check if this parent actor is a viewpad and
		 * if it contains this view.
		 */
		if(XFCE_OVERVIEW_WORKSPACES_IS_VIEWPAD(viewpad) &&
			xfce_overview_workspaces_viewpad_has_view(XFCE_OVERVIEW_WORKSPACES_VIEWPAD(viewpad), self))
		{
			/* Viewpad found so return it */
			return(XFCE_OVERVIEW_WORKSPACES_VIEWPAD(viewpad));
		}

		/* Continue with next parent actor */
		viewpad=clutter_actor_get_parent(viewpad);
	}

	/* If we get here the viewpad could not be found so return NULL */
	return(NULL);
}

/* Action signal to close currently selected window was emitted */
static gboolean _xfce_overview_workspaces_view_activate(XfceOverviewWorkspacesView *self,
											XfceOverviewWorkspacesFocusable *inSource,
											const gchar *inAction,
											ClutterEvent *inEvent)
{
	XfceOverviewWorkspacesViewPrivate		*priv;
	XfceOverviewWorkspacesViewpad			*viewpad;
	XfceOverviewWorkspacesFocusManager		*focusManager;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_VIEW(self), CLUTTER_EVENT_PROPAGATE);

	priv=self->priv;

	/* Only enabled views can be activated */
	if(!priv->isEnabled) return(CLUTTER_EVENT_STOP);

	/* Find viewpad which contains this view */
	viewpad=_xfce_overview_workspaces_view_find_viewpad(self);
	if(!viewpad) return(CLUTTER_EVENT_STOP);

	/* Activate view at viewpad if this view is not the active one */
	if(xfce_overview_workspaces_viewpad_get_active_view(viewpad)!=self)
	{
		xfce_overview_workspaces_viewpad_set_active_view(viewpad, self);
	}

	/* Set focus to view if it has not the focus */
	focusManager=xfce_overview_workspaces_core_get_focus_manager(NULL);
	if(XFCE_OVERVIEW_WORKSPACES_IS_FOCUSABLE(self) &&
		!xfce_overview_workspaces_focus_manager_has_focus(focusManager, XFCE_OVERVIEW_WORKSPACES_FOCUSABLE(self)))
	{
		xfce_overview_workspaces_focus_manager_set_focus(focusManager, XFCE_OVERVIEW_WORKSPACES_FOCUSABLE(self));
	}
	g_object_unref(focusManager);

	/* Action handled */
	return(CLUTTER_EVENT_STOP);
}

/* This view was enabled */
static void _xfce_overview_workspaces_view_enabled(XfceOverviewWorkspacesView *self)
{
	XfceOverviewWorkspacesViewpad			*viewpad;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_VIEW(self));

	/* If view is not a child of a viewpad show view actor directly */
	viewpad=_xfce_overview_workspaces_view_find_viewpad(self);
	if(!viewpad)
	{
		clutter_actor_show(CLUTTER_ACTOR(self));
	}
}

/* This view was disabled */
static void _xfce_overview_workspaces_view_disabled(XfceOverviewWorkspacesView *self)
{
	XfceOverviewWorkspacesViewpad			*viewpad;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_VIEW(self));

	/* If view is not a child of a viewpad hide view actor directly */
	viewpad=_xfce_overview_workspaces_view_find_viewpad(self);
	if(!viewpad)
	{
		clutter_actor_hide(CLUTTER_ACTOR(self));
	}
}

/* Set view ID */
static void _xfce_overview_workspaces_view_set_id(XfceOverviewWorkspacesView *self, const gchar *inID)
{
	XfceOverviewWorkspacesViewPrivate	*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_VIEW(self));
	g_return_if_fail(inID && *inID);

	priv=self->priv;

	/* Set value if changed */
	if(g_strcmp0(priv->viewID, inID)!=0)
	{
		if(priv->viewID) g_free(priv->viewID);
		priv->viewID=g_strdup(inID);

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesViewProperties[PROP_VIEW_ID]);
	}
}

/* IMPLEMENTATION: GObject */

/* Dispose this object */
static void _xfce_overview_workspaces_view_dispose(GObject *inObject)
{
	XfceOverviewWorkspacesView			*self=XFCE_OVERVIEW_WORKSPACES_VIEW(inObject);
	XfceOverviewWorkspacesViewPrivate	*priv=self->priv;

	/* Release allocated resources */
	if(priv->viewID)
	{
		g_free(priv->viewID);
		priv->viewID=NULL;
	}

	if(priv->viewName)
	{
		g_free(priv->viewName);
		priv->viewName=NULL;
	}

	if(priv->viewIcon)
	{
		g_free(priv->viewIcon);
		priv->viewIcon=NULL;
	}

	if(priv->viewIconImage)
	{
		g_object_unref(priv->viewIconImage);
		priv->viewIconImage=NULL;
	}

	/* Call parent's class dispose method */
	G_OBJECT_CLASS(xfce_overview_workspaces_view_parent_class)->dispose(inObject);
}

/* Set/get properties */
static void _xfce_overview_workspaces_view_set_property(GObject *inObject,
											guint inPropID,
											const GValue *inValue,
											GParamSpec *inSpec)
{
	XfceOverviewWorkspacesView		*self=XFCE_OVERVIEW_WORKSPACES_VIEW(inObject);
	
	switch(inPropID)
	{
		case PROP_VIEW_ID:
			_xfce_overview_workspaces_view_set_id(self, g_value_get_string(inValue));
			break;

		case PROP_VIEW_NAME:
			xfce_overview_workspaces_view_set_name(self, g_value_get_string(inValue));
			break;
			
		case PROP_VIEW_ICON:
			xfce_overview_workspaces_view_set_icon(self, g_value_get_string(inValue));
			break;

		case PROP_VIEW_FIT_MODE:
			xfce_overview_workspaces_view_set_view_fit_mode(self, (XfceOverviewWorkspacesViewFitMode)g_value_get_enum(inValue));
			break;

		case PROP_ENABLED:
			xfce_overview_workspaces_view_set_enabled(self, g_value_get_boolean(inValue));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(inObject, inPropID, inSpec);
			break;
	}
}

static void _xfce_overview_workspaces_view_get_property(GObject *inObject,
											guint inPropID,
											GValue *outValue,
											GParamSpec *inSpec)
{
	XfceOverviewWorkspacesView		*self=XFCE_OVERVIEW_WORKSPACES_VIEW(inObject);

	switch(inPropID)
	{
		case PROP_VIEW_ID:
			g_value_set_string(outValue, self->priv->viewID);
			break;

		case PROP_VIEW_NAME:
			g_value_set_string(outValue, self->priv->viewName);
			break;

		case PROP_VIEW_ICON:
			g_value_set_string(outValue, self->priv->viewIcon);
			break;

		case PROP_VIEW_FIT_MODE:
			g_value_set_enum(outValue, self->priv->fitMode);
			break;

		case PROP_ENABLED:
			g_value_set_boolean(outValue, self->priv->isEnabled);
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
static void xfce_overview_workspaces_view_class_init(XfceOverviewWorkspacesViewClass *klass)
{
	XfceOverviewWorkspacesActorClass	*actorClass=XFCE_OVERVIEW_WORKSPACES_ACTOR_CLASS(klass);
	GObjectClass			*gobjectClass=G_OBJECT_CLASS(klass);

	/* Override functions */
	gobjectClass->set_property=_xfce_overview_workspaces_view_set_property;
	gobjectClass->get_property=_xfce_overview_workspaces_view_get_property;
	gobjectClass->dispose=_xfce_overview_workspaces_view_dispose;

	klass->view_activate=_xfce_overview_workspaces_view_activate;
	klass->enabled=_xfce_overview_workspaces_view_enabled;
	klass->disabled=_xfce_overview_workspaces_view_disabled;

	/* Define properties */
	XfceOverviewWorkspacesViewProperties[PROP_VIEW_ID]=
		g_param_spec_string("view-id",
							"View ID",
							"The internal ID used to register this type of view",
							NULL,
							G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY);

	XfceOverviewWorkspacesViewProperties[PROP_VIEW_NAME]=
		g_param_spec_string("view-name",
							"View name",
							"Name of view used to display",
							NULL,
							G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	XfceOverviewWorkspacesViewProperties[PROP_VIEW_ICON]=
		g_param_spec_string("view-icon",
							"View icon",
							"Icon of view used to display. Icon name can be a themed icon name or file name",
							NULL,
							G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	XfceOverviewWorkspacesViewProperties[PROP_VIEW_FIT_MODE]=
		g_param_spec_enum("view-fit-mode",
							"View fit mode",
							"Defines if view should be fit into viewpad and in which directions it should fit into it",
							XFCE_OVERVIEW_WORKSPACES_TYPE_VIEW_FIT_MODE,
							XFCE_OVERVIEW_WORKSPACES_VIEW_FIT_MODE_NONE,
							G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	XfceOverviewWorkspacesViewProperties[PROP_ENABLED]=
		g_param_spec_boolean("enabled",
								"Enabled",
								"This flag indicates if is view is enabled and activable",
								TRUE,
								G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(gobjectClass, PROP_LAST, XfceOverviewWorkspacesViewProperties);

	/* Define stylable properties */
	xfce_overview_workspaces_actor_install_stylable_property(actorClass, XfceOverviewWorkspacesViewProperties[PROP_VIEW_ICON]);

	/* Define signals */
	XfceOverviewWorkspacesViewSignals[SIGNAL_ACTIVATING]=
		g_signal_new("activating",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST,
						G_STRUCT_OFFSET(XfceOverviewWorkspacesViewClass, activating),
						NULL,
						NULL,
						g_cclosure_marshal_VOID__VOID,
						G_TYPE_NONE,
						0);

	XfceOverviewWorkspacesViewSignals[SIGNAL_ACTIVATED]=
		g_signal_new("activated",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST,
						G_STRUCT_OFFSET(XfceOverviewWorkspacesViewClass, activated),
						NULL,
						NULL,
						g_cclosure_marshal_VOID__VOID,
						G_TYPE_NONE,
						0);

	XfceOverviewWorkspacesViewSignals[SIGNAL_DEACTIVATING]=
		g_signal_new("deactivating",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST,
						G_STRUCT_OFFSET(XfceOverviewWorkspacesViewClass, deactivating),
						NULL,
						NULL,
						g_cclosure_marshal_VOID__VOID,
						G_TYPE_NONE,
						0);

	XfceOverviewWorkspacesViewSignals[SIGNAL_DEACTIVATED]=
		g_signal_new("deactivated",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST,
						G_STRUCT_OFFSET(XfceOverviewWorkspacesViewClass, deactivated),
						NULL,
						NULL,
						g_cclosure_marshal_VOID__VOID,
						G_TYPE_NONE,
						0);

	XfceOverviewWorkspacesViewSignals[SIGNAL_ENABLING]=
		g_signal_new("enabling",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST,
						G_STRUCT_OFFSET(XfceOverviewWorkspacesViewClass, enabling),
						NULL,
						NULL,
						g_cclosure_marshal_VOID__VOID,
						G_TYPE_NONE,
						0);

	XfceOverviewWorkspacesViewSignals[SIGNAL_ENABLED]=
		g_signal_new("enabled",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST,
						G_STRUCT_OFFSET(XfceOverviewWorkspacesViewClass, enabled),
						NULL,
						NULL,
						g_cclosure_marshal_VOID__VOID,
						G_TYPE_NONE,
						0);

	XfceOverviewWorkspacesViewSignals[SIGNAL_DISABLING]=
		g_signal_new("disabling",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST,
						G_STRUCT_OFFSET(XfceOverviewWorkspacesViewClass, disabling),
						NULL,
						NULL,
						g_cclosure_marshal_VOID__VOID,
						G_TYPE_NONE,
						0);

	XfceOverviewWorkspacesViewSignals[SIGNAL_DISABLED]=
		g_signal_new("disabled",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST,
						G_STRUCT_OFFSET(XfceOverviewWorkspacesViewClass, disabled),
						NULL,
						NULL,
						g_cclosure_marshal_VOID__VOID,
						G_TYPE_NONE,
						0);

	XfceOverviewWorkspacesViewSignals[SIGNAL_NAME_CHANGED]=
		g_signal_new("name-changed",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST,
						G_STRUCT_OFFSET(XfceOverviewWorkspacesViewClass, name_changed),
						NULL,
						NULL,
						g_cclosure_marshal_VOID__STRING,
						G_TYPE_NONE,
						1,
						G_TYPE_STRING);

	XfceOverviewWorkspacesViewSignals[SIGNAL_ICON_CHANGED]=
		g_signal_new("icon-changed",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST,
						G_STRUCT_OFFSET(XfceOverviewWorkspacesViewClass, icon_changed),
						NULL,
						NULL,
						g_cclosure_marshal_VOID__OBJECT,
						G_TYPE_NONE,
						1,
						CLUTTER_TYPE_IMAGE);

	XfceOverviewWorkspacesViewSignals[SIGNAL_SCROLL_TO]=
		g_signal_new("scroll-to",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
						G_STRUCT_OFFSET(XfceOverviewWorkspacesViewClass, scroll_to),
						NULL,
						NULL,
						_xfce_overview_workspaces_marshal_VOID__FLOAT_FLOAT,
						G_TYPE_NONE,
						2,
						G_TYPE_FLOAT,
						G_TYPE_FLOAT);

	XfceOverviewWorkspacesViewSignals[SIGNAL_CHILD_NEEDS_SCROLL]=
		g_signal_new("child-needs-scroll",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
						G_STRUCT_OFFSET(XfceOverviewWorkspacesViewClass, child_needs_scroll),
						NULL,
						NULL,
						_xfce_overview_workspaces_marshal_BOOLEAN__OBJECT,
						G_TYPE_BOOLEAN,
						1,
						CLUTTER_TYPE_ACTOR);

	XfceOverviewWorkspacesViewSignals[SIGNAL_CHILD_ENSURE_VISIBLE]=
		g_signal_new("child-ensure-visible",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
						G_STRUCT_OFFSET(XfceOverviewWorkspacesViewClass, child_ensure_visible),
						NULL,
						NULL,
						g_cclosure_marshal_VOID__OBJECT,
						G_TYPE_NONE,
						1,
						CLUTTER_TYPE_ACTOR);

	/* Define actions */
	XfceOverviewWorkspacesViewSignals[ACTION_VIEW_ACTIVATE]=
		g_signal_new("view-activate",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
						G_STRUCT_OFFSET(XfceOverviewWorkspacesViewClass, view_activate),
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
static void xfce_overview_workspaces_view_init(XfceOverviewWorkspacesView *self)
{
	XfceOverviewWorkspacesViewPrivate	*priv;

	priv=self->priv=xfce_overview_workspaces_view_get_instance_private(self);

	/* Set up default values */
	priv->viewName=NULL;
	priv->viewIcon=NULL;
	priv->viewIconImage=NULL;
	priv->fitMode=XFCE_OVERVIEW_WORKSPACES_VIEW_FIT_MODE_NONE;
	priv->isEnabled=TRUE;

	/* Set up actor */
	clutter_actor_set_reactive(CLUTTER_ACTOR(self), TRUE);
	xfce_overview_workspaces_actor_invalidate(XFCE_OVERVIEW_WORKSPACES_ACTOR(self));
}

/* IMPLEMENTATION: Public API */

/* Get view ID */
const gchar* xfce_overview_workspaces_view_get_id(XfceOverviewWorkspacesView *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_VIEW(self), NULL);

	return(self->priv->viewID);
}

/* Check if view has requested ID */
gboolean xfce_overview_workspaces_view_has_id(XfceOverviewWorkspacesView *self, const gchar *inID)
{
	XfceOverviewWorkspacesViewPrivate	*priv;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_VIEW(self), FALSE);
	g_return_val_if_fail(inID && *inID, FALSE);

	priv=self->priv;

	/* Check if requested ID matches the ID of this view */
	if(g_strcmp0(priv->viewID, inID)!=0) return(FALSE);

	/* If we get here the requested ID matches view's ID */
	return(TRUE);
}

/* Get/set name of view */
const gchar* xfce_overview_workspaces_view_get_name(XfceOverviewWorkspacesView *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_VIEW(self), NULL);

	return(self->priv->viewName);
}

void xfce_overview_workspaces_view_set_name(XfceOverviewWorkspacesView *self, const gchar *inName)
{
	XfceOverviewWorkspacesViewPrivate	*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_VIEW(self));
	g_return_if_fail(inName!=NULL);

	priv=self->priv;

	/* Set value if changed */
	if(g_strcmp0(priv->viewName, inName)!=0)
	{
		if(priv->viewName) g_free(priv->viewName);
		priv->viewName=g_strdup(inName);

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesViewProperties[PROP_VIEW_NAME]);
		g_signal_emit(self, XfceOverviewWorkspacesViewSignals[SIGNAL_NAME_CHANGED], 0, priv->viewName);
	}
}

/* Get/set icon of view */
const gchar* xfce_overview_workspaces_view_get_icon(XfceOverviewWorkspacesView *self)
{
  g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_VIEW(self), NULL);

  return(self->priv->viewIcon);
}

void xfce_overview_workspaces_view_set_icon(XfceOverviewWorkspacesView *self, const gchar *inIcon)
{
	XfceOverviewWorkspacesViewPrivate	*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_VIEW(self));
	g_return_if_fail(inIcon!=NULL);

	priv=self->priv;

	/* Set value if changed */
	if(g_strcmp0(priv->viewIcon, inIcon)!=0)
	{
		/* Set new icon name */
		if(priv->viewIcon) g_free(priv->viewIcon);
		priv->viewIcon=g_strdup(inIcon);

		/* Set new icon */
		if(priv->viewIconImage) g_object_unref(priv->viewIconImage);
		priv->viewIconImage=xfce_overview_workspaces_image_content_new_for_icon_name(priv->viewIcon, 64.0f);

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesViewProperties[PROP_VIEW_ICON]);
		g_signal_emit(self, XfceOverviewWorkspacesViewSignals[SIGNAL_ICON_CHANGED], 0, CLUTTER_IMAGE(priv->viewIconImage));
	}
}

/* Get/set fit mode of view */
XfceOverviewWorkspacesViewFitMode xfce_overview_workspaces_view_get_view_fit_mode(XfceOverviewWorkspacesView *self)
{
  g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_VIEW(self), XFCE_OVERVIEW_WORKSPACES_VIEW_FIT_MODE_NONE);

  return(self->priv->fitMode);
}

void xfce_overview_workspaces_view_set_view_fit_mode(XfceOverviewWorkspacesView *self, XfceOverviewWorkspacesViewFitMode inFitMode)
{
	XfceOverviewWorkspacesViewPrivate	*priv;
	XfceOverviewWorkspacesViewClass	*klass;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_VIEW(self));

	priv=self->priv;
	klass=XFCE_OVERVIEW_WORKSPACES_VIEW_GET_CLASS(self);

	/* Set value if changed */
	if(priv->fitMode!=inFitMode)
	{
		/* Set new fit mode */
		priv->fitMode=inFitMode;

		/* Call virtual function for setting fit mode */
		if(klass->set_view_fit_mode) klass->set_view_fit_mode(self, inFitMode);

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesViewProperties[PROP_VIEW_FIT_MODE]);
	}
}

/* Get/set enabled state of view */
gboolean xfce_overview_workspaces_view_get_enabled(XfceOverviewWorkspacesView *self)
{
  g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_VIEW(self), FALSE);

  return(self->priv->isEnabled);
}

void xfce_overview_workspaces_view_set_enabled(XfceOverviewWorkspacesView *self, gboolean inIsEnabled)
{
	XfceOverviewWorkspacesViewPrivate	*priv;
	guint					signalBeforeID, signalAfterID;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_VIEW(self));

	priv=self->priv;

	/* Set value if changed */
	if(priv->isEnabled!=inIsEnabled)
	{
		/* Get signal ID to emit depending on enabled state */
		signalBeforeID=(inIsEnabled==TRUE ? SIGNAL_ENABLING : SIGNAL_DISABLING);
		signalAfterID=(inIsEnabled==TRUE ? SIGNAL_ENABLED : SIGNAL_DISABLED);

		/* Set new enabled state */
		g_signal_emit(self, XfceOverviewWorkspacesViewSignals[signalBeforeID], 0, self);

		priv->isEnabled=inIsEnabled;

		g_signal_emit(self, XfceOverviewWorkspacesViewSignals[signalAfterID], 0, self);

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesViewProperties[PROP_ENABLED]);
	}
}

/* Scroll view to requested coordinates */
void xfce_overview_workspaces_view_scroll_to(XfceOverviewWorkspacesView *self, gfloat inX, gfloat inY)
{
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_VIEW(self));

	g_signal_emit(self, XfceOverviewWorkspacesViewSignals[SIGNAL_SCROLL_TO], 0, inX, inY);
}

/* Determine if scrolling is needed to get requested actor visible */
gboolean xfce_overview_workspaces_view_child_needs_scroll(XfceOverviewWorkspacesView *self, ClutterActor *inActor)
{
	gboolean			result;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_VIEW(self), FALSE);
	g_return_val_if_fail(CLUTTER_IS_ACTOR(inActor), FALSE);

	result=FALSE;

	/* Only emit signal if given actor is a child of this view */
	if(clutter_actor_contains(CLUTTER_ACTOR(self), inActor))
	{
		g_signal_emit(self, XfceOverviewWorkspacesViewSignals[SIGNAL_CHILD_NEEDS_SCROLL], 0, inActor, &result);
	}

	return(result);
}

/* Ensure that a child of this view is visible by scrolling if needed */
void xfce_overview_workspaces_view_child_ensure_visible(XfceOverviewWorkspacesView *self, ClutterActor *inActor)
{
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_VIEW(self));
	g_return_if_fail(CLUTTER_IS_ACTOR(inActor));

	/* Only emit signal if given actor is a child of this view */
	if(clutter_actor_contains(CLUTTER_ACTOR(self), inActor))
	{
		g_signal_emit(self, XfceOverviewWorkspacesViewSignals[SIGNAL_CHILD_ENSURE_VISIBLE], 0, inActor);
	}
}

/* Determine if view has the focus */
gboolean xfce_overview_workspaces_view_has_focus(XfceOverviewWorkspacesView *self)
{
	XfceOverviewWorkspacesViewPrivate		*priv;
	XfceOverviewWorkspacesFocusManager		*focusManager;
	XfceOverviewWorkspacesViewpad			*viewpad;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_VIEW(self), FALSE);

	priv=self->priv;

	/* The view can only have the focus if this view is enabled, active and
	 * has the current focus.
	 */
	if(!priv->isEnabled)
	{
		return(FALSE);
	}

	viewpad=_xfce_overview_workspaces_view_find_viewpad(self);
	if(!viewpad)
	{
		return(FALSE);
	}

	if(xfce_overview_workspaces_viewpad_get_active_view(XFCE_OVERVIEW_WORKSPACES_VIEWPAD(viewpad))!=self)
	{
		return(FALSE);
	}

	focusManager=xfce_overview_workspaces_core_get_focus_manager(NULL);
	if(!XFCE_OVERVIEW_WORKSPACES_IS_FOCUSABLE(self) ||
		!xfce_overview_workspaces_focus_manager_has_focus(focusManager, XFCE_OVERVIEW_WORKSPACES_FOCUSABLE(self)))
	{
		g_object_unref(focusManager);
		return(FALSE);
	}

	/* Release allocated resources */
	g_object_unref(focusManager);

	/* All tests passed so this view has the focus */
	return(TRUE);
}
