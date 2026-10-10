/*
 * scrollbar: A scroll bar
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

#include <libxfce-overview-workspaces/scrollbar.h>

#include <glib/gi18n-lib.h>

#include <libxfce-overview-workspaces/stylable.h>
#include <libxfce-overview-workspaces/debug.h>


/* Define this class in GObject system */
struct _XfceOverviewWorkspacesScrollbarPrivate
{
	/* Properties related */
	ClutterOrientation		orientation;
	gfloat					value;
	gfloat					valueRange;
	gfloat					range;
	gfloat					pageSizeFactor;
	gfloat					spacing;
	gfloat					sliderWidth;
	gfloat					sliderRadius;
	ClutterColor			*sliderColor;

	/* Instance related */
	ClutterContent			*slider;
	ClutterSize				lastViewportSize;
	ClutterSize				lastSliderSize;
	gfloat					sliderPosition, sliderSize;
	gfloat					dragAlignment;
	ClutterInputDevice		*dragDevice;
	guint					signalButtonReleasedID;
	guint					signalMotionEventID;
};

G_DEFINE_TYPE_WITH_PRIVATE(XfceOverviewWorkspacesScrollbar,
							xfce_overview_workspaces_scrollbar,
							XFCE_OVERVIEW_WORKSPACES_TYPE_BACKGROUND)

/* Properties */
enum
{
	PROP_0,

	PROP_ORIENTATION,
	PROP_VALUE,
	PROP_VALUE_RANGE,
	PROP_RANGE,
	PROP_PAGE_SIZE_FACTOR,
	PROP_SPACING,
	PROP_SLIDER_WIDTH,
	PROP_SLIDER_RADIUS,
	PROP_SLIDER_COLOR,

	PROP_LAST
};

static GParamSpec* XfceOverviewWorkspacesScrollbarProperties[PROP_LAST]={ 0, };

/* Signals */
enum
{
	SIGNAL_VALUE_CHANGED,

	SIGNAL_LAST
};

static guint XfceOverviewWorkspacesScrollbarSignals[SIGNAL_LAST]={ 0, };

/* IMPLEMENTATION: Private variables and methods */

/* Get value from coord */
static gfloat _xfce_overview_workspaces_scrollbar_get_value_from_coord(XfceOverviewWorkspacesScrollbar *self,
															gfloat inX,
															gfloat inY,
															gfloat inAlignment)
{
	XfceOverviewWorkspacesScrollbarPrivate		*priv;
	gfloat							coord;
	gfloat							width;
	gfloat							value;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_SCROLLBAR(self), 0.0f);
	g_return_val_if_fail(inAlignment>=0.0f && inAlignment<=1.0f, 0.0f);

	priv=self->priv;

	/* Get coordinate for calculation depending on orientation and
	 * subtract spacing
	 */
	if(priv->orientation==CLUTTER_ORIENTATION_HORIZONTAL)
	{
		coord=inX;
		width=priv->lastSliderSize.width;
	}
		else
		{
			coord=inY;
			width=priv->lastSliderSize.height;
		}

	coord-=priv->spacing;

	/* Apply alignment */
	coord-=(priv->sliderSize*inAlignment);

	/* Calculate new value to set by coordinate */
	value=(coord/width)*priv->range;
	value=MAX(0.0f, value);
	value=MIN(value, priv->range-priv->valueRange);

	/* Return calculated value */
	return(value);
}

/* Pointer moved inside scroll bar (usually called after button-pressed event) */
static gboolean _xfce_overview_workspaces_scrollbar_on_motion_event(ClutterActor *inActor,
														ClutterEvent *inEvent,
														gpointer inUserData)
{
	XfceOverviewWorkspacesScrollbar			*self;
	XfceOverviewWorkspacesScrollbarPrivate		*priv;
	gfloat							eventX, eventY;
	gfloat							x, y;
	gfloat							value;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_SCROLLBAR(inActor), FALSE);
	g_return_val_if_fail(inEvent, FALSE);

	self=XFCE_OVERVIEW_WORKSPACES_SCROLLBAR(inActor);
	priv=self->priv;

	/* Get coords where event happened */
	clutter_event_get_coords(inEvent, &eventX, &eventY);
	if(!clutter_actor_transform_stage_point(inActor, eventX, eventY, &x, &y)) return(FALSE);

	/* Set value from motion */
	value=_xfce_overview_workspaces_scrollbar_get_value_from_coord(self, x, y, priv->dragAlignment);
	xfce_overview_workspaces_scrollbar_set_value(self, value);

	return(CLUTTER_EVENT_STOP);
}

