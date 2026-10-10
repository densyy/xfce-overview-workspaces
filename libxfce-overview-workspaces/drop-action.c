/*
 * drop-action: Drop action for drop targets
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

#include <libxfce-overview-workspaces/drop-action.h>

#include <glib/gi18n-lib.h>

#include <libxfce-overview-workspaces/marshal.h>
#include <libxfce-overview-workspaces/actor.h>
#include <libxfce-overview-workspaces/stylable.h>


/* Define this class in GObject system */
struct _XfceOverviewWorkspacesDropActionPrivate
{
	/* Instance related */
	ClutterActor	*actor;
	guint			destroySignalID;
};

G_DEFINE_TYPE_WITH_PRIVATE(XfceOverviewWorkspacesDropAction,
							xfce_overview_workspaces_drop_action,
							CLUTTER_TYPE_ACTION)

/* Signals */
enum
{
	SIGNAL_BEGIN,
	SIGNAL_CAN_DROP,
	SIGNAL_DROP,
	SIGNAL_END,

	SIGNAL_DRAG_ENTER,
	SIGNAL_DRAG_MOTION,
	SIGNAL_DRAG_LEAVE,

	SIGNAL_LAST
};

guint XfceOverviewWorkspacesDropActionSignals[SIGNAL_LAST]={ 0, };

/* IMPLEMENTATION: Private variables and methods */
static GSList		*_xfce_overview_workspaces_drop_action_targets=NULL;

/* Register a new drop target */
static void _xfce_overview_workspaces_drop_action_register_target(XfceOverviewWorkspacesDropAction *self)
{
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_DROP_ACTION(self));

	/* Check if target is already registered */
	if(g_slist_find(_xfce_overview_workspaces_drop_action_targets, self))
	{
		g_warning("Target %s is already registered", G_OBJECT_TYPE_NAME(self));
		return;
	}

	/* Add object to list of dropable targets */
	_xfce_overview_workspaces_drop_action_targets=g_slist_prepend(_xfce_overview_workspaces_drop_action_targets, self);
}

/* Unregister a drop target */
static void _xfce_overview_workspaces_drop_action_unregister_target(XfceOverviewWorkspacesDropAction *self)
{
	XfceOverviewWorkspacesDropActionPrivate		*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_DROP_ACTION(self));

	priv=self->priv;

	/* Unset style */
	if(priv->actor &&
		XFCE_OVERVIEW_WORKSPACES_IS_ACTOR(priv->actor))
	{
		xfce_overview_workspaces_stylable_remove_pseudo_class(XFCE_OVERVIEW_WORKSPACES_STYLABLE(priv->actor), "drop-target");
	}

	/* Remove target from list of dropable targets */
	_xfce_overview_workspaces_drop_action_targets=g_slist_remove(_xfce_overview_workspaces_drop_action_targets, self);
}

/* Signal accumulator which stops further signal emission on first
 * signal handler returning FALSE
 */
static gboolean _xfce_overview_workspaces_drop_action_stop_signal_on_first_false(GSignalInvocationHint *inHint,
																		GValue *outAccumulator,
																		const GValue *inSignalResult,
																		gpointer inUserData)
{
	gboolean		continueEmission;

	continueEmission=g_value_get_boolean(inSignalResult);
	g_value_set_boolean(outAccumulator, continueEmission);

	return(continueEmission);
}

/* Target actor will be destroyed */
static void _xfce_overview_workspaces_drop_action_on_target_actor_destroy(XfceOverviewWorkspacesDropAction *self, ClutterActor *inTarget)
{
	XfceOverviewWorkspacesDropActionPrivate		*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_DROP_ACTION(self));
	g_return_if_fail(CLUTTER_IS_ACTOR(inTarget));

	priv=self->priv;

	/* Check that destroyed actor matches drop action's target actor */
	g_return_if_fail(inTarget==priv->actor);

	/* Disconnect signals */
	if(priv->destroySignalID) g_signal_handler_disconnect(priv->actor, priv->destroySignalID);

	/* Unregister drop target */
	_xfce_overview_workspaces_drop_action_unregister_target(self);

	priv->destroySignalID=0;
	priv->actor=NULL;
}

