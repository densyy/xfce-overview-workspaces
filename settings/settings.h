/*
 * settings: Settings of application
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

#ifndef __XFCE_OVERVIEW_WORKSPACES_SETTINGS_APP__
#define __XFCE_OVERVIEW_WORKSPACES_SETTINGS_APP__

#include <gtk/gtkx.h>
#include <libxfce-overview-workspaces/settings.h>

G_BEGIN_DECLS

#define XFCE_OVERVIEW_WORKSPACES_TYPE_SETTINGS_APP				(xfce_overview_workspaces_settings_app_get_type())
#define XFCE_OVERVIEW_WORKSPACES_SETTINGS_APP(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_SETTINGS_APP, XfceOverviewWorkspacesSettingsApp))
#define XFCE_OVERVIEW_WORKSPACES_IS_SETTINGS_APP(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_SETTINGS_APP))
#define XFCE_OVERVIEW_WORKSPACES_SETTINGS_APP_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_SETTINGS_APP, XfceOverviewWorkspacesSettingsAppClass))
#define XFCE_OVERVIEW_WORKSPACES_IS_SETTINGS_APP_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_SETTINGS_APP))
#define XFCE_OVERVIEW_WORKSPACES_SETTINGS_APP_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_SETTINGS_APP, XfceOverviewWorkspacesSettingsAppClass))

typedef struct _XfceOverviewWorkspacesSettingsApp				XfceOverviewWorkspacesSettingsApp;
typedef struct _XfceOverviewWorkspacesSettingsAppClass			XfceOverviewWorkspacesSettingsAppClass;
typedef struct _XfceOverviewWorkspacesSettingsAppPrivate		XfceOverviewWorkspacesSettingsAppPrivate;

struct _XfceOverviewWorkspacesSettingsApp
{
	/* Parent instance */
	GObject							parent_instance;

	/* Private structure */
	XfceOverviewWorkspacesSettingsAppPrivate	*priv;
};

struct _XfceOverviewWorkspacesSettingsAppClass
{
	/*< private >*/
	/* Parent class */
	GObjectClass					parent_class;

	/*< public >*/
	/* Virtual functions */
};

/* Public API */
GType xfce_overview_workspaces_settings_app_get_type(void) G_GNUC_CONST;

XfceOverviewWorkspacesSettingsApp* xfce_overview_workspaces_settings_app_new(void);

GtkWidget* xfce_overview_workspaces_settings_app_create_dialog(XfceOverviewWorkspacesSettingsApp *self);
GtkWidget* xfce_overview_workspaces_settings_app_create_plug(XfceOverviewWorkspacesSettingsApp *self, Window inSocketID);

GtkBuilder* xfce_overview_workspaces_settings_app_get_builder(XfceOverviewWorkspacesSettingsApp *self);
XfceOverviewWorkspacesSettings* xfce_overview_workspaces_settings_app_get_settings(XfceOverviewWorkspacesSettingsApp *self);

G_END_DECLS

#endif	/* __XFCE_OVERVIEW_WORKSPACES_SETTINGS_APP__ */
