/*
 * clock-view-settings: Shared object instance holding settings for plugin
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

#ifndef __XFCE_OVERVIEW_WORKSPACES_CLOCK_VIEW_SETTINGS__
#define __XFCE_OVERVIEW_WORKSPACES_CLOCK_VIEW_SETTINGS__

#include <libxfce-overview-workspaces/libxfce-overview-workspaces.h>
#include <glib-object.h>
#include <clutter/clutter.h>

G_BEGIN_DECLS

#define XFCE_OVERVIEW_WORKSPACES_TYPE_CLOCK_VIEW_SETTINGS				(xfce_overview_workspaces_clock_view_settings_get_type())
#define XFCE_OVERVIEW_WORKSPACES_CLOCK_VIEW_SETTINGS(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_CLOCK_VIEW_SETTINGS, XfceOverviewWorkspacesClockViewSettings))
#define XFCE_OVERVIEW_WORKSPACES_IS_CLOCK_VIEW_SETTINGS(obj)				(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_CLOCK_VIEW_SETTINGS))
#define XFCE_OVERVIEW_WORKSPACES_CLOCK_VIEW_SETTINGS_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_CLOCK_VIEW_SETTINGS, XfceOverviewWorkspacesClockViewSettingsClass))
#define XFCE_OVERVIEW_WORKSPACES_IS_CLOCK_VIEW_SETTINGS_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_CLOCK_VIEW_SETTINGS))
#define XFCE_OVERVIEW_WORKSPACES_CLOCK_VIEW_SETTINGS_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_CLOCK_VIEW_SETTINGS, XfceOverviewWorkspacesClockViewSettingsClass))

typedef struct _XfceOverviewWorkspacesClockViewSettings				XfceOverviewWorkspacesClockViewSettings; 
typedef struct _XfceOverviewWorkspacesClockViewSettingsPrivate			XfceOverviewWorkspacesClockViewSettingsPrivate;
typedef struct _XfceOverviewWorkspacesClockViewSettingsClass			XfceOverviewWorkspacesClockViewSettingsClass;

struct _XfceOverviewWorkspacesClockViewSettings
{
	/* Parent instance */
	XfceOverviewWorkspacesPluginSettings				parent_instance;

	/* Private structure */
	XfceOverviewWorkspacesClockViewSettingsPrivate		*priv;
};

struct _XfceOverviewWorkspacesClockViewSettingsClass
{
	/*< private >*/
	/* Parent class */
	XfceOverviewWorkspacesPluginSettingsClass			parent_class;
};

/* Public API */
GType xfce_overview_workspaces_clock_view_settings_get_type(void) G_GNUC_CONST;

XfceOverviewWorkspacesClockViewSettings* xfce_overview_workspaces_clock_view_settings_new(void);

const ClutterColor* xfce_overview_workspaces_clock_view_settings_get_hour_color(XfceOverviewWorkspacesClockViewSettings *self);
void xfce_overview_workspaces_clock_view_settings_set_hour_color(XfceOverviewWorkspacesClockViewSettings *self, const ClutterColor *inColor);

const ClutterColor* xfce_overview_workspaces_clock_view_settings_get_minute_color(XfceOverviewWorkspacesClockViewSettings *self);
void xfce_overview_workspaces_clock_view_settings_set_minute_color(XfceOverviewWorkspacesClockViewSettings *self, const ClutterColor *inColor);

const ClutterColor* xfce_overview_workspaces_clock_view_settings_get_second_color(XfceOverviewWorkspacesClockViewSettings *self);
void xfce_overview_workspaces_clock_view_settings_set_second_color(XfceOverviewWorkspacesClockViewSettings *self, const ClutterColor *inColor);

const ClutterColor* xfce_overview_workspaces_clock_view_settings_get_background_color(XfceOverviewWorkspacesClockViewSettings *self);
void xfce_overview_workspaces_clock_view_settings_set_background_color(XfceOverviewWorkspacesClockViewSettings *self, const ClutterColor *inColor);

XFCE_OVERVIEW_WORKSPACES_DECLARE_PLUGIN_TYPE(xfce_overview_workspaces_clock_view_settings);

G_END_DECLS

#endif
