/*
 * plugin: A plugin class managing loading the shared object as well as
 *         initializing and setting up extensions to this application
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

#include <libxfce-overview-workspaces/plugin.h>

#include <glib/gi18n-lib.h>

#include <libxfce-overview-workspaces/core.h>
#include <libxfce-overview-workspaces/enums.h>
#include <libxfce-overview-workspaces/marshal.h>
#include <libxfce-overview-workspaces/debug.h>


/* Forward declaration */
typedef enum /*< skip,prefix=XFCE_OVERVIEW_WORKSPACES_PLUGIN_STATE >*/
{
	XFCE_OVERVIEW_WORKSPACES_PLUGIN_STATE_NONE=0,
	XFCE_OVERVIEW_WORKSPACES_PLUGIN_STATE_INITIALIZED,
	XFCE_OVERVIEW_WORKSPACES_PLUGIN_STATE_ENABLED
} XfceOverviewWorkspacesPluginState;


/* Define this class in GObject system */
struct _XfceOverviewWorkspacesPluginPrivate
{
	/* Properties related */
	gchar						*id;
	XfceOverviewWorkspacesPluginFlag		flags;
	gchar						*name;
	gchar						*description;
	gchar						*author;
	gchar						*copyright;
	gchar						*license;
	XfceOverviewWorkspacesPluginSettings	*settings;

	/* Instance related */
	gchar						*filename;
	gboolean					didLoad;
	GModule						*module;
	void 						(*initialize)(XfceOverviewWorkspacesPlugin *self);
	XfceOverviewWorkspacesPluginState		state;
	gchar						*lastLoadingError;

	gpointer					userData;
	GDestroyNotify				userDataDestroyCallback;

	gchar						*configPath;
	gchar						*dataPath;
};

G_DEFINE_TYPE_WITH_PRIVATE(XfceOverviewWorkspacesPlugin,
							xfce_overview_workspaces_plugin,
							G_TYPE_TYPE_MODULE)

/* Properties */
enum
{
	PROP_0,

	PROP_FILENAME,

	PROP_ID,
	PROP_FLAGS,
	PROP_NAME,
	PROP_DESCRIPTION,
	PROP_AUTHOR,
	PROP_COPYRIGHT,
	PROP_LICENSE,

	PROP_SETTINGS,
	PROP_CONFIG_PATH,
	PROP_DATA_PATH,

	PROP_LAST
};

static GParamSpec* XfceOverviewWorkspacesPluginProperties[PROP_LAST]={ 0, };

/* Signals */
enum
{
	/* Actions */
	ACTION_ENABLE,
	ACTION_DISABLE,
	ACTION_CONFIGURE,

	SIGNAL_LAST
};

static guint XfceOverviewWorkspacesPluginSignals[SIGNAL_LAST]={ 0, };


/* IMPLEMENTATION: Private variables and methods */
#define XFCE_OVERVIEW_WORKSPACES_PLUGIN_FUNCTION_NAME_INITIALIZE		"plugin_init"

/* Get display name for XFCE_OVERVIEW_WORKSPACES_PLUGIN_STATE_* enum values */
static const gchar* _xfce_overview_workspaces_plugin_get_plugin_state_value_name(XfceOverviewWorkspacesPluginState inState)
{
	g_return_val_if_fail(inState<=XFCE_OVERVIEW_WORKSPACES_PLUGIN_STATE_ENABLED, NULL);

	/* Lookup name for value and return it */
	switch(inState)
	{
		case XFCE_OVERVIEW_WORKSPACES_PLUGIN_STATE_NONE:
			return("none");

		case XFCE_OVERVIEW_WORKSPACES_PLUGIN_STATE_INITIALIZED:
			return("initialized");

		case XFCE_OVERVIEW_WORKSPACES_PLUGIN_STATE_ENABLED:
			return("enabled");

		default:
			break;
	}

	/* We should normally never get here. But if we do return NULL. */
	return(NULL);
}

/* Return error message of last load attempt */
static const gchar* _xfce_overview_workspaces_plugin_get_loading_error(XfceOverviewWorkspacesPlugin *self)
{
	XfceOverviewWorkspacesPluginPrivate	*priv;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_PLUGIN(self), NULL);

	priv=self->priv;

	/* Return error message of last loading attempt */
	return(priv->lastLoadingError);
}

/* Set file name for plugin */
static void _xfce_overview_workspaces_plugin_set_filename(XfceOverviewWorkspacesPlugin *self, const gchar *inFilename)
{
	XfceOverviewWorkspacesPluginPrivate		*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_PLUGIN(self));
	g_return_if_fail(inFilename && *inFilename);
	g_return_if_fail(self->priv->state==XFCE_OVERVIEW_WORKSPACES_PLUGIN_STATE_NONE);
	g_return_if_fail(self->priv->filename==NULL);

	priv=self->priv;

	/* Set value if changed */
	if(g_strcmp0(priv->filename, inFilename)!=0)
	{
		/* Set value */
		if(priv->filename) g_free(priv->filename);
		priv->filename=g_strdup(inFilename);

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesPluginProperties[PROP_FILENAME]);
	}
}

