/*
 * emblem-effect: Draws an emblem on top of an actor
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

#define COGL_ENABLE_EXPERIMENTAL_API
#define CLUTTER_ENABLE_EXPERIMENTAL_API

#include <libxfce-overview-workspaces/emblem-effect.h>

#include <glib/gi18n-lib.h>
#include <math.h>
#include <cogl/cogl.h>

#include <libxfce-overview-workspaces/image-content.h>
#include <libxfce-overview-workspaces/enums.h>
#include <libxfce-overview-workspaces/debug.h>


/* Define this class in GObject system */
struct _XfceOverviewWorkspacesEmblemEffectPrivate
{
	/* Properties related */
	gchar						*iconName;
	gint						iconSize;
	gfloat						padding;
	gfloat						xAlign;
	gfloat						yAlign;
	XfceOverviewWorkspacesAnchorPoint		anchorPoint;

	/* Instance related */
	ClutterContent				*icon;
	guint						loadSuccessSignalID;
	guint						loadFailedSignalID;

	CoglPipeline				*pipeline;
};

G_DEFINE_TYPE_WITH_PRIVATE(XfceOverviewWorkspacesEmblemEffect,
							xfce_overview_workspaces_emblem_effect,
							CLUTTER_TYPE_EFFECT)

/* Properties */
enum
{
	PROP_0,

	PROP_ICON_NAME,
	PROP_ICON_SIZE,
	PROP_PADDING,
	PROP_X_ALIGN,
	PROP_Y_ALIGN,
	PROP_ANCHOR_POINT,

	PROP_LAST
};

static GParamSpec* XfceOverviewWorkspacesEmblemEffectProperties[PROP_LAST]={ 0, };

/* IMPLEMENTATION: Private variables and methods */
static CoglPipeline		*_xfce_overview_workspaces_emblem_effect_base_pipeline=NULL;

/* Icon image was loaded */
static void _xfce_overview_workspaces_emblem_effect_on_load_finished(XfceOverviewWorkspacesEmblemEffect *self, gpointer inUserData)
{
	XfceOverviewWorkspacesEmblemEffectPrivate		*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_EMBLEM_EFFECT(self));

	priv=self->priv;

	/* Disconnect signal handlers */
	if(priv->loadSuccessSignalID)
	{
		g_signal_handler_disconnect(priv->icon, priv->loadSuccessSignalID);
		priv->loadSuccessSignalID=0;
	}

	if(priv->loadFailedSignalID)
	{
		g_signal_handler_disconnect(priv->icon, priv->loadFailedSignalID);
		priv->loadFailedSignalID=0;
	}

	/* Set image at pipeline */
	cogl_pipeline_set_layer_texture(priv->pipeline,
									0,
									clutter_image_get_texture(CLUTTER_IMAGE(priv->icon)));

	/* Invalidate effect to get it redrawn */
	clutter_effect_queue_repaint(CLUTTER_EFFECT(self));
}

/* IMPLEMENTATION: ClutterEffect */

