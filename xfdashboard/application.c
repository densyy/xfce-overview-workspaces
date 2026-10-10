/*
 * application: The xfdashboard application
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
#ifdef HAVE_XFCE_REVISION_H
#include "xfce-revision.h"
#endif

#include "application.h"

#include <string.h>

#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>
#include <libxfce4ui/libxfce4ui.h>

#include <common/xfconf-settings.h>
#include <libxfdashboard/core.h>
#include <libxfdashboard/debug.h>


/* Define this class in GObject system */
struct _XfdashboardApplicationPrivate
{
	/* Properties related */
	gboolean							isDaemon;

	/* Instance related */
	gboolean							initialized;
	gboolean							forcedNewInstance;

	XfdashboardCore						*core;

	XfceSMClient						*sessionManagementClient;
};

G_DEFINE_TYPE_WITH_PRIVATE(XfdashboardApplication,
							xfdashboard_application,
							G_TYPE_APPLICATION)

/* Properties */
enum
{
	PROP_0,

	PROP_DAEMONIZED,

	PROP_LAST
};

static GParamSpec* XfdashboardApplicationProperties[PROP_LAST]={ 0, };


/* IMPLEMENTATION: Private variables and methods */
#define XFDASHBOARD_APP_ID					"de.froevel.nomad.xfdashboard"

/* Forward declarations */
static void _xfdashboard_application_activate(GApplication *inApplication);

/* Command-line options (file-scope static const, filled into AppOpts by parse_opts) */
typedef struct _AppOpts AppOpts;
struct _AppOpts
{
	gboolean						daemon;
	gboolean						quit;
	gboolean						restart;
	gboolean						toggle;
	gchar							*view;
	gboolean						version;
};

static const GOptionEntry _appOptionEntriesTemplate[]=
{
	{ "daemonize", 'd', 0, G_OPTION_ARG_NONE, NULL, N_("Fork to background"), NULL },
	{ "quit", 'q', 0, G_OPTION_ARG_NONE, NULL, N_("Quit running instance"), NULL },
	{ "restart", 'r', 0, G_OPTION_ARG_NONE, NULL, N_("Restart running instance"), NULL },
	{ "toggle", 't', 0, G_OPTION_ARG_NONE, NULL, N_("Toggles visibility if running instance was started in daemon mode otherwise it quits running non-daemon instance"), NULL },
	{ "view", 0, 0, G_OPTION_ARG_STRING, NULL, N_("The ID of view to switch to on startup or resume"), "ID" },
	{ "version", 'v', 0, G_OPTION_ARG_NONE, NULL, N_("Show version"), NULL },
	{ NULL }
};

/* Quit application depending on daemon mode and force parameter */
static void _xfdashboard_application_quit_intern(XfdashboardApplication *self, gboolean inForceQuit)
{
	XfdashboardApplicationPrivate	*priv;
	gboolean						shouldQuit;

	g_return_if_fail(XFDASHBOARD_IS_APPLICATION(self));

	priv=self->priv;
	shouldQuit=FALSE;

	/* Check if we should really quit this instance */
	if(inForceQuit==TRUE || priv->isDaemon==FALSE) shouldQuit=TRUE;

	/* Do nothing if application is already quitting. This can happen if
	 * application is running in daemon mode (primary instance) and another
	 * instance was called with "quit" or "restart" parameter which would
	 * cause this function to be called twice.
	 */
	if(!priv->core || xfdashboard_core_is_quitting(priv->core)) return;

	/* If application is not in daemon mode or if forced is set to TRUE
	 * destroy all stage windows ...
	 */
	if(shouldQuit==TRUE)
	{
		/* If application is told to quit, set the restart style to something
		 * when it won't restart itself.
		 */
		if(priv->sessionManagementClient &&
			XFCE_IS_SM_CLIENT(priv->sessionManagementClient))
		{
			xfce_sm_client_set_restart_style(priv->sessionManagementClient, XFCE_SM_CLIENT_RESTART_NORMAL);
		}

		/* Really quit application here and now */
		if(priv->initialized)
		{
			/* Release extra reference on application which causes g_application_run()
			 * to exit when returning.
			 */
			g_application_release(G_APPLICATION(self));
		}
	}
		/* ... otherwise emit "suspend" signal */
		else
		{
			/* Only send signal if not suspended already */
			if(!xfdashboard_core_is_suspended(priv->core))
			{
				/* Suspend core instance */
				xfdashboard_core_suspend(priv->core);
			}
		}
}

/* The session is going to quit */
static void _xfdashboard_application_on_session_quit(XfdashboardApplication *self,
														gpointer inUserData)
{
	g_return_if_fail(XFDASHBOARD_IS_APPLICATION(self));
	g_return_if_fail(XFCE_IS_SM_CLIENT(inUserData));

	/* Force application to quit */
	XFDASHBOARD_DEBUG(self, MISC, "Received 'quit' from session management client - initiating shutdown");
	_xfdashboard_application_quit_intern(self, TRUE);
}

