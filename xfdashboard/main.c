/*
 * main: Common functions, shared data and main entry point of application
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

#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>
#include <clutter/clutter.h>
#ifdef CLUTTER_WINDOWING_X11
#include <clutter/x11/clutter-x11.h>
#endif
#include <libxfce4util/libxfce4util.h>
#include <libxfdashboard/window-tracker-backend.h>

#include "application.h"
#include "restart.h"
#include "toolkit-init.h"


/* Main entry point */
int main(int argc, char **argv)
{
	XfdashboardApplication		*app=NULL;
	gint						status;
	const gchar					*backend;
	gboolean					isRemote=FALSE;

	/* Set up localization */
	xfce_textdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR, "UTF-8");

	/* Enforce X11 backend in Clutter if no specific backend was requested via
	 * the XFDASHBOARD_BACKEND environment variable. If this environment variable
	 * is set, enforce this backend.
	 *
	 * This function must be called before any API function call at libxfdashboard
	 * or other library API function like the one of Clutter, GTK+ etc.
	 * It is cheap (no display round-trip) so it stays on both paths.
	 */
	backend=g_getenv("XFDASHBOARD_BACKEND");
	if(backend)
	{
		xfdashboard_window_tracker_backend_set_backend(backend);
		g_debug("Setting backend to '%s'", backend);
	}
		else
		{
			clutter_set_windowing_backend("x11");
			g_debug("Enforcing X11 backend");
		}

#ifdef CLUTTER_WINDOWING_X11
	clutter_x11_set_use_argb_visual(TRUE);
#endif

	/* Create application instance early and register on DBUS _before_ any
	 * heavy toolkit initialization. g_application_register() is idempotent,
	 * so the later g_application_run() re-registers cheaply from cache.
	 * A remote instance (e.g. '-t'/'--toggle' or '-q'/'--quit' while the
	 * daemon runs) only forwards its arguments to the primary instance and
	 * never needs xfconf, GTK+, Clutter or core initialization, so all of
	 * that is skipped on the remote path below.
	 */
	app=xfdashboard_application_new();
	if(!app)
	{
		g_warning("Failed to create application instance");
		return(XFDASHBOARD_APPLICATION_ERROR_FAILED);
	}

	if(g_application_register(G_APPLICATION(app), NULL, NULL))
	{
		isRemote=g_application_get_is_remote(G_APPLICATION(app));
	}
	/* If registration failed (e.g. no session bus) keep isRemote FALSE so the
	 * normal path runs and reports the error in local_command_line. The extra
	 * failed round-trip here is negligible next to process start-up.
	 */
	if(isRemote) g_debug("Remote instance detected - skipping local toolkit initialization");
		else
		{
			if(!xfdashboard_init_toolkit_primary(&argc, &argv)) return(1);
		}

	status=g_application_run(G_APPLICATION(app), argc, argv);
	/* NB: do NOT call g_application_get_is_remote() here: after g_application_run()
	 * returns, a remote instance is no longer registered (GLib unregisters the
	 * transient remote), so get_is_remote() would warn with
	 * "assertion 'application->priv->is_registered' failed" and return FALSE,
	 * skipping the restart and leaving no instance running. Use the cached
	 * isRemote from before run instead.
	 */
	if(status==XFDASHBOARD_APPLICATION_ERROR_RESTART &&
		isRemote)
	{
		if(xfdashboard_restart(app))
		{
			g_debug("Reached clean state to restart application");

			g_object_unref(app);
			app=NULL;

			app=xfdashboard_application_new();
			if(!app)
			{
				g_warning("Failed to create application instance");
				return(XFDASHBOARD_APPLICATION_ERROR_FAILED);
			}

			g_debug("Starting new primary instance");

			/* This process was a remote instance and skipped toolkit init for
			 * fast-path. It must initialize now before becoming primary,
			 * otherwise core/stage init will crash (no GDK display, no xfconf).
			 */
			if(!xfdashboard_init_toolkit_primary(&argc, &argv))
			{
				g_object_unref(app);
				return(XFDASHBOARD_APPLICATION_ERROR_FAILED);
			}

			isRemote=FALSE;
			status=g_application_run(G_APPLICATION(app), argc, argv);
		}
			else
			{
				g_warning("Could not restart application because existing instance seems still to be running.");
			}
	}

	g_object_unref(app);

	return(status);
}
