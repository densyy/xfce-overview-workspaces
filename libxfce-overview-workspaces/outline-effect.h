/*
 * outline-effect: Draws an outline on top of actor
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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_OUTLINE_EFFECT__
#define __LIBXFCE_OVERVIEW_WORKSPACES_OUTLINE_EFFECT__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <clutter/clutter.h>

#include <libxfce-overview-workspaces/gradient-color.h>
#include <libxfce-overview-workspaces/types.h>

G_BEGIN_DECLS

#define XFCE_OVERVIEW_WORKSPACES_TYPE_OUTLINE_EFFECT				(xfce_overview_workspaces_outline_effect_get_type())
#define XFCE_OVERVIEW_WORKSPACES_OUTLINE_EFFECT(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_OUTLINE_EFFECT, XfceOverviewWorkspacesOutlineEffect))
#define XFCE_OVERVIEW_WORKSPACES_IS_OUTLINE_EFFECT(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_OUTLINE_EFFECT))
#define XFCE_OVERVIEW_WORKSPACES_OUTLINE_EFFECT_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_OUTLINE_EFFECT, XfceOverviewWorkspacesOutlineEffectClass))
#define XFCE_OVERVIEW_WORKSPACES_IS_OUTLINE_EFFECT_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_OUTLINE_EFFECT))
#define XFCE_OVERVIEW_WORKSPACES_OUTLINE_EFFECT_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_OUTLINE_EFFECT, XfceOverviewWorkspacesOutlineEffectClass))

typedef struct _XfceOverviewWorkspacesOutlineEffect			XfceOverviewWorkspacesOutlineEffect;
typedef struct _XfceOverviewWorkspacesOutlineEffectClass		XfceOverviewWorkspacesOutlineEffectClass;
typedef struct _XfceOverviewWorkspacesOutlineEffectPrivate		XfceOverviewWorkspacesOutlineEffectPrivate;

struct _XfceOverviewWorkspacesOutlineEffect
{
	/*< private >*/
	/* Parent instance */
	ClutterEffect						parent_instance;

	/* Private structure */
	XfceOverviewWorkspacesOutlineEffectPrivate		*priv;
};

struct _XfceOverviewWorkspacesOutlineEffectClass
{
	/*< private >*/
	/* Parent class */
	ClutterEffectClass					parent_class;
};

/* Public API */
GType xfce_overview_workspaces_outline_effect_get_type(void) G_GNUC_CONST;

ClutterEffect* xfce_overview_workspaces_outline_effect_new(void);

const XfceOverviewWorkspacesGradientColor* xfce_overview_workspaces_outline_effect_get_color(XfceOverviewWorkspacesOutlineEffect *self);
void xfce_overview_workspaces_outline_effect_set_color(XfceOverviewWorkspacesOutlineEffect *self, const XfceOverviewWorkspacesGradientColor *inColor);

gfloat xfce_overview_workspaces_outline_effect_get_width(XfceOverviewWorkspacesOutlineEffect *self);
void xfce_overview_workspaces_outline_effect_set_width(XfceOverviewWorkspacesOutlineEffect *self, const gfloat inWidth);

XfceOverviewWorkspacesBorders xfce_overview_workspaces_outline_effect_get_borders(XfceOverviewWorkspacesOutlineEffect *self);
void xfce_overview_workspaces_outline_effect_set_borders(XfceOverviewWorkspacesOutlineEffect *self, const XfceOverviewWorkspacesBorders inBorders);

XfceOverviewWorkspacesCorners xfce_overview_workspaces_outline_effect_get_corners(XfceOverviewWorkspacesOutlineEffect *self);
void xfce_overview_workspaces_outline_effect_set_corners(XfceOverviewWorkspacesOutlineEffect *self, const XfceOverviewWorkspacesCorners inCorners);

gfloat xfce_overview_workspaces_outline_effect_get_corner_radius(XfceOverviewWorkspacesOutlineEffect *self);
void xfce_overview_workspaces_outline_effect_set_corner_radius(XfceOverviewWorkspacesOutlineEffect *self, const gfloat inRadius);

G_END_DECLS

#endif	/* __LIBXFCE_OVERVIEW_WORKSPACES_OUTLINE_EFFECT__ */
