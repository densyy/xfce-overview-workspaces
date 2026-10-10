/*
 * actor: Abstract base actor
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

#include <libxfce-overview-workspaces/actor.h>

#include <glib/gi18n-lib.h>

#include <libxfce-overview-workspaces/core.h>
#include <libxfce-overview-workspaces/stylable.h>
#include <libxfce-overview-workspaces/focusable.h>
#include <libxfce-overview-workspaces/animation.h>
#include <libxfce-overview-workspaces/utils.h>
#include <libxfce-overview-workspaces/debug.h>
#include <math.h>


/* Define this class in GObject system */
static gpointer				xfce_overview_workspaces_actor_parent_class=NULL;
static gint					xfce_overview_workspaces_actor_private_offset=0;

static gpointer xfce_overview_workspaces_actor_get_instance_private(XfceOverviewWorkspacesActor *self);
void xfce_overview_workspaces_actor_class_init(XfceOverviewWorkspacesActorClass *klass);
void xfce_overview_workspaces_actor_init(XfceOverviewWorkspacesActor *self);
void xfce_overview_workspaces_actor_base_class_finalize(XfceOverviewWorkspacesActorClass *klass);

struct _XfceOverviewWorkspacesActorPrivate
{
	/* Properties related */
	gboolean						canFocus;
	gchar							*effects;
	gboolean						visibility;

	gchar							*styleClasses;
	gchar							*stylePseudoClasses;

	/* Instance related */
	gboolean						inDestruction;

	GHashTable						*lastThemeStyleSet;
	gboolean						forceStyleRevalidation;

	gboolean						isFirstParent;

	gboolean						firstTimeMapped;
	XfceOverviewWorkspacesAnimation			*firstTimeMappedAnimation;

	GSList							*animations;

	ClutterActorBox					*allocationTrackBox;

	gboolean						disallowAllocationAnimation;
	gboolean						doAllocationAnimation;
	XfceOverviewWorkspacesAnimation			*allocationAnimation;
	ClutterActorBox					*allocationInitialBox;
	ClutterActorBox					*allocationFinalBox;

	gboolean						shouldVisible;
};

/* Properties */
enum
{
	PROP_0,

	PROP_CAN_FOCUS,
	PROP_EFFECTS,
	PROP_VISIBILITY,

	/* Overriden properties of interface: XfceOverviewWorkspacesStylable */
	PROP_STYLE_CLASSES,
	PROP_STYLE_PSEUDO_CLASSES,

	PROP_LAST
};

static GParamSpec* XfceOverviewWorkspacesActorProperties[PROP_LAST]={ 0, };

/* IMPLEMENTATION: Private variables and methods */
#define ALLOCATION_ANIMATION_SIGNAL		"move-resize"

typedef struct _XfceOverviewWorkspacesActorAnimationEntry		XfceOverviewWorkspacesActorAnimationEntry;
struct _XfceOverviewWorkspacesActorAnimationEntry
{
	gboolean									inDestruction;
	gchar										*signal;
	XfceOverviewWorkspacesAnimation						*animation;
};

#define XFCE_OVERVIEW_WORKSPACES_ACTOR_PARAM_SPEC_REF		(_xfce_overview_workspaces_actor_param_spec_ref_quark())

static GParamSpecPool		*_xfce_overview_workspaces_actor_stylable_properties_pool=NULL;


/* Quark declarations */
static GQuark _xfce_overview_workspaces_actor_param_spec_ref_quark(void)
{
	return(g_quark_from_static_string("xfce-overview-workspaces-actor-param-spec-ref-quark"));
}


/* Free an animation entry */
static void _xfce_overview_workspaces_actor_animation_entry_free(XfceOverviewWorkspacesActorAnimationEntry *inData)
{
	g_return_if_fail(inData);

	/* Do not free anything if this entry is already in destruction */
	if(inData->inDestruction) return;

	/* Set flag that this data will be freed now as this function could
	 * be called recursive (e.g. by other signal handlers) resulting
	 * in double-free.
	 */
	inData->inDestruction=TRUE;

	/* Release allocated resources */
	if(inData->animation) g_object_unref(inData->animation);
	if(inData->signal) g_free(inData->signal);
	g_free(inData);
}

/* Free an animation entry called by dispose function */
static void _xfce_overview_workspaces_actor_animation_entry_free_dispose_callback(gpointer inData, gpointer inUserData)
{
	_xfce_overview_workspaces_actor_animation_entry_free(inData);
}

/* Invalidate all stylable children recursively beginning at given actor */
static void _xfce_overview_workspaces_actor_invalidate_recursive(ClutterActor *inActor)
{
	ClutterActor			*child;
	ClutterActorIter		actorIter;

	g_return_if_fail(CLUTTER_IS_ACTOR(inActor));

	/* If actor is stylable invalidate it to get its style recomputed */
	if(XFCE_OVERVIEW_WORKSPACES_IS_STYLABLE(inActor))
	{
		xfce_overview_workspaces_stylable_invalidate(XFCE_OVERVIEW_WORKSPACES_STYLABLE(inActor));
	}

	/* Recompute styles for all children recursively */
	clutter_actor_iter_init(&actorIter, inActor);
	while(clutter_actor_iter_next(&actorIter, &child))
	{
		/* Call ourselve recursive with child as top-level actor.
		 * We return immediately if it has no children but invalidate child
		 * before. If it has children it will first invalidated and will be
		 * iterated over its children then. In both cases the child will
		 * be invalidated.
		 */
		_xfce_overview_workspaces_actor_invalidate_recursive(child);
	}
}

/* Get parameter specification of stylable properties and add them to hashtable.
 * If requested do it recursively over all parent classes.
 */
static void _xfce_overview_workspaces_actor_hashtable_get_all_stylable_param_specs(GHashTable *ioHashtable, GObjectClass *inClass, gboolean inRecursive)
{
	GList						*paramSpecs, *entry;
	GObjectClass				*parentClass;

	/* Get list of all parameter specification registered for class */
	paramSpecs=g_param_spec_pool_list_owned(_xfce_overview_workspaces_actor_stylable_properties_pool, G_OBJECT_CLASS_TYPE(inClass));
	for(entry=paramSpecs; entry; entry=g_list_next(entry))
	{
		GParamSpec				*paramSpec=G_PARAM_SPEC(entry->data);

		/* Only add parameter specification which aren't already in hashtable */
		if(paramSpec &&
			!g_hash_table_lookup_extended(ioHashtable, g_param_spec_get_name(paramSpec), NULL, NULL))
		{
			g_hash_table_insert(ioHashtable,
									g_strdup(g_param_spec_get_name(paramSpec)),
									g_param_spec_ref(paramSpec));
		}
	}
	g_list_free(paramSpecs);

	/* Call us recursive for parent class if it exists and requested */
	parentClass=g_type_class_peek_parent(inClass);
	if(inRecursive && parentClass) _xfce_overview_workspaces_actor_hashtable_get_all_stylable_param_specs(ioHashtable, parentClass, inRecursive);
}

/* Remove entries from hashtable whose key is a duplicate
 * in other hashtable (given in user data)
 */
static gboolean _xfce_overview_workspaces_actor_hashtable_is_duplicate_key(gpointer inKey,
																gpointer inValue,
																gpointer inUserData)
{
	GHashTable			*otherHashtable=(GHashTable*)inUserData;

	return(g_hash_table_lookup_extended(otherHashtable, inKey, NULL, NULL));
}

/* 'created' animation has completed */
static void _xfce_overview_workspaces_actor_first_time_created_animation_done(XfceOverviewWorkspacesAnimation *inAnimation,
																	gpointer inUserData)
{
	XfceOverviewWorkspacesActor				*self;
	XfceOverviewWorkspacesActorPrivate			*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_ANIMATION(inAnimation));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_ACTOR(inUserData));

	self=XFCE_OVERVIEW_WORKSPACES_ACTOR(inUserData);
	priv=self->priv;

	/* Mark completed first-time animation as removed */
	priv->firstTimeMappedAnimation=NULL;

	/* Allow allocation animation again */
	priv->disallowAllocationAnimation=FALSE;
}

/* Actor was mapped or unmapped */
static void _xfce_overview_workspaces_actor_on_mapped_changed(GObject *inObject,
													GParamSpec *inSpec,
													gpointer inUserData)
{
	XfceOverviewWorkspacesActor					*self;
	XfceOverviewWorkspacesActorPrivate				*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_ACTOR(inObject));

	self=XFCE_OVERVIEW_WORKSPACES_ACTOR(inObject);
	priv=self->priv;

	/* If actor was mapped, invalidate styling and check for first-time animation */
	if(clutter_actor_is_mapped(CLUTTER_ACTOR(self)))
	{
		/* Invalide styling to get it recomputed if actor was mapped */
		xfce_overview_workspaces_stylable_invalidate(XFCE_OVERVIEW_WORKSPACES_STYLABLE(self));

		/* If actor was mapped for the first time then check if an animation
		 * should be created and run.
		 */
		if(!priv->firstTimeMapped)
		{
			XfceOverviewWorkspacesAnimationValue	**initials;
			XfceOverviewWorkspacesAnimationValue	**finals;
			ClutterActorBox				finalBox={ 0, };

			g_assert(!priv->firstTimeMappedAnimation);

			/* Set flag that first-time visible happened at this actor */
			priv->firstTimeMapped=TRUE;

			/* If an animation for "created" signal exists, then set up animation */
			if(xfce_overview_workspaces_animation_has_animation(self, "created"))
			{
				/* Set up default initial values for animation */
				initials=xfce_overview_workspaces_animation_defaults_new(4,
															"x", G_TYPE_FLOAT, 0.0f,
															"y", G_TYPE_FLOAT, 0.0f,
															"width", G_TYPE_FLOAT, 0.0f,
															"height", G_TYPE_FLOAT, 0.0f);

				/* Set up default final values for animation */
				xfce_overview_workspaces_actor_get_allocation_box(self, &finalBox);
				finals=xfce_overview_workspaces_animation_defaults_new(4,
															"x", G_TYPE_FLOAT, finalBox.x1,
															"y", G_TYPE_FLOAT, finalBox.y1,
															"width", G_TYPE_FLOAT, clutter_actor_box_get_width(&finalBox),
															"height", G_TYPE_FLOAT, clutter_actor_box_get_height(&finalBox));

				/* Lookup animation for create signal and set up values for allocation */
				priv->firstTimeMappedAnimation=xfce_overview_workspaces_animation_new_with_values(self, "created", 0, initials, finals);

				/* Free default initial and final values */
				xfce_overview_workspaces_animation_defaults_free(initials);
				xfce_overview_workspaces_animation_defaults_free(finals);

				/* Run animation if found */
				if(!priv->firstTimeMappedAnimation ||
					!xfce_overview_workspaces_animation_get_id(priv->firstTimeMappedAnimation) ||
					xfce_overview_workspaces_animation_is_empty(priv->firstTimeMappedAnimation))
				{
					/* Empty or invalid animation, so release allocated resources and return */
					if(priv->firstTimeMappedAnimation) g_object_unref(priv->firstTimeMappedAnimation);
					priv->firstTimeMappedAnimation=NULL;

					return;
				}

				/* Disallow allocation animation if not set in 'created' animation */
				priv->disallowAllocationAnimation=TRUE;

				/* Start animation */
				g_signal_connect_after(priv->firstTimeMappedAnimation, "animation-done", G_CALLBACK(_xfce_overview_workspaces_actor_first_time_created_animation_done), self);
				xfce_overview_workspaces_animation_run(priv->firstTimeMappedAnimation);
				XFCE_OVERVIEW_WORKSPACES_DEBUG(self, ANIMATION,
										"Found and starting animation '%s' for created signal at actor %s",
										xfce_overview_workspaces_animation_get_id(priv->firstTimeMappedAnimation),
										G_OBJECT_TYPE_NAME(self));
			}
		}
	}
}