/* The core instance asks for suspend/resume support */
static gboolean _xfdashboard_application_on_core_can_suspend(XfdashboardApplication *self, gpointer inUserData)
{
	g_return_val_if_fail(XFDASHBOARD_IS_APPLICATION(self), FALSE);
	g_return_val_if_fail(XFDASHBOARD_IS_CORE(inUserData), FALSE);

	return(self->priv->isDaemon ? TRUE : FALSE);
}

/* The core instance has requested to quit */
static void _xfdashboard_application_on_core_quit(XfdashboardApplication *self, gpointer inUserData)
{
	g_return_if_fail(XFDASHBOARD_IS_APPLICATION(self));
	g_return_if_fail(XFDASHBOARD_IS_CORE(inUserData));

	/* Quit application */
	_xfdashboard_application_quit_intern(self, FALSE);
}

/* The "quit" action was triggered */
static void _xfdashboard_application_on_simple_action_quit(XfdashboardApplication *self,
															GVariant *inParameter,
															gpointer inUserData)
{
	g_return_if_fail(XFDASHBOARD_IS_APPLICATION(self));
	g_return_if_fail(G_IS_SIMPLE_ACTION(inUserData));

	/* Force quit */
	_xfdashboard_application_quit_intern(self, TRUE);
}

/* Create and set up settings object for xfdashboard */
static void _xfdashboard_application_add_settings_path_list(GArray *inSearchPaths, const gchar *inPath, gboolean isFile)
{
	gchar								*normalizedPath;
	guint								i;
	gchar								*entry;

	g_return_if_fail(inSearchPaths);
	g_return_if_fail(inPath && *inPath);

	/* Normalize requested path to add to list of search paths that means
	 * that it should end with a directory seperator.
	 */
	if(!isFile && !g_str_has_suffix(inPath, G_DIR_SEPARATOR_S))
	{
		normalizedPath=g_strjoin(NULL, inPath, G_DIR_SEPARATOR_S, NULL);
	}
		else normalizedPath=g_strdup(inPath);

	/* Check if path is already in list of search paths */
	for(i=0; i<inSearchPaths->len; i++)
	{
		/* Get search path at iterator */
		entry=g_array_index(inSearchPaths, gchar*, i);

		/* If the path at iterator matches the requested one that it is
		 * already in list of search path, so return here with fail result.
		 */
		if(g_strcmp0(entry, normalizedPath)==0)
		{
			/* Release allocated resources */
			if(normalizedPath) g_free(normalizedPath);

			/* Return without modifying search path list */
			return;
		}
	}

	/* If we get here the requested path is not in list of search path and
	 * we can add it now.
	 */
	entry=g_strdup(normalizedPath);
	g_array_append_val(inSearchPaths, entry);

	/* Release allocated resources */
	if(normalizedPath) g_free(normalizedPath);
}

/* Add a single path to search list (wraps dedup helper) */
static void _add_one_path(GArray *inSearchPaths, const gchar *inPath, gboolean inIsFile)
{
	if(!inSearchPaths) return;
	if(!inPath || !*inPath) return;

	_xfdashboard_application_add_settings_path_list(inSearchPaths, inPath, inIsFile);
}

/* Add colon-separated paths from environment variable to search list */
static void _add_env_colon_paths(GArray *inSearchPaths, const gchar *inEnvVar, gboolean inIsFile)
{
	const gchar					*envValue;
	gchar						**paths;
	gchar						**iter;

	g_return_if_fail(inSearchPaths);
	g_return_if_fail(inEnvVar && *inEnvVar);

	envValue=g_getenv(inEnvVar);
	if(!envValue) return;

	iter=paths=g_strsplit(envValue, ":", -1);
	while(*iter)
	{
		_add_one_path(inSearchPaths, *iter, inIsFile);
		iter++;
	}
	g_strfreev(paths);
}

/* Base directory getters for default-path table */
static const gchar* _app_base_user_data(void) { return(g_get_user_data_dir()); }
static const gchar* _app_base_home(void) { return(g_get_home_dir()); }
static const gchar* _app_base_user_config(void) { return(g_get_user_config_dir()); }
static const gchar* _app_base_pkg_data(void) { return(PACKAGE_DATADIR); }
static const gchar* _app_base_pkg_lib(void) { return(PACKAGE_LIBDIR); }

typedef struct _AppDefaultPath AppDefaultPath;
struct _AppDefaultPath
{
	const gchar* (*getBase)(void);
	const gchar					*child1;
	const gchar					*child2;
	gboolean					isFile;
};

static const AppDefaultPath _themesDefaults[]=
{
	{ _app_base_user_data, "themes", NULL, FALSE },
	{ _app_base_home, ".themes", NULL, FALSE },
	{ _app_base_pkg_data, "themes", NULL, FALSE },
	{ NULL, NULL, NULL, FALSE }
};

