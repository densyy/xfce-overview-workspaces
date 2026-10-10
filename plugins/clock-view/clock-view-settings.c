/*
 * clock-view-settings: Shared object instance holding settings for plugin
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

#include "clock-view-settings.h"

#include <libxfce-overview-workspaces/libxfce-overview-workspaces.h>
#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>
#include <math.h>


/* Define this class in GObject system */
struct _XfceOverviewWorkspacesClockViewSettingsPrivate
{
	/* Properties related */
	ClutterColor			*hourColor;
	ClutterColor			*minuteColor;
	ClutterColor			*secondColor;
	ClutterColor			*backgroundColor;
};

G_DEFINE_DYNAMIC_TYPE_EXTENDED(XfceOverviewWorkspacesClockViewSettings,
								xfce_overview_workspaces_clock_view_settings,
								XFCE_OVERVIEW_WORKSPACES_TYPE_PLUGIN_SETTINGS,
								0,
								G_ADD_PRIVATE_DYNAMIC(XfceOverviewWorkspacesClockViewSettings))

/* Define this class in this plugin */
XFCE_OVERVIEW_WORKSPACES_DEFINE_PLUGIN_TYPE(xfce_overview_workspaces_clock_view_settings);

/* Properties */
enum
{
	PROP_0,

	PROP_HOUR_COLOR,
	PROP_MINUTE_COLOR,
	PROP_SECOND_COLOR,
	PROP_BACKGROUOND_COLOR,

	PROP_LAST
};

static GParamSpec* XfceOverviewWorkspacesClockViewSettingsProperties[PROP_LAST]={ 0, };


/* IMPLEMENTATION: Private variables and methods */

/* Single instance of plugin settings */
static XfceOverviewWorkspacesClockViewSettings*		_xfce_overview_workspaces_clock_view_settings=NULL;


/* IMPLEMENTATION: GObject */

/* Construct this object */
static GObject* _xfce_overview_workspaces_clock_view_settings_constructor(GType inType,
																guint inNumberConstructParams,
																GObjectConstructParam *inConstructParams)
{
	GObject									*object;

	if(!_xfce_overview_workspaces_clock_view_settings)
	{
		object=G_OBJECT_CLASS(xfce_overview_workspaces_clock_view_settings_parent_class)->constructor(inType, inNumberConstructParams, inConstructParams);
		_xfce_overview_workspaces_clock_view_settings=XFCE_OVERVIEW_WORKSPACES_CLOCK_VIEW_SETTINGS(object);
	}
		else
		{
			object=g_object_ref(G_OBJECT(_xfce_overview_workspaces_clock_view_settings));
		}

	return(object);
}

/* Dispose this object */
static void _xfce_overview_workspaces_clock_view_settings_dispose(GObject *inObject)
{
	XfceOverviewWorkspacesClockViewSettings			*self=XFCE_OVERVIEW_WORKSPACES_CLOCK_VIEW_SETTINGS(inObject);
	XfceOverviewWorkspacesClockViewSettingsPrivate		*priv=self->priv;

	/* Release allocated resources */
	if(priv->hourColor)
	{
		clutter_color_free(priv->hourColor);
		priv->hourColor=NULL;
	}

	if(priv->minuteColor)
	{
		clutter_color_free(priv->minuteColor);
		priv->minuteColor=NULL;
	}

	if(priv->secondColor)
	{
		clutter_color_free(priv->secondColor);
		priv->secondColor=NULL;
	}

	/* Call parent's class dispose method */
	G_OBJECT_CLASS(xfce_overview_workspaces_clock_view_settings_parent_class)->dispose(inObject);
}

/* Finalize this object */
static void _xfce_overview_workspaces_clock_view_settings_finalize(GObject *inObject)
{
	/* Release allocated resources finally, e.g. unset singleton */
	if(G_LIKELY(G_OBJECT(_xfce_overview_workspaces_clock_view_settings)==inObject))
	{
		_xfce_overview_workspaces_clock_view_settings=NULL;
	}

	/* Call parent's class dispose method */
	G_OBJECT_CLASS(xfce_overview_workspaces_clock_view_settings_parent_class)->finalize(inObject);
}

