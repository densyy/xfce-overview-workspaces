/*
 * scaled-table-layout: Layouts children in a dynamic table grid
 *                      (rows and columns are inserted and deleted
 *                      automatically depending on the number of
 *                      child actors) and scaled to fit the allocation
 *                      of the actor holding all child actors.
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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_SCALED_TABLE_LAYOUT__
#define __LIBXFCE_OVERVIEW_WORKSPACES_SCALED_TABLE_LAYOUT__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <clutter/clutter.h>

G_BEGIN_DECLS

#define XFCE_OVERVIEW_WORKSPACES_TYPE_SCALED_TABLE_LAYOUT			(xfce_overview_workspaces_scaled_table_layout_get_type())
#define XFCE_OVERVIEW_WORKSPACES_SCALED_TABLE_LAYOUT(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_SCALED_TABLE_LAYOUT, XfceOverviewWorkspacesScaledTableLayout))
#define XFCE_OVERVIEW_WORKSPACES_IS_SCALED_TABLE_LAYOUT(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_SCALED_TABLE_LAYOUT))
#define XFCE_OVERVIEW_WORKSPACES_SCALED_TABLE_LAYOUT_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_SCALED_TABLE_LAYOUT, XfceOverviewWorkspacesScaledTableLayoutClass))
#define XFCE_OVERVIEW_WORKSPACES_IS_SCALED_TABLE_LAYOUT_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_SCALED_TABLE_LAYOUT))
#define XFCE_OVERVIEW_WORKSPACES_SCALED_TABLE_LAYOUT_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_SCALED_TABLE_LAYOUT, XfceOverviewWorkspacesScaledTableLayoutClass))

typedef struct _XfceOverviewWorkspacesScaledTableLayout			XfceOverviewWorkspacesScaledTableLayout;
typedef struct _XfceOverviewWorkspacesScaledTableLayoutPrivate		XfceOverviewWorkspacesScaledTableLayoutPrivate;
typedef struct _XfceOverviewWorkspacesScaledTableLayoutClass		XfceOverviewWorkspacesScaledTableLayoutClass;

struct _XfceOverviewWorkspacesScaledTableLayout
{
	/*< private >*/
	/* Parent instance */
	ClutterLayoutManager 				parent_instance;

	/* Private structure */
	XfceOverviewWorkspacesScaledTableLayoutPrivate	*priv;
};

struct _XfceOverviewWorkspacesScaledTableLayoutClass
{
	/*< private >*/
	/* Parent class */
	ClutterLayoutManagerClass			parent_class;
};

/* Public API */
GType xfce_overview_workspaces_scaled_table_layout_get_type(void) G_GNUC_CONST;

ClutterLayoutManager* xfce_overview_workspaces_scaled_table_layout_new(void);

gint xfce_overview_workspaces_scaled_table_layout_get_number_children(XfceOverviewWorkspacesScaledTableLayout *self);
gint xfce_overview_workspaces_scaled_table_layout_get_rows(XfceOverviewWorkspacesScaledTableLayout *self);
gint xfce_overview_workspaces_scaled_table_layout_get_columns(XfceOverviewWorkspacesScaledTableLayout *self);

gboolean xfce_overview_workspaces_scaled_table_layout_get_relative_scale(XfceOverviewWorkspacesScaledTableLayout *self);
void xfce_overview_workspaces_scaled_table_layout_set_relative_scale(XfceOverviewWorkspacesScaledTableLayout *self, gboolean inScaling);

gboolean xfce_overview_workspaces_scaled_table_layout_get_prevent_upscaling(XfceOverviewWorkspacesScaledTableLayout *self);
void xfce_overview_workspaces_scaled_table_layout_set_prevent_upscaling(XfceOverviewWorkspacesScaledTableLayout *self, gboolean inPreventUpscaling);

void xfce_overview_workspaces_scaled_table_layout_set_spacing(XfceOverviewWorkspacesScaledTableLayout *self, gfloat inSpacing);

gfloat xfce_overview_workspaces_scaled_table_layout_get_row_spacing(XfceOverviewWorkspacesScaledTableLayout *self);
void xfce_overview_workspaces_scaled_table_layout_set_row_spacing(XfceOverviewWorkspacesScaledTableLayout *self, gfloat inSpacing);

gfloat xfce_overview_workspaces_scaled_table_layout_get_column_spacing(XfceOverviewWorkspacesScaledTableLayout *self);
void xfce_overview_workspaces_scaled_table_layout_set_column_spacing(XfceOverviewWorkspacesScaledTableLayout *self, gfloat inSpacing);

G_END_DECLS

#endif	/* __LIBXFCE_OVERVIEW_WORKSPACES_SCALED_TABLE_LAYOUT__ */
