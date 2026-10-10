/*
 * drag-action: Drag action for actors
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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_DRAG_ACTION__
#define __LIBXFCE_OVERVIEW_WORKSPACES_DRAG_ACTION__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <clutter/clutter.h>

G_BEGIN_DECLS

#define XFCE_OVERVIEW_WORKSPACES_TYPE_DRAG_ACTION				(xfce_overview_workspaces_drag_action_get_type())
#define XFCE_OVERVIEW_WORKSPACES_DRAG_ACTION(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_DRAG_ACTION, XfceOverviewWorkspacesDragAction))
#define XFCE_OVERVIEW_WORKSPACES_IS_DRAG_ACTION(obj)				(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_DRAG_ACTION))
#define XFCE_OVERVIEW_WORKSPACES_DRAG_ACTION_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_DRAG_ACTION, XfceOverviewWorkspacesDragActionClass))
#define XFCE_OVERVIEW_WORKSPACES_IS_DRAG_ACTION_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_DRAG_ACTION))
#define XFCE_OVERVIEW_WORKSPACES_DRAG_ACTION_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_DRAG_ACTION, XfceOverviewWorkspacesDragActionClass))

typedef struct _XfceOverviewWorkspacesDragAction				XfceOverviewWorkspacesDragAction;
typedef struct _XfceOverviewWorkspacesDragActionClass			XfceOverviewWorkspacesDragActionClass;
typedef struct _XfceOverviewWorkspacesDragActionPrivate		XfceOverviewWorkspacesDragActionPrivate;

struct _XfceOverviewWorkspacesDragAction
{
	/*< private >*/
	/* Parent instance */
	ClutterDragAction				parent_instance;

	/* Private structure */
	XfceOverviewWorkspacesDragActionPrivate	*priv;
};

struct _XfceOverviewWorkspacesDragActionClass
{
	/*< private >*/
	/* Parent class */
	ClutterDragActionClass			parent_class;

	/*< public >*/
	/* Virtual functions */
	void (*drag_cancel)(XfceOverviewWorkspacesDragAction *self, ClutterActor *inActor, gfloat inX, gfloat inY);
};

/* Public API */
GType xfce_overview_workspaces_drag_action_get_type(void) G_GNUC_CONST;

ClutterAction* xfce_overview_workspaces_drag_action_new();
ClutterAction* xfce_overview_workspaces_drag_action_new_with_source(ClutterActor *inSource);

ClutterActor* xfce_overview_workspaces_drag_action_get_source(XfceOverviewWorkspacesDragAction *self);
ClutterActor* xfce_overview_workspaces_drag_action_get_actor(XfceOverviewWorkspacesDragAction *self);

void xfce_overview_workspaces_drag_action_get_motion_delta(XfceOverviewWorkspacesDragAction *self, gfloat *outDeltaX, gfloat *outDeltaY);

G_END_DECLS

#endif	/* __LIBXFCE_OVERVIEW_WORKSPACES_DRAG_ACTION__ */
