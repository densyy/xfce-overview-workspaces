/*
 * core: Single-instance core library object managing the library and
 *       single-instance objects like window manager and so on.
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
 * SECTION:core
 * @short_description: The core library class
 * @include: xfce-overview-workspaces/core.h
 *
 * #XfceOverviewWorkspacesCore is a single instance object. Its main purpose
 * is to setup and start-up the library and also to manage other
 * (mainly single instance) objects.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_XFCE_REVISION_H
#include "xfce-revision.h"
#endif

#include <libxfce-overview-workspaces/core.h>

#include <glib/gi18n-lib.h>
#include <clutter/x11/clutter-x11.h>
#include <gtk/gtk.h>
#include <garcon/garcon.h>
#include <libxfce4ui/libxfce4ui.h>

#include <libxfce-overview-workspaces/stage.h>
#include <libxfce-overview-workspaces/types.h>
#include <libxfce-overview-workspaces/applications-view.h>
#include <libxfce-overview-workspaces/windows-view.h>
#include <libxfce-overview-workspaces/utils.h>
#include <libxfce-overview-workspaces/theme.h>
#include <libxfce-overview-workspaces/marshal.h>
#include <libxfce-overview-workspaces/debug.h>


/* Forward declaration */
static void _xfce_overview_workspaces_core_gvalue_transform_string_int(const GValue *inSourceValue, GValue *ioDestValue);
static void _xfce_overview_workspaces_core_gvalue_transform_string_uint(const GValue *inSourceValue, GValue *ioDestValue);
static void _xfce_overview_workspaces_core_gvalue_transform_string_long(const GValue *inSourceValue, GValue *ioDestValue);
static void _xfce_overview_workspaces_core_gvalue_transform_string_ulong(const GValue *inSourceValue, GValue *ioDestValue);
static void _xfce_overview_workspaces_core_gvalue_transform_string_int64(const GValue *inSourceValue, GValue *ioDestValue);
static void _xfce_overview_workspaces_core_gvalue_transform_string_uint64(const GValue *inSourceValue, GValue *ioDestValue);
static void _xfce_overview_workspaces_core_gvalue_transform_string_float(const GValue *inSourceValue, GValue *ioDestValue);
static void _xfce_overview_workspaces_core_gvalue_transform_string_double(const GValue *inSourceValue, GValue *ioDestValue);
static void _xfce_overview_workspaces_core_gvalue_transform_string_boolean(const GValue *inSourceValue, GValue *ioDestValue);
static void _xfce_overview_workspaces_core_gvalue_transform_string_flags(const GValue *inSourceValue, GValue *ioDestValue);
static void _xfce_overview_workspaces_core_gvalue_transform_string_enum(const GValue *inSourceValue, GValue *ioDestValue);


/* Define this class in GObject system */
struct _XfceOverviewWorkspacesCorePrivate
{
	/* Properties related */
	gboolean							isSuspended;
	gchar								*themeName;
	XfceOverviewWorkspacesSettings					*settings;

	/* Instance related */
	gboolean							initialized;
	gboolean							isQuitting;

	XfceOverviewWorkspacesStage					*stage;
	XfceOverviewWorkspacesViewManager				*viewManager;
	XfceOverviewWorkspacesFocusManager				*focusManager;

	XfceOverviewWorkspacesTheme					*theme;
	GBinding							*themeChangedBinding;

	XfceOverviewWorkspacesBindingsPool				*bindings;

	XfceOverviewWorkspacesApplicationDatabase		*appDatabase;
	XfceOverviewWorkspacesApplicationTracker		*appTracker;

	XfceOverviewWorkspacesPluginsManager			*pluginsManager;

	XfceOverviewWorkspacesWindowTrackerBackend		*windowTrackerBackend;
};

G_DEFINE_TYPE_EXTENDED(XfceOverviewWorkspacesCore,
						xfce_overview_workspaces_core,
						G_TYPE_OBJECT,
						0,
						G_ADD_PRIVATE(XfceOverviewWorkspacesCore)
						{
							/* Register GValue transformation functions not provided by Glib */
							g_value_register_transform_func(G_TYPE_STRING, G_TYPE_INT, _xfce_overview_workspaces_core_gvalue_transform_string_int);
							g_value_register_transform_func(G_TYPE_STRING, G_TYPE_UINT, _xfce_overview_workspaces_core_gvalue_transform_string_uint);
							g_value_register_transform_func(G_TYPE_STRING, G_TYPE_LONG, _xfce_overview_workspaces_core_gvalue_transform_string_long);
							g_value_register_transform_func(G_TYPE_STRING, G_TYPE_ULONG, _xfce_overview_workspaces_core_gvalue_transform_string_ulong);
							g_value_register_transform_func(G_TYPE_STRING, G_TYPE_INT64, _xfce_overview_workspaces_core_gvalue_transform_string_int64);
							g_value_register_transform_func(G_TYPE_STRING, G_TYPE_UINT64, _xfce_overview_workspaces_core_gvalue_transform_string_uint64);
							g_value_register_transform_func(G_TYPE_STRING, G_TYPE_FLOAT, _xfce_overview_workspaces_core_gvalue_transform_string_float);
							g_value_register_transform_func(G_TYPE_STRING, G_TYPE_DOUBLE, _xfce_overview_workspaces_core_gvalue_transform_string_double);
							g_value_register_transform_func(G_TYPE_STRING, G_TYPE_BOOLEAN, _xfce_overview_workspaces_core_gvalue_transform_string_boolean);
							g_value_register_transform_func(G_TYPE_STRING, G_TYPE_FLAGS, _xfce_overview_workspaces_core_gvalue_transform_string_flags);
							g_value_register_transform_func(G_TYPE_STRING, G_TYPE_ENUM, _xfce_overview_workspaces_core_gvalue_transform_string_enum);
						} )

/* Properties */
enum
{
	PROP_0,

	PROP_SUSPENDED,
	PROP_THEME_NAME,
	PROP_STAGE,
	PROP_SETTINGS,

	PROP_LAST
};

static GParamSpec* XfceOverviewWorkspacesCoreProperties[PROP_LAST]={ 0, };

/* Signals */
enum
{
	SIGNAL_INITIALIZED,
	SIGNAL_QUIT,
	SIGNAL_SHUTDOWN,

	SIGNAL_CAN_SUSPEND,
	SIGNAL_SUSPEND,
	SIGNAL_RESUME,

	SIGNAL_THEME_LOADING,
	SIGNAL_THEME_LOADED,
	SIGNAL_THEME_CHANGED,

	SIGNAL_CORE_LAUNCHED,

	/* Actions */
	ACTION_EXIT,

	SIGNAL_LAST
};

static guint XfceOverviewWorkspacesCoreSignals[SIGNAL_LAST]={ 0, };


/* IMPLEMENTATION: Private variables and methods */

/* Single instance of application */
static XfceOverviewWorkspacesCore*		_xfce_overview_workspaces_core=NULL;

/* GValue transformation function for G_TYPE_STRING to various other types */
static void _xfce_overview_workspaces_core_gvalue_transform_string_int(const GValue *inSourceValue, GValue *ioDestValue)
{
	ioDestValue->data[0].v_int=g_ascii_strtoll(inSourceValue->data[0].v_pointer, NULL, 10);
}

static void _xfce_overview_workspaces_core_gvalue_transform_string_uint(const GValue *inSourceValue, GValue *ioDestValue)
{
	ioDestValue->data[0].v_uint=g_ascii_strtoull(inSourceValue->data[0].v_pointer, NULL, 10);
}

static void _xfce_overview_workspaces_core_gvalue_transform_string_long(const GValue *inSourceValue, GValue *ioDestValue)
{
	ioDestValue->data[0].v_long=g_ascii_strtoll(inSourceValue->data[0].v_pointer, NULL, 10);
}

static void _xfce_overview_workspaces_core_gvalue_transform_string_ulong(const GValue *inSourceValue, GValue *ioDestValue)
{
	ioDestValue->data[0].v_ulong=g_ascii_strtoull(inSourceValue->data[0].v_pointer, NULL, 10);
}

static void _xfce_overview_workspaces_core_gvalue_transform_string_int64(const GValue *inSourceValue, GValue *ioDestValue)
{
	ioDestValue->data[0].v_int64=g_ascii_strtoll(inSourceValue->data[0].v_pointer, NULL, 10);
}