/* Set/get properties */
static void _xfce_overview_workspaces_clock_view_settings_set_property(GObject *inObject,
															guint inPropID,
															const GValue *inValue,
															GParamSpec *inSpec)
{
	XfceOverviewWorkspacesClockViewSettings			*self=XFCE_OVERVIEW_WORKSPACES_CLOCK_VIEW_SETTINGS(inObject);

	switch(inPropID)
	{
		case PROP_HOUR_COLOR:
			xfce_overview_workspaces_clock_view_settings_set_hour_color(self, clutter_value_get_color(inValue));
			break;

		case PROP_MINUTE_COLOR:
			xfce_overview_workspaces_clock_view_settings_set_minute_color(self, clutter_value_get_color(inValue));
			break;

		case PROP_SECOND_COLOR:
			xfce_overview_workspaces_clock_view_settings_set_second_color(self, clutter_value_get_color(inValue));
			break;

		case PROP_BACKGROUOND_COLOR:
			xfce_overview_workspaces_clock_view_settings_set_background_color(self, clutter_value_get_color(inValue));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(inObject, inPropID, inSpec);
			break;
	}
}

static void _xfce_overview_workspaces_clock_view_settings_get_property(GObject *inObject,
															guint inPropID,
															GValue *outValue,
															GParamSpec *inSpec)
{
	XfceOverviewWorkspacesClockViewSettings			*self=XFCE_OVERVIEW_WORKSPACES_CLOCK_VIEW_SETTINGS(inObject);
	XfceOverviewWorkspacesClockViewSettingsPrivate		*priv=self->priv;

	switch(inPropID)
	{
		case PROP_HOUR_COLOR:
			clutter_value_set_color(outValue, priv->hourColor);
			break;

		case PROP_MINUTE_COLOR:
			clutter_value_set_color(outValue, priv->minuteColor);
			break;

		case PROP_SECOND_COLOR:
			clutter_value_set_color(outValue, priv->secondColor);
			break;

		case PROP_BACKGROUOND_COLOR:
			clutter_value_set_color(outValue, priv->backgroundColor);
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
void xfce_overview_workspaces_clock_view_settings_class_init(XfceOverviewWorkspacesClockViewSettingsClass *klass)
{
	GObjectClass			*gobjectClass=G_OBJECT_CLASS(klass);

	/* Override functions */
	gobjectClass->constructor=_xfce_overview_workspaces_clock_view_settings_constructor;
	gobjectClass->dispose=_xfce_overview_workspaces_clock_view_settings_dispose;
	gobjectClass->finalize=_xfce_overview_workspaces_clock_view_settings_finalize;
	gobjectClass->set_property=_xfce_overview_workspaces_clock_view_settings_set_property;
	gobjectClass->get_property=_xfce_overview_workspaces_clock_view_settings_get_property;

	/* Define properties */
	XfceOverviewWorkspacesClockViewSettingsProperties[PROP_HOUR_COLOR]=
		clutter_param_spec_color("hour-color",
									"Hour color",
									"Color to draw the hour hand with",
									CLUTTER_COLOR_LightChameleon,
									G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	XfceOverviewWorkspacesClockViewSettingsProperties[PROP_MINUTE_COLOR]=
		clutter_param_spec_color("minute-color",
									"Minute color",
									"Color to draw the minute hand with",
									CLUTTER_COLOR_LightChameleon,
									G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	XfceOverviewWorkspacesClockViewSettingsProperties[PROP_SECOND_COLOR]=
		clutter_param_spec_color("second-color",
									"Second color",
									"Color to draw the second hand with",
									CLUTTER_COLOR_White,
									G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	XfceOverviewWorkspacesClockViewSettingsProperties[PROP_BACKGROUOND_COLOR]=
		clutter_param_spec_color("background-color",
									"Background color",
									"Color to draw the circle with that holds the second hand",
									CLUTTER_COLOR_Blue,
									G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(gobjectClass, PROP_LAST, XfceOverviewWorkspacesClockViewSettingsProperties);
}

/* Class finalization */
void xfce_overview_workspaces_clock_view_settings_class_finalize(XfceOverviewWorkspacesClockViewSettingsClass *klass)
{
}

/* Object initialization
 * Create private structure and set up default values
 */
void xfce_overview_workspaces_clock_view_settings_init(XfceOverviewWorkspacesClockViewSettings *self)
{
	XfceOverviewWorkspacesClockViewSettingsPrivate		*priv;

	self->priv=priv=xfce_overview_workspaces_clock_view_settings_get_instance_private(self);

	/* Set up default values */
	priv->hourColor=clutter_color_copy(CLUTTER_COLOR_LightChameleon);
	priv->minuteColor=clutter_color_copy(CLUTTER_COLOR_LightChameleon);
	priv->secondColor=clutter_color_copy(CLUTTER_COLOR_White);
	priv->backgroundColor=clutter_color_copy(CLUTTER_COLOR_Blue);
}


/* IMPLEMENTATION: Public API */

/* Create new plugin settings */
XfceOverviewWorkspacesClockViewSettings* xfce_overview_workspaces_clock_view_settings_new(void)
{
	GObject			*object;

	object=g_object_new(XFCE_OVERVIEW_WORKSPACES_TYPE_CLOCK_VIEW_SETTINGS, NULL);

	return(XFCE_OVERVIEW_WORKSPACES_CLOCK_VIEW_SETTINGS(object));
}

/* Get/set color to draw hour hand with */
const ClutterColor* xfce_overview_workspaces_clock_view_settings_get_hour_color(XfceOverviewWorkspacesClockViewSettings *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_CLOCK_VIEW_SETTINGS(self), NULL);

	return(self->priv->hourColor);
}

void xfce_overview_workspaces_clock_view_settings_set_hour_color(XfceOverviewWorkspacesClockViewSettings *self, const ClutterColor *inColor)
{
	XfceOverviewWorkspacesClockViewSettingsPrivate		*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_CLOCK_VIEW_SETTINGS(self));
	g_return_if_fail(inColor);

	priv=self->priv;

	/* Set value if changed */
	if(priv->hourColor==NULL ||
		!clutter_color_equal(inColor, priv->hourColor))
	{
		/* Set value */
		if(priv->hourColor) clutter_color_free(priv->hourColor);
		priv->hourColor=clutter_color_copy(inColor);

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesClockViewSettingsProperties[PROP_HOUR_COLOR]);
	}
}

/* Get/set color to draw minute hand with */
const ClutterColor* xfce_overview_workspaces_clock_view_settings_get_minute_color(XfceOverviewWorkspacesClockViewSettings *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_CLOCK_VIEW_SETTINGS(self), NULL);

	return(self->priv->minuteColor);
}

void xfce_overview_workspaces_clock_view_settings_set_minute_color(XfceOverviewWorkspacesClockViewSettings *self, const ClutterColor *inColor)
{
	XfceOverviewWorkspacesClockViewSettingsPrivate		*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_CLOCK_VIEW_SETTINGS(self));
	g_return_if_fail(inColor);

	priv=self->priv;

	/* Set value if changed */
	if(priv->minuteColor==NULL ||
		!clutter_color_equal(inColor, priv->minuteColor))
	{
		/* Set value */
		if(priv->minuteColor) clutter_color_free(priv->minuteColor);
		priv->minuteColor=clutter_color_copy(inColor);

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesClockViewSettingsProperties[PROP_MINUTE_COLOR]);
	}
}

