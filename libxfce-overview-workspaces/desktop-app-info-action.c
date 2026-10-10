/*
 * desktop-app-info-action: An application action defined at desktop entry
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

/**
 * SECTION:desktop-app-info-action
 * @short_description: An application action defined at a desktop entry
 * @include: xfce-overview-workspaces/desktop-app-info-action.h
 *
 * A #XfceOverviewWorkspacesDesktopAppInfoAction provides information about an application
 * command as it is defined at a desktop entry - see #XfceOverviewWorkspacesDesktopAppInfo.
 * The information can be used to build so called "Jumplists" or "Quicklists" for
 * an application.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glib/gi18n-lib.h>

#include <libxfce-overview-workspaces/desktop-app-info-action.h>
#include <libxfce-overview-workspaces/debug.h>


/* Define this class in GObject system */
struct _XfceOverviewWorkspacesDesktopAppInfoActionPrivate
{
	/* Properties related */
	gchar				*name;
	gchar				*iconName;
	gchar				*command;
};

G_DEFINE_TYPE_WITH_PRIVATE(XfceOverviewWorkspacesDesktopAppInfoAction,
							xfce_overview_workspaces_desktop_app_info_action,
							G_TYPE_OBJECT)

/* Properties */
enum
{
	PROP_0,

	PROP_NAME,
	PROP_ICON_NAME,
	PROP_COMMAND,

	PROP_LAST
};

static GParamSpec* XfceOverviewWorkspacesDesktopAppInfoActionProperties[PROP_LAST]={ 0, };


/* IMPLEMENTATION: Private variables and methods */


/* IMPLEMENTATION: GObject */

/* Dispose this object */
static void _xfce_overview_workspaces_desktop_app_info_action_dispose(GObject *inObject)
{
	XfceOverviewWorkspacesDesktopAppInfoAction			*self=XFCE_OVERVIEW_WORKSPACES_DESKTOP_APP_INFO_ACTION(inObject);
	XfceOverviewWorkspacesDesktopAppInfoActionPrivate	*priv=self->priv;

	/* Release allocated variables */
	if(priv->name)
	{
		g_free(priv->name);
		priv->name=NULL;
	}

	if(priv->iconName)
	{
		g_free(priv->iconName);
		priv->iconName=NULL;
	}

	if(priv->command)
	{
		g_free(priv->command);
		priv->command=NULL;
	}

	/* Call parent's class dispose method */
	G_OBJECT_CLASS(xfce_overview_workspaces_desktop_app_info_action_parent_class)->dispose(inObject);
}

/* Set/get properties */
static void _xfce_overview_workspaces_desktop_app_info_action_set_property(GObject *inObject,
																guint inPropID,
																const GValue *inValue,
																GParamSpec *inSpec)
{
	XfceOverviewWorkspacesDesktopAppInfoAction			*self=XFCE_OVERVIEW_WORKSPACES_DESKTOP_APP_INFO_ACTION(inObject);

	switch(inPropID)
	{
		case PROP_NAME:
			xfce_overview_workspaces_desktop_app_info_action_set_name(self, g_value_get_string(inValue));
			break;

		case PROP_ICON_NAME:
			xfce_overview_workspaces_desktop_app_info_action_set_icon_name(self, g_value_get_string(inValue));
			break;

		case PROP_COMMAND:
			xfce_overview_workspaces_desktop_app_info_action_set_command(self, g_value_get_string(inValue));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(inObject, inPropID, inSpec);
			break;
	}
}

