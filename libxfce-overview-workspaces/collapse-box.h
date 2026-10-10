/*
 * collapse-box: A collapsable container for one actor
 *               with capability to expand
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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_COLLAPSE_BOX__
#define __LIBXFCE_OVERVIEW_WORKSPACES_COLLAPSE_BOX__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <clutter/clutter.h>

#include <libxfce-overview-workspaces/actor.h>
#include <libxfce-overview-workspaces/types.h>

G_BEGIN_DECLS

#define XFCE_OVERVIEW_WORKSPACES_TYPE_COLLAPSE_BOX				(xfce_overview_workspaces_collapse_box_get_type())
#define XFCE_OVERVIEW_WORKSPACES_COLLAPSE_BOX(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_COLLAPSE_BOX, XfceOverviewWorkspacesCollapseBox))
#define XFCE_OVERVIEW_WORKSPACES_IS_COLLAPSE_BOX(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_COLLAPSE_BOX))
#define XFCE_OVERVIEW_WORKSPACES_COLLAPSE_BOX_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_COLLAPSE_BOX, XfceOverviewWorkspacesCollapseBoxClass))
#define XFCE_OVERVIEW_WORKSPACES_IS_COLLAPSE_BOX_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_COLLAPSE_BOX))
#define XFCE_OVERVIEW_WORKSPACES_COLLAPSE_BOX_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_COLLAPSE_BOX, XfceOverviewWorkspacesCollapseBoxClass))

typedef struct _XfceOverviewWorkspacesCollapseBox				XfceOverviewWorkspacesCollapseBox; 
typedef struct _XfceOverviewWorkspacesCollapseBoxPrivate		XfceOverviewWorkspacesCollapseBoxPrivate;
typedef struct _XfceOverviewWorkspacesCollapseBoxClass			XfceOverviewWorkspacesCollapseBoxClass;

struct _XfceOverviewWorkspacesCollapseBox
{
	/*< private >*/
	/* Parent instance */
	XfceOverviewWorkspacesActor				parent_instance;

	/* Private structure */
	XfceOverviewWorkspacesCollapseBoxPrivate	*priv;
};

struct _XfceOverviewWorkspacesCollapseBoxClass
{
	/*< private >*/
	/* Parent class */
	XfceOverviewWorkspacesActorClass			parent_class;

	/*< public >*/
	/* Virtual functions */
	void (*collapse_changed)(XfceOverviewWorkspacesCollapseBox *self, gboolean isCollapsed);
};

/* Public API */
GType xfce_overview_workspaces_collapse_box_get_type(void) G_GNUC_CONST;

ClutterActor* xfce_overview_workspaces_collapse_box_new(void);

gboolean xfce_overview_workspaces_collapse_box_get_collapsed(XfceOverviewWorkspacesCollapseBox *self);
void xfce_overview_workspaces_collapse_box_set_collapsed(XfceOverviewWorkspacesCollapseBox *self, gboolean inCollapsed);

gfloat xfce_overview_workspaces_collapse_box_get_collapsed_size(XfceOverviewWorkspacesCollapseBox *self);
void xfce_overview_workspaces_collapse_box_set_collapsed_size(XfceOverviewWorkspacesCollapseBox *self, gfloat inCollapsedSize);

XfceOverviewWorkspacesOrientation xfce_overview_workspaces_collapse_box_get_collapse_orientation(XfceOverviewWorkspacesCollapseBox *self);
void xfce_overview_workspaces_collapse_box_set_collapse_orientation(XfceOverviewWorkspacesCollapseBox *self, XfceOverviewWorkspacesOrientation inOrientation);

gfloat xfce_overview_workspaces_collapse_box_get_collapse_progress(XfceOverviewWorkspacesCollapseBox *self);
void xfce_overview_workspaces_collapse_box_set_collapse_progress(XfceOverviewWorkspacesCollapseBox *self, gfloat inProgress);

G_END_DECLS

#endif	/* __LIBXFCE_OVERVIEW_WORKSPACES_COLLAPSE_BOX__ */
