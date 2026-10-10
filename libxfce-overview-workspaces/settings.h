/*
 * settings: A generic class containing the settings
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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_SETTINGS__
#define __LIBXFCE_OVERVIEW_WORKSPACES_SETTINGS__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <glib-object.h>

#include <libxfce-overview-workspaces/plugin.h>

G_BEGIN_DECLS

#define XFCE_OVERVIEW_WORKSPACES_TYPE_SETTINGS				(xfce_overview_workspaces_settings_get_type())
#define XFCE_OVERVIEW_WORKSPACES_SETTINGS(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_SETTINGS, XfceOverviewWorkspacesSettings))
#define XFCE_OVERVIEW_WORKSPACES_IS_SETTINGS(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_SETTINGS))
#define XFCE_OVERVIEW_WORKSPACES_SETTINGS_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_SETTINGS, XfceOverviewWorkspacesSettingsClass))
#define XFCE_OVERVIEW_WORKSPACES_IS_SETTINGS_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_SETTINGS))
#define XFCE_OVERVIEW_WORKSPACES_SETTINGS_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_SETTINGS, XfceOverviewWorkspacesSettingsClass))

typedef struct _XfceOverviewWorkspacesSettings				XfceOverviewWorkspacesSettings;
typedef struct _XfceOverviewWorkspacesSettingsClass		XfceOverviewWorkspacesSettingsClass;
typedef struct _XfceOverviewWorkspacesSettingsPrivate		XfceOverviewWorkspacesSettingsPrivate;

/**
 * XfceOverviewWorkspacesSettings:
 *
 * The #XfceOverviewWorkspacesSettings structure contains only private data and
 * should be accessed using the provided API
 */
struct _XfceOverviewWorkspacesSettings
{
	/*< private >*/
	/* Parent instance */
	GInitiallyUnowned					parent_instance;

	/* Private structure */
	XfceOverviewWorkspacesSettingsPrivate			*priv;
};

/**
 * XfceOverviewWorkspacesSettingsClass:
 * @settings_changed: The class closure for changed signal
 *
 * The #XfceOverviewWorkspacesSettingsClass class structure
 */
struct _XfceOverviewWorkspacesSettingsClass
{
	/*< private >*/
	/* Parent class */
	GInitiallyUnownedClass				parent_class;

	/*< public >*/
	/* Virtual functions */
	void (*changed)(XfceOverviewWorkspacesSettings *self, const gchar *inPluginID, GParamSpec *inParamSpec);

	void (*plugin_added)(XfceOverviewWorkspacesSettings *self, XfceOverviewWorkspacesPlugin *inPlugin);
	void (*plugin_removed)(XfceOverviewWorkspacesSettings *self, XfceOverviewWorkspacesPlugin *inPlugin);
};


/* Public API */
GType xfce_overview_workspaces_settings_get_type(void) G_GNUC_CONST;

void xfce_overview_workspaces_settings_add_plugin(XfceOverviewWorkspacesSettings *self, XfceOverviewWorkspacesPlugin *inPlugin);
void xfce_overview_workspaces_settings_remove_plugin(XfceOverviewWorkspacesSettings *self, XfceOverviewWorkspacesPlugin *inPlugin);
XfceOverviewWorkspacesPlugin* xfce_overview_workspaces_settings_lookup_plugin_by_id(XfceOverviewWorkspacesSettings *self, const gchar *inPluginID);

const gchar* xfce_overview_workspaces_settings_get_theme(XfceOverviewWorkspacesSettings *self);
void xfce_overview_workspaces_settings_set_theme(XfceOverviewWorkspacesSettings *self, const gchar *inTheme);

gboolean xfce_overview_workspaces_settings_get_allow_subwindows(XfceOverviewWorkspacesSettings *self);
void xfce_overview_workspaces_settings_set_allow_subwindows(XfceOverviewWorkspacesSettings *self, gboolean inAllowSubwindows);

const gchar** xfce_overview_workspaces_settings_get_enabled_plugins(XfceOverviewWorkspacesSettings *self);
void xfce_overview_workspaces_settings_set_enabled_plugins(XfceOverviewWorkspacesSettings *self, const gchar **inEnabledPlugins);

gboolean xfce_overview_workspaces_settings_get_enable_workaround_unmapped_window(XfceOverviewWorkspacesSettings *self);
void xfce_overview_workspaces_settings_set_enable_workaround_unmapped_window(XfceOverviewWorkspacesSettings *self, gboolean inEnableWorkaroundUnmappedWindow);

const gchar* xfce_overview_workspaces_settings_get_window_content_creation_priority(XfceOverviewWorkspacesSettings *self);
void xfce_overview_workspaces_settings_set_window_content_creation_priority(XfceOverviewWorkspacesSettings *self, const gchar *inWindowContentCreationPriority);

gboolean xfce_overview_workspaces_settings_get_enable_animations(XfceOverviewWorkspacesSettings *self);
void xfce_overview_workspaces_settings_set_enable_animations(XfceOverviewWorkspacesSettings *self, gboolean inEnableAnimations);

guint xfce_overview_workspaces_settings_get_notification_timeout(XfceOverviewWorkspacesSettings *self);
void xfce_overview_workspaces_settings_set_notification_timeout(XfceOverviewWorkspacesSettings *self, guint inNotificationTimeout);

const gchar* xfce_overview_workspaces_settings_get_switch_to_view_on_resume(XfceOverviewWorkspacesSettings *self);
void xfce_overview_workspaces_settings_set_switch_to_view_on_resume(XfceOverviewWorkspacesSettings *self, const gchar *inSwitchToViewOnResume);

gboolean xfce_overview_workspaces_settings_get_reselect_theme_focus_on_resume(XfceOverviewWorkspacesSettings *self);
void xfce_overview_workspaces_settings_set_reselect_theme_focus_on_resume(XfceOverviewWorkspacesSettings *self, gboolean inReselectThemeFocusOnResume);

gboolean xfce_overview_workspaces_settings_get_show_all_applications(XfceOverviewWorkspacesSettings *self);
void xfce_overview_workspaces_settings_set_show_all_applications(XfceOverviewWorkspacesSettings *self, gboolean inApplicationsViewShowAllApps);

gboolean xfce_overview_workspaces_settings_get_scroll_event_changes_workspace(XfceOverviewWorkspacesSettings *self);
void xfce_overview_workspaces_settings_set_scroll_event_changes_workspace(XfceOverviewWorkspacesSettings *self, gboolean inWindowsViewScrollEventChangesWorkspace);

const gchar** xfce_overview_workspaces_settings_get_binding_files(XfceOverviewWorkspacesSettings *self);

const gchar** xfce_overview_workspaces_settings_get_theme_search_paths(XfceOverviewWorkspacesSettings *self);

const gchar** xfce_overview_workspaces_settings_get_plugin_search_paths(XfceOverviewWorkspacesSettings *self);

const gchar* xfce_overview_workspaces_settings_get_config_path(XfceOverviewWorkspacesSettings *self);

const gchar* xfce_overview_workspaces_settings_get_data_path(XfceOverviewWorkspacesSettings *self);

G_END_DECLS

#endif	/* __LIBXFCE_OVERVIEW_WORKSPACES_SETTINGS__ */
