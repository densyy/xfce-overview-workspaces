/*
 * plugins-manager: Single-instance managing plugins
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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_PLUGINS_MANAGER__
#define __LIBXFCE_OVERVIEW_WORKSPACES_PLUGINS_MANAGER__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <glib-object.h>

G_BEGIN_DECLS

#define XFCE_OVERVIEW_WORKSPACES_TYPE_PLUGINS_MANAGER			(xfce_overview_workspaces_plugins_manager_get_type())
#define XFCE_OVERVIEW_WORKSPACES_PLUGINS_MANAGER(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_PLUGINS_MANAGER, XfceOverviewWorkspacesPluginsManager))
#define XFCE_OVERVIEW_WORKSPACES_IS_PLUGINS_MANAGER(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_PLUGINS_MANAGER))
#define XFCE_OVERVIEW_WORKSPACES_PLUGINS_MANAGER_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_PLUGINS_MANAGER, XfceOverviewWorkspacesPluginsManagerClass))
#define XFCE_OVERVIEW_WORKSPACES_IS_PLUGINS_MANAGER_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_PLUGINS_MANAGER))
#define XFCE_OVERVIEW_WORKSPACES_PLUGINS_MANAGER_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_PLUGINS_MANAGER, XfceOverviewWorkspacesPluginsManagerClass))

typedef struct _XfceOverviewWorkspacesPluginsManager			XfceOverviewWorkspacesPluginsManager;
typedef struct _XfceOverviewWorkspacesPluginsManagerClass		XfceOverviewWorkspacesPluginsManagerClass;
typedef struct _XfceOverviewWorkspacesPluginsManagerPrivate	XfceOverviewWorkspacesPluginsManagerPrivate;

/**
 * XfceOverviewWorkspacesPluginsManager:
 *
 * The #XfceOverviewWorkspacesPluginsManager structure contains only private data and
 * should be accessed using the provided API
 */
struct _XfceOverviewWorkspacesPluginsManager
{
	/*< private >*/
	/* Parent instance */
	GObject								parent_instance;

	/* Private structure */
	XfceOverviewWorkspacesPluginsManagerPrivate	*priv;
};

/**
 * XfceOverviewWorkspacesPluginsManagerClass:
 *
 * The #XfceOverviewWorkspacesPluginsManagerClass structure contains only private data
 */
struct _XfceOverviewWorkspacesPluginsManagerClass
{
	/*< private >*/
	/* Parent class */
	GObjectClass						parent_class;
};

/* Public API */
GType xfce_overview_workspaces_plugins_manager_get_type(void) G_GNUC_CONST;

gboolean xfce_overview_workspaces_plugins_manager_setup(XfceOverviewWorkspacesPluginsManager *self);

G_END_DECLS

#endif	/* __LIBXFCE_OVERVIEW_WORKSPACES_PLUGINS_MANAGER__ */
