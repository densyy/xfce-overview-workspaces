/*
 * xfconf-settings: A class storing settings in Xfconf permanently
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

#ifndef __XFCE_OVERVIEW_WORKSPACES_XFCONF_XFCONF_SETTINGS__
#define __XFCE_OVERVIEW_WORKSPACES_XFCONF_XFCONF_SETTINGS__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <glib-object.h>

#include <libxfce-overview-workspaces/settings.h>

G_BEGIN_DECLS

#define XFCE_OVERVIEW_WORKSPACES_TYPE_XFCONF_SETTINGS				(xfce_overview_workspaces_xfconf_settings_get_type())
#define XFCE_OVERVIEW_WORKSPACES_XFCONF_SETTINGS(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_XFCONF_SETTINGS, XfceOverviewWorkspacesXfconfSettings))
#define XFCE_OVERVIEW_WORKSPACES_IS_XFCONF_SETTINGS(obj)				(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_XFCONF_SETTINGS))
#define XFCE_OVERVIEW_WORKSPACES_XFCONF_SETTINGS_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_XFCONF_SETTINGS, XfceOverviewWorkspacesXfconfSettingsClass))
#define XFCE_OVERVIEW_WORKSPACES_IS_XFCONF_SETTINGS_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_XFCONF_SETTINGS))
#define XFCE_OVERVIEW_WORKSPACES_XFCONF_SETTINGS_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_XFCONF_SETTINGS, XfceOverviewWorkspacesXfconfSettingsClass))

typedef struct _XfceOverviewWorkspacesXfconfSettings				XfceOverviewWorkspacesXfconfSettings;
typedef struct _XfceOverviewWorkspacesXfconfSettingsClass			XfceOverviewWorkspacesXfconfSettingsClass;
typedef struct _XfceOverviewWorkspacesXfconfSettingsPrivate		XfceOverviewWorkspacesXfconfSettingsPrivate;

/**
 * XfceOverviewWorkspacesXfconfSettings:
 *
 * The #XfceOverviewWorkspacesXfconfSettings structure contains only private data and
 * should be accessed using the provided API
 */
struct _XfceOverviewWorkspacesXfconfSettings
{
	/*< private >*/
	/* Parent instance */
	XfceOverviewWorkspacesSettings							parent_instance;

	/* Private structure */
	XfceOverviewWorkspacesXfconfSettingsPrivate			*priv;
};

/**
 * XfceOverviewWorkspacesXfconfSettingsClass:
 *
 * The #XfceOverviewWorkspacesXfconfSettingsClass class structure contains only private data
 */
struct _XfceOverviewWorkspacesXfconfSettingsClass
{
	/*< private >*/
	/* Parent class */
	XfceOverviewWorkspacesSettingsClass					parent_class;

	/*< public >*/
	/* Virtual functions */
};

/* Public API */
GType xfce_overview_workspaces_xfconf_settings_get_type(void) G_GNUC_CONST;

G_END_DECLS

#endif	/* __LIBXFCE_OVERVIEW_WORKSPACES_XFCONF_SETTINGS__ */
