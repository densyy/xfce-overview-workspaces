/*
 * focus-manager: Single-instance managing focusable actors
 *                for keyboard navigation
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

#include <libxfce-overview-workspaces/focus-manager.h>

#include <glib/gi18n-lib.h>

#include <libxfce-overview-workspaces/marshal.h>
#include <libxfce-overview-workspaces/stylable.h>
#include <libxfce-overview-workspaces/bindings-pool.h>
#include <libxfce-overview-workspaces/core.h>
#include <libxfce-overview-workspaces/debug.h>


/* Define this class in GObject system */
struct _XfceOverviewWorkspacesFocusManagerPrivate
{
	/* Instance related */
	GList					*registeredFocusables;
	XfceOverviewWorkspacesFocusable	*currentFocus;
};

G_DEFINE_TYPE_WITH_PRIVATE(XfceOverviewWorkspacesFocusManager,
							xfce_overview_workspaces_focus_manager,
							G_TYPE_OBJECT)

/* Signals */
enum
{
	SIGNAL_REGISTERED,
	SIGNAL_UNREGISTERED,

	SIGNAL_CHANGED,

	/* Actions */
	ACTION_FOCUS_MOVE_FIRST,
	ACTION_FOCUS_MOVE_LAST,
	ACTION_FOCUS_MOVE_NEXT,
	ACTION_FOCUS_MOVE_PREVIOUS,

	SIGNAL_LAST
};

static guint XfceOverviewWorkspacesFocusManagerSignals[SIGNAL_LAST]={ 0, };

/* IMPLEMENTATION: Private variables and methods */

/* A registered focusable actor is going to be destroyed so unregister it */
static void _xfce_overview_workspaces_focus_manager_on_focusable_destroy(XfceOverviewWorkspacesFocusManager *self,
															gpointer inUserData)
{
	XfceOverviewWorkspacesFocusable			*focusable;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_FOCUS_MANAGER(self));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_FOCUSABLE(inUserData));

	focusable=XFCE_OVERVIEW_WORKSPACES_FOCUSABLE(inUserData);

	/* Unregister going-to-be-destroyed focusable actor */
	xfce_overview_workspaces_focus_manager_unregister(self, focusable);
}

/* A registered focusable actor is going to be hidden or unrealized */
static void _xfce_overview_workspaces_focus_manager_on_focusable_hide(XfceOverviewWorkspacesFocusManager *self,
															gpointer inUserData)
{
	XfceOverviewWorkspacesFocusManagerPrivate	*priv;
	XfceOverviewWorkspacesFocusable			*focusable;
	XfceOverviewWorkspacesFocusable			*nextFocusable;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_FOCUS_MANAGER(self));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_FOCUSABLE(inUserData));

	priv=self->priv;
	focusable=XFCE_OVERVIEW_WORKSPACES_FOCUSABLE(inUserData);

	/* Only move focus if hidden or unrealized focusable actor is the one
	 * which has the focus currently.
	 */
	if(priv->currentFocus!=focusable) return;

	if(clutter_actor_is_mapped(CLUTTER_ACTOR(focusable)) &&
		clutter_actor_is_realized(CLUTTER_ACTOR(focusable)) &&
		clutter_actor_is_visible(CLUTTER_ACTOR(focusable)))
	{
		return;
	}

	/* Move focus to next focusable actor if this actor which has the current focus
	 * is going to be unrealized or hidden.
	 */
	nextFocusable=xfce_overview_workspaces_focus_manager_get_next_focusable(self, priv->currentFocus);
	if(nextFocusable && nextFocusable!=priv->currentFocus) xfce_overview_workspaces_focus_manager_set_focus(self, nextFocusable);
		else
		{
			xfce_overview_workspaces_focusable_unset_focus(priv->currentFocus);
			priv->currentFocus=NULL;
		}
}

/* Build target list of registered focusable actors for requested binding but also check
 * if this focus manager is a target.
 */
static GSList* _xfce_overview_workspaces_focus_manager_get_targets_for_binding(XfceOverviewWorkspacesFocusManager *self,
																	const XfceOverviewWorkspacesBinding *inBinding)
{
	GSList							*targets;
	gboolean						mustBeFocusable;
	GSList							*iter;
	XfceOverviewWorkspacesFocusable			*focusable;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_FOCUS_MANAGER(self), NULL);
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_BINDING(inBinding), NULL);

	targets=NULL;
	mustBeFocusable=TRUE;

	/* Get list of possible targets */
	targets=xfce_overview_workspaces_focus_manager_get_targets(self, xfce_overview_workspaces_binding_get_target(inBinding));

	/* Determine if unfocusable targets should be included */
	if(xfce_overview_workspaces_binding_get_flags(inBinding) & XFCE_OVERVIEW_WORKSPACES_BINDING_FLAGS_ALLOW_UNFOCUSABLE_TARGET)
	{
		mustBeFocusable=FALSE;
	}

	/* Remove unfocusable targets from list if they should not be included */
	if(mustBeFocusable)
	{
		for(iter=targets; iter; iter=g_slist_next(iter))
		{
			/* Get focusable actor */
			if(!XFCE_OVERVIEW_WORKSPACES_IS_FOCUSABLE(iter->data)) continue;
			focusable=XFCE_OVERVIEW_WORKSPACES_FOCUSABLE(iter->data);

			/* Check if focusable actor can be focused as it may be disabled */
			if(!xfce_overview_workspaces_focusable_can_focus(focusable))
			{
				/* Remove target from list as it cannot be focused */
				g_object_unref(focusable);
				targets=g_slist_delete_link(targets, iter);
			}
		}
	}

	/* Return list of targets found */
	XFCE_OVERVIEW_WORKSPACES_DEBUG(self, MISC,
						"Target list for action '%s' and target class '%s' has %d entries",
						xfce_overview_workspaces_binding_get_action(inBinding),
						xfce_overview_workspaces_binding_get_target(inBinding),
						g_slist_length(targets));
	return(targets);
}

