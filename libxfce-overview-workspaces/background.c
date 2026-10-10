/*
 * background: An actor providing background rendering. Usually other
 *             actors are derived from this one.
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

#include <libxfce-overview-workspaces/background.h>

#include <glib/gi18n-lib.h>
#include <gdk/gdk.h>
#include <math.h>

#include <libxfce-overview-workspaces/enums.h>
#include <libxfce-overview-workspaces/outline-effect.h>


/* Define this class in GObject system */
struct _XfceOverviewWorkspacesBackgroundPrivate
{
	/* Properties related */
	XfceOverviewWorkspacesBackgroundType	type;

	XfceOverviewWorkspacesGradientColor	*fillColor;
	XfceOverviewWorkspacesCorners			fillCorners;
	gfloat						fillCornersRadius;

	XfceOverviewWorkspacesGradientColor	*outlineColor;
	gfloat						outlineWidth;
	XfceOverviewWorkspacesBorders			outlineBorders;
	XfceOverviewWorkspacesCorners			outlineCorners;
	gfloat						outlineCornersRadius;

	/* Instance related */
	ClutterContent				*fillCanvas;
	XfceOverviewWorkspacesOutlineEffect	*outline;
	ClutterImage				*image;
};

G_DEFINE_TYPE_WITH_PRIVATE(XfceOverviewWorkspacesBackground,
							xfce_overview_workspaces_background,
							XFCE_OVERVIEW_WORKSPACES_TYPE_ACTOR)

/* Properties */
enum
{
	PROP_0,

	PROP_TYPE,

	PROP_CORNERS,
	PROP_CORNERS_RADIUS,

	PROP_FILL_COLOR,
	PROP_FILL_CORNERS,
	PROP_FILL_CORNERS_RADIUS,

	PROP_OUTLINE_COLOR,
	PROP_OUTLINE_WIDTH,
	PROP_OUTLINE_BORDERS,
	PROP_OUTLINE_CORNERS,
	PROP_OUTLINE_CORNERS_RADIUS,

	PROP_IMAGE,

	PROP_LAST
};

static GParamSpec* XfceOverviewWorkspacesBackgroundProperties[PROP_LAST]={ 0, };

/* IMPLEMENTATION: Private variables and methods */

/* Create pattern for simple canvas fill function */
static cairo_pattern_t* _xfce_overview_workspaces_background_create_fill_pattern(XfceOverviewWorkspacesBackground *self,
																	cairo_t *inContext,
																	int inWidth,
																	int inHeight)
{
	XfceOverviewWorkspacesBackgroundPrivate	*priv;
	XfceOverviewWorkspacesGradientType			type;
	cairo_pattern_t					*pattern;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_BACKGROUND(self), NULL);

	priv=self->priv;
	pattern=NULL;

	/* Get type of gradient to create pattern for */
	type=xfce_overview_workspaces_gradient_color_get_gradient_type(priv->fillColor);

	/* Create solid pattern if gradient is solid type */
	if(type==XFCE_OVERVIEW_WORKSPACES_GRADIENT_TYPE_SOLID)
	{
		const ClutterColor			*color;

		color=xfce_overview_workspaces_gradient_color_get_solid_color(priv->fillColor);
		pattern=cairo_pattern_create_rgba(CLAMP(color->red / 255.0, 0.0, 1.0),
											CLAMP(color->green / 255.0, 0.0, 1.0),
											CLAMP(color->blue / 255.0, 0.0, 1.0),
											CLAMP(color->alpha / 255.0, 0.0, 1.0));
	}

	/* Create linear gradient pattern if gradient is linear */
	if(type==XFCE_OVERVIEW_WORKSPACES_GRADIENT_TYPE_LINEAR_GRADIENT)
	{
		gdouble					width, height;
		gdouble					midX, midY;
		gdouble					startX, startY;
		gdouble					endX, endY;
		gdouble					angle;
		gdouble					atanRectangle;
		gdouble					tanAngle;
		guint					i, stops;

		/* Adjust angle to find region */
		angle=(2*M_PI)-xfce_overview_workspaces_gradient_color_get_angle(priv->fillColor);
		while(angle<-M_PI) angle+=(2*M_PI);
		while(angle>M_PI) angle-=(2*M_PI);

		/* I do not know why but exactly at radians of 0.0 and M_PI the
		 * start and end points are mirrored so this is a dirty workaround
		 * to swap angles. Maybe it is because tan(angle) resolves to -0.0
		 * instead of 0.0. Looks like a sign error.
		 */
		if(angle==0.0) angle=M_PI;
			else if(angle==M_PI) angle=0.0;

		/* Get size of region */
		width=inWidth;
		height=inHeight;

		/* Find region and calculates points */
		atanRectangle=atan2(height, width);
		tanAngle=tan(angle);

		midX=(width/2.0);
		midY=(height/2.0);

		if(angle!=0.0 && (angle>-atanRectangle) && (angle<=atanRectangle))
		{
			/* Region 1 */
			startX=midX+(width/2.0);
			startY=midY-((width/2.0)*tanAngle);

			endX=midX-(width/2.0);
			endY=midY+((width/2.0)*tanAngle);
		}
			else if((angle>atanRectangle) && (angle<=(M_PI-atanRectangle)))
			{
				/* Region 2 */
				startX=midX+(height/(2.0*tanAngle));
				startY=midY-(height/2.0);

				endX=midX-(height/(2.0*tanAngle));
				endY=midY+(height/2.0);
			}
			else if(angle==0.0 || (angle>(M_PI-atanRectangle)) || (angle<=-(M_PI-atanRectangle)))
			{
				/* Region 3 */
				startX=midX-(width/2.0);
				startY=midY+((width/2.0)*tanAngle);

				endX=midX+(width/2.0);
				endY=midY-((width/2.0)*tanAngle);
			}
			else
			{
				/* Region 4 */
				startX=midX-(height/(2.0*tanAngle));
				startY=midY+(height/2.0);

				endX=midX+(height/(2.0*tanAngle));
				endY=midY-(height/2.0);
			}

		/* Reduce full length to requested length if gradient should be repeated */
		if(xfce_overview_workspaces_gradient_color_get_repeat(priv->fillColor))
		{
			gdouble				vectorX, vectorY;
			gdouble				distance;
			gdouble				requestedLength;

			/* Calculate distance between start and end point as this is the current
			 * length of pattern.
			 */
			vectorX=endX-startX;
			vectorY=endY-startY;
			distance=sqrt((vectorX*vectorX) + (vectorY*vectorY));

			/* Reduce distance by percentage or absolute length */
			requestedLength=xfce_overview_workspaces_gradient_color_get_length(priv->fillColor);
			if(requestedLength<0.0)
			{
				vectorX*=-requestedLength;
				vectorY*=-requestedLength;
			}
				else
				{
					vectorX=(vectorX/distance)*requestedLength;
					vectorY=(vectorY/distance)*requestedLength;
				}

			endX=startX+vectorX;
			endY=startY+vectorY;
		}

		/* Create pattern based on calculated points */
		pattern=cairo_pattern_create_linear(startX, startY, endX, endY);

		stops=xfce_overview_workspaces_gradient_color_get_number_stops(priv->fillColor);
		for(i=0; i<stops; i++)
		{
			gdouble				offset;
			ClutterColor		color;

			xfce_overview_workspaces_gradient_color_get_stop(priv->fillColor, i, &offset, &color);
			cairo_pattern_add_color_stop_rgba(pattern,
												offset,
												CLAMP(color.red / 255.0, 0.0, 1.0),
												CLAMP(color.green / 255.0, 0.0, 1.0),
												CLAMP(color.blue / 255.0, 0.0, 1.0),
												CLAMP(color.alpha / 255.0, 0.0, 1.0));
		}

		cairo_pattern_set_extend(pattern, xfce_overview_workspaces_gradient_color_get_repeat(priv->fillColor) ? CAIRO_EXTEND_REPEAT : CAIRO_EXTEND_PAD);
	}

	/* Return created pattern */
	return(pattern);
}

