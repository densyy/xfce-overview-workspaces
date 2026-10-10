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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <libxfce-overview-workspaces/window-tracker-backend.h>

#include <glib/gi18n-lib.h>

#include <libxfce-overview-workspaces/x11/window-tracker-backend-x11.h>
#ifdef HAVE_BACKEND_GDK
#include <libxfce-overview-workspaces/gdk/window-tracker-backend-gdk.h>
#endif
#include <libxfce-overview-workspaces/core.h>
#include <libxfce-overview-workspaces/marshal.h>
#include <libxfce-overview-workspaces/debug.h>


/* Define this class in GObject system */
G_DEFINE_INTERFACE(XfceOverviewWorkspacesWindowTrackerBackend,
					xfce_overview_workspaces_window_tracker_backend,
					G_TYPE_OBJECT)


/* IMPLEMENTATION: Private variables and methods */
#define XFCE_OVERVIEW_WORKSPACES_WINDOWS_TRACKER_BACKEND_WARN_NOT_IMPLEMENTED(self, vfunc)  \
	g_warning("Object of type %s does not implement required virtual function XfceOverviewWorkspacesWindowTrackerBackend::%s",\
				G_OBJECT_TYPE_NAME(self), \
				vfunc);

struct _XfceOverviewWorkspacesWindowTrackerBackendMap
{
	const gchar							*backendID;
	const gchar							*clutterBackendID;
	XfceOverviewWorkspacesWindowTrackerBackend*	(*createBackend)(void);
};
typedef struct _XfceOverviewWorkspacesWindowTrackerBackendMap	XfceOverviewWorkspacesWindowTrackerBackendMap;

static XfceOverviewWorkspacesWindowTrackerBackendMap	_xfce_overview_workspaces_window_tracker_backend_map[]=
											{
#ifdef CLUTTER_WINDOWING_X11
												{ "x11", CLUTTER_WINDOWING_X11, xfce_overview_workspaces_window_tracker_backend_x11_new },
#endif
#ifdef HAVE_BACKEND_GDK
#ifdef   CLUTTER_WINDOWING_GDK
												{ "gdk", CLUTTER_WINDOWING_GDK, xfce_overview_workspaces_window_tracker_backend_gdk_new },
#endif   /* CLUTTER_WINDOWING_GDK */
#endif /* HAVE_BACKEND_GDK */
												{ NULL, NULL, NULL }
											};


/* IMPLEMENTATION: GObject */

/* Interface initialization
 * Set up default functions
 */
void xfce_overview_workspaces_window_tracker_backend_default_init(XfceOverviewWorkspacesWindowTrackerBackendInterface *iface)
{
	static gboolean		initialized=FALSE;

	/* Define properties, signals and actions */
	if(!initialized)
	{
		/* Set flag that base initialization was done for this interface */
		initialized=TRUE;
	}
}


/* IMPLEMENTATION: Public API */

/**
 * xfce_overview_workspaces_window_tracker_backend_set_backend:
 * @inBackend: the backend to use
 *
 * Sets the backend that xfce-overview-workspaces should try to use. It will also restrict
 * the backend Clutter should try to use. By default xfce-overview-workspaces will select
 * the backend automatically based on the backend Clutter uses.
 *
 * For example:
 *
 * |[<!-- language="C" -->
 *   xfce_overview_workspaces_window_tracker_backend_set_backend("x11");
 * ]|
 *
 * Will make xfce-overview-workspaces and Clutter use the X11 backend.
 *
 * Possible backends are: x11 and gdk.
 *
 * This function must be called before the first API call to xfce-overview-workspaces or any
 * library xfce-overview-workspaces depends on like Clutter, GTK+ etc. This function can also
 * be called only once.
 */
void xfce_overview_workspaces_window_tracker_backend_set_backend(const gchar *inBackend)
{
	XfceOverviewWorkspacesWindowTrackerBackendMap	*iter;
	static gboolean						wasSet=FALSE;

	g_return_if_fail(inBackend && *inBackend);

	/* Warn if this function was called more than once */
	if(wasSet)
	{
		g_critical("Cannot set backend to '%s' because it the backend was already set",
					inBackend);
		return;
	}

	/* Set flag that this function was called regardless of the result of this
	 * function call.
	 */
	wasSet=TRUE;

	/* Backend can only be set if application was not already created */
	if(xfce_overview_workspaces_core_has_default())
	{
		g_critical("Cannot set backend to '%s' because application is already initialized",
					inBackend);
		return;
	}

	/* Iterate through list of available backends and lookup the requested
	 * backend. If this entry is found, restrict Clutter backend as listed in
	 * found entry and return.
	 */
	for(iter=_xfce_overview_workspaces_window_tracker_backend_map; iter->backendID; iter++)
	{
		/* If this entry does not match requested backend, try next one */
		if(g_strcmp0(iter->backendID, inBackend)!=0) continue;

		/* The entry matches so restrict allowed backends in Clutter to the one
		 * listed at this entry.
		 */
		clutter_set_windowing_backend(iter->clutterBackendID);

		return;
	}

	/* If we get here the requested backend is unknown */
	g_warning("Unknown backend '%s' - using default backend", inBackend);
}

