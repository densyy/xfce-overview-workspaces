/*
 * general: General settings of application
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

#ifndef __XFCE_OVERVIEW_WORKSPACES_SETTINGS_GENERAL__
#define __XFCE_OVERVIEW_WORKSPACES_SETTINGS_GENERAL__

#include <gtk/gtkx.h>

#include "settings.h"

G_BEGIN_DECLS

#define XFCE_OVERVIEW_WORKSPACES_TYPE_SETTINGS_GENERAL				(xfce_overview_workspaces_settings_general_get_type())
#define XFCE_OVERVIEW_WORKSPACES_SETTINGS_GENERAL(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_SETTINGS_GENERAL, XfceOverviewWorkspacesSettingsGeneral))
#define XFCE_OVERVIEW_WORKSPACES_IS_SETTINGS_GENERAL(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_SETTINGS_GENERAL))
#define XFCE_OVERVIEW_WORKSPACES_SETTINGS_GENERAL_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_SETTINGS_GENERAL, XfceOverviewWorkspacesSettingsGeneralClass))
#define XFCE_OVERVIEW_WORKSPACES_IS_SETTINGS_GENERAL_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_SETTINGS_GENERAL))
#define XFCE_OVERVIEW_WORKSPACES_SETTINGS_GENERAL_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_SETTINGS_GENERAL, XfceOverviewWorkspacesSettingsGeneralClass))

typedef struct _XfceOverviewWorkspacesSettingsGeneral				XfceOverviewWorkspacesSettingsGeneral;
typedef struct _XfceOverviewWorkspacesSettingsGeneralClass			XfceOverviewWorkspacesSettingsGeneralClass;
typedef struct _XfceOverviewWorkspacesSettingsGeneralPrivate		XfceOverviewWorkspacesSettingsGeneralPrivate;

struct _XfceOverviewWorkspacesSettingsGeneral
{
	/* Parent instance */
	GObject								parent_instance;

	/* Private structure */
	XfceOverviewWorkspacesSettingsGeneralPrivate	*priv;
};

struct _XfceOverviewWorkspacesSettingsGeneralClass
{
	/*< private >*/
	/* Parent class */
	GObjectClass						parent_class;

	/*< public >*/
	/* Virtual functions */
};

/* Public API */
GType xfce_overview_workspaces_settings_general_get_type(void) G_GNUC_CONST;

XfceOverviewWorkspacesSettingsGeneral* xfce_overview_workspaces_settings_general_new(XfceOverviewWorkspacesSettingsApp *inApp);

G_END_DECLS

#endif	/* __XFCE_OVERVIEW_WORKSPACES_SETTINGS_GENERAL__ */
