/*
 * live-window: An actor showing the content of a window which will
 *              be updated if changed and visible on active workspace.
 *              It also provides controls to manipulate it.
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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_LIVE_WINDOW__
#define __LIBXFCE_OVERVIEW_WORKSPACES_LIVE_WINDOW__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <clutter/clutter.h>

#include <libxfce-overview-workspaces/live-window-simple.h>
#include <libxfce-overview-workspaces/window-tracker.h>

G_BEGIN_DECLS

#define XFCE_OVERVIEW_WORKSPACES_TYPE_LIVE_WINDOW				(xfce_overview_workspaces_live_window_get_type())
#define XFCE_OVERVIEW_WORKSPACES_LIVE_WINDOW(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_LIVE_WINDOW, XfceOverviewWorkspacesLiveWindow))
#define XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WINDOW(obj)				(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_LIVE_WINDOW))
#define XFCE_OVERVIEW_WORKSPACES_LIVE_WINDOW_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_LIVE_WINDOW, XfceOverviewWorkspacesLiveWindowClass))
#define XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WINDOW_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_LIVE_WINDOW))
#define XFCE_OVERVIEW_WORKSPACES_LIVE_WINDOW_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_LIVE_WINDOW, XfceOverviewWorkspacesLiveWindowClass))

typedef struct _XfceOverviewWorkspacesLiveWindow				XfceOverviewWorkspacesLiveWindow;
typedef struct _XfceOverviewWorkspacesLiveWindowClass			XfceOverviewWorkspacesLiveWindowClass;
typedef struct _XfceOverviewWorkspacesLiveWindowPrivate		XfceOverviewWorkspacesLiveWindowPrivate;

struct _XfceOverviewWorkspacesLiveWindow
{
	/*< private >*/
	/* Parent instance */
	XfceOverviewWorkspacesLiveWindowSimple			parent_instance;

	/* Private structure */
	XfceOverviewWorkspacesLiveWindowPrivate		*priv;
};

struct _XfceOverviewWorkspacesLiveWindowClass
{
	/*< private >*/
	/* Parent class */
	XfceOverviewWorkspacesLiveWindowSimpleClass	parent_class;

	/*< public >*/
	/* Virtual functions */
	void (*clicked)(XfceOverviewWorkspacesLiveWindow *self);
	void (*close)(XfceOverviewWorkspacesLiveWindow *self);
};

/* Public API */
GType xfce_overview_workspaces_live_window_get_type(void) G_GNUC_CONST;

ClutterActor* xfce_overview_workspaces_live_window_new(void);
ClutterActor* xfce_overview_workspaces_live_window_new_for_window(XfceOverviewWorkspacesWindowTrackerWindow *inWindow);

gfloat xfce_overview_workspaces_live_window_get_title_actor_padding(XfceOverviewWorkspacesLiveWindow *self);
void xfce_overview_workspaces_live_window_set_title_actor_padding(XfceOverviewWorkspacesLiveWindow *self, gfloat inPadding);

gfloat xfce_overview_workspaces_live_window_get_close_button_padding(XfceOverviewWorkspacesLiveWindow *self);
void xfce_overview_workspaces_live_window_set_close_button_padding(XfceOverviewWorkspacesLiveWindow *self, gfloat inPadding);

gboolean xfce_overview_workspaces_live_window_get_show_subwindows(XfceOverviewWorkspacesLiveWindow *self);
void xfce_overview_workspaces_live_window_set_show_subwindows(XfceOverviewWorkspacesLiveWindow *self, gboolean inShowSubwindows);

gboolean xfce_overview_workspaces_live_window_get_allow_subwindows(XfceOverviewWorkspacesLiveWindow *self);
void xfce_overview_workspaces_live_window_set_allow_subwindows(XfceOverviewWorkspacesLiveWindow *self, gboolean inAllowSubwindows);

G_END_DECLS

#endif	/* __LIBXFCE_OVERVIEW_WORKSPACES_LIVE_WINDOW__ */