static const AppDefaultPath _pluginsDefaults[]=
{
	{ _app_base_user_data, "xfdashboard", "plugins", FALSE },
	{ _app_base_pkg_lib, "xfdashboard", "plugins", FALSE },
	{ NULL, NULL, NULL, FALSE }
};

static const AppDefaultPath _bindingsDefaults[]=
{
	{ _app_base_pkg_data, "xfdashboard", "bindings.xml", TRUE },
	{ _app_base_user_config, "xfdashboard", "bindings.xml", TRUE },
	{ NULL, NULL, NULL, FALSE }
};

/* Add default paths from static table to search list */
static void _add_default_paths(GArray *inSearchPaths, const AppDefaultPath *inDefaults)
{
	const AppDefaultPath		*def;
	const gchar					*base;
	gchar						*path;

	g_return_if_fail(inSearchPaths);
	g_return_if_fail(inDefaults);

	for(def=inDefaults; def->getBase; def++)
	{
		base=def->getBase();
		if(!base || !*base) continue;

		if(def->child2) path=g_build_filename(base, def->child1, def->child2, NULL);
			else path=g_build_filename(base, def->child1, NULL);
		_add_one_path(inSearchPaths, path, def->isFile);
		g_free(path);
	}
}

static XfdashboardSettings* _xfdashboard_application_create_settings(XfdashboardApplication *self)
{
	XfdashboardSettings			*settings;
	GArray						*pathFileList;
	gchar						**themesSearchPaths;
	gchar						**pluginsSearchPaths;
	gchar						**bindingFilePaths;
	gchar						*configPath;
	gchar						*dataPath;

	g_return_val_if_fail(XFDASHBOARD_IS_APPLICATION(self), NULL);

	themesSearchPaths=NULL;
	pluginsSearchPaths=NULL;
	bindingFilePaths=NULL;

	/* Set up search path for themes (env first, then static defaults table) */
	pathFileList=g_array_new(TRUE, TRUE, sizeof(gchar*));

	_add_env_colon_paths(pathFileList, "XFDASHBOARD_THEME_PATH", FALSE);
	_add_default_paths(pathFileList, _themesDefaults);

	themesSearchPaths=(gchar**)(gpointer)g_array_free(pathFileList, FALSE);

	/* Set up search path for plugins (env first, then static defaults table) */
	pathFileList=g_array_new(TRUE, TRUE, sizeof(gchar*));

	_add_env_colon_paths(pathFileList, "XFDASHBOARD_PLUGINS_PATH", FALSE);
	_add_default_paths(pathFileList, _pluginsDefaults);

	pluginsSearchPaths=(gchar**)(gpointer)g_array_free(pathFileList, FALSE);

	/* Set up file path for bindings (defaults table first, then env) */
	pathFileList=g_array_new(TRUE, TRUE, sizeof(gchar*));

	_add_default_paths(pathFileList, _bindingsDefaults);
	_add_env_colon_paths(pathFileList, "XFDASHBOARD_BINDINGS_POOL_FILE", TRUE);

	bindingFilePaths=(gchar**)(gpointer)g_array_free(pathFileList, FALSE);

	/* Set up base path to configuration files */
	configPath=g_build_filename(g_get_user_config_dir(), "xfdashboard", NULL);

	/* Set up base path to data files */
	dataPath=g_build_filename(g_get_user_data_dir(), "xfdashboard", NULL);

	/* Create settings instance for Xfconf settings storage */
	settings=g_object_new(XFDASHBOARD_TYPE_XFCONF_SETTINGS,
							"binding-files", bindingFilePaths,
							"theme-search-paths", themesSearchPaths,
							"plugin-search-paths", pluginsSearchPaths,
							"config-path", configPath,
							"data-path", dataPath,
							NULL);

	/* Release allocated resources */
	if(dataPath) g_free(dataPath);
	if(configPath) g_free(configPath);
	if(themesSearchPaths) g_strfreev(themesSearchPaths);
	if(pluginsSearchPaths) g_strfreev(pluginsSearchPaths);
	if(bindingFilePaths) g_strfreev(bindingFilePaths);

	/* Return settings instance regardless if it could be
	 * created or not (NULL pointer).
	 */
	return(settings);
}

