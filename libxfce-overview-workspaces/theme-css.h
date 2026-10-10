/*
 * theme-css: A theme used for rendering xfce-overview-workspaces actors with CSS.
 *            The parser and the handling of CSS files is heavily based
 *            on mx-css, mx-style and mx-stylable of library mx
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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_THEME_CSS__
#define __LIBXFCE_OVERVIEW_WORKSPACES_THEME_CSS__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <glib-object.h>

#include <libxfce-overview-workspaces/stylable.h>

G_BEGIN_DECLS

#define XFCE_OVERVIEW_WORKSPACES_TYPE_THEME_CSS					(xfce_overview_workspaces_theme_css_get_type())
#define XFCE_OVERVIEW_WORKSPACES_THEME_CSS(obj)					(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_THEME_CSS, XfceOverviewWorkspacesThemeCSS))
#define XFCE_OVERVIEW_WORKSPACES_IS_THEME_CSS(obj)				(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_THEME_CSS))
#define XFCE_OVERVIEW_WORKSPACES_THEME_CSS_CLASS(klass)			(G_TYPE_CHECK_CLASS_CAST((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_THEME_CSS, XfceOverviewWorkspacesThemeCSSClass))
#define XFCE_OVERVIEW_WORKSPACES_IS_THEME_CSS_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_THEME_CSS))
#define XFCE_OVERVIEW_WORKSPACES_THEME_CSS_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_THEME_CSS, XfceOverviewWorkspacesThemeCSSClass))

typedef struct _XfceOverviewWorkspacesThemeCSS					XfceOverviewWorkspacesThemeCSS;
typedef struct _XfceOverviewWorkspacesThemeCSSClass			XfceOverviewWorkspacesThemeCSSClass;
typedef struct _XfceOverviewWorkspacesThemeCSSPrivate			XfceOverviewWorkspacesThemeCSSPrivate;

/**
 * XfceOverviewWorkspacesThemeCSS:
 *
 * The #XfceOverviewWorkspacesThemeCSS structure contains only private data and
 * should be accessed using the provided API
 */
struct _XfceOverviewWorkspacesThemeCSS
{
	/*< private >*/
	/* Parent instance */
	GObject							parent_instance;

	/* Private structure */
	XfceOverviewWorkspacesThemeCSSPrivate		*priv;
};

/**
 * XfceOverviewWorkspacesThemeCSSClass:
 *
 * The #XfceOverviewWorkspacesThemeCSSClass structure contains only private data
 */struct _XfceOverviewWorkspacesThemeCSSClass
{
	/*< private >*/
	/* Parent class */
	GObjectClass					parent_class;

	/*< public >*/
	/* Virtual functions */
};

/* Errors */
/**
 * XFCE_OVERVIEW_WORKSPACES_THEME_CSS_ERROR:
 *
 * Error domain for theme CSS errors.
 * Errors in this domain will be from the #XfceOverviewWorkspacesThemeCSSError enumeration.
 */
#define XFCE_OVERVIEW_WORKSPACES_THEME_CSS_ERROR					(xfce_overview_workspaces_theme_css_error_quark())

GQuark xfce_overview_workspaces_theme_css_error_quark(void);

/**
 * XfceOverviewWorkspacesThemeCSSError:
 * @XFCE_OVERVIEW_WORKSPACES_THEME_CSS_ERROR_INVALID_ARGUMENT: An invalid argument was given
 * @XFCE_OVERVIEW_WORKSPACES_THEME_CSS_ERROR_UNSUPPORTED_STREAM: The source CSS is not a file
 * @XFCE_OVERVIEW_WORKSPACES_THEME_CSS_ERROR_PARSER_ERROR: The CSS file could not be parsed
 * @XFCE_OVERVIEW_WORKSPACES_THEME_CSS_ERROR_FUNCTION_ERROR: A function used in CSS failed
 *
 * Error codes returned by theme CSS.
 */
typedef enum /*< prefix=XFCE_OVERVIEW_WORKSPACES_THEME_CSS_ERROR >*/
{
	XFCE_OVERVIEW_WORKSPACES_THEME_CSS_ERROR_INVALID_ARGUMENT,
	XFCE_OVERVIEW_WORKSPACES_THEME_CSS_ERROR_UNSUPPORTED_STREAM,
	XFCE_OVERVIEW_WORKSPACES_THEME_CSS_ERROR_PARSER_ERROR,
	XFCE_OVERVIEW_WORKSPACES_THEME_CSS_ERROR_FUNCTION_ERROR
} XfceOverviewWorkspacesThemeCSSError;

/* Public declarations */
typedef struct _XfceOverviewWorkspacesThemeCSSValue			XfceOverviewWorkspacesThemeCSSValue;

/**
 * XfceOverviewWorkspacesThemeCSSValue:
 * @value: The textual value (string) of a CSS property
 * @source: The source CSS file
 */
struct _XfceOverviewWorkspacesThemeCSSValue
{
	const gchar						*value;
	const gchar						*source;
};

/* Public API */
GType xfce_overview_workspaces_theme_css_get_type(void) G_GNUC_CONST;

XfceOverviewWorkspacesThemeCSS* xfce_overview_workspaces_theme_css_new(const gchar *inThemePath);

gboolean xfce_overview_workspaces_theme_css_add_file(XfceOverviewWorkspacesThemeCSS *self,
											const gchar *inPath,
											gint inPriority,
											GError **outError);

GHashTable* xfce_overview_workspaces_theme_css_get_properties(XfceOverviewWorkspacesThemeCSS *self,
													XfceOverviewWorkspacesStylable *inStylable);

G_END_DECLS

#endif	/* __LIBXFCE_OVERVIEW_WORKSPACES_THEME_CSS__ */