/* Draw effect after actor was drawn */
static void _xfce_overview_workspaces_emblem_effect_paint(ClutterEffect *inEffect, ClutterEffectPaintFlags inFlags)
{
	XfceOverviewWorkspacesEmblemEffect					*self;
	XfceOverviewWorkspacesEmblemEffectPrivate			*priv;
	ClutterActor							*target;
	gfloat									actorWidth;
	gfloat									actorHeight;
	ClutterActorBox							actorBox;
	ClutterActorBox							rectangleBox;
	XfceOverviewWorkspacesImageContentLoadingState		loadingState;
	gfloat									textureWidth;
	gfloat									textureHeight;
	ClutterActorBox							textureCoordBox;
	gfloat									offset;
	gfloat									oversize;
	CoglFramebuffer							*framebuffer;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_EMBLEM_EFFECT(inEffect));

	self=XFCE_OVERVIEW_WORKSPACES_EMBLEM_EFFECT(inEffect);
	priv=self->priv;

	/* Chain to the next item in the paint sequence */
	target=clutter_actor_meta_get_actor(CLUTTER_ACTOR_META(self));
	clutter_actor_continue_paint(target);

	/* If no icon name is set do not apply this effect */
	if(!priv->iconName) return;

	/* Load image if not done yet */
	if(!priv->icon)
	{
		/* Get image from cache */
		priv->icon=xfce_overview_workspaces_image_content_new_for_icon_name(priv->iconName, priv->iconSize);

		/* Ensure image is being loaded */
		loadingState=xfce_overview_workspaces_image_content_get_state(XFCE_OVERVIEW_WORKSPACES_IMAGE_CONTENT(priv->icon));
		if(loadingState==XFCE_OVERVIEW_WORKSPACES_IMAGE_CONTENT_LOADING_STATE_NONE ||
			loadingState==XFCE_OVERVIEW_WORKSPACES_IMAGE_CONTENT_LOADING_STATE_LOADING)
		{
			/* Connect signals just because we need to wait for image being loaded */
			priv->loadSuccessSignalID=g_signal_connect_swapped(priv->icon,
																"loaded",
																G_CALLBACK(_xfce_overview_workspaces_emblem_effect_on_load_finished),
																self);
			priv->loadFailedSignalID=g_signal_connect_swapped(priv->icon,
																"loading-failed",
																G_CALLBACK(_xfce_overview_workspaces_emblem_effect_on_load_finished),
																self);

			/* If image is not being loaded currently enforce loading now */
			if(loadingState==XFCE_OVERVIEW_WORKSPACES_IMAGE_CONTENT_LOADING_STATE_NONE)
			{
				xfce_overview_workspaces_image_content_force_load(XFCE_OVERVIEW_WORKSPACES_IMAGE_CONTENT(priv->icon));
			}
		}
			else
			{
				/* Image is already loaded so set image at pipeline */
				cogl_pipeline_set_layer_texture(priv->pipeline,
												0,
												clutter_image_get_texture(CLUTTER_IMAGE(priv->icon)));
			}
	}

	/* Get actor size and apply padding. If actor width or height will drop
	 * to zero or below then the emblem could not be drawn and we return here.
	 */
	clutter_actor_get_content_box(target, &actorBox);
	actorBox.x1+=priv->padding;
	actorBox.x2-=priv->padding;
	actorBox.y1+=priv->padding;
	actorBox.y2-=priv->padding;

	if(actorBox.x2<=actorBox.x1 ||
		actorBox.y2<=actorBox.y1)
	{
		XFCE_OVERVIEW_WORKSPACES_DEBUG(self, ACTOR,
							"Will not draw emblem '%s' because width or height of actor is zero or below after padding was applied.",
							priv->iconName);
		return;
	}

	actorWidth=actorBox.x2-actorBox.x1;
	actorHeight=actorBox.y2-actorBox.y1;

	/* Get texture size */
	clutter_content_get_preferred_size(CLUTTER_CONTENT(priv->icon), &textureWidth, &textureHeight);
	clutter_actor_box_init(&textureCoordBox, 0.0f, 0.0f, 1.0f, 1.0f);

	/* Get boundary in X axis depending on anchorPoint and scaled width */
	offset=(priv->xAlign*actorWidth);
	switch(priv->anchorPoint)
	{
		/* Align to left boundary.
		 * This is also the default if anchor point is none or undefined.
		 */
		default:
		case XFCE_OVERVIEW_WORKSPACES_ANCHOR_POINT_NONE:
		case XFCE_OVERVIEW_WORKSPACES_ANCHOR_POINT_WEST:
		case XFCE_OVERVIEW_WORKSPACES_ANCHOR_POINT_NORTH_WEST:
		case XFCE_OVERVIEW_WORKSPACES_ANCHOR_POINT_SOUTH_WEST:
			break;

		/* Align to center of X axis */
		case XFCE_OVERVIEW_WORKSPACES_ANCHOR_POINT_CENTER:
		case XFCE_OVERVIEW_WORKSPACES_ANCHOR_POINT_NORTH:
		case XFCE_OVERVIEW_WORKSPACES_ANCHOR_POINT_SOUTH:
			offset-=(textureWidth/2.0f);
			break;

		/* Align to right boundary */
		case XFCE_OVERVIEW_WORKSPACES_ANCHOR_POINT_EAST:
		case XFCE_OVERVIEW_WORKSPACES_ANCHOR_POINT_NORTH_EAST:
		case XFCE_OVERVIEW_WORKSPACES_ANCHOR_POINT_SOUTH_EAST:
			offset-=textureWidth;
			break;
	}

	/* Set boundary in X axis */
	rectangleBox.x1=actorBox.x1+offset;
	rectangleBox.x2=rectangleBox.x1+textureWidth;

	/* Clip texture in X axis if it does not fit into allocation */
	if(rectangleBox.x1<actorBox.x1)
	{
		oversize=actorBox.x1-rectangleBox.x1;
		textureCoordBox.x1=oversize/textureWidth;
		rectangleBox.x1=actorBox.x1;
	}

	if(rectangleBox.x2>actorBox.x2)
	{
		oversize=rectangleBox.x2-actorBox.x2;
		textureCoordBox.x2=1.0f-(oversize/textureWidth);
		rectangleBox.x2=actorBox.x2;
	}

	/* Get boundary in Y axis depending on anchorPoint and scaled width */
	offset=(priv->yAlign*actorHeight);
	switch(priv->anchorPoint)
	{
		/* Align to upper boundary.
		 * This is also the default if anchor point is none or undefined.
		 */
		default:
		case XFCE_OVERVIEW_WORKSPACES_ANCHOR_POINT_NONE:
		case XFCE_OVERVIEW_WORKSPACES_ANCHOR_POINT_NORTH:
		case XFCE_OVERVIEW_WORKSPACES_ANCHOR_POINT_NORTH_WEST:
		case XFCE_OVERVIEW_WORKSPACES_ANCHOR_POINT_NORTH_EAST:
			break;

		/* Align to center of Y axis */
		case XFCE_OVERVIEW_WORKSPACES_ANCHOR_POINT_CENTER:
		case XFCE_OVERVIEW_WORKSPACES_ANCHOR_POINT_WEST:
		case XFCE_OVERVIEW_WORKSPACES_ANCHOR_POINT_EAST:
			offset-=(textureHeight/2.0f);
			break;

		/* Align to lower boundary */
		case XFCE_OVERVIEW_WORKSPACES_ANCHOR_POINT_SOUTH:
		case XFCE_OVERVIEW_WORKSPACES_ANCHOR_POINT_SOUTH_WEST:
		case XFCE_OVERVIEW_WORKSPACES_ANCHOR_POINT_SOUTH_EAST:
			offset-=textureHeight;
			break;
	}

	/* Set boundary in Y axis */
	rectangleBox.y1=actorBox.y1+offset;
	rectangleBox.y2=rectangleBox.y1+textureHeight;

	/* Clip texture in Y axis if it does not fit into allocation */
	if(rectangleBox.y1<actorBox.y1)
	{
		oversize=actorBox.y1-rectangleBox.y1;
		textureCoordBox.y1=oversize/textureHeight;
		rectangleBox.y1=actorBox.y1;
	}

	if(rectangleBox.y2>actorBox.y2)
	{
		oversize=rectangleBox.y2-actorBox.y2;
		textureCoordBox.y2=1.0f-(oversize/textureHeight);
		rectangleBox.y2=actorBox.y2;
	}

	/* Draw icon if image was loaded */
	loadingState=xfce_overview_workspaces_image_content_get_state(XFCE_OVERVIEW_WORKSPACES_IMAGE_CONTENT(priv->icon));
	if(loadingState!=XFCE_OVERVIEW_WORKSPACES_IMAGE_CONTENT_LOADING_STATE_LOADED_SUCCESSFULLY &&
		loadingState!=XFCE_OVERVIEW_WORKSPACES_IMAGE_CONTENT_LOADING_STATE_LOADED_FAILED)
	{
		XFCE_OVERVIEW_WORKSPACES_DEBUG(self, ACTOR,
							"Emblem image '%s' is still being loaded at %s",
							priv->iconName,
							G_OBJECT_TYPE_NAME(inEffect));
		return;
	}

	framebuffer=cogl_get_draw_framebuffer();
	cogl_framebuffer_draw_textured_rectangle(framebuffer,
												priv->pipeline,
												rectangleBox.x1, rectangleBox.y1,
												rectangleBox.x2, rectangleBox.y2,
												textureCoordBox.x1, textureCoordBox.y1,
												textureCoordBox.x2, textureCoordBox.y2);
}

