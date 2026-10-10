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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_BACKGROUND__
#define __LIBXFCE_OVERVIEW_WORKSPACES_BACKGROUND__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <clutter/clutter.h>

#include <libxfce-overview-workspaces/actor.h>
#include <libxfce-overview-workspaces/gradient-color.h>
#include <libxfce-overview-workspaces/types.h>

G_BEGIN_DECLS

/* Public definitions */
/**
 * XfceOverviewWorkspacesBackgroundType:
 * @XFCE_OVERVIEW_WORKSPACES_BACKGROUND_TYPE_NONE: The actor will be displayed unmodified.
 * @XFCE_OVERVIEW_WORKSPACES_BACKGROUND_TYPE_FILL: The actor background will be filled with a color.
 * @XFCE_OVERVIEW_WORKSPACES_BACKGROUND_TYPE_OUTLINE: The actor will get an outline.
 * @XFCE_OVERVIEW_WORKSPACES_BACKGROUND_TYPE_ROUNDED_CORNERS: The edges of actor will be rounded.
 *
 * Determines how the background of an actor will be displayed and if it get an styled outline.
 */
typedef enum /*< flags,prefix=XFCE_OVERVIEW_WORKSPACES_BACKGROUND_TYPE >*/
{
	XFCE_OVERVIEW_WORKSPACES_BACKGROUND_TYPE_NONE=0,

	XFCE_OVERVIEW_WORKSPACES_BACKGROUND_TYPE_FILL=1 << 0,
	XFCE_OVERVIEW_WORKSPACES_BACKGROUND_TYPE_OUTLINE=1 << 1,
	XFCE_OVERVIEW_WORKSPACES_BACKGROUND_TYPE_ROUNDED_CORNERS=1 << 2
} XfceOverviewWorkspacesBackgroundType;


/* Object declaration */
#define XFCE_OVERVIEW_WORKSPACES_TYPE_BACKGROUND					(xfce_overview_workspaces_background_get_type())
#define XFCE_OVERVIEW_WORKSPACES_BACKGROUND(obj)					(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_BACKGROUND, XfceOverviewWorkspacesBackground))
#define XFCE_OVERVIEW_WORKSPACES_IS_BACKGROUND(obj)				(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_BACKGROUND))
#define XFCE_OVERVIEW_WORKSPACES_BACKGROUND_CLASS(klass)			(G_TYPE_CHECK_CLASS_CAST((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_BACKGROUND, XfceOverviewWorkspacesBackgroundClass))
#define XFCE_OVERVIEW_WORKSPACES_IS_BACKGROUND_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_BACKGROUND))
#define XFCE_OVERVIEW_WORKSPACES_BACKGROUND_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_BACKGROUND, XfceOverviewWorkspacesBackgroundClass))

typedef struct _XfceOverviewWorkspacesBackground				XfceOverviewWorkspacesBackground;
typedef struct _XfceOverviewWorkspacesBackgroundClass			XfceOverviewWorkspacesBackgroundClass;
typedef struct _XfceOverviewWorkspacesBackgroundPrivate		XfceOverviewWorkspacesBackgroundPrivate;

/**
 * XfceOverviewWorkspacesBackground:
 *
 * The #XfceOverviewWorkspacesBackground structure contains only private data and
 * should be accessed using the provided API
 */struct _XfceOverviewWorkspacesBackground
{
	/*< private >*/
	/* Parent instance */
	XfceOverviewWorkspacesActor				parent_instance;

	/* Private structure */
	XfceOverviewWorkspacesBackgroundPrivate	*priv;
};

/**
 * XfceOverviewWorkspacesBackgroundClass:
 *
 * The #XfceOverviewWorkspacesBackgroundClass structure contains only private data
 */
struct _XfceOverviewWorkspacesBackgroundClass
{
	/*< private >*/
	/* Parent class */
	XfceOverviewWorkspacesActorClass			parent_class;

	/*< public >*/
	/* Virtual functions */
};

/* Public API */
GType xfce_overview_workspaces_background_get_type(void) G_GNUC_CONST;

ClutterActor* xfce_overview_workspaces_background_new(void);

/* General functions */
XfceOverviewWorkspacesBackgroundType xfce_overview_workspaces_background_get_background_type(XfceOverviewWorkspacesBackground *self);
void xfce_overview_workspaces_background_set_background_type(XfceOverviewWorkspacesBackground *self, const XfceOverviewWorkspacesBackgroundType inType);

void xfce_overview_workspaces_background_set_corners(XfceOverviewWorkspacesBackground *self, XfceOverviewWorkspacesCorners inCorners);
void xfce_overview_workspaces_background_set_corner_radius(XfceOverviewWorkspacesBackground *self, const gfloat inRadius);

/* Fill functions */
const XfceOverviewWorkspacesGradientColor* xfce_overview_workspaces_background_get_fill_color(XfceOverviewWorkspacesBackground *self);
void xfce_overview_workspaces_background_set_fill_color(XfceOverviewWorkspacesBackground *self, const XfceOverviewWorkspacesGradientColor *inColor);

XfceOverviewWorkspacesCorners xfce_overview_workspaces_background_get_fill_corners(XfceOverviewWorkspacesBackground *self);
void xfce_overview_workspaces_background_set_fill_corners(XfceOverviewWorkspacesBackground *self, const XfceOverviewWorkspacesCorners inCorners);

gfloat xfce_overview_workspaces_background_get_fill_corner_radius(XfceOverviewWorkspacesBackground *self);
void xfce_overview_workspaces_background_set_fill_corner_radius(XfceOverviewWorkspacesBackground *self, const gfloat inRadius);

/* Outline functions */
const XfceOverviewWorkspacesGradientColor* xfce_overview_workspaces_background_get_outline_color(XfceOverviewWorkspacesBackground *self);
void xfce_overview_workspaces_background_set_outline_color(XfceOverviewWorkspacesBackground *self, const XfceOverviewWorkspacesGradientColor *inColor);

gfloat xfce_overview_workspaces_background_get_outline_width(XfceOverviewWorkspacesBackground *self);
void xfce_overview_workspaces_background_set_outline_width(XfceOverviewWorkspacesBackground *self, const gfloat inWidth);

XfceOverviewWorkspacesBorders xfce_overview_workspaces_background_get_outline_borders(XfceOverviewWorkspacesBackground *self);
void xfce_overview_workspaces_background_set_outline_borders(XfceOverviewWorkspacesBackground *self, const XfceOverviewWorkspacesBorders inBorders);

XfceOverviewWorkspacesCorners xfce_overview_workspaces_background_get_outline_corners(XfceOverviewWorkspacesBackground *self);
void xfce_overview_workspaces_background_set_outline_corners(XfceOverviewWorkspacesBackground *self, const XfceOverviewWorkspacesCorners inCorners);

gfloat xfce_overview_workspaces_background_get_outline_corner_radius(XfceOverviewWorkspacesBackground *self);
void xfce_overview_workspaces_background_set_outline_corner_radius(XfceOverviewWorkspacesBackground *self, const gfloat inRadius);

/* Image functions */
ClutterImage* xfce_overview_workspaces_background_get_image(XfceOverviewWorkspacesBackground *self);
void xfce_overview_workspaces_background_set_image(XfceOverviewWorkspacesBackground *self, ClutterImage *inImage);

G_END_DECLS

#endif	/* __LIBXFCE_OVERVIEW_WORKSPACES_BACKGROUND__ */