static void _xfce_overview_workspaces_desktop_app_info_action_get_property(GObject *inObject,
																guint inPropID,
																GValue *outValue,
																GParamSpec *inSpec)
{
	XfceOverviewWorkspacesDesktopAppInfoAction			*self=XFCE_OVERVIEW_WORKSPACES_DESKTOP_APP_INFO_ACTION(inObject);
	XfceOverviewWorkspacesDesktopAppInfoActionPrivate	*priv=self->priv;

	switch(inPropID)
	{
		case PROP_NAME:
			g_value_set_string(outValue, priv->name);
			break;

		case PROP_ICON_NAME:
			g_value_set_string(outValue, priv->iconName);
			break;

		case PROP_COMMAND:
			g_value_set_string(outValue, priv->command);
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
static void xfce_overview_workspaces_desktop_app_info_action_class_init(XfceOverviewWorkspacesDesktopAppInfoActionClass *klass)
{
	GObjectClass					*gobjectClass=G_OBJECT_CLASS(klass);

	/* Override functions */
	gobjectClass->dispose=_xfce_overview_workspaces_desktop_app_info_action_dispose;
	gobjectClass->set_property=_xfce_overview_workspaces_desktop_app_info_action_set_property;
	gobjectClass->get_property=_xfce_overview_workspaces_desktop_app_info_action_get_property;

	/* Define properties */
	/**
	 * XfceOverviewWorkspacesDesktopAppInfoAction:name:
	 *
	 * Name of the application action which can used as label in menu items or
	 * buttons. It is the display name for this action.
	 */
	XfceOverviewWorkspacesDesktopAppInfoActionProperties[PROP_NAME]=
		g_param_spec_string("name",
								"Name",
								"Name of the action",
								NULL,
								G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * XfceOverviewWorkspacesDesktopAppInfoAction:icon-name:
	 *
	 * Name of the icon associated with this application action which can be used
	 * in menu items or buttons.
	 */
	XfceOverviewWorkspacesDesktopAppInfoActionProperties[PROP_ICON_NAME]=
		g_param_spec_string("icon-name",
								"Icon name",
								"Icon name of action",
								NULL,
								G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * XfceOverviewWorkspacesDesktopAppInfoAction:command:
	 *
	 * The command to execute when this application action is launched.
	 */
	XfceOverviewWorkspacesDesktopAppInfoActionProperties[PROP_COMMAND]=
		g_param_spec_string("command",
								"Name",
								"Application command of action",
								NULL,
								G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(gobjectClass, PROP_LAST, XfceOverviewWorkspacesDesktopAppInfoActionProperties);
}

/* Object initialization
 * Create private structure and set up default values
 */
static void xfce_overview_workspaces_desktop_app_info_action_init(XfceOverviewWorkspacesDesktopAppInfoAction *self)
{
	XfceOverviewWorkspacesDesktopAppInfoActionPrivate	*priv;

	priv=self->priv=xfce_overview_workspaces_desktop_app_info_action_get_instance_private(self);

	/* Set up default values */
	priv->name=NULL;
	priv->iconName=NULL;
	priv->command=NULL;
}


/* IMPLEMENTATION: Public API */

/**
 * xfce_overview_workspaces_desktop_app_info_action_get_name:
 * @self: A #XfceOverviewWorkspacesDesktopAppInfoAction
 *
 * Retrieves the name of the application action at @self.
 *
 * Return value: A string with the name of the application action
 */
const gchar* xfce_overview_workspaces_desktop_app_info_action_get_name(XfceOverviewWorkspacesDesktopAppInfoAction *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_DESKTOP_APP_INFO_ACTION(self), NULL);

	return(self->priv->name);
}

/**
 * xfce_overview_workspaces_desktop_app_info_action_set_name:
 * @self: A #XfceOverviewWorkspacesDesktopAppInfoAction
 * @inName: The application action's name
 *
 * Sets the (display) name of the application action at @self. The name must not
 * be empty.
 */
void xfce_overview_workspaces_desktop_app_info_action_set_name(XfceOverviewWorkspacesDesktopAppInfoAction *self,
													const gchar *inName)
{
	XfceOverviewWorkspacesDesktopAppInfoActionPrivate		*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_DESKTOP_APP_INFO_ACTION(self));
	g_return_if_fail(inName && *inName);

	priv=self->priv;

	/* Set value if changed */
	if(g_strcmp0(priv->name, inName)!=0)
	{
		/* Set value */
		if(priv->name) g_free(priv->name);
		priv->name=g_strdup(inName);

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesDesktopAppInfoActionProperties[PROP_NAME]);
	}
}

/**
 * xfce_overview_workspaces_desktop_app_info_action_get_icon_name:
 * @self: A #XfceOverviewWorkspacesDesktopAppInfoAction
 *
 * Retrieves the icon name of the application action at @self.
 *
 * Return value: A string with the icon name of the application action
 */
const gchar* xfce_overview_workspaces_desktop_app_info_action_get_icon_name(XfceOverviewWorkspacesDesktopAppInfoAction *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_DESKTOP_APP_INFO_ACTION(self), NULL);

	return(self->priv->iconName);
}

/**
 * xfce_overview_workspaces_desktop_app_info_action_set_icon_name:
 * @self: A #XfceOverviewWorkspacesDesktopAppInfoAction
 * @inIconName: The application action's icon name
 *
 * Sets the icon name of the application action at @self.
 */
void xfce_overview_workspaces_desktop_app_info_action_set_icon_name(XfceOverviewWorkspacesDesktopAppInfoAction *self,
														const gchar *inIconName)
{
	XfceOverviewWorkspacesDesktopAppInfoActionPrivate		*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_DESKTOP_APP_INFO_ACTION(self));

	priv=self->priv;

	/* Set value if changed */
	if(g_strcmp0(priv->iconName, inIconName)!=0)
	{
		/* Set value */
		if(priv->iconName)
		{
			g_free(priv->iconName);
			priv->iconName=NULL;
		}

		if(inIconName) priv->iconName=g_strdup(inIconName);

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesDesktopAppInfoActionProperties[PROP_ICON_NAME]);
	}
}

/**
 * xfce_overview_workspaces_desktop_app_info_action_get_command:
 * @self: A #XfceOverviewWorkspacesDesktopAppInfoAction
 *
 * Retrieves the command of the application action at @self to execute.
 *
 * Return value: A string with the command of the application action
 */
const gchar* xfce_overview_workspaces_desktop_app_info_action_get_command(XfceOverviewWorkspacesDesktopAppInfoAction *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_DESKTOP_APP_INFO_ACTION(self), NULL);

	return(self->priv->command);
}

/**
 * xfce_overview_workspaces_desktop_app_info_action_set_command:
 * @self: A #XfceOverviewWorkspacesDesktopAppInfoAction
 * @inIconName: The application action's icon name
 *
 * Sets the command of the application action at @self to execute when launched.
 * The command must not be empty.
 */
void xfce_overview_workspaces_desktop_app_info_action_set_command(XfceOverviewWorkspacesDesktopAppInfoAction *self,
														const gchar *inCommand)
{
	XfceOverviewWorkspacesDesktopAppInfoActionPrivate		*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_DESKTOP_APP_INFO_ACTION(self));
	g_return_if_fail(inCommand && *inCommand);

	priv=self->priv;

	/* Set value if changed */
	if(g_strcmp0(priv->command, inCommand)!=0)
	{
		/* Set value */
		if(priv->command) g_free(priv->command);
		priv->command=g_strdup(inCommand);

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesDesktopAppInfoActionProperties[PROP_COMMAND]);
	}
}