/* User released button in scroll bar */
static gboolean _xfce_overview_workspaces_scrollbar_on_button_released(ClutterActor *inActor,
															ClutterEvent *inEvent,
															gpointer inUserData)
{
	XfceOverviewWorkspacesScrollbar			*self;
	XfceOverviewWorkspacesScrollbarPrivate		*priv;
	gfloat							eventX, eventY;
	gfloat							x, y;
	gfloat							value;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_SCROLLBAR(inActor), FALSE);
	g_return_val_if_fail(inEvent, FALSE);

	self=XFCE_OVERVIEW_WORKSPACES_SCROLLBAR(inActor);
	priv=self->priv;

	/* If user did not release a left-click do nothing */
	if(clutter_event_get_button(inEvent)!=1) return(FALSE);

	/* Release exclusive handling of pointer events and
	 * disconnect motion and button-release events
	 */
	if(priv->dragDevice)
	{
		clutter_input_device_ungrab(priv->dragDevice);
		priv->dragDevice=NULL;
	}

	if(priv->signalMotionEventID)
	{
		g_signal_handler_disconnect(inActor, priv->signalMotionEventID);
		priv->signalMotionEventID=0L;
	}

	if(priv->signalButtonReleasedID)
	{
		g_signal_handler_disconnect(inActor, priv->signalButtonReleasedID);
		priv->signalButtonReleasedID=0L;
	}

	/* Remove style */
	xfce_overview_workspaces_stylable_remove_pseudo_class(XFCE_OVERVIEW_WORKSPACES_STYLABLE(self), "pressed");

	/* Get coords where event happened */
	clutter_event_get_coords(inEvent, &eventX, &eventY);
	if(!clutter_actor_transform_stage_point(inActor, eventX, eventY, &x, &y)) return(FALSE);

	/* Set new value */
	value=_xfce_overview_workspaces_scrollbar_get_value_from_coord(self, x, y, priv->dragAlignment);
	xfce_overview_workspaces_scrollbar_set_value(self, value);

	return(CLUTTER_EVENT_STOP);
}

/* User pressed button in scroll bar */
static gboolean _xfce_overview_workspaces_scrollbar_on_button_pressed(ClutterActor *inActor,
															ClutterEvent *inEvent,
															gpointer inUserData)
{
	XfceOverviewWorkspacesScrollbar			*self;
	XfceOverviewWorkspacesScrollbarPrivate		*priv;
	gfloat							eventX, eventY;
	gfloat							x, y;
	gfloat							value;
	gfloat							dragOffset;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_SCROLLBAR(inActor), FALSE);
	g_return_val_if_fail(inEvent, FALSE);

	self=XFCE_OVERVIEW_WORKSPACES_SCROLLBAR(inActor);
	priv=self->priv;

	/* If user left-clicked into scroll bar adjust value to point
	 * where the click happened
	 */
	if(clutter_event_get_button(inEvent)!=1) return(FALSE);

	/* Get coords where event happened. If coords are not in bar handle set value
	 * otherwise only determine offset used in event handlers and do not adjust
	 * value as this would cause a movement of bar handle */
	clutter_event_get_coords(inEvent, &eventX, &eventY);
	if(!clutter_actor_transform_stage_point(inActor, eventX, eventY, &x, &y)) return(FALSE);

	value=_xfce_overview_workspaces_scrollbar_get_value_from_coord(self, x, y, 0.0f);
	if(value<priv->value || value>=(priv->value+priv->valueRange))
	{
		/* Set new value */
		value=_xfce_overview_workspaces_scrollbar_get_value_from_coord(self, x, y, 0.5f);
		xfce_overview_workspaces_scrollbar_set_value(self, value);
		return(FALSE);
	}

	/* Remember event values for drag'n'drop of slider */
	dragOffset=-(priv->spacing+priv->sliderPosition);
	if(priv->orientation==CLUTTER_ORIENTATION_HORIZONTAL) dragOffset+=x;
		else dragOffset+=y;

	priv->dragAlignment=dragOffset/priv->sliderSize;

	/* Connect signals for motion and button-release events */
	priv->signalMotionEventID=g_signal_connect_after(inActor,
														"motion-event",
														G_CALLBACK(_xfce_overview_workspaces_scrollbar_on_motion_event),
														NULL);
	priv->signalButtonReleasedID=g_signal_connect_after(inActor,
															"button-release-event",
															G_CALLBACK(_xfce_overview_workspaces_scrollbar_on_button_released),
															NULL);

	/* Add style */
	xfce_overview_workspaces_stylable_add_pseudo_class(XFCE_OVERVIEW_WORKSPACES_STYLABLE(self), "pressed");

	/* Handle pointer events exclusively */
	priv->dragDevice=clutter_event_get_device(inEvent);
	clutter_input_device_grab(priv->dragDevice, inActor);

	return(CLUTTER_EVENT_STOP);
}

