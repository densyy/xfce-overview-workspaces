/*
 * popup-menu-item-button: A button pop-up menu item
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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_POPUP_MENU_ITEM_BUTTON__
#define __LIBXFCE_OVERVIEW_WORKSPACES_POPUP_MENU_ITEM_BUTTON__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <libxfce-overview-workspaces/label.h>

G_BEGIN_DECLS

#define XFCE_OVERVIEW_WORKSPACES_TYPE_POPUP_MENU_ITEM_BUTTON				(xfce_overview_workspaces_popup_menu_item_button_get_type())
#define XFCE_OVERVIEW_WORKSPACES_POPUP_MENU_ITEM_BUTTON(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_POPUP_MENU_ITEM_BUTTON, XfceOverviewWorkspacesPopupMenuItemButton))
#define XFCE_OVERVIEW_WORKSPACES_IS_POPUP_MENU_ITEM_BUTTON(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_POPUP_MENU_ITEM_BUTTON))
#define XFCE_OVERVIEW_WORKSPACES_POPUP_MENU_ITEM_BUTTON_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_POPUP_MENU_ITEM_BUTTON, XfceOverviewWorkspacesPopupMenuItemButtonClass))
#define XFCE_OVERVIEW_WORKSPACES_IS_POPUP_MENU_ITEM_BUTTON_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_POPUP_MENU_ITEM_BUTTON))
#define XFCE_OVERVIEW_WORKSPACES_POPUP_MENU_ITEM_BUTTON_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_POPUP_MENU_ITEM_BUTTON, XfceOverviewWorkspacesPopupMenuItemButtonClass))

typedef struct _XfceOverviewWorkspacesPopupMenuItemButton				XfceOverviewWorkspacesPopupMenuItemButton;
typedef struct _XfceOverviewWorkspacesPopupMenuItemButtonClass			XfceOverviewWorkspacesPopupMenuItemButtonClass;
typedef struct _XfceOverviewWorkspacesPopupMenuItemButtonPrivate		XfceOverviewWorkspacesPopupMenuItemButtonPrivate;

/**
 * XfceOverviewWorkspacesPopupMenuItemButton:
 *
 * The #XfceOverviewWorkspacesPopupMenuItemButton structure contains only private data and
 * should be accessed using the provided API
 */
struct _XfceOverviewWorkspacesPopupMenuItemButton
{
	/*< private >*/
	/* Parent instance */
	XfceOverviewWorkspacesLabel							parent_instance;

	/* Private structure */
	XfceOverviewWorkspacesPopupMenuItemButtonPrivate		*priv;
};

/**
 * XfceOverviewWorkspacesPopupMenuItemButtonClass:
 *
 * The #XfceOverviewWorkspacesPopupMenuItemButtonClass structure contains only private data
 */
struct _XfceOverviewWorkspacesPopupMenuItemButtonClass
{
	/*< private >*/
	/* Parent class */
	XfceOverviewWorkspacesLabelClass						parent_class;

	/*< public >*/
	/* Virtual functions */
};

/* Public API */
GType xfce_overview_workspaces_popup_menu_item_button_get_type(void) G_GNUC_CONST;

ClutterActor* xfce_overview_workspaces_popup_menu_item_button_new(void);
ClutterActor* xfce_overview_workspaces_popup_menu_item_button_new_with_text(const gchar *inText);

G_END_DECLS

#endif	/* __LIBXFCE_OVERVIEW_WORKSPACES_POPUP_MENU_ITEM_BUTTON__ */
