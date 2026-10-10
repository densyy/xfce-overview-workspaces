/*
 * application-tracker: A singleton managing states of applications
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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_APPLICATION_TRACKER__
#define __LIBXFCE_OVERVIEW_WORKSPACES_APPLICATION_TRACKER__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <glib-object.h>
#include <gio/gio.h>

G_BEGIN_DECLS

#define XFCE_OVERVIEW_WORKSPACES_TYPE_APPLICATION_TRACKER				(xfce_overview_workspaces_application_tracker_get_type())
#define XFCE_OVERVIEW_WORKSPACES_APPLICATION_TRACKER(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_APPLICATION_TRACKER, XfceOverviewWorkspacesApplicationTracker))
#define XFCE_OVERVIEW_WORKSPACES_IS_APPLICATION_TRACKER(obj)				(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_APPLICATION_TRACKER))
#define XFCE_OVERVIEW_WORKSPACES_APPLICATION_TRACKER_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_APPLICATION_TRACKER, XfceOverviewWorkspacesApplicationTrackerClass))
#define XFCE_OVERVIEW_WORKSPACES_IS_APPLICATION_TRACKER_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_APPLICATION_TRACKER))
#define XFCE_OVERVIEW_WORKSPACES_APPLICATION_TRACKER_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_APPLICATION_TRACKER, XfceOverviewWorkspacesApplicationTrackerClass))

typedef struct _XfceOverviewWorkspacesApplicationTracker				XfceOverviewWorkspacesApplicationTracker;
typedef struct _XfceOverviewWorkspacesApplicationTrackerClass			XfceOverviewWorkspacesApplicationTrackerClass;
typedef struct _XfceOverviewWorkspacesApplicationTrackerPrivate		XfceOverviewWorkspacesApplicationTrackerPrivate;

struct _XfceOverviewWorkspacesApplicationTracker
{
	/*< private >*/
	/* Parent instance */
	GObject									parent_instance;

	/* Private structure */
	XfceOverviewWorkspacesApplicationTrackerPrivate	*priv;
};

struct _XfceOverviewWorkspacesApplicationTrackerClass
{
	/*< private >*/
	/* Parent class */
	GObjectClass							parent_class;

	/*< public >*/
	/* Virtual functions */
	void (*state_changed)(XfceOverviewWorkspacesApplicationTracker *self, const gchar *inDesktopID, gboolean inRunning);
};

/* Public API */
GType xfce_overview_workspaces_application_tracker_get_type(void) G_GNUC_CONST;

gboolean xfce_overview_workspaces_application_tracker_is_running_by_desktop_id(XfceOverviewWorkspacesApplicationTracker *self,
																	const gchar *inDesktopID);
gboolean xfce_overview_workspaces_application_tracker_is_running_by_app_info(XfceOverviewWorkspacesApplicationTracker *self,
																GAppInfo *inAppInfo);

const GList* xfce_overview_workspaces_application_tracker_get_window_list_by_desktop_id(XfceOverviewWorkspacesApplicationTracker *self,
																			const gchar *inDesktopID);
const GList*  xfce_overview_workspaces_application_tracker_get_window_list_by_app_info(XfceOverviewWorkspacesApplicationTracker *self,
																			GAppInfo *inAppInfo);

G_END_DECLS

#endif	/* __LIBXFCE_OVERVIEW_WORKSPACES_APPLICATION_TRACKER__ */