/* Actor was (re)named */
static void _xfce_overview_workspaces_actor_on_name_changed(GObject *inObject,
												GParamSpec *inSpec,
												gpointer inUserData)
{
	XfceOverviewWorkspacesActor			*self;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_ACTOR(inObject));

	self=XFCE_OVERVIEW_WORKSPACES_ACTOR(inObject);

	/* Invalide styling to get it recomputed because its ID (from point
	 * of view of css) has changed. Also invalidate children as they
	 * might reference the old, invalid ID or the new, valid one.
	 */
	_xfce_overview_workspaces_actor_invalidate_recursive(CLUTTER_ACTOR(self));
}

/* Actor's reactive state changed */
static void _xfce_overview_workspaces_actor_on_reactive_changed(GObject *inObject,
													GParamSpec *inSpec,
													gpointer inUserData)
{
	XfceOverviewWorkspacesActor			*self;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_ACTOR(inObject));

	self=XFCE_OVERVIEW_WORKSPACES_ACTOR(inObject);

	/* Add pseudo-class ':insensitive' if actor is now not reactive
	 * and remove this pseudo-class if actor is now reactive.
	 */
	if(clutter_actor_get_reactive(CLUTTER_ACTOR(self)))
	{
		xfce_overview_workspaces_stylable_remove_pseudo_class(XFCE_OVERVIEW_WORKSPACES_STYLABLE(self), "insensitive");
	}
		else
		{
			xfce_overview_workspaces_stylable_add_pseudo_class(XFCE_OVERVIEW_WORKSPACES_STYLABLE(self), "insensitive");
		}

	/* Invalide styling to get it recomputed */
	_xfce_overview_workspaces_actor_invalidate_recursive(CLUTTER_ACTOR(self));
}

/* Update effects of actor with string of list of effect IDs */
static void _xfce_overview_workspaces_actor_update_effects(XfceOverviewWorkspacesActor *self, const gchar *inEffects)
{
	XfceOverviewWorkspacesActorPrivate		*priv;
	XfceOverviewWorkspacesTheme			*theme;
	XfceOverviewWorkspacesThemeEffects		*themeEffects;
	gchar						**effectIDs;
	gchar						**iter;
	gchar						*effectsList;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_ACTOR(self));

	priv=self->priv;
	effectIDs=NULL;
	effectsList=NULL;

	/* Get theme effect instance which is needed to create effect objects.
	 * Also take a reference on theme effect instance as it needs to be alive
	 * while iterating through list of effect IDs and creating these effects.
	 */
	theme=xfce_overview_workspaces_core_get_theme(NULL);

	themeEffects=xfce_overview_workspaces_theme_get_effects(theme);
	g_object_ref(themeEffects);

	/* Get array of effect ID to create */
	if(inEffects) effectIDs=xfce_overview_workspaces_split_string(inEffects, " \t\r\n");

	/* Remove all effects from actor */
	clutter_actor_clear_effects(CLUTTER_ACTOR(self));

	/* Create effects by their ID, add them to actor and
	 * build result string with new list of effect IDs
	 */
	iter=effectIDs;
	while(iter && *iter)
	{
		ClutterEffect			*effect;

		/* Create effect and if it was created successfully
		 * add it to actor and update final string with list
		 * of effect IDs.
		 */
		effect=xfce_overview_workspaces_theme_effects_create_effect(themeEffects, *iter);
		if(effect)
		{
			clutter_actor_add_effect(CLUTTER_ACTOR(self), effect);

			if(effectsList)
			{
				gchar			*tempEffectsList;

				tempEffectsList=g_strconcat(effectsList, " ", *iter, NULL);
				g_free(effectsList);
				effectsList=tempEffectsList;
			}
				else effectsList=g_strdup(*iter);
		}

		/* Continue with next ID */
		iter++;
	}

	/* Set new string with list of effects */
	if(priv->effects) g_free(priv->effects);
	priv->effects=g_strdup(effectsList);

	/* Release allocated resources */
	if(effectsList) g_free(effectsList);
	if(effectIDs) g_strfreev(effectIDs);
	g_object_unref(themeEffects);
}

/* Check if actor should be visible */
static gboolean _xfce_overview_workspaces_actor_can_be_visible(XfceOverviewWorkspacesActor *self)
{
	XfceOverviewWorkspacesActorPrivate		*priv;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_ACTOR(self), FALSE);

	priv=self->priv;

	/* If actor has requested to be visible and visibility is allowed,
	 * then return TRUE for actor being visible. Otherwise it should
	 * not visible but hidden, so return FALSE.
	 */
	if(priv->shouldVisible==TRUE && priv->visibility==TRUE) return(TRUE);
	return(FALSE);
}

/* IMPLEMENTATION: Interface XfceOverviewWorkspacesFocusable */

/* Check if actor can get focus */
static gboolean _xfce_overview_workspaces_actor_focusable_can_focus(XfceOverviewWorkspacesFocusable *inFocusable)
{
	XfceOverviewWorkspacesActor			*self;
	XfceOverviewWorkspacesActorPrivate		*priv;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_FOCUSABLE(inFocusable), FALSE);
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_ACTOR(inFocusable), FALSE);

	self=XFCE_OVERVIEW_WORKSPACES_ACTOR(inFocusable);
	priv=self->priv;

	/* This actor can only be focused if it is mapped, visible and reactive */
	if(priv->canFocus &&
		clutter_actor_is_mapped(CLUTTER_ACTOR(self)) &&
		clutter_actor_is_visible(CLUTTER_ACTOR(self)) &&
		clutter_actor_get_reactive(CLUTTER_ACTOR(self)))
	{
		return(TRUE);
	}

	/* If we get here this actor does not fulfill the requirements to get focus */
	return(FALSE);
}

/* Interface initialization
 * Set up default functions
 */
static void _xfce_overview_workspaces_actor_focusable_iface_init(XfceOverviewWorkspacesFocusableInterface *iface)
{
	iface->can_focus=_xfce_overview_workspaces_actor_focusable_can_focus;
}

/* IMPLEMENTATION: Interface XfceOverviewWorkspacesStylable */

/* Get stylable properties of actor */
static void _xfce_overview_workspaces_actor_stylable_get_stylable_properties(XfceOverviewWorkspacesStylable *inStylable,
																		GHashTable *ioStylableProperties)
{
	g_return_if_fail(CLUTTER_IS_ACTOR(inStylable));

	/* Set up hashtable of stylable properties for this instance */
	_xfce_overview_workspaces_actor_hashtable_get_all_stylable_param_specs(ioStylableProperties,
																G_OBJECT_CLASS(inStylable),
																TRUE);
}

/* Get stylable name of actor */
static const gchar* _xfce_overview_workspaces_actor_stylable_get_name(XfceOverviewWorkspacesStylable *inStylable)
{
	g_return_val_if_fail(CLUTTER_IS_ACTOR(inStylable), NULL);

	return(clutter_actor_get_name(CLUTTER_ACTOR(inStylable)));
}

/* Get stylable parent of actor */
static XfceOverviewWorkspacesStylable* _xfce_overview_workspaces_actor_stylable_get_parent(XfceOverviewWorkspacesStylable *inStylable)
{
	ClutterActor			*self;
	ClutterActor			*parent;

	g_return_val_if_fail(CLUTTER_IS_ACTOR(inStylable), NULL);

	self=CLUTTER_ACTOR(inStylable);

	/* Get parent and check if stylable. If not return NULL. */
	parent=clutter_actor_get_parent(self);
	if(!XFCE_OVERVIEW_WORKSPACES_IS_STYLABLE(parent)) return(NULL);

	/* Return stylable parent */
	return(XFCE_OVERVIEW_WORKSPACES_STYLABLE(parent));
}

/* Get/set style classes of actor */
static const gchar* _xfce_overview_workspaces_actor_stylable_get_classes(XfceOverviewWorkspacesStylable *inStylable)
{
	XfceOverviewWorkspacesActor			*self;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_ACTOR(inStylable), NULL);

	self=XFCE_OVERVIEW_WORKSPACES_ACTOR(inStylable);

	return(self->priv->styleClasses);
}

static void _xfce_overview_workspaces_actor_stylable_set_classes(XfceOverviewWorkspacesStylable *inStylable, const gchar *inStyleClasses)
{
	XfceOverviewWorkspacesActor			*self;
	XfceOverviewWorkspacesActorPrivate		*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_ACTOR(inStylable));

	self=XFCE_OVERVIEW_WORKSPACES_ACTOR(inStylable);
	priv=self->priv;

	/* Set value if changed */
	if(g_strcmp0(priv->styleClasses, inStyleClasses))
	{
		/* Set value */
		if(priv->styleClasses)
		{
			g_free(priv->styleClasses);
			priv->styleClasses=NULL;
		}

		if(inStyleClasses) priv->styleClasses=g_strdup(inStyleClasses);

		/* Invalidate style to get it restyled and redrawn. Also invalidate
		 * children as they might reference the old, invalid classes or
		 * the new, valid ones.
		 */
		_xfce_overview_workspaces_actor_invalidate_recursive(CLUTTER_ACTOR(self));

		/* Notify about property change */
		g_object_notify(G_OBJECT(self), "style-classes");
	}
}

