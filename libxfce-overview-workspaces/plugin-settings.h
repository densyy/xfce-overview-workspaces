/*
 * plugin-settings: A generic class containing the settings of a plugin
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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_PLUGIN_SETTINGS__
#define __LIBXFCE_OVERVIEW_WORKSPACES_PLUGIN_SETTINGS__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <glib-object.h>

G_BEGIN_DECLS

#define XFCE_OVERVIEW_WORKSPACES_TYPE_PLUGIN_SETTINGS				(xfce_overview_workspaces_plugin_settings_get_type())
#define XFCE_OVERVIEW_WORKSPACES_PLUGIN_SETTINGS(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_PLUGIN_SETTINGS, XfceOverviewWorkspacesPluginSettings))
#define XFCE_OVERVIEW_WORKSPACES_IS_PLUGIN_SETTINGS(obj)				(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_PLUGIN_SETTINGS))
#define XFCE_OVERVIEW_WORKSPACES_PLUGIN_SETTINGS_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_PLUGIN_SETTINGS, XfceOverviewWorkspacesPluginSettingsClass))
#define XFCE_OVERVIEW_WORKSPACES_IS_PLUGIN_SETTINGS_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_PLUGIN_SETTINGS))
#define XFCE_OVERVIEW_WORKSPACES_PLUGIN_SETTINGS_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_PLUGIN_SETTINGS, XfceOverviewWorkspacesPluginSettingsClass))

typedef struct _XfceOverviewWorkspacesPluginSettings				XfceOverviewWorkspacesPluginSettings;
typedef struct _XfceOverviewWorkspacesPluginSettingsClass			XfceOverviewWorkspacesPluginSettingsClass;

/**
 * XfceOverviewWorkspacesPluginSettings:
 *
 * The #XfceOverviewWorkspacesPluginSettings structure contains only private data and
 * should be accessed using the provided API
 */
struct _XfceOverviewWorkspacesPluginSettings
{
	/*< private >*/
	/* Parent instance */
	GInitiallyUnowned					parent_instance;
};

/**
 * XfceOverviewWorkspacesPluginSettingsClass:
 * @settings_changed: The class closure for changed signal
 *
 * The #XfceOverviewWorkspacesPluginSettingsClass class structure
 */
struct _XfceOverviewWorkspacesPluginSettingsClass
{
	/*< private >*/
	/* Parent class */
	GInitiallyUnownedClass				parent_class;

	/*< public >*/
	/* Virtual functions */
	void (*changed)(XfceOverviewWorkspacesPluginSettings *self, GParamSpec *inParamSpec);
};


/* Public API */
GType xfce_overview_workspaces_plugin_settings_get_type(void) G_GNUC_CONST;

G_END_DECLS

#endif	/* __LIBXFCE_OVERVIEW_WORKSPACES_PLUGIN_SETTINGS__ */
