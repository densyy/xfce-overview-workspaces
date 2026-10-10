/*
 * theme-layout: A theme used for build and layout objects by XML files
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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_THEME_LAYOUT__
#define __LIBXFCE_OVERVIEW_WORKSPACES_THEME_LAYOUT__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <clutter/clutter.h>

G_BEGIN_DECLS

/* Public definitions */
/**
 * XfceOverviewWorkspacesThemeLayoutBuildGet:
 * @XFCE_OVERVIEW_WORKSPACES_THEME_LAYOUT_BUILD_GET_FOCUSABLES: Get #GPtrArray of pointer to
 *   defined focusable actors. Caller must free returned #GPtrArray with
 *   g_ptr_array_unref().
 * @XFCE_OVERVIEW_WORKSPACES_THEME_LAYOUT_BUILD_GET_SELECTED_FOCUS: Get #ClutterActor which
 *   should gain the focus. Caller must unref returned #ClutterActor with
 *   g_object_unref().
 *
 * The extra data to fetch when building an object from theme layout.
 */
typedef enum /*< prefix=XFCE_OVERVIEW_WORKSPACES_THEME_LAYOUT_BUILD_GET >*/
{
	XFCE_OVERVIEW_WORKSPACES_THEME_LAYOUT_BUILD_GET_FOCUSABLES=0,
	XFCE_OVERVIEW_WORKSPACES_THEME_LAYOUT_BUILD_GET_SELECTED_FOCUS
} XfceOverviewWorkspacesThemeLayoutBuildGet;

/* Object declaration */
#define XFCE_OVERVIEW_WORKSPACES_TYPE_THEME_LAYOUT				(xfce_overview_workspaces_theme_layout_get_type())
#define XFCE_OVERVIEW_WORKSPACES_THEME_LAYOUT(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_THEME_LAYOUT, XfceOverviewWorkspacesThemeLayout))
#define XFCE_OVERVIEW_WORKSPACES_IS_THEME_LAYOUT(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_THEME_LAYOUT))
#define XFCE_OVERVIEW_WORKSPACES_THEME_LAYOUT_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_THEME_LAYOUT, XfceOverviewWorkspacesThemeLayoutClass))
#define XFCE_OVERVIEW_WORKSPACES_IS_THEME_LAYOUT_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_THEME_LAYOUT))
#define XFCE_OVERVIEW_WORKSPACES_THEME_LAYOUT_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_THEME_LAYOUT, XfceOverviewWorkspacesThemeLayoutClass))

typedef struct _XfceOverviewWorkspacesThemeLayout				XfceOverviewWorkspacesThemeLayout;
typedef struct _XfceOverviewWorkspacesThemeLayoutClass			XfceOverviewWorkspacesThemeLayoutClass;
typedef struct _XfceOverviewWorkspacesThemeLayoutPrivate		XfceOverviewWorkspacesThemeLayoutPrivate;

/**
 * XfceOverviewWorkspacesThemeLayout:
 *
 * The #XfceOverviewWorkspacesThemeLayout structure contains only private data and
 * should be accessed using the provided API
 */
struct _XfceOverviewWorkspacesThemeLayout
{
	/*< private >*/
	/* Parent instance */
	GObject								parent_instance;

	/* Private structure */
	XfceOverviewWorkspacesThemeLayoutPrivate		*priv;
};

/**
 * XfceOverviewWorkspacesThemeLayoutClass:
 *
 * The #XfceOverviewWorkspacesThemeLayoutClass structure contains only private data
 */
struct _XfceOverviewWorkspacesThemeLayoutClass
{
	/*< private >*/
	/* Parent class */
	GObjectClass						parent_class;

	/*< public >*/
	/* Virtual functions */
};

/* Errors */
/**
 * XFCE_OVERVIEW_WORKSPACES_THEME_LAYOUT_ERROR:
 *
 * Error domain for theme layout errors.
 * Errors in this domain will be from the #XfceOverviewWorkspacesThemeLayoutError enumeration.
 */
#define XFCE_OVERVIEW_WORKSPACES_THEME_LAYOUT_ERROR				(xfce_overview_workspaces_theme_layout_error_quark())

GQuark xfce_overview_workspaces_theme_layout_error_quark(void);

/**
 * XfceOverviewWorkspacesThemeLayoutError:
 * @XFCE_OVERVIEW_WORKSPACES_THEME_LAYOUT_ERROR_ERROR: A general theme layout error
 * @XFCE_OVERVIEW_WORKSPACES_THEME_LAYOUT_ERROR_MALFORMED: The XML of theme layout is malformed
 *
 * Error codes returned by loading theme layout.
 */
typedef enum /*< prefix=XFCE_OVERVIEW_WORKSPACES_THEME_LAYOUT_ERROR >*/
{
	XFCE_OVERVIEW_WORKSPACES_THEME_LAYOUT_ERROR_ERROR,
	XFCE_OVERVIEW_WORKSPACES_THEME_LAYOUT_ERROR_MALFORMED,
} XfceOverviewWorkspacesThemeLayoutError;

/* Public API */
GType xfce_overview_workspaces_theme_layout_get_type(void) G_GNUC_CONST;

XfceOverviewWorkspacesThemeLayout* xfce_overview_workspaces_theme_layout_new(void);

gboolean xfce_overview_workspaces_theme_layout_add_file(XfceOverviewWorkspacesThemeLayout *self,
											const gchar *inPath,
											GError **outError);

ClutterActor* xfce_overview_workspaces_theme_layout_build_interface(XfceOverviewWorkspacesThemeLayout *self,
														const gchar *inID,
														...);

G_END_DECLS

#endif	/* __LIBXFCE_OVERVIEW_WORKSPACES_THEME_LAYOUT__ */
