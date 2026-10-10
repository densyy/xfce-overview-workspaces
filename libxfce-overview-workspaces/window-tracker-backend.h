/*
 * window-tracker-backend: Window tracker backend providing special functions
 *                         for different windowing and clutter backends.
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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_BACKEND__
#define __LIBXFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_BACKEND__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <glib-object.h>

#include <libxfce-overview-workspaces/window-tracker.h>

G_BEGIN_DECLS

/* Object declaration */
#define XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_TRACKER_BACKEND				(xfce_overview_workspaces_window_tracker_backend_get_type())
#define XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_BACKEND(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_TRACKER_BACKEND, XfceOverviewWorkspacesWindowTrackerBackend))
#define XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_BACKEND(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_TRACKER_BACKEND))
#define XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_BACKEND_GET_IFACE(obj)	(G_TYPE_INSTANCE_GET_INTERFACE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_TRACKER_BACKEND, XfceOverviewWorkspacesWindowTrackerBackendInterface))

typedef struct _XfceOverviewWorkspacesWindowTrackerBackend				XfceOverviewWorkspacesWindowTrackerBackend;
typedef struct _XfceOverviewWorkspacesWindowTrackerBackendInterface	XfceOverviewWorkspacesWindowTrackerBackendInterface;

/**
 * XfceOverviewWorkspacesWindowTrackerBackendInterface:
 * @get_name: Name of window tracker backend
 * @get_window_tracker: Get window tracker instance used by backend
 * @get_stage_from_window: Get stage using requested window
 * @show_stage_window: Setup and show stage window
 * @hide_stage_window: Hide (and maybe deconfigure) stage window
 */
struct _XfceOverviewWorkspacesWindowTrackerBackendInterface
{
	/*< private >*/
	/* Parent interface */
	GTypeInterface						parent_interface;

	/*< public >*/
	/* Virtual functions */
	const gchar* (*get_name)(XfceOverviewWorkspacesWindowTrackerBackend *self);

	XfceOverviewWorkspacesWindowTracker* (*get_window_tracker)(XfceOverviewWorkspacesWindowTrackerBackend *self);

	XfceOverviewWorkspacesWindowTrackerWindow* (*get_window_for_stage)(XfceOverviewWorkspacesWindowTrackerBackend *self,
															ClutterStage *inStage);
	ClutterStage* (*get_stage_from_window)(XfceOverviewWorkspacesWindowTrackerBackend *self,
											XfceOverviewWorkspacesWindowTrackerWindow *inWindow);
	void (*show_stage_window)(XfceOverviewWorkspacesWindowTrackerBackend *self,
								XfceOverviewWorkspacesWindowTrackerWindow *inWindow);
	void (*hide_stage_window)(XfceOverviewWorkspacesWindowTrackerBackend *self,
								XfceOverviewWorkspacesWindowTrackerWindow *inWindow);
};


/* Public API */
GType xfce_overview_workspaces_window_tracker_backend_get_type(void) G_GNUC_CONST;

void xfce_overview_workspaces_window_tracker_backend_set_backend(const gchar *inBackend);

XfceOverviewWorkspacesWindowTrackerBackend* xfce_overview_workspaces_window_tracker_backend_create(void);

const gchar* xfce_overview_workspaces_window_tracker_backend_get_name(XfceOverviewWorkspacesWindowTrackerBackend *self);

XfceOverviewWorkspacesWindowTracker* xfce_overview_workspaces_window_tracker_backend_get_window_tracker(XfceOverviewWorkspacesWindowTrackerBackend *self);

XfceOverviewWorkspacesWindowTrackerWindow* xfce_overview_workspaces_window_tracker_backend_get_window_for_stage(XfceOverviewWorkspacesWindowTrackerBackend *self,
																						ClutterStage *inStage);
ClutterStage* xfce_overview_workspaces_window_tracker_backend_get_stage_from_window(XfceOverviewWorkspacesWindowTrackerBackend *self,
																		XfceOverviewWorkspacesWindowTrackerWindow *inWindow);
void xfce_overview_workspaces_window_tracker_backend_show_stage_window(XfceOverviewWorkspacesWindowTrackerBackend *self,
															XfceOverviewWorkspacesWindowTrackerWindow *inWindow);
void xfce_overview_workspaces_window_tracker_backend_hide_stage_window(XfceOverviewWorkspacesWindowTrackerBackend *self,
															XfceOverviewWorkspacesWindowTrackerWindow *inWindow);

G_END_DECLS

#endif	/* __LIBXFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_BACKEND__ */