/* Set ID for plugin */
static void _xfce_overview_workspaces_plugin_set_id(XfceOverviewWorkspacesPlugin *self, const gchar *inID)
{
	XfceOverviewWorkspacesPluginPrivate		*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_PLUGIN(self));
	g_return_if_fail(inID && *inID);
	g_return_if_fail(self->priv->id==NULL);
	g_return_if_fail(self->priv->state==XFCE_OVERVIEW_WORKSPACES_PLUGIN_STATE_NONE);

	priv=self->priv;

	/* Set value if changed */
	if(g_strcmp0(priv->id, inID)!=0)
	{
		XfceOverviewWorkspacesSettings			*settings;
		const gchar					*basePath;
		gchar						*newPath;

		/* Freeze notification */
		g_object_freeze_notify(G_OBJECT(self));

		/* Set value */
		if(priv->id) g_free(priv->id);
		priv->id=g_strdup(inID);

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesPluginProperties[PROP_ID]);

		/* Set up new configuration and data base path from plugin ID and notify
		 * about changes if any.
		 */
		settings=xfce_overview_workspaces_core_get_settings(NULL);

		newPath=NULL;
		if(settings)
		{
			basePath=xfce_overview_workspaces_settings_get_config_path(settings);
			newPath=g_build_filename(basePath, "plugins", priv->id, NULL);
		}

		if(g_strcmp0(newPath, priv->configPath)!=0)
		{
			if(priv->configPath)
			{
				g_free(priv->configPath);
				priv->configPath=NULL;
			}

			if(newPath) priv->configPath=newPath;

			/* Notify about property change */
			g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesPluginProperties[PROP_CONFIG_PATH]);
		}

		newPath=NULL;
		if(settings)
		{
			basePath=xfce_overview_workspaces_settings_get_data_path(settings);
			newPath=g_build_filename(basePath, "plugins", priv->id, NULL);
		}

		if(g_strcmp0(newPath, priv->dataPath)!=0)
		{
			if(priv->dataPath)
			{
				g_free(priv->dataPath);
				priv->dataPath=NULL;
			}

			if(newPath) priv->dataPath=newPath;

			/* Notify about property change */
			g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesPluginProperties[PROP_DATA_PATH]);
		}

		/* Thaw notification */
		g_object_thaw_notify(G_OBJECT(self));
	}
}

/* Set flags for plugin */
static void _xfce_overview_workspaces_plugin_set_flags(XfceOverviewWorkspacesPlugin *self, XfceOverviewWorkspacesPluginFlag inFlags)
{
	XfceOverviewWorkspacesPluginPrivate		*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_PLUGIN(self));
	g_return_if_fail(self->priv->flags==XFCE_OVERVIEW_WORKSPACES_PLUGIN_FLAG_NONE);
	g_return_if_fail(self->priv->state==XFCE_OVERVIEW_WORKSPACES_PLUGIN_STATE_NONE);

	priv=self->priv;

	/* Set value if changed */
	if(priv->flags!=inFlags)
	{
		/* Set value */
		priv->flags=inFlags;

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesPluginProperties[PROP_FLAGS]);
	}
}

/* Set name for plugin */
static void _xfce_overview_workspaces_plugin_set_name(XfceOverviewWorkspacesPlugin *self, const gchar *inName)
{
	XfceOverviewWorkspacesPluginPrivate		*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_PLUGIN(self));
	g_return_if_fail(self->priv->name==NULL);
	g_return_if_fail(self->priv->state==XFCE_OVERVIEW_WORKSPACES_PLUGIN_STATE_NONE);

	priv=self->priv;

	/* Set value if changed */
	if(g_strcmp0(priv->name, inName)!=0)
	{
		/* Set value */
		if(priv->name)
		{
			g_free(priv->name);
			priv->name=NULL;
		}

		if(inName) priv->name=g_strdup(inName);

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesPluginProperties[PROP_NAME]);
	}
}

/* Set description for plugin */
static void _xfce_overview_workspaces_plugin_set_description(XfceOverviewWorkspacesPlugin *self, const gchar *inDescription)
{
	XfceOverviewWorkspacesPluginPrivate		*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_PLUGIN(self));
	g_return_if_fail(self->priv->description==NULL);
	g_return_if_fail(self->priv->state==XFCE_OVERVIEW_WORKSPACES_PLUGIN_STATE_NONE);

	priv=self->priv;

	/* Set value if changed */
	if(g_strcmp0(priv->description, inDescription)!=0)
	{
		/* Set value */
		if(priv->description)
		{
			g_free(priv->description);
			priv->description=NULL;
		}

		if(inDescription) priv->description=g_strdup(inDescription);

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesPluginProperties[PROP_DESCRIPTION]);
	}
}

/* Set author for plugin */
static void _xfce_overview_workspaces_plugin_set_author(XfceOverviewWorkspacesPlugin *self, const gchar *inAuthor)
{
	XfceOverviewWorkspacesPluginPrivate		*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_PLUGIN(self));
	g_return_if_fail(self->priv->author==NULL);
	g_return_if_fail(self->priv->state==XFCE_OVERVIEW_WORKSPACES_PLUGIN_STATE_NONE);

	priv=self->priv;

	/* Set value if changed */
	if(g_strcmp0(priv->author, inAuthor)!=0)
	{
		/* Set value */
		if(priv->author)
		{
			g_free(priv->author);
			priv->author=NULL;
		}

		if(inAuthor) priv->author=g_strdup(inAuthor);

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesPluginProperties[PROP_AUTHOR]);
	}
}

/* Set copyright for plugin */
static void _xfce_overview_workspaces_plugin_set_copyright(XfceOverviewWorkspacesPlugin *self, const gchar *inCopyright)
{
	XfceOverviewWorkspacesPluginPrivate		*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_PLUGIN(self));
	g_return_if_fail(self->priv->copyright==NULL);
	g_return_if_fail(self->priv->state==XFCE_OVERVIEW_WORKSPACES_PLUGIN_STATE_NONE);

	priv=self->priv;

	/* Set value if changed */
	if(g_strcmp0(priv->copyright, inCopyright)!=0)
	{
		/* Set value */
		if(priv->copyright)
		{
			g_free(priv->copyright);
			priv->copyright=NULL;
		}

		if(inCopyright) priv->copyright=g_strdup(inCopyright);

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesPluginProperties[PROP_COPYRIGHT]);
	}
}

