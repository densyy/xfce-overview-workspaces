/*
 * theme-animation: A theme used for building animations by XML files
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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_THEME_ANIMATION__
#define __LIBXFCE_OVERVIEW_WORKSPACES_THEME_ANIMATION__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <clutter/clutter.h>

#include <libxfce-overview-workspaces/actor.h>
#include <libxfce-overview-workspaces/animation.h>

G_BEGIN_DECLS


/* Object declaration */
#define XFCE_OVERVIEW_WORKSPACES_TYPE_THEME_ANIMATION				(xfce_overview_workspaces_theme_animation_get_type())
#define XFCE_OVERVIEW_WORKSPACES_THEME_ANIMATION(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_THEME_ANIMATION, XfceOverviewWorkspacesThemeAnimation))
#define XFCE_OVERVIEW_WORKSPACES_IS_THEME_ANIMATION(obj)				(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_THEME_ANIMATION))
#define XFCE_OVERVIEW_WORKSPACES_THEME_ANIMATION_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_THEME_ANIMATION, XfceOverviewWorkspacesThemeAnimationClass))
#define XFCE_OVERVIEW_WORKSPACES_IS_THEME_ANIMATION_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_THEME_ANIMATION))
#define XFCE_OVERVIEW_WORKSPACES_THEME_ANIMATION_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_THEME_ANIMATION, XfceOverviewWorkspacesThemeAnimationClass))

typedef struct _XfceOverviewWorkspacesThemeAnimation				XfceOverviewWorkspacesThemeAnimation;
typedef struct _XfceOverviewWorkspacesThemeAnimationClass			XfceOverviewWorkspacesThemeAnimationClass;
typedef struct _XfceOverviewWorkspacesThemeAnimationPrivate		XfceOverviewWorkspacesThemeAnimationPrivate;

/**
 * XfceOverviewWorkspacesThemeAnimation:
 *
 * The #XfceOverviewWorkspacesThemeAnimation structure contains only private data and
 * should be accessed using the provided API
 */
struct _XfceOverviewWorkspacesThemeAnimation
{
	/*< private >*/
	/* Parent instance */
	GObject									parent_instance;

	/* Private structure */
	XfceOverviewWorkspacesThemeAnimationPrivate		*priv;
};

/**
 * XfceOverviewWorkspacesThemeAnimationClass:
 *
 * The #XfceOverviewWorkspacesThemeAnimationClass structure contains only private data
 */
struct _XfceOverviewWorkspacesThemeAnimationClass
{
	/*< private >*/
	/* Parent class */
	GObjectClass							parent_class;

	/*< public >*/
	/* Virtual functions */
};

/* Errors */
/**
 * XFCE_OVERVIEW_WORKSPACES_THEME_ANIMATION_ERROR:
 *
 * Error domain for theme animation errors.
 * Errors in this domain will be from the #XfceOverviewWorkspacesThemeAnimationError enumeration.
 */
#define XFCE_OVERVIEW_WORKSPACES_THEME_ANIMATION_ERROR				(xfce_overview_workspaces_theme_animation_error_quark())

GQuark xfce_overview_workspaces_theme_animation_error_quark(void);

/**
 * XfceOverviewWorkspacesThemeAnimationError:
 * @XFCE_OVERVIEW_WORKSPACES_THEME_ANIMATION_ERROR_ERROR: A general theme animation error
 * @XFCE_OVERVIEW_WORKSPACES_THEME_ANIMATION_ERROR_MALFORMED: The XML of theme animation is
 *   malformed
 *
 * Error codes returned by loading theme animations.
 */
typedef enum /*< prefix=XFCE_OVERVIEW_WORKSPACES_THEME_ANIMATION_ERROR >*/
{
	XFCE_OVERVIEW_WORKSPACES_THEME_ANIMATION_ERROR_ERROR,
	XFCE_OVERVIEW_WORKSPACES_THEME_ANIMATION_ERROR_MALFORMED,
} XfceOverviewWorkspacesThemeAnimationError;

/* Public API */
GType xfce_overview_workspaces_theme_animation_get_type(void) G_GNUC_CONST;

XfceOverviewWorkspacesThemeAnimation* xfce_overview_workspaces_theme_animation_new(void);

gboolean xfce_overview_workspaces_theme_animation_add_file(XfceOverviewWorkspacesThemeAnimation *self,
											const gchar *inPath,
											GError **outError);

XfceOverviewWorkspacesAnimation* xfce_overview_workspaces_theme_animation_create(XfceOverviewWorkspacesThemeAnimation *self,
															XfceOverviewWorkspacesActor *inSender,
															const gchar *inID,
															XfceOverviewWorkspacesAnimationValue **inDefaultInitialValues,
															XfceOverviewWorkspacesAnimationValue **inDefaultFinalValues);

gchar* xfce_overview_workspaces_theme_animation_lookup_id(XfceOverviewWorkspacesThemeAnimation *self,
															XfceOverviewWorkspacesActor *inSender,
															const gchar *inSignal);

G_END_DECLS

#endif	/* __LIBXFCE_OVERVIEW_WORKSPACES_THEME_ANIMATION__ */