/* Get/set style pseudo-classes of actor */
static const gchar* _xfce_overview_workspaces_actor_stylable_get_pseudo_classes(XfceOverviewWorkspacesStylable *inStylable)
{
	XfceOverviewWorkspacesActor			*self;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_ACTOR(inStylable), NULL);

	self=XFCE_OVERVIEW_WORKSPACES_ACTOR(inStylable);

	return(self->priv->stylePseudoClasses);
}

static void _xfce_overview_workspaces_actor_stylable_set_pseudo_classes(XfceOverviewWorkspacesStylable *inStylable, const gchar *inStylePseudoClasses)
{
	XfceOverviewWorkspacesActor			*self;
	XfceOverviewWorkspacesActorPrivate		*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_ACTOR(inStylable));

	self=XFCE_OVERVIEW_WORKSPACES_ACTOR(inStylable);
	priv=self->priv;

	/* Set value if changed */
	if(g_strcmp0(priv->stylePseudoClasses, inStylePseudoClasses))
	{
		/* Set value */
		if(priv->stylePseudoClasses)
		{
			g_free(priv->stylePseudoClasses);
			priv->stylePseudoClasses=NULL;
		}

		if(inStylePseudoClasses) priv->stylePseudoClasses=g_strdup(inStylePseudoClasses);

		/* Invalidate style to get it restyled and redrawn. Also invalidate
		 * children as they might reference the old, invalid pseudo-classes
		 * or the new, valid ones.
		 */
		_xfce_overview_workspaces_actor_invalidate_recursive(CLUTTER_ACTOR(self));

		/* Notify about property change */
		g_object_notify(G_OBJECT(self), "style-pseudo-classes");
	}
}

/* Invalidate style to recompute styles */
static void _xfce_overview_workspaces_actor_stylable_invalidate(XfceOverviewWorkspacesStylable *inStylable)
{
	XfceOverviewWorkspacesActor			*self;
	XfceOverviewWorkspacesActorPrivate		*priv;
	XfceOverviewWorkspacesActorClass		*klass;
	XfceOverviewWorkspacesTheme			*theme;
	XfceOverviewWorkspacesThemeCSS			*themeCSS;
	GHashTable					*possibleStyleSet;
	GParamSpec					*paramSpec;
	GHashTableIter				hashIter;
	GHashTable					*themeStyleSet;
	gchar						*styleName;
	XfceOverviewWorkspacesThemeCSSValue	*styleValue;
	gboolean					didChange;
#ifdef DEBUG
	gboolean					doDebug=FALSE;
#endif

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_ACTOR(inStylable));

	self=XFCE_OVERVIEW_WORKSPACES_ACTOR(inStylable);
	priv=self->priv;
	klass=XFCE_OVERVIEW_WORKSPACES_ACTOR_GET_CLASS(self);
	didChange=FALSE;

	/* Only recompute style for mapped actors or if revalidation was forced */
	if(!priv->forceStyleRevalidation && !clutter_actor_is_mapped(CLUTTER_ACTOR(self))) return;

	/* Get theme */
	theme=xfce_overview_workspaces_core_get_theme(NULL);

	/* Get theme CSS */
	themeCSS=xfce_overview_workspaces_theme_get_css(theme);

	/* First get list of all stylable properties of this and parent classes.
	 * It is used to determine if key in theme style sets are valid.
	 */
	possibleStyleSet=xfce_overview_workspaces_actor_get_stylable_properties_full(klass);

#ifdef DEBUG
	if(doDebug)
	{
		gint					i=0;
		gchar					*defaultsKey;
		GValue					defaultsVal=G_VALUE_INIT;
		gchar					*defaultsValStr;
		GParamSpec				*realParamSpec;

		XFCE_OVERVIEW_WORKSPACES_DEBUG(self, STYLE,
							"Got param specs for %p (%s) with class '%s' and pseudo-class '%s'",
							self,
							G_OBJECT_TYPE_NAME(self),
							priv->styleClasses,
							priv->stylePseudoClasses);

		g_hash_table_iter_init(&hashIter, possibleStyleSet);
		while(g_hash_table_iter_next(&hashIter, (gpointer*)&defaultsKey, (gpointer*)&paramSpec))
		{
			realParamSpec=(GParamSpec*)g_param_spec_get_qdata(paramSpec, XFCE_OVERVIEW_WORKSPACES_ACTOR_PARAM_SPEC_REF);

			g_value_init(&defaultsVal, G_PARAM_SPEC_VALUE_TYPE(realParamSpec));
			g_param_value_set_default(realParamSpec, &defaultsVal);

			defaultsValStr=g_strdup_value_contents(&defaultsVal);
			XFCE_OVERVIEW_WORKSPACES_DEBUG(self, STYLE,
								"%d: param spec [%s] %s=%s\n",
								++i,
								G_OBJECT_CLASS_NAME(klass),
								defaultsKey,
								defaultsValStr);
			g_free(defaultsValStr);

			g_value_unset(&defaultsVal);
		}

		XFCE_OVERVIEW_WORKSPACES_DEBUG(self, STYLE, "End of param specs");
	}
#endif

	/* Get style information from theme */
	themeStyleSet=xfce_overview_workspaces_theme_css_get_properties(themeCSS, XFCE_OVERVIEW_WORKSPACES_STYLABLE(self));

#ifdef DEBUG
	if(doDebug)
	{
		gint					i=0;

		XFCE_OVERVIEW_WORKSPACES_DEBUG(self, STYLE,
							"Got styles from theme for %p (%s) with class '%s' and pseudo-class '%s'",
							self,
							G_OBJECT_TYPE_NAME(self),
							priv->styleClasses,
							priv->stylePseudoClasses);

		g_hash_table_iter_init(&hashIter, themeStyleSet);
		while(g_hash_table_iter_next(&hashIter, (gpointer*)&styleName, (gpointer*)&styleValue))
		{
			XFCE_OVERVIEW_WORKSPACES_DEBUG(self, STYLE,
								"%d: [%s] %s=%s\n",
								++i,
								styleValue->source,
								(gchar*)styleName,
								styleValue->value);
		}

		XFCE_OVERVIEW_WORKSPACES_DEBUG(self, STYLE, "End of styles from theme");
	}
#endif

	/* The 'property-changed' notification will be freezed and thawed
	 * (fired at once) after all stylable properties of this instance are set.
	 */
	g_object_freeze_notify(G_OBJECT(self));

	/* Iterate through style information retrieved from theme and
	 * set the corresponding property in object instance if key
	 * is valid.
	 */
	g_hash_table_iter_init(&hashIter, themeStyleSet);
	while(g_hash_table_iter_next(&hashIter, (gpointer*)&styleName, (gpointer*)&styleValue))
	{
		GValue					cssValue=G_VALUE_INIT;
		GValue					propertyValue=G_VALUE_INIT;
		GParamSpec				*realParamSpec;

		/* Check if key is a valid object property name */
		if(!g_hash_table_lookup_extended(possibleStyleSet, styleName, NULL, (gpointer*)&paramSpec)) continue;

		/* Get original referenced parameter specification. It does not need
		 * to be referenced while converting because it is valid as this
		 * value is stored in hashtable.
		 */
		realParamSpec=(GParamSpec*)g_param_spec_get_qdata(paramSpec, XFCE_OVERVIEW_WORKSPACES_ACTOR_PARAM_SPEC_REF);

		/* Convert style value to type of object property and set value
		 * if conversion was successful. Otherwise do nothing.
		 */
		g_value_init(&cssValue, G_TYPE_STRING);
		g_value_set_string(&cssValue, styleValue->value);

		g_value_init(&propertyValue, G_PARAM_SPEC_VALUE_TYPE(realParamSpec));

		if(g_param_value_convert(realParamSpec, &cssValue, &propertyValue, FALSE))
		{
			g_object_set_property(G_OBJECT(self), styleName, &propertyValue);
			didChange=TRUE;
#ifdef DEBUG
			if(doDebug)
			{
				gchar					*valstr;

				valstr=g_strdup_value_contents(&propertyValue);
				XFCE_OVERVIEW_WORKSPACES_DEBUG(self, STYLE,
									"Setting theme value of style property [%s] %s=%s\n",
									G_OBJECT_CLASS_NAME(klass),
									styleName,
									valstr);
				g_free(valstr);
			}
#endif
		}
			else
			{
				g_warning("Could not transform CSS string value for property '%s' to type %s of class %s",
							styleName, g_type_name(G_PARAM_SPEC_VALUE_TYPE(realParamSpec)), G_OBJECT_CLASS_NAME(klass));
			}

		/* Release allocated resources */
		g_value_unset(&propertyValue);
		g_value_unset(&cssValue);
	}

	/* Now remove all duplicate keys in set of properties changed we set the last
	 * time. The remaining keys determine the properties which were set the last
	 * time but not this time and should be restored to their default values.
	 */
	if(priv->lastThemeStyleSet)
	{
		/* Remove duplicate keys from set of last changed properties */
		g_hash_table_foreach_remove(priv->lastThemeStyleSet, (GHRFunc)_xfce_overview_workspaces_actor_hashtable_is_duplicate_key, themeStyleSet);

		/* Iterate through remaining key and restore corresponding object properties
		 * to their default values.
		 */
		g_hash_table_iter_init(&hashIter, priv->lastThemeStyleSet);
		while(g_hash_table_iter_next(&hashIter, (gpointer*)&styleName, (gpointer*)&paramSpec))
		{
			GValue				propertyValue=G_VALUE_INIT;
			GParamSpec			*realParamSpec;

			/* Check if key is a valid object property name */
			if(!g_hash_table_lookup_extended(possibleStyleSet, styleName, NULL, (gpointer*)&paramSpec)) continue;

			/* Get original referenced parameter specification. It does not need
			 * to be referenced while converting because it is valid as this
			 * value is stored in hashtable.
			 */
			realParamSpec=(GParamSpec*)g_param_spec_get_qdata(paramSpec, XFCE_OVERVIEW_WORKSPACES_ACTOR_PARAM_SPEC_REF);

			/* Initialize property value to its type and default value */
			g_value_init(&propertyValue, G_PARAM_SPEC_VALUE_TYPE(realParamSpec));
			g_param_value_set_default(realParamSpec, &propertyValue);

			/* Set value at object property */
			g_object_set_property(G_OBJECT(self), styleName, &propertyValue);
			didChange=TRUE;
#ifdef DEBUG
			if(doDebug)
			{
				gchar					*valstr;

				valstr=g_strdup_value_contents(&propertyValue);
				XFCE_OVERVIEW_WORKSPACES_DEBUG(self, STYLE,
									"Restoring default value of style property [%s] %s=%s\n",
									G_OBJECT_CLASS_NAME(klass),
									styleName,
									valstr);
				g_free(valstr);
			}
#endif

			/* Release allocated resources */
			g_value_unset(&propertyValue);
		}

		/* Release resources of set of last changed properties as we do not need
		 * it anymore.
		 */
		g_hash_table_destroy(priv->lastThemeStyleSet);
		priv->lastThemeStyleSet=NULL;
	}

	/* Remember this set of changed properties for next time to determine properties
	 * which need to be restored to their default value.
	 */
	priv->lastThemeStyleSet=themeStyleSet;

	/* Release allocated resources */
	g_hash_table_destroy(possibleStyleSet);

	/* Force a redraw if any change was made at this actor */
	if(didChange) clutter_actor_queue_redraw(CLUTTER_ACTOR(self));

	/* Reset force style revalidation flag because it's done now */
	priv->forceStyleRevalidation=FALSE;

	/* All stylable properties are set now. So thaw 'property-changed'
	 * notification now and fire all notifications at once.
	 */
	g_object_thaw_notify(G_OBJECT(self));
}