/* A scroll event occured in scroll bar (e.g. by mouse-wheel) */
static gboolean _xfce_overview_workspaces_scrollbar_on_scroll_event(ClutterActor *inActor,
														ClutterEvent *inEvent,
														gpointer inUserData)
{
	XfceOverviewWorkspacesScrollbar			*self;
	XfceOverviewWorkspacesScrollbarPrivate		*priv;
	gfloat							value;
	gfloat							directionFactor;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_SCROLLBAR(inActor), FALSE);
	g_return_val_if_fail(inEvent, FALSE);

	self=XFCE_OVERVIEW_WORKSPACES_SCROLLBAR(inActor);
	priv=self->priv;

	/* Get direction of scroll event */
	switch(clutter_event_get_scroll_direction(inEvent))
	{
		case CLUTTER_SCROLL_UP:
		case CLUTTER_SCROLL_LEFT:
			directionFactor=-priv->pageSizeFactor;
			break;

		case CLUTTER_SCROLL_DOWN:
		case CLUTTER_SCROLL_RIGHT:
			directionFactor=priv->pageSizeFactor;
			break;

		/* Unhandled directions */
		default:
			XFCE_OVERVIEW_WORKSPACES_DEBUG(self, ACTOR,
								"Cannot handle scroll direction %d in %s",
								clutter_event_get_scroll_direction(inEvent),
								G_OBJECT_TYPE_NAME(self));
			return(CLUTTER_EVENT_PROPAGATE);
	}

	/* Calculate new value by increasing or decreasing value by value-range
	 * of slider and adjust new value to fit into range (respecting value-range)
	 */
	value=priv->value+(priv->valueRange*directionFactor);
	value=MAX(value, 0);
	value=MIN(priv->range-priv->valueRange, value);

	/* Set new value */
	xfce_overview_workspaces_scrollbar_set_value(self, value);

	return(CLUTTER_EVENT_STOP);
}

/* Rectangle canvas should be redrawn */
static gboolean _xfce_overview_workspaces_scrollbar_on_draw_slider(XfceOverviewWorkspacesScrollbar *self,
														cairo_t *inContext,
														int inWidth,
														int inHeight,
														gpointer inUserData)
{
	XfceOverviewWorkspacesScrollbarPrivate		*priv;
	gdouble							radius;
	gdouble							top, left, bottom, right;
	gdouble							barValueRange;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_SCROLLBAR(self), TRUE);
	g_return_val_if_fail(CLUTTER_IS_CANVAS(inUserData), TRUE);

	priv=self->priv;

	/* Clear current contents of the canvas */
	cairo_save(inContext);
	cairo_set_operator(inContext, CAIRO_OPERATOR_CLEAR);
	cairo_paint(inContext);
	cairo_restore(inContext);

	cairo_set_operator(inContext, CAIRO_OPERATOR_OVER);

	/* Set color for slider */
	if(priv->sliderColor) clutter_cairo_set_source_color(inContext, priv->sliderColor);

	/* Determine radius for rounded corners */
	radius=MIN(priv->sliderRadius, inWidth/2.0f);
	radius=MIN(radius, inHeight/2.0f);

	/* Calculate bounding coordinates for slider and viewport */
	priv->lastViewportSize.width=inWidth;
	priv->lastViewportSize.height=inHeight;
	priv->lastSliderSize.width=MAX(0, inWidth-(2*priv->spacing));
	priv->lastSliderSize.height=MAX(0, inHeight-(2*priv->spacing));

	if(priv->orientation==CLUTTER_ORIENTATION_HORIZONTAL)
	{
		if(priv->range > priv->lastViewportSize.width)
		{
			priv->sliderSize=(priv->lastViewportSize.width/priv->range)*priv->lastSliderSize.width;
		}
			else priv->sliderSize=priv->lastSliderSize.width;

		barValueRange=(priv->sliderSize/priv->lastSliderSize.width)*priv->range;

		priv->sliderPosition=MAX(0, (priv->value/priv->range)*priv->lastSliderSize.width);
		priv->sliderPosition=MIN(priv->sliderPosition, priv->lastSliderSize.width);
		if(priv->sliderPosition+priv->sliderSize>priv->lastSliderSize.width) priv->sliderPosition=priv->lastSliderSize.width-priv->sliderSize;

		top=priv->spacing;
		bottom=priv->lastSliderSize.height;
		left=priv->sliderPosition;
		right=priv->sliderPosition+priv->sliderSize;
	}
		else
		{
			if(priv->range > priv->lastViewportSize.height)
			{
				priv->sliderSize=(priv->lastViewportSize.height/priv->range)*priv->lastSliderSize.height;
			}
				else priv->sliderSize=priv->lastSliderSize.height;

			barValueRange=(priv->sliderSize/priv->lastSliderSize.height)*priv->range;

			priv->sliderPosition=MAX(0, (priv->value/priv->range)*priv->lastSliderSize.height);
			priv->sliderPosition=MIN(priv->sliderPosition, priv->lastSliderSize.height);
			if(priv->sliderPosition+priv->sliderSize>priv->lastSliderSize.height) priv->sliderPosition=priv->lastSliderSize.height-priv->sliderSize;

			left=priv->spacing;
			right=priv->lastSliderSize.width;
			top=priv->sliderPosition;
			bottom=priv->sliderPosition+priv->sliderSize;
		}

	/* Draw slider */
	if(radius>0.0f)
	{
		cairo_move_to(inContext, left, top+radius);
		cairo_arc(inContext, left+radius, top+radius, radius, G_PI, G_PI*1.5);

		cairo_line_to(inContext, right-radius, top);
		cairo_arc(inContext, right-radius, top+radius, radius, G_PI*1.5, 0);

		cairo_line_to(inContext, right, bottom-radius);
		cairo_arc(inContext, right-radius, bottom-radius, radius, 0, G_PI/2.0);

		cairo_line_to(inContext, left+radius, bottom);
		cairo_arc(inContext, left+radius, bottom-radius, radius, G_PI/2.0, G_PI);

		cairo_line_to(inContext, left, radius);
	}
		else
		{
			cairo_rectangle(inContext, left, top, right, bottom);
		}

	cairo_fill(inContext);

	/* Set value if changed */
	if(barValueRange!=priv->valueRange)
	{
		/* Set value */
		priv->valueRange=barValueRange;

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesScrollbarProperties[PROP_VALUE_RANGE]);

		/* Adjust value to fit into range (respecting value-range) if needed */
		if(priv->value+priv->valueRange>priv->range)
		{
			xfce_overview_workspaces_scrollbar_set_value(self, priv->range-priv->valueRange);
		}
	}

	/* Done drawing */
	return(CLUTTER_EVENT_STOP);
}

