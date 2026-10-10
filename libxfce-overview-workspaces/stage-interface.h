/*
 * stage-interface: A top-level actor for a monitor at stage
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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_STAGE_INTERFACE__
#define __LIBXFCE_OVERVIEW_WORKSPACES_STAGE_INTERFACE__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <clutter/clutter.h>

#include <libxfce-overview-workspaces/stage.h>
#include <libxfce-overview-workspaces/types.h>
#include <libxfce-overview-workspaces/window-tracker-monitor.h>

G_BEGIN_DECLS

#define XFCE_OVERVIEW_WORKSPACES_TYPE_STAGE_INTERFACE			(xfce_overview_workspaces_stage_interface_get_type())
#define XFCE_OVERVIEW_WORKSPACES_STAGE_INTERFACE(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_STAGE_INTERFACE, XfceOverviewWorkspacesStageInterface))
#define XFCE_OVERVIEW_WORKSPACES_IS_STAGE_INTERFACE(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_STAGE_INTERFACE))
#define XFCE_OVERVIEW_WORKSPACES_STAGE_INTERFACE_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_STAGE_INTERFACE, XfceOverviewWorkspacesStageInterfaceClass))
#define XFCE_OVERVIEW_WORKSPACES_IS_STAGE_INTERFACE_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_STAGE_INTERFACE))
#define XFCE_OVERVIEW_WORKSPACES_STAGE_INTERFACE_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_STAGE_INTERFACE, XfceOverviewWorkspacesStageInterfaceClass))

typedef struct _XfceOverviewWorkspacesStageInterface			XfceOverviewWorkspacesStageInterface;
typedef struct _XfceOverviewWorkspacesStageInterfaceClass		XfceOverviewWorkspacesStageInterfaceClass;
typedef struct _XfceOverviewWorkspacesStageInterfacePrivate	XfceOverviewWorkspacesStageInterfacePrivate;

struct _XfceOverviewWorkspacesStageInterface
{
	/*< private >*/
	/* Parent instance */
	XfceOverviewWorkspacesStage						parent_instance;

	/* Private structure */
	XfceOverviewWorkspacesStageInterfacePrivate		*priv;
};

struct _XfceOverviewWorkspacesStageInterfaceClass
{
	/*< private >*/
	/* Parent class */
	XfceOverviewWorkspacesStageClass					parent_class;
};

/* Public API */
GType xfce_overview_workspaces_stage_interface_get_type(void) G_GNUC_CONST;

ClutterActor* xfce_overview_workspaces_stage_interface_new(void);

XfceOverviewWorkspacesWindowTrackerMonitor* xfce_overview_workspaces_stage_interface_get_monitor(XfceOverviewWorkspacesStageInterface *self);
void xfce_overview_workspaces_stage_interface_set_monitor(XfceOverviewWorkspacesStageInterface *self, XfceOverviewWorkspacesWindowTrackerMonitor *inMonitor);

XfceOverviewWorkspacesStageBackgroundImageType xfce_overview_workspaces_stage_interface_get_background_image_type(XfceOverviewWorkspacesStageInterface *self);
void xfce_overview_workspaces_stage_interface_set_background_image_type(XfceOverviewWorkspacesStageInterface *self, XfceOverviewWorkspacesStageBackgroundImageType inType);

ClutterColor* xfce_overview_workspaces_stage_interface_get_background_color(XfceOverviewWorkspacesStageInterface *self);
void xfce_overview_workspaces_stage_interface_set_background_color(XfceOverviewWorkspacesStageInterface *self, const ClutterColor *inColor);

G_END_DECLS

#endif	/* __LIBXFCE_OVERVIEW_WORKSPACES_STAGE_INTERFACE__ */
