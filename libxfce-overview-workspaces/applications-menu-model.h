/*
 * applications-menu-model: A list model containing menu items
 *                          of applications
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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_APPLICATIONS_MENU_MODEL__
#define __LIBXFCE_OVERVIEW_WORKSPACES_APPLICATIONS_MENU_MODEL__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <garcon/garcon.h>

#include <libxfce-overview-workspaces/model.h>

G_BEGIN_DECLS

#define XFCE_OVERVIEW_WORKSPACES_TYPE_APPLICATIONS_MENU_MODEL			(xfce_overview_workspaces_applications_menu_model_get_type())
#define XFCE_OVERVIEW_WORKSPACES_APPLICATIONS_MENU_MODEL(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_APPLICATIONS_MENU_MODEL, XfceOverviewWorkspacesApplicationsMenuModel))
#define XFCE_OVERVIEW_WORKSPACES_IS_APPLICATIONS_MENU_MODEL(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_APPLICATIONS_MENU_MODEL))
#define XFCE_OVERVIEW_WORKSPACES_APPLICATIONS_MENU_MODEL_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_APPLICATIONS_MENU_MODEL, XfceOverviewWorkspacesApplicationsMenuModelClass))
#define XFCE_OVERVIEW_WORKSPACES_IS_APPLICATIONS_MENU_MODEL_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_APPLICATIONS_MENU_MODEL))
#define XFCE_OVERVIEW_WORKSPACES_APPLICATIONS_MENU_MODEL_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_APPLICATIONS_MENU_MODEL, XfceOverviewWorkspacesApplicationsMenuModelClass))

typedef struct _XfceOverviewWorkspacesApplicationsMenuModel			XfceOverviewWorkspacesApplicationsMenuModel; 
typedef struct _XfceOverviewWorkspacesApplicationsMenuModelPrivate		XfceOverviewWorkspacesApplicationsMenuModelPrivate;
typedef struct _XfceOverviewWorkspacesApplicationsMenuModelClass		XfceOverviewWorkspacesApplicationsMenuModelClass;

struct _XfceOverviewWorkspacesApplicationsMenuModel
{
	/*< private >*/
	/* Parent instance */
	XfceOverviewWorkspacesModel							parent_instance;

	/* Private structure */
	XfceOverviewWorkspacesApplicationsMenuModelPrivate		*priv;
};

struct _XfceOverviewWorkspacesApplicationsMenuModelClass
{
	/*< private >*/
	/* Parent class */
	XfceOverviewWorkspacesModelClass						parent_class;

	/*< public >*/
	/* Virtual functions */
	void (*loaded)(XfceOverviewWorkspacesApplicationsMenuModel *self);
};

/* Public API */

/* Columns of model */
enum
{
	XFCE_OVERVIEW_WORKSPACES_APPLICATIONS_MENU_MODEL_COLUMN_SEQUENCE_ID,

	XFCE_OVERVIEW_WORKSPACES_APPLICATIONS_MENU_MODEL_COLUMN_MENU_ELEMENT,
	XFCE_OVERVIEW_WORKSPACES_APPLICATIONS_MENU_MODEL_COLUMN_PARENT_MENU,
	XFCE_OVERVIEW_WORKSPACES_APPLICATIONS_MENU_MODEL_COLUMN_SECTION,

	XFCE_OVERVIEW_WORKSPACES_APPLICATIONS_MENU_MODEL_COLUMN_TITLE,
	XFCE_OVERVIEW_WORKSPACES_APPLICATIONS_MENU_MODEL_COLUMN_DESCRIPTION,

	XFCE_OVERVIEW_WORKSPACES_APPLICATIONS_MENU_MODEL_COLUMN_LAST
};

GType xfce_overview_workspaces_applications_menu_model_get_type(void) G_GNUC_CONST;

XfceOverviewWorkspacesModel* xfce_overview_workspaces_applications_menu_model_new(void);

void xfce_overview_workspaces_applications_menu_model_get(XfceOverviewWorkspacesApplicationsMenuModel *self,
												XfceOverviewWorkspacesModelIter *inIter,
												...);

void xfce_overview_workspaces_applications_menu_model_filter_by_menu(XfceOverviewWorkspacesApplicationsMenuModel *self,
														GarconMenu *inMenu);
void xfce_overview_workspaces_applications_menu_model_filter_by_section(XfceOverviewWorkspacesApplicationsMenuModel *self,
															GarconMenu *inSection);

G_END_DECLS

#endif	/* __LIBXFCE_OVERVIEW_WORKSPACES_APPLICATIONS_MENU_MODEL__ */