/* Perform full initialization of this application instance */
static gboolean _xfdashboard_application_initialize_full(XfdashboardApplication *self)
{
	XfdashboardApplicationPrivate	*priv;
	GError							*error;
	XfceSMClientRestartStyle		sessionManagementRestartStyle;
	XfdashboardSettings				*settings;

	g_return_val_if_fail(XFDASHBOARD_IS_APPLICATION(self), FALSE);

	priv=self->priv;
	error=NULL;

	/* Set up the session management */
	g_assert(priv->sessionManagementClient==NULL);

	sessionManagementRestartStyle=XFCE_SM_CLIENT_RESTART_IMMEDIATELY;
	if(priv->forcedNewInstance) sessionManagementRestartStyle=XFCE_SM_CLIENT_RESTART_NORMAL;

	priv->sessionManagementClient=xfce_sm_client_get();
	xfce_sm_client_set_priority(priv->sessionManagementClient, XFCE_SM_CLIENT_PRIORITY_DEFAULT);
	xfce_sm_client_set_restart_style(priv->sessionManagementClient, sessionManagementRestartStyle);
	g_signal_connect_swapped(priv->sessionManagementClient, "quit", G_CALLBACK(_xfdashboard_application_on_session_quit), self);

	if(!xfce_sm_client_connect(priv->sessionManagementClient, &error))
	{
		g_warning("Failed to connect to session manager: %s",
					(error && error->message) ? error->message : "unknown error");
		g_clear_error(&error);
	}

	/* Set up core instance */
	g_assert(priv->core==NULL);

	settings=_xfdashboard_application_create_settings(self);
	if(!settings)
	{
		g_critical("Could not set up runtime settings for xfdashboard");
		return(FALSE);
	}

	priv->core=g_object_new(XFDASHBOARD_TYPE_CORE,
								"settings", settings,
								NULL);
	if(!priv->core)
	{
		g_critical("Could not create core instance for xfdashboard");
		return(FALSE);
	}

	g_signal_connect_swapped(priv->core, "can-suspend", G_CALLBACK(_xfdashboard_application_on_core_can_suspend), self);
	g_signal_connect_swapped(priv->core, "quit", G_CALLBACK(_xfdashboard_application_on_core_quit), self);

	if(!xfdashboard_core_initialize(priv->core, &error))
	{
		g_warning("Failed to set up core instance for xfdashboard: %s",
					(error && error->message) ? error->message : "unknown error");
		g_clear_error(&error);
		return(FALSE);
	}

	return(TRUE);
}

/* Switch to requested view */
static void _xfdashboard_application_switch_to_view(XfdashboardApplication *self, const gchar *inInternalViewName)
{
	XfdashboardApplicationPrivate	*priv;
	XfdashboardStage				*stage;

	g_return_if_fail(XFDASHBOARD_IS_APPLICATION(self));

	priv=self->priv;

	/* If no view name was specified then do nothing and return immediately */
	if(!inInternalViewName ||
		!inInternalViewName[0])
	{
		XFDASHBOARD_DEBUG(self, MISC, "No view to switch to specified");
		return;
	}

	/* Tell core to switch requested view */
	XFDASHBOARD_DEBUG(self, MISC,
						"Trying to switch to view '%s'",
						inInternalViewName);

	stage=xfdashboard_core_get_stage(priv->core);
	if(!stage)
	{
		g_critical("No stage found to switch to view '%s'", inInternalViewName);
		return;
	}

	xfdashboard_stage_set_switch_to_view(stage, inInternalViewName);
}

/* Pure option parsing (no application side effects except filling AppOpts) */
static gboolean parse_opts(gint *ioArgc, gchar ***ioArgv, AppOpts *outOpts, GError **outError)
{
	GOptionContext				*context;
	GOptionEntry				entries[G_N_ELEMENTS(_appOptionEntriesTemplate)];
	gchar						buf[1024];

	g_return_val_if_fail(ioArgc, FALSE);
	g_return_val_if_fail(ioArgv, FALSE);
	g_return_val_if_fail(outOpts, FALSE);
	g_return_val_if_fail(outError==NULL || *outError==NULL, FALSE);

	/* Set up default option values */
	outOpts->daemon=FALSE;
	outOpts->quit=FALSE;
	outOpts->restart=FALSE;
	outOpts->toggle=FALSE;
	outOpts->view=NULL;
	outOpts->version=FALSE;

	/* Bind file-scope template to this AppOpts instance */
	memcpy(entries, _appOptionEntriesTemplate, sizeof(entries));
	entries[0].arg_data=&outOpts->daemon;
	entries[1].arg_data=&outOpts->quit;
	entries[2].arg_data=&outOpts->restart;
	entries[3].arg_data=&outOpts->toggle;
	entries[4].arg_data=&outOpts->view;
	entries[5].arg_data=&outOpts->version;

	/* Setup command-line options */
	context=g_option_context_new("");
	g_snprintf(buf, sizeof(buf), _("A Gnome Shell like dashboard for Xfce4 - version %s"), VERSION_FULL);
	g_option_context_set_summary(context, buf);
	g_option_context_set_translation_domain(context, GETTEXT_PACKAGE);
	/* Do not open the default display while parsing: the primary instance
	 * already opened it via gtk_init() in main(), and a remote instance
	 * (e.g. '-t'/'-q' forwarding) must never touch the display. TRUE would
	 * make GOption call gtk_init-equivalent work during parse.
	 */
	g_option_context_add_group(context, gtk_get_option_group(FALSE));
	g_option_context_add_group(context, clutter_get_option_group_without_init());
	g_option_context_add_group(context, xfce_sm_client_get_option_group(*ioArgc, *ioArgv));
	g_option_context_add_main_entries(context, entries, GETTEXT_PACKAGE);

#ifdef DEBUG
#ifdef XFDASHBOARD_ENABLE_DEBUG
	g_print("** Use environment variable XFDASHBOARD_DEBUG to enable debug messages\n");
	g_print("** To get a list of debug categories set XFDASHBOARD_DEBUG=help\n");
#endif
#endif

	if(!g_option_context_parse(context, ioArgc, ioArgv, outError))
	{
		if(context) g_option_context_free(context);
		return(FALSE);
	}

	if(context) g_option_context_free(context);
	return(TRUE);
}