/* Set license for plugin */
static void _xfce_overview_workspaces_plugin_set_license(XfceOverviewWorkspacesPlugin *self, const gchar *inLicense)
{
	XfceOverviewWorkspacesPluginPrivate		*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_PLUGIN(self));
	g_return_if_fail(self->priv->license==NULL);
	g_return_if_fail(self->priv->state==XFCE_OVERVIEW_WORKSPACES_PLUGIN_STATE_NONE);

	priv=self->priv;

	/* Set value if changed */
	if(g_strcmp0(priv->license, inLicense)!=0)
	{
		/* Set value */
		if(priv->license)
		{
			g_free(priv->license);
			priv->license=NULL;
		}

		if(inLicense) priv->license=g_strdup(inLicense);

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesPluginProperties[PROP_LICENSE]);
	}
}

/* Set plugin settings object for plugin */
static void _xfce_overview_workspaces_plugin_set_settings(XfceOverviewWorkspacesPlugin *self, XfceOverviewWorkspacesPluginSettings *inSettings)
{
	XfceOverviewWorkspacesPluginPrivate		*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_PLUGIN(self));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_PLUGIN_SETTINGS(inSettings));
	g_return_if_fail(self->priv->settings==NULL);
	g_return_if_fail(self->priv->state==XFCE_OVERVIEW_WORKSPACES_PLUGIN_STATE_NONE);

	priv=self->priv;

	/* Set value if changed */
	if(inSettings!=priv->settings)
	{
		/* Set value */
		if(priv->settings)
		{
			g_object_unref(priv->settings);
			priv->settings=NULL;
		}

		if(inSettings)
		{
			priv->settings=g_object_ref(inSettings);
		}

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesPluginProperties[PROP_SETTINGS]);
	}
}

/* Destroy user data */
static void _xfce_overview_workspaces_plugin_destroy_user_data(XfceOverviewWorkspacesPlugin *self)
{
	XfceOverviewWorkspacesPluginPrivate		*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_PLUGIN(self));

	priv=self->priv;

	/* Check if user data is set and call its destruction callback function
	 * if it was set also.
	 */
	if(priv->userData)
	{
		/* Call destruction callback function with user data if available */
		if(priv->userDataDestroyCallback)
		{
			(priv->userDataDestroyCallback)(priv->userData);
		}

		/* Unset user data and destruction callback */
		priv->userData=NULL;
		priv->userDataDestroyCallback=NULL;
	}
}


/* IMPLEMENTATION: GTypeModule */

/* Load and initialize plugin */
static gboolean _xfce_overview_workspaces_plugin_load(GTypeModule *inModule)
{
	XfceOverviewWorkspacesPlugin			*self;
	XfceOverviewWorkspacesPluginPrivate	*priv;
	guint						signalID;
	gulong						handlerID;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_PLUGIN(inModule), FALSE);
	g_return_val_if_fail(G_IS_TYPE_MODULE(inModule), FALSE);

	self=XFCE_OVERVIEW_WORKSPACES_PLUGIN(inModule);
	priv=self->priv;

	/* Reset last loading error if set */
	if(priv->lastLoadingError)
	{
		g_free(priv->lastLoadingError);
		priv->lastLoadingError=NULL;
	}

	/* Check if path to plugin was set and exists */
	if(!priv->filename)
	{
		priv->lastLoadingError=g_strdup("Missing path to plugin");
		return(FALSE);
	}

	if(!g_file_test(priv->filename, G_FILE_TEST_IS_REGULAR))
	{
		priv->lastLoadingError=g_strdup_printf("Path '%s' does not exist", priv->filename);
		return(FALSE);
	}

	/* Check that plugin is not in any state */
	if(priv->state!=XFCE_OVERVIEW_WORKSPACES_PLUGIN_STATE_NONE)
	{
		priv->lastLoadingError=
			g_strdup_printf("Bad state '%s' - expected '%s'",
							_xfce_overview_workspaces_plugin_get_plugin_state_value_name(priv->state),
							_xfce_overview_workspaces_plugin_get_plugin_state_value_name(XFCE_OVERVIEW_WORKSPACES_PLUGIN_STATE_NONE));
		return(FALSE);
	}

	/* Open plugin module */
	if(priv->module)
	{
		priv->lastLoadingError=g_strdup("Plugin was already initialized");
		return(FALSE);
	}

	priv->module=g_module_open(priv->filename, G_MODULE_BIND_LAZY | G_MODULE_BIND_LOCAL);
	if(!priv->module)
	{
		priv->lastLoadingError=g_strdup(g_module_error());
		return(FALSE);
	}

	/* Check that plugin provides all necessary functions and get the address
	 * to these functions.
	 */
	if(!g_module_symbol(priv->module, XFCE_OVERVIEW_WORKSPACES_PLUGIN_FUNCTION_NAME_INITIALIZE, (gpointer*)&priv->initialize))
	{
		priv->lastLoadingError=g_strdup(g_module_error());
		return(FALSE);
	}

	/* Initialize plugin */
	if(priv->initialize)
	{
		priv->initialize(self);
	}
		else
		{
			/* If we get here the virtual function was not overridden */
			priv->lastLoadingError=g_strdup_printf("Plugin does not implement required function %s", XFCE_OVERVIEW_WORKSPACES_PLUGIN_FUNCTION_NAME_INITIALIZE);
			g_critical("Loading plugin at '%s' failed: %s", priv->filename, priv->lastLoadingError);
			return(FALSE);
		}

	/* Check that plugin has required properties set */
	if(!priv->id)
	{
		priv->lastLoadingError=g_strdup("Plugin did not set required ID");
		return(FALSE);
	}

	/* Check if plugin is valid, that means that plugin can be enabled and disabled */
	signalID=g_signal_lookup("enable", XFCE_OVERVIEW_WORKSPACES_TYPE_PLUGIN);
	handlerID=g_signal_handler_find(self,
									G_SIGNAL_MATCH_ID,
									signalID,
									0,
									NULL,
									NULL,
									NULL);
	if(!handlerID)
	{
		priv->lastLoadingError=g_strdup("Plugin cannot be enabled");
		g_critical("Loading plugin at '%s' failed: %s", priv->filename, priv->lastLoadingError);
		return(FALSE);
	}

	signalID=g_signal_lookup("disable", XFCE_OVERVIEW_WORKSPACES_TYPE_PLUGIN);
	handlerID=g_signal_handler_find(self,
									G_SIGNAL_MATCH_ID,
									signalID,
									0,
									NULL,
									NULL,
									NULL);
	if(!handlerID)
	{
		priv->lastLoadingError=g_strdup("Plugin cannot be disabled");
		g_critical("Loading plugin at '%s' failed: %s", priv->filename, priv->lastLoadingError);
		return(FALSE);
	}

	/* Set state of plugin */
	priv->state=XFCE_OVERVIEW_WORKSPACES_PLUGIN_STATE_INITIALIZED;

	/* If we get here then loading and initializing plugin was successful */
	XFCE_OVERVIEW_WORKSPACES_DEBUG(self, PLUGINS,
						"Loaded plugin '%s' successfully:\n  File: %s\n  Name: %s\n  Description: %s\n  Author: %s\n  Copyright: %s\n  License: %s",
						priv->id,
						priv->filename,
						priv->name ? priv->name : "",
						priv->description ? priv->description : "",
						priv->author ? priv->author : "",
						priv->copyright ? priv->copyright : "",
						priv->license ? priv->license : "");

	return(TRUE);
}