/* IMPLEMENTATION: ClutterActor */

/* Get preferred width/height */
static void _xfce_overview_workspaces_scrollbar_get_preferred_height(ClutterActor *self,
														gfloat inForWidth,
														gfloat *outMinHeight,
														gfloat *outNaturalHeight)
{
	XfceOverviewWorkspacesScrollbarPrivate		*priv=XFCE_OVERVIEW_WORKSPACES_SCROLLBAR(self)->priv;
	gfloat							minHeight, naturalHeight;

	minHeight=naturalHeight=0.0f;

	/* Determine sizes */
	if(priv->orientation==CLUTTER_ORIENTATION_HORIZONTAL)
	{
		minHeight=naturalHeight=(2*priv->spacing)+priv->sliderWidth;
	}
		else
		{
			/* Call parent's class method to determine sizes */
			ClutterActorClass		*actorClass=CLUTTER_ACTOR_CLASS(xfce_overview_workspaces_scrollbar_parent_class);

			if(actorClass->get_preferred_height)
			{
				actorClass->get_preferred_height(self,
												inForWidth,
												&minHeight,
												&naturalHeight);
			}
		}

	/* Store sizes computed */
	if(outMinHeight) *outMinHeight=minHeight;
	if(outNaturalHeight) *outNaturalHeight=naturalHeight;
}

static void _xfce_overview_workspaces_scrollbar_get_preferred_width(ClutterActor *self,
														gfloat inForHeight,
														gfloat *outMinWidth,
														gfloat *outNaturalWidth)
{
	XfceOverviewWorkspacesScrollbarPrivate		*priv=XFCE_OVERVIEW_WORKSPACES_SCROLLBAR(self)->priv;
	gfloat							minWidth, naturalWidth;

	minWidth=naturalWidth=0.0f;

	/* Determine sizes */
	if(priv->orientation==CLUTTER_ORIENTATION_VERTICAL)
	{
		minWidth=naturalWidth=(2*priv->spacing)+priv->sliderWidth;
	}
		else
		{
			/* Call parent's class method to determine sizes */
			ClutterActorClass		*actorClass=CLUTTER_ACTOR_CLASS(xfce_overview_workspaces_scrollbar_parent_class);

			if(actorClass->get_preferred_width)
			{
				actorClass->get_preferred_width(self,
												inForHeight,
												&minWidth,
												&naturalWidth);
			}
		}

	/* Store sizes computed */
	if(outMinWidth) *outMinWidth=minWidth;
	if(outNaturalWidth) *outNaturalWidth=naturalWidth;
}

/* Allocate position and size of actor and its children */
static void _xfce_overview_workspaces_scrollbar_allocate(ClutterActor *self,
												const ClutterActorBox *inBox,
												ClutterAllocationFlags inFlags)
{
	XfceOverviewWorkspacesScrollbarPrivate		*priv=XFCE_OVERVIEW_WORKSPACES_SCROLLBAR(self)->priv;

	/* Chain up to store the allocation of the actor */
	CLUTTER_ACTOR_CLASS(xfce_overview_workspaces_scrollbar_parent_class)->allocate(self, inBox, inFlags);

	/* Set size of slider canvas */
	clutter_canvas_set_size(CLUTTER_CANVAS(priv->slider),
								clutter_actor_box_get_width(inBox),
								clutter_actor_box_get_height(inBox));
}

/* IMPLEMENTATION: GObject */

/* Dispose this object */
static void _xfce_overview_workspaces_scrollbar_dispose(GObject *inObject)
{
	XfceOverviewWorkspacesScrollbar			*self=XFCE_OVERVIEW_WORKSPACES_SCROLLBAR(inObject);
	XfceOverviewWorkspacesScrollbarPrivate		*priv=self->priv;

	/* Release allocated resources */
	if(priv->sliderColor)
	{
		clutter_color_free(priv->sliderColor);
		priv->sliderColor=NULL;
	}

	if(priv->slider)
	{
		g_object_unref(priv->slider);
		priv->slider=NULL;
	}

	if(priv->dragDevice)
	{
		clutter_input_device_ungrab(priv->dragDevice);
		priv->dragDevice=NULL;
	}

	if(priv->signalMotionEventID)
	{
		g_signal_handler_disconnect(self, priv->signalMotionEventID);
		priv->signalMotionEventID=0L;
	}

	if(priv->signalButtonReleasedID)
	{
		g_signal_handler_disconnect(self, priv->signalButtonReleasedID);
		priv->signalButtonReleasedID=0L;
	}

	/* Call parent's class dispose method */
	G_OBJECT_CLASS(xfce_overview_workspaces_scrollbar_parent_class)->dispose(inObject);
}

