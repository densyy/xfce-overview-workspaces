/*
 * desktop-app-info-action: An application action defined at desktop entry
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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_DESKTOP_APP_INFO_ACTION__
#define __LIBXFCE_OVERVIEW_WORKSPACES_DESKTOP_APP_INFO_ACTION__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <garcon/garcon.h>

G_BEGIN_DECLS

#define XFCE_OVERVIEW_WORKSPACES_TYPE_DESKTOP_APP_INFO_ACTION			(xfce_overview_workspaces_desktop_app_info_action_get_type())
#define XFCE_OVERVIEW_WORKSPACES_DESKTOP_APP_INFO_ACTION(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_DESKTOP_APP_INFO_ACTION, XfceOverviewWorkspacesDesktopAppInfoAction))
#define XFCE_OVERVIEW_WORKSPACES_IS_DESKTOP_APP_INFO_ACTION(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_DESKTOP_APP_INFO_ACTION))
#define XFCE_OVERVIEW_WORKSPACES_DESKTOP_APP_INFO_ACTION_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_DESKTOP_APP_INFO_ACTION, XfceOverviewWorkspacesDesktopAppInfoActionClass))
#define XFCE_OVERVIEW_WORKSPACES_IS_DESKTOP_APP_INFO_ACTION_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_DESKTOP_APP_INFO_ACTION))
#define XFCE_OVERVIEW_WORKSPACES_DESKTOP_APP_INFO_ACTION_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_DESKTOP_APP_INFO_ACTION, XfceOverviewWorkspacesDesktopAppInfoActionClass))

typedef struct _XfceOverviewWorkspacesDesktopAppInfoAction				XfceOverviewWorkspacesDesktopAppInfoAction;
typedef struct _XfceOverviewWorkspacesDesktopAppInfoActionClass		XfceOverviewWorkspacesDesktopAppInfoActionClass;
typedef struct _XfceOverviewWorkspacesDesktopAppInfoActionPrivate		XfceOverviewWorkspacesDesktopAppInfoActionPrivate;

struct _XfceOverviewWorkspacesDesktopAppInfoAction
{
	/*< private >*/
	/* Parent instance */
	GObject									parent_instance;

	/* Private structure */
	XfceOverviewWorkspacesDesktopAppInfoActionPrivate	*priv;
};

struct _XfceOverviewWorkspacesDesktopAppInfoActionClass
{
	/*< private >*/
	/* Parent class */
	GObjectClass							parent_class;

	/*< public >*/
	/* Virtual functions */
	void (*activate)(XfceOverviewWorkspacesDesktopAppInfoAction *self);
};

/* Public API */
GType xfce_overview_workspaces_desktop_app_info_action_get_type(void) G_GNUC_CONST;

const gchar* xfce_overview_workspaces_desktop_app_info_action_get_name(XfceOverviewWorkspacesDesktopAppInfoAction *self);
void xfce_overview_workspaces_desktop_app_info_action_set_name(XfceOverviewWorkspacesDesktopAppInfoAction *self,
													const gchar *inName);

const gchar* xfce_overview_workspaces_desktop_app_info_action_get_icon_name(XfceOverviewWorkspacesDesktopAppInfoAction *self);
void xfce_overview_workspaces_desktop_app_info_action_set_icon_name(XfceOverviewWorkspacesDesktopAppInfoAction *self,
														const gchar *inIconName);

const gchar* xfce_overview_workspaces_desktop_app_info_action_get_command(XfceOverviewWorkspacesDesktopAppInfoAction *self);
void xfce_overview_workspaces_desktop_app_info_action_set_command(XfceOverviewWorkspacesDesktopAppInfoAction *self,
														const gchar *inCommand);

G_END_DECLS

#endif	/* __LIBXFCE_OVERVIEW_WORKSPACES_DESKTOP_APP_INFO_ACTION__ */
