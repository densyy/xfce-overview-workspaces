/*
 * focus-manager: Single-instance managing focusable actors
 *                for keyboard navigation
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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_FOCUS_MANAGER__
#define __LIBXFCE_OVERVIEW_WORKSPACES_FOCUS_MANAGER__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <glib-object.h>

#include <libxfce-overview-workspaces/focusable.h>

G_BEGIN_DECLS

#define XFCE_OVERVIEW_WORKSPACES_TYPE_FOCUS_MANAGER				(xfce_overview_workspaces_focus_manager_get_type())
#define XFCE_OVERVIEW_WORKSPACES_FOCUS_MANAGER(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_FOCUS_MANAGER, XfceOverviewWorkspacesFocusManager))
#define XFCE_OVERVIEW_WORKSPACES_IS_FOCUS_MANAGER(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_FOCUS_MANAGER))
#define XFCE_OVERVIEW_WORKSPACES_FOCUS_MANAGER_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_FOCUS_MANAGER, XfceOverviewWorkspacesFocusManagerClass))
#define XFCE_OVERVIEW_WORKSPACES_IS_FOCUS_MANAGER_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_FOCUS_MANAGER))
#define XFCE_OVERVIEW_WORKSPACES_FOCUS_MANAGER_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_FOCUS_MANAGER, XfceOverviewWorkspacesFocusManagerClass))

typedef struct _XfceOverviewWorkspacesFocusManager				XfceOverviewWorkspacesFocusManager;
typedef struct _XfceOverviewWorkspacesFocusManagerClass		XfceOverviewWorkspacesFocusManagerClass;
typedef struct _XfceOverviewWorkspacesFocusManagerPrivate		XfceOverviewWorkspacesFocusManagerPrivate;

struct _XfceOverviewWorkspacesFocusManager
{
	/*< private >*/
	/* Parent instance */
	GObject							parent_instance;

	/* Private structure */
	XfceOverviewWorkspacesFocusManagerPrivate	*priv;
};

struct _XfceOverviewWorkspacesFocusManagerClass
{
	/*< private >*/
	/* Parent class */
	GObjectClass					parent_class;

	/*< public >*/
	/* Virtual functions */
	void (*registered)(XfceOverviewWorkspacesFocusManager *self, XfceOverviewWorkspacesFocusable *inActor);
	void (*unregistered)(XfceOverviewWorkspacesFocusManager *self, XfceOverviewWorkspacesFocusable *inActor);

	void (*changed)(XfceOverviewWorkspacesFocusManager *self,
						XfceOverviewWorkspacesFocusable *oldActor,
						XfceOverviewWorkspacesFocusable *newActor);

	/* Binding actions */
	gboolean (*focus_move_first)(XfceOverviewWorkspacesFocusManager *self,
									XfceOverviewWorkspacesFocusable *inSource,
									const gchar *inAction,
									ClutterEvent *inEvent);
	gboolean (*focus_move_last)(XfceOverviewWorkspacesFocusManager *self,
								XfceOverviewWorkspacesFocusable *inSource,
								const gchar *inAction,
								ClutterEvent *inEvent);
	gboolean (*focus_move_next)(XfceOverviewWorkspacesFocusManager *self,
									XfceOverviewWorkspacesFocusable *inSource,
									const gchar *inAction,
									ClutterEvent *inEvent);
	gboolean (*focus_move_previous)(XfceOverviewWorkspacesFocusManager *self,
									XfceOverviewWorkspacesFocusable *inSource,
									const gchar *inAction,
									ClutterEvent *inEvent);
};

/* Public API */
GType xfce_overview_workspaces_focus_manager_get_type(void) G_GNUC_CONST;

void xfce_overview_workspaces_focus_manager_register(XfceOverviewWorkspacesFocusManager *self, XfceOverviewWorkspacesFocusable *inFocusable);
void xfce_overview_workspaces_focus_manager_register_after(XfceOverviewWorkspacesFocusManager *self, XfceOverviewWorkspacesFocusable *inFocusable, XfceOverviewWorkspacesFocusable *inAfterFocusable);
void xfce_overview_workspaces_focus_manager_unregister(XfceOverviewWorkspacesFocusManager *self, XfceOverviewWorkspacesFocusable *inFocusable);
GList* xfce_overview_workspaces_focus_manager_get_registered(XfceOverviewWorkspacesFocusManager *self);
gboolean xfce_overview_workspaces_focus_manager_is_registered(XfceOverviewWorkspacesFocusManager *self, XfceOverviewWorkspacesFocusable *inFocusable);

GSList* xfce_overview_workspaces_focus_manager_get_targets(XfceOverviewWorkspacesFocusManager *self, const gchar *inTarget);

gboolean xfce_overview_workspaces_focus_manager_has_focus(XfceOverviewWorkspacesFocusManager *self, XfceOverviewWorkspacesFocusable *inFocusable);
XfceOverviewWorkspacesFocusable* xfce_overview_workspaces_focus_manager_get_focus(XfceOverviewWorkspacesFocusManager *self);
void xfce_overview_workspaces_focus_manager_set_focus(XfceOverviewWorkspacesFocusManager *self, XfceOverviewWorkspacesFocusable *inFocusable);

XfceOverviewWorkspacesFocusable* xfce_overview_workspaces_focus_manager_get_next_focusable(XfceOverviewWorkspacesFocusManager *self,
																	XfceOverviewWorkspacesFocusable *inBeginFocusable);
XfceOverviewWorkspacesFocusable* xfce_overview_workspaces_focus_manager_get_previous_focusable(XfceOverviewWorkspacesFocusManager *self,
																		XfceOverviewWorkspacesFocusable *inBeginFocusable);

gboolean xfce_overview_workspaces_focus_manager_get_event_targets_and_action(XfceOverviewWorkspacesFocusManager *self,
																const ClutterEvent *inEvent,
																XfceOverviewWorkspacesFocusable *inFocusable,
																GSList **outTargets,
																const gchar **outAction);
gboolean xfce_overview_workspaces_focus_manager_handle_key_event(XfceOverviewWorkspacesFocusManager *self,
													const ClutterEvent *inEvent,
													XfceOverviewWorkspacesFocusable *inFocusable);

G_END_DECLS

#endif	/* __LIBXFCE_OVERVIEW_WORKSPACES_FOCUS_MANAGER__ */
