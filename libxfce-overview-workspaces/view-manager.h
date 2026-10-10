/*
 * view-manager: Single-instance managing views
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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_VIEW_MANAGER__
#define __LIBXFCE_OVERVIEW_WORKSPACES_VIEW_MANAGER__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <glib-object.h>

G_BEGIN_DECLS

#define XFCE_OVERVIEW_WORKSPACES_TYPE_VIEW_MANAGER				(xfce_overview_workspaces_view_manager_get_type())
#define XFCE_OVERVIEW_WORKSPACES_VIEW_MANAGER(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_VIEW_MANAGER, XfceOverviewWorkspacesViewManager))
#define XFCE_OVERVIEW_WORKSPACES_IS_VIEW_MANAGER(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_VIEW_MANAGER))
#define XFCE_OVERVIEW_WORKSPACES_VIEW_MANAGER_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_VIEW_MANAGER, XfceOverviewWorkspacesViewManagerClass))
#define XFCE_OVERVIEW_WORKSPACES_IS_VIEW_MANAGER_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_VIEW_MANAGER))
#define XFCE_OVERVIEW_WORKSPACES_VIEW_MANAGER_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_VIEW_MANAGER, XfceOverviewWorkspacesViewManagerClass))

typedef struct _XfceOverviewWorkspacesViewManager				XfceOverviewWorkspacesViewManager;
typedef struct _XfceOverviewWorkspacesViewManagerClass			XfceOverviewWorkspacesViewManagerClass;
typedef struct _XfceOverviewWorkspacesViewManagerPrivate		XfceOverviewWorkspacesViewManagerPrivate;

struct _XfceOverviewWorkspacesViewManager
{
	/*< private >*/
	/* Parent instance */
	GObject							parent_instance;

	/* Private structure */
	XfceOverviewWorkspacesViewManagerPrivate	*priv;
};

struct _XfceOverviewWorkspacesViewManagerClass
{
	/*< private >*/
	/* Parent class */
	GObjectClass					parent_class;

	/*< public >*/
	/* Virtual functions */
	void (*registered)(XfceOverviewWorkspacesViewManager *self, const gchar *inID);
	void (*unregistered)(XfceOverviewWorkspacesViewManager *self, const gchar *inID);
};

/* Public API */
GType xfce_overview_workspaces_view_manager_get_type(void) G_GNUC_CONST;

gboolean xfce_overview_workspaces_view_manager_register(XfceOverviewWorkspacesViewManager *self, const gchar *inID, GType inViewType);
gboolean xfce_overview_workspaces_view_manager_unregister(XfceOverviewWorkspacesViewManager *self, const gchar *inID);
GList* xfce_overview_workspaces_view_manager_get_registered(XfceOverviewWorkspacesViewManager *self);
gboolean xfce_overview_workspaces_view_manager_has_registered_id(XfceOverviewWorkspacesViewManager *self, const gchar *inID);

GObject* xfce_overview_workspaces_view_manager_create_view(XfceOverviewWorkspacesViewManager *self, const gchar *inID);

G_END_DECLS

#endif	/* __LIBXFCE_OVERVIEW_WORKSPACES_VIEW_MANAGER__ */
