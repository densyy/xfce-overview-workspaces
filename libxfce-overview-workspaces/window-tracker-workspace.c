/*
 * window-tracker-workspace: A workspace tracked by window tracker.
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

#include <libxfce-overview-workspaces/window-tracker-workspace.h>

#include <glib/gi18n-lib.h>

#include <libxfce-overview-workspaces/window-tracker.h>
#include <libxfce-overview-workspaces/marshal.h>


/* Define this class in GObject system */
G_DEFINE_INTERFACE(XfceOverviewWorkspacesWindowTrackerWorkspace,
					xfce_overview_workspaces_window_tracker_workspace,
					G_TYPE_OBJECT)


/* Signals */
enum
{
	SIGNAL_NAME_CHANGED,

	SIGNAL_LAST
};

static guint XfceOverviewWorkspacesWindowTrackerWorkspaceSignals[SIGNAL_LAST]={ 0, };


/* IMPLEMENTATION: Private variables and methods */
#define XFCE_OVERVIEW_WORKSPACES_WINDOWS_TRACKER_WORKSPACE_WARN_NOT_IMPLEMENTED(self, vfunc)\
	g_warning("Object of type %s does not implement required virtual function XfceOverviewWorkspacesWindowTrackerWorkspace::%s",\
				G_OBJECT_TYPE_NAME(self), \
				vfunc);

/* Default implementation of virtual function "is_equal" */
static gboolean _xfce_overview_workspaces_window_tracker_workspace_real_is_equal(XfceOverviewWorkspacesWindowTrackerWorkspace *inLeft,
																	XfceOverviewWorkspacesWindowTrackerWorkspace *inRight)
{
	gint			leftIndex, rightIndex;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_WORKSPACE(inLeft), FALSE);
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_WORKSPACE(inRight), FALSE);

	/* Check if both are the same workspace or refer to same one */
	leftIndex=xfce_overview_workspaces_window_tracker_workspace_get_number(inLeft);
	rightIndex=xfce_overview_workspaces_window_tracker_workspace_get_number(inRight);
	if(inLeft==inRight || leftIndex==rightIndex) return(TRUE);

	/* If we get here then they cannot be considered equal */
	return(FALSE);
}


/* IMPLEMENTATION: GObject */

/* Interface initialization
 * Set up default functions
 */
static void xfce_overview_workspaces_window_tracker_workspace_default_init(XfceOverviewWorkspacesWindowTrackerWorkspaceInterface *iface)
{
	static gboolean		initialized=FALSE;

	/* The following virtual functions should be overriden if default
	 * implementation does not fit.
	 */
	iface->is_equal=_xfce_overview_workspaces_window_tracker_workspace_real_is_equal;

	/* Define signals */
	if(!initialized)
	{
		XfceOverviewWorkspacesWindowTrackerWorkspaceSignals[SIGNAL_NAME_CHANGED]=
			g_signal_new("name-changed",
							G_TYPE_FROM_INTERFACE(iface),
							G_SIGNAL_RUN_LAST,
							G_STRUCT_OFFSET(XfceOverviewWorkspacesWindowTrackerWorkspaceInterface, name_changed),
							NULL,
							NULL,
							g_cclosure_marshal_VOID__VOID,
							G_TYPE_NONE,
							0);

		/* Set flag that base initialization was done for this interface */
		initialized=TRUE;
	}
}


/* IMPLEMENTATION: Public API */

/* Check if both workspaces are the same */
gboolean xfce_overview_workspaces_window_tracker_workspace_is_equal(XfceOverviewWorkspacesWindowTrackerWorkspace *inLeft,
														XfceOverviewWorkspacesWindowTrackerWorkspace *inRight)
{
	XfceOverviewWorkspacesWindowTrackerWorkspaceInterface		*iface;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_WORKSPACE(inLeft), FALSE);
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_WORKSPACE(inRight), FALSE);

	iface=XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WORKSPACE_GET_IFACE(inLeft);

	/* Call virtual function */
	if(iface->is_equal)
	{
		return(iface->is_equal(inLeft, inRight));
	}

	/* If we get here the virtual function was not overridden */
	XFCE_OVERVIEW_WORKSPACES_WINDOWS_TRACKER_WORKSPACE_WARN_NOT_IMPLEMENTED(inLeft, "is_equal");
	return(FALSE);
}

