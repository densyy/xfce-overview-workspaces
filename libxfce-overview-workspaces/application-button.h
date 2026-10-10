/*
 * application-button: A button representing an application
 *                     (either by menu item or desktop file)
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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_APPLICATION_BUTTON__
#define __LIBXFCE_OVERVIEW_WORKSPACES_APPLICATION_BUTTON__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <garcon/garcon.h>

#include <libxfce-overview-workspaces/button.h>
#include <libxfce-overview-workspaces/desktop-app-info.h>
#include <libxfce-overview-workspaces/popup-menu.h>

G_BEGIN_DECLS

#define XFCE_OVERVIEW_WORKSPACES_TYPE_APPLICATION_BUTTON				(xfce_overview_workspaces_application_button_get_type())
#define XFCE_OVERVIEW_WORKSPACES_APPLICATION_BUTTON(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_APPLICATION_BUTTON, XfceOverviewWorkspacesApplicationButton))
#define XFCE_OVERVIEW_WORKSPACES_IS_APPLICATION_BUTTON(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_APPLICATION_BUTTON))
#define XFCE_OVERVIEW_WORKSPACES_APPLICATION_BUTTON_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_APPLICATION_BUTTON, XfceOverviewWorkspacesApplicationButtonClass))
#define XFCE_OVERVIEW_WORKSPACES_IS_APPLICATION_BUTTON_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_APPLICATION_BUTTON))
#define XFCE_OVERVIEW_WORKSPACES_APPLICATION_BUTTON_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_APPLICATION_BUTTON, XfceOverviewWorkspacesApplicationButtonClass))

typedef struct _XfceOverviewWorkspacesApplicationButton			XfceOverviewWorkspacesApplicationButton;
typedef struct _XfceOverviewWorkspacesApplicationButtonClass		XfceOverviewWorkspacesApplicationButtonClass;
typedef struct _XfceOverviewWorkspacesApplicationButtonPrivate		XfceOverviewWorkspacesApplicationButtonPrivate;

struct _XfceOverviewWorkspacesApplicationButton
{
	/*< private >*/
	/* Parent instance */
	XfceOverviewWorkspacesButton						parent_instance;

	/* Private structure */
	XfceOverviewWorkspacesApplicationButtonPrivate		*priv;
};

struct _XfceOverviewWorkspacesApplicationButtonClass
{
	/*< private >*/
	/* Parent class */
	XfceOverviewWorkspacesButtonClass					parent_class;

	/*< public >*/
	/* Virtual functions */
};

/* Public API */
GType xfce_overview_workspaces_application_button_get_type(void) G_GNUC_CONST;

ClutterActor* xfce_overview_workspaces_application_button_new(void);
ClutterActor* xfce_overview_workspaces_application_button_new_from_app_info(GAppInfo *inAppInfo);

GAppInfo* xfce_overview_workspaces_application_button_get_app_info(XfceOverviewWorkspacesApplicationButton *self);
void xfce_overview_workspaces_application_button_set_app_info(XfceOverviewWorkspacesApplicationButton *self,
													GAppInfo *inAppInfo);

gboolean xfce_overview_workspaces_application_button_get_show_description(XfceOverviewWorkspacesApplicationButton *self);
void xfce_overview_workspaces_application_button_set_show_description(XfceOverviewWorkspacesApplicationButton *self,
															gboolean inShowDescription);

const gchar* xfce_overview_workspaces_application_button_get_format_title_only(XfceOverviewWorkspacesApplicationButton *self);
void xfce_overview_workspaces_application_button_set_format_title_only(XfceOverviewWorkspacesApplicationButton *self,
															const gchar *inFormat);

const gchar* xfce_overview_workspaces_application_button_get_format_title_description(XfceOverviewWorkspacesApplicationButton *self);
void xfce_overview_workspaces_application_button_set_format_title_description(XfceOverviewWorkspacesApplicationButton *self,
																	const gchar *inFormat);

const gchar* xfce_overview_workspaces_application_button_get_display_name(XfceOverviewWorkspacesApplicationButton *self);
const gchar* xfce_overview_workspaces_application_button_get_icon_name(XfceOverviewWorkspacesApplicationButton *self);

gboolean xfce_overview_workspaces_application_button_execute(XfceOverviewWorkspacesApplicationButton *self, GAppLaunchContext *inContext);

guint xfce_overview_workspaces_application_button_add_popup_menu_items_for_windows(XfceOverviewWorkspacesApplicationButton *self,
																		XfceOverviewWorkspacesPopupMenu *inMenu);
guint xfce_overview_workspaces_application_button_add_popup_menu_items_for_actions(XfceOverviewWorkspacesApplicationButton *self,
																		XfceOverviewWorkspacesPopupMenu *inMenu);

G_END_DECLS

#endif	/* __LIBXFCE_OVERVIEW_WORKSPACES_APPLICATION_BUTTON__ */