static void _xfce_overview_workspaces_core_gvalue_transform_string_uint64(const GValue *inSourceValue, GValue *ioDestValue)
{
	ioDestValue->data[0].v_uint64=g_ascii_strtoull(inSourceValue->data[0].v_pointer, NULL, 10);
}

static void _xfce_overview_workspaces_core_gvalue_transform_string_float(const GValue *inSourceValue, GValue *ioDestValue)
{
	ioDestValue->data[0].v_float=g_ascii_strtod(inSourceValue->data[0].v_pointer, NULL);
}

static void _xfce_overview_workspaces_core_gvalue_transform_string_double(const GValue *inSourceValue, GValue *ioDestValue)
{
	ioDestValue->data[0].v_double=g_ascii_strtod(inSourceValue->data[0].v_pointer, NULL);
}

static void _xfce_overview_workspaces_core_gvalue_transform_string_boolean(const GValue *inSourceValue, GValue *ioDestValue)
{
	/* Convert case-insentive "true" to TRUE */
	if(!g_ascii_strncasecmp(inSourceValue->data[0].v_pointer, "true", 4))
	{
		ioDestValue->data[0].v_int=TRUE;
	}
		/* Convert case-insentive "false" to FALSE */
		else if(!g_ascii_strncasecmp(inSourceValue->data[0].v_pointer, "false", 5))
		{
			ioDestValue->data[0].v_int=FALSE;
		}
		/* Convert to unsigned integer if set destination to TRUE if non-zero
		 * otherweise set destination to FALSE
		 */
		else
		{
			ioDestValue->data[0].v_int=(g_ascii_strtoull(inSourceValue->data[0].v_pointer, NULL, 10)!=0 ? TRUE : FALSE);
		}
}

static void _xfce_overview_workspaces_core_gvalue_transform_string_enum(const GValue *inSourceValue, GValue *ioDestValue)
{
	GEnumClass		*enumClass;
	GEnumValue		*enumValue;
	const gchar		*value;

	/* Reference enum class to keep it alive for transformation */
	enumClass=g_type_class_ref(G_VALUE_TYPE(ioDestValue));

	/* Get value to convert */
	value=(const gchar*)inSourceValue->data[0].v_pointer;

	/* Get enum value either by name or by nickname (whatever matches first) */
	enumValue=g_enum_get_value_by_name(enumClass, value);
	if(!enumValue) enumValue=g_enum_get_value_by_nick(enumClass, value);

	/* Set value if enum could be found otherwise set 0 */
	if(enumValue) ioDestValue->data[0].v_int=enumValue->value;
		else
		{
			ioDestValue->data[0].v_int=0;
			XFCE_OVERVIEW_WORKSPACES_DEBUG(NULL, MISC,
								"Cannot get value for unknown enum '%s' for type %s",
								value,
								g_type_name(G_VALUE_TYPE(ioDestValue)));
		}

	/* Release allocated resources */
	g_type_class_unref(enumClass);
}

static void _xfce_overview_workspaces_core_gvalue_transform_string_flags(const GValue *inSourceValue, GValue *ioDestValue)
{
	GFlagsClass		*flagsClass;
	GFlagsValue		*flagsValue;
	guint			finalValue;
	gchar			**values, **entry;

	/* Reference flags class to keep it alive for transformation */
	flagsClass=g_type_class_ref(G_VALUE_TYPE(ioDestValue));

	/* Split string into space-separated needles and lookup each needle
	 * for a match and add found values OR'ed to final value
	 */
	finalValue=0;
	entry=values=g_strsplit(inSourceValue->data[0].v_pointer, " ", 0);
	while(*entry)
	{
		/* Do not look-up empty values */
		if(!entry[0]) continue;

		/* Get flags value either by name or by nickname (whatever matches first) */
		flagsValue=g_flags_get_value_by_name(flagsClass, *entry);
		if(!flagsValue) flagsValue=g_flags_get_value_by_nick(flagsClass, *entry);

		/* Add value OR'ed if flags could be found */
		if(flagsValue) finalValue|=flagsValue->value;
			else
			{
				XFCE_OVERVIEW_WORKSPACES_DEBUG(NULL, MISC,
									"Cannot get value for unknown flag '%s' for type %s",
									*entry,
									g_type_name(G_VALUE_TYPE(ioDestValue)));
			}

		/* Continue with next entry */
		entry++;
	}
	g_strfreev(values);

	/* Set value */
	ioDestValue->data[0].v_uint=finalValue;

	/* Release allocated resources */
	g_type_class_unref(flagsClass);
}

/* Welcome notification in an idle callback.
 * xfce_overview_workspaces_notify() traverses the whole actor tree looking for the stage
 * interface (CSS selector + full traversal) and resolves the notification
 * icon. None of that is needed before the first frame, so it is deferred to
 * a low-priority idle callback instead of running synchronously at the end
 * of initialization. The notification itself appears a main-loop tick later
 * which is not perceivable.
 */
static gboolean _xfce_overview_workspaces_core_welcome_notify_idle(gpointer inUserData)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_CORE(inUserData), G_SOURCE_REMOVE);

	/* Skip notification if core is already shutting down (stage may be gone,
	 * notify would only emit a critical warning).
	 */
	if(xfce_overview_workspaces_core_is_quitting(XFCE_OVERVIEW_WORKSPACES_CORE(inUserData))) return(G_SOURCE_REMOVE);

#ifdef DEBUG
	xfce_overview_workspaces_notify(NULL, NULL, _("Welcome to %s (%s)!"), PACKAGE_NAME, VERSION_FULL);
#else
	xfce_overview_workspaces_notify(NULL, NULL, _("Welcome to %s!"), PACKAGE_NAME);
#endif

	/* Extra reference taken when the idle source was created is released
	 * automatically via destroy notify.
	 */
	return(G_SOURCE_REMOVE);
}

/* Action "exit" was called at application */
static gboolean _xfce_overview_workspaces_core_action_exit(XfceOverviewWorkspacesCore *self,
												XfceOverviewWorkspacesFocusable *inSource,
												const gchar *inAction,
												ClutterEvent *inEvent)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_CORE(self), CLUTTER_EVENT_PROPAGATE);

	/* Quit application */
	xfce_overview_workspaces_core_quit(self);

	/* Prevent the default handler being called */
	return(CLUTTER_EVENT_STOP);
}

/* A stage window should be destroyed */
static gboolean _xfce_overview_workspaces_core_on_delete_stage(XfceOverviewWorkspacesCore *self,
													ClutterEvent *inEvent,
													gpointer inUserData)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_CORE(self), CLUTTER_EVENT_PROPAGATE);

	/* Quit application */
	xfce_overview_workspaces_core_quit(self);

	/* Prevent the default handler being called */
	return(CLUTTER_EVENT_STOP);
}

/* Set theme name and reload theme */
static void _xfce_overview_workspaces_core_set_theme_name(XfceOverviewWorkspacesCore *self, const gchar *inThemeName)
{
	XfceOverviewWorkspacesCorePrivate	*priv;
	GError							*error;
	XfceOverviewWorkspacesTheme				*theme;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_CORE(self));
	g_return_if_fail(inThemeName && *inThemeName);

	priv=self->priv;
	error=NULL;

	/* Set value if changed */
	if(g_strcmp0(priv->themeName, inThemeName)!=0)
	{
		/* Create new theme instance */
		theme=xfce_overview_workspaces_theme_new(inThemeName);

		/* Emit signal that theme is going to be loaded */
		g_signal_emit(self, XfceOverviewWorkspacesCoreSignals[SIGNAL_THEME_LOADING], 0, theme);

		/* Load theme */
		if(!xfce_overview_workspaces_theme_load(theme, &error))
		{
			/* Show critical warning at console */
			g_critical("Could not load theme '%s': %s",
						inThemeName,
						(error && error->message) ? error->message : "unknown error");

			/* Show warning as notification */
			xfce_overview_workspaces_notify(NULL,
								"dialog-error",
								_("Could not load theme '%s': %s"),
								inThemeName,
								(error && error->message) ? error->message : _("unknown error"));

			/* Release allocated resources */
			if(error!=NULL) g_error_free(error);
			g_object_unref(theme);

			return;
		}

		/* Emit signal that theme was loaded successfully and will soon be applied */
		g_signal_emit(self, XfceOverviewWorkspacesCoreSignals[SIGNAL_THEME_LOADED], 0, theme);

		/* Set value */
		if(priv->themeName) g_free(priv->themeName);
		priv->themeName=g_strdup(inThemeName);

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesCoreProperties[PROP_THEME_NAME]);

		/* Release current theme and store new one */
		if(priv->theme) g_object_unref(priv->theme);
		priv->theme=theme;

		/* Emit signal that theme has changed to get all top-level actors
		 * to apply new theme.
		 */
		g_signal_emit(self, XfceOverviewWorkspacesCoreSignals[SIGNAL_THEME_CHANGED], 0, priv->theme);
	}
}