/* Action signal to move focus to first focusable actor was emitted */
static gboolean _xfce_overview_workspaces_focus_manager_move_focus_first(XfceOverviewWorkspacesFocusManager *self,
															XfceOverviewWorkspacesFocusable *inSource,
															const gchar *inAction,
															ClutterEvent *inEvent)
{
	XfceOverviewWorkspacesFocusManagerPrivate	*priv;
	XfceOverviewWorkspacesFocusable			*currentFocusable;
	XfceOverviewWorkspacesFocusable			*newFocusable;
	GList							*iter;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_FOCUS_MANAGER(self), CLUTTER_EVENT_PROPAGATE);
	g_return_val_if_fail(inEvent, CLUTTER_EVENT_PROPAGATE);

	priv=self->priv;

	/* Get current focus */
	currentFocusable=xfce_overview_workspaces_focus_manager_get_focus(self);

	/* Iterate through registered focusable actor and find the first focusable one.
	 * We do not use xfce_overview_workspaces_focus_manager_get_next_focusable(self, NULL) as
	 * it could return a focusable actor which is beyond the current one in order.
	 * We do not want to change the focus if it is not "before" the current one.
	 */
	for(iter=priv->registeredFocusables; iter; iter=g_list_next(iter))
	{
		newFocusable=(XfceOverviewWorkspacesFocusable*)iter->data;

		/* If iterate reached the current focused actor then there it is no first
		 * focusable actor and we do not need to change the focus and can return.
		 */
		if(currentFocusable && newFocusable==currentFocusable) return(CLUTTER_EVENT_STOP);

		/* If focusable can be focused then focus it and return */
		if(xfce_overview_workspaces_focusable_can_focus(newFocusable))
		{
			xfce_overview_workspaces_focus_manager_set_focus(self, newFocusable);

			return(CLUTTER_EVENT_STOP);
		}
	}

	/* If we get here we iterated through all registered focusable actors but
	 * could not find a matching one to set focus to.
	 */
	return(CLUTTER_EVENT_STOP);
}

/* Action signal to move focus to last focusable actor was emitted */
static gboolean _xfce_overview_workspaces_focus_manager_move_focus_last(XfceOverviewWorkspacesFocusManager *self,
															XfceOverviewWorkspacesFocusable *inSource,
															const gchar *inAction,
															ClutterEvent *inEvent)
{
	XfceOverviewWorkspacesFocusManagerPrivate	*priv;
	XfceOverviewWorkspacesFocusable			*currentFocusable;
	XfceOverviewWorkspacesFocusable			*newFocusable;
	GList							*iter;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_FOCUS_MANAGER(self), CLUTTER_EVENT_PROPAGATE);
	g_return_val_if_fail(inEvent, CLUTTER_EVENT_PROPAGATE);

	priv=self->priv;

	/* Get current focus */
	currentFocusable=xfce_overview_workspaces_focus_manager_get_focus(self);

	/* Iterate backwards through registered focusable actor and find the last focusable
	 * one. We do not use xfce_overview_workspaces_focus_manager_get_previous_focusable(self, NULL)
	 * as it could return a focusable actor which is before the current one in order.
	 * We do not want to change the focus if it is not "after" the current one.
	 */
	for(iter=g_list_last(priv->registeredFocusables); iter; iter=g_list_previous(iter))
	{
		newFocusable=(XfceOverviewWorkspacesFocusable*)iter->data;

		/* If iterate reached the current focused actor then there it is no last
		 * focusable actor and we do not need to change the focus and can return.
		 */
		if(currentFocusable && newFocusable==currentFocusable) return(CLUTTER_EVENT_STOP);

		/* If focusable can be focused then focus it and return */
		if(xfce_overview_workspaces_focusable_can_focus(newFocusable))
		{
			xfce_overview_workspaces_focus_manager_set_focus(self, newFocusable);

			return(CLUTTER_EVENT_STOP);
		}
	}

	/* If we get here we iterated through all registered focusable actors but
	 * could not find a matching one to set focus to.
	 */
	return(CLUTTER_EVENT_STOP);
}

/* Action signal to move focus to next focusable actor was emitted */
static gboolean _xfce_overview_workspaces_focus_manager_move_focus_next(XfceOverviewWorkspacesFocusManager *self,
															XfceOverviewWorkspacesFocusable *inSource,
															const gchar *inAction,
															ClutterEvent *inEvent)
{
	XfceOverviewWorkspacesFocusable			*currentFocusable;
	XfceOverviewWorkspacesFocusable			*newFocusable;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_FOCUS_MANAGER(self), CLUTTER_EVENT_PROPAGATE);
	g_return_val_if_fail(inEvent, CLUTTER_EVENT_PROPAGATE);

	/* Get current focus */
	currentFocusable=xfce_overview_workspaces_focus_manager_get_focus(self);

	/* Get next focusable actor to focus */
	newFocusable=xfce_overview_workspaces_focus_manager_get_next_focusable(self, currentFocusable);
	if(newFocusable) xfce_overview_workspaces_focus_manager_set_focus(self, newFocusable);

	return(CLUTTER_EVENT_STOP);
}

/* Action signal to move focus to previous focusable actor was emitted */
static gboolean _xfce_overview_workspaces_focus_manager_move_focus_previous(XfceOverviewWorkspacesFocusManager *self,
																XfceOverviewWorkspacesFocusable *inSource,
																const gchar *inAction,
																ClutterEvent *inEvent)
{
	XfceOverviewWorkspacesFocusable			*currentFocusable;
	XfceOverviewWorkspacesFocusable			*newFocusable;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_FOCUS_MANAGER(self), CLUTTER_EVENT_PROPAGATE);
	g_return_val_if_fail(inEvent, CLUTTER_EVENT_PROPAGATE);

	/* Get current focus */
	currentFocusable=xfce_overview_workspaces_focus_manager_get_focus(self);

	/* Get next focusable actor to focus */
	newFocusable=xfce_overview_workspaces_focus_manager_get_previous_focusable(self, currentFocusable);
	if(newFocusable) xfce_overview_workspaces_focus_manager_set_focus(self, newFocusable);

	return(CLUTTER_EVENT_STOP);
}

/* IMPLEMENTATION: GObject */

/* Dispose this object */
static void _xfce_overview_workspaces_focus_manager_dispose_unregister_focusable(gpointer inData, gpointer inUserData)
{
	XfceOverviewWorkspacesFocusManager			*self;
	XfceOverviewWorkspacesFocusable			*focusable;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_FOCUS_MANAGER(inUserData));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_FOCUSABLE(inData));

	self=XFCE_OVERVIEW_WORKSPACES_FOCUS_MANAGER(inUserData);
	focusable=XFCE_OVERVIEW_WORKSPACES_FOCUSABLE(inData);

	/* Unregister focusable actor but do not call general unregister function
	 * to avoid spamming focus changes and to avoid modifying list of focusable
	 * actor while iterating through it.
	 */
	g_signal_handlers_disconnect_by_func(focusable,
											G_CALLBACK(_xfce_overview_workspaces_focus_manager_on_focusable_destroy),
											self);
	g_signal_handlers_disconnect_by_func(focusable,
											G_CALLBACK(_xfce_overview_workspaces_focus_manager_on_focusable_hide),
											self);

	g_signal_emit(self, XfceOverviewWorkspacesFocusManagerSignals[SIGNAL_UNREGISTERED], 0, focusable);
}

