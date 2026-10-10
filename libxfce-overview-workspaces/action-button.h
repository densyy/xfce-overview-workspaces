/*
 * action-button: A button representing an action to execute when clicked
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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_ACTION_BUTTON__
#define __LIBXFCE_OVERVIEW_WORKSPACES_ACTION_BUTTON__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <libxfce-overview-workspaces/button.h>

G_BEGIN_DECLS

#define XFCE_OVERVIEW_WORKSPACES_TYPE_ACTION_BUTTON				(xfce_overview_workspaces_action_button_get_type())
#define XFCE_OVERVIEW_WORKSPACES_ACTION_BUTTON(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_ACTION_BUTTON, XfceOverviewWorkspacesActionButton))
#define XFCE_OVERVIEW_WORKSPACES_IS_ACTION_BUTTON(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_ACTION_BUTTON))
#define XFCE_OVERVIEW_WORKSPACES_ACTION_BUTTON_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_ACTION_BUTTON, XfceOverviewWorkspacesActionButtonClass))
#define XFCE_OVERVIEW_WORKSPACES_IS_ACTION_BUTTON_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_ACTION_BUTTON))
#define XFCE_OVERVIEW_WORKSPACES_ACTION_BUTTON_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_ACTION_BUTTON, XfceOverviewWorkspacesActionButtonClass))

typedef struct _XfceOverviewWorkspacesActionButton				XfceOverviewWorkspacesActionButton;
typedef struct _XfceOverviewWorkspacesActionButtonClass		XfceOverviewWorkspacesActionButtonClass;
typedef struct _XfceOverviewWorkspacesActionButtonPrivate		XfceOverviewWorkspacesActionButtonPrivate;

/**
 * XfceOverviewWorkspacesActionButton:
 *
 * The #XfceOverviewWorkspacesActionButton structure contains only private data and
 * should be accessed using the provided API
 */
struct _XfceOverviewWorkspacesActionButton
{
	/*< private >*/
	/* Parent instance */
	XfceOverviewWorkspacesButton						parent_instance;

	/* Private structure */
	XfceOverviewWorkspacesActionButtonPrivate			*priv;
};

/**
 * XfceOverviewWorkspacesActionButtonClass:
 *
 * The #XfceOverviewWorkspacesActionButtonClass structure contains only private data
 */
struct _XfceOverviewWorkspacesActionButtonClass
{
	/*< private >*/
	/* Parent class */
	XfceOverviewWorkspacesButtonClass					parent_class;

	/*< public >*/
	/* Virtual functions */
};

/* Public API */
GType xfce_overview_workspaces_action_button_get_type(void) G_GNUC_CONST;

ClutterActor* xfce_overview_workspaces_action_button_new(void);

const gchar* xfce_overview_workspaces_action_button_get_target(XfceOverviewWorkspacesActionButton *self);
void xfce_overview_workspaces_action_button_set_target(XfceOverviewWorkspacesActionButton *self, const gchar *inTarget);

const gchar* xfce_overview_workspaces_action_button_get_action(XfceOverviewWorkspacesActionButton *self);
void xfce_overview_workspaces_action_button_set_action(XfceOverviewWorkspacesActionButton *self, const gchar *inAction);

G_END_DECLS

#endif	/* __LIBXFCE_OVERVIEW_WORKSPACES_ACTION_BUTTON__ */
