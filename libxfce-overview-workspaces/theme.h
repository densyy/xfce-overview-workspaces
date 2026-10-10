/*
 * theme: Top-level theme object (parses key file and manages loading
 *        resources like css style files, xml layout files etc.)
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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_THEME__
#define __LIBXFCE_OVERVIEW_WORKSPACES_THEME__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <clutter/clutter.h>

#include <libxfce-overview-workspaces/theme-css.h>
#include <libxfce-overview-workspaces/theme-layout.h>
#include <libxfce-overview-workspaces/theme-effects.h>
#include <libxfce-overview-workspaces/theme-animation.h>

G_BEGIN_DECLS

#define XFCE_OVERVIEW_WORKSPACES_TYPE_THEME					(xfce_overview_workspaces_theme_get_type())
#define XFCE_OVERVIEW_WORKSPACES_THEME(obj)					(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_THEME, XfceOverviewWorkspacesTheme))
#define XFCE_OVERVIEW_WORKSPACES_IS_THEME(obj)				(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_THEME))
#define XFCE_OVERVIEW_WORKSPACES_THEME_CLASS(klass)			(G_TYPE_CHECK_CLASS_CAST((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_THEME, XfceOverviewWorkspacesThemeClass))
#define XFCE_OVERVIEW_WORKSPACES_IS_THEME_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_THEME))
#define XFCE_OVERVIEW_WORKSPACES_THEME_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_THEME, XfceOverviewWorkspacesThemeClass))

typedef struct _XfceOverviewWorkspacesTheme				XfceOverviewWorkspacesTheme;
typedef struct _XfceOverviewWorkspacesThemeClass			XfceOverviewWorkspacesThemeClass;
typedef struct _XfceOverviewWorkspacesThemePrivate			XfceOverviewWorkspacesThemePrivate;

/**
 * XfceOverviewWorkspacesTheme:
 *
 * The #XfceOverviewWorkspacesTheme structure contains only private data and
 * should be accessed using the provided API
 */
struct _XfceOverviewWorkspacesTheme
{
	/*< private >*/
	/* Parent instance */
	GObject						parent_instance;

	/* Private structure */
	XfceOverviewWorkspacesThemePrivate		*priv;
};

/**
 * XfceOverviewWorkspacesThemeClass:
 *
 * The #XfceOverviewWorkspacesThemeClass structure contains only private data
 */
struct _XfceOverviewWorkspacesThemeClass
{
	/*< private >*/
	/* Parent class */
	GObjectClass				parent_class;

	/*< public >*/
	/* Virtual functions */
};

/* Errors */
/**
 * XFCE_OVERVIEW_WORKSPACES_THEME_ERROR:
 *
 * Error domain for theme errors.
 * Errors in this domain will be from the #XfceOverviewWorkspacesThemeError enumeration.
 */
#define XFCE_OVERVIEW_WORKSPACES_THEME_ERROR					(xfce_overview_workspaces_theme_error_quark())

GQuark xfce_overview_workspaces_theme_error_quark(void);

/**
 * XfceOverviewWorkspacesThemeError:
 * @XFCE_OVERVIEW_WORKSPACES_THEME_ERROR_THEME_NOT_FOUND: The theme with requested name was
 *   not found
 * @XFCE_OVERVIEW_WORKSPACES_THEME_ERROR_ALREADY_LOADED: The theme was already loaded
 *
 * Error codes returned by loading theme.
 */
typedef enum /*< prefix=XFCE_OVERVIEW_WORKSPACES_THEME_ERROR >*/
{
	XFCE_OVERVIEW_WORKSPACES_THEME_ERROR_THEME_NOT_FOUND,
	XFCE_OVERVIEW_WORKSPACES_THEME_ERROR_ALREADY_LOADED
} XfceOverviewWorkspacesThemeError;


/* Public API */
GType xfce_overview_workspaces_theme_get_type(void) G_GNUC_CONST;

XfceOverviewWorkspacesTheme* xfce_overview_workspaces_theme_new(const gchar *inThemeName);

const gchar* xfce_overview_workspaces_theme_get_path(XfceOverviewWorkspacesTheme *self);

const gchar* xfce_overview_workspaces_theme_get_theme_name(XfceOverviewWorkspacesTheme *self);
const gchar* xfce_overview_workspaces_theme_get_display_name(XfceOverviewWorkspacesTheme *self);
const gchar* xfce_overview_workspaces_theme_get_comment(XfceOverviewWorkspacesTheme *self);

gboolean xfce_overview_workspaces_theme_load(XfceOverviewWorkspacesTheme *self,
								GError **outError);

XfceOverviewWorkspacesThemeCSS* xfce_overview_workspaces_theme_get_css(XfceOverviewWorkspacesTheme *self);
XfceOverviewWorkspacesThemeLayout* xfce_overview_workspaces_theme_get_layout(XfceOverviewWorkspacesTheme *self);
XfceOverviewWorkspacesThemeEffects* xfce_overview_workspaces_theme_get_effects(XfceOverviewWorkspacesTheme *self);
XfceOverviewWorkspacesThemeAnimation* xfce_overview_workspaces_theme_get_animation(XfceOverviewWorkspacesTheme *self);

G_END_DECLS

#endif	/* __LIBXFCE_OVERVIEW_WORKSPACES_THEME__ */
