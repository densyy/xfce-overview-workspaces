/*
 * fill-box-layout: A box layout expanding actors in one direction
 *                  (fill to fit parent's size) and using natural
 *                  size in other direction
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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_FILL_BOX_LAYOUT__
#define __LIBXFCE_OVERVIEW_WORKSPACES_FILL_BOX_LAYOUT__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <clutter/clutter.h>

G_BEGIN_DECLS

#define XFCE_OVERVIEW_WORKSPACES_TYPE_FILL_BOX_LAYOUT			(xfce_overview_workspaces_fill_box_layout_get_type())
#define XFCE_OVERVIEW_WORKSPACES_FILL_BOX_LAYOUT(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_FILL_BOX_LAYOUT, XfceOverviewWorkspacesFillBoxLayout))
#define XFCE_OVERVIEW_WORKSPACES_IS_FILL_BOX_LAYOUT(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_FILL_BOX_LAYOUT))
#define XFCE_OVERVIEW_WORKSPACES_FILL_BOX_LAYOUT_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_FILL_BOX_LAYOUT, XfceOverviewWorkspacesFillBoxLayoutClass))
#define XFCE_OVERVIEW_WORKSPACES_IS_FILL_BOX_LAYOUT_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_FILL_BOX_LAYOUT))
#define XFCE_OVERVIEW_WORKSPACES_FILL_BOX_LAYOUT_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_FILL_BOX_LAYOUT, XfceOverviewWorkspacesFillBoxLayoutClass))

typedef struct _XfceOverviewWorkspacesFillBoxLayout			XfceOverviewWorkspacesFillBoxLayout;
typedef struct _XfceOverviewWorkspacesFillBoxLayoutPrivate		XfceOverviewWorkspacesFillBoxLayoutPrivate;
typedef struct _XfceOverviewWorkspacesFillBoxLayoutClass		XfceOverviewWorkspacesFillBoxLayoutClass;

struct _XfceOverviewWorkspacesFillBoxLayout
{
	/*< private >*/
	/* Parent instance */
	ClutterLayoutManager 			parent_instance;

	/* Private structure */
	XfceOverviewWorkspacesFillBoxLayoutPrivate	*priv;
};

struct _XfceOverviewWorkspacesFillBoxLayoutClass
{
	/*< private >*/
	/* Parent class */
	ClutterLayoutManagerClass		parent_class;
};

/* Public API */
GType xfce_overview_workspaces_fill_box_layout_get_type(void) G_GNUC_CONST;

ClutterLayoutManager* xfce_overview_workspaces_fill_box_layout_new(void);
ClutterLayoutManager* xfce_overview_workspaces_fill_box_layout_new_with_orientation(ClutterOrientation inOrientation);

ClutterOrientation xfce_overview_workspaces_fill_box_layout_get_orientation(XfceOverviewWorkspacesFillBoxLayout *self);
void xfce_overview_workspaces_fill_box_layout_set_orientation(XfceOverviewWorkspacesFillBoxLayout *self, ClutterOrientation inOrientation);

gfloat xfce_overview_workspaces_fill_box_layout_get_spacing(XfceOverviewWorkspacesFillBoxLayout *self);
void xfce_overview_workspaces_fill_box_layout_set_spacing(XfceOverviewWorkspacesFillBoxLayout *self, gfloat inSpacing);

gboolean xfce_overview_workspaces_fill_box_layout_get_homogeneous(XfceOverviewWorkspacesFillBoxLayout *self);
void xfce_overview_workspaces_fill_box_layout_set_homogeneous(XfceOverviewWorkspacesFillBoxLayout *self, gboolean inIsHomogeneous);

gboolean xfce_overview_workspaces_fill_box_layout_get_keep_aspect(XfceOverviewWorkspacesFillBoxLayout *self);
void xfce_overview_workspaces_fill_box_layout_set_keep_aspect(XfceOverviewWorkspacesFillBoxLayout *self, gboolean inKeepAspect);

G_END_DECLS

#endif	/* __LIBXFCE_OVERVIEW_WORKSPACES_FILL_BOX_LAYOUT__ */
