/*
 * clock-view: A view showing a clock
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

#ifndef __XFCE_OVERVIEW_WORKSPACES_CLOCK_VIEW__
#define __XFCE_OVERVIEW_WORKSPACES_CLOCK_VIEW__

#include <libxfce-overview-workspaces/libxfce-overview-workspaces.h>

G_BEGIN_DECLS

#define XFCE_OVERVIEW_WORKSPACES_TYPE_CLOCK_VIEW				(xfce_overview_workspaces_clock_view_get_type())
#define XFCE_OVERVIEW_WORKSPACES_CLOCK_VIEW(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_CLOCK_VIEW, XfceOverviewWorkspacesClockView))
#define XFCE_OVERVIEW_WORKSPACES_IS_CLOCK_VIEW(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_CLOCK_VIEW))
#define XFCE_OVERVIEW_WORKSPACES_CLOCK_VIEW_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_CLOCK_VIEW, XfceOverviewWorkspacesClockViewClass))
#define XFCE_OVERVIEW_WORKSPACES_IS_CLOCK_VIEW_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_CLOCK_VIEW))
#define XFCE_OVERVIEW_WORKSPACES_CLOCK_VIEW_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_CLOCK_VIEW, XfceOverviewWorkspacesClockViewClass))

typedef struct _XfceOverviewWorkspacesClockView			XfceOverviewWorkspacesClockView; 
typedef struct _XfceOverviewWorkspacesClockViewPrivate		XfceOverviewWorkspacesClockViewPrivate;
typedef struct _XfceOverviewWorkspacesClockViewClass		XfceOverviewWorkspacesClockViewClass;

struct _XfceOverviewWorkspacesClockView
{
	/* Parent instance */
	XfceOverviewWorkspacesView						parent_instance;

	/* Private structure */
	XfceOverviewWorkspacesClockViewPrivate			*priv;
};

struct _XfceOverviewWorkspacesClockViewClass
{
	/*< private >*/
	/* Parent class */
	XfceOverviewWorkspacesViewClass				parent_class;
};

/* Public API */
GType xfce_overview_workspaces_clock_view_get_type(void) G_GNUC_CONST;

XFCE_OVERVIEW_WORKSPACES_DECLARE_PLUGIN_TYPE(xfce_overview_workspaces_clock_view);

G_END_DECLS

#endif