/* Default signal handler for "begin" */
static gboolean _xfce_overview_workspaces_drop_action_class_real_begin(XfceOverviewWorkspacesDropAction *self,
															XfceOverviewWorkspacesDragAction *inDragAction)
{
	XfceOverviewWorkspacesDropActionPrivate		*priv;
	ClutterActorMeta					*actorMeta;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_DROP_ACTION(self), FALSE);
	g_return_val_if_fail(self->priv->actor, FALSE);

	priv=self->priv;
	actorMeta=CLUTTER_ACTOR_META(self);

	/* Return TRUE means we can handle dragged actor on this drop target. This is only
	 * possible if drop target is active, visible and reactive. Otherwise we have to
	 * return FALSE to indicate that we cannot handle dragged actor.
	 */
	return(clutter_actor_meta_get_enabled(actorMeta) &&
			clutter_actor_is_visible(priv->actor) &&
			clutter_actor_get_reactive(priv->actor));
}

/* Default signal handler for "end" */
static void _xfce_overview_workspaces_drop_action_class_real_end(XfceOverviewWorkspacesDropAction *self,
													XfceOverviewWorkspacesDragAction *inDragAction)
{
	XfceOverviewWorkspacesDropActionPrivate		*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_DROP_ACTION(self));

	priv=self->priv;

	/* Unset style */
	if(priv->actor &&
		XFCE_OVERVIEW_WORKSPACES_IS_ACTOR(priv->actor))
	{
		xfce_overview_workspaces_stylable_remove_pseudo_class(XFCE_OVERVIEW_WORKSPACES_STYLABLE(priv->actor), "drop-target");
	}
}

/* Default signal handler for "can-drop" */
static gboolean _xfce_overview_workspaces_drop_action_class_real_can_drop(XfceOverviewWorkspacesDropAction *self,
																XfceOverviewWorkspacesDragAction *inDragAction,
																gfloat inX,
																gfloat inY)
{
	XfceOverviewWorkspacesDropActionPrivate		*priv;
	ClutterActorMeta					*actorMeta;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_DROP_ACTION(self), FALSE);
	g_return_val_if_fail(self->priv->actor, FALSE);

	priv=self->priv;
	actorMeta=CLUTTER_ACTOR_META(self);

	/* Return TRUE means we can handle dragged actor on this drop target. This is only
	 * possible if drop target is visible and reactive. Otherwise we have to return FALSE.
	 */
	return(clutter_actor_meta_get_enabled(actorMeta) &&
			clutter_actor_is_visible(priv->actor) &&
			clutter_actor_get_reactive(priv->actor));
}

/* Default signal handler for "drop" */
static void _xfce_overview_workspaces_drop_action_class_real_drop(XfceOverviewWorkspacesDropAction *self,
														XfceOverviewWorkspacesDragAction *inDragAction,
														gfloat inX,
														gfloat inY)
{
	XfceOverviewWorkspacesDropActionPrivate		*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_DROP_ACTION(self));

	priv=self->priv;

	/* Unset style */
	if(priv->actor &&
		XFCE_OVERVIEW_WORKSPACES_IS_ACTOR(priv->actor))
	{
		xfce_overview_workspaces_stylable_remove_pseudo_class(XFCE_OVERVIEW_WORKSPACES_STYLABLE(priv->actor), "drop-target");
	}
}

/* Default signal handler for "drag-enter" */
static void _xfce_overview_workspaces_drop_action_class_real_drag_enter(XfceOverviewWorkspacesDropAction *self,
															XfceOverviewWorkspacesDragAction *inDragAction)
{
	XfceOverviewWorkspacesDropActionPrivate		*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_DROP_ACTION(self));

	priv=self->priv;

	/* Unset style */
	if(priv->actor &&
		XFCE_OVERVIEW_WORKSPACES_IS_ACTOR(priv->actor))
	{
		xfce_overview_workspaces_stylable_add_pseudo_class(XFCE_OVERVIEW_WORKSPACES_STYLABLE(priv->actor), "drop-target");
	}
}