/* Disable and unload plugin */
static void _xfce_overview_workspaces_plugin_unload(GTypeModule *inModule)
{
	XfceOverviewWorkspacesPlugin			*self;
	XfceOverviewWorkspacesPluginPrivate	*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_PLUGIN(inModule));
	g_return_if_fail(G_IS_TYPE_MODULE(inModule));

	self=XFCE_OVERVIEW_WORKSPACES_PLUGIN(inModule);
	priv=self->priv;

	/* Disable plugin if it is still enabled */
	if(priv->state==XFCE_OVERVIEW_WORKSPACES_PLUGIN_STATE_ENABLED)
	{
		XFCE_OVERVIEW_WORKSPACES_DEBUG(self, PLUGINS,
							"Disabing plugin '%s' before unloading module",
							priv->id);
		xfce_overview_workspaces_plugin_disable(self);
	}

	/* Close plugin module */
	if(priv->module)
	{
		/* Close module */
		if(!g_module_close(priv->module))
		{
			g_warning("Plugin '%s' could not be unloaded successfully: %s",
						priv->id ? priv->id : "Unknown",
						g_module_error());
			return;
		}

		/* Unset module and function pointers from plugin module */
		priv->initialize=NULL;

		priv->module=NULL;
	}

	/* Set state of plugin to uninitialzed */
	priv->state=XFCE_OVERVIEW_WORKSPACES_PLUGIN_STATE_NONE;
}

/* IMPLEMENTATION: GObject */

/* Dispose this object */
static void _xfce_overview_workspaces_plugin_dispose(GObject *inObject)
{
	XfceOverviewWorkspacesPlugin			*self=XFCE_OVERVIEW_WORKSPACES_PLUGIN(inObject);
	XfceOverviewWorkspacesPluginPrivate	*priv=self->priv;

	/* First of all destroy user data as long as this object was not disposed */
	_xfce_overview_workspaces_plugin_destroy_user_data(self);

	/* Release allocated resources */
	if(priv->module)
	{
		_xfce_overview_workspaces_plugin_unload(G_TYPE_MODULE(self));
	}

	if(priv->lastLoadingError)
	{
		g_free(priv->lastLoadingError);
		priv->lastLoadingError=NULL;
	}

	if(priv->id)
	{
		g_free(priv->id);
		priv->id=NULL;
	}

	if(priv->name)
	{
		g_free(priv->name);
		priv->name=NULL;
	}

	if(priv->description)
	{
		g_free(priv->description);
		priv->description=NULL;
	}

	if(priv->author)
	{
		g_free(priv->author);
		priv->author=NULL;
	}

	if(priv->copyright)
	{
		g_free(priv->copyright);
		priv->copyright=NULL;
	}

	if(priv->license)
	{
		g_free(priv->license);
		priv->license=NULL;
	}

	if(priv->settings)
	{
		g_object_unref(priv->settings);
		priv->settings=NULL;
	}

	/* Sanity checks that module was unloaded - at least by us */
	g_assert(priv->initialize==NULL);

	/* Call parent's class dispose method */
	G_OBJECT_CLASS(xfce_overview_workspaces_plugin_parent_class)->dispose(inObject);
}

/* Set/get properties */
static void _xfce_overview_workspaces_plugin_set_property(GObject *inObject,
											guint inPropID,
											const GValue *inValue,
											GParamSpec *inSpec)
{
	XfceOverviewWorkspacesPlugin			*self=XFCE_OVERVIEW_WORKSPACES_PLUGIN(inObject);
	
	switch(inPropID)
	{
		case PROP_FILENAME:
			_xfce_overview_workspaces_plugin_set_filename(self, g_value_get_string(inValue));
			break;

		case PROP_ID:
			_xfce_overview_workspaces_plugin_set_id(self, g_value_get_string(inValue));
			break;

		case PROP_FLAGS:
			_xfce_overview_workspaces_plugin_set_flags(self, g_value_get_flags(inValue));
			break;

		case PROP_NAME:
			_xfce_overview_workspaces_plugin_set_name(self, g_value_get_string(inValue));
			break;

		case PROP_DESCRIPTION:
			_xfce_overview_workspaces_plugin_set_description(self, g_value_get_string(inValue));
			break;

		case PROP_AUTHOR:
			_xfce_overview_workspaces_plugin_set_author(self, g_value_get_string(inValue));
			break;

		case PROP_COPYRIGHT:
			_xfce_overview_workspaces_plugin_set_copyright(self, g_value_get_string(inValue));
			break;

		case PROP_LICENSE:
			_xfce_overview_workspaces_plugin_set_license(self, g_value_get_string(inValue));
			break;

		case PROP_SETTINGS:
			_xfce_overview_workspaces_plugin_set_settings(self, g_value_get_object(inValue));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(inObject, inPropID, inSpec);
			break;
	}
}

