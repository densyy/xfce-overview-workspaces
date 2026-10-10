/*
 * view: Abstract class for views, optional with scrollbars
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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_VIEW__
#define __LIBXFCE_OVERVIEW_WORKSPACES_VIEW__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <clutter/clutter.h>

#include <libxfce-overview-workspaces/actor.h>
#include <libxfce-overview-workspaces/types.h>
#include <libxfce-overview-workspaces/focusable.h>

G_BEGIN_DECLS

/* Public definitions */
/**
 * XfceOverviewWorkspacesViewFitMode:
 * @XFCE_OVERVIEW_WORKSPACES_VIEW_FIT_MODE_NONE: Do not try to fit view into viewpad.
 * @XFCE_OVERVIEW_WORKSPACES_VIEW_FIT_MODE_HORIZONTAL: Try to fit view into viewpad horizontally.
 * @XFCE_OVERVIEW_WORKSPACES_VIEW_FIT_MODE_VERTICAL: Try to fit view into viewpad vertically.
 * @XFCE_OVERVIEW_WORKSPACES_VIEW_FIT_MODE_BOTH: Try to fit view into viewpad horizontally and vertically.
 *
 * Determines how a view should fit into a viewpad.
 */
typedef enum /*< prefix=XFCE_OVERVIEW_WORKSPACES_VIEW_FIT_MODE >*/
{
	XFCE_OVERVIEW_WORKSPACES_VIEW_FIT_MODE_NONE=0,
	XFCE_OVERVIEW_WORKSPACES_VIEW_FIT_MODE_HORIZONTAL,
	XFCE_OVERVIEW_WORKSPACES_VIEW_FIT_MODE_VERTICAL,
	XFCE_OVERVIEW_WORKSPACES_VIEW_FIT_MODE_BOTH
} XfceOverviewWorkspacesViewFitMode;


/* Object declaration */
#define XFCE_OVERVIEW_WORKSPACES_TYPE_VIEW				(xfce_overview_workspaces_view_get_type())
#define XFCE_OVERVIEW_WORKSPACES_VIEW(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_VIEW, XfceOverviewWorkspacesView))
#define XFCE_OVERVIEW_WORKSPACES_IS_VIEW(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_VIEW))
#define XFCE_OVERVIEW_WORKSPACES_VIEW_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_VIEW, XfceOverviewWorkspacesViewClass))
#define XFCE_OVERVIEW_WORKSPACES_IS_VIEW_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_VIEW))
#define XFCE_OVERVIEW_WORKSPACES_VIEW_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_VIEW, XfceOverviewWorkspacesViewClass))

typedef struct _XfceOverviewWorkspacesView				XfceOverviewWorkspacesView; 
typedef struct _XfceOverviewWorkspacesViewPrivate		XfceOverviewWorkspacesViewPrivate;
typedef struct _XfceOverviewWorkspacesViewClass		XfceOverviewWorkspacesViewClass;

struct _XfceOverviewWorkspacesView
{
	/*< private >*/
	/* Parent instance */
	XfceOverviewWorkspacesActor			parent_instance;

	/* Private structure */
	XfceOverviewWorkspacesViewPrivate		*priv;
};

struct _XfceOverviewWorkspacesViewClass
{
	/*< private >*/
	/* Parent class */
	XfceOverviewWorkspacesActorClass		parent_class;

	/*< public >*/
	/* Virtual functions */
	void (*set_view_fit_mode)(XfceOverviewWorkspacesView *self, XfceOverviewWorkspacesViewFitMode inFitMode);

	void (*activating)(XfceOverviewWorkspacesView *self);
	void (*activated)(XfceOverviewWorkspacesView *self);
	void (*deactivating)(XfceOverviewWorkspacesView *self);
	void (*deactivated)(XfceOverviewWorkspacesView *self);

	void (*enabling)(XfceOverviewWorkspacesView *self);
	void (*enabled)(XfceOverviewWorkspacesView *self);
	void (*disabling)(XfceOverviewWorkspacesView *self);
	void (*disabled)(XfceOverviewWorkspacesView *self);

	void (*name_changed)(XfceOverviewWorkspacesView *self, gchar *inName);
	void (*icon_changed)(XfceOverviewWorkspacesView *self, ClutterImage *inIcon);

	void (*scroll_to)(XfceOverviewWorkspacesView *self, gfloat inX, gfloat inY);
	gboolean (*child_needs_scroll)(XfceOverviewWorkspacesView *self, ClutterActor *inActor);
	void (*child_ensure_visible)(XfceOverviewWorkspacesView *self, ClutterActor *inActor);

	/* Binding actions */
	gboolean (*view_activate)(XfceOverviewWorkspacesView *self,
								XfceOverviewWorkspacesFocusable *inSource,
								const gchar *inAction,
								ClutterEvent *inEvent);
};


/* Public API */
GType xfce_overview_workspaces_view_get_type(void) G_GNUC_CONST;

const gchar* xfce_overview_workspaces_view_get_id(XfceOverviewWorkspacesView *self);
gboolean xfce_overview_workspaces_view_has_id(XfceOverviewWorkspacesView *self, const gchar *inID);

const gchar* xfce_overview_workspaces_view_get_name(XfceOverviewWorkspacesView *self);
void xfce_overview_workspaces_view_set_name(XfceOverviewWorkspacesView *self, const gchar *inName);

const gchar* xfce_overview_workspaces_view_get_icon(XfceOverviewWorkspacesView *self);
void xfce_overview_workspaces_view_set_icon(XfceOverviewWorkspacesView *self, const gchar *inIcon);

XfceOverviewWorkspacesViewFitMode xfce_overview_workspaces_view_get_view_fit_mode(XfceOverviewWorkspacesView *self);
void xfce_overview_workspaces_view_set_view_fit_mode(XfceOverviewWorkspacesView *self, XfceOverviewWorkspacesViewFitMode inFitMode);

gboolean xfce_overview_workspaces_view_get_enabled(XfceOverviewWorkspacesView *self);
void xfce_overview_workspaces_view_set_enabled(XfceOverviewWorkspacesView *self, gboolean inIsEnabled);

void xfce_overview_workspaces_view_scroll_to(XfceOverviewWorkspacesView *self, gfloat inX, gfloat inY);
gboolean xfce_overview_workspaces_view_child_needs_scroll(XfceOverviewWorkspacesView *self, ClutterActor *inActor);
void xfce_overview_workspaces_view_child_ensure_visible(XfceOverviewWorkspacesView *self, ClutterActor *inActor);

gboolean xfce_overview_workspaces_view_has_focus(XfceOverviewWorkspacesView *self);

G_END_DECLS

#endif	/* __LIBXFCE_OVERVIEW_WORKSPACES_VIEW__ */