/* Get number of workspace */
gint xfce_overview_workspaces_window_tracker_workspace_get_number(XfceOverviewWorkspacesWindowTrackerWorkspace *self)
{
	XfceOverviewWorkspacesWindowTrackerWorkspaceInterface		*iface;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_WORKSPACE(self), 0);

	iface=XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WORKSPACE_GET_IFACE(self);

	/* Call virtual function */
	if(iface->get_number)
	{
		return(iface->get_number(self));
	}

	/* If we get here the virtual function was not overridden */
	XFCE_OVERVIEW_WORKSPACES_WINDOWS_TRACKER_WORKSPACE_WARN_NOT_IMPLEMENTED(self, "get_number");
	return(0);
}

/* Get name of workspace */
const gchar* xfce_overview_workspaces_window_tracker_workspace_get_name(XfceOverviewWorkspacesWindowTrackerWorkspace *self)
{
	XfceOverviewWorkspacesWindowTrackerWorkspaceInterface		*iface;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_WORKSPACE(self), NULL);

	iface=XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WORKSPACE_GET_IFACE(self);

	/* Call virtual function */
	if(iface->get_name)
	{
		return(iface->get_name(self));
	}

	/* If we get here the virtual function was not overridden */
	XFCE_OVERVIEW_WORKSPACES_WINDOWS_TRACKER_WORKSPACE_WARN_NOT_IMPLEMENTED(self, "get_name");
	return(NULL);
}

/* Get size of workspace */
void xfce_overview_workspaces_window_tracker_workspace_get_size(XfceOverviewWorkspacesWindowTrackerWorkspace *self,
													gint *outWidth,
													gint *outHeight)
{
	XfceOverviewWorkspacesWindowTrackerWorkspaceInterface		*iface;
	gint											width, height;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_WORKSPACE(self));

	iface=XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WORKSPACE_GET_IFACE(self);

	/* Call virtual function */
	if(iface->get_size)
	{
		/* Get geometry of workspace */
		iface->get_size(self, &width, &height);

		/* Store result */
		if(outWidth) *outWidth=width;
		if(outHeight) *outHeight=height;

		return;
	}

	/* If we get here the virtual function was not overridden */
	XFCE_OVERVIEW_WORKSPACES_WINDOWS_TRACKER_WORKSPACE_WARN_NOT_IMPLEMENTED(self, "get_size");
}

/* Determine if requested workspace is the active one */
gint xfce_overview_workspaces_window_tracker_workspace_is_active(XfceOverviewWorkspacesWindowTrackerWorkspace *self)
{
	XfceOverviewWorkspacesWindowTrackerWorkspaceInterface		*iface;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_WORKSPACE(self), FALSE);

	iface=XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WORKSPACE_GET_IFACE(self);

	/* Call virtual function */
	if(iface->is_active)
	{
		return(iface->is_active(self));
	}

	/* If we get here the virtual function was not overridden */
	XFCE_OVERVIEW_WORKSPACES_WINDOWS_TRACKER_WORKSPACE_WARN_NOT_IMPLEMENTED(self, "is_active");
	return(FALSE);
}

/* Activate workspace */
void xfce_overview_workspaces_window_tracker_workspace_activate(XfceOverviewWorkspacesWindowTrackerWorkspace *self)
{
	XfceOverviewWorkspacesWindowTrackerWorkspaceInterface		*iface;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_WORKSPACE(self));

	iface=XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WORKSPACE_GET_IFACE(self);

	/* Call virtual function */
	if(iface->activate)
	{
		iface->activate(self);
		return;
	}

	/* If we get here the virtual function was not overridden */
	XFCE_OVERVIEW_WORKSPACES_WINDOWS_TRACKER_WORKSPACE_WARN_NOT_IMPLEMENTED(self, "activate");
}
