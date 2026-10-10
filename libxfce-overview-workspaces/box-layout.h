/*
 * box-layout: A ClutterBoxLayout derived layout manager disregarding
 *             text direction and enforcing left-to-right layout in
 *             horizontal orientation
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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_BOX_LAYOUT__
#define __LIBXFCE_OVERVIEW_WORKSPACES_BOX_LAYOUT__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <clutter/clutter.h>

G_BEGIN_DECLS

#define XFCE_OVERVIEW_WORKSPACES_TYPE_BOX_LAYOUT				(xfce_overview_workspaces_box_layout_get_type())
#define XFCE_OVERVIEW_WORKSPACES_BOX_LAYOUT(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_BOX_LAYOUT, XfceOverviewWorkspacesBoxLayout))
#define XFCE_OVERVIEW_WORKSPACES_IS_BOX_LAYOUT(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_BOX_LAYOUT))
#define XFCE_OVERVIEW_WORKSPACES_BOX_LAYOUT_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_BOX_LAYOUT, XfceOverviewWorkspacesBoxLayoutClass))
#define XFCE_OVERVIEW_WORKSPACES_IS_BOX_LAYOUT_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_BOX_LAYOUT))
#define XFCE_OVERVIEW_WORKSPACES_BOX_LAYOUT_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_BOX_LAYOUT, XfceOverviewWorkspacesBoxLayoutClass))

typedef struct _XfceOverviewWorkspacesBoxLayout			XfceOverviewWorkspacesBoxLayout;
typedef struct _XfceOverviewWorkspacesBoxLayoutClass		XfceOverviewWorkspacesBoxLayoutClass;

/**
 * XfceOverviewWorkspacesBoxLayout:
 *
 * The #XfceOverviewWorkspacesBoxLayout structure contains only private data and
 * should be accessed using the provided API
 */
struct _XfceOverviewWorkspacesBoxLayout
{
	/*< private >*/
	/* Parent instance */
	ClutterBoxLayout 				parent_instance;
};

/**
 * XfceOverviewWorkspacesBoxLayoutClass:
 *
 * The #XfceOverviewWorkspacesBoxLayoutClass structure contains only private data
 */
struct _XfceOverviewWorkspacesBoxLayoutClass
{
	/*< private >*/
	/* Parent class */
	ClutterBoxLayoutClass			parent_class;
};

/* Public API */
GType xfce_overview_workspaces_box_layout_get_type(void) G_GNUC_CONST;

ClutterLayoutManager* xfce_overview_workspaces_box_layout_new(void);

G_END_DECLS

#endif	/* __LIBXFCE_OVERVIEW_WORKSPACES_BOX_LAYOUT__ */
