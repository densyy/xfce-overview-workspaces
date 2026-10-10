/*
 * dynamic-table-layout: Layouts children in a dynamic table grid
 *                       (rows and columns are inserted and deleted
 *                       automatically depending on the number of
 *                       child actors).
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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_DYNAMIC_TABLE_LAYOUT__
#define __LIBXFCE_OVERVIEW_WORKSPACES_DYNAMIC_TABLE_LAYOUT__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <clutter/clutter.h>

G_BEGIN_DECLS

#define XFCE_OVERVIEW_WORKSPACES_TYPE_DYNAMIC_TABLE_LAYOUT				(xfce_overview_workspaces_dynamic_table_layout_get_type())
#define XFCE_OVERVIEW_WORKSPACES_DYNAMIC_TABLE_LAYOUT(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_DYNAMIC_TABLE_LAYOUT, XfceOverviewWorkspacesDynamicTableLayout))
#define XFCE_OVERVIEW_WORKSPACES_IS_DYNAMIC_TABLE_LAYOUT(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_DYNAMIC_TABLE_LAYOUT))
#define XFCE_OVERVIEW_WORKSPACES_DYNAMIC_TABLE_LAYOUT_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_DYNAMIC_TABLE_LAYOUT, XfceOverviewWorkspacesDynamicTableLayoutClass))
#define XFCE_OVERVIEW_WORKSPACES_IS_DYNAMIC_TABLE_LAYOUT_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_DYNAMIC_TABLE_LAYOUT))
#define XFCE_OVERVIEW_WORKSPACES_DYNAMIC_TABLE_LAYOUT_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_DYNAMIC_TABLE_LAYOUT, XfceOverviewWorkspacesDynamicTableLayoutClass))

typedef struct _XfceOverviewWorkspacesDynamicTableLayout				XfceOverviewWorkspacesDynamicTableLayout;
typedef struct _XfceOverviewWorkspacesDynamicTableLayoutPrivate		XfceOverviewWorkspacesDynamicTableLayoutPrivate;
typedef struct _XfceOverviewWorkspacesDynamicTableLayoutClass			XfceOverviewWorkspacesDynamicTableLayoutClass;

struct _XfceOverviewWorkspacesDynamicTableLayout
{
	/*< private >*/
	/* Parent instance */
	ClutterLayoutManager 					parent_instance;

	/* Private structure */
	XfceOverviewWorkspacesDynamicTableLayoutPrivate	*priv;
};

struct _XfceOverviewWorkspacesDynamicTableLayoutClass
{
	/*< private >*/
	/* Parent class */
	ClutterLayoutManagerClass				parent_class;
};

/* Public API */
GType xfce_overview_workspaces_dynamic_table_layout_get_type(void) G_GNUC_CONST;

ClutterLayoutManager* xfce_overview_workspaces_dynamic_table_layout_new(void);

gint xfce_overview_workspaces_dynamic_table_layout_get_number_children(XfceOverviewWorkspacesDynamicTableLayout *self);
gint xfce_overview_workspaces_dynamic_table_layout_get_rows(XfceOverviewWorkspacesDynamicTableLayout *self);
gint xfce_overview_workspaces_dynamic_table_layout_get_columns(XfceOverviewWorkspacesDynamicTableLayout *self);

void xfce_overview_workspaces_dynamic_table_layout_set_spacing(XfceOverviewWorkspacesDynamicTableLayout *self, gfloat inSpacing);

gfloat xfce_overview_workspaces_dynamic_table_layout_get_row_spacing(XfceOverviewWorkspacesDynamicTableLayout *self);
void xfce_overview_workspaces_dynamic_table_layout_set_row_spacing(XfceOverviewWorkspacesDynamicTableLayout *self, gfloat inSpacing);

gfloat xfce_overview_workspaces_dynamic_table_layout_get_column_spacing(XfceOverviewWorkspacesDynamicTableLayout *self);
void xfce_overview_workspaces_dynamic_table_layout_set_column_spacing(XfceOverviewWorkspacesDynamicTableLayout *self, gfloat inSpacing);

gint xfce_overview_workspaces_dynamic_table_layout_get_fixed_columns(XfceOverviewWorkspacesDynamicTableLayout *self);
void xfce_overview_workspaces_dynamic_table_layout_set_fixed_columns(XfceOverviewWorkspacesDynamicTableLayout *self, gint inColumns);

G_END_DECLS

#endif	/* __LIBXFCE_OVERVIEW_WORKSPACES_DYNAMIC_TABLE_LAYOUT__ */
