/*
 * desktop-app-info: A GDesktopAppInfo like object for garcon menu
 *                   items implementing and supporting GAppInfo
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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_DESKTOP_APP_INFO__
#define __LIBXFCE_OVERVIEW_WORKSPACES_DESKTOP_APP_INFO__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <libxfce-overview-workspaces/desktop-app-info-action.h>
#include <garcon/garcon.h>

G_BEGIN_DECLS

#define XFCE_OVERVIEW_WORKSPACES_TYPE_DESKTOP_APP_INFO				(xfce_overview_workspaces_desktop_app_info_get_type())
#define XFCE_OVERVIEW_WORKSPACES_DESKTOP_APP_INFO(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_DESKTOP_APP_INFO, XfceOverviewWorkspacesDesktopAppInfo))
#define XFCE_OVERVIEW_WORKSPACES_IS_DESKTOP_APP_INFO(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_DESKTOP_APP_INFO))
#define XFCE_OVERVIEW_WORKSPACES_DESKTOP_APP_INFO_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_DESKTOP_APP_INFO, XfceOverviewWorkspacesDesktopAppInfoClass))
#define XFCE_OVERVIEW_WORKSPACES_IS_DESKTOP_APP_INFO_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_DESKTOP_APP_INFO))
#define XFCE_OVERVIEW_WORKSPACES_DESKTOP_APP_INFO_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_DESKTOP_APP_INFO, XfceOverviewWorkspacesDesktopAppInfoClass))

typedef struct _XfceOverviewWorkspacesDesktopAppInfo				XfceOverviewWorkspacesDesktopAppInfo;
typedef struct _XfceOverviewWorkspacesDesktopAppInfoClass			XfceOverviewWorkspacesDesktopAppInfoClass;
typedef struct _XfceOverviewWorkspacesDesktopAppInfoPrivate		XfceOverviewWorkspacesDesktopAppInfoPrivate;

struct _XfceOverviewWorkspacesDesktopAppInfo
{
	/*< private >*/
	/* Parent instance */
	GObject								parent_instance;

	/* Private structure */
	XfceOverviewWorkspacesDesktopAppInfoPrivate	*priv;
};

struct _XfceOverviewWorkspacesDesktopAppInfoClass
{
	/*< private >*/
	/* Parent class */
	GObjectClass						parent_class;

	/*< public >*/
	/* Virtual functions */
	void (*changed)(XfceOverviewWorkspacesDesktopAppInfo *self);
};

/* Public API */
GType xfce_overview_workspaces_desktop_app_info_get_type(void) G_GNUC_CONST;

GAppInfo* xfce_overview_workspaces_desktop_app_info_new_from_desktop_id(const gchar *inDesktopID);
GAppInfo* xfce_overview_workspaces_desktop_app_info_new_from_path(const gchar *inPath);
GAppInfo* xfce_overview_workspaces_desktop_app_info_new_from_file(GFile *inFile);
GAppInfo* xfce_overview_workspaces_desktop_app_info_new_from_menu_item(GarconMenuItem *inMenuItem);

gboolean xfce_overview_workspaces_desktop_app_info_is_valid(XfceOverviewWorkspacesDesktopAppInfo *self);

GFile* xfce_overview_workspaces_desktop_app_info_get_file(XfceOverviewWorkspacesDesktopAppInfo *self);
gboolean xfce_overview_workspaces_desktop_app_info_reload(XfceOverviewWorkspacesDesktopAppInfo *self);

const GList* xfce_overview_workspaces_desktop_app_info_get_actions(XfceOverviewWorkspacesDesktopAppInfo *self);
gboolean xfce_overview_workspaces_desktop_app_info_launch_action(XfceOverviewWorkspacesDesktopAppInfo *self,
													XfceOverviewWorkspacesDesktopAppInfoAction *inAction,
													GAppLaunchContext *inContext,
													GError **outError);
gboolean xfce_overview_workspaces_desktop_app_info_launch_action_by_name(XfceOverviewWorkspacesDesktopAppInfo *self,
															const gchar *inActionName,
															GAppLaunchContext *inContext,
															GError **outError);

const GList* xfce_overview_workspaces_desktop_app_info_get_keywords(XfceOverviewWorkspacesDesktopAppInfo *self);

gboolean xfce_overview_workspaces_desktop_app_info_has_key(XfceOverviewWorkspacesDesktopAppInfo *self, const gchar *inKey);
gchar* xfce_overview_workspaces_desktop_app_info_get_string(XfceOverviewWorkspacesDesktopAppInfo *self, const gchar *inKey);
gchar* xfce_overview_workspaces_desktop_app_info_get_locale_string(XfceOverviewWorkspacesDesktopAppInfo *self, const gchar *inKey);
gchar** xfce_overview_workspaces_desktop_app_info_get_string_list(XfceOverviewWorkspacesDesktopAppInfo *self, const gchar *inKey);
gchar** xfce_overview_workspaces_desktop_app_info_get_locale_string_list(XfceOverviewWorkspacesDesktopAppInfo *self, const gchar *inKey);
gboolean xfce_overview_workspaces_desktop_app_info_get_bool(XfceOverviewWorkspacesDesktopAppInfo *self, const gchar *inKey);

G_END_DECLS

#endif	/* __LIBXFCE_OVERVIEW_WORKSPACES_DESKTOP_APP_INFO__ */