/* Animation has completed, so remove from list */
static void _xfce_overview_workspaces_actor_animation_done(XfceOverviewWorkspacesAnimation *inAnimation,
												gpointer inUserData)
{
	XfceOverviewWorkspacesActor				*self;
	XfceOverviewWorkspacesActorPrivate			*priv;
	GSList							*iter;
	XfceOverviewWorkspacesActorAnimationEntry	*data;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_ANIMATION(inAnimation));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_ACTOR(inUserData));

	self=XFCE_OVERVIEW_WORKSPACES_ACTOR(inUserData);
	priv=self->priv;

	/* Lookup animation done in list of animation and remove animation entry */
	for(iter=priv->animations; iter; )
	{
		GSList						*currentIter;

		/* As we might remove an entry from list, remember current iterator
		 * and move real iterator to next entry now.
		 */
		currentIter=iter;
		iter=g_slist_next(iter);

		/* Get animation entry at iterator */
		data=(XfceOverviewWorkspacesActorAnimationEntry*)currentIter->data;
		if(!data) continue;

		/* Check if animation entry matches the animation done */
		if(data->animation==inAnimation)
		{
			XFCE_OVERVIEW_WORKSPACES_DEBUG(self, ANIMATION,
									"Removing stopped animation '%s'",
									xfce_overview_workspaces_animation_get_id(data->animation));

			/* Remove entry from list, but set pointer to animation in entry
			 * to NULL to avoid unreffing an already disposed or finalized
			 * object instance.
			 */
			priv->animations=g_slist_remove_link(priv->animations, currentIter);

			if(!data->inDestruction) data->animation=NULL;
			_xfce_overview_workspaces_actor_animation_entry_free(data);
			g_slist_free_1(currentIter);
		}
	}
}

/* Remove animations for signal and (pseudo-)class */
static void _xfce_overview_workspaces_actor_remove_animation(XfceOverviewWorkspacesActor *self,
												const gchar *inAnimationSignal)
{
	XfceOverviewWorkspacesActorPrivate			*priv;
	GSList							*iter;
	XfceOverviewWorkspacesActorAnimationEntry	*data;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_ACTOR(self));
	g_return_if_fail(inAnimationSignal && *inAnimationSignal);

	priv=self->priv;

	/* Iterate through list of animation and lookup animation for signal. If an
	 * animation is found, stop it. It will be remove from list of animations
	 * automatically.
	 */
	for(iter=priv->animations; iter; iter=g_slist_next(iter))
	{
		/* Get animation entry at iterator */
		data=(XfceOverviewWorkspacesActorAnimationEntry*)iter->data;
		if(!data) continue;

		/* Check if animation entry matches the signal to lookup */
		if(g_strcmp0(data->signal, inAnimationSignal)==0)
		{
			XFCE_OVERVIEW_WORKSPACES_DEBUG(self, ANIMATION,
									"Stopping and removing animation '%s' for signal '%s'",
									xfce_overview_workspaces_animation_get_id(data->animation),
									inAnimationSignal);

			/* Stop animation by unreffing object instance which calls the
			 * done callback _xfce_overview_workspaces_actor_animation_done() of animation.
			 */
			g_object_unref(data->animation);
		}
	}
}

/* Lookup animations for signal and (pseudo-)class and run animation at actor */
static XfceOverviewWorkspacesAnimation* _xfce_overview_workspaces_actor_add_animation(XfceOverviewWorkspacesActor *self,
																const gchar *inAnimationSignal)
{
	XfceOverviewWorkspacesActorPrivate			*priv;
	XfceOverviewWorkspacesAnimation			*animation;
	XfceOverviewWorkspacesActorAnimationEntry	*data;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_ACTOR(self), NULL);
	g_return_val_if_fail(inAnimationSignal && *inAnimationSignal, NULL);

	priv=self->priv;

	/* Do not lookup and add animations if actor is in destruction now */
	if(priv->inDestruction) return(NULL);

	/* Lookup animation for signal-(pseudo-)class combination and if any found
	 * (i.e. has an ID) add it to list of animations of actor and run it.
	 */
	animation=xfce_overview_workspaces_animation_new(XFCE_OVERVIEW_WORKSPACES_ACTOR(self), inAnimationSignal, 0);
	if(!animation ||
		xfce_overview_workspaces_animation_is_empty(animation))
	{
		/* Release animation and return NULL */
		if(animation) g_object_unref(animation);
		return(NULL);
	}

	/* Check for duplicate animation */
	if(clutter_actor_get_transition(CLUTTER_ACTOR(self), xfce_overview_workspaces_animation_get_id(animation)))
	{
		XFCE_OVERVIEW_WORKSPACES_DEBUG(self, ANIMATION,
								"Duplicate animation found for signal '%s'",
								inAnimationSignal);

		/* Release animation and return NULL */
		g_object_unref(animation);
		return(NULL);
	}

	/* Create animation entry data and add to list of animations */
	data=g_new0(XfceOverviewWorkspacesActorAnimationEntry, 1);
	if(!data)
	{
		g_critical("Cannot allocate memory for animation entry for animation '%s' with signal '%s'",
					xfce_overview_workspaces_animation_get_id(animation),
					inAnimationSignal);

		/* Release animation and return NULL */
		g_object_unref(animation);
		return(NULL);
	}

	data->signal=g_strdup(inAnimationSignal);
	data->animation=animation;

	priv->animations=g_slist_prepend(priv->animations, data);

	/* Start animation */
	g_signal_connect_after(animation, "animation-done", G_CALLBACK(_xfce_overview_workspaces_actor_animation_done), self);
	xfce_overview_workspaces_animation_run(animation);
	XFCE_OVERVIEW_WORKSPACES_DEBUG(self, ANIMATION,
							"Found and starting animation '%s' for signal '%s'",
							xfce_overview_workspaces_animation_get_id(animation),
							inAnimationSignal);

	return(animation);
}

/* Signal handler for "class-added" signal of stylable interface */
static void _xfce_overview_workspaces_actor_stylable_class_added(XfceOverviewWorkspacesStylable *inStylable, const gchar *inClass)
{
	gchar			*animationSignal;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_ACTOR(inStylable));

	/* Remove any animation that was added when this class was removed */
	animationSignal=g_strdup_printf("class-removed:%s", inClass);
	_xfce_overview_workspaces_actor_remove_animation(XFCE_OVERVIEW_WORKSPACES_ACTOR(inStylable), animationSignal);
	g_free(animationSignal);

	/* Create animation for this class added */
	animationSignal=g_strdup_printf("class-added:%s", inClass);
	_xfce_overview_workspaces_actor_add_animation(XFCE_OVERVIEW_WORKSPACES_ACTOR(inStylable), animationSignal);
	g_free(animationSignal);
}

/* Signal handler for "class-removed" signal of stylable interface */
static void _xfce_overview_workspaces_actor_stylable_class_removed(XfceOverviewWorkspacesStylable *inStylable, const gchar *inClass)
{
	gchar			*animationSignal;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_ACTOR(inStylable));

	/* Remove any animation that was added when this class was added */
	animationSignal=g_strdup_printf("class-added:%s", inClass);
	_xfce_overview_workspaces_actor_remove_animation(XFCE_OVERVIEW_WORKSPACES_ACTOR(inStylable), animationSignal);
	g_free(animationSignal);

	/* Create animation for this class removed */
	animationSignal=g_strdup_printf("class-removed:%s", inClass);
	_xfce_overview_workspaces_actor_add_animation(XFCE_OVERVIEW_WORKSPACES_ACTOR(inStylable), animationSignal);
	g_free(animationSignal);
}

/* Signal handler for "pseudo-class-added" signal of stylable interface */
static void _xfce_overview_workspaces_actor_stylable_pseudo_class_added(XfceOverviewWorkspacesStylable *inStylable, const gchar *inClass)
{
	gchar			*animationSignal;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_ACTOR(inStylable));

	/* Remove any animation that was added when this pseudo-class was removed */
	animationSignal=g_strdup_printf("pseudo-class-removed:%s", inClass);
	_xfce_overview_workspaces_actor_remove_animation(XFCE_OVERVIEW_WORKSPACES_ACTOR(inStylable), animationSignal);
	g_free(animationSignal);

	/* Create animation for this pseudo-class added */
	animationSignal=g_strdup_printf("pseudo-class-added:%s", inClass);
	_xfce_overview_workspaces_actor_add_animation(XFCE_OVERVIEW_WORKSPACES_ACTOR(inStylable), animationSignal);
	g_free(animationSignal);
}

/* Signal handler for "pseudo-class-removed" signal of stylable interface */
static void _xfce_overview_workspaces_actor_stylable_pseudo_class_removed(XfceOverviewWorkspacesStylable *inStylable, const gchar *inClass)
{
	gchar			*animationSignal;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_ACTOR(inStylable));

	/* Remove any animation that was added when this pseudo-class was added */
	animationSignal=g_strdup_printf("pseudo-class-added:%s", inClass);
	_xfce_overview_workspaces_actor_remove_animation(XFCE_OVERVIEW_WORKSPACES_ACTOR(inStylable), animationSignal);
	g_free(animationSignal);

	/* Create animation for this pseudo-class removed */
	animationSignal=g_strdup_printf("pseudo-class-removed:%s", inClass);
	_xfce_overview_workspaces_actor_add_animation(XFCE_OVERVIEW_WORKSPACES_ACTOR(inStylable), animationSignal);
	g_free(animationSignal);
}

