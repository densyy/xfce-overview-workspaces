/*
 * utils: Common functions, helpers and definitions
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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_UTILS__
#define __LIBXFCE_OVERVIEW_WORKSPACES_UTILS__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <clutter/clutter.h>
#include <gio/gio.h>

#include <libxfce-overview-workspaces/window-tracker-workspace.h>
#include <libxfce-overview-workspaces/stage-interface.h>
#include <libxfce-overview-workspaces/stage.h>
#include <libxfce-overview-workspaces/css-selector.h>

G_BEGIN_DECLS

/* Debug macros */
#define _DEBUG_OBJECT_NAME(x) \
	((x)!=NULL ? G_OBJECT_TYPE_NAME((x)) : "<nil>")

#define _DEBUG_BOX(msg, box) \
	g_message("%s: %s: x1=%.2f, y1=%.2f, x2=%.2f, y2=%.2f [%.2fx%.2f]", \
				__func__, \
				msg, \
				(box).x1, (box).y1, (box).x2, (box).y2, \
				(box).x2-(box).x1, (box).y2-(box).y1)

#define _DEBUG_NOTIFY(self, property, format, ...) \
	g_message("%s: Property '%s' of %p (%s) changed to "format, \
				__func__, \
				property, \
				(self), (self) ? G_OBJECT_TYPE_NAME((self)) : "<nil>", \
				## __VA_ARGS__)

/* Gobject type of pointer array (GPtrArray) */
#define XFCE_OVERVIEW_WORKSPACES_TYPE_POINTER_ARRAY		(xfce_overview_workspaces_pointer_array_get_type())

/* Public API */

/**
 * GTYPE_TO_POINTER:
 * @gtype: A #GType to stuff into the pointer
 *
 * Stuffs the #GType specified at @gtype into a pointer type.
 */
#ifdef GTYPE_TO_POINTER /* added in glib 2.80 */
#undef GTYPE_TO_POINTER
#endif
#define GTYPE_TO_POINTER(gtype) \
	(GSIZE_TO_POINTER(gtype))

/**
 * GPOINTER_TO_GTYPE:
 * @pointer: A pointer to extract a #GType from
 *
 * Extracts a #GType from a pointer. The #GType must have been stored in the pointer with GTYPE_TO_POINTER().
 */
#ifdef GPOINTER_TO_GTYPE /* added in glib 2.80 */
#undef GPOINTER_TO_GTYPE
#endif
#define GPOINTER_TO_GTYPE(pointer) \
	((GType)GPOINTER_TO_SIZE(pointer))

void xfce_overview_workspaces_notify(ClutterActor *inSender,
							const gchar *inIconName,
							const gchar *inFormat, ...)
							G_GNUC_PRINTF(3, 4);

GAppLaunchContext* xfce_overview_workspaces_create_app_context(XfceOverviewWorkspacesWindowTrackerWorkspace *inWorkspace);

ClutterActor* xfce_overview_workspaces_find_actor_by_name(ClutterActor *inActor, const gchar *inName);

/**
 * XfceOverviewWorkspacesTraversalCallback:
 * @inActor: The actor currently processed and has matched the selector in traversal
 * @inUserData: Data passed to the function, set with xfce_overview_workspaces_traverse_actor()
 *
 * A callback called each time an actor matches the provided css selector
 * in xfce_overview_workspaces_traverse_actor().
 *
 * Returns: %FALSE if the traversal should be stopped. #XFCE_OVERVIEW_WORKSPACES_TRAVERSAL_STOP
 * and #XFCE_OVERVIEW_WORKSPACES_TRAVERSAL_CONTINUE are more memorable names for the return value.
 */
typedef gboolean (*XfceOverviewWorkspacesTraversalCallback)(ClutterActor *inActor, gpointer inUserData);

/**
 * XFCE_OVERVIEW_WORKSPACES_TRAVERSAL_STOP:
 *
 * Use this macro as the return value of a #XfceOverviewWorkspacesTraversalCallback to stop
 * further traversal in xfce_overview_workspaces_traverse_actor().
 */
#define XFCE_OVERVIEW_WORKSPACES_TRAVERSAL_STOP			(FALSE)

/**
 * XFCE_OVERVIEW_WORKSPACES_TRAVERSAL_CONTINUE:
 *
 * Use this macro as the return value of a #XfceOverviewWorkspacesTraversalCallback to continue
 * further traversal in xfce_overview_workspaces_traverse_actor().
 */
#define XFCE_OVERVIEW_WORKSPACES_TRAVERSAL_CONTINUE		(TRUE)

void xfce_overview_workspaces_traverse_actor(ClutterActor *inRootActor,
								XfceOverviewWorkspacesCssSelector *inSelector,
								XfceOverviewWorkspacesTraversalCallback inCallback,
								gpointer inUserData);

XfceOverviewWorkspacesStageInterface* xfce_overview_workspaces_get_stage_of_actor(ClutterActor *inActor);

gchar** xfce_overview_workspaces_split_string(const gchar *inString, const gchar *inDelimiters);

gboolean xfce_overview_workspaces_is_valid_id(const gchar *inString);

gchar* xfce_overview_workspaces_get_enum_value_name(GType inEnumClass, gint inValue);
gint xfce_overview_workspaces_get_enum_value_from_nickname(GType inEnumClass, const gchar *inNickname);

void xfce_overview_workspaces_dump_actor(ClutterActor *inActor);

gboolean xfce_overview_workspaces_strv_equal(const gchar **inLeft, const gchar **inRight);

G_END_DECLS

#endif	/* __LIBXFCE_OVERVIEW_WORKSPACES_UTILS__ */