static void _xfce_overview_workspaces_focus_manager_dispose(GObject *inObject)
{
	XfceOverviewWorkspacesFocusManager			*self=XFCE_OVERVIEW_WORKSPACES_FOCUS_MANAGER(inObject);
	XfceOverviewWorkspacesFocusManagerPrivate	*priv=self->priv;

	/* Release allocated resouces */
	if(priv->registeredFocusables)
	{
		g_list_foreach(priv->registeredFocusables, _xfce_overview_workspaces_focus_manager_dispose_unregister_focusable, self);
		g_list_free(priv->registeredFocusables);
		priv->registeredFocusables=NULL;
	}

	/* Call parent's class dispose method */
	G_OBJECT_CLASS(xfce_overview_workspaces_focus_manager_parent_class)->dispose(inObject);
}


/* Class initialization
 * Override functions in parent classes and define properties
 * and signals
 */
static void xfce_overview_workspaces_focus_manager_class_init(XfceOverviewWorkspacesFocusManagerClass *klass)
{
	GObjectClass		*gobjectClass=G_OBJECT_CLASS(klass);

	/* Override functions */
	gobjectClass->dispose=_xfce_overview_workspaces_focus_manager_dispose;

	klass->focus_move_first=_xfce_overview_workspaces_focus_manager_move_focus_first;
	klass->focus_move_last=_xfce_overview_workspaces_focus_manager_move_focus_last;
	klass->focus_move_next=_xfce_overview_workspaces_focus_manager_move_focus_next;
	klass->focus_move_previous=_xfce_overview_workspaces_focus_manager_move_focus_previous;

	/* Define signals */
	XfceOverviewWorkspacesFocusManagerSignals[SIGNAL_REGISTERED]=
		g_signal_new("registered",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST,
						G_STRUCT_OFFSET(XfceOverviewWorkspacesFocusManagerClass, registered),
						NULL,
						NULL,
						g_cclosure_marshal_VOID__OBJECT,
						G_TYPE_NONE,
						1,
						XFCE_OVERVIEW_WORKSPACES_TYPE_FOCUSABLE);

	XfceOverviewWorkspacesFocusManagerSignals[SIGNAL_UNREGISTERED]=
		g_signal_new("unregistered",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST,
						G_STRUCT_OFFSET(XfceOverviewWorkspacesFocusManagerClass, unregistered),
						NULL,
						NULL,
						g_cclosure_marshal_VOID__OBJECT,
						G_TYPE_NONE,
						1,
						XFCE_OVERVIEW_WORKSPACES_TYPE_FOCUSABLE);

	XfceOverviewWorkspacesFocusManagerSignals[SIGNAL_CHANGED]=
		g_signal_new("changed",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST,
						G_STRUCT_OFFSET(XfceOverviewWorkspacesFocusManagerClass, changed),
						NULL,
						NULL,
						_xfce_overview_workspaces_marshal_VOID__OBJECT_OBJECT,
						G_TYPE_NONE,
						2,
						XFCE_OVERVIEW_WORKSPACES_TYPE_FOCUSABLE,
						XFCE_OVERVIEW_WORKSPACES_TYPE_FOCUSABLE);

	XfceOverviewWorkspacesFocusManagerSignals[ACTION_FOCUS_MOVE_FIRST]=
		g_signal_new("focus-move-first",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
						G_STRUCT_OFFSET(XfceOverviewWorkspacesFocusManagerClass, focus_move_first),
						g_signal_accumulator_true_handled,
						NULL,
						_xfce_overview_workspaces_marshal_BOOLEAN__OBJECT_STRING_BOXED,
						G_TYPE_BOOLEAN,
						3,
						XFCE_OVERVIEW_WORKSPACES_TYPE_FOCUSABLE,
						G_TYPE_STRING,
						CLUTTER_TYPE_EVENT);

	XfceOverviewWorkspacesFocusManagerSignals[ACTION_FOCUS_MOVE_LAST]=
		g_signal_new("focus-move-last",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
						G_STRUCT_OFFSET(XfceOverviewWorkspacesFocusManagerClass, focus_move_last),
						g_signal_accumulator_true_handled,
						NULL,
						_xfce_overview_workspaces_marshal_BOOLEAN__OBJECT_STRING_BOXED,
						G_TYPE_BOOLEAN,
						3,
						XFCE_OVERVIEW_WORKSPACES_TYPE_FOCUSABLE,
						G_TYPE_STRING,
						CLUTTER_TYPE_EVENT);

	XfceOverviewWorkspacesFocusManagerSignals[ACTION_FOCUS_MOVE_NEXT]=
		g_signal_new("focus-move-next",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
						G_STRUCT_OFFSET(XfceOverviewWorkspacesFocusManagerClass, focus_move_next),
						g_signal_accumulator_true_handled,
						NULL,
						_xfce_overview_workspaces_marshal_BOOLEAN__OBJECT_STRING_BOXED,
						G_TYPE_BOOLEAN,
						3,
						XFCE_OVERVIEW_WORKSPACES_TYPE_FOCUSABLE,
						G_TYPE_STRING,
						CLUTTER_TYPE_EVENT);

	XfceOverviewWorkspacesFocusManagerSignals[ACTION_FOCUS_MOVE_PREVIOUS]=
		g_signal_new("focus-move-previous",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
						G_STRUCT_OFFSET(XfceOverviewWorkspacesFocusManagerClass, focus_move_previous),
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
static void xfce_overview_workspaces_focus_manager_init(XfceOverviewWorkspacesFocusManager *self)
{
	XfceOverviewWorkspacesFocusManagerPrivate	*priv;

	priv=self->priv=xfce_overview_workspaces_focus_manager_get_instance_private(self);

	/* Set default values */
	priv->registeredFocusables=NULL;
	priv->currentFocus=NULL;
}

/* IMPLEMENTATION: Public API */

/* Register a focusable actor */
void xfce_overview_workspaces_focus_manager_register(XfceOverviewWorkspacesFocusManager *self, XfceOverviewWorkspacesFocusable *inFocusable)
{
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_FOCUS_MANAGER(self));

	xfce_overview_workspaces_focus_manager_register_after(self, inFocusable, NULL);
}

void xfce_overview_workspaces_focus_manager_register_after(XfceOverviewWorkspacesFocusManager *self, XfceOverviewWorkspacesFocusable *inFocusable, XfceOverviewWorkspacesFocusable *inAfterFocusable)
{
	XfceOverviewWorkspacesFocusManagerPrivate	*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_FOCUS_MANAGER(self));
	g_return_if_fail(inFocusable);
	g_return_if_fail(!inAfterFocusable || XFCE_OVERVIEW_WORKSPACES_IS_FOCUSABLE(inAfterFocusable));

	priv=self->priv;

	/* Check if given focusable actor is really focusable and stylable */
	if(!XFCE_OVERVIEW_WORKSPACES_IS_FOCUSABLE(inFocusable))
	{
		g_warning("Object %s does not inherit %s and cannot be registered",
					G_OBJECT_TYPE_NAME(inFocusable),
					g_type_name(XFCE_OVERVIEW_WORKSPACES_TYPE_FOCUSABLE));
		return;
	}

	if(!XFCE_OVERVIEW_WORKSPACES_IS_STYLABLE(inFocusable))
	{
		g_warning("Object %s does not inherit %s and cannot be registered",
					G_OBJECT_TYPE_NAME(inFocusable),
					g_type_name(XFCE_OVERVIEW_WORKSPACES_TYPE_STYLABLE));
		return;
	}

	/* Register focusable actor if not already registered */
	if(g_list_find(priv->registeredFocusables, inFocusable)==NULL)
	{
		gint						insertPosition;

		XFCE_OVERVIEW_WORKSPACES_DEBUG(self, MISC,
							"Registering focusable %s",
							G_OBJECT_TYPE_NAME(inFocusable));

		/* If requested find position of focusable actor to insert new focusable actor after.
		 * Increase found position by one and add new focusable actor to list of registered
		 * focusable actors at this position. Otherwise add new focusable actor to end of list.
		 */
		insertPosition=-1;
		if(inAfterFocusable)
		{
			insertPosition=g_list_index(priv->registeredFocusables, inAfterFocusable);
			if(insertPosition!=-1) insertPosition++;
				else
				{
					g_warning("Could not find registered focusable object %s to register object %s - appending to end of list.",
								G_OBJECT_TYPE_NAME(inAfterFocusable),
								G_OBJECT_TYPE_NAME(inFocusable));
				}
		}
		priv->registeredFocusables=g_list_insert(priv->registeredFocusables, inFocusable, insertPosition);

		/* Connect to signals to get notified if actor is going to be destroy,
		 * unrealized or hidden to remove it from list of focusable actors.
		 */
		g_signal_connect_swapped(inFocusable,
									"destroy",
									G_CALLBACK(_xfce_overview_workspaces_focus_manager_on_focusable_destroy),
									self);
		g_signal_connect_swapped(inFocusable,
									"realize",
									G_CALLBACK(_xfce_overview_workspaces_focus_manager_on_focusable_hide),
									self);
		g_signal_connect_swapped(inFocusable,
									"hide",
									G_CALLBACK(_xfce_overview_workspaces_focus_manager_on_focusable_hide),
									self);

		/* Emit signal */
		g_signal_emit(self, XfceOverviewWorkspacesFocusManagerSignals[SIGNAL_REGISTERED], 0, inFocusable);
	}
}