/* Set settings object */
static void _xfce_overview_workspaces_core_set_settings(XfceOverviewWorkspacesCore *self, XfceOverviewWorkspacesSettings *inSettings)
{
	XfceOverviewWorkspacesCorePrivate	*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_CORE(self));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_SETTINGS(inSettings));

	priv=self->priv;

	/* Set value if changed */
	if(priv->settings!=inSettings)
	{
		/* Set value */
		if(priv->settings)
		{
			g_object_unref(priv->settings);
			priv->settings=NULL;
		}
		priv->settings=g_object_ref(inSettings);

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesCoreProperties[PROP_SETTINGS]);
	}
}

/* Default implementation of virtual function for signal "can-suspend" */
static gboolean _xfce_overview_workspaces_core_can_suspend(XfceOverviewWorkspacesCore *self)
{
	/* This default implementation of the virtual function for the singal is
	 * to return FALSE as no suspend/resume is supported by core itself. It needs
	 * an application connecting a signal handler to this signal action to tell
	 * that suspend/resume is supported and handled by application using this
	 * library.
	 */
	return(FALSE);
}


/* IMPLEMENTATION: GObject */

/* Dispose this object */
static void _xfce_overview_workspaces_core_dispose(GObject *inObject)
{
	XfceOverviewWorkspacesCore			*self=XFCE_OVERVIEW_WORKSPACES_CORE(inObject);
	XfceOverviewWorkspacesCorePrivate	*priv=self->priv;

	/* Ensure "is-quitting" flag is set just in case someone asks */
	priv->isQuitting=TRUE;

	/* First destroy stage to release all actors and their bindings
	 * to any other sub-system.
	 */
	if(priv->stage)
	{
		clutter_actor_destroy(CLUTTER_ACTOR(priv->stage));
		priv->stage=NULL;
	}

	/* Signal "shutdown-final" of application */
	g_signal_emit(self, XfceOverviewWorkspacesCoreSignals[SIGNAL_SHUTDOWN], 0);

	/* Release allocated resources */
	if(priv->windowTrackerBackend)
	{
		g_object_unref(priv->windowTrackerBackend);
		priv->windowTrackerBackend=NULL;
	}

	if(priv->pluginsManager)
	{
		g_object_unref(priv->pluginsManager);
		priv->pluginsManager=NULL;
	}

	if(priv->themeChangedBinding)
	{
		g_object_unref(priv->themeChangedBinding);
		priv->themeChangedBinding=NULL;
	}

	if(priv->viewManager)
	{
		/* Unregisters all remaining registered views - no need to unregister them here */
		g_object_unref(priv->viewManager);
		priv->viewManager=NULL;
	}

	if(priv->focusManager)
	{
		/* Unregisters all remaining registered focusable actors.
		 * There is no need to unregister them here.
		 */
		g_object_unref(priv->focusManager);
		priv->focusManager=NULL;
	}

	if(priv->bindings)
	{
		g_object_unref(priv->bindings);
		priv->bindings=NULL;
	}

	if(priv->appDatabase)
	{
		g_object_unref(priv->appDatabase);
		priv->appDatabase=NULL;
	}

	if(priv->appTracker)
	{
		g_object_unref(priv->appTracker);
		priv->appTracker=NULL;
	}

	if(priv->theme)
	{
		g_object_unref(priv->theme);
		priv->theme=NULL;
	}

	if(priv->themeName)
	{
		g_free(priv->themeName);
		priv->themeName=NULL;
	}

	if(priv->settings)
	{
		g_object_unref(priv->settings);
		priv->settings=NULL;
	}

	/* Unset singleton */
	if(G_LIKELY(G_OBJECT(_xfce_overview_workspaces_core)==inObject)) _xfce_overview_workspaces_core=NULL;

	/* Call parent's class dispose method */
	G_OBJECT_CLASS(xfce_overview_workspaces_core_parent_class)->dispose(inObject);
}

/* Set/get properties */
static void _xfce_overview_workspaces_core_set_property(GObject *inObject,
													guint inPropID,
													const GValue *inValue,
													GParamSpec *inSpec)
{
	XfceOverviewWorkspacesCore	*self=XFCE_OVERVIEW_WORKSPACES_CORE(inObject);

	switch(inPropID)
	{
		case PROP_THEME_NAME:
			_xfce_overview_workspaces_core_set_theme_name(self, g_value_get_string(inValue));
			break;

		case PROP_SETTINGS:
			_xfce_overview_workspaces_core_set_settings(self, g_value_get_object(inValue));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(inObject, inPropID, inSpec);
			break;
	}
}

