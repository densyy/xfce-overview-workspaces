/*
 * window-content: A content to share texture of a window
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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_WINDOW_CONTENT_X11__
#define __LIBXFCE_OVERVIEW_WORKSPACES_WINDOW_CONTENT_X11__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <clutter/clutter.h>

#include <libxfce-overview-workspaces/x11/window-tracker-window-x11.h>
#include <libxfce-overview-workspaces/window-tracker-window.h>
#include <libxfce-overview-workspaces/types.h>

G_BEGIN_DECLS

#define XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_CONTENT_X11				(xfce_overview_workspaces_window_content_x11_get_type())
#define XFCE_OVERVIEW_WORKSPACES_WINDOW_CONTENT_X11(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_CONTENT_X11, XfceOverviewWorkspacesWindowContentX11))
#define XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_CONTENT_X11(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_CONTENT_X11))
#define XFCE_OVERVIEW_WORKSPACES_WINDOW_CONTENT_X11_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_CONTENT_X11, XfceOverviewWorkspacesWindowContentX11Class))
#define XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_CONTENT_X11_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_CONTENT_X11))
#define XFCE_OVERVIEW_WORKSPACES_WINDOW_CONTENT_X11_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_CONTENT_X11, XfceOverviewWorkspacesWindowContentX11Class))

typedef struct _XfceOverviewWorkspacesWindowContentX11				XfceOverviewWorkspacesWindowContentX11;
typedef struct _XfceOverviewWorkspacesWindowContentX11Class		XfceOverviewWorkspacesWindowContentX11Class;
typedef struct _XfceOverviewWorkspacesWindowContentX11Private		XfceOverviewWorkspacesWindowContentX11Private;

struct _XfceOverviewWorkspacesWindowContentX11
{
	/*< private >*/
	/* Parent instance */
	GObject										parent_instance;

	/* Private structure */
	XfceOverviewWorkspacesWindowContentX11Private			*priv;
};

struct _XfceOverviewWorkspacesWindowContentX11Class
{
	/*< private >*/
	/* Parent class */
	GObjectClass								parent_class;

	/*< public >*/
	/* Virtual functions */
};

/* Public API */
GType xfce_overview_workspaces_window_content_x11_get_type(void) G_GNUC_CONST;

ClutterContent* xfce_overview_workspaces_window_content_x11_new_for_window(XfceOverviewWorkspacesWindowTrackerWindowX11 *inWindow);

XfceOverviewWorkspacesWindowTrackerWindow* xfce_overview_workspaces_window_content_x11_get_window(XfceOverviewWorkspacesWindowContentX11 *self);

gboolean xfce_overview_workspaces_window_content_x11_is_suspended(XfceOverviewWorkspacesWindowContentX11 *self);

const ClutterColor* xfce_overview_workspaces_window_content_x11_get_outline_color(XfceOverviewWorkspacesWindowContentX11 *self);
void xfce_overview_workspaces_window_content_x11_set_outline_color(XfceOverviewWorkspacesWindowContentX11 *self, const ClutterColor *inColor);

gfloat xfce_overview_workspaces_window_content_x11_get_outline_width(XfceOverviewWorkspacesWindowContentX11 *self);
void xfce_overview_workspaces_window_content_x11_set_outline_width(XfceOverviewWorkspacesWindowContentX11 *self, const gfloat inWidth);

gboolean xfce_overview_workspaces_window_content_x11_get_include_window_frame(XfceOverviewWorkspacesWindowContentX11 *self);
void xfce_overview_workspaces_window_content_x11_set_include_window_frame(XfceOverviewWorkspacesWindowContentX11 *self, const gboolean inIncludeFrame);

gboolean xfce_overview_workspaces_window_content_x11_get_unmapped_window_icon_x_fill(XfceOverviewWorkspacesWindowContentX11 *self);
void xfce_overview_workspaces_window_content_x11_set_unmapped_window_icon_x_fill(XfceOverviewWorkspacesWindowContentX11 *self, const gboolean inFill);

gboolean xfce_overview_workspaces_window_content_x11_get_unmapped_window_icon_y_fill(XfceOverviewWorkspacesWindowContentX11 *self);
void xfce_overview_workspaces_window_content_x11_set_unmapped_window_icon_y_fill(XfceOverviewWorkspacesWindowContentX11 *self, const gboolean inFill);

gfloat xfce_overview_workspaces_window_content_x11_get_unmapped_window_icon_x_align(XfceOverviewWorkspacesWindowContentX11 *self);
void xfce_overview_workspaces_window_content_x11_set_unmapped_window_icon_x_align(XfceOverviewWorkspacesWindowContentX11 *self, const gfloat inAlign);

gfloat xfce_overview_workspaces_window_content_x11_get_unmapped_window_icon_y_align(XfceOverviewWorkspacesWindowContentX11 *self);
void xfce_overview_workspaces_window_content_x11_set_unmapped_window_icon_y_align(XfceOverviewWorkspacesWindowContentX11 *self, const gfloat inAlign);

gfloat xfce_overview_workspaces_window_content_x11_get_unmapped_window_icon_x_scale(XfceOverviewWorkspacesWindowContentX11 *self);
void xfce_overview_workspaces_window_content_x11_set_unmapped_window_icon_x_scale(XfceOverviewWorkspacesWindowContentX11 *self, const gfloat inScale);

gfloat xfce_overview_workspaces_window_content_x11_get_unmapped_window_icon_y_scale(XfceOverviewWorkspacesWindowContentX11 *self);
void xfce_overview_workspaces_window_content_x11_set_unmapped_window_icon_y_scale(XfceOverviewWorkspacesWindowContentX11 *self, const gfloat inScale);

XfceOverviewWorkspacesAnchorPoint xfce_overview_workspaces_window_content_x11_get_unmapped_window_icon_anchor_point(XfceOverviewWorkspacesWindowContentX11 *self);
void xfce_overview_workspaces_window_content_x11_set_unmapped_window_icon_anchor_point(XfceOverviewWorkspacesWindowContentX11 *self, const XfceOverviewWorkspacesAnchorPoint inAnchorPoint);

G_END_DECLS

#endif