/* Simple canvas fill function which can use a cairo pattern */
static void _xfce_overview_workspaces_background_draw_fill_canvas_simple(XfceOverviewWorkspacesBackground *self,
															cairo_t *inContext,
															int inWidth,
															int inHeight)
{
	XfceOverviewWorkspacesBackgroundPrivate	*priv;
	cairo_pattern_t					*pattern;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_BACKGROUND(self));

	priv=self->priv;

	/* Create pattern for filling */
	pattern=_xfce_overview_workspaces_background_create_fill_pattern(self, inContext, inWidth, inHeight);

	/* Draw rectangle with or without rounded corners */
	if((priv->type & XFCE_OVERVIEW_WORKSPACES_BACKGROUND_TYPE_ROUNDED_CORNERS) &&
		(priv->fillCorners & XFCE_OVERVIEW_WORKSPACES_CORNERS_ALL) &&
		priv->fillCornersRadius>0.0f)
	{
		/* Determine radius for rounded corners */
		gdouble						radius;

		radius=MIN(priv->fillCornersRadius, inWidth/2.0f);
		radius=MIN(radius, inHeight/2.0f);

		/* Top-left */
		if(priv->fillCorners & XFCE_OVERVIEW_WORKSPACES_CORNERS_TOP_LEFT)
		{
			cairo_move_to(inContext, 0, radius);
			cairo_arc(inContext, radius, radius, radius, G_PI, G_PI*1.5);
		}
			else cairo_move_to(inContext, 0, 0);

		/* Top-right */
		if(priv->fillCorners & XFCE_OVERVIEW_WORKSPACES_CORNERS_TOP_RIGHT)
		{
			cairo_line_to(inContext, inWidth-radius, 0);
			cairo_arc(inContext, inWidth-radius, radius, radius, G_PI*1.5, 0);
		}
			else cairo_line_to(inContext, inWidth, 0);

		/* Bottom-right */
		if(priv->fillCorners & XFCE_OVERVIEW_WORKSPACES_CORNERS_BOTTOM_RIGHT)
		{
			cairo_line_to(inContext, inWidth, inHeight-radius);
			cairo_arc(inContext, inWidth-radius, inHeight-radius, radius, 0, G_PI/2.0);
		}
			else cairo_line_to(inContext, inWidth, inHeight);

		/* Bottom-left */
		if(priv->fillCorners & XFCE_OVERVIEW_WORKSPACES_CORNERS_BOTTOM_LEFT)
		{
			cairo_line_to(inContext, radius, inHeight);
			cairo_arc(inContext, radius, inHeight-radius, radius, G_PI/2.0, G_PI);
		}
			else cairo_line_to(inContext, 0, inHeight);

		/* Close to top-left */
		if(priv->fillCorners & XFCE_OVERVIEW_WORKSPACES_CORNERS_TOP_LEFT) cairo_line_to(inContext, 0, radius);
			else cairo_line_to(inContext, 0, 0);
	}
		else
		{
			cairo_rectangle(inContext, 0, 0, inWidth, inHeight);
		}

	/* Set up pattern for filling and fill pattern */
	if(pattern) cairo_set_source(inContext, pattern);
	cairo_fill_preserve(inContext);

	/* Done drawing */
	if(pattern) cairo_pattern_destroy(pattern);
	cairo_close_path(inContext);
}