/* Set/get properties */
static void _xfce_overview_workspaces_scrollbar_set_property(GObject *inObject,
												guint inPropID,
												const GValue *inValue,
												GParamSpec *inSpec)
{
	XfceOverviewWorkspacesScrollbar		*self=XFCE_OVERVIEW_WORKSPACES_SCROLLBAR(inObject);
	
	switch(inPropID)
	{
		case PROP_ORIENTATION:
			xfce_overview_workspaces_scrollbar_set_orientation(self, g_value_get_enum(inValue));
			break;

		case PROP_VALUE:
			xfce_overview_workspaces_scrollbar_set_value(self, g_value_get_float(inValue));
			break;

		case PROP_RANGE:
			xfce_overview_workspaces_scrollbar_set_range(self, g_value_get_float(inValue));
			break;

		case PROP_PAGE_SIZE_FACTOR:
			xfce_overview_workspaces_scrollbar_set_page_size_factor(self, g_value_get_float(inValue));
			break;

		case PROP_SPACING:
			xfce_overview_workspaces_scrollbar_set_spacing(self, g_value_get_float(inValue));
			break;

		case PROP_SLIDER_WIDTH:
			xfce_overview_workspaces_scrollbar_set_slider_width(self, g_value_get_float(inValue));
			break;

		case PROP_SLIDER_RADIUS:
			xfce_overview_workspaces_scrollbar_set_slider_radius(self, g_value_get_float(inValue));
			break;

		case PROP_SLIDER_COLOR:
			xfce_overview_workspaces_scrollbar_set_slider_color(self, clutter_value_get_color(inValue));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(inObject, inPropID, inSpec);
			break;
	}
}

