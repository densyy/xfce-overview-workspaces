/*
 * stylable: An interface which can be inherited by actor and objects
 *           to get styled by a theme
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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_STYLABLE__
#define __LIBXFCE_OVERVIEW_WORKSPACES_STYLABLE__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <glib-object.h>

#include <libxfce-overview-workspaces/focusable.h>

G_BEGIN_DECLS

#define XFCE_OVERVIEW_WORKSPACES_TYPE_STYLABLE				(xfce_overview_workspaces_stylable_get_type())
#define XFCE_OVERVIEW_WORKSPACES_STYLABLE(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_STYLABLE, XfceOverviewWorkspacesStylable))
#define XFCE_OVERVIEW_WORKSPACES_IS_STYLABLE(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_STYLABLE))
#define XFCE_OVERVIEW_WORKSPACES_STYLABLE_GET_IFACE(obj)		(G_TYPE_INSTANCE_GET_INTERFACE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_STYLABLE, XfceOverviewWorkspacesStylableInterface))

typedef struct _XfceOverviewWorkspacesStylable				XfceOverviewWorkspacesStylable;
typedef struct _XfceOverviewWorkspacesStylableInterface	XfceOverviewWorkspacesStylableInterface;

struct _XfceOverviewWorkspacesStylableInterface
{
	/*< private >*/
	/* Parent interface */
	GTypeInterface				parent_interface;

	/*< public >*/
	/* Virtual functions */
	void (*get_stylable_properties)(XfceOverviewWorkspacesStylable *self, GHashTable *ioStylableProperties);

	const gchar* (*get_name)(XfceOverviewWorkspacesStylable *self);

	XfceOverviewWorkspacesStylable* (*get_parent)(XfceOverviewWorkspacesStylable *self);

	const gchar* (*get_classes)(XfceOverviewWorkspacesStylable *self);
	void (*set_classes)(XfceOverviewWorkspacesStylable *self, const gchar *inClasses);
	void (*class_added)(XfceOverviewWorkspacesStylable *self, const gchar *inClass);
	void (*class_removed)(XfceOverviewWorkspacesStylable *self, const gchar *inClass);

	const gchar* (*get_pseudo_classes)(XfceOverviewWorkspacesStylable *self);
	void (*set_pseudo_classes)(XfceOverviewWorkspacesStylable *self, const gchar *inClasses);
	void (*pseudo_class_added)(XfceOverviewWorkspacesStylable *self, const gchar *inClass);
	void (*pseudo_class_removed)(XfceOverviewWorkspacesStylable *self, const gchar *inClass);

	void (*invalidate)(XfceOverviewWorkspacesStylable *self);

	/* Binding actions */
	gboolean (*add_classes)(XfceOverviewWorkspacesStylable *self,
							XfceOverviewWorkspacesFocusable *inSource,
							const gchar *inAction,
							const gchar *inDetail,
							ClutterEvent *inEvent);
	gboolean (*remove_classes)(XfceOverviewWorkspacesStylable *self,
								XfceOverviewWorkspacesFocusable *inSource,
								const gchar *inAction,
								const gchar *inDetail,
								ClutterEvent *inEvent);
	gboolean (*toggle_classes)(XfceOverviewWorkspacesStylable *self,
								XfceOverviewWorkspacesFocusable *inSource,
								const gchar *inAction,
								const gchar *inDetail,
								ClutterEvent *inEvent);
	gboolean (*apply_classes)(XfceOverviewWorkspacesStylable *self,
								XfceOverviewWorkspacesFocusable *inSource,
								const gchar *inAction,
								const gchar *inDetail,
								ClutterEvent *inEvent);
};

/* Public API */
GType xfce_overview_workspaces_stylable_get_type(void) G_GNUC_CONST;

GHashTable* xfce_overview_workspaces_stylable_get_stylable_properties(XfceOverviewWorkspacesStylable *self);
gboolean xfce_overview_workspaces_stylable_add_stylable_property(XfceOverviewWorkspacesStylable *self,
													GHashTable *ioStylableProperties,
													const gchar *inProperty);

const gchar* xfce_overview_workspaces_stylable_get_name(XfceOverviewWorkspacesStylable *self);

XfceOverviewWorkspacesStylable* xfce_overview_workspaces_stylable_get_parent(XfceOverviewWorkspacesStylable *self);

const gchar* xfce_overview_workspaces_stylable_get_classes(XfceOverviewWorkspacesStylable *self);
void xfce_overview_workspaces_stylable_set_classes(XfceOverviewWorkspacesStylable *self, const gchar *inClasses);
gboolean xfce_overview_workspaces_stylable_has_class(XfceOverviewWorkspacesStylable *self, const gchar *inClass);
void xfce_overview_workspaces_stylable_add_class(XfceOverviewWorkspacesStylable *self, const gchar *inClass);
void xfce_overview_workspaces_stylable_remove_class(XfceOverviewWorkspacesStylable *self, const gchar *inClass);

const gchar* xfce_overview_workspaces_stylable_get_pseudo_classes(XfceOverviewWorkspacesStylable *self);
void xfce_overview_workspaces_stylable_set_pseudo_classes(XfceOverviewWorkspacesStylable *self, const gchar *inClasses);
gboolean xfce_overview_workspaces_stylable_has_pseudo_class(XfceOverviewWorkspacesStylable *self, const gchar *inClass);
void xfce_overview_workspaces_stylable_add_pseudo_class(XfceOverviewWorkspacesStylable *self, const gchar *inClass);
void xfce_overview_workspaces_stylable_remove_pseudo_class(XfceOverviewWorkspacesStylable *self, const gchar *inClass);

void xfce_overview_workspaces_stylable_invalidate(XfceOverviewWorkspacesStylable *self);

G_END_DECLS

#endif	/* __LIBXFCE_OVERVIEW_WORKSPACES_STYLABLE__ */