/* Unregister a focusable actor */
void xfce_overview_workspaces_focus_manager_unregister(XfceOverviewWorkspacesFocusManager *self, XfceOverviewWorkspacesFocusable *inFocusable)
{
	XfceOverviewWorkspacesFocusManagerPrivate	*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_FOCUS_MANAGER(self));
	g_return_if_fail(inFocusable);

	priv=self->priv;

	/* Unregister type if registered.
	 * We do not need to check if the given actor is focusable or stylable
	 * because it could not be registered if it is not.
	 */
	if(g_list_find(priv->registeredFocusables, inFocusable)!=NULL)
	{
		XFCE_OVERVIEW_WORKSPACES_DEBUG(self, MISC,
							"Unregistering focusable %s",
							G_OBJECT_TYPE_NAME(inFocusable));

		/* If we unregister the focusable actor which has the focus currently
		 * move focus to next focusable actor first but check that we will not
		 * reselect the focusable actor which should be unregistered. That can
		 * happen because this actor is not yet removed from list of registered
		 * focusable actor and is the only selectable one. But it needs to be
		 * still in the list otherwise we could not find the next actor to
		 * focus appropiately.
		 */
		if(inFocusable==priv->currentFocus)
		{
			XfceOverviewWorkspacesFocusable	*focusable;

			focusable=xfce_overview_workspaces_focus_manager_get_next_focusable(self, priv->currentFocus);
			if(focusable && focusable!=priv->currentFocus) xfce_overview_workspaces_focus_manager_set_focus(self, focusable);
				else
				{
					xfce_overview_workspaces_focusable_unset_focus(priv->currentFocus);
					priv->currentFocus=NULL;
				}
		}

		/* Remove focusable actor from list of registered focusable actors */
		priv->registeredFocusables=g_list_remove(priv->registeredFocusables, inFocusable);

		/* Disconnect from signals because we are not interested in this actor anymore */
		g_signal_handlers_disconnect_by_func(inFocusable,
												G_CALLBACK(_xfce_overview_workspaces_focus_manager_on_focusable_destroy),
												self);
		g_signal_handlers_disconnect_by_func(inFocusable,
												G_CALLBACK(_xfce_overview_workspaces_focus_manager_on_focusable_hide),
												self);

		/* Emit signal */
		g_signal_emit(self, XfceOverviewWorkspacesFocusManagerSignals[SIGNAL_UNREGISTERED], 0, inFocusable);
	}
}

/* Get list of registered views types.
 * Returned GList must be freed with g_list_free() by caller.
 */
GList* xfce_overview_workspaces_focus_manager_get_registered(XfceOverviewWorkspacesFocusManager *self)
{
	XfceOverviewWorkspacesFocusManagerPrivate	*priv;
	GList							*copy;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_FOCUS_MANAGER(self), NULL);

	priv=self->priv;

	/* Return a copy of list of registered view types */
	copy=g_list_copy(priv->registeredFocusables);
	return(copy);
}

