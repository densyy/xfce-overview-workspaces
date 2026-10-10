/*
 * animation: A animation for an actor
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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_ANIMATION__
#define __LIBXFCE_OVERVIEW_WORKSPACES_ANIMATION__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <clutter/clutter.h>

#include <libxfce-overview-workspaces/actor.h>
#include <libxfce-overview-workspaces/css-selector.h>


G_BEGIN_DECLS

/* Object declaration */
#define XFCE_OVERVIEW_WORKSPACES_TYPE_ANIMATION				(xfce_overview_workspaces_animation_get_type())
#define XFCE_OVERVIEW_WORKSPACES_ANIMATION(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_ANIMATION, XfceOverviewWorkspacesAnimation))
#define XFCE_OVERVIEW_WORKSPACES_IS_ANIMATION(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_ANIMATION))
#define XFCE_OVERVIEW_WORKSPACES_ANIMATION_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_ANIMATION, XfceOverviewWorkspacesAnimationClass))
#define XFCE_OVERVIEW_WORKSPACES_IS_ANIMATION_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_ANIMATION))
#define XFCE_OVERVIEW_WORKSPACES_ANIMATION_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_ANIMATION, XfceOverviewWorkspacesAnimationClass))

typedef struct _XfceOverviewWorkspacesAnimation			XfceOverviewWorkspacesAnimation;
typedef struct _XfceOverviewWorkspacesAnimationClass		XfceOverviewWorkspacesAnimationClass;
typedef struct _XfceOverviewWorkspacesAnimationPrivate		XfceOverviewWorkspacesAnimationPrivate;

/**
 * XfceOverviewWorkspacesAnimation:
 *
 * The #XfceOverviewWorkspacesAnimation structure contains only private data and
 * should be accessed using the provided API
 */
struct _XfceOverviewWorkspacesAnimation
{
	/*< private >*/
	/* Parent instance */
	GObject								parent_instance;

	/* Private structure */
	XfceOverviewWorkspacesAnimationPrivate			*priv;
};

/**
 * XfceOverviewWorkspacesAnimationClass:
 *
 * The #XfceOverviewWorkspacesAnimationClass structure contains only private data
 */
struct _XfceOverviewWorkspacesAnimationClass
{
	/*< private >*/
	/* Parent class */
	GObjectClass						parent_class;

	/*< public >*/
	/* Virtual functions */
	void (*add_animation)(XfceOverviewWorkspacesAnimation *self, ClutterActor *inActor, ClutterTransition *inTransition);
	void (*animation_done)(XfceOverviewWorkspacesAnimation *self);
};

/**
 * XfceOverviewWorkspacesAnimationValue:
 * @selector: A #XfceOverviewWorkspacesCssSelector to find matching actors for the property's
 *   value in animation or %NULL to match sender
 * @property: A string containing the name of the property this value belongs to
 * @value: A #GValue containing the value for the property
 *
 */
struct _XfceOverviewWorkspacesAnimationValue
{
	XfceOverviewWorkspacesCssSelector				*selector;
	gchar								*property;
	GValue								*value;
};

typedef struct _XfceOverviewWorkspacesAnimationValue		XfceOverviewWorkspacesAnimationValue;

/**
 * XfceOverviewWorkspacesAnimationCreateFlags:
 * @XFCE_OVERVIEW_WORKSPACES_ANIMATION_CREATE_FLAG_ALLOW_EMPTY: Return an empty animation
 *   when no animation for requested signal or ID does not exist
 *
 * Flags passed on creation of animations.
 */
typedef enum /*< prefix=XFCE_OVERVIEW_WORKSPACES_ANIMATION_CREATE_FLAG >*/
{
	XFCE_OVERVIEW_WORKSPACES_ANIMATION_CREATE_FLAG_ALLOW_EMPTY = 1 << 0
} XfceOverviewWorkspacesAnimationCreateFlags;


/* Public API */
GType xfce_overview_workspaces_animation_get_type(void) G_GNUC_CONST;

XfceOverviewWorkspacesAnimation* xfce_overview_workspaces_animation_new(XfceOverviewWorkspacesActor *inSender,
												const gchar *inSignal,
												XfceOverviewWorkspacesAnimationCreateFlags inFlags);
XfceOverviewWorkspacesAnimation* xfce_overview_workspaces_animation_new_with_values(XfceOverviewWorkspacesActor *inSender,
															const gchar *inSignal,
															XfceOverviewWorkspacesAnimationCreateFlags inFlags,
															XfceOverviewWorkspacesAnimationValue **inDefaultInitialValues,
															XfceOverviewWorkspacesAnimationValue **inDefaultFinalValues);

XfceOverviewWorkspacesAnimation* xfce_overview_workspaces_animation_new_by_id(XfceOverviewWorkspacesActor *inSender,
														const gchar *inID,
														XfceOverviewWorkspacesAnimationCreateFlags inFlags);
XfceOverviewWorkspacesAnimation* xfce_overview_workspaces_animation_new_by_id_with_values(XfceOverviewWorkspacesActor *inSender,
																	const gchar *inID,
																	XfceOverviewWorkspacesAnimationCreateFlags inFlags,
																	XfceOverviewWorkspacesAnimationValue **inDefaultInitialValues,
																	XfceOverviewWorkspacesAnimationValue **inDefaultFinalValues);

gboolean xfce_overview_workspaces_animation_has_animation(XfceOverviewWorkspacesActor *inSender, const gchar *inSignal);

const gchar* xfce_overview_workspaces_animation_get_id(XfceOverviewWorkspacesAnimation *self);

gboolean xfce_overview_workspaces_animation_is_empty(XfceOverviewWorkspacesAnimation *self);

void xfce_overview_workspaces_animation_run(XfceOverviewWorkspacesAnimation *self);

void xfce_overview_workspaces_animation_ensure_complete(XfceOverviewWorkspacesAnimation *self);

void xfce_overview_workspaces_animation_dump(XfceOverviewWorkspacesAnimation *self);

XfceOverviewWorkspacesAnimationValue** xfce_overview_workspaces_animation_defaults_new(gint inNumberValues, ...);
void xfce_overview_workspaces_animation_defaults_free(XfceOverviewWorkspacesAnimationValue **inDefaultValues);

G_END_DECLS

#endif	/* __LIBXFCE_OVERVIEW_WORKSPACES_ANIMATION__ */