/* Interface initialization
 * Set up default functions
 */
static void _xfce_overview_workspaces_actor_stylable_iface_init(XfceOverviewWorkspacesStylableInterface *iface)
{
	iface->get_stylable_properties=_xfce_overview_workspaces_actor_stylable_get_stylable_properties;
	iface->get_name=_xfce_overview_workspaces_actor_stylable_get_name;
	iface->get_parent=_xfce_overview_workspaces_actor_stylable_get_parent;
	iface->get_classes=_xfce_overview_workspaces_actor_stylable_get_classes;
	iface->set_classes=_xfce_overview_workspaces_actor_stylable_set_classes;
	iface->class_added=_xfce_overview_workspaces_actor_stylable_class_added;
	iface->class_removed=_xfce_overview_workspaces_actor_stylable_class_removed;
	iface->get_pseudo_classes=_xfce_overview_workspaces_actor_stylable_get_pseudo_classes;
	iface->set_pseudo_classes=_xfce_overview_workspaces_actor_stylable_set_pseudo_classes;
	iface->pseudo_class_added=_xfce_overview_workspaces_actor_stylable_pseudo_class_added;
	iface->pseudo_class_removed=_xfce_overview_workspaces_actor_stylable_pseudo_class_removed;
	iface->invalidate=_xfce_overview_workspaces_actor_stylable_invalidate;
}

/* IMPLEMENTATION: ClutterActor */

/* Allocation animation ended */
static void _xfce_overview_workspaces_actor_on_allocation_animation_done(XfceOverviewWorkspacesAnimation *inAnimation,
															gpointer inUserData)
{
	XfceOverviewWorkspacesActor				*self;
	XfceOverviewWorkspacesActorPrivate			*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_ANIMATION(inAnimation));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_ACTOR(inUserData));

	self=XFCE_OVERVIEW_WORKSPACES_ACTOR(inUserData);
	priv=self->priv;

	/* Release allocated resources */
	priv->allocationAnimation=NULL;
}

/* Actor's allocation changed */
static void _xfce_overview_workspaces_actor_on_allocation_changed(ClutterActor *inActor,
														const ClutterActorBox *inAllocationBox,
														ClutterAllocationFlags inFlags,
														gpointer inUserData)
{
	XfceOverviewWorkspacesActor				*self;
	XfceOverviewWorkspacesActorPrivate			*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_ACTOR(inActor));
	g_return_if_fail(inAllocationBox);

	self=XFCE_OVERVIEW_WORKSPACES_ACTOR(inActor);
	priv=self->priv;

	/* Track allocation changes */
	if(G_LIKELY(priv->allocationTrackBox))
	{
		priv->allocationTrackBox->x1=inAllocationBox->x1;
		priv->allocationTrackBox->x2=inAllocationBox->x2;
		priv->allocationTrackBox->y1=inAllocationBox->y1;
		priv->allocationTrackBox->y2=inAllocationBox->y2;
	}
		else
		{
			priv->allocationTrackBox=clutter_actor_box_copy(inAllocationBox);
		}

		/* Check if allocation animation was requested explicitly */
	if(priv->doAllocationAnimation &&
		!priv->disallowAllocationAnimation)
	{
		XfceOverviewWorkspacesAnimation		*animation;
		XfceOverviewWorkspacesAnimationValue	**initials;
		XfceOverviewWorkspacesAnimationValue	**finals;

		/* Stop currently running animation if any */
		if(priv->allocationAnimation)
		{
			/* Stop animation */
			g_object_unref(priv->allocationAnimation);
		}

		/* Set up default initial values for animation */
		g_assert(priv->allocationInitialBox!=NULL);
		initials=xfce_overview_workspaces_animation_defaults_new(4,
													"x", G_TYPE_FLOAT, priv->allocationInitialBox->x1,
													"y", G_TYPE_FLOAT, priv->allocationInitialBox->y1,
													"width", G_TYPE_FLOAT, clutter_actor_box_get_width(priv->allocationInitialBox),
													"height", G_TYPE_FLOAT, clutter_actor_box_get_height(priv->allocationInitialBox));

		/* Set up default final values for animation */
		finals=xfce_overview_workspaces_animation_defaults_new(4,
													"x", G_TYPE_FLOAT, inAllocationBox->x1,
													"y", G_TYPE_FLOAT, inAllocationBox->y1,
													"width", G_TYPE_FLOAT, clutter_actor_box_get_width(inAllocationBox),
													"height", G_TYPE_FLOAT, clutter_actor_box_get_height(inAllocationBox));

		/* Create and start animation */
		animation=xfce_overview_workspaces_animation_new_with_values(self,
														ALLOCATION_ANIMATION_SIGNAL,
														0,
														initials,
														finals);
		if(animation)
		{
			/* If animation is not empty, start it now */
			if(!xfce_overview_workspaces_animation_is_empty(animation))
			{
				/* Remember the allocation animation */
				priv->allocationAnimation=animation;

				/* Start animation */
				g_signal_connect_after(animation,
										"animation-done",
										G_CALLBACK(_xfce_overview_workspaces_actor_on_allocation_animation_done),
										inActor);
				xfce_overview_workspaces_animation_run(priv->allocationAnimation);

				/* Take an extra reference on animation to keep it alive
				 * when it is unreferenced later this function.
				 */
				g_object_ref(priv->allocationAnimation);
			}

			/* Unreference animation, so it will either be disposed
			 * because it was empty or it will survive because it
			 * was started and an extra reference was taken.
			 */
			g_object_unref(animation);
		}

		/* Free default initial and final values */
		xfce_overview_workspaces_animation_defaults_free(initials);
		xfce_overview_workspaces_animation_defaults_free(finals);
	}

	/* Unset flag indicating an allocation animation was requested,
	 * as it was handled now or prevented if disallowed.
	 */
	priv->doAllocationAnimation=FALSE;
}

/* Pointer left actor */
static gboolean _xfce_overview_workspaces_actor_leave_event(ClutterActor *inActor, ClutterCrossingEvent *inEvent)
{
	XfceOverviewWorkspacesActor		*self;
	ClutterActorClass		*parentClass;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_ACTOR(inActor), CLUTTER_EVENT_PROPAGATE);

	self=XFCE_OVERVIEW_WORKSPACES_ACTOR(inActor);

	/* Call parent's virtual function */
	parentClass=CLUTTER_ACTOR_CLASS(xfce_overview_workspaces_actor_parent_class);
	if(parentClass->leave_event)
	{
		parentClass->leave_event(inActor, inEvent);
	}

	/* Remove pseudo-class ":hover" because pointer left actor */
	xfce_overview_workspaces_stylable_remove_pseudo_class(XFCE_OVERVIEW_WORKSPACES_STYLABLE(self), "hover");

	return(CLUTTER_EVENT_PROPAGATE);
}

/* Pointer entered actor */
static gboolean _xfce_overview_workspaces_actor_enter_event(ClutterActor *inActor, ClutterCrossingEvent *inEvent)
{
	XfceOverviewWorkspacesActor		*self;
	ClutterActorClass		*parentClass;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_ACTOR(inActor), CLUTTER_EVENT_PROPAGATE);

	self=XFCE_OVERVIEW_WORKSPACES_ACTOR(inActor);

	/* Call parent's virtual function */
	parentClass=CLUTTER_ACTOR_CLASS(xfce_overview_workspaces_actor_parent_class);
	if(parentClass->enter_event)
	{
		parentClass->enter_event(inActor, inEvent);
	}

	/* Add pseudo-class ":hover" because pointer entered actor */
	xfce_overview_workspaces_stylable_add_pseudo_class(XFCE_OVERVIEW_WORKSPACES_STYLABLE(self), "hover");

	return(CLUTTER_EVENT_PROPAGATE);
}

/* Actor was (re)parented */
static void _xfce_overview_workspaces_actor_parent_set(ClutterActor *inActor, ClutterActor *inOldParent)
{
	XfceOverviewWorkspacesActor				*self;
	XfceOverviewWorkspacesActorPrivate			*priv;
	ClutterActorClass				*parentClass;
	ClutterActor					*parent;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_ACTOR(inActor));

	self=XFCE_OVERVIEW_WORKSPACES_ACTOR(inActor);
	priv=self->priv;

	/* Call parent's virtual function */
	parentClass=CLUTTER_ACTOR_CLASS(xfce_overview_workspaces_actor_parent_class);
	if(parentClass->parent_set)
	{
		parentClass->parent_set(inActor, inOldParent);
	}

	/* Get new parent of actor */
	parent=clutter_actor_get_parent(inActor);

	/* Check if it is a newly created actor which is parented for the first time.
	 * Then emit 'actor-created' signal on stage.
	 */
	if(priv->isFirstParent &&
		!inOldParent &&
		parent)
	{
		ClutterActor			*stage;

		/* Get stage where this actor belongs to and emit signal at stage */
		stage=clutter_actor_get_stage(inActor);
		if(XFCE_OVERVIEW_WORKSPACES_IS_STAGE(stage))
		{
			g_signal_emit_by_name(stage, "actor-created", inActor, NULL);
		}

		/* Set flag that a parent set and signal was emitted */
		priv->isFirstParent=FALSE;
	}

	/* Invalide styling to get it recomputed because its ID (from point
	 * of view of css) has changed. Also invalidate children as they might
	 * reference the old, invalid parent or the new, valid one.
	 */
	_xfce_overview_workspaces_actor_invalidate_recursive(CLUTTER_ACTOR(self));
}

