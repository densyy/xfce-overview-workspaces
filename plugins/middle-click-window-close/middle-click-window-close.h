/*
 * middle-click-window-close: Closes windows in window by middle-click
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

#ifndef __XFCE_OVERVIEW_WORKSPACES_MIDDLE_CLICK_WINDOW_CLOSE__
#define __XFCE_OVERVIEW_WORKSPACES_MIDDLE_CLICK_WINDOW_CLOSE__

#include <libxfce-overview-workspaces/libxfce-overview-workspaces.h>

G_BEGIN_DECLS

#define XFCE_OVERVIEW_WORKSPACES_TYPE_MIDDLE_CLICK_WINDOW_CLOSE				(xfce_overview_workspaces_middle_click_window_close_get_type())
#define XFCE_OVERVIEW_WORKSPACES_MIDDLE_CLICK_WINDOW_CLOSE(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_MIDDLE_CLICK_WINDOW_CLOSE, XfceOverviewWorkspacesMiddleClickWindowClose))
#define XFCE_OVERVIEW_WORKSPACES_IS_MIDDLE_CLICK_WINDOW_CLOSE(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_MIDDLE_CLICK_WINDOW_CLOSE))
#define XFCE_OVERVIEW_WORKSPACES_MIDDLE_CLICK_WINDOW_CLOSE_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_MIDDLE_CLICK_WINDOW_CLOSE, XfceOverviewWorkspacesMiddleClickWindowCloseClass))
#define XFCE_OVERVIEW_WORKSPACES_IS_MIDDLE_CLICK_WINDOW_CLOSE_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_MIDDLE_CLICK_WINDOW_CLOSE))
#define XFCE_OVERVIEW_WORKSPACES_MIDDLE_CLICK_WINDOW_CLOSE_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_MIDDLE_CLICK_WINDOW_CLOSE, XfceOverviewWorkspacesMiddleClickWindowCloseClass))

typedef struct _XfceOverviewWorkspacesMiddleClickWindowClose				XfceOverviewWorkspacesMiddleClickWindowClose; 
typedef struct _XfceOverviewWorkspacesMiddleClickWindowClosePrivate		XfceOverviewWorkspacesMiddleClickWindowClosePrivate;
typedef struct _XfceOverviewWorkspacesMiddleClickWindowCloseClass			XfceOverviewWorkspacesMiddleClickWindowCloseClass;

struct _XfceOverviewWorkspacesMiddleClickWindowClose
{
	/* Parent instance */
	GObject										parent_instance;

	/* Private structure */
	XfceOverviewWorkspacesMiddleClickWindowClosePrivate	*priv;
};

struct _XfceOverviewWorkspacesMiddleClickWindowCloseClass
{
	/*< private >*/
	/* Parent class */
	GObjectClass								parent_class;
};

/* Public API */
GType xfce_overview_workspaces_middle_click_window_close_get_type(void) G_GNUC_CONST;

XFCE_OVERVIEW_WORKSPACES_DECLARE_PLUGIN_TYPE(xfce_overview_workspaces_middle_click_window_close);

XfceOverviewWorkspacesMiddleClickWindowClose* xfce_overview_workspaces_middle_click_window_close_new(void);

G_END_DECLS

#endif