/* Dispatch helpers (moved from handle_command_line_arguments, order kept) */
static gboolean _handle_restart(XfdashboardApplication *self, const AppOpts *inOpts, gint *outStatus)
{
	XfdashboardApplicationPrivate	*priv;

	g_return_val_if_fail(XFDASHBOARD_IS_APPLICATION(self), FALSE);
	g_return_val_if_fail(inOpts, FALSE);

	priv=self->priv;

	if(inOpts->restart &&
		priv->initialized)
	{
		/* Quit existing instance for restart */
		XFDASHBOARD_DEBUG(self, MISC, "Received request to restart application!");
		_xfdashboard_application_quit_intern(self, TRUE);

		if(outStatus) *outStatus=XFDASHBOARD_APPLICATION_ERROR_RESTART;
		return(TRUE);
	}

	return(FALSE);
}

static gboolean _handle_quit(XfdashboardApplication *self, const AppOpts *inOpts, gint *outStatus)
{
	g_return_val_if_fail(XFDASHBOARD_IS_APPLICATION(self), FALSE);
	g_return_val_if_fail(inOpts, FALSE);

	if(inOpts->quit)
	{
		/* Quit existing instance */
		XFDASHBOARD_DEBUG(self, MISC, "Received request to quit running instance!");
		xfdashboard_application_quit_forced(self);

		if(outStatus) *outStatus=XFDASHBOARD_APPLICATION_ERROR_QUIT;
		return(TRUE);
	}

	return(FALSE);
}

static gboolean _handle_toggle(XfdashboardApplication *self, const AppOpts *inOpts, gint *outStatus)
{
	XfdashboardApplicationPrivate	*priv;

	g_return_val_if_fail(XFDASHBOARD_IS_APPLICATION(self), FALSE);
	g_return_val_if_fail(inOpts, FALSE);

	priv=self->priv;

	if(inOpts->toggle &&
		priv->initialized)
	{
		/* If application is running in daemon mode, toggle between suspend/resume ... */
		if(priv->isDaemon)
		{
			if(xfdashboard_core_is_suspended(priv->core))
			{
				/* Switch to view if requested */
				_xfdashboard_application_switch_to_view(self, inOpts->view);

				/* Show application again */
				_xfdashboard_application_activate(G_APPLICATION(self));
			}
				else
				{
					/* Hide application */
					_xfdashboard_application_quit_intern(self, FALSE);
				}
		}
			/* ... otherwise if not running in daemon mode, just quit */
			else
			{
				/* Hide application */
				_xfdashboard_application_quit_intern(self, FALSE);
			}

		if(outStatus) *outStatus=XFDASHBOARD_APPLICATION_ERROR_NONE;
		return(TRUE);
	}

	return(FALSE);
}