static void _xfce_overview_workspaces_scrollbar_get_property(GObject *inObject,
												guint inPropID,
												GValue *outValue,
												GParamSpec *inSpec)
{
	XfceOverviewWorkspacesScrollbar		*self=XFCE_OVERVIEW_WORKSPACES_SCROLLBAR(inObject);

	switch(inPropID)
	{
		case PROP_ORIENTATION:
			g_value_set_enum(outValue, self->priv->orientation);
			break;

		case PROP_VALUE:
			g_value_set_float(outValue, self->priv->value);
			break;

		case PROP_VALUE_RANGE:
			g_value_set_float(outValue, self->priv->valueRange);
			break;

		case PROP_RANGE:
			g_value_set_float(outValue, self->priv->range);
			break;

		case PROP_PAGE_SIZE_FACTOR:
			g_value_set_float(outValue, self->priv->pageSizeFactor);
			break;

		case PROP_SPACING:
			g_value_set_float(outValue, self->priv->spacing);
			break;

		case PROP_SLIDER_WIDTH:
			g_value_set_float(outValue, self->priv->sliderWidth);
			break;

		case PROP_SLIDER_RADIUS:
			g_value_set_float(outValue, self->priv->sliderRadius);
			break;

		case PROP_SLIDER_COLOR:
			clutter_value_set_color(outValue, self->priv->sliderColor);
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
static void xfce_overview_workspaces_scrollbar_class_init(XfceOverviewWorkspacesScrollbarClass *klass)
{
	XfceOverviewWorkspacesActorClass	*actorClass=XFCE_OVERVIEW_WORKSPACES_ACTOR_CLASS(klass);
	ClutterActorClass		*clutterActorClass=CLUTTER_ACTOR_CLASS(klass);
	GObjectClass			*gobjectClass=G_OBJECT_CLASS(klass);

	/* Override functions */
	gobjectClass->set_property=_xfce_overview_workspaces_scrollbar_set_property;
	gobjectClass->get_property=_xfce_overview_workspaces_scrollbar_get_property;
	gobjectClass->dispose=_xfce_overview_workspaces_scrollbar_dispose;

	clutterActorClass->get_preferred_width=_xfce_overview_workspaces_scrollbar_get_preferred_width;
	clutterActorClass->get_preferred_height=_xfce_overview_workspaces_scrollbar_get_preferred_height;
	clutterActorClass->allocate=_xfce_overview_workspaces_scrollbar_allocate;

	/* Define properties */
	XfceOverviewWorkspacesScrollbarProperties[PROP_ORIENTATION]=
		g_param_spec_enum("orientation",
							"Orientation",
							"Defines if scrollbar is horizontal or vertical",
							CLUTTER_TYPE_ORIENTATION,
							CLUTTER_ORIENTATION_HORIZONTAL,
							G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	XfceOverviewWorkspacesScrollbarProperties[PROP_VALUE]=
		g_param_spec_float("value",
							"Value",
							"Current value of scroll bar within range",
							0.0f, G_MAXFLOAT,
							0.0f,
							G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	XfceOverviewWorkspacesScrollbarProperties[PROP_VALUE_RANGE]=
		g_param_spec_float("value-range",
							"Value range",
							"The range the slider of scroll bar covers",
							0.0f, G_MAXFLOAT,
							0.0f,
							G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

	XfceOverviewWorkspacesScrollbarProperties[PROP_RANGE]=
		g_param_spec_float("range",
							"Range",
							"Range to scroll within",
							0.0f, G_MAXFLOAT,
							1.0f,
							G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	XfceOverviewWorkspacesScrollbarProperties[PROP_PAGE_SIZE_FACTOR]=
		g_param_spec_float("page-size-factor",
							"Page size factor",
							"The factor of value range to increase or decrease value by on pointer scroll events.",
							0.1f, 1.0f,
							0.5f,
							G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	XfceOverviewWorkspacesScrollbarProperties[PROP_SPACING]=
		g_param_spec_float("spacing",
							"Spacing",
							"The spacing between scrollbar and background",
							0.0f, G_MAXFLOAT,
							0.0f,
							G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	XfceOverviewWorkspacesScrollbarProperties[PROP_SLIDER_WIDTH]=
		g_param_spec_float("slider-width",
							"Slider width",
							"The width of slider",
							1.0f, G_MAXFLOAT,
							1.0f,
							G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	XfceOverviewWorkspacesScrollbarProperties[PROP_SLIDER_RADIUS]=
		g_param_spec_float("slider-radius",
							"Slider radius",
							"The radius of slider's rounded corners",
							0.0f, G_MAXFLOAT,
							0.0f,
							G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	XfceOverviewWorkspacesScrollbarProperties[PROP_SLIDER_COLOR]=
		clutter_param_spec_color("slider-color",
									"Slider color",
									"Color of slider",
									CLUTTER_COLOR_White,
									G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT);

	g_object_class_install_properties(gobjectClass, PROP_LAST, XfceOverviewWorkspacesScrollbarProperties);

	/* Define stylable properties */
	xfce_overview_workspaces_actor_install_stylable_property(actorClass, XfceOverviewWorkspacesScrollbarProperties[PROP_ORIENTATION]);
	xfce_overview_workspaces_actor_install_stylable_property(actorClass, XfceOverviewWorkspacesScrollbarProperties[PROP_PAGE_SIZE_FACTOR]);
	xfce_overview_workspaces_actor_install_stylable_property(actorClass, XfceOverviewWorkspacesScrollbarProperties[PROP_SPACING]);
	xfce_overview_workspaces_actor_install_stylable_property(actorClass, XfceOverviewWorkspacesScrollbarProperties[PROP_SLIDER_WIDTH]);
	xfce_overview_workspaces_actor_install_stylable_property(actorClass, XfceOverviewWorkspacesScrollbarProperties[PROP_SLIDER_RADIUS]);
	xfce_overview_workspaces_actor_install_stylable_property(actorClass, XfceOverviewWorkspacesScrollbarProperties[PROP_SLIDER_COLOR]);

	/* Define signals */
	XfceOverviewWorkspacesScrollbarSignals[SIGNAL_VALUE_CHANGED]=
		g_signal_new("value-changed",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST,
						G_STRUCT_OFFSET(XfceOverviewWorkspacesScrollbarClass, value_changed),
						NULL,
						NULL,
						g_cclosure_marshal_VOID__FLOAT,
						G_TYPE_NONE,
						1,
						G_TYPE_FLOAT);
}

/* Object initialization
 * Create private structure and set up default values
 */
static void xfce_overview_workspaces_scrollbar_init(XfceOverviewWorkspacesScrollbar *self)
{
	XfceOverviewWorkspacesScrollbarPrivate		*priv;

	priv=self->priv=xfce_overview_workspaces_scrollbar_get_instance_private(self);

	/* Set up default values */
	priv->orientation=CLUTTER_ORIENTATION_HORIZONTAL;
	priv->value=0.0f;
	priv->valueRange=0.0f;
	priv->range=1.0f;
	priv->pageSizeFactor=0.5f;
	priv->spacing=0.0f;
	priv->sliderWidth=1.0f;
	priv->sliderRadius=0.0f;
	priv->sliderColor=NULL;
	priv->slider=clutter_canvas_new();
	priv->signalButtonReleasedID=0;
	priv->signalMotionEventID=0;
	priv->dragDevice=NULL;

	/* Set up actor */
	clutter_actor_set_reactive(CLUTTER_ACTOR(self), TRUE);
	clutter_actor_set_content(CLUTTER_ACTOR(self), priv->slider);
	if(priv->orientation==CLUTTER_ORIENTATION_HORIZONTAL) clutter_actor_set_request_mode(CLUTTER_ACTOR(self), CLUTTER_REQUEST_HEIGHT_FOR_WIDTH);
		else clutter_actor_set_request_mode(CLUTTER_ACTOR(self), CLUTTER_REQUEST_WIDTH_FOR_HEIGHT);

	/* Connect signals */
	g_signal_connect_swapped(priv->slider, "draw", G_CALLBACK(_xfce_overview_workspaces_scrollbar_on_draw_slider), self);
	g_signal_connect(self, "button-press-event", G_CALLBACK(_xfce_overview_workspaces_scrollbar_on_button_pressed), NULL);
	g_signal_connect(self, "scroll-event", G_CALLBACK(_xfce_overview_workspaces_scrollbar_on_scroll_event), NULL);
}

/* IMPLEMENTATION: Public API */

/* Create new actor */
ClutterActor* xfce_overview_workspaces_scrollbar_new(ClutterOrientation inOrientation)
{
	return(g_object_new(XFCE_OVERVIEW_WORKSPACES_TYPE_SCROLLBAR,
							"orientation", inOrientation,
							NULL));
}

/* Get/set orientation */
gfloat xfce_overview_workspaces_scrollbar_get_orientation(XfceOverviewWorkspacesScrollbar *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_SCROLLBAR(self), CLUTTER_ORIENTATION_HORIZONTAL);

	return(self->priv->spacing);
}

void xfce_overview_workspaces_scrollbar_set_orientation(XfceOverviewWorkspacesScrollbar *self, ClutterOrientation inOrientation)
{
	XfceOverviewWorkspacesScrollbarPrivate		*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_SCROLLBAR(self));
	g_return_if_fail(inOrientation==CLUTTER_ORIENTATION_HORIZONTAL || inOrientation==CLUTTER_ORIENTATION_VERTICAL);

	priv=self->priv;

	/* Only set value if it changes */
	if(inOrientation==priv->orientation) return;

	/* Set new value */
	priv->orientation=inOrientation;

	if(priv->orientation==CLUTTER_ORIENTATION_HORIZONTAL) clutter_actor_set_request_mode(CLUTTER_ACTOR(self), CLUTTER_REQUEST_HEIGHT_FOR_WIDTH);
		else clutter_actor_set_request_mode(CLUTTER_ACTOR(self), CLUTTER_REQUEST_WIDTH_FOR_HEIGHT);

	if(priv->slider) clutter_content_invalidate(priv->slider);
	clutter_actor_queue_relayout(CLUTTER_ACTOR(self));

	/* Notify about property change */
	g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesScrollbarProperties[PROP_ORIENTATION]);
}

/* Get/set value */
gfloat xfce_overview_workspaces_scrollbar_get_value(XfceOverviewWorkspacesScrollbar *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_SCROLLBAR(self), 0.0f);

	return(self->priv->value);
}

void xfce_overview_workspaces_scrollbar_set_value(XfceOverviewWorkspacesScrollbar *self, gfloat inValue)
{
	XfceOverviewWorkspacesScrollbarPrivate		*priv;
	gboolean						enforceNewValue;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_SCROLLBAR(self));
	g_return_if_fail(inValue>=0.0f);

	priv=self->priv;

	/* Check if value is within range */
	enforceNewValue=FALSE;
	if(inValue+priv->valueRange>priv->range)
	{
		G_GNUC_UNUSED gfloat	oldValue;

		oldValue=inValue;
		inValue=MAX(0.0f, priv->range-priv->valueRange);
		XFCE_OVERVIEW_WORKSPACES_DEBUG(self, ACTOR,
							"Adjusting value %.2f to %.2f in scrollbar to fit into range [0-%.2f]",
							oldValue,
							inValue,
							priv->range);
		enforceNewValue=TRUE;
	}

	/* Only set value if it changes */
	if(inValue==priv->value && enforceNewValue==FALSE) return;

	/* Set new value */
	priv->value=inValue;
	if(priv->slider) clutter_content_invalidate(priv->slider);
	clutter_actor_queue_redraw(CLUTTER_ACTOR(self));

	/* Notify about property change */
	g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesScrollbarProperties[PROP_VALUE]);
	g_signal_emit(self, XfceOverviewWorkspacesScrollbarSignals[SIGNAL_VALUE_CHANGED], 0, priv->value);
}

/* Get value range */
gfloat xfce_overview_workspaces_scrollbar_get_value_range(XfceOverviewWorkspacesScrollbar *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_SCROLLBAR(self), 0.0f);

	return(self->priv->valueRange);
}

/* Get/set range */
gfloat xfce_overview_workspaces_scrollbar_get_range(XfceOverviewWorkspacesScrollbar *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_SCROLLBAR(self), 0.0f);

	return(self->priv->range);
}

void xfce_overview_workspaces_scrollbar_set_range(XfceOverviewWorkspacesScrollbar *self, gfloat inRange)
{
	XfceOverviewWorkspacesScrollbarPrivate		*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_SCROLLBAR(self));
	g_return_if_fail(inRange>=0.0f);

	priv=self->priv;

	/* Only set value if it changes */
	if(inRange==priv->range) return;

	/* Freeze notification */
	g_object_freeze_notify(G_OBJECT(self));

	/* Set new value */
	priv->range=inRange;
	if(priv->slider) clutter_content_invalidate(priv->slider);
	clutter_actor_queue_redraw(CLUTTER_ACTOR(self));

	/* Notify about property change */
	g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesScrollbarProperties[PROP_RANGE]);

	/* Check if value is still within new range otherwise adjust value */
	if(priv->value>priv->range)
	{
		G_GNUC_UNUSED gfloat	oldValue;

		XFCE_OVERVIEW_WORKSPACES_DEBUG(self, ACTOR,
							"Adjusting value %.2f in scrollbar to fit into new range %.2f",
							priv->value,
							priv->range);
		oldValue=priv->value;
		xfce_overview_workspaces_scrollbar_set_value(self, priv->range);
		XFCE_OVERVIEW_WORKSPACES_DEBUG(self, ACTOR,
							"Adjusted value %.2f to %.2f in scrollbar to fit into new range %.2f",
							oldValue,
							priv->value,
							priv->range);
	}

	/* Thaw notification */
	g_object_thaw_notify(G_OBJECT(self));
}

