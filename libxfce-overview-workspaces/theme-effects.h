/*
 * theme-effects: A theme used for build effects by XML files
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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_THEME_EFFECTS__
#define __LIBXFCE_OVERVIEW_WORKSPACES_THEME_EFFECTS__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <clutter/clutter.h>

G_BEGIN_DECLS

#define XFCE_OVERVIEW_WORKSPACES_TYPE_THEME_EFFECTS				(xfce_overview_workspaces_theme_effects_get_type())
#define XFCE_OVERVIEW_WORKSPACES_THEME_EFFECTS(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_THEME_EFFECTS, XfceOverviewWorkspacesThemeEffects))
#define XFCE_OVERVIEW_WORKSPACES_IS_THEME_EFFECTS(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_THEME_EFFECTS))
#define XFCE_OVERVIEW_WORKSPACES_THEME_EFFECTS_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_THEME_EFFECTS, XfceOverviewWorkspacesThemeEffectsClass))
#define XFCE_OVERVIEW_WORKSPACES_IS_THEME_EFFECTS_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_THEME_EFFECTS))
#define XFCE_OVERVIEW_WORKSPACES_THEME_EFFECTS_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_THEME_EFFECTS, XfceOverviewWorkspacesThemeEffectsClass))

typedef struct _XfceOverviewWorkspacesThemeEffects				XfceOverviewWorkspacesThemeEffects;
typedef struct _XfceOverviewWorkspacesThemeEffectsClass		XfceOverviewWorkspacesThemeEffectsClass;
typedef struct _XfceOverviewWorkspacesThemeEffectsPrivate		XfceOverviewWorkspacesThemeEffectsPrivate;

/**
 * XfceOverviewWorkspacesThemeEffects:
 *
 * The #XfceOverviewWorkspacesThemeEffects structure contains only private data and
 * should be accessed using the provided API
 */
struct _XfceOverviewWorkspacesThemeEffects
{
	/*< private >*/
	/* Parent instance */
	GObject								parent_instance;

	/* Private structure */
	XfceOverviewWorkspacesThemeEffectsPrivate		*priv;
};

/**
 * XfceOverviewWorkspacesThemeEffectsClass:
 *
 * The #XfceOverviewWorkspacesThemeEffectsClass structure contains only private data
 */
struct _XfceOverviewWorkspacesThemeEffectsClass
{
	/*< private >*/
	/* Parent class */
	GObjectClass						parent_class;

	/*< public >*/
	/* Virtual functions */
};

/* Errors */
/**
 * XFCE_OVERVIEW_WORKSPACES_THEME_EFFECTS_ERROR:
 *
 * Error domain for theme effect errors.
 * Errors in this domain will be from the #XfceOverviewWorkspacesThemeEffectsError enumeration.
 */
#define XFCE_OVERVIEW_WORKSPACES_THEME_EFFECTS_ERROR				(xfce_overview_workspaces_theme_effects_error_quark())

GQuark xfce_overview_workspaces_theme_effects_error_quark(void);

/**
 * XfceOverviewWorkspacesThemeEffectsError:
 * @XFCE_OVERVIEW_WORKSPACES_THEME_EFFECTS_ERROR_ERROR: A general theme effect error
 * @XFCE_OVERVIEW_WORKSPACES_THEME_EFFECTS_ERROR_MALFORMED: The XML of theme effect is malformed
 *
 * Error codes returned by loading theme effects.
 */
typedef enum /*< prefix=XFCE_OVERVIEW_WORKSPACES_THEME_EFFECTS_ERROR >*/
{
	XFCE_OVERVIEW_WORKSPACES_THEME_EFFECTS_ERROR_ERROR,
	XFCE_OVERVIEW_WORKSPACES_THEME_EFFECTS_ERROR_MALFORMED,
} XfceOverviewWorkspacesThemeEffectsError;

/* Public API */
GType xfce_overview_workspaces_theme_effects_get_type(void) G_GNUC_CONST;

XfceOverviewWorkspacesThemeEffects* xfce_overview_workspaces_theme_effects_new(void);

gboolean xfce_overview_workspaces_theme_effects_add_file(XfceOverviewWorkspacesThemeEffects *self,
											const gchar *inPath,
											GError **outError);

ClutterEffect* xfce_overview_workspaces_theme_effects_create_effect(XfceOverviewWorkspacesThemeEffects *self,
														const gchar *inID);

G_END_DECLS

#endif	/* __LIBXFCE_OVERVIEW_WORKSPACES_THEME_EFFECTS__ */