/* More complex canvas fill function for path gradient colors */
static void _xfce_overview_workspaces_background_draw_fill_canvas_path_gradient(XfceOverviewWorkspacesBackground *self,
																	cairo_t *inContext,
																	int inWidth,
																	int inHeight)
{
	XfceOverviewWorkspacesBackgroundPrivate	*priv;
	gfloat							offset;
	gfloat							maxOffset;
	gfloat							progress;
	gfloat							radius;
	ClutterColor					progressColor;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_BACKGROUND(self));

	priv=self->priv;

	/* Set line width */
	cairo_set_line_width(inContext, 1.0f);

	/* Draw rounded or flat rectangle in 0.5 pixel steps from inner to outer
	 * in color matching the progress. 0.5 pixel is chosen to mostly ensure
	 * that the lines drawn will slightly overlap to prevent "holes". As
	 * width or height of canvas to fill are integer and therefore
	 * "round/non-floating" numbers we chose that the loop will continue
	 * as long as it is above -0.1 so that 0.0 as last line will be drawn
	 * for sure.
	 */
	offset=maxOffset=MIN(inWidth, inHeight)/2.0;

	while(offset>-0.1)
	{
		/* Calculate progress */
		progress=(MAX(0.0f, offset)/maxOffset);

		/* Interpolate color according to progress */
		xfce_overview_workspaces_gradient_color_interpolate(priv->fillColor, progress, &progressColor);

		/* Set line color */
		clutter_cairo_set_source_color(inContext, &progressColor);

		/* Determine radius for rounded corners */
		radius=MIN(priv->fillCornersRadius, maxOffset);
		radius=MAX(0.0f, radius-offset);

		/* Draw rectangle with or without rounded corners */
		if((priv->type & XFCE_OVERVIEW_WORKSPACES_BACKGROUND_TYPE_ROUNDED_CORNERS) &&
			(priv->fillCorners & XFCE_OVERVIEW_WORKSPACES_CORNERS_ALL) &&
			priv->fillCornersRadius>0.0f &&
			radius>0.0f)
		{
			/* Top-left */
			if(priv->fillCorners & XFCE_OVERVIEW_WORKSPACES_CORNERS_TOP_LEFT)
			{
				cairo_move_to(inContext, 0+offset, 0+offset+radius);
				cairo_arc(inContext, 0+offset+radius, 0+offset+radius, radius, G_PI, G_PI*1.5);
			}
				else cairo_move_to(inContext, 0+offset, 0+offset);

			/* Top-right */
			if(priv->fillCorners & XFCE_OVERVIEW_WORKSPACES_CORNERS_TOP_RIGHT)
			{
				cairo_line_to(inContext, inWidth-radius-offset, 0+offset);
				cairo_arc(inContext, inWidth-radius-offset, 0+offset+radius, radius, G_PI*1.5, 0);
			}
				else cairo_line_to(inContext, inWidth-offset, 0+offset);

			/* Bottom-right */
			if(priv->fillCorners & XFCE_OVERVIEW_WORKSPACES_CORNERS_BOTTOM_RIGHT)
			{
				cairo_line_to(inContext, inWidth-offset, inHeight-offset-radius);
				cairo_arc(inContext, inWidth-offset-radius, inHeight-offset-radius, radius, 0, G_PI/2.0);
			}
				else cairo_line_to(inContext, inWidth-offset, inHeight-offset);

			/* Bottom-left */
			if(priv->fillCorners & XFCE_OVERVIEW_WORKSPACES_CORNERS_BOTTOM_LEFT)
			{
				cairo_line_to(inContext, 0+offset+radius, inHeight-offset);
				cairo_arc(inContext, 0+offset+radius, inHeight-offset-radius, radius, G_PI/2.0, G_PI);
			}
				else cairo_line_to(inContext, 0+offset, inHeight-offset);

			/* Close to top-left */
			if(priv->fillCorners & XFCE_OVERVIEW_WORKSPACES_CORNERS_TOP_LEFT) cairo_line_to(inContext, 0+offset, 0+offset+radius);
				else cairo_line_to(inContext, 0+offset, 0+offset);
		}
			else
			{
				cairo_rectangle(inContext, offset, offset, inWidth-offset-offset, inHeight-offset-offset);
			}

		/* Draw line for fill */
		cairo_stroke(inContext);

		/* Adjust offset for next path gradient line to draw */
		offset-=0.5f;
	}
}

/* Rectangle canvas (filling) should be redrawn */
static gboolean _xfce_overview_workspaces_background_on_draw_fill_canvas(XfceOverviewWorkspacesBackground *self,
															cairo_t *inContext,
															int inWidth,
															int inHeight,
															gpointer inUserData)
{
	XfceOverviewWorkspacesBackgroundPrivate	*priv;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_BACKGROUND(self), CLUTTER_EVENT_PROPAGATE);
	g_return_val_if_fail(CLUTTER_IS_CANVAS(inUserData), CLUTTER_EVENT_PROPAGATE);

	priv=self->priv;

	/* Clear current contents of the canvas */
	cairo_save(inContext);
	cairo_set_operator(inContext, CAIRO_OPERATOR_CLEAR);
	cairo_paint(inContext);
	cairo_restore(inContext);

	cairo_set_operator(inContext, CAIRO_OPERATOR_OVER);

	/* Do nothing if type does not include filling background */
	if(!(priv->type & XFCE_OVERVIEW_WORKSPACES_BACKGROUND_TYPE_FILL)) return(CLUTTER_EVENT_PROPAGATE);

	/* Depending on solid color or gradient type call draw function */
	if(xfce_overview_workspaces_gradient_color_get_gradient_type(priv->fillColor)!=XFCE_OVERVIEW_WORKSPACES_GRADIENT_TYPE_PATH_GRADIENT)
	{
		_xfce_overview_workspaces_background_draw_fill_canvas_simple(self, inContext, inWidth, inHeight);
	}
		else
		{
			_xfce_overview_workspaces_background_draw_fill_canvas_path_gradient(self, inContext, inWidth, inHeight);
		}

	/* Done filling canvas */
	return(CLUTTER_EVENT_PROPAGATE);
}

/* IMPLEMENTATION: ClutterActor */