static void _xfce_overview_workspaces_core_get_property(GObject *inObject,
													guint inPropID,
													GValue *outValue,
													GParamSpec *inSpec)
{
	XfceOverviewWorkspacesCore	*self=XFCE_OVERVIEW_WORKSPACES_CORE(inObject);

	switch(inPropID)
	{
		case PROP_SUSPENDED:
			g_value_set_boolean(outValue, self->priv->isSuspended);
			break;

		case PROP_STAGE:
			g_value_set_object(outValue, self->priv->stage);
			break;

		case PROP_THEME_NAME:
			g_value_set_string(outValue, self->priv->themeName);
			break;

		case PROP_SETTINGS:
			g_value_set_object(outValue, self->priv->settings);
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
static void xfce_overview_workspaces_core_class_init(XfceOverviewWorkspacesCoreClass *klass)
{
	GObjectClass		*gobjectClass=G_OBJECT_CLASS(klass);

	/* Override functions */
	klass->can_suspend=_xfce_overview_workspaces_core_can_suspend;
	klass->exit=_xfce_overview_workspaces_core_action_exit;

	gobjectClass->dispose=_xfce_overview_workspaces_core_dispose;
	gobjectClass->set_property=_xfce_overview_workspaces_core_set_property;
	gobjectClass->get_property=_xfce_overview_workspaces_core_get_property;

	/* Define properties */
	/**
	 * XfceOverviewWorkspacesCore:is-suspended:
	 *
	 * A flag indicating if core was suspended. It is set to %TRUE if core is
	 * suspended. If it is %FALSE then core is resumed, active and visible.
	 */
	XfceOverviewWorkspacesCoreProperties[PROP_SUSPENDED]=
		g_param_spec_boolean("is-suspended",
								"Is suspended",
								"Flag indicating if core is suspended currently",
								FALSE,
								G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

	/**
	 * XfceOverviewWorkspacesCore:stage:
	 *
	 * The #XfceOverviewWorkspacesStage of core.
	 */
	XfceOverviewWorkspacesCoreProperties[PROP_STAGE]=
		g_param_spec_object("stage",
								"Stage",
								"The stage object of core",
								XFCE_OVERVIEW_WORKSPACES_TYPE_STAGE,
								G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * XfceOverviewWorkspacesCore:theme-name:
	 *
	 * The name of current theme used in core.
	 */
	XfceOverviewWorkspacesCoreProperties[PROP_THEME_NAME]=
		g_param_spec_string("theme-name",
								"Theme name",
								"Name of current theme",
								NULL,
								G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * XfceOverviewWorkspacesCore:settings:
	 *
	 * The #XfceOverviewWorkspacesSettings of core.
	 */
	XfceOverviewWorkspacesCoreProperties[PROP_SETTINGS]=
		g_param_spec_object("settings",
								"Settings",
								"The settings object of core",
								XFCE_OVERVIEW_WORKSPACES_TYPE_SETTINGS,
								G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(gobjectClass, PROP_LAST, XfceOverviewWorkspacesCoreProperties);

	/* Define signals */
	/**
	 * XfceOverviewWorkspacesCore::initialized:
	 * @self: The core instance was fully initialized
	 *
	 * The ::initialized signal is emitted when the core instance was fully
	 * initialized.
	 *
	 * The main purpose of this signal is to give other objects and plugins
	 * a chance to initialize properly after core library was fully initialized
	 * and all other components are really available and also initialized.
	 */
	XfceOverviewWorkspacesCoreSignals[SIGNAL_INITIALIZED]=
		g_signal_new("initialized",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST,
						G_STRUCT_OFFSET(XfceOverviewWorkspacesCoreClass, initialized),
						NULL,
						NULL,
						g_cclosure_marshal_VOID__VOID,
						G_TYPE_NONE,
						0);

	/**
	 * XfceOverviewWorkspacesCore::quit:
	 * @self: The core instance is requested to quit
	 *
	 * The ::quit signal is emitted when the core library is requested to quit.
	 * It up to the connected signal handlers to shutdown or suspend the core
	 * instance.
	 */
	XfceOverviewWorkspacesCoreSignals[SIGNAL_QUIT]=
		g_signal_new("quit",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST,
						G_STRUCT_OFFSET(XfceOverviewWorkspacesCoreClass, quit),
						NULL,
						NULL,
						g_cclosure_marshal_VOID__VOID,
						G_TYPE_NONE,
						0);

	/**
	 * XfceOverviewWorkspacesCore::shutdown-final:
	 * @self: The core instance will shut down
	 *
	 * The ::shutdown-final signal is emitted when the core library object
	 * is currently destroyed and can be considered as quitted. It is the
	 * last chance or other components and plugins to work with the library.
	 *
	 * The main purpose of this signal is to give the other components,
	 * objects and plugins a chance to clean up properly.
	 */
	XfceOverviewWorkspacesCoreSignals[SIGNAL_SHUTDOWN]=
		g_signal_new("shutdown",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST,
						G_STRUCT_OFFSET(XfceOverviewWorkspacesCoreClass, shutdown),
						NULL,
						NULL,
						g_cclosure_marshal_VOID__VOID,
						G_TYPE_NONE,
						0);

	/**
	 * XfceOverviewWorkspacesCore::can-suspend:
	 * @self: The core instance is asked for support of suspend
	 *
	 * The ::can-suspend signal is emitted when to check if core can
	 * support suspend and resume. The connected signal handler must
	 * return %TRUE if suspend and resume is supported and %FALSE if
	 * it is not suppoerted.
	 *
	 * Usually only one signal handler should be connected, but if more
	 * are connected the first one returning %TRUE to tell that suspend
	 * is supported will stop the signal emission. If no one is connected
	 * the default %FALSE will be returned as the only result indicating
	 * that by default no suspend is supported.
	 */
	XfceOverviewWorkspacesCoreSignals[SIGNAL_CAN_SUSPEND]=
		g_signal_new("can-suspend",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST,
						G_STRUCT_OFFSET(XfceOverviewWorkspacesCoreClass, can_suspend),
						g_signal_accumulator_true_handled,
						NULL,
						_xfce_overview_workspaces_marshal_BOOLEAN__VOID,
						G_TYPE_BOOLEAN,
						0);

	/**
	 * XfceOverviewWorkspacesCore::suspend:
	 * @self: The core instance which should suspend
	 *
	 * The ::suspend signal is an action and when emitted it causes all
	 * connected signal handlers to prepare their state for suspend, that
	 * means to be send to background and waiting for ::resume.
	 */
	XfceOverviewWorkspacesCoreSignals[SIGNAL_SUSPEND]=
		g_signal_new("suspend",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST,
						G_STRUCT_OFFSET(XfceOverviewWorkspacesCoreClass, suspend),
						NULL,
						NULL,
						g_cclosure_marshal_VOID__VOID,
						G_TYPE_NONE,
						0);

	/**
	 * XfceOverviewWorkspacesCore::resume:
	 * @self: The core instance which should resume
	 *
	 * The ::resume signal is an action and when emitted it causes all
	 * connected signal handlers to prepare their state for return from
	 * suspend and to resume and to be brought to foreground again.
	 */
	XfceOverviewWorkspacesCoreSignals[SIGNAL_RESUME]=
		g_signal_new("resume",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST,
						G_STRUCT_OFFSET(XfceOverviewWorkspacesCoreClass, resume),
						NULL,
						NULL,
						g_cclosure_marshal_VOID__VOID,
						G_TYPE_NONE,
						0);

	/**
	 * XfceOverviewWorkspacesCore::theme-loading:
	 * @self: The core instance whose theme is going to change
	 * @inTheme: The new #XfceOverviewWorkspacesTheme to use
	 *
	 * The ::theme-loading signal is emitted when a theme at core instance is
	 * going to be loaded. When this signal is received no file was loaded so far
	 * but will be. For example, at this moment it is possible to load a CSS file
	 * by a plugin before the CSS files of the new theme will be loaded to give
	 * the theme a chance to override the default CSS of plugin.
	 */
	XfceOverviewWorkspacesCoreSignals[SIGNAL_THEME_LOADING]=
		g_signal_new("theme-loading",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST,
						G_STRUCT_OFFSET(XfceOverviewWorkspacesCoreClass, theme_loading),
						NULL,
						NULL,
						g_cclosure_marshal_VOID__OBJECT,
						G_TYPE_NONE,
						1,
						XFCE_OVERVIEW_WORKSPACES_TYPE_THEME);

	/**
	 * XfceOverviewWorkspacesCore::theme-loaded:
	 * @self: The core instance whose theme has changed
	 * @inTheme: The new #XfceOverviewWorkspacesTheme to use
	 * 
	 * The ::theme-loaded signal is emitted when the new theme at core instance
	 * was loaded and will soon be applied. When this signal is received it is
	 * the last chance for other components and plugins to load additionally
	 * resources like CSS, e.g. to override CSS of theme.
	 */
	XfceOverviewWorkspacesCoreSignals[SIGNAL_THEME_LOADED]=
		g_signal_new("theme-loaded",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST,
						G_STRUCT_OFFSET(XfceOverviewWorkspacesCoreClass, theme_loaded),
						NULL,
						NULL,
						g_cclosure_marshal_VOID__OBJECT,
						G_TYPE_NONE,
						1,
						XFCE_OVERVIEW_WORKSPACES_TYPE_THEME);

	/**
	 * XfceOverviewWorkspacesCore::theme-changed:
	 * @self: The core instance whose theme has changed
	 * @inTheme: The new #XfceOverviewWorkspacesTheme applied
	 *
	 * The ::theme-changed signal is emitted when a new theme at core instance
	 * has been loaded and applied.
	 */
	XfceOverviewWorkspacesCoreSignals[SIGNAL_THEME_CHANGED]=
		g_signal_new("theme-changed",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST,
						G_STRUCT_OFFSET(XfceOverviewWorkspacesCoreClass, theme_changed),
						NULL,
						NULL,
						g_cclosure_marshal_VOID__OBJECT,
						G_TYPE_NONE,
						1,
						XFCE_OVERVIEW_WORKSPACES_TYPE_THEME);

	/**
	 * XfceOverviewWorkspacesCore::application-launched:
	 * @self: The core instance
	 * @inAppInfo: The #GAppInfo that was just launched
	 *
	 * The ::application-launched signal is emitted when a #GAppInfo has been
	 * launched successfully, e.g. via any #XfceOverviewWorkspacesApplicationButton.
	 */
	XfceOverviewWorkspacesCoreSignals[SIGNAL_CORE_LAUNCHED]=
		g_signal_new("application-launched",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST,
						G_STRUCT_OFFSET(XfceOverviewWorkspacesCoreClass, application_launched),
						NULL,
						NULL,
						g_cclosure_marshal_VOID__OBJECT,
						G_TYPE_NONE,
						1,
						G_TYPE_APP_INFO);

	/**
	 * XfceOverviewWorkspacesCore::exit:
	 * @self: The application
	 * @inFocusable: The source #XfceOverviewWorkspacesFocusable which send this signal
	 * @inAction: A string containing the action (that is this signal name)
	 * @inEvent: The #ClutterEvent associated to this signal and action
	 *
	 * The ::exit signal is an action and when emitted it causes the core to
	 * send the :quit signal like calling xfce_overview_workspaces_core_quit(). It up to
	 * the connected signal handlers to shutdown or suspend the core library.
	 *
	 * Note: This action can be used at key bindings.
	 */
	XfceOverviewWorkspacesCoreSignals[ACTION_EXIT]=
		g_signal_new("exit",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
						G_STRUCT_OFFSET(XfceOverviewWorkspacesCoreClass, exit),
						g_signal_accumulator_true_handled,
						NULL,
						_xfce_overview_workspaces_marshal_BOOLEAN__OBJECT_STRING_BOXED,
						G_TYPE_BOOLEAN,
						3,
						XFCE_OVERVIEW_WORKSPACES_TYPE_FOCUSABLE,
						G_TYPE_STRING,
						CLUTTER_TYPE_EVENT);
}

/* Object initialization
 * Create private structure and set up default values
 */
static void xfce_overview_workspaces_core_init(XfceOverviewWorkspacesCore *self)
{
	XfceOverviewWorkspacesCorePrivate			*priv;

	priv=self->priv=xfce_overview_workspaces_core_get_instance_private(self);

	/* Set up singleton of core object instance.
	 * Note: Setting singleton here is wrong but at current state essential
	 * for futher development and testing. It should be find a way how to
	 * remove it and use a pointer to core object instance instead at the
	 * other components need access to core object functions.
	 */
	if(_xfce_overview_workspaces_core==NULL) _xfce_overview_workspaces_core=self;

	/* Set default values */
	priv->isSuspended=FALSE;
	priv->themeName=NULL;
	priv->initialized=FALSE;
	priv->settings=NULL;
	priv->stage=NULL;
	priv->viewManager=NULL;
	priv->focusManager=NULL;
	priv->theme=NULL;
	priv->themeChangedBinding=NULL;
	priv->isQuitting=FALSE;
	priv->pluginsManager=NULL;
	priv->windowTrackerBackend=NULL;
}


/* IMPLEMENTATION: Errors */

/**
 * xfce_overview_workspaces_core_error_quark:
 *
 * Gets the core error quark.
 *
 * Returns: The #GQuark for core errors.
 */
GQuark xfce_overview_workspaces_core_error_quark(void)
{
	return(g_quark_from_static_string("xfce-overview-workspaces-core-error-quark"));
}


/* IMPLEMENTATION: Public API */

/** xfce_overview_workspaces_core_initialize:
 * @self: A #XfceOverviewWorkspacesCore
 * @outError: A return location for a #GError or %NULL
 *
 * Initializes the core instance at @self and sets up all other components
 * like plugin manager, window tracker manager etc.
 *
 * If initialization fails, the error message will be placed inside error
 * at @outError (if not %NULL) instead of being printed on the display.
 *
 * Return value: %TRUE if instance of #XfceOverviewWorkspacesCore could be initialized
 *   or %FALSE if not and error is stored at @outError.
 */
gboolean xfce_overview_workspaces_core_initialize(XfceOverviewWorkspacesCore *self, GError **outError)
{
	XfceOverviewWorkspacesCorePrivate			*priv;
	GError							*error;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_CORE(self), FALSE);
	g_return_val_if_fail(outError==NULL || *outError==NULL, FALSE);

	priv=self->priv;
	error=NULL;

	/* Skip if initilization was done already and set error as well as
	 * return FALSE as this is an error.
	 */
	if(priv->initialized)
	{
		/* Set error */
		g_set_error(outError,
					XFCE_OVERVIEW_WORKSPACES_CORE_ERROR,
					XFCE_OVERVIEW_WORKSPACES_CORE_ERROR_INITIALIZATION_FAILED,
					"Core instance was already initiliazed");

		/* Return failed state */
		return(FALSE);
	}

	/* Set flag that this core instance was initialized regardless if
	 * it succeeds or fails.
	 */
	priv->initialized=TRUE;

#ifdef DEBUG
#ifdef XFCE_OVERVIEW_WORKSPACES_ENABLE_DEBUG
	g_print("** Use environment variable XFCE_OVERVIEW_WORKSPACES_DEBUG to enable debug messages\n");
	g_print("** To get a list of debug categories set XFCE_OVERVIEW_WORKSPACES_DEBUG=help\n");
#endif
#endif

#ifdef XFCE_OVERVIEW_WORKSPACES_ENABLE_DEBUG
	/* Set up debug flags */
	{
		const gchar					*environment;
		static const GDebugKey		debugKeys[]=
									{
										{ "misc", XFCE_OVERVIEW_WORKSPACES_DEBUG_MISC },
										{ "actor", XFCE_OVERVIEW_WORKSPACES_DEBUG_ACTOR },
										{ "style", XFCE_OVERVIEW_WORKSPACES_DEBUG_STYLE },
										{ "styling", XFCE_OVERVIEW_WORKSPACES_DEBUG_STYLE },
										{ "theme", XFCE_OVERVIEW_WORKSPACES_DEBUG_THEME },
										{ "apps", XFCE_OVERVIEW_WORKSPACES_DEBUG_APPLICATIONS },
										{ "applications", XFCE_OVERVIEW_WORKSPACES_DEBUG_APPLICATIONS },
										{ "images", XFCE_OVERVIEW_WORKSPACES_DEBUG_IMAGES },
										{ "windows", XFCE_OVERVIEW_WORKSPACES_DEBUG_WINDOWS },
										{ "window-tracker", XFCE_OVERVIEW_WORKSPACES_DEBUG_WINDOWS },
										{ "animation", XFCE_OVERVIEW_WORKSPACES_DEBUG_ANIMATION },
										{ "animations", XFCE_OVERVIEW_WORKSPACES_DEBUG_ANIMATION },
										{ "plugin", XFCE_OVERVIEW_WORKSPACES_DEBUG_PLUGINS },
										{ "plugins", XFCE_OVERVIEW_WORKSPACES_DEBUG_PLUGINS },
									};

		/* Parse debug flags */
		environment=g_getenv("XFCE_OVERVIEW_WORKSPACES_DEBUG");
		if(environment)
		{
			xfce_overview_workspaces_debug_flags=
				g_parse_debug_string(environment,
										debugKeys,
										G_N_ELEMENTS(debugKeys));
			environment=NULL;
		}

		/* Parse object names to debug */
		environment=g_getenv("XFCE_OVERVIEW_WORKSPACES_DEBUG");
		if(environment)
		{
			if(G_UNLIKELY(xfce_overview_workspaces_debug_classes))
			{
				g_strfreev(xfce_overview_workspaces_debug_classes);
				xfce_overview_workspaces_debug_classes=NULL;
			}

			xfce_overview_workspaces_debug_classes=g_strsplit(environment, ",", -1);
			environment=NULL;
		}
	}
#endif

	/* Initialize garcon for current desktop environment */
	garcon_set_environment_xdg(GARCON_ENVIRONMENT_XFCE);

	/* Check for settings */
	if(!priv->settings)
	{
		/* Set error */
		g_set_error(outError,
					XFCE_OVERVIEW_WORKSPACES_CORE_ERROR,
					XFCE_OVERVIEW_WORKSPACES_CORE_ERROR_INITIALIZATION_FAILED,
					"No settings provided");

		/* Return failed state */
		return(FALSE);
	}

	if(!XFCE_OVERVIEW_WORKSPACES_IS_SETTINGS(priv->settings))
	{
		/* Set error */
		g_set_error(outError,
					XFCE_OVERVIEW_WORKSPACES_CORE_ERROR,
					XFCE_OVERVIEW_WORKSPACES_CORE_ERROR_INITIALIZATION_FAILED,
					"Expected settings of type %s but got %s",
					G_OBJECT_TYPE_NAME(priv->settings),
					g_type_name(XFCE_OVERVIEW_WORKSPACES_TYPE_SETTINGS));

		/* Return failed state */
		return(FALSE);
	}

	/* Set up keyboard and pointer bindings */
	priv->bindings=XFCE_OVERVIEW_WORKSPACES_BINDINGS_POOL(g_object_new(XFCE_OVERVIEW_WORKSPACES_TYPE_BINDINGS_POOL, NULL));
	if(!priv->bindings)
	{
		/* Set error */
		g_set_error(outError,
					XFCE_OVERVIEW_WORKSPACES_CORE_ERROR,
					XFCE_OVERVIEW_WORKSPACES_CORE_ERROR_INITIALIZATION_FAILED,
					"Could not initialize bindings");

		/* Return failed state */
		return(FALSE);
	}

	if(!xfce_overview_workspaces_bindings_pool_load(priv->bindings, &error))
	{
		/* Set error */
		g_propagate_error(outError, error);

		/* Return failed state */
		return(FALSE);
	}

	/* Create single-instance of window tracker backend to keep it alive while
	 * application is running and to avoid multiple reinitializations. It must
	 * be create before any class using a window tracker.
	 */
	priv->windowTrackerBackend=xfce_overview_workspaces_window_tracker_backend_create();
	if(!priv->windowTrackerBackend)
	{
		/* Set error */
		g_set_error(outError,
					XFCE_OVERVIEW_WORKSPACES_CORE_ERROR,
					XFCE_OVERVIEW_WORKSPACES_CORE_ERROR_INITIALIZATION_FAILED,
					"Could not setup window tracker backend");

		/* Return failed state */
		return(FALSE);
	}

	/* Set up application database */
	priv->appDatabase=XFCE_OVERVIEW_WORKSPACES_APPLICATION_DATABASE(g_object_new(XFCE_OVERVIEW_WORKSPACES_TYPE_APPLICATION_DATABASE, NULL));
	if(!priv->appDatabase)
	{
		/* Set error */
		g_set_error(outError,
					XFCE_OVERVIEW_WORKSPACES_CORE_ERROR,
					XFCE_OVERVIEW_WORKSPACES_CORE_ERROR_INITIALIZATION_FAILED,
					"Could not initialize application database");

		/* Return failed state */
		return(FALSE);
	}

	if(!xfce_overview_workspaces_application_database_load(priv->appDatabase, &error))
	{
		/* Set error */
		g_propagate_error(outError, error);

		/* Return failed state */
		return(FALSE);
	}

	/* Set up application tracker */
	priv->appTracker=XFCE_OVERVIEW_WORKSPACES_APPLICATION_TRACKER(g_object_new(XFCE_OVERVIEW_WORKSPACES_TYPE_APPLICATION_TRACKER, NULL));
	if(!priv->appTracker)
	{
		/* Set error */
		g_set_error(outError,
					XFCE_OVERVIEW_WORKSPACES_CORE_ERROR,
					XFCE_OVERVIEW_WORKSPACES_CORE_ERROR_INITIALIZATION_FAILED,
					"Could not initialize application tracker");

		/* Return failed state */
		return(FALSE);
	}

	/* Register built-in views (order of registration is important) */
	priv->viewManager=XFCE_OVERVIEW_WORKSPACES_VIEW_MANAGER(g_object_new(XFCE_OVERVIEW_WORKSPACES_TYPE_VIEW_MANAGER, NULL));

	xfce_overview_workspaces_view_manager_register(priv->viewManager, "builtin.windows", XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOWS_VIEW);
	xfce_overview_workspaces_view_manager_register(priv->viewManager, "builtin.applications", XFCE_OVERVIEW_WORKSPACES_TYPE_APPLICATIONS_VIEW);

	/* Create single-instance of focus manager to keep it alive while
	 * application is running.
	 */
	priv->focusManager=XFCE_OVERVIEW_WORKSPACES_FOCUS_MANAGER(g_object_new(XFCE_OVERVIEW_WORKSPACES_TYPE_FOCUS_MANAGER, NULL));

	/* Create single-instance of plugin manager to keep it alive while
	 * application is running.
	 */
	priv->pluginsManager=XFCE_OVERVIEW_WORKSPACES_PLUGINS_MANAGER(g_object_new(XFCE_OVERVIEW_WORKSPACES_TYPE_PLUGINS_MANAGER, NULL));
	if(!priv->pluginsManager)
	{
		/* Set error */
		g_set_error(outError,
					XFCE_OVERVIEW_WORKSPACES_CORE_ERROR,
					XFCE_OVERVIEW_WORKSPACES_CORE_ERROR_INITIALIZATION_FAILED,
					"Could not initialize plugin manager");

		/* Return failed state */
		return(FALSE);
	}

	if(!xfce_overview_workspaces_plugins_manager_setup(priv->pluginsManager))
	{
		/* Set error */
		g_set_error(outError,
					XFCE_OVERVIEW_WORKSPACES_CORE_ERROR,
					XFCE_OVERVIEW_WORKSPACES_CORE_ERROR_INITIALIZATION_FAILED,
					"Could not setup plugin manager");

		/* Return failed state */
		return(FALSE);
	}

	/* Set up and load theme */
	priv->themeChangedBinding=g_object_bind_property(priv->settings,
																"theme",
																self,
																"theme-name",
																G_BINDING_SYNC_CREATE);
	if(!priv->themeChangedBinding)
	{
		g_warning("Could not create binding between settings property and local resource for theme change notification.");
	}

	/* At this time the theme must have been loaded, either because we
	 * set the theme name when creating the property binding to settings
	 * what caused a call to function to set theme name in this object
	 * and also caused a reload of theme.
	 * So if no theme object is set in this object then loading theme has
	 * failed and we have to return FALSE.
	 */
	if(!priv->theme) return(FALSE);

	/* Create stage containing all monitors */
	priv->stage=XFCE_OVERVIEW_WORKSPACES_STAGE(xfce_overview_workspaces_stage_new());
	g_signal_connect_swapped(priv->stage, "delete-event", G_CALLBACK(_xfce_overview_workspaces_core_on_delete_stage), self);

	/* Emit signal 'theme-changed' to get current theme loaded at each stage created */
	g_signal_emit(self, XfceOverviewWorkspacesCoreSignals[SIGNAL_THEME_CHANGED], 0, priv->theme);

	/* Initialization was successful so send signal and return TRUE.
	 * The welcome notification is deferred to an idle callback (see above)
	 * to keep tree traversal and icon lookup off the first-frame path.
	 */
	g_signal_emit(self, XfceOverviewWorkspacesCoreSignals[SIGNAL_INITIALIZED], 0);

	g_idle_add_full(G_PRIORITY_LOW,
					_xfce_overview_workspaces_core_welcome_notify_idle,
					g_object_ref(self),
					g_object_unref);

	return(TRUE);
}

/**
 * xfce_overview_workspaces_core_has_default:
 *
 * Determine if the singleton instance of #XfceOverviewWorkspacesCore was created.
 * If the singleton instance of #XfceOverviewWorkspacesCore was created, it can be
 * retrieved with xfce_overview_workspaces_core_get_default().
 *
 * This function is useful if only the availability of the singleton instance
 * wants to be checked as xfce_overview_workspaces_core_get_default() will create this
 * singleton instance if not available.
 *
 * Return value: %TRUE if singleton instance of #XfceOverviewWorkspacesCore was
 *   created or %FALSE if not.
 */
gboolean xfce_overview_workspaces_core_has_default(void)
{
	if(G_LIKELY(_xfce_overview_workspaces_core)) return(TRUE);

	return(FALSE);
}

/**
 * xfce_overview_workspaces_core_get_default:
 *
 * Retrieves the singleton instance of #XfceOverviewWorkspacesCore.
 *
 * Return value: (transfer none): The instance of #XfceOverviewWorkspacesCore.
 *   The returned object is owned by XfceOverviewWorkspaces and it should not be
 *   unreferenced directly.
 */
XfceOverviewWorkspacesCore* xfce_overview_workspaces_core_get_default(void)
{
	if(G_UNLIKELY(!_xfce_overview_workspaces_core))
	{
		_xfce_overview_workspaces_core=g_object_new(XFCE_OVERVIEW_WORKSPACES_TYPE_CORE, NULL);
	}

	return(_xfce_overview_workspaces_core);
}

/**
 * xfce_overview_workspaces_core_is_quitting:
 * @self: A #XfceOverviewWorkspacesCore
 *
 * Checks if application is in progress to quit.
 *
 * Return value: %TRUE if @self is going to quit, otherwise %FALSE.
 */
gboolean xfce_overview_workspaces_core_is_quitting(XfceOverviewWorkspacesCore *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_CORE(self), FALSE);

	return(self->priv->isQuitting);
}

/**
 * xfce_overview_workspaces_core_quit:
 * @self: A #XfceOverviewWorkspacesCore or %NULL
 *
 * Requests the core instance at @self to quit by sending the ::quit signal.
 * The connected signal handlers are responsible to handle the request and
 * to shutdown or suspend the core instance.
 *
 * If @self is %NULL the default singleton is used if it was created.
 */
void xfce_overview_workspaces_core_quit(XfceOverviewWorkspacesCore *self)
{
	g_return_if_fail(self==NULL || XFCE_OVERVIEW_WORKSPACES_IS_CORE(self));

	/* Get default single instance if NULL is requested */
	if(!self) self=_xfce_overview_workspaces_core;

	/* Send quit request */
	if(G_LIKELY(self))
	{
		g_signal_emit(self, XfceOverviewWorkspacesCoreSignals[SIGNAL_QUIT], 0);
	}
}

/**
 * xfce_overview_workspaces_core_can_suspend:
 * @self: A #XfceOverviewWorkspacesCore or %NULL
 *
 * Asks the core instance at @self if suspend/resume is supported.
 * As the core itself does not provide any support of suspend/resume
 * it will emit the ::can-suspend signal to ask the connected signal
 * handlers (usally connected by the application using this library)
 * if they support suspend/resume.
 *
 * If any signal handler returns %TRUE, suspend/resume is supported
 * and %TRUE is also returned by this functions. Otherwise %FALSE
 * is returned indicating no support for suspend/resume.
 *
 * If @self is %NULL the default singleton is used if it was created.
 *
 * Return value: %TRUE if suspend/resume is supported, otherwise %FALSE.
 */
gboolean xfce_overview_workspaces_core_can_suspend(XfceOverviewWorkspacesCore *self)
{
	gboolean				suspendSupported;

	g_return_val_if_fail(self==NULL || XFCE_OVERVIEW_WORKSPACES_IS_CORE(self), FALSE);

	suspendSupported=FALSE;

	/* Get default single instance if NULL is requested */
	if(!self) self=_xfce_overview_workspaces_core;

	/* Emit "can-suspend" signal and collect return values */
	if(G_LIKELY(self))
	{
		g_signal_emit(self, XfceOverviewWorkspacesCoreSignals[SIGNAL_CAN_SUSPEND], 0, &suspendSupported);
	}

	/* Return collected value for support of suspend/resume */
	return(suspendSupported);
}

/**
 * xfce_overview_workspaces_core_is_suspended:
 * @self: A #XfceOverviewWorkspacesCore
 *
 * Checks if core instance at @self is suspended.
 *
 * Note: This state can only be checked when @self supports suspend/resume
 * which be checked by calling xfce_overview_workspaces_core_can_suspend() before.
 *
 * Return value: %TRUE if @self is suspended and %FALSE if it is resumed.
 */
gboolean xfce_overview_workspaces_core_is_suspended(XfceOverviewWorkspacesCore *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_CORE(self), FALSE);

	return(self->priv->isSuspended);
}

/**
 * xfce_overview_workspaces_core_suspend:
 * @self: A #XfceOverviewWorkspacesCore or %NULL
 *
 * Requests the core instance at @self to suspend by sending the ::suspend
 * signal. The connected signal handlers are responsible to handle the
 * request, e.g. hiding stage window etc..
 *
 * If @self is %NULL the default singleton is used if it was created.
 */
void xfce_overview_workspaces_core_suspend(XfceOverviewWorkspacesCore *self)
{
	g_return_if_fail(self==NULL || XFCE_OVERVIEW_WORKSPACES_IS_CORE(self));

	/* Get default single instance if NULL is requested */
	if(!self) self=_xfce_overview_workspaces_core;

	/* Send suspend request */
	if(G_LIKELY(self) &&
		xfce_overview_workspaces_core_can_suspend(self))
	{
		/* Emit signal */
		g_signal_emit(self, XfceOverviewWorkspacesCoreSignals[SIGNAL_SUSPEND], 0);

		/* Set flag for suspension */
		self->priv->isSuspended=TRUE;
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesCoreProperties[PROP_SUSPENDED]);
	}
}

/**
 * xfce_overview_workspaces_core_resume:
 * @self: A #XfceOverviewWorkspacesCore or %NULL
 *
 * Requests the core instance at @self to resume by sending the ::suspend
 * signal. The connected signal handlers are responsible to handle the
 * request, e.g. hiding stage window etc..
 *
 * If @self is %NULL the default singleton is used if it was created.
 */
void xfce_overview_workspaces_core_resume(XfceOverviewWorkspacesCore *self)
{
	g_return_if_fail(self==NULL || XFCE_OVERVIEW_WORKSPACES_IS_CORE(self));

	/* Get default single instance if NULL is requested */
	if(!self) self=_xfce_overview_workspaces_core;

	/* Send suspend request */
	if(G_LIKELY(self) &&
		xfce_overview_workspaces_core_can_suspend(self))
	{
		/* Emit signal */
		g_signal_emit(self, XfceOverviewWorkspacesCoreSignals[SIGNAL_RESUME], 0);

		/* Clear flag for suspension */
		self->priv->isSuspended=FALSE;
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesCoreProperties[PROP_SUSPENDED]);
	}
}

/**
 * xfce_overview_workspaces_core_get_stage:
 * @self: A #XfceOverviewWorkspacesCore or %NULL
 *
 * Retrieve #XfceOverviewWorkspacesStage of @self.
 *
 * If @self is %NULL the default singleton is used if it was created.
 *
 * Return value: (transfer none): The #XfceOverviewWorkspacesStage of application at @self.
 */
XfceOverviewWorkspacesStage* xfce_overview_workspaces_core_get_stage(XfceOverviewWorkspacesCore *self)
{
	XfceOverviewWorkspacesStage					*stage;

	g_return_val_if_fail(self==NULL || XFCE_OVERVIEW_WORKSPACES_IS_CORE(self), NULL);

	stage=NULL;

	/* Get default single instance if NULL is requested */
	if(!self) self=_xfce_overview_workspaces_core;

	/* Get xfconf channel */
	if(G_LIKELY(self)) stage=self->priv->stage;

	return(stage);
}

/**
 * xfce_overview_workspaces_core_get_theme:
 * @self: A #XfceOverviewWorkspacesCore or %NULL
 *
 * Retrieve the current #XfceOverviewWorkspacesTheme of @self.
 *
 * If @self is %NULL the default singleton is used if it was created.
 *
 * Return value: (transfer none): The current #XfceOverviewWorkspacesTheme of application at @self.
 */
XfceOverviewWorkspacesTheme* xfce_overview_workspaces_core_get_theme(XfceOverviewWorkspacesCore *self)
{
	XfceOverviewWorkspacesTheme					*theme;

	g_return_val_if_fail(self==NULL || XFCE_OVERVIEW_WORKSPACES_IS_CORE(self), NULL);

	theme=NULL;

	/* Get default single instance if NULL is requested */
	if(!self) self=_xfce_overview_workspaces_core;

	/* Get theme */
	if(G_LIKELY(self)) theme=self->priv->theme;

	return(theme);
}

/**
 * xfce_overview_workspaces_core_get_settings:
 * @self: A #XfceOverviewWorkspacesCore or %NULL
 *
 * Retrieve the #XfceOverviewWorkspacesSettings class of @self used to query or modify settings.
 *
 * If @self is %NULL the default singleton is used if it was created.
 *
 * Return value: (transfer none): The #XfceOverviewWorkspacesSettings of application at @self.
 */
XfceOverviewWorkspacesSettings* xfce_overview_workspaces_core_get_settings(XfceOverviewWorkspacesCore *self)
{
	XfceOverviewWorkspacesSettings					*settings;

	g_return_val_if_fail(self==NULL || XFCE_OVERVIEW_WORKSPACES_IS_CORE(self), NULL);

	settings=NULL;

	/* Get default single instance if NULL is requested */
	if(!self) self=_xfce_overview_workspaces_core;

	/* Get settings */
	if(G_LIKELY(self)) settings=self->priv->settings;

	return(settings);
}

/**
 * xfce_overview_workspaces_core_get_application_database:
 *
 * Retrieves the singleton instance of #XfceOverviewWorkspacesApplicationDatabase from
 * core instance at @self.
 *
 * Return value: (transfer full): The instance of #XfceOverviewWorkspacesApplicationDatabase.
 *   Use g_object_unref() when done.
 */
XfceOverviewWorkspacesApplicationDatabase* xfce_overview_workspaces_core_get_application_database(XfceOverviewWorkspacesCore *self)
{
	XfceOverviewWorkspacesApplicationDatabase		*applicationDatabase;

	g_return_val_if_fail(self==NULL || XFCE_OVERVIEW_WORKSPACES_IS_CORE(self), NULL);

	applicationDatabase=NULL;

	/* Get default single instance if NULL is requested */
	if(!self) self=_xfce_overview_workspaces_core;

	/* Get application database */
	if(G_LIKELY(self)) applicationDatabase=self->priv->appDatabase;

	/* Take extra reference on object */
	if(applicationDatabase) g_object_ref(applicationDatabase);

	return(applicationDatabase);
}

/**
 * xfce_overview_workspaces_core_get_application_tracker:
 *
 * Retrieves the singleton instance of #XfceOverviewWorkspacesApplicationTracker from
 * core instance at @self.
 *
 * Return value: (transfer full): The instance of #XfceOverviewWorkspacesApplicationTracker.
 *   Use g_object_unref() when done.
 */
XfceOverviewWorkspacesApplicationTracker* xfce_overview_workspaces_core_get_application_tracker(XfceOverviewWorkspacesCore *self)
{
	XfceOverviewWorkspacesApplicationTracker		*applicationTracker;

	g_return_val_if_fail(self==NULL || XFCE_OVERVIEW_WORKSPACES_IS_CORE(self), NULL);

	applicationTracker=NULL;

	/* Get default single instance if NULL is requested */
	if(!self) self=_xfce_overview_workspaces_core;

	/* Get application tracker */
	if(G_LIKELY(self)) applicationTracker=self->priv->appTracker;

	/* Take extra reference on object */
	if(applicationTracker) g_object_ref(applicationTracker);

	return(applicationTracker);
}

/**
 * xfce_overview_workspaces_core_get_bindings_pool:
 *
 * Retrieves the singleton instance of #XfceOverviewWorkspacesBindingsPool from
 * core instance at @self.
 *
 * Return value: (transfer full): The instance of #XfceOverviewWorkspacesBindingsPool.
 *   Use g_object_unref() when done.
 */
XfceOverviewWorkspacesBindingsPool* xfce_overview_workspaces_core_get_bindings_pool(XfceOverviewWorkspacesCore *self)
{
	XfceOverviewWorkspacesBindingsPool				*bindingsPool;

	g_return_val_if_fail(self==NULL || XFCE_OVERVIEW_WORKSPACES_IS_CORE(self), NULL);

	bindingsPool=NULL;

	/* Get default single instance if NULL is requested */
	if(!self) self=_xfce_overview_workspaces_core;

	/* Get bindings pool */
	if(G_LIKELY(self)) bindingsPool=self->priv->bindings;

	/* Take extra reference on object */
	if(bindingsPool) g_object_ref(bindingsPool);

	return(bindingsPool);
}

/**
 * xfce_overview_workspaces_core_get_focus_manager:
 *
 * Retrieves the singleton instance of #XfceOverviewWorkspacesFocusManager from
 * core instance at @self.
 *
 * Return value: (transfer full): The instance of #XfceOverviewWorkspacesFocusManager.
 *   Use g_object_unref() when done.
 */
XfceOverviewWorkspacesFocusManager* xfce_overview_workspaces_core_get_focus_manager(XfceOverviewWorkspacesCore *self)
{
	XfceOverviewWorkspacesFocusManager				*focusManager;

	g_return_val_if_fail(self==NULL || XFCE_OVERVIEW_WORKSPACES_IS_CORE(self), NULL);

	focusManager=NULL;

	/* Get default single instance if NULL is requested */
	if(!self) self=_xfce_overview_workspaces_core;

	/* Get focus manager */
	if(G_LIKELY(self)) focusManager=self->priv->focusManager;

	/* Take extra reference on object */
	if(focusManager) g_object_ref(focusManager);

	return(focusManager);
}

/**
 * xfce_overview_workspaces_core_get_plugins_manager:
 *
 * Retrieves the singleton instance of #XfceOverviewWorkspacesPluginsManager from
 * core instance at @self.
 *
 * Return value: (transfer full): The instance of #XfceOverviewWorkspacesPluginsManager.
 *   Use g_object_unref() when done.
 */
XfceOverviewWorkspacesPluginsManager* xfce_overview_workspaces_core_get_plugins_manager(XfceOverviewWorkspacesCore *self)
{
	XfceOverviewWorkspacesPluginsManager			*pluginsManager;

	g_return_val_if_fail(self==NULL || XFCE_OVERVIEW_WORKSPACES_IS_CORE(self), NULL);

	pluginsManager=NULL;

	/* Get default single instance if NULL is requested */
	if(!self) self=_xfce_overview_workspaces_core;

	/* Get plugins manager */
	if(G_LIKELY(self)) pluginsManager=self->priv->pluginsManager;

	/* Take extra reference on object */
	if(pluginsManager) g_object_ref(pluginsManager);

	return(pluginsManager);
}

/**
 * xfce_overview_workspaces_core_get_view_manager:
 *
 * Retrieves the singleton instance of #XfceOverviewWorkspacesViewManager from
 * core instance at @self.
 *
 * Return value: (transfer full): The instance of #XfceOverviewWorkspacesViewManager.
 *   Use g_object_unref() when done.
 */
XfceOverviewWorkspacesViewManager* xfce_overview_workspaces_core_get_view_manager(XfceOverviewWorkspacesCore *self)
{
	XfceOverviewWorkspacesViewManager				*viewManager;

	g_return_val_if_fail(self==NULL || XFCE_OVERVIEW_WORKSPACES_IS_CORE(self), NULL);

	viewManager=NULL;

	/* Get default single instance if NULL is requested */
	if(!self) self=_xfce_overview_workspaces_core;

	/* Get view manager */
	if(G_LIKELY(self)) viewManager=self->priv->viewManager;

	/* Take extra reference on object */
	if(viewManager) g_object_ref(viewManager);

	return(viewManager);
}

/**
 * xfce_overview_workspaces_core_get_window_tracker:
 *
 * Retrieves the singleton instance of #XfceOverviewWorkspacesWindowTracker from
 * core instance at @self.
 *
 * This function is the logical equivalent of:
 *
 * |[<!-- language="C" -->
 *   XfceOverviewWorkspacesWindowTrackerBackend *backend;
 *   XfceOverviewWorkspacesWindowTracker        *tracker;
 *
 *   backend=xfce_overview_workspaces_core_get_window_tracker_backend(core);
 *   tracker=xfce_overview_workspaces_window_tracker_backend_get_window_tracker(backend);
 *   g_object_unref(backend);
 * ]|
 *
 * Return value: (transfer full): The instance of #XfceOverviewWorkspacesWindowTracker.
 *   Use g_object_unref() when done.
 */
XfceOverviewWorkspacesWindowTracker* xfce_overview_workspaces_core_get_window_tracker(XfceOverviewWorkspacesCore *self)
{
	XfceOverviewWorkspacesWindowTracker			*windowTracker;

	g_return_val_if_fail(self==NULL || XFCE_OVERVIEW_WORKSPACES_IS_CORE(self), NULL);

	windowTracker=NULL;

	/* Get default single instance if NULL is requested */
	if(!self) self=_xfce_overview_workspaces_core;

	/* Get window tracker */
	if(G_LIKELY(self))
	{
		windowTracker=xfce_overview_workspaces_window_tracker_backend_get_window_tracker(self->priv->windowTrackerBackend);
	}

	/* Take extra reference on object */
	if(windowTracker) g_object_ref(windowTracker);

	return(windowTracker);
}

/**
 * xfce_overview_workspaces_core_get_window_tracker_backend:
 *
 * Retrieves the singleton instance of #XfceOverviewWorkspacesWindowTrackerBackend from
 * core instance at @self.
 *
 * Return value: (transfer full): The instance of #XfceOverviewWorkspacesWindowTrackerBackend.
 *   Use g_object_unref() when done.
 */
XfceOverviewWorkspacesWindowTrackerBackend* xfce_overview_workspaces_core_get_window_tracker_backend(XfceOverviewWorkspacesCore *self)
{
	XfceOverviewWorkspacesWindowTrackerBackend		*windowTrackerBackend;

	g_return_val_if_fail(self==NULL || XFCE_OVERVIEW_WORKSPACES_IS_CORE(self), NULL);

	windowTrackerBackend=NULL;

	/* Get default single instance if NULL is requested */
	if(!self) self=_xfce_overview_workspaces_core;

	/* Get window tracker backend */
	if(G_LIKELY(self)) windowTrackerBackend=self->priv->windowTrackerBackend;

	/* Take extra reference on object */
	if(windowTrackerBackend) g_object_ref(windowTrackerBackend);

	return(windowTrackerBackend);
}