/* IMPLEMENTATION: GObject */

/* Dispose this object */
static void _xfce_overview_workspaces_emblem_effect_dispose(GObject *inObject)
{
	/* Release allocated variables */
	XfceOverviewWorkspacesEmblemEffect			*self=XFCE_OVERVIEW_WORKSPACES_EMBLEM_EFFECT(inObject);
	XfceOverviewWorkspacesEmblemEffectPrivate	*priv=self->priv;

	if(priv->pipeline)
	{
		cogl_object_unref(priv->pipeline);
		priv->pipeline=NULL;
	}

	if(priv->icon)
	{
		/* Disconnect any connected signal handler */
		if(priv->loadSuccessSignalID)
		{
			g_signal_handler_disconnect(priv->icon, priv->loadSuccessSignalID);
			priv->loadSuccessSignalID=0;
		}

		if(priv->loadFailedSignalID)
		{
			g_signal_handler_disconnect(priv->icon, priv->loadFailedSignalID);
			priv->loadFailedSignalID=0;
		}

		/* Release image itself */
		g_object_unref(priv->icon);
		priv->icon=NULL;
	}

	if(priv->iconName)
	{
		g_free(priv->iconName);
		priv->iconName=NULL;
	}

	/* Call parent's class dispose method */
	G_OBJECT_CLASS(xfce_overview_workspaces_emblem_effect_parent_class)->dispose(inObject);
}