/* Replace a running animation for old signal with the one for new signal */
static XfceOverviewWorkspacesAnimation* _xfce_overview_workspaces_actor_replace_animation(XfceOverviewWorkspacesActor *self,
																	const gchar *inOldSignal,
																	const gchar *inNewSignal)
{
	XfceOverviewWorkspacesActorPrivate				*priv;
	XfceOverviewWorkspacesAnimation				*oldAnimation;
	XfceOverviewWorkspacesAnimation				*newAnimation;
	GSList								*iter;
	XfceOverviewWorkspacesActorAnimationEntry		*data;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_ACTOR(self), NULL);
	g_return_val_if_fail(inOldSignal && *inOldSignal, NULL);
	g_return_val_if_fail(inNewSignal && *inNewSignal, NULL);

	priv=self->priv;
	oldAnimation=NULL;
	newAnimation=NULL;

	/* Iterate through list of animation and lookup animation for old signal */
	for(iter=priv->animations; iter; iter=g_slist_next(iter))
	{
		/* Get animation entry at iterator */
		data=(XfceOverviewWorkspacesActorAnimationEntry*)iter->data;
		if(!data) continue;

		/* Check if animation entry matches the signal to lookup */
		if(g_strcmp0(data->signal, inOldSignal)!=0) continue;

		/* Found animation, so stop further iterations */
		oldAnimation=data->animation;
	}

	/* Get animation for new signal to replace old one */
	newAnimation=_xfce_overview_workspaces_actor_add_animation(self, inNewSignal);

	/* If an animation for old signal, stop it */
	if(oldAnimation)
	{
		/* If no new animation will be started, ensure old one completes
		 * before it will be removed.
		 */
		if(!newAnimation ||
			!xfce_overview_workspaces_animation_get_id(newAnimation))
		{
			xfce_overview_workspaces_animation_ensure_complete(oldAnimation);
		}

		/* Stop old animation by unreffing object instance which calls the
		 * done callback _xfce_overview_workspaces_actor_animation_done() of animation.
		 */
		g_object_unref(oldAnimation);
	}

	/* Return new animation which replaced old one */
	return(newAnimation);
}

/* Actor will be shown */
static void _xfce_overview_workspaces_actor_show(ClutterActor *inActor)
{
	XfceOverviewWorkspacesActor			*self;
	XfceOverviewWorkspacesActorPrivate		*priv;
	ClutterActorClass			*parentClass;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_ACTOR(inActor));

	self=XFCE_OVERVIEW_WORKSPACES_ACTOR(inActor);
	priv=self->priv;

	/* Set flag that actor requests to be shown */
	priv->shouldVisible=TRUE;

	/* Check if actor should be hidden forcibly */
	if(!_xfce_overview_workspaces_actor_can_be_visible(self)) return;

	/* Call parent's virtual function */
	parentClass=CLUTTER_ACTOR_CLASS(xfce_overview_workspaces_actor_parent_class);
	if(parentClass->show)
	{
		parentClass->show(inActor);
	}

	/* If actor is visible now check if pointer is inside this actor
	 * then add pseudo-class ":hover" to it
	 */
	if(clutter_actor_has_pointer(inActor))
	{
		xfce_overview_workspaces_stylable_add_pseudo_class(XFCE_OVERVIEW_WORKSPACES_STYLABLE(self), "hover");
	}

	/* Stop any animation started for "hiding" actor which may be still running,
	 * and lookup the one should be run when actor gets visible */
	_xfce_overview_workspaces_actor_replace_animation(self, "hide", "show");
}

/* Actor will be hidden */
static void _xfce_overview_workspaces_actor_hide_on_animation_done(XfceOverviewWorkspacesActor *inActor, gpointer inUserData)
{
	ClutterActorClass		*parentClass;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_ACTOR(inActor));

	/* Call parent's virtual function to hide actor */
	parentClass=CLUTTER_ACTOR_CLASS(xfce_overview_workspaces_actor_parent_class);
	if(parentClass->hide)
	{
		parentClass->hide(CLUTTER_ACTOR(inActor));
	}
}

static void _xfce_overview_workspaces_actor_hide(ClutterActor *inActor)
{
	XfceOverviewWorkspacesActor			*self;
	XfceOverviewWorkspacesActorPrivate		*priv;
	XfceOverviewWorkspacesAnimation		*animation;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_ACTOR(inActor));

	self=XFCE_OVERVIEW_WORKSPACES_ACTOR(inActor);
	priv=self->priv;

	/* Set flag that actor requests to be hidden */
	priv->shouldVisible=FALSE;

	/* Actor is hidden now so remove pseudo-class ":hover" because pointer cannot
	 * be in an actor hidden.
	 */
	xfce_overview_workspaces_stylable_remove_pseudo_class(XFCE_OVERVIEW_WORKSPACES_STYLABLE(self), "hover");

	/* Stop any animation started for "showing" actor which may be still running,
	 * and lookup the one should be run when actor gets hidden.
	 */
	animation=_xfce_overview_workspaces_actor_replace_animation(self, "show", "hide");
	if(animation) g_signal_connect_swapped(animation, "animation-done", G_CALLBACK(_xfce_overview_workspaces_actor_hide_on_animation_done), self);
		else _xfce_overview_workspaces_actor_hide_on_animation_done(self, NULL);
}


/* IMPLEMENTATION: GObject */

/* Dispose this object */
static void _xfce_overview_workspaces_actor_dispose(GObject *inObject)
{
	XfceOverviewWorkspacesActor			*self=XFCE_OVERVIEW_WORKSPACES_ACTOR(inObject);
	XfceOverviewWorkspacesActorPrivate		*priv=self->priv;

	/* Set flag that actor will be destructed */
	priv->inDestruction=TRUE;

	/* Release allocated variables */
	if(priv->effects)
	{
		g_free(priv->effects);
		priv->effects=NULL;
	}

	if(priv->styleClasses)
	{
		g_free(priv->styleClasses);
		priv->styleClasses=NULL;
	}

	if(priv->stylePseudoClasses)
	{
		g_free(priv->stylePseudoClasses);
		priv->stylePseudoClasses=NULL;
	}

	if(priv->lastThemeStyleSet)
	{
		g_hash_table_destroy(priv->lastThemeStyleSet);
		priv->lastThemeStyleSet=NULL;
	}

	if(priv->firstTimeMappedAnimation)
	{
		g_object_unref(priv->firstTimeMappedAnimation);
		priv->firstTimeMappedAnimation=NULL;
	}

	if(priv->animations)
	{
		/* Iterate through list of animation and call destruction function
		 * _xfce_overview_workspaces_actor_animation_entry_free() for each entry in list but
		 * do not free the list itself with g_slist_free* function familiy as the
		 * connected signal handler _xfce_overview_workspaces_actor_animation_done() for
		 * 'animation-done' will actually remove the this entry from list resulting
		 * in an empty list with pointer NULL.
		 */
		g_slist_foreach(priv->animations, _xfce_overview_workspaces_actor_animation_entry_free_dispose_callback, NULL);
		priv->animations=NULL;
	}

	if(priv->allocationAnimation)
	{
		g_object_unref(priv->allocationAnimation);
		priv->allocationAnimation=NULL;
	}

	if(priv->allocationInitialBox)
	{
		clutter_actor_box_free(priv->allocationInitialBox);
		priv->allocationInitialBox=NULL;
	}

	if(priv->allocationTrackBox)
	{
		clutter_actor_box_free(priv->allocationTrackBox);
		priv->allocationTrackBox=NULL;
	}

	priv->doAllocationAnimation=FALSE;

	/* Call parent's class dispose method */
	G_OBJECT_CLASS(xfce_overview_workspaces_actor_parent_class)->dispose(inObject);
}

/* Set/get properties */
static void _xfce_overview_workspaces_actor_set_property(GObject *inObject,
											guint inPropID,
											const GValue *inValue,
											GParamSpec *inSpec)
{
	XfceOverviewWorkspacesActor			*self=XFCE_OVERVIEW_WORKSPACES_ACTOR(inObject);

	switch(inPropID)
	{
		case PROP_CAN_FOCUS:
			xfce_overview_workspaces_actor_set_can_focus(self, g_value_get_boolean(inValue));
			break;

		case PROP_EFFECTS:
			xfce_overview_workspaces_actor_set_effects(self, g_value_get_string(inValue));
			break;

		case PROP_VISIBILITY:
			xfce_overview_workspaces_actor_set_visibility(self, g_value_get_boolean(inValue));
			break;

		case PROP_STYLE_CLASSES:
			_xfce_overview_workspaces_actor_stylable_set_classes(XFCE_OVERVIEW_WORKSPACES_STYLABLE(self),
													g_value_get_string(inValue));
			break;

		case PROP_STYLE_PSEUDO_CLASSES:
			_xfce_overview_workspaces_actor_stylable_set_pseudo_classes(XFCE_OVERVIEW_WORKSPACES_STYLABLE(self),
															g_value_get_string(inValue));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(inObject, inPropID, inSpec);
			break;
	}
}

static void _xfce_overview_workspaces_actor_get_property(GObject *inObject,
											guint inPropID,
											GValue *outValue,
											GParamSpec *inSpec)
{
	XfceOverviewWorkspacesActor			*self=XFCE_OVERVIEW_WORKSPACES_ACTOR(inObject);
	XfceOverviewWorkspacesActorPrivate		*priv=self->priv;

	switch(inPropID)
	{
		case PROP_CAN_FOCUS:
			g_value_set_boolean(outValue, priv->canFocus);
			break;

		case PROP_EFFECTS:
			g_value_set_string(outValue, priv->effects);
			break;

		case PROP_VISIBILITY:
			g_value_set_boolean(outValue, priv->visibility);
			break;

		case PROP_STYLE_CLASSES:
			g_value_set_string(outValue, priv->styleClasses);
			break;

		case PROP_STYLE_PSEUDO_CLASSES:
			g_value_set_string(outValue, priv->stylePseudoClasses);
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(inObject, inPropID, inSpec);
			break;
	}
}

/* Access instance's private structure */
static inline gpointer xfce_overview_workspaces_actor_get_instance_private(XfceOverviewWorkspacesActor *self)
{
	return(G_STRUCT_MEMBER_P(self, xfce_overview_workspaces_actor_private_offset));
}
 
/* Class initialization
 * Override functions in parent classes and define properties
 * and signals
 */
