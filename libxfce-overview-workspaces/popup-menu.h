/*
 * popup-menu: A pop-up menu with entries performing an action when an entry
 *             was clicked
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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_POPUP_MENU__
#define __LIBXFCE_OVERVIEW_WORKSPACES_POPUP_MENU__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <libxfce-overview-workspaces/background.h>
#include <libxfce-overview-workspaces/popup-menu-item.h>

G_BEGIN_DECLS

#define XFCE_OVERVIEW_WORKSPACES_TYPE_POPUP_MENU				(xfce_overview_workspaces_popup_menu_get_type())
#define XFCE_OVERVIEW_WORKSPACES_POPUP_MENU(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_POPUP_MENU, XfceOverviewWorkspacesPopupMenu))
#define XFCE_OVERVIEW_WORKSPACES_IS_POPUP_MENU(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_POPUP_MENU))
#define XFCE_OVERVIEW_WORKSPACES_POPUP_MENU_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_POPUP_MENU, XfceOverviewWorkspacesPopupMenuClass))
#define XFCE_OVERVIEW_WORKSPACES_IS_POPUP_MENU_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_POPUP_MENU))
#define XFCE_OVERVIEW_WORKSPACES_POPUP_MENU_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_POPUP_MENU, XfceOverviewWorkspacesPopupMenuClass))

typedef struct _XfceOverviewWorkspacesPopupMenu			XfceOverviewWorkspacesPopupMenu;
typedef struct _XfceOverviewWorkspacesPopupMenuClass		XfceOverviewWorkspacesPopupMenuClass;
typedef struct _XfceOverviewWorkspacesPopupMenuPrivate		XfceOverviewWorkspacesPopupMenuPrivate;

/**
 * XfceOverviewWorkspacesPopupMenu:
 *
 * The #XfceOverviewWorkspacesPopupMenu structure contains only private data and
 * should be accessed using the provided API
 */
struct _XfceOverviewWorkspacesPopupMenu
{
	/*< private >*/
	/* Parent instance */
	XfceOverviewWorkspacesBackground				parent_instance;

	/* Private structure */
	XfceOverviewWorkspacesPopupMenuPrivate			*priv;
};

/**
 * XfceOverviewWorkspacesPopupMenuClass:
 *
 * The #XfceOverviewWorkspacesPopupMenuClass structure contains only private data
 */
struct _XfceOverviewWorkspacesPopupMenuClass
{
	/*< private >*/
	/* Parent class */
	XfceOverviewWorkspacesBackgroundClass			parent_class;

	/*< public >*/
	/* Virtual functions */
	void (*activated)(XfceOverviewWorkspacesPopupMenu *self);
	void (*cancelled)(XfceOverviewWorkspacesPopupMenu *self);

	void (*item_activated)(XfceOverviewWorkspacesPopupMenu *self, XfceOverviewWorkspacesPopupMenuItem *inMenuItem);

	void (*item_added)(XfceOverviewWorkspacesPopupMenu *self, XfceOverviewWorkspacesPopupMenuItem *inMenuItem);
	void (*item_removed)(XfceOverviewWorkspacesPopupMenu *self, XfceOverviewWorkspacesPopupMenuItem *inMenuItem);
};

/* Public API */
GType xfce_overview_workspaces_popup_menu_get_type(void) G_GNUC_CONST;

ClutterActor* xfce_overview_workspaces_popup_menu_new(void);
ClutterActor* xfce_overview_workspaces_popup_menu_new_for_source(ClutterActor *inSource);

gboolean xfce_overview_workspaces_popup_menu_get_destroy_on_cancel(XfceOverviewWorkspacesPopupMenu *self);
void xfce_overview_workspaces_popup_menu_set_destroy_on_cancel(XfceOverviewWorkspacesPopupMenu *self, gboolean inDestroyOnCancel);

ClutterActor* xfce_overview_workspaces_popup_menu_get_source(XfceOverviewWorkspacesPopupMenu *self);
void xfce_overview_workspaces_popup_menu_set_source(XfceOverviewWorkspacesPopupMenu *self, ClutterActor *inSource);

gboolean xfce_overview_workspaces_popup_menu_get_show_title(XfceOverviewWorkspacesPopupMenu *self);
void xfce_overview_workspaces_popup_menu_set_show_title(XfceOverviewWorkspacesPopupMenu *self, gboolean inShowTitle);

const gchar* xfce_overview_workspaces_popup_menu_get_title(XfceOverviewWorkspacesPopupMenu *self);
void xfce_overview_workspaces_popup_menu_set_title(XfceOverviewWorkspacesPopupMenu *self, const gchar *inMarkupTitle);

gboolean xfce_overview_workspaces_popup_menu_get_show_title_icon(XfceOverviewWorkspacesPopupMenu *self);
void xfce_overview_workspaces_popup_menu_set_show_title_icon(XfceOverviewWorkspacesPopupMenu *self, gboolean inShowTitleIcon);

const gchar* xfce_overview_workspaces_popup_menu_get_title_icon_name(XfceOverviewWorkspacesPopupMenu *self);
void xfce_overview_workspaces_popup_menu_set_title_icon_name(XfceOverviewWorkspacesPopupMenu *self, const gchar *inIconName);

GIcon* xfce_overview_workspaces_popup_menu_get_title_gicon(XfceOverviewWorkspacesPopupMenu *self);
void xfce_overview_workspaces_popup_menu_set_title_gicon(XfceOverviewWorkspacesPopupMenu *self, GIcon *inIcon);


gint xfce_overview_workspaces_popup_menu_add_item(XfceOverviewWorkspacesPopupMenu *self,
										XfceOverviewWorkspacesPopupMenuItem *inMenuItem);
gint xfce_overview_workspaces_popup_menu_insert_item(XfceOverviewWorkspacesPopupMenu *self,
										XfceOverviewWorkspacesPopupMenuItem *inMenuItem,
										gint inIndex);
gboolean xfce_overview_workspaces_popup_menu_move_item(XfceOverviewWorkspacesPopupMenu *self,
											XfceOverviewWorkspacesPopupMenuItem *inMenuItem,
											gint inIndex);
XfceOverviewWorkspacesPopupMenuItem* xfce_overview_workspaces_popup_menu_get_item(XfceOverviewWorkspacesPopupMenu *self, gint inIndex);
gint xfce_overview_workspaces_popup_menu_get_item_index(XfceOverviewWorkspacesPopupMenu *self, XfceOverviewWorkspacesPopupMenuItem *inMenuItem);
gboolean xfce_overview_workspaces_popup_menu_remove_item(XfceOverviewWorkspacesPopupMenu *self, XfceOverviewWorkspacesPopupMenuItem *inMenuItem);


void xfce_overview_workspaces_popup_menu_activate(XfceOverviewWorkspacesPopupMenu *self);
void xfce_overview_workspaces_popup_menu_cancel(XfceOverviewWorkspacesPopupMenu *self);

G_END_DECLS

#endif	/* __LIBXFCE_OVERVIEW_WORKSPACES_POPUP_MENU__ */