/* Set/get properties */
static void _xfce_overview_workspaces_emblem_effect_set_property(GObject *inObject,
														guint inPropID,
														const GValue *inValue,
														GParamSpec *inSpec)
{
	XfceOverviewWorkspacesEmblemEffect			*self=XFCE_OVERVIEW_WORKSPACES_EMBLEM_EFFECT(inObject);

	switch(inPropID)
	{
		case PROP_ICON_NAME:
			xfce_overview_workspaces_emblem_effect_set_icon_name(self, g_value_get_string(inValue));
			break;

		case PROP_ICON_SIZE:
			xfce_overview_workspaces_emblem_effect_set_icon_size(self, g_value_get_int(inValue));
			break;

		case PROP_PADDING:
			xfce_overview_workspaces_emblem_effect_set_padding(self, g_value_get_float(inValue));
			break;

		case PROP_X_ALIGN:
			xfce_overview_workspaces_emblem_effect_set_x_align(self, g_value_get_float(inValue));
			break;

		case PROP_Y_ALIGN:
			xfce_overview_workspaces_emblem_effect_set_y_align(self, g_value_get_float(inValue));
			break;

		case PROP_ANCHOR_POINT:
			xfce_overview_workspaces_emblem_effect_set_anchor_point(self, g_value_get_enum(inValue));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(inObject, inPropID, inSpec);
			break;
	}
}

