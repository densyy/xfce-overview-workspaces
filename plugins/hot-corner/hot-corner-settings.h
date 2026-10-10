/*
 * hot-corner-settings: Shared object instance holding settings for plugin
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

#ifndef __XFCE_OVERVIEW_WORKSPACES_HOT_CORNER_SETTINGS__
#define __XFCE_OVERVIEW_WORKSPACES_HOT_CORNER_SETTINGS__

#include <libxfce-overview-workspaces/libxfce-overview-workspaces.h>

G_BEGIN_DECLS

/* Public definitions */
typedef enum /*< prefix=XFCE_OVERVIEW_WORKSPACES_HOT_CORNER_SETTINGS_ACTIVATION_CORNER >*/
{
	XFCE_OVERVIEW_WORKSPACES_HOT_CORNER_SETTINGS_ACTIVATION_CORNER_TOP_LEFT=0,
	XFCE_OVERVIEW_WORKSPACES_HOT_CORNER_SETTINGS_ACTIVATION_CORNER_TOP_RIGHT,
	XFCE_OVERVIEW_WORKSPACES_HOT_CORNER_SETTINGS_ACTIVATION_CORNER_BOTTOM_LEFT,
	XFCE_OVERVIEW_WORKSPACES_HOT_CORNER_SETTINGS_ACTIVATION_CORNER_BOTTOM_RIGHT,
} XfceOverviewWorkspacesHotCornerSettingsActivationCorner;

GType xfce_overview_workspaces_hot_corner_settings_activation_corner_get_type(void) G_GNUC_CONST;
#define XFCE_OVERVIEW_WORKSPACES_TYPE_HOT_CORNER_SETTINGS_ACTIVATION_CORNER	(xfce_overview_workspaces_hot_corner_settings_activation_corner_get_type())


/* Object declaration */
#define XFCE_OVERVIEW_WORKSPACES_TYPE_HOT_CORNER_SETTINGS			(xfce_overview_workspaces_hot_corner_settings_get_type())
#define XFCE_OVERVIEW_WORKSPACES_HOT_CORNER_SETTINGS(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_HOT_CORNER_SETTINGS, XfceOverviewWorkspacesHotCornerSettings))
#define XFCE_OVERVIEW_WORKSPACES_IS_HOT_CORNER_SETTINGS(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_HOT_CORNER_SETTINGS))
#define XFCE_OVERVIEW_WORKSPACES_HOT_CORNER_SETTINGS_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_HOT_CORNER_SETTINGS, XfceOverviewWorkspacesHotCornerSettingsClass))
#define XFCE_OVERVIEW_WORKSPACES_IS_HOT_CORNER_SETTINGS_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_HOT_CORNER_SETTINGS))
#define XFCE_OVERVIEW_WORKSPACES_HOT_CORNER_SETTINGS_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_HOT_CORNER_SETTINGS, XfceOverviewWorkspacesHotCornerSettingsClass))

typedef struct _XfceOverviewWorkspacesHotCornerSettings			XfceOverviewWorkspacesHotCornerSettings; 
typedef struct _XfceOverviewWorkspacesHotCornerSettingsPrivate		XfceOverviewWorkspacesHotCornerSettingsPrivate;
typedef struct _XfceOverviewWorkspacesHotCornerSettingsClass		XfceOverviewWorkspacesHotCornerSettingsClass;

struct _XfceOverviewWorkspacesHotCornerSettings
{
	/* Parent instance */
	XfceOverviewWorkspacesPluginSettings				parent_instance;

	/* Private structure */
	XfceOverviewWorkspacesHotCornerSettingsPrivate		*priv;
};

struct _XfceOverviewWorkspacesHotCornerSettingsClass
{
	/*< private >*/
	/* Parent class */
	XfceOverviewWorkspacesPluginSettingsClass			parent_class;
};

/* Public API */
GType xfce_overview_workspaces_hot_corner_settings_get_type(void) G_GNUC_CONST;

XFCE_OVERVIEW_WORKSPACES_DECLARE_PLUGIN_TYPE(xfce_overview_workspaces_hot_corner_settings);

XfceOverviewWorkspacesHotCornerSettings* xfce_overview_workspaces_hot_corner_settings_new(void);

XfceOverviewWorkspacesHotCornerSettingsActivationCorner xfce_overview_workspaces_hot_corner_settings_get_activation_corner(XfceOverviewWorkspacesHotCornerSettings *self);
void xfce_overview_workspaces_hot_corner_settings_set_activation_corner(XfceOverviewWorkspacesHotCornerSettings *self, const XfceOverviewWorkspacesHotCornerSettingsActivationCorner inCorner);

gint xfce_overview_workspaces_hot_corner_settings_get_activation_radius(XfceOverviewWorkspacesHotCornerSettings *self);
void xfce_overview_workspaces_hot_corner_settings_set_activation_radius(XfceOverviewWorkspacesHotCornerSettings *self, gint inRadius);

gint64 xfce_overview_workspaces_hot_corner_settings_get_activation_duration(XfceOverviewWorkspacesHotCornerSettings *self);
void xfce_overview_workspaces_hot_corner_settings_set_activation_duration(XfceOverviewWorkspacesHotCornerSettings *self, gint64 inDuration);

gboolean xfce_overview_workspaces_hot_corner_settings_get_primary_monitor_only(XfceOverviewWorkspacesHotCornerSettings *self);
void xfce_overview_workspaces_hot_corner_settings_set_primary_monitor_only(XfceOverviewWorkspacesHotCornerSettings *self, gboolean inPrimaryOnly);

G_END_DECLS

#endif
