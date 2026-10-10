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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_EMBLEM_EFFECT__
#define __LIBXFCE_OVERVIEW_WORKSPACES_EMBLEM_EFFECT__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <clutter/clutter.h>

#include <libxfce-overview-workspaces/types.h>

G_BEGIN_DECLS

#define XFCE_OVERVIEW_WORKSPACES_TYPE_EMBLEM_EFFECT				(xfce_overview_workspaces_emblem_effect_get_type())
#define XFCE_OVERVIEW_WORKSPACES_EMBLEM_EFFECT(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_EMBLEM_EFFECT, XfceOverviewWorkspacesEmblemEffect))
#define XFCE_OVERVIEW_WORKSPACES_IS_EMBLEM_EFFECT(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_EMBLEM_EFFECT))
#define XFCE_OVERVIEW_WORKSPACES_EMBLEM_EFFECT_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_EMBLEM_EFFECT, XfceOverviewWorkspacesEmblemEffectClass))
#define XFCE_OVERVIEW_WORKSPACES_IS_EMBLEM_EFFECT_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_EMBLEM_EFFECT))
#define XFCE_OVERVIEW_WORKSPACES_EMBLEM_EFFECT_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_EMBLEM_EFFECT, XfceOverviewWorkspacesEmblemEffectClass))

typedef struct _XfceOverviewWorkspacesEmblemEffect			XfceOverviewWorkspacesEmblemEffect;
typedef struct _XfceOverviewWorkspacesEmblemEffectClass		XfceOverviewWorkspacesEmblemEffectClass;
typedef struct _XfceOverviewWorkspacesEmblemEffectPrivate		XfceOverviewWorkspacesEmblemEffectPrivate;

struct _XfceOverviewWorkspacesEmblemEffect
{
	/*< private >*/
	/* Parent instance */
	ClutterEffect						parent_instance;

	/* Private structure */
	XfceOverviewWorkspacesEmblemEffectPrivate		*priv;
};

struct _XfceOverviewWorkspacesEmblemEffectClass
{
	/*< private >*/
	/* Parent class */
	ClutterEffectClass					parent_class;
};

/* Public API */
GType xfce_overview_workspaces_emblem_effect_get_type(void) G_GNUC_CONST;

ClutterEffect* xfce_overview_workspaces_emblem_effect_new(void);

const gchar* xfce_overview_workspaces_emblem_effect_get_icon_name(XfceOverviewWorkspacesEmblemEffect *self);
void xfce_overview_workspaces_emblem_effect_set_icon_name(XfceOverviewWorkspacesEmblemEffect *self, const gchar *inIconName);

gint xfce_overview_workspaces_emblem_effect_get_icon_size(XfceOverviewWorkspacesEmblemEffect *self);
void xfce_overview_workspaces_emblem_effect_set_icon_size(XfceOverviewWorkspacesEmblemEffect *self, const gint inSize);

gfloat xfce_overview_workspaces_emblem_effect_get_padding(XfceOverviewWorkspacesEmblemEffect *self);
void xfce_overview_workspaces_emblem_effect_set_padding(XfceOverviewWorkspacesEmblemEffect *self, const gfloat inPadding);

gfloat xfce_overview_workspaces_emblem_effect_get_x_align(XfceOverviewWorkspacesEmblemEffect *self);
void xfce_overview_workspaces_emblem_effect_set_x_align(XfceOverviewWorkspacesEmblemEffect *self, const gfloat inAlign);

gfloat xfce_overview_workspaces_emblem_effect_get_y_align(XfceOverviewWorkspacesEmblemEffect *self);
void xfce_overview_workspaces_emblem_effect_set_y_align(XfceOverviewWorkspacesEmblemEffect *self, const gfloat inAlign);

XfceOverviewWorkspacesAnchorPoint xfce_overview_workspaces_emblem_effect_get_anchor_point(XfceOverviewWorkspacesEmblemEffect *self);
void xfce_overview_workspaces_emblem_effect_set_anchor_point(XfceOverviewWorkspacesEmblemEffect *self, const XfceOverviewWorkspacesAnchorPoint inAnchorPoint);

G_END_DECLS

#endif	/* __LIBXFCE_OVERVIEW_WORKSPACES_EMBLEM_EFFECT__ */
