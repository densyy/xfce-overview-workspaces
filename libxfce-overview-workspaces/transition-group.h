/*
 * transition-group: A grouping transition
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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_TRANSITION_GROUP__
#define __LIBXFCE_OVERVIEW_WORKSPACES_TRANSITION_GROUP__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <clutter/clutter.h>

#include <libxfce-overview-workspaces/actor.h>

G_BEGIN_DECLS

/* Object declaration */
#define XFCE_OVERVIEW_WORKSPACES_TYPE_TRANSITION_GROUP				(xfce_overview_workspaces_transition_group_get_type())
#define XFCE_OVERVIEW_WORKSPACES_TRANSITION_GROUP(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_TRANSITION_GROUP, XfceOverviewWorkspacesTransitionGroup))
#define XFCE_OVERVIEW_WORKSPACES_IS_TRANSITION_GROUP(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_TRANSITION_GROUP))
#define XFCE_OVERVIEW_WORKSPACES_TRANSITION_GROUP_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_TRANSITION_GROUP, XfceOverviewWorkspacesTransitionGroupClass))
#define XFCE_OVERVIEW_WORKSPACES_IS_TRANSITION_GROUP_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_TRANSITION_GROUP))
#define XFCE_OVERVIEW_WORKSPACES_TRANSITION_GROUP_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_TRANSITION_GROUP, XfceOverviewWorkspacesTransitionGroupClass))

typedef struct _XfceOverviewWorkspacesTransitionGroup			XfceOverviewWorkspacesTransitionGroup;
typedef struct _XfceOverviewWorkspacesTransitionGroupClass		XfceOverviewWorkspacesTransitionGroupClass;
typedef struct _XfceOverviewWorkspacesTransitionGroupPrivate		XfceOverviewWorkspacesTransitionGroupPrivate;

/**
 * XfceOverviewWorkspacesTransitionGroup:
 *
 * The #XfceOverviewWorkspacesTransitionGroup structure contains only private data and
 * should be accessed using the provided API
 */
 struct _XfceOverviewWorkspacesTransitionGroup
{
	/*< private >*/
	/* Parent instance */
	ClutterTransition					parent_instance;

	/* Private structure */
	XfceOverviewWorkspacesTransitionGroupPrivate	*priv;
};

/**
 * XfceOverviewWorkspacesTransitionGroupClass:
 *
 * The #XfceOverviewWorkspacesTransitionGroupClass structure contains only private data
 */
struct _XfceOverviewWorkspacesTransitionGroupClass
{
	/*< private >*/
	/* Parent class */
	ClutterTransitionClass				parent_class;

	/*< public >*/
	/* Virtual functions */
};

/* Public API */
GType xfce_overview_workspaces_transition_group_get_type(void) G_GNUC_CONST;

ClutterTransition* xfce_overview_workspaces_transition_group_new(void);

void xfce_overview_workspaces_transition_group_add_transition(XfceOverviewWorkspacesTransitionGroup *self,
													ClutterTransition *inTransition);
void xfce_overview_workspaces_transition_group_remove_transition(XfceOverviewWorkspacesTransitionGroup *self,
													ClutterTransition *inTransition);
void xfce_overview_workspaces_transition_group_remove_all(XfceOverviewWorkspacesTransitionGroup *self);

GSList* xfce_overview_workspaces_transition_group_get_transitions(XfceOverviewWorkspacesTransitionGroup *self);

G_END_DECLS

#endif	/* __LIBXFCE_OVERVIEW_WORKSPACES_TRANSITION_GROUP__ */