/* Get/set page-size factor */
gfloat xfce_overview_workspaces_scrollbar_get_page_size_factor(XfceOverviewWorkspacesScrollbar *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_SCROLLBAR(self), 0.0f);

	return(self->priv->pageSizeFactor);
}

void xfce_overview_workspaces_scrollbar_set_page_size_factor(XfceOverviewWorkspacesScrollbar *self, gfloat inFactor)
{
	XfceOverviewWorkspacesScrollbarPrivate		*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_SCROLLBAR(self));
	g_return_if_fail(inFactor>=0.1f && inFactor<=1.0f);

	priv=self->priv;

	/* Only set value if it changes */
	if(inFactor==priv->pageSizeFactor) return;

	/* Set new value */
	priv->pageSizeFactor=inFactor;

	/* Notify about property change */
	g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesScrollbarProperties[PROP_PAGE_SIZE_FACTOR]);
}

/* Get/set spacing */
gfloat xfce_overview_workspaces_scrollbar_get_spacing(XfceOverviewWorkspacesScrollbar *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_SCROLLBAR(self), 0.0f);

	return(self->priv->spacing);
}

void xfce_overview_workspaces_scrollbar_set_spacing(XfceOverviewWorkspacesScrollbar *self, gfloat inSpacing)
{
	XfceOverviewWorkspacesScrollbarPrivate		*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_SCROLLBAR(self));
	g_return_if_fail(inSpacing>=0.0f);

	priv=self->priv;

	/* Only set value if it changes */
	if(inSpacing==priv->spacing) return;

	/* Set new value */
	priv->spacing=inSpacing;
	clutter_actor_queue_relayout(CLUTTER_ACTOR(self));

	/* Notify about property change */
	g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesScrollbarProperties[PROP_SPACING]);
}