/* Default signal handler for "drag-leave" */
static void _xfce_overview_workspaces_drop_action_class_real_drag_leave(XfceOverviewWorkspacesDropAction *self,
															XfceOverviewWorkspacesDragAction *inDragAction)
{
	XfceOverviewWorkspacesDropActionPrivate		*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_DROP_ACTION(self));

	priv=self->priv;

	/* Unset style */
	if(priv->actor &&
		XFCE_OVERVIEW_WORKSPACES_IS_ACTOR(priv->actor))
	{
		xfce_overview_workspaces_stylable_remove_pseudo_class(XFCE_OVERVIEW_WORKSPACES_STYLABLE(priv->actor), "drop-target");
	}
}

/* IMPLEMENTATION: ClutterActorMeta */

/* Called when attaching and detaching a ClutterActorMeta instance to a ClutterActor */
static void _xfce_overview_workspaces_drop_action_set_actor(ClutterActorMeta *inActorMeta, ClutterActor *inActor)
{
	XfceOverviewWorkspacesDropAction			*self;
	XfceOverviewWorkspacesDropActionPrivate	*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_DROP_ACTION(inActorMeta));
	g_return_if_fail(inActor==NULL || CLUTTER_IS_ACTOR(inActor));

	self=XFCE_OVERVIEW_WORKSPACES_DROP_ACTION(inActorMeta);
	priv=self->priv;

	/* Unregister current drop target */
	if(priv->actor)
	{
		/* Disconnect signals */
		if(priv->destroySignalID) g_signal_handler_disconnect(priv->actor, priv->destroySignalID);

		/* Unset style */
		if(XFCE_OVERVIEW_WORKSPACES_IS_ACTOR(priv->actor))
		{
			xfce_overview_workspaces_stylable_remove_pseudo_class(XFCE_OVERVIEW_WORKSPACES_STYLABLE(priv->actor), "drop-target");
		}

		/* Unregister drop target */
		_xfce_overview_workspaces_drop_action_unregister_target(self);

		priv->destroySignalID=0;
		priv->actor=NULL;
	}

	/* Register new drop target */
	if(inActor)
	{
		priv->actor=inActor;

		/* Register drop target */
		_xfce_overview_workspaces_drop_action_register_target(self);

		/* Connect signals */
		priv->destroySignalID=g_signal_connect_swapped(priv->actor,
														"destroy",
														G_CALLBACK(_xfce_overview_workspaces_drop_action_on_target_actor_destroy),
														self);
	}

	/* Call parent's class method */
	CLUTTER_ACTOR_META_CLASS(xfce_overview_workspaces_drop_action_parent_class)->set_actor(inActorMeta, inActor);
}

/* IMPLEMENTATION: GObject */

/* Class initialization
 * Override functions in parent classes and define properties
 * and signals
 */