/**
 * xfce_overview_workspaces_window_tracker_backend_create:
 *
 * Creates an instance of #XfceOverviewWorkspacesWindowTrackerBackend matching the clutter
 * windowing backend used. If no matching window tracker backend can be found
 * or created, %NULL will returned.
 *
 * Return value: (transfer full): The instance of #XfceOverviewWorkspacesWindowTrackerBackend
 *   or %NULL if no instance could be created
 */
XfceOverviewWorkspacesWindowTrackerBackend* xfce_overview_workspaces_window_tracker_backend_create(void)
{
	XfceOverviewWorkspacesWindowTrackerBackend		*backend;
	XfceOverviewWorkspacesWindowTrackerBackendMap	*iter;

	/* Iterate through list of available backends and check if any entry
	 * matches the backend Clutter is using. If we can find a matching entry
	 * then create our backend which interacts with Clutter's backend.
	 */
	backend=NULL;
	for(iter=_xfce_overview_workspaces_window_tracker_backend_map; !backend && iter->backendID; iter++)
	{
		/* If this entry does not match any backend at Clutter, try next one */
		if(!clutter_check_windowing_backend(iter->clutterBackendID)) continue;

		/* The entry matches so try to create our backend */
		XFCE_OVERVIEW_WORKSPACES_DEBUG(NULL, WINDOWS,
							"Found window tracker backend ID '%s' for clutter backend '%s'",
							iter->backendID,
							iter->clutterBackendID);

		backend=(iter->createBackend)();
		if(!backend)
		{
			XFCE_OVERVIEW_WORKSPACES_DEBUG(NULL, WINDOWS,
								"Could not create window tracker backend of ID '%s' for clutter backend '%s'",
								iter->backendID,
								iter->clutterBackendID);
		}
			else
			{
				XFCE_OVERVIEW_WORKSPACES_DEBUG(backend, WINDOWS,
									"Create window tracker backend of type %s with ID '%s' for clutter backend '%s'",
									G_OBJECT_TYPE_NAME(backend),
									iter->backendID,
									iter->clutterBackendID);
			}
	}

	/* Check if we could create any backend */
	if(!backend)
	{
		g_critical("Cannot find any usable window tracker backend");
		return(NULL);
	}

	/* Return created backend */
	return(backend);
}

/**
 * xfce_overview_workspaces_window_tracker_backend_get_name:
 * @self: A #XfceOverviewWorkspacesWindowTrackerBackend
 *
 * Retrieves the name of #XfceOverviewWorkspacesWindowTrackerBackend at @self.
 *
 * Return value: String containing the name of the window tracker backend.
 */
const gchar* xfce_overview_workspaces_window_tracker_backend_get_name(XfceOverviewWorkspacesWindowTrackerBackend *self)
{
	XfceOverviewWorkspacesWindowTrackerBackendInterface		*iface;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_BACKEND(self), NULL);

	iface=XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_BACKEND_GET_IFACE(self);

	/* Call virtual function */
	if(iface->get_name)
	{
		return(iface->get_name(self));
	}

	/* If we get here the virtual function was not overridden */
	XFCE_OVERVIEW_WORKSPACES_WINDOWS_TRACKER_BACKEND_WARN_NOT_IMPLEMENTED(self, "get_name");
	return(NULL);
}

/**
 * xfce_overview_workspaces_window_tracker_backend_get_window_tracker:
 * @self: A #XfceOverviewWorkspacesWindowTrackerBackend
 *
 * Retrieves the #XfceOverviewWorkspacesWindowTracker used by backend @self. If not needed
 * anymore the caller must unreference the returned object instance.
 *
 * Return value: (transfer full): The instance of #XfceOverviewWorkspacesWindowTracker.
 */
XfceOverviewWorkspacesWindowTracker* xfce_overview_workspaces_window_tracker_backend_get_window_tracker(XfceOverviewWorkspacesWindowTrackerBackend *self)
{
	XfceOverviewWorkspacesWindowTrackerBackendInterface		*iface;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_BACKEND(self), NULL);

	iface=XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_BACKEND_GET_IFACE(self);

	/* Call virtual function */
	if(iface->get_window_tracker)
	{
		XfceOverviewWorkspacesWindowTracker					*windowTracker;

		windowTracker=iface->get_window_tracker(self);
		if(windowTracker) g_object_ref(windowTracker);

		return(windowTracker);
	}

	/* If we get here the virtual function was not overridden */
	XFCE_OVERVIEW_WORKSPACES_WINDOWS_TRACKER_BACKEND_WARN_NOT_IMPLEMENTED(self, "get_window_tracker");
	return(NULL);
}