/* Check if given focusable actor is registered */
gboolean xfce_overview_workspaces_focus_manager_is_registered(XfceOverviewWorkspacesFocusManager *self, XfceOverviewWorkspacesFocusable *inFocusable)
{
	XfceOverviewWorkspacesFocusManagerPrivate	*priv;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_FOCUS_MANAGER(self), FALSE);
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_FOCUSABLE(inFocusable), FALSE);

	priv=self->priv;

	/* If given focusable actor is in list of registered ones return TRUE */
	if(g_list_find(priv->registeredFocusables, inFocusable)!=NULL) return(TRUE);

	/* If here get here the given focusable actor is not registered */
	return(FALSE);
}

/* Build target list of registered focusable actors for requested target class
 * but also check if this focus manager is a target.
 */
GSList* xfce_overview_workspaces_focus_manager_get_targets(XfceOverviewWorkspacesFocusManager *self, const gchar *inTarget)
{
	XfceOverviewWorkspacesFocusManagerPrivate	*priv;
	GList							*focusablesIter;
	GList							*focusablesStartPoint;
	XfceOverviewWorkspacesFocusable			*focusable;
	GType							targetType;
	GSList							*targets;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_FOCUS_MANAGER(self), NULL);
	g_return_val_if_fail(inTarget && *inTarget, NULL);

	priv=self->priv;
	targets=NULL;

	/* Provide backward-compatibility by checking target for old application
	 * type by name (XfceOverviewWorkspacesApplication).
	 */
	if(g_strcmp0("XfceOverviewWorkspacesApplication", inTarget)==0)
	{
		static gboolean				warnedDeprecation=FALSE;

		if(!warnedDeprecation)
		{
			g_warning("Bindings uses deprecated target 'XfceOverviewWorkspacesApplication'. Please update to use target 'XfceOverviewWorkspacesCore'.");
			warnedDeprecation=TRUE;
		}

		inTarget="XfceOverviewWorkspacesCore";
	}

	/* Get type of target */
	targetType=g_type_from_name(inTarget);
	if(!targetType)
	{
		g_warning("Cannot build target list for unknown type %s", inTarget);
		return(NULL);
	}

	/* Check if class name of requested target points to ourselve */
	if(g_type_is_a(G_OBJECT_TYPE(self), targetType))
	{
		targets=g_slist_append(targets, g_object_ref(self));
	}

	/* Check if class name of requested target points to core
	 * and provide backward-compatibility by checking for old
	 * application object by name.
	 */
	if(g_type_is_a(XFCE_OVERVIEW_WORKSPACES_TYPE_CORE, targetType))
	{
		targets=g_slist_append(targets, g_object_ref(xfce_overview_workspaces_core_get_default()));
	}

	/* Iterate through list of registered actors and add each one
	 * matching the target class name to the list of targets.
	 * Begin with finding starting point of iteration.
	 */
	focusablesStartPoint=g_list_find(priv->registeredFocusables, priv->currentFocus);
	if(!focusablesStartPoint) focusablesStartPoint=priv->registeredFocusables;

	/* Iterate through list of registered actors beginning at found starting
	 * point of iteration (might be begin of list of registered actors)
	 * and add each actor matching target class name to target list.
	 */
	for(focusablesIter=focusablesStartPoint; focusablesIter; focusablesIter=g_list_next(focusablesIter))
	{
		focusable=(XfceOverviewWorkspacesFocusable*)focusablesIter->data;

		/* If focusable can be focused and matches target class name
		 * then add it to target list.
		 */
		if(g_type_is_a(G_OBJECT_TYPE(focusable), targetType))
		{
			targets=g_slist_append(targets, g_object_ref(focusable));
		}
	}

	/* We have to continue search at the beginning of list of registered actors
	 * up to the found starting point of iteration. Add each actor matching
	 * target class name to target list.
	 */
	for(focusablesIter=priv->registeredFocusables; focusablesIter!=focusablesStartPoint; focusablesIter=g_list_next(focusablesIter))
	{
		focusable=(XfceOverviewWorkspacesFocusable*)focusablesIter->data;

		/* If focusable can be focused and matches target class name
		 * then add it to target list.
		 */
		if(g_type_is_a(G_OBJECT_TYPE(focusable), targetType))
		{
			targets=g_slist_append(targets, g_object_ref(focusable));
		}
	}

	/* Return list of targets found */
	XFCE_OVERVIEW_WORKSPACES_DEBUG(self, MISC,
						"Target list for target class '%s' has %d entries",
						inTarget,
						g_slist_length(targets));

	return(targets);
}

/* Determine if a specific actor has the focus */
gboolean xfce_overview_workspaces_focus_manager_has_focus(XfceOverviewWorkspacesFocusManager *self, XfceOverviewWorkspacesFocusable *inFocusable)
{
	XfceOverviewWorkspacesFocusManagerPrivate	*priv;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_FOCUS_MANAGER(self), FALSE);
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_FOCUSABLE(inFocusable), FALSE);

	priv=self->priv;

	/* Return TRUE if given actor has the focus otherwise return FALSE */
	return(priv->currentFocus==inFocusable ? TRUE : FALSE);
}

/* Get focusable actor which has the focus currently */
XfceOverviewWorkspacesFocusable* xfce_overview_workspaces_focus_manager_get_focus(XfceOverviewWorkspacesFocusManager *self)
{
	XfceOverviewWorkspacesFocusManagerPrivate	*priv;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_FOCUS_MANAGER(self), NULL);

	priv=self->priv;

	/* Return found focused focusable actor */
	return(priv->currentFocus);
}