/* Paint actor */
static void _xfce_overview_workspaces_background_paint_node(ClutterActor *self,
												ClutterPaintNode *inRootNode)
{
	XfceOverviewWorkspacesBackgroundPrivate	*priv=XFCE_OVERVIEW_WORKSPACES_BACKGROUND(self)->priv;
	ClutterContentIface				*iface;

	/* First draw canvas for background */
	if(priv->type & XFCE_OVERVIEW_WORKSPACES_BACKGROUND_TYPE_FILL)
	{
		iface=CLUTTER_CONTENT_GET_IFACE(priv->fillCanvas);
		if(iface->paint_content) iface->paint_content(priv->fillCanvas, self, inRootNode);
	}

	/* If available draw image for background */
	if(priv->image)
	{
		iface=CLUTTER_CONTENT_GET_IFACE(priv->image);
		if(iface->paint_content) iface->paint_content(CLUTTER_CONTENT(priv->image), self, inRootNode);
	}

	/* Now chain up to draw the actor */
	if(CLUTTER_ACTOR_CLASS(xfce_overview_workspaces_background_parent_class)->paint_node)
	{
		CLUTTER_ACTOR_CLASS(xfce_overview_workspaces_background_parent_class)->paint_node(self, inRootNode);
	}
}

/* Allocate position and size of actor and its children */
static void _xfce_overview_workspaces_background_allocate(ClutterActor *self,
												const ClutterActorBox *inBox,
												ClutterAllocationFlags inFlags)
{
	XfceOverviewWorkspacesBackgroundPrivate	*priv=XFCE_OVERVIEW_WORKSPACES_BACKGROUND(self)->priv;

	/* Chain up to store the allocation of the actor */
	CLUTTER_ACTOR_CLASS(xfce_overview_workspaces_background_parent_class)->allocate(self, inBox, inFlags);

	/* Set size of canvas */
	if(priv->fillCanvas)
	{
		clutter_canvas_set_size(CLUTTER_CANVAS(priv->fillCanvas),
									clutter_actor_box_get_width(inBox),
									clutter_actor_box_get_height(inBox));
	}
}

/* IMPLEMENTATION: GObject */

/* Dispose this object */
static void _xfce_overview_workspaces_background_dispose(GObject *inObject)
{
	/* Release allocated variables */
	XfceOverviewWorkspacesBackgroundPrivate	*priv=XFCE_OVERVIEW_WORKSPACES_BACKGROUND(inObject)->priv;

	if(priv->fillCanvas)
	{
		g_object_unref(priv->fillCanvas);
		priv->fillCanvas=NULL;
	}

	if(priv->image)
	{
		g_object_unref(priv->image);
		priv->image=NULL;
	}

	if(priv->fillColor)
	{
		xfce_overview_workspaces_gradient_color_free(priv->fillColor);
		priv->fillColor=NULL;
	}

	if(priv->outlineColor)
	{
		xfce_overview_workspaces_gradient_color_free(priv->outlineColor);
		priv->outlineColor=NULL;
	}

	if(priv->outline)
	{
		g_object_unref(priv->outline);
		priv->outline=NULL;
	}

	/* Call parent's class dispose method */
	G_OBJECT_CLASS(xfce_overview_workspaces_background_parent_class)->dispose(inObject);
}

/* Set/get properties */
static void _xfce_overview_workspaces_background_set_property(GObject *inObject,
													guint inPropID,
													const GValue *inValue,
													GParamSpec *inSpec)
{
	XfceOverviewWorkspacesBackground			*self=XFCE_OVERVIEW_WORKSPACES_BACKGROUND(inObject);

	switch(inPropID)
	{
		case PROP_TYPE:
			xfce_overview_workspaces_background_set_background_type(self, g_value_get_flags(inValue));
			break;

		case PROP_CORNERS:
			xfce_overview_workspaces_background_set_corners(self, g_value_get_flags(inValue));
			break;

		case PROP_CORNERS_RADIUS:
			xfce_overview_workspaces_background_set_corner_radius(self, g_value_get_float(inValue));
			break;

		case PROP_FILL_COLOR:
			xfce_overview_workspaces_background_set_fill_color(self, xfce_overview_workspaces_value_get_gradient_color(inValue));
			break;

		case PROP_FILL_CORNERS:
			xfce_overview_workspaces_background_set_fill_corners(self, g_value_get_flags(inValue));
			break;

		case PROP_FILL_CORNERS_RADIUS:
			xfce_overview_workspaces_background_set_fill_corner_radius(self, g_value_get_float(inValue));
			break;

		case PROP_OUTLINE_COLOR:
			xfce_overview_workspaces_background_set_outline_color(self, g_value_get_boxed(inValue));
			break;

		case PROP_OUTLINE_WIDTH:
			xfce_overview_workspaces_background_set_outline_width(self, g_value_get_float(inValue));
			break;

		case PROP_OUTLINE_BORDERS:
			xfce_overview_workspaces_background_set_outline_borders(self, g_value_get_flags(inValue));
			break;

		case PROP_OUTLINE_CORNERS:
			xfce_overview_workspaces_background_set_outline_corners(self, g_value_get_flags(inValue));
			break;

		case PROP_OUTLINE_CORNERS_RADIUS:
			xfce_overview_workspaces_background_set_outline_corner_radius(self, g_value_get_float(inValue));
			break;

		case PROP_IMAGE:
			xfce_overview_workspaces_background_set_image(self, CLUTTER_IMAGE(g_value_get_object(inValue)));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(inObject, inPropID, inSpec);
			break;
	}
}

