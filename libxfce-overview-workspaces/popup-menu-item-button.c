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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <libxfce-overview-workspaces/popup-menu-item-button.h>

#include <glib/gi18n-lib.h>

#include <libxfce-overview-workspaces/popup-menu-item.h>
#include <libxfce-overview-workspaces/click-action.h>


/* Define this class in GObject system */
static void _xfce_overview_workspaces_popup_menu_item_button_popup_menu_item_iface_init(XfceOverviewWorkspacesPopupMenuItemInterface *iface);

struct _XfceOverviewWorkspacesPopupMenuItemButtonPrivate
{
	/* Instance related */
	ClutterAction				*clickAction;
	gboolean					enabled;
};

G_DEFINE_TYPE_WITH_CODE(XfceOverviewWorkspacesPopupMenuItemButton,
						xfce_overview_workspaces_popup_menu_item_button,
						XFCE_OVERVIEW_WORKSPACES_TYPE_LABEL,
						G_ADD_PRIVATE(XfceOverviewWorkspacesPopupMenuItemButton)
						G_IMPLEMENT_INTERFACE(XFCE_OVERVIEW_WORKSPACES_TYPE_POPUP_MENU_ITEM, _xfce_overview_workspaces_popup_menu_item_button_popup_menu_item_iface_init))

/* IMPLEMENTATION: Private variables and methods */

/* Pop-up menu item button was clicked so activate this item */
static void _xfce_overview_workspaces_popup_menu_item_button_clicked(XfceOverviewWorkspacesClickAction *inAction,
														ClutterActor *self,
														gpointer inUserData)
{
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_CLICK_ACTION(inAction));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_POPUP_MENU_ITEM_BUTTON(self));

	/* Only emit any of these signals if click was perform with left button 
	 * or is a short touchscreen touch event.
	 */
	if(xfce_overview_workspaces_click_action_is_left_button_or_tap(inAction))
	{
		xfce_overview_workspaces_popup_menu_item_activate(XFCE_OVERVIEW_WORKSPACES_POPUP_MENU_ITEM(self));
	}
}

/* IMPLEMENTATION: Interface XfceOverviewWorkspacesPopupMenuItem */

/* Get enable state of this pop-up menu item */
static gboolean _xfce_overview_workspaces_popup_menu_item_button_popup_menu_item_get_enabled(XfceOverviewWorkspacesPopupMenuItem *inMenuItem)
{
	XfceOverviewWorkspacesPopupMenuItemButton			*self;
	XfceOverviewWorkspacesPopupMenuItemButtonPrivate	*priv;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_POPUP_MENU_ITEM_BUTTON(inMenuItem), FALSE);

	self=XFCE_OVERVIEW_WORKSPACES_POPUP_MENU_ITEM_BUTTON(inMenuItem);
	priv=self->priv;

	/* Return enabled state */
	return(priv->enabled);
}

/* Set enable state of this pop-up menu item */
static void _xfce_overview_workspaces_popup_menu_item_button_popup_menu_item_set_enabled(XfceOverviewWorkspacesPopupMenuItem *inMenuItem, gboolean inEnabled)
{
	XfceOverviewWorkspacesPopupMenuItemButton			*self;
	XfceOverviewWorkspacesPopupMenuItemButtonPrivate	*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_POPUP_MENU_ITEM_BUTTON(inMenuItem));

	self=XFCE_OVERVIEW_WORKSPACES_POPUP_MENU_ITEM_BUTTON(inMenuItem);
	priv=self->priv;

	/* Set enabled state */
	priv->enabled=inEnabled;
}

/* Interface initialization
 * Set up default functions
 */
void _xfce_overview_workspaces_popup_menu_item_button_popup_menu_item_iface_init(XfceOverviewWorkspacesPopupMenuItemInterface *iface)
{
	iface->get_enabled=_xfce_overview_workspaces_popup_menu_item_button_popup_menu_item_get_enabled;
	iface->set_enabled=_xfce_overview_workspaces_popup_menu_item_button_popup_menu_item_set_enabled;
}

/* IMPLEMENTATION: GObject */

/* Class initialization
 * Override functions in parent classes and define properties
 * and signals
 */
static void xfce_overview_workspaces_popup_menu_item_button_class_init(XfceOverviewWorkspacesPopupMenuItemButtonClass *klass)
{
}

/* Object initialization
 * Create private structure and set up default values
 */
static void xfce_overview_workspaces_popup_menu_item_button_init(XfceOverviewWorkspacesPopupMenuItemButton *self)
{
	XfceOverviewWorkspacesPopupMenuItemButtonPrivate	*priv;

	priv=self->priv=xfce_overview_workspaces_popup_menu_item_button_get_instance_private(self);

	/* Set up default values */
	priv->enabled=TRUE;

	/* This actor reacts on events */
	clutter_actor_set_reactive(CLUTTER_ACTOR(self), TRUE);

	/* Connect signals */
	priv->clickAction=xfce_overview_workspaces_click_action_new();
	clutter_actor_add_action(CLUTTER_ACTOR(self), priv->clickAction);
	g_signal_connect(priv->clickAction, "clicked", G_CALLBACK(_xfce_overview_workspaces_popup_menu_item_button_clicked), NULL);
}

/* IMPLEMENTATION: Public API */

/* Create new actor */
ClutterActor* xfce_overview_workspaces_popup_menu_item_button_new(void)
{
	return(g_object_new(XFCE_OVERVIEW_WORKSPACES_TYPE_POPUP_MENU_ITEM_BUTTON,
						"text", "",
						"label-style", XFCE_OVERVIEW_WORKSPACES_LABEL_STYLE_TEXT,
						NULL));
}

ClutterActor* xfce_overview_workspaces_popup_menu_item_button_new_with_text(const gchar *inText)
{
	return(g_object_new(XFCE_OVERVIEW_WORKSPACES_TYPE_POPUP_MENU_ITEM_BUTTON,
						"text", inText,
						"label-style", XFCE_OVERVIEW_WORKSPACES_LABEL_STYLE_TEXT,
						NULL));
}
