/*
 * viewpad: A viewpad managing views
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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_VIEWPAD__
#define __LIBXFCE_OVERVIEW_WORKSPACES_VIEWPAD__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <clutter/clutter.h>

#include <libxfce-overview-workspaces/background.h>
#include <libxfce-overview-workspaces/view.h>
#include <libxfce-overview-workspaces/types.h>

G_BEGIN_DECLS

#define XFCE_OVERVIEW_WORKSPACES_TYPE_VIEWPAD				(xfce_overview_workspaces_viewpad_get_type())
#define XFCE_OVERVIEW_WORKSPACES_VIEWPAD(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_VIEWPAD, XfceOverviewWorkspacesViewpad))
#define XFCE_OVERVIEW_WORKSPACES_IS_VIEWPAD(obj)				(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_VIEWPAD))
#define XFCE_OVERVIEW_WORKSPACES_VIEWPAD_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_VIEWPAD, XfceOverviewWorkspacesViewpadClass))
#define XFCE_OVERVIEW_WORKSPACES_IS_VIEWPAD_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_VIEWPAD))
#define XFCE_OVERVIEW_WORKSPACES_VIEWPAD_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_VIEWPAD, XfceOverviewWorkspacesViewpadClass))

typedef struct _XfceOverviewWorkspacesViewpad				XfceOverviewWorkspacesViewpad; 
typedef struct _XfceOverviewWorkspacesViewpadPrivate		XfceOverviewWorkspacesViewpadPrivate;
typedef struct _XfceOverviewWorkspacesViewpadClass			XfceOverviewWorkspacesViewpadClass;

struct _XfceOverviewWorkspacesViewpad
{
	/*< private >*/
	/* Parent instance */
	XfceOverviewWorkspacesBackground		parent_instance;

	/* Private structure */
	XfceOverviewWorkspacesViewpadPrivate	*priv;
};

struct _XfceOverviewWorkspacesViewpadClass
{
	/*< private >*/
	/* Parent class */
	XfceOverviewWorkspacesBackgroundClass	parent_class;

	/*< public >*/
	/* Virtual functions */
	void (*view_added)(XfceOverviewWorkspacesViewpad *self, XfceOverviewWorkspacesView *inView);
	void (*view_removed)(XfceOverviewWorkspacesViewpad *self, XfceOverviewWorkspacesView *inView);

	void (*view_activating)(XfceOverviewWorkspacesViewpad *self, XfceOverviewWorkspacesView *inView);
	void (*view_activated)(XfceOverviewWorkspacesViewpad *self, XfceOverviewWorkspacesView *inView);
	void (*view_deactivating)(XfceOverviewWorkspacesViewpad *self, XfceOverviewWorkspacesView *inView);
	void (*view_deactivated)(XfceOverviewWorkspacesViewpad *self, XfceOverviewWorkspacesView *inView);
};

/* Public API */
GType xfce_overview_workspaces_viewpad_get_type(void) G_GNUC_CONST;

ClutterActor* xfce_overview_workspaces_viewpad_new(void);

gfloat xfce_overview_workspaces_viewpad_get_spacing(XfceOverviewWorkspacesViewpad *self);
void xfce_overview_workspaces_viewpad_set_spacing(XfceOverviewWorkspacesViewpad *self, gfloat inSpacing);

GList* xfce_overview_workspaces_viewpad_get_views(XfceOverviewWorkspacesViewpad *self);
gboolean xfce_overview_workspaces_viewpad_has_view(XfceOverviewWorkspacesViewpad *self, XfceOverviewWorkspacesView *inView);
XfceOverviewWorkspacesView* xfce_overview_workspaces_viewpad_find_view_by_type(XfceOverviewWorkspacesViewpad *self, GType inType);
XfceOverviewWorkspacesView* xfce_overview_workspaces_viewpad_find_view_by_id(XfceOverviewWorkspacesViewpad *self, const gchar *inID);

XfceOverviewWorkspacesView* xfce_overview_workspaces_viewpad_get_active_view(XfceOverviewWorkspacesViewpad *self);
void xfce_overview_workspaces_viewpad_set_active_view(XfceOverviewWorkspacesViewpad *self, XfceOverviewWorkspacesView *inView);

gboolean xfce_overview_workspaces_viewpad_get_horizontal_scrollbar_visible(XfceOverviewWorkspacesViewpad *self);
gboolean xfce_overview_workspaces_viewpad_get_vertical_scrollbar_visible(XfceOverviewWorkspacesViewpad *self);

XfceOverviewWorkspacesVisibilityPolicy xfce_overview_workspaces_viewpad_get_horizontal_scrollbar_policy(XfceOverviewWorkspacesViewpad *self);
void xfce_overview_workspaces_viewpad_set_horizontal_scrollbar_policy(XfceOverviewWorkspacesViewpad *self, XfceOverviewWorkspacesVisibilityPolicy inPolicy);

XfceOverviewWorkspacesVisibilityPolicy xfce_overview_workspaces_viewpad_get_vertical_scrollbar_policy(XfceOverviewWorkspacesViewpad *self);
void xfce_overview_workspaces_viewpad_set_vertical_scrollbar_policy(XfceOverviewWorkspacesViewpad *self, XfceOverviewWorkspacesVisibilityPolicy inPolicy);

G_END_DECLS

#endif	/* __LIBXFCE_OVERVIEW_WORKSPACES_VIEWPAD__ */