static void _xfce_overview_workspaces_background_get_property(GObject *inObject,
													guint inPropID,
													GValue *outValue,
													GParamSpec *inSpec)
{
	XfceOverviewWorkspacesBackground			*self=XFCE_OVERVIEW_WORKSPACES_BACKGROUND(inObject);
	XfceOverviewWorkspacesBackgroundPrivate	*priv=self->priv;

	switch(inPropID)
	{
		case PROP_TYPE:
			g_value_set_flags(outValue, priv->type);
			break;

		case PROP_FILL_COLOR:
			xfce_overview_workspaces_value_set_gradient_color(outValue, priv->fillColor);
			break;

		case PROP_FILL_CORNERS:
			g_value_set_flags(outValue, priv->fillCorners);
			break;

		case PROP_FILL_CORNERS_RADIUS:
			g_value_set_float(outValue, priv->fillCornersRadius);
			break;

		case PROP_OUTLINE_COLOR:
			g_value_set_boxed(outValue, priv->outlineColor);
			break;

		case PROP_OUTLINE_WIDTH:
			g_value_set_float(outValue, priv->outlineWidth);
			break;

		case PROP_OUTLINE_BORDERS:
			g_value_set_flags(outValue, priv->outlineBorders);
			break;

		case PROP_OUTLINE_CORNERS:
			g_value_set_flags(outValue, priv->outlineCorners);
			break;

		case PROP_OUTLINE_CORNERS_RADIUS:
			g_value_set_float(outValue, priv->outlineCornersRadius);
			break;

		case PROP_IMAGE:
			g_value_set_object(outValue, priv->image);
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
static void xfce_overview_workspaces_background_class_init(XfceOverviewWorkspacesBackgroundClass *klass)
{
	XfceOverviewWorkspacesActorClass			*actorClass=XFCE_OVERVIEW_WORKSPACES_ACTOR_CLASS(klass);
	ClutterActorClass				*clutterActorClass=CLUTTER_ACTOR_CLASS(klass);
	GObjectClass					*gobjectClass=G_OBJECT_CLASS(klass);
	static XfceOverviewWorkspacesGradientColor	*defaultFillColor=NULL;
	static XfceOverviewWorkspacesGradientColor	*defaultOutlineColor=NULL;

	/* Set up default value for param spec */
	if(G_UNLIKELY(!defaultFillColor))
	{
		defaultFillColor=xfce_overview_workspaces_gradient_color_new_solid(CLUTTER_COLOR_Black);
	}

	if(G_UNLIKELY(!defaultOutlineColor))
	{
		defaultOutlineColor=xfce_overview_workspaces_gradient_color_new_solid(CLUTTER_COLOR_White);
	}

	/* Override functions */
	gobjectClass->dispose=_xfce_overview_workspaces_background_dispose;
	gobjectClass->set_property=_xfce_overview_workspaces_background_set_property;
	gobjectClass->get_property=_xfce_overview_workspaces_background_get_property;

	clutterActorClass->paint_node=_xfce_overview_workspaces_background_paint_node;
	clutterActorClass->allocate=_xfce_overview_workspaces_background_allocate;

	/* Define properties */
	XfceOverviewWorkspacesBackgroundProperties[PROP_TYPE]=
		g_param_spec_flags("background-type",
							"Background type",
							"Background type",
							XFCE_OVERVIEW_WORKSPACES_TYPE_BACKGROUND_TYPE,
							XFCE_OVERVIEW_WORKSPACES_BACKGROUND_TYPE_NONE,
							G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	XfceOverviewWorkspacesBackgroundProperties[PROP_CORNERS]=
		g_param_spec_flags("corners",
							"Corners",
							"Determines which corners are rounded for background and outline",
							XFCE_OVERVIEW_WORKSPACES_TYPE_CORNERS,
							XFCE_OVERVIEW_WORKSPACES_CORNERS_ALL,
							G_PARAM_WRITABLE | G_PARAM_STATIC_STRINGS);

	XfceOverviewWorkspacesBackgroundProperties[PROP_CORNERS_RADIUS]=
		g_param_spec_float("corner-radius",
							"Corner radius",
							"Radius of rounded corners for background and outline",
							0.0f, G_MAXFLOAT,
							0.0f,
							G_PARAM_WRITABLE | G_PARAM_STATIC_STRINGS);

	XfceOverviewWorkspacesBackgroundProperties[PROP_FILL_COLOR]=
		xfce_overview_workspaces_param_spec_gradient_color("background-fill-color",
												"Background fill color",
												"Color to fill background with",
												defaultFillColor,
												G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	XfceOverviewWorkspacesBackgroundProperties[PROP_FILL_CORNERS]=
		g_param_spec_flags("background-fill-corners",
							"Fill corners",
							"Determines which corners are rounded at background",
							XFCE_OVERVIEW_WORKSPACES_TYPE_CORNERS,
							XFCE_OVERVIEW_WORKSPACES_CORNERS_ALL,
							G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	XfceOverviewWorkspacesBackgroundProperties[PROP_FILL_CORNERS_RADIUS]=
		g_param_spec_float("background-fill-corner-radius",
							"Corner radius",
							"Radius of rounded corners of background",
							0.0f, G_MAXFLOAT,
							0.0f,
							G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	XfceOverviewWorkspacesBackgroundProperties[PROP_OUTLINE_COLOR]=
		xfce_overview_workspaces_param_spec_gradient_color("outline-color",
												"Outline color",
												"Color to draw outline with",
												defaultOutlineColor,
												G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	XfceOverviewWorkspacesBackgroundProperties[PROP_OUTLINE_WIDTH]=
		g_param_spec_float("outline-width",
							"Outline width",
							"Width of line used to draw outline",
							0.0f, G_MAXFLOAT,
							1.0f,
							G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	XfceOverviewWorkspacesBackgroundProperties[PROP_OUTLINE_BORDERS]=
		g_param_spec_flags("outline-borders",
							"Outline borders",
							"Determines which sides of border of outline should be drawn",
							XFCE_OVERVIEW_WORKSPACES_TYPE_BORDERS,
							XFCE_OVERVIEW_WORKSPACES_BORDERS_ALL,
							G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	XfceOverviewWorkspacesBackgroundProperties[PROP_OUTLINE_CORNERS]=
		g_param_spec_flags("outline-corners",
							"Outline corners",
							"Determines which corners are rounded at outline",
							XFCE_OVERVIEW_WORKSPACES_TYPE_CORNERS,
							XFCE_OVERVIEW_WORKSPACES_CORNERS_ALL,
							G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	XfceOverviewWorkspacesBackgroundProperties[PROP_OUTLINE_CORNERS_RADIUS]=
		g_param_spec_float("outline-corner-radius",
							"Outline corner radius",
							"Radius of rounded corners of outline",
							0.0f, G_MAXFLOAT,
							0.0f,
							G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	XfceOverviewWorkspacesBackgroundProperties[PROP_IMAGE]=
		g_param_spec_object("image",
							"Image",
							"Image to draw as background",
							CLUTTER_TYPE_IMAGE,
							G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(gobjectClass, PROP_LAST, XfceOverviewWorkspacesBackgroundProperties);

	/* Define stylable properties */
	xfce_overview_workspaces_actor_install_stylable_property(actorClass, XfceOverviewWorkspacesBackgroundProperties[PROP_TYPE]);
	xfce_overview_workspaces_actor_install_stylable_property(actorClass, XfceOverviewWorkspacesBackgroundProperties[PROP_CORNERS]);
	xfce_overview_workspaces_actor_install_stylable_property(actorClass, XfceOverviewWorkspacesBackgroundProperties[PROP_CORNERS_RADIUS]);
	xfce_overview_workspaces_actor_install_stylable_property(actorClass, XfceOverviewWorkspacesBackgroundProperties[PROP_FILL_COLOR]);
	xfce_overview_workspaces_actor_install_stylable_property(actorClass, XfceOverviewWorkspacesBackgroundProperties[PROP_FILL_CORNERS]);
	xfce_overview_workspaces_actor_install_stylable_property(actorClass, XfceOverviewWorkspacesBackgroundProperties[PROP_FILL_CORNERS_RADIUS]);
	xfce_overview_workspaces_actor_install_stylable_property(actorClass, XfceOverviewWorkspacesBackgroundProperties[PROP_OUTLINE_COLOR]);
	xfce_overview_workspaces_actor_install_stylable_property(actorClass, XfceOverviewWorkspacesBackgroundProperties[PROP_OUTLINE_WIDTH]);
	xfce_overview_workspaces_actor_install_stylable_property(actorClass, XfceOverviewWorkspacesBackgroundProperties[PROP_OUTLINE_BORDERS]);
	xfce_overview_workspaces_actor_install_stylable_property(actorClass, XfceOverviewWorkspacesBackgroundProperties[PROP_OUTLINE_CORNERS]);
	xfce_overview_workspaces_actor_install_stylable_property(actorClass, XfceOverviewWorkspacesBackgroundProperties[PROP_OUTLINE_CORNERS_RADIUS]);
	xfce_overview_workspaces_actor_install_stylable_property(actorClass, XfceOverviewWorkspacesBackgroundProperties[PROP_IMAGE]);
}

/* Object initialization
 * Create private structure and set up default values
 */
static void xfce_overview_workspaces_background_init(XfceOverviewWorkspacesBackground *self)
{
	XfceOverviewWorkspacesBackgroundPrivate	*priv;

	priv=self->priv=xfce_overview_workspaces_background_get_instance_private(self);

	/* This actor reacts on events */
	clutter_actor_set_reactive(CLUTTER_ACTOR(self), TRUE);

	/* Set up default values */
	priv->type=XFCE_OVERVIEW_WORKSPACES_BACKGROUND_TYPE_NONE;

	priv->fillCanvas=clutter_canvas_new();
	priv->fillColor=xfce_overview_workspaces_gradient_color_new_solid(CLUTTER_COLOR_Black);
	priv->fillCorners=XFCE_OVERVIEW_WORKSPACES_CORNERS_ALL;
	priv->fillCornersRadius=0.0f;

	priv->outline=XFCE_OVERVIEW_WORKSPACES_OUTLINE_EFFECT(g_object_ref(xfce_overview_workspaces_outline_effect_new()));
	priv->outlineColor=xfce_overview_workspaces_gradient_color_new_solid(CLUTTER_COLOR_White);
	priv->outlineWidth=1.0f;
	priv->outlineCorners=XFCE_OVERVIEW_WORKSPACES_CORNERS_ALL;
	priv->outlineBorders=XFCE_OVERVIEW_WORKSPACES_BORDERS_ALL;
	priv->outlineCornersRadius=0.0f;

	priv->image=NULL;

	/* Set up actor */
	clutter_actor_set_content_scaling_filters(CLUTTER_ACTOR(self),
												CLUTTER_SCALING_FILTER_TRILINEAR,
												CLUTTER_SCALING_FILTER_LINEAR);

	clutter_actor_meta_set_enabled(CLUTTER_ACTOR_META(priv->outline), FALSE);
	clutter_actor_add_effect(CLUTTER_ACTOR(self), CLUTTER_EFFECT(priv->outline));

	/* Connect signals */
	g_signal_connect_swapped(priv->fillCanvas, "draw", G_CALLBACK(_xfce_overview_workspaces_background_on_draw_fill_canvas), self);
}

/* IMPLEMENTATION: Public API */

/* Create new actor */
ClutterActor* xfce_overview_workspaces_background_new(void)
{
	return(g_object_new(XFCE_OVERVIEW_WORKSPACES_TYPE_BACKGROUND,
						"background-type", XFCE_OVERVIEW_WORKSPACES_BACKGROUND_TYPE_NONE,
						NULL));
}

/* Get/set type of background */
XfceOverviewWorkspacesBackgroundType xfce_overview_workspaces_background_get_background_type(XfceOverviewWorkspacesBackground *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_BACKGROUND(self), XFCE_OVERVIEW_WORKSPACES_BACKGROUND_TYPE_NONE);

	return(self->priv->type);
}

void xfce_overview_workspaces_background_set_background_type(XfceOverviewWorkspacesBackground *self, const XfceOverviewWorkspacesBackgroundType inType)
{
	XfceOverviewWorkspacesBackgroundPrivate	*priv;
	gboolean						enableOutline;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_BACKGROUND(self));

	priv=self->priv;

	/* Set value if changed */
	if(priv->type!=inType)
	{
		/* Set value */
		priv->type=inType;

		/* Force redraw of background canvas */
		if(priv->fillCanvas) clutter_content_invalidate(priv->fillCanvas);

		/* Enable or disable drawing outline and also check if type does not
		 * include rounded corners set corner radius at outline to zero. But
		 * if set then set requested outline corner radius at outline.
		 */
		if(priv->outline)
		{
			/* Enable or disable outlines */
			if(inType & XFCE_OVERVIEW_WORKSPACES_BACKGROUND_TYPE_OUTLINE) enableOutline=TRUE;
				else enableOutline=FALSE;

			clutter_actor_meta_set_enabled(CLUTTER_ACTOR_META(priv->outline), enableOutline);

			/* Set corner radius at outline depending on if rounded corners
			 * at type is set or not.
			 */
			if(inType & XFCE_OVERVIEW_WORKSPACES_BACKGROUND_TYPE_ROUNDED_CORNERS)
			{
				xfce_overview_workspaces_outline_effect_set_corner_radius(priv->outline, priv->outlineCornersRadius);
			}
				else
				{
					xfce_overview_workspaces_outline_effect_set_corner_radius(priv->outline, 0.0f);
				}
		}

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesBackgroundProperties[PROP_TYPE]);
	}
}

/* Set radius for rounded corners of background and outline */
void xfce_overview_workspaces_background_set_corners(XfceOverviewWorkspacesBackground *self, XfceOverviewWorkspacesCorners inCorners)
{
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_BACKGROUND(self));

	/* Set radius */
	xfce_overview_workspaces_background_set_fill_corners(self, inCorners);
	xfce_overview_workspaces_background_set_outline_corners(self, inCorners);
}

/* Set radius for rounded corners of background and outline */
void xfce_overview_workspaces_background_set_corner_radius(XfceOverviewWorkspacesBackground *self, const gfloat inRadius)
{
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_BACKGROUND(self));
	g_return_if_fail(inRadius>=0.0f);

	/* Set radius */
	xfce_overview_workspaces_background_set_fill_corner_radius(self, inRadius);
	xfce_overview_workspaces_background_set_outline_corner_radius(self, inRadius);
}

/* Get/set color to fill background with */
const XfceOverviewWorkspacesGradientColor* xfce_overview_workspaces_background_get_fill_color(XfceOverviewWorkspacesBackground *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_BACKGROUND(self), NULL);

	return(self->priv->fillColor);
}

void xfce_overview_workspaces_background_set_fill_color(XfceOverviewWorkspacesBackground *self, const XfceOverviewWorkspacesGradientColor *inColor)
{
	XfceOverviewWorkspacesBackgroundPrivate	*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_BACKGROUND(self));
	g_return_if_fail(inColor);

	priv=self->priv;

	/* Set value if changed */
	if(priv->fillColor==NULL ||
		!xfce_overview_workspaces_gradient_color_equal(inColor, priv->fillColor))
	{
		/* Set value */
		if(priv->fillColor) xfce_overview_workspaces_gradient_color_free(priv->fillColor);
		priv->fillColor=xfce_overview_workspaces_gradient_color_copy(inColor);

		/* Invalidate canvas to get it redrawn */
		if(priv->fillCanvas) clutter_content_invalidate(priv->fillCanvas);

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesBackgroundProperties[PROP_FILL_COLOR]);
	}
}

/* Get/set corners to draw rounded when drawing background */
XfceOverviewWorkspacesCorners xfce_overview_workspaces_background_get_fill_corners(XfceOverviewWorkspacesBackground *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_BACKGROUND(self), XFCE_OVERVIEW_WORKSPACES_CORNERS_NONE);

	return(self->priv->fillCorners);
}

void xfce_overview_workspaces_background_set_fill_corners(XfceOverviewWorkspacesBackground *self, const XfceOverviewWorkspacesCorners inCorners)
{
	XfceOverviewWorkspacesBackgroundPrivate	*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_BACKGROUND(self));

	priv=self->priv;

	/* Set value if changed */
	if(priv->fillCorners!=inCorners)
	{
		/* Set value */
		priv->fillCorners=inCorners;

		/* Invalidate canvas to get it redrawn */
		if(priv->fillCanvas) clutter_content_invalidate(priv->fillCanvas);

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesBackgroundProperties[PROP_FILL_CORNERS]);
	}
}

/* Get/set radius for rounded corners when drawing background */
gfloat xfce_overview_workspaces_background_get_fill_corner_radius(XfceOverviewWorkspacesBackground *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_BACKGROUND(self), 0.0f);

	return(self->priv->fillCornersRadius);
}

