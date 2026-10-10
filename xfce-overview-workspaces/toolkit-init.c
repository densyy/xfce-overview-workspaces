/*
 * toolkit-init: Toolkit initialization for the primary instance
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

#include "toolkit-init.h"

#include <gtk/gtk.h>
#include <clutter/clutter.h>
#include <xfconf/xfconf.h>


/**
 * xfce_overview_workspaces_init_toolkit_primary:
 * @ioArgc: Pointer to argument count (may be modified by toolkit init)
 * @ioArgv: Pointer to argument vector (may be modified by toolkit init)
 *
 * Initialize toolkit for primary instance (xfconf + GTK + Clutter).
 * Must be called exactly once in a process that will become primary.
 * Remote instances skip this for fast-path; a remote-turned-primary after
 * xfce_overview_workspaces_restart() must call it before the second g_application_run().
 *
 * Returns: %TRUE on success, %FALSE on fatal error (xfconf/clutter).
 */
gboolean xfce_overview_workspaces_init_toolkit_primary(int *ioArgc, char ***ioArgv)
{
	GError		*error=NULL;

	if(G_UNLIKELY(!xfconf_init(&error)))
	{
		if(G_LIKELY(error))
		{
			g_error("Failed to initialize xfconf: %s",
						error ? error->message : "Unknown error");
			if(error) g_error_free(error);
		}

		return(FALSE);
	}

	gtk_init(ioArgc, ioArgv);
	if(!clutter_init(ioArgc, ioArgv))
	{
		g_error("Initializing clutter failed!");
		return(FALSE);
	}

	gdk_notify_startup_complete();

	return(TRUE);
}