/* Handle command-line on primary instance (parse->dispatch->init) */
static gint _xfdashboard_application_handle_command_line_arguments(XfdashboardApplication *self,
															gint inArgc,
															gchar **inArgv)
{
	XfdashboardApplicationPrivate	*priv;
	gboolean						result;
	GError							*error;
	AppOpts							opts;
	gint							dispatchStatus;

	g_return_val_if_fail(XFDASHBOARD_IS_APPLICATION(self), XFDASHBOARD_APPLICATION_ERROR_FAILED);

	priv=self->priv;
	error=NULL;

	/* Parse command-line (pure, no side effects) */
	if(!parse_opts(&inArgc, &inArgv, &opts, &error))
	{
		/* Show error */
		g_print(N_("%s\n"),
				(error && error->message) ? error->message : _("unknown error"));
		if(error)
		{
			g_error_free(error);
			error=NULL;
		}

		/* Release allocated resources */
		if(opts.view) g_free(opts.view);

		return(XFDASHBOARD_APPLICATION_ERROR_FAILED);
	}

	/* Dispatch: remote instance handling */
	if(g_application_get_is_remote(G_APPLICATION(self)))
	{
		XFDASHBOARD_DEBUG(self, MISC, "Do not handle command-line parameters on remote application instance");

		/* One exception is "--version" */
		if(opts.version)
		{
			g_print("Remote instance: %s-%s\n", PACKAGE_NAME, VERSION_FULL);
		}

		/* Release allocated resources */
		if(opts.view) g_free(opts.view);

		/* No errors so far and no errors will happen as we do not handle
		 * any command-line arguments here.
		 */
		return(XFDASHBOARD_APPLICATION_ERROR_NONE);
	}
	XFDASHBOARD_DEBUG(self, MISC, "Handling command-line parameters on primary application instance");

#ifdef DEBUG
	/* Check if a new instance was forced by checking application ID */
	if(g_strcmp0(g_application_get_application_id(G_APPLICATION(self)), XFDASHBOARD_APP_ID)!=0)
	{
		priv->forcedNewInstance=TRUE;
	}
#endif

	/* Dispatch: restart, quit, toggle (order kept) */
	if(_handle_restart(self, &opts, &dispatchStatus))
	{
		if(opts.view) g_free(opts.view);
		return(dispatchStatus);
	}

	if(_handle_quit(self, &opts, &dispatchStatus))
	{
		if(opts.view) g_free(opts.view);
		return(dispatchStatus);
	}

	if(_handle_toggle(self, &opts, &dispatchStatus))
	{
		if(opts.view) g_free(opts.view);
		return(dispatchStatus);
	}

	/* Handle options: version
	 *
	 * Show the version of this (maybe daemonized) instance.
	 */
	if(opts.version)
	{
		if(priv->isDaemon)
		{
			g_print("Daemon instance: %s-%s\n", PACKAGE_NAME, VERSION_FULL);
			if(opts.view) g_free(opts.view);
			return(XFDASHBOARD_APPLICATION_ERROR_NONE);
		}
			else
			{
				g_print("Version: %s-%s\n", PACKAGE_NAME, VERSION_FULL);
				if(opts.view) g_free(opts.view);
				return(XFDASHBOARD_APPLICATION_ERROR_QUIT);
			}
	}

	/* Init: full initialization and daemon handling */
	if(!priv->initialized)
	{
		/* Set daemon mode property if requested which must be done before the core is
		 * initialized, otherwise the "can-suspend" signal handler will return FALSE
		 * which causes plugins depending on daemon to be disabled. Setting daemon mode
		 * does not work if a new instance was forced.
		 */
		if(opts.daemon && !priv->forcedNewInstance)
		{
			priv->isDaemon=opts.daemon;
			g_object_notify_by_pspec(G_OBJECT(self), XfdashboardApplicationProperties[PROP_DAEMONIZED]);
		}

		/* Perform full initialization of this application instance */
		result=_xfdashboard_application_initialize_full(self);
		if(result==FALSE)
		{
			if(opts.view) g_free(opts.view);
			return(XFDASHBOARD_APPLICATION_ERROR_FAILED);
		}

		/* Handle options: daemonize
		 *
		 * Check if application shoud run in daemon mode. A daemonized instance runs in
		 * background and does not present the stage initially. The application must not
		 * be initialized yet as it can only be done on start-up.
		 *
		 * Does not work if a new instance was forced.
		 */
		if(opts.daemon)
		{
			if(!priv->forcedNewInstance)
			{
				if(priv->isDaemon)
				{
					xfdashboard_core_suspend(priv->core);
				}
			}
				else
				{
					g_warning("Cannot daemonize because a temporary new instance of application was forced.");
				}
		}

		/* Switch to view if requested */
		_xfdashboard_application_switch_to_view(self, opts.view);

		/* Show application if not started daemonized */
		if(!priv->isDaemon)
		{
			XfdashboardStage* 		stage;

			/* Get stage to show */
			stage=xfdashboard_core_get_stage(priv->core);
			if(!stage)
			{
				g_critical("No stage available to show at start-up of standalone application");

				if(opts.view) g_free(opts.view);
				return(XFDASHBOARD_APPLICATION_ERROR_FAILED);
			}

			/* Show stage */
			clutter_actor_show(CLUTTER_ACTOR(stage));
		}

		/* Take extra reference on the application to keep application in
		 * function g_application_run() alive when returning.
		 */
		g_application_hold(G_APPLICATION(self));
	}

	/* Check if this instance need to be activated. Is should only be done
	 * if instance is initialized
	 */
	if(priv->initialized)
	{
		/* Switch to view if requested */
		_xfdashboard_application_switch_to_view(self, opts.view);

		/* Show application */
		_xfdashboard_application_activate(G_APPLICATION(self));
	}

	/* Release allocated resources */
	if(opts.view) g_free(opts.view);

	/* All done successfully so return status code 0 for success */
	priv->initialized=TRUE;
	return(XFDASHBOARD_APPLICATION_ERROR_NONE);
}

/* IMPLEMENTATION: GApplication */

/* Received "activate" signal on primary instance */
static void _xfdashboard_application_activate(GApplication *inApplication)
{
	XfdashboardApplication			*self;
	XfdashboardApplicationPrivate	*priv;

	g_return_if_fail(XFDASHBOARD_IS_APPLICATION(inApplication));

	self=XFDASHBOARD_APPLICATION(inApplication);
	priv=self->priv;

	/* Resume core instance */
	if(xfdashboard_core_is_suspended(priv->core))
	{
		xfdashboard_core_resume(priv->core);
	}
}