static void _xfce_overview_workspaces_emblem_effect_get_property(GObject *inObject,
														guint inPropID,
														GValue *outValue,
														GParamSpec *inSpec)
{
	XfceOverviewWorkspacesEmblemEffect			*self=XFCE_OVERVIEW_WORKSPACES_EMBLEM_EFFECT(inObject);
	XfceOverviewWorkspacesEmblemEffectPrivate		*priv=self->priv;

	switch(inPropID)
	{
		case PROP_ICON_NAME:
			g_value_set_string(outValue, priv->iconName);
			break;

		case PROP_ICON_SIZE:
			g_value_set_int(outValue, priv->iconSize);
			break;

		case PROP_PADDING:
			g_value_set_float(outValue, priv->padding);
			break;

		case PROP_X_ALIGN:
			g_value_set_float(outValue, priv->xAlign);
			break;

		case PROP_Y_ALIGN:
			g_value_set_float(outValue, priv->yAlign);
			break;

		case PROP_ANCHOR_POINT:
			g_value_set_enum(outValue, priv->anchorPoint);
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
static void xfce_overview_workspaces_emblem_effect_class_init(XfceOverviewWorkspacesEmblemEffectClass *klass)
{
	ClutterEffectClass				*effectClass=CLUTTER_EFFECT_CLASS(klass);
	GObjectClass					*gobjectClass=G_OBJECT_CLASS(klass);

	/* Override functions */
	gobjectClass->dispose=_xfce_overview_workspaces_emblem_effect_dispose;
	gobjectClass->set_property=_xfce_overview_workspaces_emblem_effect_set_property;
	gobjectClass->get_property=_xfce_overview_workspaces_emblem_effect_get_property;

	effectClass->paint=_xfce_overview_workspaces_emblem_effect_paint;

	/* Define properties */
	XfceOverviewWorkspacesEmblemEffectProperties[PROP_ICON_NAME]=
		g_param_spec_string("icon-name",
							"Icon name",
							"Themed icon name or file name of icon",
							"",
							G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	XfceOverviewWorkspacesEmblemEffectProperties[PROP_ICON_SIZE]=
		g_param_spec_int("icon-size",
							"Icon size",
							"Size of icon",
							1, G_MAXINT,
							16,
							G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	XfceOverviewWorkspacesEmblemEffectProperties[PROP_PADDING]=
		g_param_spec_float("padding",
							"Padding",
							"Padding around emblem",
							0.0f, G_MAXFLOAT,
							0.0f,
							G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	XfceOverviewWorkspacesEmblemEffectProperties[PROP_X_ALIGN]=
		g_param_spec_float("x-align",
							"X align",
							"The alignment of emblem on the X axis within the allocation in normalized coordinate between 0 and 1",
							0.0f, 1.0f,
							0.0f,
							G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	XfceOverviewWorkspacesEmblemEffectProperties[PROP_Y_ALIGN]=
		g_param_spec_float("y-align",
							"Y align",
							"The alignment of emblem on the Y axis within the allocation in normalized coordinate between 0 and 1",
							0.0f, 1.0f,
							0.0f,
							G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	XfceOverviewWorkspacesEmblemEffectProperties[PROP_ANCHOR_POINT]=
		g_param_spec_enum("anchor-point",
							"Anchor point",
							"The anchor point of emblem",
							XFCE_OVERVIEW_WORKSPACES_TYPE_ANCHOR_POINT,
							XFCE_OVERVIEW_WORKSPACES_ANCHOR_POINT_NONE,
							G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(gobjectClass, PROP_LAST, XfceOverviewWorkspacesEmblemEffectProperties);
}

/* Object initialization
 * Create private structure and set up default values
 */
static void xfce_overview_workspaces_emblem_effect_init(XfceOverviewWorkspacesEmblemEffect *self)
{
	XfceOverviewWorkspacesEmblemEffectPrivate	*priv;

	priv=self->priv=xfce_overview_workspaces_emblem_effect_get_instance_private(self);

	/* Set up default values */
	priv->iconName=NULL;
	priv->iconSize=16;
	priv->padding=0.0f;
	priv->xAlign=0.0f;
	priv->yAlign=0.0f;
	priv->anchorPoint=XFCE_OVERVIEW_WORKSPACES_ANCHOR_POINT_NONE;
	priv->icon=NULL;
	priv->loadSuccessSignalID=0;
	priv->loadFailedSignalID=0;

	/* Set up pipeline */
	if(G_UNLIKELY(!_xfce_overview_workspaces_emblem_effect_base_pipeline))
    {
		CoglContext					*context;

		/* Get context to create base pipeline */
		context=clutter_backend_get_cogl_context(clutter_get_default_backend());

		/* Create base pipeline */
		_xfce_overview_workspaces_emblem_effect_base_pipeline=cogl_pipeline_new(context);
		cogl_pipeline_set_layer_null_texture(_xfce_overview_workspaces_emblem_effect_base_pipeline,
												0, /* layer number */
												COGL_TEXTURE_TYPE_2D);
	}

	priv->pipeline=cogl_pipeline_copy(_xfce_overview_workspaces_emblem_effect_base_pipeline);
}

/* IMPLEMENTATION: Public API */

/* Create new actor */
ClutterEffect* xfce_overview_workspaces_emblem_effect_new(void)
{
	return(g_object_new(XFCE_OVERVIEW_WORKSPACES_TYPE_EMBLEM_EFFECT, NULL));
}

/* Get/set icon name of emblem to draw */
const gchar* xfce_overview_workspaces_emblem_effect_get_icon_name(XfceOverviewWorkspacesEmblemEffect *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_EMBLEM_EFFECT(self), NULL);

	return(self->priv->iconName);
}

void xfce_overview_workspaces_emblem_effect_set_icon_name(XfceOverviewWorkspacesEmblemEffect *self, const gchar *inIconName)
{
	XfceOverviewWorkspacesEmblemEffectPrivate	*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_EMBLEM_EFFECT(self));
	g_return_if_fail(inIconName);

	priv=self->priv;

	/* Set value if changed */
	if(priv->icon || g_strcmp0(priv->iconName, inIconName)!=0)
	{
		/* Set value */
		if(priv->iconName) g_free(priv->iconName);
		priv->iconName=g_strdup(inIconName);

		/* Dispose any icon image loaded */
		if(priv->icon)
		{
			g_object_unref(priv->icon);
			priv->icon=NULL;
		}

		/* Invalidate effect to get it redrawn */
		clutter_effect_queue_repaint(CLUTTER_EFFECT(self));

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesEmblemEffectProperties[PROP_ICON_NAME]);
	}
}

/* Get/set icon size of emblem to draw */
gint xfce_overview_workspaces_emblem_effect_get_icon_size(XfceOverviewWorkspacesEmblemEffect *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_EMBLEM_EFFECT(self), 0);

	return(self->priv->iconSize);
}

void xfce_overview_workspaces_emblem_effect_set_icon_size(XfceOverviewWorkspacesEmblemEffect *self, const gint inSize)
{
	XfceOverviewWorkspacesEmblemEffectPrivate	*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_EMBLEM_EFFECT(self));
	g_return_if_fail(inSize>0);

	priv=self->priv;

	/* Set value if changed */
	if(priv->iconSize!=inSize)
	{
		/* Set value */
		priv->iconSize=inSize;

		/* Dispose any icon image loaded */
		if(priv->icon)
		{
			g_object_unref(priv->icon);
			priv->icon=NULL;
		}

		/* Invalidate effect to get it redrawn */
		clutter_effect_queue_repaint(CLUTTER_EFFECT(self));

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesEmblemEffectProperties[PROP_ICON_SIZE]);
	}
}

/* Get/set x align of emblem */
gfloat xfce_overview_workspaces_emblem_effect_get_padding(XfceOverviewWorkspacesEmblemEffect *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_EMBLEM_EFFECT(self), 0.0f);

	return(self->priv->padding);
}

void xfce_overview_workspaces_emblem_effect_set_padding(XfceOverviewWorkspacesEmblemEffect *self, const gfloat inPadding)
{
	XfceOverviewWorkspacesEmblemEffectPrivate		*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_EMBLEM_EFFECT(self));
	g_return_if_fail(inPadding>=0.0f);

	priv=self->priv;

	/* Set value if changed */
	if(priv->padding!=inPadding)
	{
		/* Set value */
		priv->padding=inPadding;

		/* Invalidate effect to get it redrawn */
		clutter_effect_queue_repaint(CLUTTER_EFFECT(self));

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesEmblemEffectProperties[PROP_PADDING]);
	}
}