/* Set focus to a registered focusable actor */
void xfce_overview_workspaces_focus_manager_set_focus(XfceOverviewWorkspacesFocusManager *self, XfceOverviewWorkspacesFocusable *inFocusable)
{
	XfceOverviewWorkspacesFocusManagerPrivate	*priv;
	XfceOverviewWorkspacesFocusable			*oldFocusable;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_FOCUS_MANAGER(self));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_FOCUSABLE(inFocusable));

	priv=self->priv;
	oldFocusable=NULL;

	/* Check if focusable actor is really registered */
	if(g_list_find(priv->registeredFocusables, inFocusable)==NULL)
	{
		g_warning("Trying to focus an unregistered focusable actor");
		return;
	}

	/* Check if new focusable actor can be focussed. If it cannot be focussed
	 * move focus to next focusable actor. If no focusable actor can be found
	 * do not change focus at all.
	 */
	if(!xfce_overview_workspaces_focusable_can_focus(inFocusable))
	{
		XfceOverviewWorkspacesFocusable		*newFocusable;

		newFocusable=xfce_overview_workspaces_focus_manager_get_next_focusable(self, inFocusable);
		if(!newFocusable)
		{
			XFCE_OVERVIEW_WORKSPACES_DEBUG(self, MISC,
								"Requested focusable actor '%s' cannot be focus but no other focusable actor was found",
								G_OBJECT_TYPE_NAME(inFocusable));
			return;
		}

		XFCE_OVERVIEW_WORKSPACES_DEBUG(self, MISC,
							"Requested focusable actor '%s' cannot be focused - moving focus to '%s'",
							G_OBJECT_TYPE_NAME(inFocusable),
							newFocusable ? G_OBJECT_TYPE_NAME(newFocusable) : "<nothing>");
		inFocusable=newFocusable;
	}

	/* Do nothing if current focused actor and new one are the same */
	oldFocusable=priv->currentFocus;
	if(oldFocusable==inFocusable)
	{
		XFCE_OVERVIEW_WORKSPACES_DEBUG(self, MISC, "Current focused actor and new one are the same so do nothing.");
		return;
	}

	/* Unset focus at current focused actor */
	if(priv->currentFocus)
	{
		xfce_overview_workspaces_focusable_unset_focus(priv->currentFocus);
		priv->currentFocus=NULL;
	}

	/* Set focus to new focusable actor */
	priv->currentFocus=inFocusable;
	xfce_overview_workspaces_focusable_set_focus(priv->currentFocus);
	XFCE_OVERVIEW_WORKSPACES_DEBUG(self, MISC,
						"Moved focus from '%s' to '%s'",
						oldFocusable ? G_OBJECT_TYPE_NAME(oldFocusable) : "<nothing>",
						G_OBJECT_TYPE_NAME(priv->currentFocus));

	/* Emit signal for changed focus */
	g_signal_emit(self, XfceOverviewWorkspacesFocusManagerSignals[SIGNAL_CHANGED], 0, oldFocusable, priv->currentFocus);
}

/* Find next focusable actor from given focusable actor */
XfceOverviewWorkspacesFocusable* xfce_overview_workspaces_focus_manager_get_next_focusable(XfceOverviewWorkspacesFocusManager *self,
																	XfceOverviewWorkspacesFocusable *inBeginFocusable)
{
	XfceOverviewWorkspacesFocusManagerPrivate	*priv;
	GList							*startIteration;
	GList							*iter;
	XfceOverviewWorkspacesFocusable			*focusable;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_FOCUS_MANAGER(self), NULL);
	g_return_val_if_fail(!inBeginFocusable || XFCE_OVERVIEW_WORKSPACES_IS_FOCUSABLE(inBeginFocusable), NULL);

	priv=self->priv;
	startIteration=NULL;

	/* Find starting point of iteration.
	 * If starting focusable actor for search for next focusable actor is NULL
	 * or if it is not registered start search at begin of list of focusable actors.
	 */
	if(inBeginFocusable) startIteration=g_list_find(priv->registeredFocusables, inBeginFocusable);
	if(startIteration) startIteration=g_list_next(startIteration);
		else startIteration=priv->registeredFocusables;

	/* Iterate through list of registered focusable actors beginning at
	 * given focusable actor (might be begin of this list) and return
	 * the first focusable actor which can be focused.
	 */
	for(iter=startIteration; iter; iter=g_list_next(iter))
	{
		focusable=(XfceOverviewWorkspacesFocusable*)iter->data;

		/* If focusable can be focused then return it */
		if(xfce_overview_workspaces_focusable_can_focus(focusable)) return(focusable);
	}

	/* If we get here we have to continue search at the beginning of list
	 * of registered focusable actors. Iterate through list of registered
	 * focusable actors from the beginning of that list up to the given
	 * focusable actor and return the first focusable actor which is focusable.
	 */
	for(iter=priv->registeredFocusables; iter!=startIteration; iter=g_list_next(iter))
	{
		focusable=(XfceOverviewWorkspacesFocusable*)iter->data;

		/* If focusable can be focused then return it */
		if(xfce_overview_workspaces_focusable_can_focus(focusable)) return(focusable);
	}

	/* If we get here we could not find next focusable actor */
	return(NULL);
}

/* Find previous focusable actor from given focusable actor */
XfceOverviewWorkspacesFocusable* xfce_overview_workspaces_focus_manager_get_previous_focusable(XfceOverviewWorkspacesFocusManager *self,
																		XfceOverviewWorkspacesFocusable *inBeginFocusable)
{
	XfceOverviewWorkspacesFocusManagerPrivate	*priv;
	GList							*startIteration;
	GList							*iter;
	XfceOverviewWorkspacesFocusable			*focusable;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_FOCUS_MANAGER(self), NULL);
	g_return_val_if_fail(!inBeginFocusable || XFCE_OVERVIEW_WORKSPACES_IS_FOCUSABLE(inBeginFocusable), NULL);

	priv=self->priv;
	startIteration=NULL;

	/* Find starting point of iteration.
	 * If starting focusable actor for search for next focusable actor is NULL
	 * or if it is not registered start search at begin of list of focusable actors.
	 */
	if(inBeginFocusable) startIteration=g_list_find(priv->registeredFocusables, inBeginFocusable);
	if(startIteration) startIteration=g_list_previous(startIteration);
		else startIteration=priv->registeredFocusables;

	/* Iterate reverse through list of registered focusable actors beginning
	 * at given focusable actor (might be begin of this list) and return
	 * the first focusable actor which can be focused.
	 */
	for(iter=startIteration; iter; iter=g_list_previous(iter))
	{
		focusable=(XfceOverviewWorkspacesFocusable*)iter->data;

		/* If focusable can be focused then return it */
		if(xfce_overview_workspaces_focusable_can_focus(focusable)) return(focusable);
	}

	/* If we get here we have to continue search at the end of list
	 * of registered focusable actors. Iterate reverse through list of
	 * registered focusable actors from the beginning of that list up
	 * to the given focusable actor and return the first focusable actor
	 * which is focusable.
	 */
	for(iter=g_list_last(priv->registeredFocusables); iter!=startIteration; iter=g_list_previous(iter))
	{
		focusable=(XfceOverviewWorkspacesFocusable*)iter->data;

		/* If focusable can be focused then return it */
		if(xfce_overview_workspaces_focusable_can_focus(focusable)) return(focusable);
	}

	/* If we get here we could not find next focusable actor */
	return(NULL);
}