void xfce_overview_workspaces_drop_action_class_init(XfceOverviewWorkspacesDropActionClass *klass)
{
	ClutterActorMetaClass		*actorMetaClass=CLUTTER_ACTOR_META_CLASS(klass);

	/* Override functions */
	actorMetaClass->set_actor=_xfce_overview_workspaces_drop_action_set_actor;

	klass->begin=_xfce_overview_workspaces_drop_action_class_real_begin;
	klass->end=_xfce_overview_workspaces_drop_action_class_real_end;
	klass->can_drop=_xfce_overview_workspaces_drop_action_class_real_can_drop;
	klass->drop=_xfce_overview_workspaces_drop_action_class_real_drop;
	klass->drag_enter=_xfce_overview_workspaces_drop_action_class_real_drag_enter;
	klass->drag_leave=_xfce_overview_workspaces_drop_action_class_real_drag_leave;

	/* Define signals */
	XfceOverviewWorkspacesDropActionSignals[SIGNAL_BEGIN]=
		g_signal_new("begin",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST,
						G_STRUCT_OFFSET(XfceOverviewWorkspacesDropActionClass, begin),
						_xfce_overview_workspaces_drop_action_stop_signal_on_first_false,
						NULL,
						_xfce_overview_workspaces_marshal_BOOLEAN__OBJECT,
						G_TYPE_BOOLEAN,
						1,
						XFCE_OVERVIEW_WORKSPACES_TYPE_DRAG_ACTION);

	XfceOverviewWorkspacesDropActionSignals[SIGNAL_CAN_DROP]=
		g_signal_new("can-drop",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST,
						G_STRUCT_OFFSET(XfceOverviewWorkspacesDropActionClass, can_drop),
						_xfce_overview_workspaces_drop_action_stop_signal_on_first_false,
						NULL,
						_xfce_overview_workspaces_marshal_BOOLEAN__OBJECT_FLOAT_FLOAT,
						G_TYPE_BOOLEAN,
						3,
						XFCE_OVERVIEW_WORKSPACES_TYPE_DRAG_ACTION,
						G_TYPE_FLOAT,
						G_TYPE_FLOAT);

	XfceOverviewWorkspacesDropActionSignals[SIGNAL_DROP]=
		g_signal_new("drop",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST,
						G_STRUCT_OFFSET(XfceOverviewWorkspacesDropActionClass, drop),
						NULL,
						NULL,
						_xfce_overview_workspaces_marshal_VOID__OBJECT_FLOAT_FLOAT,
						G_TYPE_NONE,
						3,
						XFCE_OVERVIEW_WORKSPACES_TYPE_DRAG_ACTION,
						G_TYPE_FLOAT,
						G_TYPE_FLOAT);

	XfceOverviewWorkspacesDropActionSignals[SIGNAL_END]=
		g_signal_new("end",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST,
						G_STRUCT_OFFSET(XfceOverviewWorkspacesDropActionClass, end),
						NULL,
						NULL,
						g_cclosure_marshal_VOID__OBJECT,
						G_TYPE_NONE,
						1,
						XFCE_OVERVIEW_WORKSPACES_TYPE_DRAG_ACTION);

	XfceOverviewWorkspacesDropActionSignals[SIGNAL_DRAG_ENTER]=
		g_signal_new("drag-enter",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST,
						G_STRUCT_OFFSET(XfceOverviewWorkspacesDropActionClass, drag_enter),
						NULL,
						NULL,
						g_cclosure_marshal_VOID__OBJECT,
						G_TYPE_NONE,
						1,
						XFCE_OVERVIEW_WORKSPACES_TYPE_DRAG_ACTION);

	XfceOverviewWorkspacesDropActionSignals[SIGNAL_DRAG_MOTION]=
		g_signal_new("drag-motion",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST,
						G_STRUCT_OFFSET(XfceOverviewWorkspacesDropActionClass, drag_motion),
						NULL,
						NULL,
						_xfce_overview_workspaces_marshal_VOID__OBJECT_FLOAT_FLOAT,
						G_TYPE_NONE,
						3,
						XFCE_OVERVIEW_WORKSPACES_TYPE_DRAG_ACTION,
						G_TYPE_FLOAT,
						G_TYPE_FLOAT);

	XfceOverviewWorkspacesDropActionSignals[SIGNAL_DRAG_LEAVE]=
		g_signal_new("drag-leave",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST,
						G_STRUCT_OFFSET(XfceOverviewWorkspacesDropActionClass, drag_leave),
						NULL,
						NULL,
						g_cclosure_marshal_VOID__OBJECT,
						G_TYPE_NONE,
						1,
						XFCE_OVERVIEW_WORKSPACES_TYPE_DRAG_ACTION);
}

/* Object initialization
 * Create private structure and set up default values
 */
void xfce_overview_workspaces_drop_action_init(XfceOverviewWorkspacesDropAction *self)
{
	XfceOverviewWorkspacesDropActionPrivate	*priv;

	priv=self->priv=xfce_overview_workspaces_drop_action_get_instance_private(self);

	/* Set up default values */
	priv->destroySignalID=0;
	priv->actor=NULL;
}

/* IMPLEMENTATION: Public API */

/* Create new actor */
ClutterAction* xfce_overview_workspaces_drop_action_new(void)
{
	return(g_object_new(XFCE_OVERVIEW_WORKSPACES_TYPE_DROP_ACTION, NULL));
}

/* Returns a new list of all drop targets. Caller is responsible to unref each
 * drop target in list and to free list with g_slist_free(), e.g.
 * g_slist_free_full(returned_list, g_object_unref);
 */
GSList* xfce_overview_workspaces_drop_action_get_targets(void)
{
	GSList	*entry;
	GSList	*copy=NULL;

	/* Create a deeply copied list of currently registered drop targets */
	for(entry=_xfce_overview_workspaces_drop_action_targets; entry; entry=g_slist_next(entry))
	{
		copy=g_slist_prepend(copy, g_object_ref(entry->data));
	}

	return(copy);
}
