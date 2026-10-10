/*
 * popup-menu-item: An interface implemented by actors used as pop-up menu item
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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_POPUP_MENU_ITEM__
#define __LIBXFCE_OVERVIEW_WORKSPACES_POPUP_MENU_ITEM__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <glib-object.h>

G_BEGIN_DECLS

#define XFCE_OVERVIEW_WORKSPACES_TYPE_POPUP_MENU_ITEM				(xfce_overview_workspaces_popup_menu_item_get_type())
#define XFCE_OVERVIEW_WORKSPACES_POPUP_MENU_ITEM(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_POPUP_MENU_ITEM, XfceOverviewWorkspacesPopupMenuItem))
#define XFCE_OVERVIEW_WORKSPACES_IS_POPUP_MENU_ITEM(obj)				(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_POPUP_MENU_ITEM))
#define XFCE_OVERVIEW_WORKSPACES_POPUP_MENU_ITEM_GET_IFACE(obj)		(G_TYPE_INSTANCE_GET_INTERFACE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_POPUP_MENU_ITEM, XfceOverviewWorkspacesPopupMenuItemInterface))

typedef struct _XfceOverviewWorkspacesPopupMenuItem				XfceOverviewWorkspacesPopupMenuItem;
typedef struct _XfceOverviewWorkspacesPopupMenuItemInterface		XfceOverviewWorkspacesPopupMenuItemInterface;

/**
 * XfceOverviewWorkspacesPopupMenuItemInterface:
 * @parent_interface: The parent interface.
 * @get_enabled: Retrieve state if pop-up menu item is enabled or disabled
 * @set_enabled: Set state if pop-up menu item is enabled or disabled
 *
 * Provides an interface implemented by actors which will be used as pop-up menu
 * items in a #XfceOverviewWorkspacesPopupMenu.
 */
struct _XfceOverviewWorkspacesPopupMenuItemInterface
{
	/*< private >*/
	/* Parent interface */
	GTypeInterface				parent_interface;

	/*< public >*/
	/* Virtual functions */
	gboolean (*get_enabled)(XfceOverviewWorkspacesPopupMenuItem *self);
	void (*set_enabled)(XfceOverviewWorkspacesPopupMenuItem *self, gboolean inEnabled);
};

/* Public API */
GType xfce_overview_workspaces_popup_menu_item_get_type(void) G_GNUC_CONST;

gboolean xfce_overview_workspaces_popup_menu_item_get_enabled(XfceOverviewWorkspacesPopupMenuItem *self);
void xfce_overview_workspaces_popup_menu_item_set_enabled(XfceOverviewWorkspacesPopupMenuItem *self, gboolean inEnabled);

void xfce_overview_workspaces_popup_menu_item_activate(XfceOverviewWorkspacesPopupMenuItem *self);

G_END_DECLS

#endif	/* __LIBXFCE_OVERVIEW_WORKSPACES_POPUP_MENU_ITEM__ */