/* Handle command-line on primary instance */
static int _xfdashboard_application_command_line(GApplication *inApplication, GApplicationCommandLine *inCommandLine)
{
	XfdashboardApplication			*self;
	gint							argc;
	gchar							**argv;
	gint							exitStatus;

	g_return_val_if_fail(XFDASHBOARD_IS_APPLICATION(inApplication), XFDASHBOARD_APPLICATION_ERROR_FAILED);

	self=XFDASHBOARD_APPLICATION(inApplication);

	/* Get number of command-line arguments and the arguments */
	argv=g_application_command_line_get_arguments(inCommandLine, &argc);

	/* Parse command-line and get exit status code */
	exitStatus=_xfdashboard_application_handle_command_line_arguments(self, argc, argv);

	/* Release allocated resources */
	if(argv) g_strfreev(argv);

	/* Return exit status code */
	return(exitStatus);
}

/* Check and handle command-line on local instance regardless if this one
 * is the primary instance or a remote one. This functions checks for arguments
 * which can be handled locally, e.g. '--help'. Otherwise they will be send to
 * the primary instance.
 */
static gboolean _xfdashboard_application_local_command_line(GApplication *inApplication,
															gchar ***ioArguments,
															int *outExitStatus)
{
	XfdashboardApplication			*self;
	GError							*error;

	g_return_val_if_fail(XFDASHBOARD_IS_APPLICATION(inApplication), TRUE);

	self=XFDASHBOARD_APPLICATION(inApplication);
	error=NULL;

	/* Set exit status code initially to success result but can be changed later */
	if(outExitStatus) *outExitStatus=XFDASHBOARD_APPLICATION_ERROR_NONE;

	/* Try to register application to determine early if this instance will be
	 * the primary application instance or a remote one.
	 */
	if(!g_application_register(inApplication, NULL, &error))
	{
		g_critical("Unable to register application: %s",
					(error && error->message) ? error->message : "Unknown error");
		if(error)
		{
			g_error_free(error);
			error=NULL;
		}

		/* Set error status code */
		if(outExitStatus) *outExitStatus=XFDASHBOARD_APPLICATION_ERROR_FAILED;

		/* Return TRUE to indicate that command-line does not need further processing */
		return(TRUE);
	}

	/* If this is a remote instance we need to parse command-line now */
	if(g_application_get_is_remote(inApplication))
	{
		gint						argc;
		gchar						**argv;
		gchar						**originArgv;
		gint						exitStatus;
		gint						i;

		/* We have to make a extra copy of the command-line arguments, since
		 * g_option_context_parse() in command-line argument handling function
		 * might remove parameters from the arguments list and maybe we need
		 * them to sent the arguments to primary instance if not handled locally
		 * like '--help'.
		 */
		originArgv=*ioArguments;

		argc=g_strv_length(originArgv);
		argv=g_new0(gchar*, argc+1);
		for(i=0; i<=argc; i++) argv[i]=g_strdup(originArgv[i]);

		/* Parse command-line and store exit status code */
		exitStatus=_xfdashboard_application_handle_command_line_arguments(self, argc, argv);
		if(outExitStatus) *outExitStatus=exitStatus;

		/* Release allocated resources */
		if(argv) g_strfreev(argv);

		/* If exit status code indicates an error then return TRUE here to indicate
		 * that command-line does not need further processing.
		 */
		if(exitStatus==XFDASHBOARD_APPLICATION_ERROR_FAILED) return(TRUE);
	}

	/* Return FALSE to indicate that command-line was not completely handled
	 * and needs further processing, e.g. this is the primary instance or a remote
	 * instance which could not handle the arguments locally.
	 */
	return(FALSE);
}


/* IMPLEMENTATION: GObject */

/* Dispose this object */
static void _xfdashboard_application_dispose(GObject *inObject)
{
	XfdashboardApplication			*self=XFDASHBOARD_APPLICATION(inObject);
	XfdashboardApplicationPrivate	*priv=self->priv;

	/* Release allocated resources */
	if(priv->core)
	{
		g_object_unref(priv->core);
		priv->core=NULL;
	}

	/* Shutdown session management */
	if(priv->sessionManagementClient)
	{
		/* This instance looks like to be disposed normally and not like a crash
		 * so set the restart style at session management to something that it
		 * will not restart itself but shutting down.
		 */
		if(XFCE_IS_SM_CLIENT(priv->sessionManagementClient))
		{
			xfce_sm_client_set_restart_style(priv->sessionManagementClient, XFCE_SM_CLIENT_RESTART_NORMAL);
		}

		priv->sessionManagementClient=NULL;
	}

	/* Call parent's class dispose method */
	G_OBJECT_CLASS(xfdashboard_application_parent_class)->dispose(inObject);
}

