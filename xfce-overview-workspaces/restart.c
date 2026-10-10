/*
 * restart: Quit running primary instance via DBus and wait for it to vanish
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

#include "restart.h"

#include <gio/gio.h>


#define DEFAULT_RESTART_WAIT_TIMEOUT		2000	/* ms to wait for a running
													 * instance to quit on restart.
													 * Local quit via DBus is well
													 * below 1s; 2s keeps the failure
													 * path fast without cutting a
													 * slow-but-healthy shutdown.
													 */

typedef struct _RestartData		RestartData;
struct _RestartData
{
	GMainLoop				*mainLoop;
	XfceOverviewWorkspacesApplication	*application;
	gboolean				appHasQuitted;
};


static gboolean _on_quit_timeout(gpointer inUserData)
{
	RestartData			*restartData;

	g_return_val_if_fail(inUserData, G_SOURCE_REMOVE);

	restartData=(RestartData*)inUserData;
	restartData->appHasQuitted=FALSE;

	g_main_loop_quit(restartData->mainLoop);
	g_debug("Timeout for waiting the application has been reached!");

	return(G_SOURCE_REMOVE);
}

static gboolean _on_quit_idle(gpointer inUserData)
{
	RestartData			*restartData=(RestartData*)inUserData;

	/* Quit primary instance via DBUS action. restartData is stack-allocated
	 * in xfce_overview_workspaces_restart() and stays alive while its main loop runs, so this is safe.
	 */
	if(restartData && restartData->application)
	{
		xfce_overview_workspaces_application_quit_forced(restartData->application);
	}
		else
		{
			g_warning("Cannot quit running instance for restart: missing application");
		}

	return(G_SOURCE_REMOVE);
}

static void _on_dbus_watcher_name_appeared(GDBusConnection *inConnection,
											const gchar *inName,
											const gchar *inOwner,
											gpointer inUserData)
{
	RestartData			*restartData;
	GSource				*quitIdleSource;

	g_return_if_fail(inUserData);

	restartData=(RestartData*)inUserData;

	quitIdleSource=g_idle_source_new();
	g_source_set_callback(quitIdleSource, _on_quit_idle, restartData, NULL);
	g_source_attach(quitIdleSource, g_main_loop_get_context(restartData->mainLoop));
	g_source_unref(quitIdleSource);

	g_debug("Name %s appeared at DBUS and is owned by %s", inName, inOwner);
}

static void _on_dbus_watcher_name_vanished(GDBusConnection *inConnection,
											const gchar *inName,
											gpointer inUserData)
{
	RestartData			*restartData;

	g_return_if_fail(inUserData);

	restartData=(RestartData*)inUserData;
	restartData->appHasQuitted=TRUE;

	g_main_loop_quit(restartData->mainLoop);

	g_debug("Name %s does not exist at DBUS or vanished", inName);
}

/**
 * xfce_overview_workspaces_restart:
 * @application: The remote #XfceOverviewWorkspacesApplication instance requesting restart
 *
 * Quit the running primary instance via DBus and wait until its bus name
 * vanishes. Used to handle `-r`/`--restart` and `-q`/`--quit` flows.
 *
 * Returns: %TRUE if the running instance quitted, otherwise %FALSE.
 */
gboolean xfce_overview_workspaces_restart(XfceOverviewWorkspacesApplication *inApplication)
{
	GMainLoop			*mainLoop;
	guint				dbusWatcherID;
	GSource				*timeoutSource;
	RestartData			restartData;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_APPLICATION(inApplication), FALSE);

	mainLoop=g_main_loop_new(NULL, FALSE);

	restartData.mainLoop=mainLoop;
	restartData.application=inApplication;
	restartData.appHasQuitted=FALSE;

	dbusWatcherID=g_bus_watch_name(G_BUS_TYPE_SESSION,
									g_application_get_application_id(G_APPLICATION(inApplication)),
									G_BUS_NAME_WATCHER_FLAGS_NONE,
									_on_dbus_watcher_name_appeared,
									_on_dbus_watcher_name_vanished,
									&restartData,
									NULL);

	timeoutSource=g_timeout_source_new(DEFAULT_RESTART_WAIT_TIMEOUT);
	g_source_set_callback(timeoutSource, _on_quit_timeout, &restartData, NULL);
	g_source_attach(timeoutSource, g_main_loop_get_context(mainLoop));
	g_source_unref(timeoutSource);

	g_debug("Starting main loop for waiting the application to quit");
	g_main_loop_run(mainLoop);
	g_debug("Returned from main loop for waiting the application to quit");

	if(!restartData.appHasQuitted)
	{
		g_warning("Cannot restart application: Failed to quit running instance");
	}

	g_bus_unwatch_name(dbusWatcherID);

	return(restartData.appHasQuitted);
}