/* Get/set x align of emblem */
gfloat xfce_overview_workspaces_emblem_effect_get_x_align(XfceOverviewWorkspacesEmblemEffect *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_EMBLEM_EFFECT(self), 0.0f);

	return(self->priv->xAlign);
}

void xfce_overview_workspaces_emblem_effect_set_x_align(XfceOverviewWorkspacesEmblemEffect *self, const gfloat inAlign)
{
	XfceOverviewWorkspacesEmblemEffectPrivate		*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_EMBLEM_EFFECT(self));
	g_return_if_fail(inAlign>=0.0f && inAlign<=1.0f);

	priv=self->priv;

	/* Set value if changed */
	if(priv->xAlign!=inAlign)
	{
		/* Set value */
		priv->xAlign=inAlign;

		/* Invalidate effect to get it redrawn */
		clutter_effect_queue_repaint(CLUTTER_EFFECT(self));

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesEmblemEffectProperties[PROP_X_ALIGN]);
	}
}

/* Get/set y align of emblem */
gfloat xfce_overview_workspaces_emblem_effect_get_y_align(XfceOverviewWorkspacesEmblemEffect *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_EMBLEM_EFFECT(self), 0.0f);

	return(self->priv->xAlign);
}

void xfce_overview_workspaces_emblem_effect_set_y_align(XfceOverviewWorkspacesEmblemEffect *self, const gfloat inAlign)
{
	XfceOverviewWorkspacesEmblemEffectPrivate		*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_EMBLEM_EFFECT(self));
	g_return_if_fail(inAlign>=0.0f && inAlign<=1.0f);

	priv=self->priv;

	/* Set value if changed */
	if(priv->yAlign!=inAlign)
	{
		/* Set value */
		priv->yAlign=inAlign;

		/* Invalidate effect to get it redrawn */
		clutter_effect_queue_repaint(CLUTTER_EFFECT(self));

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesEmblemEffectProperties[PROP_Y_ALIGN]);
	}
}