void xfce_overview_workspaces_background_set_fill_corner_radius(XfceOverviewWorkspacesBackground *self, const gfloat inRadius)
{
	XfceOverviewWorkspacesBackgroundPrivate	*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_BACKGROUND(self));
	g_return_if_fail(inRadius>=0.0f);

	priv=self->priv;

	/* Set value if changed */
	if(priv->fillCornersRadius!=inRadius)
	{
		/* Set value */
		priv->fillCornersRadius=inRadius;

		/* Invalidate canvas to get it redrawn */
		if(priv->fillCanvas) clutter_content_invalidate(priv->fillCanvas);

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesBackgroundProperties[PROP_FILL_CORNERS_RADIUS]);
	}
}

/* Get/set color to draw outline with */
const XfceOverviewWorkspacesGradientColor* xfce_overview_workspaces_background_get_outline_color(XfceOverviewWorkspacesBackground *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_BACKGROUND(self), NULL);

	return(self->priv->outlineColor);
}

void xfce_overview_workspaces_background_set_outline_color(XfceOverviewWorkspacesBackground *self, const XfceOverviewWorkspacesGradientColor *inColor)
{
	XfceOverviewWorkspacesBackgroundPrivate	*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_BACKGROUND(self));
	g_return_if_fail(inColor);

	priv=self->priv;

	/* Set value if changed */
	if(priv->outlineColor==NULL ||
		!xfce_overview_workspaces_gradient_color_equal(inColor, priv->outlineColor))
	{
		/* Set value */
		if(priv->outlineColor) xfce_overview_workspaces_gradient_color_free(priv->outlineColor);
		priv->outlineColor=xfce_overview_workspaces_gradient_color_copy(inColor);

		/* Update effect */
		if(priv->outline) xfce_overview_workspaces_outline_effect_set_color(priv->outline, priv->outlineColor);

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesBackgroundProperties[PROP_OUTLINE_COLOR]);
	}
}

