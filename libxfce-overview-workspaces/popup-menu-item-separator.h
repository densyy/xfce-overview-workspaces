/*
 * popup-menu-item-separator: A separator menu item
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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_POPUP_MENU_ITEM_SEPARATOR__
#define __LIBXFCE_OVERVIEW_WORKSPACES_POPUP_MENU_ITEM_SEPARATOR__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <libxfce-overview-workspaces/background.h>

G_BEGIN_DECLS

#define XFCE_OVERVIEW_WORKSPACES_TYPE_POPUP_MENU_ITEM_SEPARATOR				(xfce_overview_workspaces_popup_menu_item_separator_get_type())
#define XFCE_OVERVIEW_WORKSPACES_POPUP_MENU_ITEM_SEPARATOR(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_POPUP_MENU_ITEM_SEPARATOR, XfceOverviewWorkspacesPopupMenuItemSeparator))
#define XFCE_OVERVIEW_WORKSPACES_IS_POPUP_MENU_ITEM_SEPARATOR(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_POPUP_MENU_ITEM_SEPARATOR))
#define XFCE_OVERVIEW_WORKSPACES_POPUP_MENU_ITEM_SEPARATOR_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_POPUP_MENU_ITEM_SEPARATOR, XfceOverviewWorkspacesPopupMenuItemSeparatorClass))
#define XFCE_OVERVIEW_WORKSPACES_IS_POPUP_MENU_ITEM_SEPARATOR_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_POPUP_MENU_ITEM_SEPARATOR))
#define XFCE_OVERVIEW_WORKSPACES_POPUP_MENU_ITEM_SEPARATOR_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_POPUP_MENU_ITEM_SEPARATOR, XfceOverviewWorkspacesPopupMenuItemSeparatorClass))

typedef struct _XfceOverviewWorkspacesPopupMenuItemSeparator				XfceOverviewWorkspacesPopupMenuItemSeparator;
typedef struct _XfceOverviewWorkspacesPopupMenuItemSeparatorClass			XfceOverviewWorkspacesPopupMenuItemSeparatorClass;
typedef struct _XfceOverviewWorkspacesPopupMenuItemSeparatorPrivate		XfceOverviewWorkspacesPopupMenuItemSeparatorPrivate;

/**
 * XfceOverviewWorkspacesPopupMenuItemSeparator:
 *
 * The #XfceOverviewWorkspacesPopupMenuItemSeparator structure contains only private data and
 * should be accessed using the provided API
 */
struct _XfceOverviewWorkspacesPopupMenuItemSeparator
{
	/*< private >*/
	/* Parent instance */
	XfceOverviewWorkspacesBackground							parent_instance;

	/* Private structure */
	XfceOverviewWorkspacesPopupMenuItemSeparatorPrivate		*priv;
};

/**
 * XfceOverviewWorkspacesPopupMenuItemSeparatorClass:
 *
 * The #XfceOverviewWorkspacesPopupMenuItemSeparatorClass structure contains only private data
 */
struct _XfceOverviewWorkspacesPopupMenuItemSeparatorClass
{
	/*< private >*/
	/* Parent class */
	XfceOverviewWorkspacesBackgroundClass						parent_class;

	/*< public >*/
	/* Virtual functions */
};

/* Public API */
GType xfce_overview_workspaces_popup_menu_item_separator_get_type(void) G_GNUC_CONST;

ClutterActor* xfce_overview_workspaces_popup_menu_item_separator_new(void);

gfloat xfce_overview_workspaces_popup_menu_item_separator_get_minimum_height(XfceOverviewWorkspacesPopupMenuItemSeparator *self);
void xfce_overview_workspaces_popup_menu_item_separator_set_minimum_height(XfceOverviewWorkspacesPopupMenuItemSeparator *self, const gfloat inMinimumHeight);

gfloat xfce_overview_workspaces_popup_menu_item_separator_get_line_horizontal_alignment(XfceOverviewWorkspacesPopupMenuItemSeparator *self);
void xfce_overview_workspaces_popup_menu_item_separator_set_line_horizontal_alignment(XfceOverviewWorkspacesPopupMenuItemSeparator *self, const gfloat inAlignment);

gfloat xfce_overview_workspaces_popup_menu_item_separator_get_line_vertical_alignment(XfceOverviewWorkspacesPopupMenuItemSeparator *self);
void xfce_overview_workspaces_popup_menu_item_separator_set_line_vertical_alignment(XfceOverviewWorkspacesPopupMenuItemSeparator *self, const gfloat inAlignment);

gfloat xfce_overview_workspaces_popup_menu_item_separator_get_line_length(XfceOverviewWorkspacesPopupMenuItemSeparator *self);
void xfce_overview_workspaces_popup_menu_item_separator_set_line_length(XfceOverviewWorkspacesPopupMenuItemSeparator *self, const gfloat inLength);

gfloat xfce_overview_workspaces_popup_menu_item_separator_get_line_width(XfceOverviewWorkspacesPopupMenuItemSeparator *self);
void xfce_overview_workspaces_popup_menu_item_separator_set_line_width(XfceOverviewWorkspacesPopupMenuItemSeparator *self, const gfloat inWidth);

const ClutterColor* xfce_overview_workspaces_popup_menu_item_separator_get_line_color(XfceOverviewWorkspacesPopupMenuItemSeparator *self);
void xfce_overview_workspaces_popup_menu_item_separator_set_line_color(XfceOverviewWorkspacesPopupMenuItemSeparator *self, const ClutterColor *inColor);

G_END_DECLS

#endif	/* __LIBXFCE_OVERVIEW_WORKSPACES_POPUP_MENU_ITEM_SEPARATOR__ */
