/*
 * workspace-selector: Workspace selector box
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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_WORKSPACE_SELECTOR__
#define __LIBXFCE_OVERVIEW_WORKSPACES_WORKSPACE_SELECTOR__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <libxfce-overview-workspaces/background.h>

G_BEGIN_DECLS

#define XFCE_OVERVIEW_WORKSPACES_TYPE_WORKSPACE_SELECTOR				(xfce_overview_workspaces_workspace_selector_get_type())
#define XFCE_OVERVIEW_WORKSPACES_WORKSPACE_SELECTOR(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_WORKSPACE_SELECTOR, XfceOverviewWorkspacesWorkspaceSelector))
#define XFCE_OVERVIEW_WORKSPACES_IS_WORKSPACE_SELECTOR(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_WORKSPACE_SELECTOR))
#define XFCE_OVERVIEW_WORKSPACES_WORKSPACE_SELECTOR_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_WORKSPACE_SELECTOR, XfceOverviewWorkspacesWorkspaceSelectorClass))
#define XFCE_OVERVIEW_WORKSPACES_IS_WORKSPACE_SELECTOR_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_WORKSPACE_SELECTOR))
#define XFCE_OVERVIEW_WORKSPACES_WORKSPACE_SELECTOR_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_WORKSPACE_SELECTOR, XfceOverviewWorkspacesWorkspaceSelectorClass))

typedef struct _XfceOverviewWorkspacesWorkspaceSelector			XfceOverviewWorkspacesWorkspaceSelector;
typedef struct _XfceOverviewWorkspacesWorkspaceSelectorClass		XfceOverviewWorkspacesWorkspaceSelectorClass;
typedef struct _XfceOverviewWorkspacesWorkspaceSelectorPrivate		XfceOverviewWorkspacesWorkspaceSelectorPrivate;

struct _XfceOverviewWorkspacesWorkspaceSelector
{
	/*< private >*/
	/* Parent instance */
	XfceOverviewWorkspacesBackground				parent_instance;

	/* Private structure */
	XfceOverviewWorkspacesWorkspaceSelectorPrivate	*priv;
};

struct _XfceOverviewWorkspacesWorkspaceSelectorClass
{
	/*< private >*/
	/* Parent class */
	XfceOverviewWorkspacesBackgroundClass			parent_class;

	/*< public >*/
	/* Virtual functions */
};

/* Public API */
GType xfce_overview_workspaces_workspace_selector_get_type(void) G_GNUC_CONST;

ClutterActor* xfce_overview_workspaces_workspace_selector_new(void);
ClutterActor* xfce_overview_workspaces_workspace_selector_new_with_orientation(ClutterOrientation inOrientation);

gfloat xfce_overview_workspaces_workspace_selector_get_spacing(XfceOverviewWorkspacesWorkspaceSelector *self);
void xfce_overview_workspaces_workspace_selector_set_spacing(XfceOverviewWorkspacesWorkspaceSelector *self, const gfloat inSpacing);

ClutterOrientation xfce_overview_workspaces_workspace_selector_get_orientation(XfceOverviewWorkspacesWorkspaceSelector *self);
void xfce_overview_workspaces_workspace_selector_set_orientation(XfceOverviewWorkspacesWorkspaceSelector *self, ClutterOrientation inOrientation);

gfloat xfce_overview_workspaces_workspace_selector_get_maximum_size(XfceOverviewWorkspacesWorkspaceSelector *self);
void xfce_overview_workspaces_workspace_selector_set_maximum_size(XfceOverviewWorkspacesWorkspaceSelector *self, const gfloat inSize);

gfloat xfce_overview_workspaces_workspace_selector_get_maximum_fraction(XfceOverviewWorkspacesWorkspaceSelector *self);
void xfce_overview_workspaces_workspace_selector_set_maximum_fraction(XfceOverviewWorkspacesWorkspaceSelector *self, const gfloat inFraction);

gboolean xfce_overview_workspaces_workspace_selector_is_using_fraction(XfceOverviewWorkspacesWorkspaceSelector *self);

gboolean xfce_overview_workspaces_workspace_selector_get_show_current_monitor_only(XfceOverviewWorkspacesWorkspaceSelector *self);
void xfce_overview_workspaces_workspace_selector_set_show_current_monitor_only(XfceOverviewWorkspacesWorkspaceSelector *self, gboolean inShowCurrentMonitorOnly);

G_END_DECLS

#endif	/* __LIBXFCE_OVERVIEW_WORKSPACES_WORKSPACE_SELECTOR__ */