/* Get/set line width for outline */
gfloat xfce_overview_workspaces_background_get_outline_width(XfceOverviewWorkspacesBackground *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_BACKGROUND(self), 0.0f);

	return(self->priv->outlineWidth);
}

void xfce_overview_workspaces_background_set_outline_width(XfceOverviewWorkspacesBackground *self, const gfloat inWidth)
{
	XfceOverviewWorkspacesBackgroundPrivate	*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_BACKGROUND(self));
	g_return_if_fail(inWidth>=0.0f);

	priv=self->priv;

	/* Set value if changed */
	if(priv->outlineWidth!=inWidth)
	{
		/* Set value */
		priv->outlineWidth=inWidth;

		/* Update effect */
		if(priv->outline) xfce_overview_workspaces_outline_effect_set_width(priv->outline, inWidth);

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesBackgroundProperties[PROP_OUTLINE_WIDTH]);
	}
}

/* Get/set sides of border of outline to draw rounded when drawing outline */
XfceOverviewWorkspacesBorders xfce_overview_workspaces_background_get_outline_borders(XfceOverviewWorkspacesBackground *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_BACKGROUND(self), XFCE_OVERVIEW_WORKSPACES_BORDERS_NONE);

	return(self->priv->outlineBorders);
}

void xfce_overview_workspaces_background_set_outline_borders(XfceOverviewWorkspacesBackground *self, const XfceOverviewWorkspacesBorders inBorders)
{
	XfceOverviewWorkspacesBackgroundPrivate	*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_BACKGROUND(self));

	priv=self->priv;

	/* Set value if changed */
	if(priv->outlineBorders!=inBorders)
	{
		/* Set value */
		priv->outlineBorders=inBorders;

		/* Update effect */
		if(priv->outline) xfce_overview_workspaces_outline_effect_set_borders(priv->outline, inBorders);

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesBackgroundProperties[PROP_OUTLINE_BORDERS]);
	}
}

