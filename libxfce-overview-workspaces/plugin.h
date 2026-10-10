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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_PLUGIN__
#define __LIBXFCE_OVERVIEW_WORKSPACES_PLUGIN__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <glib-object.h>
#include <gmodule.h>

#include "plugin-settings.h"

G_BEGIN_DECLS

/* Public definitions */
/**
 * XfceOverviewWorkspacesPluginFlag:
 * @XFCE_OVERVIEW_WORKSPACES_PLUGIN_FLAG_NONE: Plugin does not request anything special.
 * @XFCE_OVERVIEW_WORKSPACES_PLUGIN_FLAG_EARLY_INITIALIZATION: Plugin requests to get enabled before the stage is initialized
 *
 * Flags defining behaviour of this XfceOverviewWorkspacesPlugin.
 */
typedef enum /*< flags,prefix=XFCE_OVERVIEW_WORKSPACES_PLUGIN_FLAG >*/
{
	XFCE_OVERVIEW_WORKSPACES_PLUGIN_FLAG_NONE=0,

	XFCE_OVERVIEW_WORKSPACES_PLUGIN_FLAG_EARLY_INITIALIZATION=1 << 0,
} XfceOverviewWorkspacesPluginFlag;


/* Helper macros to declare, define and register GObject types in plugins */
#define XFCE_OVERVIEW_WORKSPACES_DECLARE_PLUGIN_TYPE(inFunctionNamePrefix) \
	void inFunctionNamePrefix##_register_plugin_type(XfceOverviewWorkspacesPlugin *inPlugin);

#define XFCE_OVERVIEW_WORKSPACES_DEFINE_PLUGIN_TYPE(inFunctionNamePrefix) \
	void inFunctionNamePrefix##_register_plugin_type(XfceOverviewWorkspacesPlugin *inPlugin) \
	{ \
		inFunctionNamePrefix##_register_type(G_TYPE_MODULE(inPlugin)); \
	}

#define XFCE_OVERVIEW_WORKSPACES_REGISTER_PLUGIN_TYPE(self, inFunctionNamePrefix) \
	inFunctionNamePrefix##_register_plugin_type(XFCE_OVERVIEW_WORKSPACES_PLUGIN(self));


/* Object declaration */
#define XFCE_OVERVIEW_WORKSPACES_TYPE_PLUGIN				(xfce_overview_workspaces_plugin_get_type())
#define XFCE_OVERVIEW_WORKSPACES_PLUGIN(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_PLUGIN, XfceOverviewWorkspacesPlugin))
#define XFCE_OVERVIEW_WORKSPACES_IS_PLUGIN(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_PLUGIN))
#define XFCE_OVERVIEW_WORKSPACES_PLUGIN_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_PLUGIN, XfceOverviewWorkspacesPluginClass))
#define XFCE_OVERVIEW_WORKSPACES_IS_PLUGIN_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_PLUGIN))
#define XFCE_OVERVIEW_WORKSPACES_PLUGIN_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_PLUGIN, XfceOverviewWorkspacesPluginClass))

typedef struct _XfceOverviewWorkspacesPlugin			XfceOverviewWorkspacesPlugin;
typedef struct _XfceOverviewWorkspacesPluginClass		XfceOverviewWorkspacesPluginClass;
typedef struct _XfceOverviewWorkspacesPluginPrivate	XfceOverviewWorkspacesPluginPrivate;

/**
 * XfceOverviewWorkspacesPlugin:
 *
 * The #XfceOverviewWorkspacesPlugin structure contains only private data and
 * should be accessed using the provided API
 */
struct _XfceOverviewWorkspacesPlugin
{
	/*< private >*/
	/* Parent instance */
	GTypeModule						parent_instance;

	/* Private structure */
	XfceOverviewWorkspacesPluginPrivate		*priv;
};

/**
 * XfceOverviewWorkspacesPluginClass:
 * @enable: The class closure for enable action signal
 * @disable: The class closure for disable action signal
 * @configure: The class closure for configure action signal
 *
 * The #XfceOverviewWorkspacesPluginClass class structure
 */
struct _XfceOverviewWorkspacesPluginClass
{
	/*< private >*/
	/* Parent class */
	GTypeModuleClass				parent_class;

	/*< public >*/
	/* Virtual functions */
	void (*enable)(XfceOverviewWorkspacesPlugin *self);
	void (*disable)(XfceOverviewWorkspacesPlugin *self);

	GObject* (*configure)(XfceOverviewWorkspacesPlugin *self);
};

/* Error */
#define XFCE_OVERVIEW_WORKSPACES_PLUGIN_ERROR					(xfce_overview_workspaces_plugin_error_quark())

GQuark xfce_overview_workspaces_plugin_error_quark(void);

/**
 * XfceOverviewWorkspacesPluginErrorEnum:
 * @XFCE_OVERVIEW_WORKSPACES_PLUGIN_ERROR_NONE: No error at plugin
 * @XFCE_OVERVIEW_WORKSPACES_PLUGIN_ERROR_ERROR: Generic error code for errors at plugin
 *
 * Error codes returned by handling #XfceOverviewWorkspacesPlugin.
 */
typedef enum /*< prefix=XFCE_OVERVIEW_WORKSPACES_PLUGIN_ERROR >*/
{
	XFCE_OVERVIEW_WORKSPACES_PLUGIN_ERROR_NONE,
	XFCE_OVERVIEW_WORKSPACES_PLUGIN_ERROR_ERROR,
} XfceOverviewWorkspacesPluginErrorEnum;

/* Public API */
GType xfce_overview_workspaces_plugin_get_type(void) G_GNUC_CONST;

XfceOverviewWorkspacesPlugin* xfce_overview_workspaces_plugin_new(const gchar *inPluginFilename, GError **outError);

const gchar* xfce_overview_workspaces_plugin_get_id(XfceOverviewWorkspacesPlugin *self);
XfceOverviewWorkspacesPluginFlag xfce_overview_workspaces_plugin_get_flags(XfceOverviewWorkspacesPlugin *self);

void xfce_overview_workspaces_plugin_set_info(XfceOverviewWorkspacesPlugin *self,
									const gchar *inFirstPropertyName, ...)
									G_GNUC_NULL_TERMINATED;

gboolean xfce_overview_workspaces_plugin_is_enabled(XfceOverviewWorkspacesPlugin *self);
void xfce_overview_workspaces_plugin_enable(XfceOverviewWorkspacesPlugin *self);
void xfce_overview_workspaces_plugin_disable(XfceOverviewWorkspacesPlugin *self);

gpointer xfce_overview_workspaces_plugin_get_user_data(XfceOverviewWorkspacesPlugin *self);
void xfce_overview_workspaces_plugin_set_user_data(XfceOverviewWorkspacesPlugin *self, gpointer inUserData);
void xfce_overview_workspaces_plugin_set_user_data_full(XfceOverviewWorkspacesPlugin *self, gpointer inUserData, GDestroyNotify inDestroyCallback);

XfceOverviewWorkspacesPluginSettings* xfce_overview_workspaces_plugin_get_settings(XfceOverviewWorkspacesPlugin *self);

const gchar* xfce_overview_workspaces_plugin_get_config_path(XfceOverviewWorkspacesPlugin *self);
const gchar* xfce_overview_workspaces_plugin_get_data_path(XfceOverviewWorkspacesPlugin *self);

G_END_DECLS

#endif	/* __LIBXFCE_OVERVIEW_WORKSPACES_PLUGIN__ */
