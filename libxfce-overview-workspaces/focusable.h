/*
 * focusable: An interface which can be inherited by actors to get
 *            managed by focus manager for keyboard navigation and
 *            selection handling
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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_FOCUSABLE__
#define __LIBXFCE_OVERVIEW_WORKSPACES_FOCUSABLE__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <clutter/clutter.h>

#include <libxfce-overview-workspaces/types.h>

G_BEGIN_DECLS

#define XFCE_OVERVIEW_WORKSPACES_TYPE_FOCUSABLE				(xfce_overview_workspaces_focusable_get_type())
#define XFCE_OVERVIEW_WORKSPACES_FOCUSABLE(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_FOCUSABLE, XfceOverviewWorkspacesFocusable))
#define XFCE_OVERVIEW_WORKSPACES_IS_FOCUSABLE(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_FOCUSABLE))
#define XFCE_OVERVIEW_WORKSPACES_FOCUSABLE_GET_IFACE(obj)	(G_TYPE_INSTANCE_GET_INTERFACE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_FOCUSABLE, XfceOverviewWorkspacesFocusableInterface))

typedef struct _XfceOverviewWorkspacesFocusable			XfceOverviewWorkspacesFocusable;
typedef struct _XfceOverviewWorkspacesFocusableInterface	XfceOverviewWorkspacesFocusableInterface;

struct _XfceOverviewWorkspacesFocusableInterface
{
	/*< private >*/
	/* Parent interface */
	GTypeInterface				parent_interface;

	/*< public >*/
	/* Virtual functions */
	gboolean (*can_focus)(XfceOverviewWorkspacesFocusable *self);
	void (*set_focus)(XfceOverviewWorkspacesFocusable *self);
	void (*unset_focus)(XfceOverviewWorkspacesFocusable *self);

	gboolean (*supports_selection)(XfceOverviewWorkspacesFocusable *self);
	ClutterActor* (*get_selection)(XfceOverviewWorkspacesFocusable *self);
	gboolean (*set_selection)(XfceOverviewWorkspacesFocusable *self, ClutterActor *inSelection);
	ClutterActor* (*find_selection)(XfceOverviewWorkspacesFocusable *self, ClutterActor *inSelection, XfceOverviewWorkspacesSelectionTarget inDirection);
	gboolean (*activate_selection)(XfceOverviewWorkspacesFocusable *self, ClutterActor *inSelection);

	/* Binding actions */
	gboolean (*selection_move_left)(XfceOverviewWorkspacesFocusable *self,
											XfceOverviewWorkspacesFocusable *inSource,
											const gchar *inAction,
											ClutterEvent *inEvent);
	gboolean (*selection_move_right)(XfceOverviewWorkspacesFocusable *self,
											XfceOverviewWorkspacesFocusable *inSource,
											const gchar *inAction,
											ClutterEvent *inEvent);
	gboolean (*selection_move_up)(XfceOverviewWorkspacesFocusable *self,
											XfceOverviewWorkspacesFocusable *inSource,
											const gchar *inAction,
											ClutterEvent *inEvent);
	gboolean (*selection_move_down)(XfceOverviewWorkspacesFocusable *self,
											XfceOverviewWorkspacesFocusable *inSource,
											const gchar *inAction,
											ClutterEvent *inEvent);
	gboolean (*selection_move_first)(XfceOverviewWorkspacesFocusable *self,
											XfceOverviewWorkspacesFocusable *inSource,
											const gchar *inAction,
											ClutterEvent *inEvent);
	gboolean (*selection_move_last)(XfceOverviewWorkspacesFocusable *self,
											XfceOverviewWorkspacesFocusable *inSource,
											const gchar *inAction,
											ClutterEvent *inEvent);
	gboolean (*selection_move_next)(XfceOverviewWorkspacesFocusable *self,
											XfceOverviewWorkspacesFocusable *inSource,
											const gchar *inAction,
											ClutterEvent *inEvent);
	gboolean (*selection_move_previous)(XfceOverviewWorkspacesFocusable *self,
											XfceOverviewWorkspacesFocusable *inSource,
											const gchar *inAction,
											ClutterEvent *inEvent);
	gboolean (*selection_move_page_left)(XfceOverviewWorkspacesFocusable *self,
											XfceOverviewWorkspacesFocusable *inSource,
											const gchar *inAction,
											ClutterEvent *inEvent);
	gboolean (*selection_move_page_right)(XfceOverviewWorkspacesFocusable *self,
											XfceOverviewWorkspacesFocusable *inSource,
											const gchar *inAction,
											ClutterEvent *inEvent);
	gboolean (*selection_move_page_up)(XfceOverviewWorkspacesFocusable *self,
											XfceOverviewWorkspacesFocusable *inSource,
											const gchar *inAction,
											ClutterEvent *inEvent);
	gboolean (*selection_move_page_down)(XfceOverviewWorkspacesFocusable *self,
											XfceOverviewWorkspacesFocusable *inSource,
											const gchar *inAction,
											ClutterEvent *inEvent);
	gboolean (*selection_activate)(XfceOverviewWorkspacesFocusable *self,
											XfceOverviewWorkspacesFocusable *inSource,
											const gchar *inAction,
											ClutterEvent *inEvent);

	gboolean (*focus_move_to)(XfceOverviewWorkspacesFocusable *self,
								XfceOverviewWorkspacesFocusable *inSource,
								const gchar *inAction,
								ClutterEvent *inEvent);
};

/* Public API */
GType xfce_overview_workspaces_focusable_get_type(void) G_GNUC_CONST;

gboolean xfce_overview_workspaces_focusable_can_focus(XfceOverviewWorkspacesFocusable *self);
void xfce_overview_workspaces_focusable_set_focus(XfceOverviewWorkspacesFocusable *self);
void xfce_overview_workspaces_focusable_unset_focus(XfceOverviewWorkspacesFocusable *self);

gboolean xfce_overview_workspaces_focusable_supports_selection(XfceOverviewWorkspacesFocusable *self);
ClutterActor* xfce_overview_workspaces_focusable_get_selection(XfceOverviewWorkspacesFocusable *self);
gboolean xfce_overview_workspaces_focusable_set_selection(XfceOverviewWorkspacesFocusable *self, ClutterActor *inSelection);
ClutterActor* xfce_overview_workspaces_focusable_find_selection(XfceOverviewWorkspacesFocusable *self, ClutterActor *inSelection, XfceOverviewWorkspacesSelectionTarget inDirection);
gboolean xfce_overview_workspaces_focusable_activate_selection(XfceOverviewWorkspacesFocusable *self, ClutterActor *inSelection);

gboolean xfce_overview_workspaces_focusable_move_focus_to(XfceOverviewWorkspacesFocusable *self);

G_END_DECLS

#endif	/* __LIBXFCE_OVERVIEW_WORKSPACES_FOCUSABLE__ */
