/*
 * scrollbar: A scroll bar
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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_SCROLLBAR__
#define __LIBXFCE_OVERVIEW_WORKSPACES_SCROLLBAR__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <clutter/clutter.h>

#include <libxfce-overview-workspaces/background.h>

G_BEGIN_DECLS

#define XFCE_OVERVIEW_WORKSPACES_TYPE_SCROLLBAR				(xfce_overview_workspaces_scrollbar_get_type())
#define XFCE_OVERVIEW_WORKSPACES_SCROLLBAR(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_SCROLLBAR, XfceOverviewWorkspacesScrollbar))
#define XFCE_OVERVIEW_WORKSPACES_IS_SCROLLBAR(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_SCROLLBAR))
#define XFCE_OVERVIEW_WORKSPACES_SCROLLBAR_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_SCROLLBAR, XfceOverviewWorkspacesScrollbarClass))
#define XFCE_OVERVIEW_WORKSPACES_IS_SCROLLBAR_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_SCROLLBAR))
#define XFCE_OVERVIEW_WORKSPACES_SCROLLBAR_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_SCROLLBAR, XfceOverviewWorkspacesScrollbarClass))

typedef struct _XfceOverviewWorkspacesScrollbar			XfceOverviewWorkspacesScrollbar; 
typedef struct _XfceOverviewWorkspacesScrollbarPrivate		XfceOverviewWorkspacesScrollbarPrivate;
typedef struct _XfceOverviewWorkspacesScrollbarClass		XfceOverviewWorkspacesScrollbarClass;

struct _XfceOverviewWorkspacesScrollbar
{
	/*< private >*/
	/* Parent instance */
	XfceOverviewWorkspacesBackground			parent_instance;

	/* Private structure */
	XfceOverviewWorkspacesScrollbarPrivate		*priv;
};

struct _XfceOverviewWorkspacesScrollbarClass
{
	/*< private >*/
	/* Parent class */
	XfceOverviewWorkspacesBackgroundClass		parent_class;

	/*< public >*/
	/* Virtual functions */
	void (*value_changed)(XfceOverviewWorkspacesScrollbar *self, gfloat inValue);
};

/* Public API */
GType xfce_overview_workspaces_scrollbar_get_type(void) G_GNUC_CONST;

ClutterActor* xfce_overview_workspaces_scrollbar_new(ClutterOrientation inOrientation);

gfloat xfce_overview_workspaces_scrollbar_get_orientation(XfceOverviewWorkspacesScrollbar *self);
void xfce_overview_workspaces_scrollbar_set_orientation(XfceOverviewWorkspacesScrollbar *self, ClutterOrientation inOrientation);

gfloat xfce_overview_workspaces_scrollbar_get_value(XfceOverviewWorkspacesScrollbar *self);
void xfce_overview_workspaces_scrollbar_set_value(XfceOverviewWorkspacesScrollbar *self, gfloat inValue);

gfloat xfce_overview_workspaces_scrollbar_get_value_range(XfceOverviewWorkspacesScrollbar *self);

gfloat xfce_overview_workspaces_scrollbar_get_range(XfceOverviewWorkspacesScrollbar *self);
void xfce_overview_workspaces_scrollbar_set_range(XfceOverviewWorkspacesScrollbar *self, gfloat inRange);

gfloat xfce_overview_workspaces_scrollbar_get_page_size_factor(XfceOverviewWorkspacesScrollbar *self);
void xfce_overview_workspaces_scrollbar_set_page_size_factor(XfceOverviewWorkspacesScrollbar *self, gfloat inFactor);

gfloat xfce_overview_workspaces_scrollbar_get_spacing(XfceOverviewWorkspacesScrollbar *self);
void xfce_overview_workspaces_scrollbar_set_spacing(XfceOverviewWorkspacesScrollbar *self, gfloat inSpacing);

gfloat xfce_overview_workspaces_scrollbar_get_slider_width(XfceOverviewWorkspacesScrollbar *self);
void xfce_overview_workspaces_scrollbar_set_slider_width(XfceOverviewWorkspacesScrollbar *self, gfloat inWidth);

gfloat xfce_overview_workspaces_scrollbar_get_slider_radius(XfceOverviewWorkspacesScrollbar *self);
void xfce_overview_workspaces_scrollbar_set_slider_radius(XfceOverviewWorkspacesScrollbar *self, gfloat inRadius);

const ClutterColor* xfce_overview_workspaces_scrollbar_get_slider_color(XfceOverviewWorkspacesScrollbar *self);
void xfce_overview_workspaces_scrollbar_set_slider_color(XfceOverviewWorkspacesScrollbar *self, const ClutterColor *inColor);

G_END_DECLS

#endif	/* __LIBXFCE_OVERVIEW_WORKSPACES_SCROLLBAR__ */