/* Get/set color to draw second hand with */
const ClutterColor* xfce_overview_workspaces_clock_view_settings_get_second_color(XfceOverviewWorkspacesClockViewSettings *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_CLOCK_VIEW_SETTINGS(self), NULL);

	return(self->priv->secondColor);
}

void xfce_overview_workspaces_clock_view_settings_set_second_color(XfceOverviewWorkspacesClockViewSettings *self, const ClutterColor *inColor)
{
	XfceOverviewWorkspacesClockViewSettingsPrivate		*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_CLOCK_VIEW_SETTINGS(self));
	g_return_if_fail(inColor);

	priv=self->priv;

	/* Set value if changed */
	if(priv->secondColor==NULL ||
		!clutter_color_equal(inColor, priv->secondColor))
	{
		/* Set value */
		if(priv->secondColor) clutter_color_free(priv->secondColor);
		priv->secondColor=clutter_color_copy(inColor);

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesClockViewSettingsProperties[PROP_SECOND_COLOR]);
	}
}

/* Get/set color to draw background with that holds second hand */
const ClutterColor* xfce_overview_workspaces_clock_view_settings_get_background_color(XfceOverviewWorkspacesClockViewSettings *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_CLOCK_VIEW_SETTINGS(self), NULL);

	return(self->priv->backgroundColor);
}

void xfce_overview_workspaces_clock_view_settings_set_background_color(XfceOverviewWorkspacesClockViewSettings *self, const ClutterColor *inColor)
{
	XfceOverviewWorkspacesClockViewSettingsPrivate		*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_CLOCK_VIEW_SETTINGS(self));
	g_return_if_fail(inColor);

	priv=self->priv;

	/* Set value if changed */
	if(priv->backgroundColor==NULL ||
		!clutter_color_equal(inColor, priv->backgroundColor))
	{
		/* Set value */
		if(priv->backgroundColor) clutter_color_free(priv->backgroundColor);
		priv->backgroundColor=clutter_color_copy(inColor);

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesClockViewSettingsProperties[PROP_BACKGROUOND_COLOR]);
	}
}