/* Get/set slider width (thickness) */
gfloat xfce_overview_workspaces_scrollbar_get_slider_width(XfceOverviewWorkspacesScrollbar *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_SCROLLBAR(self), 0.0f);

	return(self->priv->sliderWidth);
}

void xfce_overview_workspaces_scrollbar_set_slider_width(XfceOverviewWorkspacesScrollbar *self, gfloat inWidth)
{
	XfceOverviewWorkspacesScrollbarPrivate		*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_SCROLLBAR(self));
	g_return_if_fail(inWidth>=1.0f);

	priv=self->priv;

	/* Only set value if it changes */
	if(inWidth==priv->sliderWidth) return;

	/* Set new value */
	priv->sliderWidth=inWidth;
	clutter_actor_queue_relayout(CLUTTER_ACTOR(self));

	/* Notify about property change */
	g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesScrollbarProperties[PROP_SLIDER_WIDTH]);
}

/* Get/set radius of rounded corners of slider */
gfloat xfce_overview_workspaces_scrollbar_get_slider_radius(XfceOverviewWorkspacesScrollbar *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_SCROLLBAR(self), 0.0f);

	return(self->priv->sliderWidth);
}

void xfce_overview_workspaces_scrollbar_set_slider_radius(XfceOverviewWorkspacesScrollbar *self, gfloat inRadius)
{
	XfceOverviewWorkspacesScrollbarPrivate		*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_SCROLLBAR(self));
	g_return_if_fail(inRadius>=0.0f);

	priv=self->priv;

	/* Only set value if it changes */
	if(inRadius==priv->sliderRadius) return;

	/* Set new value */
	priv->sliderRadius=inRadius;
	if(priv->slider) clutter_content_invalidate(priv->slider);

	/* Notify about property change */
	g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesScrollbarProperties[PROP_SLIDER_RADIUS]);
}

/* Get/set color of slider */
const ClutterColor* xfce_overview_workspaces_scrollbar_get_slider_color(XfceOverviewWorkspacesScrollbar *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_SCROLLBAR(self), NULL);

	return(self->priv->sliderColor);
}

void xfce_overview_workspaces_scrollbar_set_slider_color(XfceOverviewWorkspacesScrollbar *self, const ClutterColor *inColor)
{
	XfceOverviewWorkspacesScrollbarPrivate		*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_SCROLLBAR(self));
	g_return_if_fail(inColor);

	priv=self->priv;

	/* Set value if changed */
	if(priv->sliderColor==NULL || clutter_color_equal(inColor, priv->sliderColor)==FALSE)
	{
		/* Set value */
		if(priv->sliderColor) clutter_color_free(priv->sliderColor);
		priv->sliderColor=clutter_color_copy(inColor);

		/* Invalidate canvas to get it redrawn */
		if(priv->slider) clutter_content_invalidate(priv->slider);

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesScrollbarProperties[PROP_SLIDER_COLOR]);
	}
}
