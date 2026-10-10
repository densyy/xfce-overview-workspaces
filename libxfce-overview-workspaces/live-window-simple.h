/*
 * live-window-simple: An actor showing the content of a window which will
 *                     be updated if changed and visible on active workspace.
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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_LIVE_WINDOW_SIMPLE__
#define __LIBXFCE_OVERVIEW_WORKSPACES_LIVE_WINDOW_SIMPLE__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <clutter/clutter.h>

#include <libxfce-overview-workspaces/background.h>
#include <libxfce-overview-workspaces/window-tracker.h>

G_BEGIN_DECLS

/* Public definitions */
/**
 * XfceOverviewWorkspacesLiveWindowSimpleDisplayType:
 * @XFCE_OVERVIEW_WORKSPACES_LIVE_WINDOW_SIMPLE_DISPLAY_TYPE_LIVE_PREVIEW: The actor will show a live preview of window
 * @XFCE_OVERVIEW_WORKSPACES_LIVE_WINDOW_SIMPLE_DISPLAY_TYPE_ICON: The actor will show the window's icon in size of window
 *
 * Determines how the window will be displayed.
 */
typedef enum /*< prefix=XFCE_OVERVIEW_WORKSPACES_LIVE_WINDOW_SIMPLE_DISPLAY_TYPE >*/
{
	XFCE_OVERVIEW_WORKSPACES_LIVE_WINDOW_SIMPLE_DISPLAY_TYPE_LIVE_PREVIEW=0,
	XFCE_OVERVIEW_WORKSPACES_LIVE_WINDOW_SIMPLE_DISPLAY_TYPE_ICON,
} XfceOverviewWorkspacesLiveWindowSimpleDisplayType;


/* Object declaration */
#define XFCE_OVERVIEW_WORKSPACES_TYPE_LIVE_WINDOW_SIMPLE				(xfce_overview_workspaces_live_window_simple_get_type())
#define XFCE_OVERVIEW_WORKSPACES_LIVE_WINDOW_SIMPLE(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_LIVE_WINDOW_SIMPLE, XfceOverviewWorkspacesLiveWindowSimple))
#define XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WINDOW_SIMPLE(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_LIVE_WINDOW_SIMPLE))
#define XFCE_OVERVIEW_WORKSPACES_LIVE_WINDOW_SIMPLE_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_LIVE_WINDOW_SIMPLE, XfceOverviewWorkspacesLiveWindowSimpleClass))
#define XFCE_OVERVIEW_WORKSPACES_IS_LIVE_WINDOW_SIMPLE_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_LIVE_WINDOW_SIMPLE))
#define XFCE_OVERVIEW_WORKSPACES_LIVE_WINDOW_SIMPLE_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_LIVE_WINDOW_SIMPLE, XfceOverviewWorkspacesLiveWindowSimpleClass))

typedef struct _XfceOverviewWorkspacesLiveWindowSimple				XfceOverviewWorkspacesLiveWindowSimple;
typedef struct _XfceOverviewWorkspacesLiveWindowSimpleClass		XfceOverviewWorkspacesLiveWindowSimpleClass;
typedef struct _XfceOverviewWorkspacesLiveWindowSimplePrivate		XfceOverviewWorkspacesLiveWindowSimplePrivate;

struct _XfceOverviewWorkspacesLiveWindowSimple
{
	/*< private >*/
	/* Parent instance */
	XfceOverviewWorkspacesBackground			parent_instance;

	/* Private structure */
	XfceOverviewWorkspacesLiveWindowSimplePrivate	*priv;
};

struct _XfceOverviewWorkspacesLiveWindowSimpleClass
{
	/*< private >*/
	/* Parent class */
	XfceOverviewWorkspacesBackgroundClass		parent_class;

	/*< public >*/
	/* Virtual functions */
	void (*geometry_changed)(XfceOverviewWorkspacesLiveWindowSimple *self);
	void (*visibility_changed)(XfceOverviewWorkspacesLiveWindowSimple *self, gboolean inVisible);
	void (*workspace_changed)(XfceOverviewWorkspacesLiveWindowSimple *self);
};

/* Public API */
GType xfce_overview_workspaces_live_window_simple_get_type(void) G_GNUC_CONST;

ClutterActor* xfce_overview_workspaces_live_window_simple_new(void);
ClutterActor* xfce_overview_workspaces_live_window_simple_new_for_window(XfceOverviewWorkspacesWindowTrackerWindow *inWindow);

XfceOverviewWorkspacesWindowTrackerWindow* xfce_overview_workspaces_live_window_simple_get_window(XfceOverviewWorkspacesLiveWindowSimple *self);
void xfce_overview_workspaces_live_window_simple_set_window(XfceOverviewWorkspacesLiveWindowSimple *self, XfceOverviewWorkspacesWindowTrackerWindow *inWindow);

XfceOverviewWorkspacesLiveWindowSimpleDisplayType xfce_overview_workspaces_live_window_simple_get_display_type(XfceOverviewWorkspacesLiveWindowSimple *self);
void xfce_overview_workspaces_live_window_simple_set_display_type(XfceOverviewWorkspacesLiveWindowSimple *self, XfceOverviewWorkspacesLiveWindowSimpleDisplayType inType);

gboolean xfce_overview_workspaces_live_window_simple_get_destroy_on_close(XfceOverviewWorkspacesLiveWindowSimple *self);
void xfce_overview_workspaces_live_window_simple_set_destroy_on_close(XfceOverviewWorkspacesLiveWindowSimple *self, gboolean inDestroyOnClose);

G_END_DECLS

#endif	/* __LIBXFCE_OVERVIEW_WORKSPACES_LIVE_WINDOW_SIMPLE__ */
