/*
 * hot-corner: Activates application when pointer is move to a corner
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

#ifndef __XFCE_OVERVIEW_WORKSPACES_HOT_CORNER__
#define __XFCE_OVERVIEW_WORKSPACES_HOT_CORNER__

#include <libxfce-overview-workspaces/libxfce-overview-workspaces.h>

G_BEGIN_DECLS

/* Public definitions */
typedef enum /*< prefix=XFCE_OVERVIEW_WORKSPACES_HOT_CORNER_ACTIVATION_CORNER >*/
{
	XFCE_OVERVIEW_WORKSPACES_HOT_CORNER_ACTIVATION_CORNER_TOP_LEFT=0,
	XFCE_OVERVIEW_WORKSPACES_HOT_CORNER_ACTIVATION_CORNER_TOP_RIGHT,
	XFCE_OVERVIEW_WORKSPACES_HOT_CORNER_ACTIVATION_CORNER_BOTTOM_LEFT,
	XFCE_OVERVIEW_WORKSPACES_HOT_CORNER_ACTIVATION_CORNER_BOTTOM_RIGHT,
} XfceOverviewWorkspacesHotCornerActivationCorner;

GType xfce_overview_workspaces_hot_corner_activation_corner_get_type(void) G_GNUC_CONST;
#define XFCE_OVERVIEW_WORKSPACES_TYPE_HOT_CORNER_ACTIVATION_CORNER	(xfce_overview_workspaces_hot_corner_activation_corner_get_type())


/* Object declaration */
#define XFCE_OVERVIEW_WORKSPACES_TYPE_HOT_CORNER				(xfce_overview_workspaces_hot_corner_get_type())
#define XFCE_OVERVIEW_WORKSPACES_HOT_CORNER(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_HOT_CORNER, XfceOverviewWorkspacesHotCorner))
#define XFCE_OVERVIEW_WORKSPACES_IS_HOT_CORNER(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_HOT_CORNER))
#define XFCE_OVERVIEW_WORKSPACES_HOT_CORNER_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_HOT_CORNER, XfceOverviewWorkspacesHotCornerClass))
#define XFCE_OVERVIEW_WORKSPACES_IS_HOT_CORNER_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_HOT_CORNER))
#define XFCE_OVERVIEW_WORKSPACES_HOT_CORNER_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_HOT_CORNER, XfceOverviewWorkspacesHotCornerClass))

typedef struct _XfceOverviewWorkspacesHotCorner			XfceOverviewWorkspacesHotCorner; 
typedef struct _XfceOverviewWorkspacesHotCornerPrivate		XfceOverviewWorkspacesHotCornerPrivate;
typedef struct _XfceOverviewWorkspacesHotCornerClass		XfceOverviewWorkspacesHotCornerClass;

struct _XfceOverviewWorkspacesHotCorner
{
	/* Parent instance */
	GObject						parent_instance;

	/* Private structure */
	XfceOverviewWorkspacesHotCornerPrivate	*priv;
};

struct _XfceOverviewWorkspacesHotCornerClass
{
	/*< private >*/
	/* Parent class */
	GObjectClass				parent_class;
};

/* Public API */
GType xfce_overview_workspaces_hot_corner_get_type(void) G_GNUC_CONST;

XFCE_OVERVIEW_WORKSPACES_DECLARE_PLUGIN_TYPE(xfce_overview_workspaces_hot_corner);

XfceOverviewWorkspacesHotCorner* xfce_overview_workspaces_hot_corner_new(void);

G_END_DECLS

#endif