/* Get/set corners to draw rounded when drawing outline */
XfceOverviewWorkspacesCorners xfce_overview_workspaces_background_get_outline_corners(XfceOverviewWorkspacesBackground *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_BACKGROUND(self), XFCE_OVERVIEW_WORKSPACES_CORNERS_NONE);

	return(self->priv->outlineCorners);
}

void xfce_overview_workspaces_background_set_outline_corners(XfceOverviewWorkspacesBackground *self, const XfceOverviewWorkspacesCorners inCorners)
{
	XfceOverviewWorkspacesBackgroundPrivate	*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_BACKGROUND(self));

	priv=self->priv;

	/* Set value if changed */
	if(priv->outlineCorners!=inCorners)
	{
		/* Set value */
		priv->outlineCorners=inCorners;

		/* Update effect */
		if(priv->outline) xfce_overview_workspaces_outline_effect_set_corners(priv->outline, inCorners);

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesBackgroundProperties[PROP_OUTLINE_CORNERS]);
	}
}

/* Get/set radius for rounded corners when drawing outline */
gfloat xfce_overview_workspaces_background_get_outline_corner_radius(XfceOverviewWorkspacesBackground *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_BACKGROUND(self), 0.0f);

	return(self->priv->fillCornersRadius);
}

void xfce_overview_workspaces_background_set_outline_corner_radius(XfceOverviewWorkspacesBackground *self, const gfloat inRadius)
{
	XfceOverviewWorkspacesBackgroundPrivate	*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_BACKGROUND(self));
	g_return_if_fail(inRadius>=0.0f);

	priv=self->priv;

	/* Set value if changed */
	if(priv->outlineCornersRadius!=inRadius)
	{
		/* Set value */
		priv->outlineCornersRadius=inRadius;

		/* Update effect but depend on type to decide if either a radius of zero
		 * is set because no rounded corners were requested or to set the radius
		 * is provided.
		 */
		if(priv->outline)
		{
			if(priv->type & XFCE_OVERVIEW_WORKSPACES_BACKGROUND_TYPE_ROUNDED_CORNERS)
			{
				xfce_overview_workspaces_outline_effect_set_corner_radius(priv->outline, priv->outlineCornersRadius);
			}
				else
				{
					xfce_overview_workspaces_outline_effect_set_corner_radius(priv->outline, 0.0f);
				}
		}

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesBackgroundProperties[PROP_OUTLINE_CORNERS_RADIUS]);
	}
}

/* Get/set image for background */
ClutterImage* xfce_overview_workspaces_background_get_image(XfceOverviewWorkspacesBackground *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_BACKGROUND(self), NULL);

	return(self->priv->image);
}

void xfce_overview_workspaces_background_set_image(XfceOverviewWorkspacesBackground *self, ClutterImage *inImage)
{
	XfceOverviewWorkspacesBackgroundPrivate	*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_BACKGROUND(self));
	g_return_if_fail(inImage==NULL || CLUTTER_IS_IMAGE(inImage));

	priv=self->priv;

	/* Set value if changed */
	if(priv->image!=inImage)
	{
		/* Set value */
		if(priv->image)
		{
			g_object_unref(priv->image);
			priv->image=NULL;
		}

		if(inImage) priv->image=CLUTTER_IMAGE(g_object_ref(inImage));

		/* Invalidate canvas to get it redrawn */
		if(priv->image) clutter_content_invalidate(CLUTTER_CONTENT(priv->image));

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesBackgroundProperties[PROP_IMAGE]);
	}
}