/**
 * xfce_overview_workspaces_window_tracker_backend_get_window_for_stage:
 * @self: A #XfceOverviewWorkspacesWindowTrackerBackend
 * @inStage: A #ClutterStage
 *
 * Retrieves the window created for the requested stage @inStage from window
 * tracker backend @self.
 *
 * Return value: (transfer none): The #XfceOverviewWorkspacesWindowTrackerWindow representing
 *   the window of requested stage or %NULL if not available. The returned object
 *   is owned by XfceOverviewWorkspaces and it should not be referenced or unreferenced.
 */
XfceOverviewWorkspacesWindowTrackerWindow* xfce_overview_workspaces_window_tracker_backend_get_window_for_stage(XfceOverviewWorkspacesWindowTrackerBackend *self,
																						ClutterStage *inStage)
{
	XfceOverviewWorkspacesWindowTrackerBackendInterface		*iface;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_BACKEND(self), NULL);

	iface=XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_BACKEND_GET_IFACE(self);

	/* Call virtual function */
	if(iface->get_window_for_stage)
	{
		return(iface->get_window_for_stage(self, inStage));
	}

	/* If we get here the virtual function was not overridden */
	XFCE_OVERVIEW_WORKSPACES_WINDOWS_TRACKER_BACKEND_WARN_NOT_IMPLEMENTED(self, "get_window_for_stage");
	return(NULL);
}

/**
 * xfce_overview_workspaces_window_tracker_backend_get_stage_from_window:
 * @self: A #XfceOverviewWorkspacesWindowTrackerBackend
 * @inWindow: A #XfceOverviewWorkspacesWindowTrackerWindow defining the stage window
 *
 * Asks the window tracker backend @self to find the #ClutterStage which uses
 * stage window @inWindow.
 *
 * Return value: (transfer none): The #ClutterStage for stage window @inWindow or
 *   %NULL if @inWindow is not a stage window or stage could not be found.
 */
ClutterStage* xfce_overview_workspaces_window_tracker_backend_get_stage_from_window(XfceOverviewWorkspacesWindowTrackerBackend *self,
																		XfceOverviewWorkspacesWindowTrackerWindow *inWindow)
{
	XfceOverviewWorkspacesWindowTrackerBackendInterface		*iface;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_BACKEND(self), NULL);
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_WINDOW(inWindow), NULL);

	iface=XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_BACKEND_GET_IFACE(self);

	/* Call virtual function */
	if(iface->get_stage_from_window)
	{
		return(iface->get_stage_from_window(self, inWindow));
	}

	/* If we get here the virtual function was not overridden */
	XFCE_OVERVIEW_WORKSPACES_WINDOWS_TRACKER_BACKEND_WARN_NOT_IMPLEMENTED(self, "get_stage_from_window");
	return(NULL);
}

/**
 * xfce_overview_workspaces_window_tracker_backend_show_stage_window:
 * @self: A #XfceOverviewWorkspacesWindowTrackerBackend
 * @inWindow: A #XfceOverviewWorkspacesWindowTrackerWindow defining the stage window
 *
 * Asks the window tracker backend @self to set up and show the window @inWindow
 * for use as stage window.
 */
void xfce_overview_workspaces_window_tracker_backend_show_stage_window(XfceOverviewWorkspacesWindowTrackerBackend *self,
															XfceOverviewWorkspacesWindowTrackerWindow *inWindow)
{
	XfceOverviewWorkspacesWindowTrackerBackendInterface		*iface;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_BACKEND(self));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_WINDOW(inWindow));

	iface=XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_BACKEND_GET_IFACE(self);

	/* Call virtual function */
	if(iface->show_stage_window)
	{
		iface->show_stage_window(self, inWindow);
		return;
	}

	/* If we get here the virtual function was not overridden */
	XFCE_OVERVIEW_WORKSPACES_WINDOWS_TRACKER_BACKEND_WARN_NOT_IMPLEMENTED(self, "show_stage_window");
}

/**
 * xfce_overview_workspaces_window_tracker_backend_hide_stage_window:
 * @self: A #XfceOverviewWorkspacesWindowTrackerBackend
 * @inWindow: A #XfceOverviewWorkspacesWindowTrackerWindow defining the stage window
 *
 * Asks the window tracker backend @self to hide the stage window @inWindow.
 */
void xfce_overview_workspaces_window_tracker_backend_hide_stage_window(XfceOverviewWorkspacesWindowTrackerBackend *self,
															XfceOverviewWorkspacesWindowTrackerWindow *inWindow)
{
	XfceOverviewWorkspacesWindowTrackerBackendInterface		*iface;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_BACKEND(self));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_WINDOW(inWindow));

	iface=XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_BACKEND_GET_IFACE(self);

	/* Call virtual function */
	if(iface->hide_stage_window)
	{
		iface->hide_stage_window(self, inWindow);
		return;
	}

	/* If we get here the virtual function was not overridden */
	XFCE_OVERVIEW_WORKSPACES_WINDOWS_TRACKER_BACKEND_WARN_NOT_IMPLEMENTED(self, "hide_stage_window");
}