/* Determine list of target actors and the action to perform for key-press or
 * key-release event.
 */
gboolean xfce_overview_workspaces_focus_manager_get_event_targets_and_action(XfceOverviewWorkspacesFocusManager *self,
																const ClutterEvent *inEvent,
																XfceOverviewWorkspacesFocusable *inFocusable,
																GSList **outTargets,
																const gchar **outAction)
{
	XfceOverviewWorkspacesFocusManagerPrivate	*priv;
	XfceOverviewWorkspacesBindingsPool			*bindingsPool;
	const XfceOverviewWorkspacesBinding		*binding;
	const gchar						*action;
	GSList							*targetFocusables;
	gboolean						status;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_FOCUS_MANAGER(self), FALSE);
	g_return_val_if_fail(inEvent, FALSE);
	g_return_val_if_fail(clutter_event_type(inEvent)==CLUTTER_KEY_PRESS || clutter_event_type(inEvent)==CLUTTER_KEY_RELEASE, FALSE);
	g_return_val_if_fail(!inFocusable || XFCE_OVERVIEW_WORKSPACES_IS_FOCUSABLE(inFocusable), FALSE);
	g_return_val_if_fail(outTargets && *outTargets==NULL, FALSE);
	g_return_val_if_fail(outAction && *outAction==NULL, FALSE);

	priv=self->priv;
	action=NULL;
	targetFocusables=NULL;
	status=FALSE;

	/* If no focusable actor was specified then use current focused actor */
	if(!inFocusable)
	{
		inFocusable=priv->currentFocus;

		/* If still no focusable actor is available we cannot handle event
		 * so let the others try it by propagating event.
		 */
		if(!inFocusable) return(FALSE);
	}

	/* Take reference on ourselve and the focusable actor to keep them alive when handling event */
	g_object_ref(self);
	g_object_ref(inFocusable);

	/* Lookup action for event and emit action if a binding was found
	 * for this event.
	 */
	bindingsPool=xfce_overview_workspaces_core_get_bindings_pool(NULL);
	binding=xfce_overview_workspaces_bindings_pool_find_for_event(bindingsPool, CLUTTER_ACTOR(inFocusable), inEvent);
	if(binding)
	{
		const gchar					*target;

		/* Get action of binding */
		action=xfce_overview_workspaces_binding_get_action(binding);

		/* Build up list of targets which is either the requested focusable actor,
		 * the current focused actor or focusable actors of a specific type
		 */
		targetFocusables=NULL;
		target=xfce_overview_workspaces_binding_get_target(binding);
		if(target)
		{
			/* Target class name is specified so build up a list of targets */
			targetFocusables=_xfce_overview_workspaces_focus_manager_get_targets_for_binding(self, binding);
		}
			else
			{
				/* No target class name was specified so add requested focusable
				 * actor to list of target.
				 */
				targetFocusables=g_slist_append(targetFocusables, g_object_ref(inFocusable));
			}

		/* If target list is not empty then this event can be handled and status
		 * can to set to TRUE to reflect this state. Otherwise release allocated
		 * resources to prevent returning them to callee.
		 */
		if(g_slist_length(targetFocusables)>0) status=TRUE;
			else
			{
				/* Release allocated resources */
				if(targetFocusables)
				{
					g_slist_free_full(targetFocusables, g_object_unref);
					targetFocusables=NULL;
				}

				if(action)
				{
					action=NULL;
				}
			}
	}
	g_object_unref(bindingsPool);

	/* Release reference on ourselve and the focusable actor to took to keep them alive  */
	g_object_unref(inFocusable);
	g_object_unref(self);

	/* Store result at pointers if given otherwise release allocated resources */
	if(outTargets) *outTargets=targetFocusables;
		else g_slist_free_full(targetFocusables, g_object_unref);

	if(outAction) *outAction=action;

	/* Return status result */
	return(status);
}

