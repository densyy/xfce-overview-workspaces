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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_ACTOR__
#define __LIBXFCE_OVERVIEW_WORKSPACES_ACTOR__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <clutter/clutter.h>

G_BEGIN_DECLS

#define XFCE_OVERVIEW_WORKSPACES_TYPE_ACTOR					(xfce_overview_workspaces_actor_get_type())
#define XFCE_OVERVIEW_WORKSPACES_ACTOR(obj)					(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_ACTOR, XfceOverviewWorkspacesActor))
#define XFCE_OVERVIEW_WORKSPACES_IS_ACTOR(obj)				(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_ACTOR))
#define XFCE_OVERVIEW_WORKSPACES_ACTOR_CLASS(klass)			(G_TYPE_CHECK_CLASS_CAST((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_ACTOR, XfceOverviewWorkspacesActorClass))
#define XFCE_OVERVIEW_WORKSPACES_IS_ACTOR_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_ACTOR))
#define XFCE_OVERVIEW_WORKSPACES_ACTOR_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_ACTOR, XfceOverviewWorkspacesActorClass))

typedef struct _XfceOverviewWorkspacesActor				XfceOverviewWorkspacesActor;
typedef struct _XfceOverviewWorkspacesActorClass			XfceOverviewWorkspacesActorClass;
typedef struct _XfceOverviewWorkspacesActorPrivate			XfceOverviewWorkspacesActorPrivate;

struct _XfceOverviewWorkspacesActor
{
	/*< private >*/
	/* Parent instance */
	ClutterActor				parent_instance;

	/* Private structure */
	XfceOverviewWorkspacesActorPrivate		*priv;
};

struct _XfceOverviewWorkspacesActorClass
{
	/*< private >*/
	/* Parent class */
	ClutterActorClass			parent_class;

	/*< public >*/
	/* Virtual functions */
};

/* Public API */
GType xfce_overview_workspaces_actor_get_type(void) G_GNUC_CONST;

ClutterActor* xfce_overview_workspaces_actor_new(void);

gboolean xfce_overview_workspaces_actor_get_can_focus(XfceOverviewWorkspacesActor *self);
void xfce_overview_workspaces_actor_set_can_focus(XfceOverviewWorkspacesActor *self, gboolean inCanFous);

const gchar* xfce_overview_workspaces_actor_get_effects(XfceOverviewWorkspacesActor *self);
void xfce_overview_workspaces_actor_set_effects(XfceOverviewWorkspacesActor *self, const gchar *inEffects);

gboolean xfce_overview_workspaces_actor_get_visibility(XfceOverviewWorkspacesActor *self);
void xfce_overview_workspaces_actor_set_visibility(XfceOverviewWorkspacesActor *self, gboolean inVisibility);

void xfce_overview_workspaces_actor_install_stylable_property(XfceOverviewWorkspacesActorClass *klass, GParamSpec *inParamSpec);
void xfce_overview_workspaces_actor_install_stylable_property_by_name(XfceOverviewWorkspacesActorClass *klass, const gchar *inParamName);
GHashTable* xfce_overview_workspaces_actor_get_stylable_properties(XfceOverviewWorkspacesActorClass *klass);
GHashTable* xfce_overview_workspaces_actor_get_stylable_properties_full(XfceOverviewWorkspacesActorClass *klass);

void xfce_overview_workspaces_actor_invalidate(XfceOverviewWorkspacesActor *self);

void xfce_overview_workspaces_actor_enable_allocation_animation_once(XfceOverviewWorkspacesActor *self);
void xfce_overview_workspaces_actor_get_allocation_box(XfceOverviewWorkspacesActor *self, ClutterActorBox *outAllocationBox);

gboolean xfce_overview_workspaces_actor_destroy(ClutterActor *self);
void xfce_overview_workspaces_actor_destroy_all_children(ClutterActor *self);
gboolean xfce_overview_workspaces_actor_iter_destroy(ClutterActorIter *self);

G_END_DECLS

#endif
