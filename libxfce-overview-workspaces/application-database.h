/*
 * application-database: A singelton managing desktop files and menus
 *                       for installed applications
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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_APPLICATION_DATABASE__
#define __LIBXFCE_OVERVIEW_WORKSPACES_APPLICATION_DATABASE__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <garcon/garcon.h>

G_BEGIN_DECLS

#define XFCE_OVERVIEW_WORKSPACES_TYPE_APPLICATION_DATABASE				(xfce_overview_workspaces_application_database_get_type())
#define XFCE_OVERVIEW_WORKSPACES_APPLICATION_DATABASE(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_APPLICATION_DATABASE, XfceOverviewWorkspacesApplicationDatabase))
#define XFCE_OVERVIEW_WORKSPACES_IS_APPLICATION_DATABASE(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_APPLICATION_DATABASE))
#define XFCE_OVERVIEW_WORKSPACES_APPLICATION_DATABASE_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_APPLICATION_DATABASE, XfceOverviewWorkspacesApplicationDatabaseClass))
#define XFCE_OVERVIEW_WORKSPACES_IS_APPLICATION_DATABASE_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_APPLICATION_DATABASE))
#define XFCE_OVERVIEW_WORKSPACES_APPLICATION_DATABASE_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_APPLICATION_DATABASE, XfceOverviewWorkspacesApplicationDatabaseClass))

typedef struct _XfceOverviewWorkspacesApplicationDatabase				XfceOverviewWorkspacesApplicationDatabase;
typedef struct _XfceOverviewWorkspacesApplicationDatabaseClass			XfceOverviewWorkspacesApplicationDatabaseClass;
typedef struct _XfceOverviewWorkspacesApplicationDatabasePrivate		XfceOverviewWorkspacesApplicationDatabasePrivate;

struct _XfceOverviewWorkspacesApplicationDatabase
{
	/*< private >*/
	/* Parent instance */
	GObject									parent_instance;

	/* Private structure */
	XfceOverviewWorkspacesApplicationDatabasePrivate	*priv;
};

struct _XfceOverviewWorkspacesApplicationDatabaseClass
{
	/*< private >*/
	/* Parent class */
	GObjectClass							parent_class;

	/*< public >*/
	/* Virtual functions */
	void (*menu_reload_required)(XfceOverviewWorkspacesApplicationDatabase *self);

	void (*application_added)(XfceOverviewWorkspacesApplicationDatabase *self, GAppInfo *inAppInfo);
	void (*application_removed)(XfceOverviewWorkspacesApplicationDatabase *self, GAppInfo *inAppInfo);
};

/* Public API */
GType xfce_overview_workspaces_application_database_get_type(void) G_GNUC_CONST;

gboolean xfce_overview_workspaces_application_database_is_loaded(const XfceOverviewWorkspacesApplicationDatabase *self);
gboolean xfce_overview_workspaces_application_database_load(XfceOverviewWorkspacesApplicationDatabase *self, GError **outError);

const GList* xfce_overview_workspaces_application_database_get_application_search_paths(const XfceOverviewWorkspacesApplicationDatabase *self);

GarconMenu* xfce_overview_workspaces_application_database_get_application_menu(XfceOverviewWorkspacesApplicationDatabase *self);
GList* xfce_overview_workspaces_application_database_get_all_applications(XfceOverviewWorkspacesApplicationDatabase *self);

GAppInfo* xfce_overview_workspaces_application_database_lookup_desktop_id(XfceOverviewWorkspacesApplicationDatabase *self,
																const gchar *inDesktopID);

gchar* xfce_overview_workspaces_application_database_get_file_from_desktop_id(const gchar *inDesktopID);
gchar* xfce_overview_workspaces_application_database_get_desktop_id_from_path(const gchar *inFilename);
gchar* xfce_overview_workspaces_application_database_get_desktop_id_from_file(GFile *inFile);

G_END_DECLS

#endif	/* __LIBXFCE_OVERVIEW_WORKSPACES_APPLICATION_DATABASE__ */