void xfce_overview_workspaces_actor_class_init(XfceOverviewWorkspacesActorClass *klass)
{
	ClutterActorClass			*clutterActorClass=CLUTTER_ACTOR_CLASS(klass);
	GObjectClass				*gobjectClass=G_OBJECT_CLASS(klass);

	/* Get parent class */
	xfce_overview_workspaces_actor_parent_class=g_type_class_peek_parent(klass);

	/* Adjust offset to instance private structure */
	if(xfce_overview_workspaces_actor_private_offset!=0)
	{
		g_type_class_adjust_private_offset(klass, &xfce_overview_workspaces_actor_private_offset);
	}

	/* Override functions */
	gobjectClass->dispose=_xfce_overview_workspaces_actor_dispose;
	gobjectClass->set_property=_xfce_overview_workspaces_actor_set_property;
	gobjectClass->get_property=_xfce_overview_workspaces_actor_get_property;

	clutterActorClass->parent_set=_xfce_overview_workspaces_actor_parent_set;
	clutterActorClass->enter_event=_xfce_overview_workspaces_actor_enter_event;
	clutterActorClass->leave_event=_xfce_overview_workspaces_actor_leave_event;
	clutterActorClass->show=_xfce_overview_workspaces_actor_show;
	clutterActorClass->hide=_xfce_overview_workspaces_actor_hide;

	/* Create param-spec pool for themable properties */
	g_assert(_xfce_overview_workspaces_actor_stylable_properties_pool==NULL);
	_xfce_overview_workspaces_actor_stylable_properties_pool=g_param_spec_pool_new(FALSE);

	/* Define properties */
	XfceOverviewWorkspacesActorProperties[PROP_CAN_FOCUS]=
		g_param_spec_boolean("can-focus",
								"Can focus",
								"This flag indicates if this actor can be focused",
								FALSE,
								G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
	g_object_class_install_property(G_OBJECT_CLASS(klass), PROP_CAN_FOCUS, XfceOverviewWorkspacesActorProperties[PROP_CAN_FOCUS]);

	XfceOverviewWorkspacesActorProperties[PROP_EFFECTS]=
		g_param_spec_string("effects",
								"Effects",
								"List of space-separated strings with IDs of effects set at this actor",
								NULL,
								G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
	g_object_class_install_property(G_OBJECT_CLASS(klass), PROP_EFFECTS, XfceOverviewWorkspacesActorProperties[PROP_EFFECTS]);

	XfceOverviewWorkspacesActorProperties[PROP_VISIBILITY]=
		g_param_spec_boolean("visibility",
								"Visibility",
								"This flag determines if this actor can be visible or should be forcibly hidden",
								TRUE,
								G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
	g_object_class_install_property(G_OBJECT_CLASS(klass), PROP_VISIBILITY, XfceOverviewWorkspacesActorProperties[PROP_VISIBILITY]);

	g_object_class_override_property(gobjectClass, PROP_STYLE_CLASSES, "style-classes");
	g_object_class_override_property(gobjectClass, PROP_STYLE_PSEUDO_CLASSES, "style-pseudo-classes");

	/* Define stylable properties */
	xfce_overview_workspaces_actor_install_stylable_property_by_name(klass, "visibility");
	xfce_overview_workspaces_actor_install_stylable_property_by_name(klass, "effects");
	xfce_overview_workspaces_actor_install_stylable_property_by_name(klass, "x-expand");
	xfce_overview_workspaces_actor_install_stylable_property_by_name(klass, "y-expand");
	xfce_overview_workspaces_actor_install_stylable_property_by_name(klass, "x-align");
	xfce_overview_workspaces_actor_install_stylable_property_by_name(klass, "y-align");
	xfce_overview_workspaces_actor_install_stylable_property_by_name(klass, "margin-top");
	xfce_overview_workspaces_actor_install_stylable_property_by_name(klass, "margin-bottom");
	xfce_overview_workspaces_actor_install_stylable_property_by_name(klass, "margin-left");
	xfce_overview_workspaces_actor_install_stylable_property_by_name(klass, "margin-right");
}

/* Object initialization
 * Create private structure and set up default values
 */
void xfce_overview_workspaces_actor_init(XfceOverviewWorkspacesActor *self)
{
	XfceOverviewWorkspacesActorPrivate		*priv;

	priv=self->priv=xfce_overview_workspaces_actor_get_instance_private(self);

	/* Set up default values */
	priv->inDestruction=FALSE;
	priv->canFocus=FALSE;
	priv->effects=NULL;
	priv->styleClasses=NULL;
	priv->stylePseudoClasses=NULL;
	priv->lastThemeStyleSet=NULL;
	priv->isFirstParent=TRUE;
	priv->firstTimeMapped=FALSE;
	priv->firstTimeMappedAnimation=NULL;
	priv->animations=NULL;
	priv->disallowAllocationAnimation=FALSE;
	priv->doAllocationAnimation=FALSE;
	priv->allocationAnimation=NULL;
	priv->allocationInitialBox=NULL;
	priv->allocationTrackBox=NULL;
	priv->shouldVisible=TRUE;
	priv->visibility=TRUE;

	/* Connect signals */
	g_signal_connect(self, "notify::mapped", G_CALLBACK(_xfce_overview_workspaces_actor_on_mapped_changed), NULL);
	g_signal_connect(self, "notify::name", G_CALLBACK(_xfce_overview_workspaces_actor_on_name_changed), NULL);
	g_signal_connect(self, "notify::reactive", G_CALLBACK(_xfce_overview_workspaces_actor_on_reactive_changed), NULL);
	g_signal_connect(self, "allocation-changed", G_CALLBACK(_xfce_overview_workspaces_actor_on_allocation_changed), NULL);
}

/* IMPLEMENTATION: GType */

/* Base class finalization */
void xfce_overview_workspaces_actor_base_class_finalize(XfceOverviewWorkspacesActorClass *klass)
{
	GList				*paramSpecs, *entry;

	paramSpecs=g_param_spec_pool_list_owned(_xfce_overview_workspaces_actor_stylable_properties_pool, G_OBJECT_CLASS_TYPE(klass));
	for(entry=paramSpecs; entry; entry=g_list_next(entry))
	{
		GParamSpec		*paramSpec=G_PARAM_SPEC(entry->data);

		if(paramSpec)
		{
			g_param_spec_pool_remove(_xfce_overview_workspaces_actor_stylable_properties_pool, paramSpec);

			XFCE_OVERVIEW_WORKSPACES_DEBUG(NULL, STYLE,
								"Unregistered stylable property named '%s' for class '%s'",
								g_param_spec_get_name(paramSpec), G_OBJECT_CLASS_NAME(klass));

			g_param_spec_unref(paramSpec);
		}
	}
	g_list_free(paramSpecs);
}

/* Register type to GObject system */
GType xfce_overview_workspaces_actor_get_type(void)
{
	static GType		actorType=0;

	if(G_UNLIKELY(actorType==0))
	{
		/* Class info */
		const GTypeInfo 		actorInfo=
		{
			sizeof(XfceOverviewWorkspacesActorClass),
			NULL,
			(GBaseFinalizeFunc)(void*)xfce_overview_workspaces_actor_base_class_finalize,
			(GClassInitFunc)(void*)xfce_overview_workspaces_actor_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof(XfceOverviewWorkspacesActor),
			0,    /* n_preallocs */
			(GInstanceInitFunc)(void*)xfce_overview_workspaces_actor_init,
			NULL, /* value_table */
		};

		/* Implemented interfaces info */
		const GInterfaceInfo	actorStylableInterfaceInfo=
		{
			(GInterfaceInitFunc)(void*)_xfce_overview_workspaces_actor_stylable_iface_init,
			NULL,
			NULL
		};

		const GInterfaceInfo	actorFocusableInterfaceInfo=
		{
			(GInterfaceInitFunc)(void*)_xfce_overview_workspaces_actor_focusable_iface_init,
			NULL,
			NULL
		};

		/* Register class */
		actorType=g_type_register_static(CLUTTER_TYPE_ACTOR,
											g_intern_static_string("XfceOverviewWorkspacesActor"),
											&actorInfo,
											0);

		/* Add private structure */
		xfce_overview_workspaces_actor_private_offset=g_type_add_instance_private(actorType, sizeof(XfceOverviewWorkspacesActorPrivate));

		/* Add implemented interfaces */
		g_type_add_interface_static(actorType,
									XFCE_OVERVIEW_WORKSPACES_TYPE_STYLABLE,
									&actorStylableInterfaceInfo);

		g_type_add_interface_static(actorType,
									XFCE_OVERVIEW_WORKSPACES_TYPE_FOCUSABLE,
									&actorFocusableInterfaceInfo);
	}

	return(actorType);
}

/* IMPLEMENTATION: Public API */

/* Create new actor */
ClutterActor* xfce_overview_workspaces_actor_new(void)
{
	return(CLUTTER_ACTOR(g_object_new(XFCE_OVERVIEW_WORKSPACES_TYPE_ACTOR, NULL)));
}

/* Get/set "can-focus" flag of actor to indicate
 * if this actor is focusable or not.
 */
gboolean xfce_overview_workspaces_actor_get_can_focus(XfceOverviewWorkspacesActor *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_ACTOR(self), FALSE);

	return(self->priv->canFocus);
}

void xfce_overview_workspaces_actor_set_can_focus(XfceOverviewWorkspacesActor *self, gboolean inCanFous)
{
	XfceOverviewWorkspacesActorPrivate		*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_ACTOR(self));

	priv=self->priv;

	/* Set value if changed */
	if(priv->canFocus!=inCanFous)
	{
		/* Set value */
		priv->canFocus=inCanFous;

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesActorProperties[PROP_CAN_FOCUS]);
	}
}

/* Get/set space-seperated list of IDs of effects used by this actor */
const gchar* xfce_overview_workspaces_actor_get_effects(XfceOverviewWorkspacesActor *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_ACTOR(self), NULL);

	return(self->priv->effects);
}

void xfce_overview_workspaces_actor_set_effects(XfceOverviewWorkspacesActor *self, const gchar *inEffects)
{
	XfceOverviewWorkspacesActorPrivate		*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_ACTOR(self));

	priv=self->priv;

	/* Set value if changed */
	if(g_strcmp0(priv->effects, inEffects)!=0)
	{
		/* Set value */
		_xfce_overview_workspaces_actor_update_effects(self, inEffects);

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesActorProperties[PROP_EFFECTS]);
	}
}

/* Get/set "visibility" flag of actor to determine
 * if this actor can be shown or should be hidden forcibly.
 */
gboolean xfce_overview_workspaces_actor_get_visibility(XfceOverviewWorkspacesActor *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_ACTOR(self), FALSE);

	return(self->priv->visibility);
}

void xfce_overview_workspaces_actor_set_visibility(XfceOverviewWorkspacesActor *self, gboolean inVisibility)
{
	XfceOverviewWorkspacesActorPrivate		*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_ACTOR(self));

	priv=self->priv;

	/* Set value if changed */
	if(priv->visibility!=inVisibility)
	{
		/* Set value */
		priv->visibility=inVisibility;

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesActorProperties[PROP_VISIBILITY]);

		/* Update actors visibility */
		if(!_xfce_overview_workspaces_actor_can_be_visible(self))
		{
			clutter_actor_hide(CLUTTER_ACTOR(self));
		}
			else
			{
				clutter_actor_show(CLUTTER_ACTOR(self));
			}
	}
}

