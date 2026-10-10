/*
 * windows-view: A view showing visible windows
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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_WINDOWS_VIEW__
#define __LIBXFCE_OVERVIEW_WORKSPACES_WINDOWS_VIEW__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <libxfce-overview-workspaces/view.h>
#include <libxfce-overview-workspaces/focusable.h>

G_BEGIN_DECLS

#define XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOWS_VIEW				(xfce_overview_workspaces_windows_view_get_type())
#define XFCE_OVERVIEW_WORKSPACES_WINDOWS_VIEW(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOWS_VIEW, XfceOverviewWorkspacesWindowsView))
#define XFCE_OVERVIEW_WORKSPACES_IS_WINDOWS_VIEW(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOWS_VIEW))
#define XFCE_OVERVIEW_WORKSPACES_WINDOWS_VIEW_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOWS_VIEW, XfceOverviewWorkspacesWindowsViewClass))
#define XFCE_OVERVIEW_WORKSPACES_IS_WINDOWS_VIEW_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOWS_VIEW))
#define XFCE_OVERVIEW_WORKSPACES_WINDOWS_VIEW_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOWS_VIEW, XfceOverviewWorkspacesWindowsViewClass))

typedef struct _XfceOverviewWorkspacesWindowsView				XfceOverviewWorkspacesWindowsView; 
typedef struct _XfceOverviewWorkspacesWindowsViewPrivate		XfceOverviewWorkspacesWindowsViewPrivate;
typedef struct _XfceOverviewWorkspacesWindowsViewClass			XfceOverviewWorkspacesWindowsViewClass;

struct _XfceOverviewWorkspacesWindowsView
{
	/*< private >*/
	/* Parent instance */
	XfceOverviewWorkspacesView					parent_instance;

	/* Private structure */
	XfceOverviewWorkspacesWindowsViewPrivate	*priv;
};

struct _XfceOverviewWorkspacesWindowsViewClass
{
	/*< private >*/
	/* Parent class */
	XfceOverviewWorkspacesViewClass			parent_class;

	/*< public >*/
	/* Virtual functions */

	/* Binding actions */
	gboolean (*window_close)(XfceOverviewWorkspacesWindowsView *self,
								XfceOverviewWorkspacesFocusable *inSource,
								const gchar *inAction,
								ClutterEvent *inEvent);
	gboolean (*windows_show_numbers)(XfceOverviewWorkspacesWindowsView *self,
										XfceOverviewWorkspacesFocusable *inSource,
										const gchar *inAction,
										ClutterEvent *inEvent);
	gboolean (*windows_hide_numbers)(XfceOverviewWorkspacesWindowsView *self,
										XfceOverviewWorkspacesFocusable *inSource,
										const gchar *inAction,
										ClutterEvent *inEvent);
	gboolean (*windows_activate_window_one)(XfceOverviewWorkspacesWindowsView *self,
												XfceOverviewWorkspacesFocusable *inSource,
												const gchar *inAction,
												ClutterEvent *inEvent);
	gboolean (*windows_activate_window_two)(XfceOverviewWorkspacesWindowsView *self,
												XfceOverviewWorkspacesFocusable *inSource,
												const gchar *inAction,
												ClutterEvent *inEvent);
	gboolean (*windows_activate_window_three)(XfceOverviewWorkspacesWindowsView *self,
												XfceOverviewWorkspacesFocusable *inSource,
												const gchar *inAction,
												ClutterEvent *inEvent);
	gboolean (*windows_activate_window_four)(XfceOverviewWorkspacesWindowsView *self,
												XfceOverviewWorkspacesFocusable *inSource,
												const gchar *inAction,
												ClutterEvent *inEvent);
	gboolean (*windows_activate_window_five)(XfceOverviewWorkspacesWindowsView *self,
												XfceOverviewWorkspacesFocusable *inSource,
												const gchar *inAction,
												ClutterEvent *inEvent);
	gboolean (*windows_activate_window_six)(XfceOverviewWorkspacesWindowsView *self,
												XfceOverviewWorkspacesFocusable *inSource,
												const gchar *inAction,
												ClutterEvent *inEvent);
	gboolean (*windows_activate_window_seven)(XfceOverviewWorkspacesWindowsView *self,
												XfceOverviewWorkspacesFocusable *inSource,
												const gchar *inAction,
												ClutterEvent *inEvent);
	gboolean (*windows_activate_window_eight)(XfceOverviewWorkspacesWindowsView *self,
												XfceOverviewWorkspacesFocusable *inSource,
												const gchar *inAction,
												ClutterEvent *inEvent);
	gboolean (*windows_activate_window_nine)(XfceOverviewWorkspacesWindowsView *self,
												XfceOverviewWorkspacesFocusable *inSource,
												const gchar *inAction,
												ClutterEvent *inEvent);
	gboolean (*windows_activate_window_ten)(XfceOverviewWorkspacesWindowsView *self,
												XfceOverviewWorkspacesFocusable *inSource,
												const gchar *inAction,
												ClutterEvent *inEvent);

};

/* Public API */
GType xfce_overview_workspaces_windows_view_get_type(void) G_GNUC_CONST;

gfloat xfce_overview_workspaces_windows_view_get_spacing(XfceOverviewWorkspacesWindowsView *self);
void xfce_overview_workspaces_windows_view_set_spacing(XfceOverviewWorkspacesWindowsView *self, const gfloat inSpacing);

gboolean xfce_overview_workspaces_windows_view_get_prevent_upscaling(XfceOverviewWorkspacesWindowsView *self);
void xfce_overview_workspaces_windows_view_set_prevent_upscaling(XfceOverviewWorkspacesWindowsView *self, gboolean inPreventUpscaling);

G_END_DECLS

#endif	/* __LIBXFCE_OVERVIEW_WORKSPACES_WINDOWS_VIEW__ */