/* Set/get properties */
static void _xfdashboard_application_get_property(GObject *inObject,
													guint inPropID,
													GValue *outValue,
													GParamSpec *inSpec)
{
	XfdashboardApplication	*self=XFDASHBOARD_APPLICATION(inObject);

	switch(inPropID)
	{
		case PROP_DAEMONIZED:
			g_value_set_boolean(outValue, self->priv->isDaemon);
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(inObject, inPropID, inSpec);
			break;
	}
}

/* Class initialization
 * Override functions in parent classes and define properties
 * and signals
 */
static void xfdashboard_application_class_init(XfdashboardApplicationClass *klass)
{
	GApplicationClass	*appClass=G_APPLICATION_CLASS(klass);
	GObjectClass		*gobjectClass=G_OBJECT_CLASS(klass);

	/* Override functions */
	appClass->activate=_xfdashboard_application_activate;
	appClass->command_line=_xfdashboard_application_command_line;
	appClass->local_command_line=_xfdashboard_application_local_command_line;

	gobjectClass->dispose=_xfdashboard_application_dispose;
	gobjectClass->get_property=_xfdashboard_application_get_property;

	/* Define properties */
	/**
	 * XfdashboardApplication:is-daemonized:
	 *
	 * A flag indicating if application is running in daemon mode. It is set to
	 * %TRUE if it running in daemon mode otherwise %FALSE as it running in
	 * standalone application.
	 */
	XfdashboardApplicationProperties[PROP_DAEMONIZED]=
		g_param_spec_boolean("is-daemonized",
								"Is daemonized",
								"Flag indicating if application is daemonized",
								FALSE,
								G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);
}

/* Object initialization
 * Create private structure and set up default values
 */
static void xfdashboard_application_init(XfdashboardApplication *self)
{
	XfdashboardApplicationPrivate	*priv;
	GSimpleAction					*action;

	priv=self->priv=xfdashboard_application_get_instance_private(self);

	/* Set default values */
	priv->isDaemon=FALSE;
	priv->initialized=FALSE;
	priv->sessionManagementClient=NULL;
	priv->forcedNewInstance=FALSE;

	/* Add callable DBUS actions for this application */
	action=g_simple_action_new("Quit", NULL);
	g_signal_connect_swapped(action, "activate", G_CALLBACK(_xfdashboard_application_on_simple_action_quit), self);
	g_action_map_add_action(G_ACTION_MAP(self), G_ACTION(action));
	g_object_unref(action);
}


/* IMPLEMENTATION: Public API */

/**
 * xfdashboard_application_new:
 *
 * Creates an instance of #XfdashboardApplication.
 *
 * Return value: (transfer none): The instance of #XfdashboardApplication.
 */
XfdashboardApplication* xfdashboard_application_new(void)
{
	XfdashboardApplication		*application;
	gchar						*appID;
	const gchar					*forceNewInstance;

	application=NULL;
	forceNewInstance=NULL;

#ifdef DEBUG
	/* If a new instance of xfdashboard is forced, e.g. for debugging purposes,
	 * then create a unique application ID.
	 */
	forceNewInstance=g_getenv("XFDASHBOARD_FORCE_NEW_INSTANCE");
#endif

	if(forceNewInstance)
	{
		appID=g_strdup_printf("%s-%u", XFDASHBOARD_APP_ID, getpid());
		g_message("Forcing new application instance with ID '%s'", appID);
	}
		else appID=g_strdup(XFDASHBOARD_APP_ID);

	application=XFDASHBOARD_APPLICATION(g_object_new(XFDASHBOARD_TYPE_APPLICATION,
														"application-id", appID,
														"flags", G_APPLICATION_HANDLES_COMMAND_LINE,
														NULL));

	/* Release allocated resources */
	if(appID) g_free(appID);

	/* Return newly created application instance */
	return(application);
}

/**
 * xfdashboard_application_is_daemonized:
 * @self: A #XfdashboardApplication
 *
 * Checks if application is running in background (daemon mode).
 *
 * Return value: %TRUE if @self is running in background and
 *   %FALSE if it is running as standalone application.
 */
gboolean xfdashboard_application_is_daemonized(XfdashboardApplication *self)
{
	g_return_val_if_fail(XFDASHBOARD_IS_APPLICATION(self), FALSE);

	return(self->priv->isDaemon);
}

/**
 * xfdashboard_application_quit_forced:
 * @self: A #XfdashboardApplication
 *
 * Quits @self regardless if it is running as standalone application
 * or in daemon mode. The application is really quitted after calling
 * this function.
 */
void xfdashboard_application_quit_forced(XfdashboardApplication *self)
{
	g_return_if_fail(XFDASHBOARD_IS_APPLICATION(self));

	/* Quit also any other running instance */
	if(g_application_get_is_remote(G_APPLICATION(self))==TRUE)
	{
		g_action_group_activate_action(G_ACTION_GROUP(self), "Quit", NULL);
	}

	/* Quit this instance */
	_xfdashboard_application_quit_intern(self, TRUE);
}
