/*
 * binding: A keyboard or pointer binding
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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_BINDING__
#define __LIBXFCE_OVERVIEW_WORKSPACES_BINDING__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <clutter/clutter.h>

G_BEGIN_DECLS

#define XFCE_OVERVIEW_WORKSPACES_TYPE_BINDING				(xfce_overview_workspaces_binding_get_type())
#define XFCE_OVERVIEW_WORKSPACES_BINDING(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_BINDING, XfceOverviewWorkspacesBinding))
#define XFCE_OVERVIEW_WORKSPACES_IS_BINDING(obj)				(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_BINDING))
#define XFCE_OVERVIEW_WORKSPACES_BINDING_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_BINDING, XfceOverviewWorkspacesBindingClass))
#define XFCE_OVERVIEW_WORKSPACES_IS_BINDING_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_BINDING))
#define XFCE_OVERVIEW_WORKSPACES_BINDING_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_BINDING, XfceOverviewWorkspacesBindingClass))

typedef struct _XfceOverviewWorkspacesBinding				XfceOverviewWorkspacesBinding;
typedef struct _XfceOverviewWorkspacesBindingClass			XfceOverviewWorkspacesBindingClass;
typedef struct _XfceOverviewWorkspacesBindingPrivate		XfceOverviewWorkspacesBindingPrivate;

struct _XfceOverviewWorkspacesBinding
{
	/*< private >*/
	/* Parent instance */
	GObject							parent_instance;

	/* Private structure */
	XfceOverviewWorkspacesBindingPrivate		*priv;
};

struct _XfceOverviewWorkspacesBindingClass
{
	/*< private >*/
	/* Parent class */
	GObjectClass					parent_class;
};

/* Public API */
#define XFCE_OVERVIEW_WORKSPACES_BINDING_MODIFIERS_MASK		(CLUTTER_SHIFT_MASK | \
													CLUTTER_CONTROL_MASK | \
													CLUTTER_MOD1_MASK | \
													CLUTTER_MOD2_MASK | \
													CLUTTER_MOD3_MASK | \
													CLUTTER_MOD4_MASK | \
													CLUTTER_MOD5_MASK | \
													CLUTTER_SUPER_MASK | \
													CLUTTER_HYPER_MASK | \
													CLUTTER_META_MASK)

typedef enum /*< flags,prefix=XFCE_OVERVIEW_WORKSPACES_BINDING_FLAGS >*/
{
	XFCE_OVERVIEW_WORKSPACES_BINDING_FLAGS_NONE=0,
	XFCE_OVERVIEW_WORKSPACES_BINDING_FLAGS_ALLOW_UNFOCUSABLE_TARGET=1 << 0,
} XfceOverviewWorkspacesBindingFlags;

GType xfce_overview_workspaces_binding_get_type(void) G_GNUC_CONST;

XfceOverviewWorkspacesBinding* xfce_overview_workspaces_binding_new(void);
XfceOverviewWorkspacesBinding* xfce_overview_workspaces_binding_new_for_event(const ClutterEvent *inEvent);

guint xfce_overview_workspaces_binding_hash(gconstpointer inValue);
gboolean xfce_overview_workspaces_binding_compare(gconstpointer inLeft, gconstpointer inRight);

ClutterEventType xfce_overview_workspaces_binding_get_event_type(const XfceOverviewWorkspacesBinding *self);
void xfce_overview_workspaces_binding_set_event_type(XfceOverviewWorkspacesBinding *self, ClutterEventType inType);

const gchar* xfce_overview_workspaces_binding_get_class_name(const XfceOverviewWorkspacesBinding *self);
void xfce_overview_workspaces_binding_set_class_name(XfceOverviewWorkspacesBinding *self, const gchar *inClassName);

guint xfce_overview_workspaces_binding_get_key(const XfceOverviewWorkspacesBinding *self);
void xfce_overview_workspaces_binding_set_key(XfceOverviewWorkspacesBinding *self, guint inKey);

ClutterModifierType xfce_overview_workspaces_binding_get_modifiers(const XfceOverviewWorkspacesBinding *self);
void xfce_overview_workspaces_binding_set_modifiers(XfceOverviewWorkspacesBinding *self, ClutterModifierType inModifiers);

const gchar* xfce_overview_workspaces_binding_get_target(const XfceOverviewWorkspacesBinding *self);
void xfce_overview_workspaces_binding_set_target(XfceOverviewWorkspacesBinding *self, const gchar *inTarget);

const gchar* xfce_overview_workspaces_binding_get_action(const XfceOverviewWorkspacesBinding *self);
void xfce_overview_workspaces_binding_set_action(XfceOverviewWorkspacesBinding *self, const gchar *inAction);

XfceOverviewWorkspacesBindingFlags xfce_overview_workspaces_binding_get_flags(const XfceOverviewWorkspacesBinding *self);
void xfce_overview_workspaces_binding_set_flags(XfceOverviewWorkspacesBinding *self, XfceOverviewWorkspacesBindingFlags inFlags);

G_END_DECLS

#endif	/* __LIBXFCE_OVERVIEW_WORKSPACES_BINDING__ */
