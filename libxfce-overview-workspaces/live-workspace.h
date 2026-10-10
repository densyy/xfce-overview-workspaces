/*
 * live-workspace: An actor showing the content of a workspace which will
 *                 be updated if changed.
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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_LIVE_WORKSPACE__
#define __LIBXFCE_OVERVIEW_WORKSPACES_LIVE_WORKSPACE__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <clutter/clutter.h>

#include <libxfce-overview-workspaces/background.h>
#include <libxfce-overview-workspaces/window-tracker-workspace.h>
#include <libxfce-overview-workspaces/window-tracker-monitor.h>

G_BEGIN_DECLS

#define XFCE_OVERVIEW_WORKSPACES_TYPE_LIVE_WORKSPACE				(xfce_overview_workspaces_live_workspace_get_type())
#define XFCE_OVERVIEW_WORKSPACES_LIVE_WORKSPACE(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_LIVE_WORKSPACE, XfceOverviewWorkspacesLiveWorkspace))
#define XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WORKSPACE(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_LIVE_WORKSPACE))
#define XFCE_OVERVIEW_WORKSPACES_LIVE_WORKSPACE_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_LIVE_WORKSPACE, XfceOverviewWorkspacesLiveWorkspaceClass))
#define XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WORKSPACE_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_LIVE_WORKSPACE))
#define XFCE_OVERVIEW_WORKSPACES_LIVE_WORKSPACE_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_LIVE_WORKSPACE, XfceOverviewWorkspacesLiveWorkspaceClass))

typedef struct _XfceOverviewWorkspacesLiveWorkspace			XfceOverviewWorkspacesLiveWorkspace;
typedef struct _XfceOverviewWorkspacesLiveWorkspaceClass		XfceOverviewWorkspacesLiveWorkspaceClass;
typedef struct _XfceOverviewWorkspacesLiveWorkspacePrivate		XfceOverviewWorkspacesLiveWorkspacePrivate;

struct _XfceOverviewWorkspacesLiveWorkspace
{
	/*< private >*/
	/* Parent instance */
	XfceOverviewWorkspacesBackground			parent_instance;

	/* Private structure */
	XfceOverviewWorkspacesLiveWorkspacePrivate	*priv;
};

struct _XfceOverviewWorkspacesLiveWorkspaceClass
{
	/*< private >*/
	/* Parent class */
	XfceOverviewWorkspacesBackgroundClass		parent_class;

	/*< public >*/
	/* Virtual functions */
	void (*clicked)(XfceOverviewWorkspacesLiveWorkspace *self);
};

/* Public API */
GType xfce_overview_workspaces_live_workspace_get_type(void) G_GNUC_CONST;

ClutterActor* xfce_overview_workspaces_live_workspace_new(void);
ClutterActor* xfce_overview_workspaces_live_workspace_new_for_workspace(XfceOverviewWorkspacesWindowTrackerWorkspace *inWorkspace);

XfceOverviewWorkspacesWindowTrackerWorkspace* xfce_overview_workspaces_live_workspace_get_workspace(XfceOverviewWorkspacesLiveWorkspace *self);
void xfce_overview_workspaces_live_workspace_set_workspace(XfceOverviewWorkspacesLiveWorkspace *self, XfceOverviewWorkspacesWindowTrackerWorkspace *inWorkspace);

XfceOverviewWorkspacesWindowTrackerMonitor* xfce_overview_workspaces_live_workspace_get_monitor(XfceOverviewWorkspacesLiveWorkspace *self);
void xfce_overview_workspaces_live_workspace_set_monitor(XfceOverviewWorkspacesLiveWorkspace *self, XfceOverviewWorkspacesWindowTrackerMonitor *inMonitor);

XfceOverviewWorkspacesStageBackgroundImageType xfce_overview_workspaces_live_workspace_get_background_image_type(XfceOverviewWorkspacesLiveWorkspace *self);
void xfce_overview_workspaces_live_workspace_set_background_image_type(XfceOverviewWorkspacesLiveWorkspace *self, XfceOverviewWorkspacesStageBackgroundImageType inType);

gboolean xfce_overview_workspaces_live_workspace_get_show_workspace_name(XfceOverviewWorkspacesLiveWorkspace *self);
void xfce_overview_workspaces_live_workspace_set_show_workspace_name(XfceOverviewWorkspacesLiveWorkspace *self, gboolean inIsVisible);

gfloat xfce_overview_workspaces_live_workspace_get_workspace_name_padding(XfceOverviewWorkspacesLiveWorkspace *self);
void xfce_overview_workspaces_live_workspace_set_workspace_name_padding(XfceOverviewWorkspacesLiveWorkspace *self, gfloat inPadding);

G_END_DECLS

#endif