/* Emit action to each target in list and return event status */
static gboolean _emit_action_to_targets(XfceOverviewWorkspacesFocusManager *self, GSList *targets, const gchar *action, const gchar *detail, XfceOverviewWorkspacesFocusable *focusable, const ClutterEvent *event)
{
	gboolean				eventStatus;
	GSList					*iter;
	GSignalQuery			signalData={ 0, };

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_FOCUS_MANAGER(self), CLUTTER_EVENT_PROPAGATE);

	eventStatus=CLUTTER_EVENT_PROPAGATE;

	for(iter=targets; iter; iter=g_slist_next(iter))
	{
		GObject				*targetObject;
		guint				signalID;

		/* Get target to emit action signal at */
		targetObject=G_OBJECT(iter->data);

		/* Check if target provides action requested as signal */
		signalID=g_signal_lookup(action, G_OBJECT_TYPE(targetObject));
		if(!signalID)
		{
			g_warning("Object type %s does not provide action '%s'",
							G_OBJECT_TYPE_NAME(targetObject),
							action);
			continue;
		}

		/* Query signal for detailed data */
		g_signal_query(signalID, &signalData);

		/* Check if signal is an action signal */
		if(!(signalData.signal_flags & G_SIGNAL_ACTION))
		{
			g_warning("Action '%s' at object type %s is not an action signal.",
						action,
						G_OBJECT_TYPE_NAME(targetObject));
			continue;
		}

		/* If detail was provided in action then check if signal supports
		 * details.
		 */
		if(detail &&
			!(signalData.signal_flags & G_SIGNAL_DETAILED))
		{
			g_warning("Action '%s' at object type %s does not support detail '%s' in action name.",
						action,
						detail,
						G_OBJECT_TYPE_NAME(targetObject));
			continue;
		}

#ifdef DEBUG
		/* In debug mode also check if signal has right signature
		 * to be able to handle this action properly.
		 */
		if(signalID)
		{
			GType				returnValueType=G_TYPE_BOOLEAN;
			GType				parameterTypesSimple[]={ XFCE_OVERVIEW_WORKSPACES_TYPE_FOCUSABLE, G_TYPE_STRING, CLUTTER_TYPE_EVENT };
			GType				parameterTypesDetail[]={ XFCE_OVERVIEW_WORKSPACES_TYPE_FOCUSABLE, G_TYPE_STRING, G_TYPE_STRING, CLUTTER_TYPE_EVENT };
			guint				parameterCount;
			guint				i;

			/* Check if signal wants the right type of return value */
			if(signalData.return_type!=returnValueType)
			{
				g_critical("Action '%s' at object type %s wants return value of type %s but expected is %s.",
							action,
							G_OBJECT_TYPE_NAME(targetObject),
							g_type_name(signalData.return_type),
							g_type_name(returnValueType));
			}

			/* Check if signals wants the right number and types of parameters */
			if(detail) parameterCount=sizeof(parameterTypesDetail)/sizeof(GType);
				else parameterCount=sizeof(parameterTypesSimple)/sizeof(GType);
			if(signalData.n_params!=parameterCount)
			{
				g_critical("Action '%s' at object type %s wants %u parameters but expected are %u.",
							action,
							G_OBJECT_TYPE_NAME(targetObject),
							signalData.n_params,
							parameterCount);
			}

			for(i=0; i<(parameterCount<signalData.n_params ? parameterCount : signalData.n_params); i++)
			{
				if((detail && signalData.param_types[i]!=parameterTypesDetail[i]) ||
					(!detail && signalData.param_types[i]!=parameterTypesSimple[i]))
				{
					g_critical("Action '%s' at object type %s wants type %s at parameter %u but type %s is expected.",
								action,
								G_OBJECT_TYPE_NAME(targetObject),
								g_type_name(signalData.param_types[i]),
								i+1,
								(detail ? g_type_name(parameterTypesDetail[i]) : g_type_name(parameterTypesSimple[i])));
				}
			}
		}
#endif

		/* Emit action signal at target */
		if(detail)
		{
			XFCE_OVERVIEW_WORKSPACES_DEBUG(self, ACTOR,
								"Emitting action signal '%s' with detail '%s' at focusable actor %s",
								action,
								detail,
								G_OBJECT_TYPE_NAME(targetObject));

			g_signal_emit_by_name(targetObject, action, focusable, action, detail, event, &eventStatus);

			XFCE_OVERVIEW_WORKSPACES_DEBUG(self, ACTOR,
								"Action signal '%s' with detail '%s' was %s by focusable actor %s",
								action,
								detail,
								eventStatus==CLUTTER_EVENT_STOP ? "handled" : "not handled",
								G_OBJECT_TYPE_NAME(targetObject));
		}
			else
			{
				XFCE_OVERVIEW_WORKSPACES_DEBUG(self, ACTOR,
									"Emitting action signal '%s' at focusable actor %s",
									action,
									G_OBJECT_TYPE_NAME(targetObject));

				g_signal_emit_by_name(targetObject, action, focusable, action, event, &eventStatus);

				XFCE_OVERVIEW_WORKSPACES_DEBUG(self, ACTOR,
									"Action signal '%s' was %s by focusable actor %s",
									action,
									eventStatus==CLUTTER_EVENT_STOP ? "handled" : "not handled",
									G_OBJECT_TYPE_NAME(targetObject));
			}
	}

	return(eventStatus);
}

/* Handle key event (it is either key-press or key-release) by focusable actor
 * which has the focus or by specified actor.
 */
gboolean xfce_overview_workspaces_focus_manager_handle_key_event(XfceOverviewWorkspacesFocusManager *self,
													const ClutterEvent *inEvent,
													XfceOverviewWorkspacesFocusable *inFocusable)
{
	XfceOverviewWorkspacesFocusManagerPrivate	*priv;
	GSList							*targetFocusables;
	const gchar						*actionFullName;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_FOCUS_MANAGER(self), CLUTTER_EVENT_PROPAGATE);
	g_return_val_if_fail(inEvent, CLUTTER_EVENT_PROPAGATE);
	g_return_val_if_fail(clutter_event_type(inEvent)==CLUTTER_KEY_PRESS || clutter_event_type(inEvent)==CLUTTER_KEY_RELEASE, CLUTTER_EVENT_PROPAGATE);
	g_return_val_if_fail(!inFocusable || XFCE_OVERVIEW_WORKSPACES_IS_FOCUSABLE(inFocusable), CLUTTER_EVENT_PROPAGATE);

	priv=self->priv;

	/* If no focusable actor was specified then use current focused actor */
	if(!inFocusable)
	{
		inFocusable=priv->currentFocus;

		/* If still no focusable actor is available we cannot handle event
		 * so let the others try it by propagating event.
		 */
		if(!inFocusable) return(CLUTTER_EVENT_PROPAGATE);
	}

	/* Get targets and action for this event and synthesize event for specified
	 * focusable actor
	 */
	targetFocusables=NULL;
	actionFullName=NULL;
	if(xfce_overview_workspaces_focus_manager_get_event_targets_and_action(self, inEvent, inFocusable, &targetFocusables, &actionFullName))
	{
		gboolean				eventStatus;
		gchar					**splitAction;
		const gchar				*action;
		const gchar				*detail;

		eventStatus=CLUTTER_EVENT_PROPAGATE;
		action=NULL;
		detail=NULL;

		/* Split action into action name and detail */
		splitAction=g_strsplit(actionFullName, "::", 2);
		if(!splitAction || !splitAction[0])
		{
			g_warning("Failed to split action '%s' into name and detail", actionFullName);

			/* Release list of targets to pervent emitting action of binding as
			 * splitting action into name and (maybe) detail failed.
			 */
			g_slist_free_full(targetFocusables, g_object_unref);
			targetFocusables=NULL;
		}
		action=splitAction[0];
		detail=splitAction[1];

		/* Emit action of binding to each actor in target list just build up */
		XFCE_OVERVIEW_WORKSPACES_DEBUG(self, MISC,
							"Target list for action '%s' has %d actors",
							action,
							g_slist_length(targetFocusables));

		eventStatus=_emit_action_to_targets(self, targetFocusables, action, detail, inFocusable, inEvent);

		/* Release allocated resources */
		g_strfreev(splitAction);
		g_slist_free_full(targetFocusables, g_object_unref);

		if(eventStatus==CLUTTER_EVENT_STOP) return(CLUTTER_EVENT_STOP);
	}

	/* Event was not handled so synthesize event to specified focusable actor */
	return(clutter_actor_event(CLUTTER_ACTOR(inFocusable), inEvent, FALSE));
}