/* Get/set anchor point of emblem */
XfceOverviewWorkspacesAnchorPoint xfce_overview_workspaces_emblem_effect_get_anchor_point(XfceOverviewWorkspacesEmblemEffect *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_EMBLEM_EFFECT(self), XFCE_OVERVIEW_WORKSPACES_ANCHOR_POINT_NORTH_WEST);

	return(self->priv->anchorPoint);
}

void xfce_overview_workspaces_emblem_effect_set_anchor_point(XfceOverviewWorkspacesEmblemEffect *self, const XfceOverviewWorkspacesAnchorPoint inAnchorPoint)
{
	XfceOverviewWorkspacesEmblemEffectPrivate		*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_EMBLEM_EFFECT(self));
	g_return_if_fail(inAnchorPoint>=XFCE_OVERVIEW_WORKSPACES_ANCHOR_POINT_NONE);
	g_return_if_fail(inAnchorPoint<=XFCE_OVERVIEW_WORKSPACES_ANCHOR_POINT_CENTER);

	priv=self->priv;

	/* Set value if changed */
	if(priv->anchorPoint!=inAnchorPoint)
	{
		/* Set value */
		priv->anchorPoint=inAnchorPoint;

		/* Invalidate effect to get it redrawn */
		clutter_effect_queue_repaint(CLUTTER_EFFECT(self));

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesEmblemEffectProperties[PROP_ANCHOR_POINT]);
	}
}
