/*
 * drop-action: Drop action for drop targets
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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_DROP_ACTION__
#define __LIBXFCE_OVERVIEW_WORKSPACES_DROP_ACTION__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <clutter/clutter.h>

#include <libxfce-overview-workspaces/drag-action.h>

G_BEGIN_DECLS

#define XFCE_OVERVIEW_WORKSPACES_TYPE_DROP_ACTION				(xfce_overview_workspaces_drop_action_get_type())
#define XFCE_OVERVIEW_WORKSPACES_DROP_ACTION(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_DROP_ACTION, XfceOverviewWorkspacesDropAction))
#define XFCE_OVERVIEW_WORKSPACES_IS_DROP_ACTION(obj)				(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_DROP_ACTION))
#define XFCE_OVERVIEW_WORKSPACES_DROP_ACTION_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_DROP_ACTION, XfceOverviewWorkspacesDropActionClass))
#define XFCE_OVERVIEW_WORKSPACES_IS_DROP_ACTION_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_DROP_ACTION))
#define XFCE_OVERVIEW_WORKSPACES_DROP_ACTION_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_DROP_ACTION, XfceOverviewWorkspacesDropActionClass))

typedef struct _XfceOverviewWorkspacesDropAction				XfceOverviewWorkspacesDropAction;
typedef struct _XfceOverviewWorkspacesDropActionClass			XfceOverviewWorkspacesDropActionClass;
typedef struct _XfceOverviewWorkspacesDropActionPrivate		XfceOverviewWorkspacesDropActionPrivate;

struct _XfceOverviewWorkspacesDropAction
{
	/*< private >*/
	/* Parent instance */
	ClutterAction					parent_instance;

	/* Private structure */
	XfceOverviewWorkspacesDropActionPrivate	*priv;
};

struct _XfceOverviewWorkspacesDropActionClass
{
	/*< private >*/
	/* Parent class */
	ClutterActionClass				parent_class;

	/*< public >*/
	/* Virtual functions */
	gboolean (*begin)(XfceOverviewWorkspacesDropAction *self, XfceOverviewWorkspacesDragAction *inDragAction);
	gboolean (*can_drop)(XfceOverviewWorkspacesDropAction *self, XfceOverviewWorkspacesDragAction *inDragAction, gfloat inX, gfloat inY);
	void (*drop)(XfceOverviewWorkspacesDropAction *self, XfceOverviewWorkspacesDragAction *inDragAction, gfloat inX, gfloat inY);
	void (*end)(XfceOverviewWorkspacesDropAction *self, XfceOverviewWorkspacesDragAction *inDragAction);

	void (*drag_enter)(XfceOverviewWorkspacesDropAction *self, XfceOverviewWorkspacesDragAction *inDragAction);
	void (*drag_motion)(XfceOverviewWorkspacesDropAction *self, XfceOverviewWorkspacesDragAction *inDragAction, gfloat inX, gfloat inY);
	void (*drag_leave)(XfceOverviewWorkspacesDropAction *self, XfceOverviewWorkspacesDragAction *inDragAction);
};

/* Public API */
GType xfce_overview_workspaces_drop_action_get_type(void) G_GNUC_CONST;

ClutterAction* xfce_overview_workspaces_drop_action_new();

GSList* xfce_overview_workspaces_drop_action_get_targets(void);

G_END_DECLS

#endif	/* __LIBXFCE_OVERVIEW_WORKSPACES_DROP_ACTION__ */