static void _xfce_overview_workspaces_plugin_get_property(GObject *inObject,
												guint inPropID,
												GValue *outValue,
												GParamSpec *inSpec)
{
	XfceOverviewWorkspacesPlugin			*self=XFCE_OVERVIEW_WORKSPACES_PLUGIN(inObject);
	XfceOverviewWorkspacesPluginPrivate	*priv=self->priv;

	switch(inPropID)
	{
		case PROP_FILENAME:
			g_value_set_string(outValue, priv->filename);
			break;

		case PROP_ID:
			g_value_set_string(outValue, priv->id);
			break;

		case PROP_FLAGS:
			g_value_set_flags(outValue, priv->flags);
			break;

		case PROP_NAME:
			g_value_set_string(outValue, priv->name);
			break;

		case PROP_DESCRIPTION:
			g_value_set_string(outValue, priv->description);
			break;

		case PROP_AUTHOR:
			g_value_set_string(outValue, priv->author);
			break;

		case PROP_COPYRIGHT:
			g_value_set_string(outValue, priv->copyright);
			break;

		case PROP_LICENSE:
			g_value_set_string(outValue, priv->license);
			break;

		case PROP_SETTINGS:
			g_value_set_object(outValue, priv->settings);
			break;

		case PROP_CONFIG_PATH:
			g_value_set_string(outValue, priv->configPath);
			break;

		case PROP_DATA_PATH:
			g_value_set_string(outValue, priv->dataPath);
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
static void xfce_overview_workspaces_plugin_class_init(XfceOverviewWorkspacesPluginClass *klass)
{
	GTypeModuleClass		*moduleClass=G_TYPE_MODULE_CLASS(klass);
	GObjectClass			*gobjectClass=G_OBJECT_CLASS(klass);

	/* Override functions */
	moduleClass->load=_xfce_overview_workspaces_plugin_load;
	moduleClass->unload=_xfce_overview_workspaces_plugin_unload;

	gobjectClass->set_property=_xfce_overview_workspaces_plugin_set_property;
	gobjectClass->get_property=_xfce_overview_workspaces_plugin_get_property;
	gobjectClass->dispose=_xfce_overview_workspaces_plugin_dispose;

	/* Define properties */
	/**
	 * XfceOverviewWorkspacesPlugin:filename:
	 *
	 * The path and file name of this plugin.
	 */
	XfceOverviewWorkspacesPluginProperties[PROP_FILENAME]=
		g_param_spec_string("filename",
							"File name",
							"Path and file name of this plugin",
							NULL,
							G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY);

	/**
	 * XfceOverviewWorkspacesPlugin:id:
	 *
	 * The unique ID of this plugin.
	 */
	XfceOverviewWorkspacesPluginProperties[PROP_ID]=
		g_param_spec_string("id",
							"ID",
							"The unique ID for this plugin",
							NULL,
							G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY);

	/**
	 * XfceOverviewWorkspacesPlugin:flags:
	 *
	 * None, one or more flags of type #XfceOverviewWorkspacesPluginFlag defining behaviour
	 * of this plugin.
	 */
	XfceOverviewWorkspacesPluginProperties[PROP_FLAGS]=
		g_param_spec_flags("flags",
							"Flags",
							"Flags defining behaviour of this plugin",
							XFCE_OVERVIEW_WORKSPACES_TYPE_PLUGIN_FLAG,
							XFCE_OVERVIEW_WORKSPACES_PLUGIN_FLAG_NONE,
							G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * XfceOverviewWorkspacesPlugin:name:
	 *
	 * The name of this plugin.
	 */
	XfceOverviewWorkspacesPluginProperties[PROP_NAME]=
		g_param_spec_string("name",
							"name",
							"Name of plugin",
							NULL,
							G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * XfceOverviewWorkspacesPlugin:description:
	 *
	 * The description for this plugin.
	 */
	XfceOverviewWorkspacesPluginProperties[PROP_DESCRIPTION]=
		g_param_spec_string("description",
							"Description",
							"A short description about this plugin",
							NULL,
							G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * XfceOverviewWorkspacesPlugin:author:
	 *
	 * The author of this plugin.
	 */
	XfceOverviewWorkspacesPluginProperties[PROP_AUTHOR]=
		g_param_spec_string("author",
							"Author",
							"The author of this plugin",
							NULL,
							G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * XfceOverviewWorkspacesPlugin:copyright:
	 *
	 * The copyright message of this plugin.
	 */
	XfceOverviewWorkspacesPluginProperties[PROP_COPYRIGHT]=
		g_param_spec_string("copyright",
							"Copyright",
							"The copyright of this plugin which usually contains year of development",
							NULL,
							G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * XfceOverviewWorkspacesPlugin:license:
	 *
	 * The license of this plugin.
	 */
	XfceOverviewWorkspacesPluginProperties[PROP_LICENSE]=
		g_param_spec_string("license",
							"License",
							"The license of this plugin",
							NULL,
							G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * XfceOverviewWorkspacesPlugin:settings:
	 *
	 * The settings object of type #XfceOverviewWorkspacesPluginSettings for this plugin.
	 */
	XfceOverviewWorkspacesPluginProperties[PROP_SETTINGS]=
		g_param_spec_object("settings",
								"Settings",
								"The plugin settings object of this plugin",
								XFCE_OVERVIEW_WORKSPACES_TYPE_PLUGIN_SETTINGS,
								G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * XfceOverviewWorkspacesPlugin:config-path:
	 *
	 * The base path of configuration files of this plugin or NULL if application
	 * specific configuration files is disabled.
	 */
	XfceOverviewWorkspacesPluginProperties[PROP_CONFIG_PATH]=
		g_param_spec_string("config-path",
								"Configuration path",
								"Base path to configuration files of this plugin",
								NULL,
								G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

	/**
	 * XfceOverviewWorkspacesPlugin:data-path:
	 *
	 * The base path of data files of this plugin or NULL if application specific
	 * data files is disabled.
	 */
	XfceOverviewWorkspacesPluginProperties[PROP_DATA_PATH]=
		g_param_spec_string("data-path",
								"Data path",
								"Base path to data files of this plugin",
								NULL,
								G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(gobjectClass, PROP_LAST, XfceOverviewWorkspacesPluginProperties);

	/* Define signals */
	/**
	 * XfceOverviewWorkspacesPlugin::enable:
	 * @self: The plugin
	 *
	 * The ::enable signal is an action and when emitted it causes the plugin
	 * at @self to get enabled.
	 */
	XfceOverviewWorkspacesPluginSignals[ACTION_ENABLE]=
		g_signal_new("enable",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
						G_STRUCT_OFFSET(XfceOverviewWorkspacesPluginClass, enable),
						NULL,
						NULL,
						g_cclosure_marshal_VOID__VOID,
						G_TYPE_NONE,
						0);

	/**
	 * XfceOverviewWorkspacesPlugin::disable:
	 * @self: The plugin
	 *
	 * The ::disable signal is an action and when emitted it causes the plugin
	 * at @self to get disabled.
	 */
	XfceOverviewWorkspacesPluginSignals[ACTION_DISABLE]=
		g_signal_new("disable",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
						G_STRUCT_OFFSET(XfceOverviewWorkspacesPluginClass, disable),
						NULL,
						NULL,
						g_cclosure_marshal_VOID__VOID,
						G_TYPE_NONE,
						0);

	/**
	 * XfceOverviewWorkspacesPlugin::configure:
	 * @self: The plugin
	 *
	 * The ::configure signal is an action and when emitted it will let the plugin
	 * at @self create any kind of object, usually an object derived from #GtkWidget,
	 * to configure the settings of this plugin.
	 *
	 * Return value: A #GObject to use to configure this plugin.
	 */
	XfceOverviewWorkspacesPluginSignals[ACTION_CONFIGURE]=
		g_signal_new("configure",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
						G_STRUCT_OFFSET(XfceOverviewWorkspacesPluginClass, configure),
						NULL,
						NULL,
						_xfce_overview_workspaces_marshal_OBJECT__VOID,
						G_TYPE_OBJECT,
						0);
}

/* Object initialization
 * Create private structure and set up default values
 */
static void xfce_overview_workspaces_plugin_init(XfceOverviewWorkspacesPlugin *self)
{
	XfceOverviewWorkspacesPluginPrivate	*priv;

	priv=self->priv=xfce_overview_workspaces_plugin_get_instance_private(self);

	/* Set up default values */
	priv->filename=NULL;
	priv->didLoad=FALSE;
	priv->module=NULL;
	priv->initialize=NULL;
	priv->state=XFCE_OVERVIEW_WORKSPACES_PLUGIN_STATE_NONE;
	priv->lastLoadingError=NULL;
	priv->userData=NULL;
	priv->userDataDestroyCallback=NULL;
	priv->configPath=NULL;
	priv->dataPath=NULL;

	priv->id=NULL;
	priv->flags=XFCE_OVERVIEW_WORKSPACES_PLUGIN_FLAG_NONE;
	priv->name=NULL;
	priv->description=NULL;
	priv->author=NULL;
	priv->copyright=NULL;
	priv->license=NULL;
	priv->settings=NULL;
}


/* IMPLEMENTATION: Errors */

GQuark xfce_overview_workspaces_plugin_error_quark(void)
{
	return(g_quark_from_static_string("xfce-overview-workspaces-plugin-error-quark"));
}


/* IMPLEMENTATION: Public API */

/**
 * xfce_overview_workspaces_plugin_new:
 *
 * Creates a plugin instance of type #XfceOverviewWorkspacesPlugin which will be loaded
 * from path at @inPluginFilename. The plugin ID is retrieved from the basename
 * of plugin's path, i.e. the file name without the absolute path and without
 * file extension.
 *
 * The plugin instance created is loaded and initialized but disabled by
 * default. This has to be done after successful creation by calling
 * xfce_overview_workspaces_plugin_enable().
 *
 * In case of errors %NULL will be returned and the error is set at
 * @outError.
 *
 * Return value: (transfer full): The disabled plugin instance of
 *   #XfceOverviewWorkspacesPlugin. Use g_object_unref() when done.
 */
XfceOverviewWorkspacesPlugin* xfce_overview_workspaces_plugin_new(const gchar *inPluginFilename, GError **outError)
{
	GObject			*plugin;
	gchar			*pluginBasename;
	gchar			*pluginID;

	g_return_val_if_fail(inPluginFilename && *inPluginFilename, NULL);
	g_return_val_if_fail(outError==NULL || *outError==NULL, NULL);

	/* Get plugin ID from filename */
	pluginBasename=g_filename_display_basename(inPluginFilename);
	if(!pluginBasename)
	{
		/* Set error */
		g_set_error(outError,
					XFCE_OVERVIEW_WORKSPACES_PLUGIN_ERROR,
					XFCE_OVERVIEW_WORKSPACES_PLUGIN_ERROR_ERROR,
					"Could not get plugin ID for file %s",
					inPluginFilename);

		/* Return NULL to indicate failure */
		return(NULL);
	}

	if(g_str_has_suffix(pluginBasename, G_MODULE_SUFFIX))
	{
		pluginID=g_strndup(pluginBasename, strlen(pluginBasename)-strlen(G_MODULE_SUFFIX)-1);
	}
		else
		{
			pluginID=g_strdup(pluginBasename);
		}

	/* Create object instance */
	plugin=g_object_new(XFCE_OVERVIEW_WORKSPACES_TYPE_PLUGIN,
						"filename", inPluginFilename,
						"id", pluginID,
						NULL);
	if(!plugin)
	{
		/* Set error */
		g_set_error(outError,
					XFCE_OVERVIEW_WORKSPACES_PLUGIN_ERROR,
					XFCE_OVERVIEW_WORKSPACES_PLUGIN_ERROR_ERROR,
					"Could not create plugin instance");

		/* Release allocated resources */
		if(pluginID) g_free(pluginID);
		if(pluginBasename) g_free(pluginBasename);

		/* Return NULL to indicate failure */
		return(NULL);
	}

	/* Load plugin */
	if(!g_type_module_use(G_TYPE_MODULE(plugin)))
	{
		/* Set error */
		g_set_error(outError,
					XFCE_OVERVIEW_WORKSPACES_PLUGIN_ERROR,
					XFCE_OVERVIEW_WORKSPACES_PLUGIN_ERROR_ERROR,
					"%s",
					_xfce_overview_workspaces_plugin_get_loading_error(XFCE_OVERVIEW_WORKSPACES_PLUGIN(plugin)));

		/* Release allocated resources */
		if(pluginID) g_free(pluginID);
		if(pluginBasename) g_free(pluginBasename);

		/* At this point we return NULL to indicate failure although the object
		 * instance (subclassing GTypeModule) now exists and it was tried to
		 * use it (via g_type_module_use). As describe in GObject documentation
		 * the object must not be unreffed via g_object_unref() but we also should
		 * not call g_type_module_unuse() because loading failed and the reference
		 */
		return(NULL);
	}

	/* Release allocated resources */
	if(pluginID) g_free(pluginID);
	if(pluginBasename) g_free(pluginBasename);

	/* Plugin loaded so return it */
	return(XFCE_OVERVIEW_WORKSPACES_PLUGIN(plugin));
}

/**
 * xfce_overview_workspaces_plugin_get_id:
 * @self: A #XfceOverviewWorkspacesPlugin
 *
 * Retrieve the ID of plugin at @self.
 *
 * Return value: The plugin ID
 */
const gchar* xfce_overview_workspaces_plugin_get_id(XfceOverviewWorkspacesPlugin *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_PLUGIN(self), NULL);

	return(self->priv->id);
}

/**
 * xfce_overview_workspaces_plugin_get_flags:
 * @self: A #XfceOverviewWorkspacesPlugin
 *
 * Retrieve the flags associated with plugin at @self.
 *
 * Return value: The flags of type #XfceOverviewWorkspacesPluginFlag
 */
XfceOverviewWorkspacesPluginFlag xfce_overview_workspaces_plugin_get_flags(XfceOverviewWorkspacesPlugin *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_PLUGIN(self), XFCE_OVERVIEW_WORKSPACES_PLUGIN_FLAG_NONE);

	return(self->priv->flags);
}

/* Set plugin information */

/**
 * xfce_overview_workspaces_plugin_set_info:
 * @self: A #XfceOverviewWorkspacesPlugin
 * @inFirstPropertyName: The name of the first informational property to set
 * @...: The value for the first informational property to set, followed
 *   optionally by more name/value pairs, followed by NULL
 *
 * Sets informational properties for plugin at @self. The plugin must not be
 * initialized or enabled.
 */
void xfce_overview_workspaces_plugin_set_info(XfceOverviewWorkspacesPlugin *self,
									const gchar *inFirstPropertyName, ...)
{
	XfceOverviewWorkspacesPluginPrivate		*priv;
	va_list							args;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_PLUGIN(self));

	priv=self->priv;

	/* Check that plugin is not initialized already */
	if(priv->state!=XFCE_OVERVIEW_WORKSPACES_PLUGIN_STATE_NONE)
	{
		g_critical("Setting plugin information for plugin '%s' at path '%s' failed: Plugin has state '%s'",
					priv->id ? priv->id : "Unknown",
					priv->filename,
					_xfce_overview_workspaces_plugin_get_plugin_state_value_name(priv->state));
		return;
	}

	/* Set up properties */
	va_start(args, inFirstPropertyName);
	g_object_set_valist(G_OBJECT(self), inFirstPropertyName, args);
	va_end(args);
}

/**
 * xfce_overview_workspaces_plugin_is_enabled:
 * @self: A #XfceOverviewWorkspacesPlugin
 *
 * Retrieve if plugin at @self was successfully loaded and enabled.
 *
 * Return value: %TRUE if plugin was enabled, otherwise %FALSE
 */
gboolean xfce_overview_workspaces_plugin_is_enabled(XfceOverviewWorkspacesPlugin *self)
{
	XfceOverviewWorkspacesPluginPrivate		*priv;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_PLUGIN(self), FALSE);

	priv=self->priv;

	/* Only return TRUE if state is enabled */
	if(priv->state==XFCE_OVERVIEW_WORKSPACES_PLUGIN_STATE_ENABLED) return(TRUE);

	/* If we get here plugin is not in enabled state so return FALSE */
	return(FALSE);
}

/**
 * xfce_overview_workspaces_plugin_enable:
 * @self: A #XfceOverviewWorkspacesPlugin
 *
 * Enables the plugin at @self.
 */
void xfce_overview_workspaces_plugin_enable(XfceOverviewWorkspacesPlugin *self)
{
	XfceOverviewWorkspacesPluginPrivate		*priv;
	gboolean						result;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_PLUGIN(self));

	priv=self->priv;
	result=FALSE;

	/* Do nothing and return immediately if plugin is enabled already */
	if(priv->state==XFCE_OVERVIEW_WORKSPACES_PLUGIN_STATE_ENABLED)
	{
		XFCE_OVERVIEW_WORKSPACES_DEBUG(self, PLUGINS,
							"Plugin '%s' is already enabled",
							priv->id);
		return;
	}

	/* Check that plugin is initialized */
	if(priv->state!=XFCE_OVERVIEW_WORKSPACES_PLUGIN_STATE_INITIALIZED)
	{
		g_critical("Enabling plugin '%s' failed: Bad state '%s' - expected '%s'",
					priv->id ? priv->id : "Unknown",
					_xfce_overview_workspaces_plugin_get_plugin_state_value_name(priv->state),
					_xfce_overview_workspaces_plugin_get_plugin_state_value_name(XFCE_OVERVIEW_WORKSPACES_PLUGIN_STATE_INITIALIZED));
		return;
	}

	/* Emit signal action 'enable' to enable plugin */
	g_signal_emit(self, XfceOverviewWorkspacesPluginSignals[ACTION_ENABLE], 0, &result);
	XFCE_OVERVIEW_WORKSPACES_DEBUG(self, PLUGINS,
						"Plugin '%s' enabled",
						priv->id);

	/* Set enabled state */
	priv->state=XFCE_OVERVIEW_WORKSPACES_PLUGIN_STATE_ENABLED;
}

/**
 * xfce_overview_workspaces_plugin_disable:
 * @self: A #XfceOverviewWorkspacesPlugin
 *
 * Disables the plugin at @self.
 */
void xfce_overview_workspaces_plugin_disable(XfceOverviewWorkspacesPlugin *self)
{
	XfceOverviewWorkspacesPluginPrivate		*priv;
	gboolean						result;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_PLUGIN(self));

	priv=self->priv;

	/* Do nothing and return immediately if plugin is not enabled */
	if(priv->state!=XFCE_OVERVIEW_WORKSPACES_PLUGIN_STATE_ENABLED)
	{
		XFCE_OVERVIEW_WORKSPACES_DEBUG(self, PLUGINS,
							"Plugin '%s' is already disabled",
							priv->id);
		return;
	}

	/* Emit signal action 'disable' to disable plugin */
	g_signal_emit(self, XfceOverviewWorkspacesPluginSignals[ACTION_DISABLE], 0, &result);
	XFCE_OVERVIEW_WORKSPACES_DEBUG(self, PLUGINS,
						"Plugin '%s' disabled",
						priv->id);

	/* Set disabled state, i.e. revert to initialized state */
	priv->state=XFCE_OVERVIEW_WORKSPACES_PLUGIN_STATE_INITIALIZED;
}

/**
 * xfce_overview_workspaces_plugin_get_user_data:
 * @self: A #XfceOverviewWorkspacesPlugin
 *
 * Retrieve the user data of plugin at @self.
 *
 * Return value: The user data
 */
gpointer xfce_overview_workspaces_plugin_get_user_data(XfceOverviewWorkspacesPlugin *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_PLUGIN(self), NULL);

	return(self->priv->userData);
}

/**
 * xfce_overview_workspaces_plugin_set_user_data:
 * @self: A #XfceOverviewWorkspacesPlugin
 * @inUserData: The user data to set
 *
 * Sets the user data @inUserData at plugin @self but without a callback
 * function to call when user data is destroyed.
 */
void xfce_overview_workspaces_plugin_set_user_data(XfceOverviewWorkspacesPlugin *self, gpointer inUserData)
{
	xfce_overview_workspaces_plugin_set_user_data_full(self, inUserData, NULL);
}

/**
 * xfce_overview_workspaces_plugin_set_user_data_full:
 * @self: A #XfceOverviewWorkspacesPlugin
 * @inUserData: The user data to set
 * @inDestroyCallback: The callback to call when destructing user data
 *
 * Sets the user data at @inUserData at plugin at @self with a callback
 * function at @inDestroyCallback which is called when user data will be
 * destroyed. The destruction callback will always be set even if user
 * data did not change. The current user data at plugin at @self will be
 * destroyed with the current destruction callback if user data changes
 * before the new user data at @inUserData and the new destruction
 * callback at @inDestroyCallback is set.
 */
void xfce_overview_workspaces_plugin_set_user_data_full(XfceOverviewWorkspacesPlugin *self, gpointer inUserData, GDestroyNotify inDestroyCallback)
{
	XfceOverviewWorkspacesPluginPrivate		*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_PLUGIN(self));

	priv=self->priv;

	/* Set value if changed */
	if(priv->userData!=inUserData)
	{
		/* Destroy old user data */
		_xfce_overview_workspaces_plugin_destroy_user_data(self);

		/* Set value */
		priv->userData=inUserData;
	}

	/* Set new destroy callback functions */
	priv->userDataDestroyCallback=inDestroyCallback;
}

/**
 * xfce_overview_workspaces_plugin_get_settings:
 * @self: A #XfceOverviewWorkspacesPlugin
 *
 * Retrieve the settings object of plugin at @self.
 *
 * Return value: The setting object of plugin
 */
XfceOverviewWorkspacesPluginSettings* xfce_overview_workspaces_plugin_get_settings(XfceOverviewWorkspacesPlugin *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_PLUGIN(self), NULL);

	return(self->priv->settings);
}

/**
 * xfce_overview_workspaces_plugin_get_config_path:
 * @self: A #XfceOverviewWorkspacesPlugin
 *
 * Retrieves the base path of configuration files for plugin at @self.
 *
 * NOTE: To retrieve the base path of configuration files for the application,
 * use xfce_overview_workspaces_settings_get_config_path() instead.
 *
 * Return value: The base path of configuration files for this plugin or %NULL
 *   if support for configuration files is disabled.
*/
const gchar* xfce_overview_workspaces_plugin_get_config_path(XfceOverviewWorkspacesPlugin *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_PLUGIN(self), NULL);

	return(self->priv->configPath);
}

/**
 * xfce_overview_workspaces_plugin_get_data_path:
 * @self: A #XfceOverviewWorkspacesPlugin
 *
 * Retrieves the base path of data files for plugin at @self.
 *
 * NOTE: To retrieve the base path of data files for the application, use
 * xfce_overview_workspaces_settings_get_data_path() instead.
 *
 * Return value: The base path of data files for this plugin or %NULL if
 *   support for data files is disabled.
 */
const gchar* xfce_overview_workspaces_plugin_get_data_path(XfceOverviewWorkspacesPlugin *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_PLUGIN(self), NULL);

	return(self->priv->dataPath);
}