/* Register stylable property of a class */
void xfce_overview_workspaces_actor_install_stylable_property(XfceOverviewWorkspacesActorClass *klass, GParamSpec *inParamSpec)
{
	GParamSpec		*stylableParamSpec;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_ACTOR_CLASS(klass));
	g_return_if_fail(G_IS_PARAM_SPEC(inParamSpec));
	g_return_if_fail(inParamSpec->flags & G_PARAM_WRITABLE);
	g_return_if_fail(!(inParamSpec->flags & G_PARAM_CONSTRUCT_ONLY));

	/* Check if param-spec is already registered */
	if(g_param_spec_pool_lookup(_xfce_overview_workspaces_actor_stylable_properties_pool, g_param_spec_get_name(inParamSpec), G_OBJECT_CLASS_TYPE(klass), FALSE))
	{
		g_warning("Class '%s' already contains a stylable property '%s'",
					G_OBJECT_CLASS_NAME(klass),
					g_param_spec_get_name(inParamSpec));
		return;
	}

	/* Add param-spec to pool of themable properties */
	stylableParamSpec=g_param_spec_internal(G_PARAM_SPEC_TYPE(inParamSpec),
												g_param_spec_get_name(inParamSpec),
												NULL,
												NULL,
												0);
	g_param_spec_set_qdata_full(stylableParamSpec,
									XFCE_OVERVIEW_WORKSPACES_ACTOR_PARAM_SPEC_REF,
									g_param_spec_ref(inParamSpec),
									(GDestroyNotify)g_param_spec_unref);
	g_param_spec_pool_insert(_xfce_overview_workspaces_actor_stylable_properties_pool, stylableParamSpec, G_OBJECT_CLASS_TYPE(klass));
	XFCE_OVERVIEW_WORKSPACES_DEBUG(NULL, STYLE,
						"Registered stylable property '%s' for class '%s'",
						g_param_spec_get_name(inParamSpec), G_OBJECT_CLASS_NAME(klass));
}

void xfce_overview_workspaces_actor_install_stylable_property_by_name(XfceOverviewWorkspacesActorClass *klass, const gchar *inParamName)
{
	GParamSpec		*paramSpec;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_ACTOR_CLASS(klass));
	g_return_if_fail(inParamName && inParamName[0]);

	/* Find parameter specification for property name and register it as stylable */
	paramSpec=g_object_class_find_property(G_OBJECT_CLASS(klass), inParamName);
	if(paramSpec) xfce_overview_workspaces_actor_install_stylable_property(klass, paramSpec);
		else
		{
			g_warning("Cannot register non-existent property '%s' of class '%s'",
						inParamName, G_OBJECT_CLASS_NAME(klass));
		}
}

/* Get hash-table with all stylable properties of this class or
 * recursively of this and all parent classes
 */
GHashTable* xfce_overview_workspaces_actor_get_stylable_properties(XfceOverviewWorkspacesActorClass *klass)
{
	GHashTable		*stylableProps;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_ACTOR_CLASS(klass), NULL);

	/* Create hashtable and insert stylable properties */
	stylableProps=g_hash_table_new_full(g_str_hash,
											g_str_equal,
											g_free,
											(GDestroyNotify)g_param_spec_unref);
	_xfce_overview_workspaces_actor_hashtable_get_all_stylable_param_specs(stylableProps, G_OBJECT_CLASS(klass), FALSE);

	return(stylableProps);
}

GHashTable* xfce_overview_workspaces_actor_get_stylable_properties_full(XfceOverviewWorkspacesActorClass *klass)
{
	GHashTable		*stylableProps;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_ACTOR_CLASS(klass), NULL);

	/* Create hashtable and insert stylable properties */
	stylableProps=g_hash_table_new_full(g_str_hash,
											g_str_equal,
											g_free,
											(GDestroyNotify)g_param_spec_unref);
	_xfce_overview_workspaces_actor_hashtable_get_all_stylable_param_specs(stylableProps, G_OBJECT_CLASS(klass), TRUE);

	return(stylableProps);
}

/* Force restyling actor by theme next time stylable invalidation
 * function of this actor is called
 */
void xfce_overview_workspaces_actor_invalidate(XfceOverviewWorkspacesActor *self)
{
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_ACTOR(self));

	self->priv->forceStyleRevalidation=TRUE;
}

/* Requests to start an animation at next allocation change
 * if theme defines an animation for move/resize.
 */
void xfce_overview_workspaces_actor_enable_allocation_animation_once(XfceOverviewWorkspacesActor *self)
{
	XfceOverviewWorkspacesActorPrivate		*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_ACTOR(self));

	priv=self->priv;

	/* Do nothing if this flag for requesting animation was already set */
	if(priv->doAllocationAnimation) return;

	/* Set flag to create and start animation at next allocation change */
	priv->doAllocationAnimation=TRUE;

	/* Use last tracked allocation as initial allocation box */
	if(priv->allocationInitialBox)
	{
		clutter_actor_box_free(priv->allocationInitialBox);
		priv->allocationInitialBox=NULL;
	}

	if(G_UNLIKELY(!priv->allocationTrackBox))
	{
		priv->allocationTrackBox=clutter_actor_box_new(0.0f, 0.0f, 0.0f, 0.0f);
	}

	priv->allocationInitialBox=clutter_actor_box_copy(priv->allocationTrackBox);
}

/* Calling clutter_actor_get_allocation_box() is dangerous with animationa
 * because it may modify the internal state of the position and size of
 * an actor used for initial values for position and size at the animation
 * to run. Therefore this safe function exists which returns the last allocation
 * set to this actor.
 */
void xfce_overview_workspaces_actor_get_allocation_box(XfceOverviewWorkspacesActor *self, ClutterActorBox *outAllocationBox)
{
	XfceOverviewWorkspacesActorPrivate		*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_ACTOR(self));

	priv=self->priv;

	/* Get current allocation box from actor the unsafe way if tracked allocation box
	 * must be initialized.
	 */
	if(G_UNLIKELY(!priv->allocationTrackBox))
	{
		priv->allocationTrackBox=clutter_actor_box_new(0.0f, 0.0f, 0.0f, 0.0f);
		clutter_actor_get_allocation_box(CLUTTER_ACTOR(self), priv->allocationTrackBox);
	}

	/* Copy tracked allocation box to result */
	if(G_LIKELY(outAllocationBox))
	{
		outAllocationBox->x1=priv->allocationTrackBox->x1;
		outAllocationBox->x2=priv->allocationTrackBox->x2;
		outAllocationBox->y1=priv->allocationTrackBox->y1;
		outAllocationBox->y2=priv->allocationTrackBox->y2;
	}
}

/* Destroys an actor but checks first if an animation should be played.
 * If an animation for this actor exists, it will be played and after it
 * has ended, it will be destroyed. If no animation exists, the actor will
 * be destroyed immediately.
 */
static void _xfce_overview_workspaces_actor_on_destroy_animation_done(XfceOverviewWorkspacesAnimation *inAnimation,
															gpointer inUserData)
{
	ClutterActor				*self;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_ANIMATION(inAnimation));
	g_return_if_fail(CLUTTER_IS_ACTOR(inUserData));

	self=CLUTTER_ACTOR(inUserData);

	/* Destroy animation has ended, so destroy actor now for real */
	clutter_actor_destroy(self);
}

gboolean xfce_overview_workspaces_actor_destroy(ClutterActor *self)
{
	XfceOverviewWorkspacesAnimation		*animation;
	gboolean					animationStarted;

	g_return_val_if_fail(CLUTTER_IS_ACTOR(self), FALSE);

	animation=NULL;
	animationStarted=FALSE;

	/* Check if an animation exists but only for actors derived from XfceOverviewWorkspacesActor */
	if(XFCE_OVERVIEW_WORKSPACES_IS_ACTOR(self))
	{
		animation=xfce_overview_workspaces_animation_new(XFCE_OVERVIEW_WORKSPACES_ACTOR(self), "destroy", 0);
	}

	if(animation &&
		!xfce_overview_workspaces_animation_is_empty(animation))
	{
		/* Connect signal to destroy actor when animation has ended */
		g_signal_connect_after(animation,
								"animation-done",
								G_CALLBACK(_xfce_overview_workspaces_actor_on_destroy_animation_done),
								self);

		/* Set CSS pseudo-class that this actor is going to be destroyed */
		xfce_overview_workspaces_stylable_add_pseudo_class(XFCE_OVERVIEW_WORKSPACES_STYLABLE(self), "destroying");

		/* Start destroy animation and set flag that animation was
		 * found and started.
		 */
		xfce_overview_workspaces_animation_run(animation);
		animationStarted=TRUE;

		/* Take extra reference to keep animation alive as it will be
		 * unreferenced when cleaning allocated resources.
		 */
		g_object_ref(animation);
	}
		else
		{
			/* No animation exists so destroy actor immediately */
			clutter_actor_destroy(self);
		}

	/* Clean allocated resources */
	if(animation) g_object_unref(animation);

	/* Return TRUE if animation was found and started or FALSE if no
	 * animation was found and actor as destroyed immediately.
	 */
	return(animationStarted);
}

void xfce_overview_workspaces_actor_destroy_all_children(ClutterActor *self)
{
	ClutterActorIter		iter;

	g_return_if_fail(CLUTTER_IS_ACTOR(self));

	/* Iterate through children and destroy them */
	g_object_freeze_notify(G_OBJECT(self));

	clutter_actor_iter_init(&iter, self);
	while(clutter_actor_iter_next(&iter, NULL))
	{
		xfce_overview_workspaces_actor_iter_destroy(&iter);
	}

	g_object_thaw_notify(G_OBJECT(self));
}

gboolean xfce_overview_workspaces_actor_iter_destroy(ClutterActorIter *self)
{
	/* Do not know currently how to wrap this safely, so just call
	 * clutter_actor_iter_destroy() and return FALSE as no animation
	 * was found (because we did not check ;) ).
	 */
	clutter_actor_iter_destroy(self);

	/* Return TRUE if animation was found and started or FALSE if no
	 * animation was found and actor as destroyed immediately.
	 */
	return(FALSE);
}
