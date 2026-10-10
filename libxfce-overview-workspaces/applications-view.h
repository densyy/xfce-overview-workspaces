/*
 * applications-view: A view showing all installed applications as menu
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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_APPLICATIONS_VIEW__
#define __LIBXFCE_OVERVIEW_WORKSPACES_APPLICATIONS_VIEW__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <libxfce-overview-workspaces/view.h>

G_BEGIN_DECLS

#define XFCE_OVERVIEW_WORKSPACES_TYPE_APPLICATIONS_VIEW				(xfce_overview_workspaces_applications_view_get_type())
#define XFCE_OVERVIEW_WORKSPACES_APPLICATIONS_VIEW(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_APPLICATIONS_VIEW, XfceOverviewWorkspacesApplicationsView))
#define XFCE_OVERVIEW_WORKSPACES_IS_APPLICATIONS_VIEW(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_APPLICATIONS_VIEW))
#define XFCE_OVERVIEW_WORKSPACES_APPLICATIONS_VIEW_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_APPLICATIONS_VIEW, XfceOverviewWorkspacesApplicationsViewClass))
#define XFCE_OVERVIEW_WORKSPACES_IS_APPLICATIONS_VIEW_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_APPLICATIONS_VIEW))
#define XFCE_OVERVIEW_WORKSPACES_APPLICATIONS_VIEW_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_APPLICATIONS_VIEW, XfceOverviewWorkspacesApplicationsViewClass))

typedef struct _XfceOverviewWorkspacesApplicationsView				XfceOverviewWorkspacesApplicationsView; 
typedef struct _XfceOverviewWorkspacesApplicationsViewPrivate		XfceOverviewWorkspacesApplicationsViewPrivate;
typedef struct _XfceOverviewWorkspacesApplicationsViewClass		XfceOverviewWorkspacesApplicationsViewClass;

struct _XfceOverviewWorkspacesApplicationsView
{
	/*< private >*/
	/* Parent instance */
	XfceOverviewWorkspacesView						parent_instance;

	/* Private structure */
	XfceOverviewWorkspacesApplicationsViewPrivate	*priv;
};

struct _XfceOverviewWorkspacesApplicationsViewClass
{
	/*< private >*/
	/* Parent class */
	XfceOverviewWorkspacesViewClass				parent_class;
};

/* Public API */
GType xfce_overview_workspaces_applications_view_get_type(void) G_GNUC_CONST;

XfceOverviewWorkspacesViewMode xfce_overview_workspaces_applications_view_get_view_mode(XfceOverviewWorkspacesApplicationsView *self);
void xfce_overview_workspaces_applications_view_set_view_mode(XfceOverviewWorkspacesApplicationsView *self, const XfceOverviewWorkspacesViewMode inMode);

gfloat xfce_overview_workspaces_applications_view_get_spacing(XfceOverviewWorkspacesApplicationsView *self);
void xfce_overview_workspaces_applications_view_set_spacing(XfceOverviewWorkspacesApplicationsView *self, const gfloat inSpacing);

const gchar* xfce_overview_workspaces_applications_view_get_parent_menu_icon(XfceOverviewWorkspacesApplicationsView *self);
void xfce_overview_workspaces_applications_view_set_parent_menu_icon(XfceOverviewWorkspacesApplicationsView *self, const gchar *inIconName);

const gchar* xfce_overview_workspaces_applications_view_get_format_title_only(XfceOverviewWorkspacesApplicationsView *self);
void xfce_overview_workspaces_applications_view_set_format_title_only(XfceOverviewWorkspacesApplicationsView *self, const gchar *inFormat);

const gchar* xfce_overview_workspaces_applications_view_get_format_title_description(XfceOverviewWorkspacesApplicationsView *self);
void xfce_overview_workspaces_applications_view_set_format_title_description(XfceOverviewWorkspacesApplicationsView *self, const gchar *inFormat);

gboolean xfce_overview_workspaces_applications_view_get_show_all_apps(XfceOverviewWorkspacesApplicationsView *self);
void xfce_overview_workspaces_applications_view_set_show_all_apps(XfceOverviewWorkspacesApplicationsView *self, gboolean inShowAllApps);

G_END_DECLS

#endif	/* __LIBXFCE_OVERVIEW_WORKSPACES_APPLICATIONS_VIEW__ */
