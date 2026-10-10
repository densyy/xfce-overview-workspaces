/*
 * applications-view: A view showing all installed applications as menu
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

#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>

#include <libxfce-overview-workspaces/applications-view.h>
#include <libxfce-overview-workspaces/view.h>
#include <libxfce-overview-workspaces/applications-menu-model.h>
#include <libxfce-overview-workspaces/types.h>
#include <libxfce-overview-workspaces/button.h>
#include <libxfce-overview-workspaces/core.h>
#include <libxfce-overview-workspaces/application-button.h>
#include <libxfce-overview-workspaces/enums.h>
#include <libxfce-overview-workspaces/drag-action.h>
#include <libxfce-overview-workspaces/stylable.h>
#include <libxfce-overview-workspaces/focusable.h>
#include <libxfce-overview-workspaces/focus-manager.h>
#include <libxfce-overview-workspaces/dynamic-table-layout.h>
#include <libxfce-overview-workspaces/utils.h>
#include <libxfce-overview-workspaces/desktop-app-info.h>
#include <libxfce-overview-workspaces/application-database.h>
#include <libxfce-overview-workspaces/click-action.h>
#include <libxfce-overview-workspaces/popup-menu.h>
#include <libxfce-overview-workspaces/popup-menu-item-button.h>
#include <libxfce-overview-workspaces/popup-menu-item-separator.h>
#include <libxfce-overview-workspaces/application-tracker.h>
#include <libxfce-overview-workspaces/debug.h>


/* Define this class in GObject system */
static void _xfce_overview_workspaces_applications_view_focusable_iface_init(XfceOverviewWorkspacesFocusableInterface *iface);

struct _XfceOverviewWorkspacesApplicationsViewPrivate
{
	/* Properties related */
	XfceOverviewWorkspacesViewMode					viewMode;
	gfloat								spacing;
	gchar								*parentMenuIcon;
	gchar								*formatTitleOnly;
	gchar								*formatTitleDescription;

	/* Instance related */
	ClutterLayoutManager				*layout;
	XfceOverviewWorkspacesApplicationsMenuModel	*apps;
	GarconMenuElement					*currentRootMenuElement;

	gpointer							selectedItem;

	gboolean							showAllAppsMenu;
	GBinding							*settingsShowAllAppsMenuBinding;
};

G_DEFINE_TYPE_WITH_CODE(XfceOverviewWorkspacesApplicationsView,
						xfce_overview_workspaces_applications_view,
						XFCE_OVERVIEW_WORKSPACES_TYPE_VIEW,
						G_ADD_PRIVATE(XfceOverviewWorkspacesApplicationsView)
						G_IMPLEMENT_INTERFACE(XFCE_OVERVIEW_WORKSPACES_TYPE_FOCUSABLE, _xfce_overview_workspaces_applications_view_focusable_iface_init))

/* Properties */
enum
{
	PROP_0,

	PROP_VIEW_MODE,
	PROP_SPACING,

	PROP_PARENT_MENU_ICON,
	PROP_FORMAT_TITLE_ONLY,
	PROP_FORMAT_TITLE_DESCRIPTION,

	PROP_SHOW_ALL_APPS,

	PROP_LAST
};

static GParamSpec* XfceOverviewWorkspacesApplicationsViewProperties[PROP_LAST]={ 0, };

/* IMPLEMENTATION: Private variables and methods */
#define ALL_APPLICATIONS_MENU_ICON		"applications-other"

/* Forward declarations */
static void _xfce_overview_workspaces_applications_view_on_item_clicked(XfceOverviewWorkspacesApplicationsView *self, gpointer inUserData);
static void _xfce_overview_workspaces_applications_view_on_parent_menu_clicked(XfceOverviewWorkspacesApplicationsView *self, gpointer inUserData);
static void _xfce_overview_workspaces_applications_view_on_all_applications_menu_parent_menu_clicked(XfceOverviewWorkspacesApplicationsView *self, gpointer inUserData);
static void _xfce_overview_workspaces_applications_view_on_all_applications_menu_clicked(XfceOverviewWorkspacesApplicationsView *self, gpointer inUserData);

/* Set up child actor for current view mode */
static void _xfce_overview_workspaces_applications_view_setup_actor_for_view_mode(XfceOverviewWorkspacesApplicationsView *self, ClutterActor *inActor)
{
	XfceOverviewWorkspacesApplicationsViewPrivate	*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_APPLICATIONS_VIEW(self));
	g_return_if_fail(CLUTTER_IS_ACTOR(inActor));

	priv=self->priv;

	/* In list mode just fill all available space and align to top-left corner */
	if(priv->viewMode==XFCE_OVERVIEW_WORKSPACES_VIEW_MODE_LIST)
	{
		clutter_actor_set_x_expand(inActor, TRUE);
		clutter_actor_set_y_expand(inActor, TRUE);
		clutter_actor_set_x_align(inActor, CLUTTER_ACTOR_ALIGN_FILL);
		clutter_actor_set_y_align(inActor, CLUTTER_ACTOR_ALIGN_FILL);

		if(XFCE_OVERVIEW_WORKSPACES_IS_STYLABLE(inActor)) xfce_overview_workspaces_stylable_add_class(XFCE_OVERVIEW_WORKSPACES_STYLABLE(inActor), "view-mode-list");
	}
		/* In view mode do not fill all available space and align to top-center
		 * corner or middle-left corner depending on request mode of actor.
		 */
		else
		{
			clutter_actor_set_x_expand(inActor, FALSE);
			clutter_actor_set_y_expand(inActor, FALSE);
			if(clutter_actor_get_request_mode(inActor)==CLUTTER_REQUEST_HEIGHT_FOR_WIDTH)
			{
				clutter_actor_set_x_align(CLUTTER_ACTOR(inActor), CLUTTER_ACTOR_ALIGN_CENTER);
				clutter_actor_set_y_align(CLUTTER_ACTOR(inActor), CLUTTER_ACTOR_ALIGN_START);
			}
				else
				{
					clutter_actor_set_x_align(CLUTTER_ACTOR(inActor), CLUTTER_ACTOR_ALIGN_START);
					clutter_actor_set_y_align(CLUTTER_ACTOR(inActor), CLUTTER_ACTOR_ALIGN_CENTER);
				}

			if(XFCE_OVERVIEW_WORKSPACES_IS_STYLABLE(inActor)) xfce_overview_workspaces_stylable_add_class(XFCE_OVERVIEW_WORKSPACES_STYLABLE(inActor), "view-mode-icon");
		}
}

/* Create back button for current view mode (extracted from 3 copied blocks) */
static ClutterActor* _add_back_button(XfceOverviewWorkspacesApplicationsView *self,
										const gchar *inIconName,
										const gchar *inTitle,
										const gchar *inDescription,
										GCallback inCallback)
{
	XfceOverviewWorkspacesApplicationsViewPrivate	*priv;
	ClutterActor						*actor;
	gchar								*actorText;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_APPLICATIONS_VIEW(self), NULL);
	g_return_val_if_fail(inTitle, NULL);
	g_return_val_if_fail(inCallback, NULL);

	priv=self->priv;

	/* Create button */
	actor=xfce_overview_workspaces_button_new();

	if(inIconName) xfce_overview_workspaces_label_set_icon_name(XFCE_OVERVIEW_WORKSPACES_LABEL(actor), inIconName);

	if(priv->viewMode==XFCE_OVERVIEW_WORKSPACES_VIEW_MODE_LIST) actorText=g_markup_printf_escaped(priv->formatTitleDescription, inTitle, inDescription ? inDescription : "");
		else actorText=g_markup_printf_escaped(priv->formatTitleOnly, inTitle);
	xfce_overview_workspaces_label_set_text(XFCE_OVERVIEW_WORKSPACES_LABEL(actor), actorText);
	g_free(actorText);

	/* Add to view and layout */
	_xfce_overview_workspaces_applications_view_setup_actor_for_view_mode(self, CLUTTER_ACTOR(actor));
	clutter_actor_add_child(CLUTTER_ACTOR(self), CLUTTER_ACTOR(actor));
	clutter_actor_show(actor);

	g_signal_connect_swapped(actor,
								"clicked",
								inCallback,
								self);

	/* Select button automatically if view has focus */
	if(xfce_overview_workspaces_view_has_focus(XFCE_OVERVIEW_WORKSPACES_VIEW(self)))
	{
		xfce_overview_workspaces_focusable_set_selection(XFCE_OVERVIEW_WORKSPACES_FOCUSABLE(self), CLUTTER_ACTOR(actor));
	}

	return(actor);
}

/* Sort entry with precomputed lower-cased keys for "all applications".
 * The original code called g_utf8_strdown() on every comparison (plus
 * g_file_equal on every comparison), i.e. O(n log n) allocations for the
 * click on "All applications". Precomputing the keys once per application
 * reduces this to O(n) allocations with plain strcmp in the comparator and
 * does not change the resulting order.
 */
typedef struct _AllAppsSortEntry		AllAppsSortEntry;
struct _AllAppsSortEntry
{
	XfceOverviewWorkspacesDesktopAppInfo		*appInfo;
	gchar							*keys[5];
};

static const gchar* (*_all_apps_field_getters[])(GAppInfo*)=
{
	g_app_info_get_name,
	g_app_info_get_display_name,
	g_app_info_get_description,
	g_app_info_get_executable,
	g_app_info_get_commandline
};

static AllAppsSortEntry* _all_apps_sort_entry_new(XfceOverviewWorkspacesDesktopAppInfo *inAppInfo)
{
	AllAppsSortEntry				*entry;
	guint						i;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_DESKTOP_APP_INFO(inAppInfo), NULL);

	entry=g_new0(AllAppsSortEntry, 1);
	entry->appInfo=inAppInfo; /* Takes over the reference from the input list */

	for(i=0; i<G_N_ELEMENTS(_all_apps_field_getters); i++)
	{
		const gchar				*raw;

		raw=_all_apps_field_getters[i](G_APP_INFO(inAppInfo));
		entry->keys[i]=raw ? g_utf8_strdown(raw, -1) : NULL;
	}

	return(entry);
}

static void _all_apps_sort_entry_free(AllAppsSortEntry *inEntry)
{
	guint						i;

	if(!inEntry) return;

	for(i=0; i<G_N_ELEMENTS(inEntry->keys); i++) g_free(inEntry->keys[i]);
	if(inEntry->appInfo) g_object_unref(inEntry->appInfo);
	g_free(inEntry);
}

/* Compare two precomputed sort entries (extracted from 5x inlined sort) */
static gint _cmp_all_apps_sort_entry(gconstpointer inLeft, gconstpointer inRight)
{
	const AllAppsSortEntry		*left=(const AllAppsSortEntry*)inLeft;
	const AllAppsSortEntry		*right=(const AllAppsSortEntry*)inRight;
	GFile						*leftFile;
	GFile						*rightFile;
	guint						i;
	gint						result;

	/* Invalid entries sort last (same semantics as before) */
	if(!xfce_overview_workspaces_desktop_app_info_is_valid(left->appInfo)) return(1);
	if(!xfce_overview_workspaces_desktop_app_info_is_valid(right->appInfo)) return(-1);

	/* Entries sharing the same desktop file are equal */
	leftFile=xfce_overview_workspaces_desktop_app_info_get_file(left->appInfo);
	rightFile=xfce_overview_workspaces_desktop_app_info_get_file(right->appInfo);
	if(leftFile && rightFile && g_file_equal(leftFile, rightFile)) return(0);

	/* Compare precomputed lower-cased keys in order */
	for(i=0; i<G_N_ELEMENTS(left->keys); i++)
	{
		result=g_strcmp0(left->keys[i], right->keys[i]);
		if(result!=0) return(result);
	}

	return(0);
}


/* Drag of an menu item begins */
static void _xfce_overview_workspaces_applications_view_on_drag_begin(ClutterDragAction *inAction,
															ClutterActor *inActor,
															gfloat inStageX,
															gfloat inStageY,
															ClutterModifierType inModifiers,
															gpointer inUserData)
{
	GAppInfo							*appInfo;
	ClutterActor						*dragHandle;
	ClutterStage						*stage;

	g_return_if_fail(CLUTTER_IS_DRAG_ACTION(inAction));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_APPLICATION_BUTTON(inActor));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_APPLICATIONS_VIEW(inUserData));

	/* Prevent signal "clicked" from being emitted on dragged icon */
	g_signal_handlers_block_by_func(inActor, _xfce_overview_workspaces_applications_view_on_item_clicked, inUserData);

	/* Get stage */
	stage=CLUTTER_STAGE(clutter_actor_get_stage(inActor));

	/* Create a application icon for drag handle */
	appInfo=xfce_overview_workspaces_application_button_get_app_info(XFCE_OVERVIEW_WORKSPACES_APPLICATION_BUTTON(inActor));

	dragHandle=xfce_overview_workspaces_application_button_new_from_app_info(appInfo);
	clutter_actor_set_position(dragHandle, inStageX, inStageY);
	clutter_actor_add_child(CLUTTER_ACTOR(stage), dragHandle);

	clutter_drag_action_set_drag_handle(inAction, dragHandle);
}

/* Drag of an menu item ends */
static void _xfce_overview_workspaces_applications_view_on_drag_end(ClutterDragAction *inAction,
														ClutterActor *inActor,
														gfloat inStageX,
														gfloat inStageY,
														ClutterModifierType inModifiers,
														gpointer inUserData)
{
	ClutterActor					*dragHandle;

	g_return_if_fail(CLUTTER_IS_DRAG_ACTION(inAction));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_APPLICATION_BUTTON(inActor));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_APPLICATIONS_VIEW(inUserData));

	/* Destroy clone of application icon used as drag handle */
	dragHandle=clutter_drag_action_get_drag_handle(inAction);
	if(dragHandle)
	{
		clutter_drag_action_set_drag_handle(inAction, NULL);
		xfce_overview_workspaces_actor_destroy(dragHandle);
	}

	/* Allow signal "clicked" from being emitted again */
	g_signal_handlers_unblock_by_func(inActor, _xfce_overview_workspaces_applications_view_on_item_clicked, inUserData);
}

/* Filter of applications data model has changed */
static void _xfce_overview_workspaces_applications_view_on_menu_clicked(XfceOverviewWorkspacesButton *inButton, gpointer inUserData)
{
	XfceOverviewWorkspacesApplicationsView			*self;
	XfceOverviewWorkspacesApplicationsViewPrivate	*priv;
	ClutterActor						*parent;
	GarconMenu							*menu;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_BUTTON(inButton));
	g_return_if_fail(GARCON_IS_MENU(inUserData));

	menu=GARCON_MENU(inUserData);

	/* Find this view's object */
	parent=clutter_actor_get_parent(CLUTTER_ACTOR(inButton));
	while(parent && !XFCE_OVERVIEW_WORKSPACES_IS_APPLICATIONS_VIEW(parent))
	{
		parent=clutter_actor_get_parent(parent);
	}

	if(!parent)
	{
		g_warning("Could not find view of type %s for menu '%s'",
					g_type_name(XFCE_OVERVIEW_WORKSPACES_TYPE_APPLICATIONS_VIEW),
					garcon_menu_element_get_name(GARCON_MENU_ELEMENT(menu)));
		return;
	}

	/* Get this view's object */
	self=XFCE_OVERVIEW_WORKSPACES_APPLICATIONS_VIEW(parent);
	priv=self->priv;

	/* Change menu */
	priv->currentRootMenuElement=GARCON_MENU_ELEMENT(menu);
	xfce_overview_workspaces_applications_menu_model_filter_by_section(priv->apps, menu);
	xfce_overview_workspaces_view_scroll_to(XFCE_OVERVIEW_WORKSPACES_VIEW(self), -1, 0);
}

/* Parent menu item to go up in menus was clicked */
static void _xfce_overview_workspaces_applications_view_on_parent_menu_clicked(XfceOverviewWorkspacesApplicationsView *self, gpointer inUserData)
{
	XfceOverviewWorkspacesApplicationsViewPrivate	*priv;
	GarconMenuElement					*element;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_APPLICATIONS_VIEW(self));

	priv=self->priv;

	/* Get associated menu element of button */
	if(priv->currentRootMenuElement &&
		GARCON_IS_MENU(priv->currentRootMenuElement))
	{
		element=GARCON_MENU_ELEMENT(garcon_menu_get_parent(GARCON_MENU(priv->currentRootMenuElement)));

		priv->currentRootMenuElement=element;
		xfce_overview_workspaces_applications_menu_model_filter_by_section(priv->apps, GARCON_MENU(element));
		xfce_overview_workspaces_view_scroll_to(XFCE_OVERVIEW_WORKSPACES_VIEW(self), -1, 0);
	}
}

/* An application menu item was clicked */
static void _xfce_overview_workspaces_applications_view_on_item_clicked(XfceOverviewWorkspacesApplicationsView *self, gpointer inUserData)
{
	XfceOverviewWorkspacesApplicationButton		*button;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_APPLICATIONS_VIEW(self));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_APPLICATION_BUTTON(inUserData));

	button=XFCE_OVERVIEW_WORKSPACES_APPLICATION_BUTTON(inUserData);

	/* A menu item was clicked so execute command and quit application */
	if(xfce_overview_workspaces_application_button_execute(button, NULL))
	{
		/* Launching application seems to be successfuly so quit application */
		xfce_overview_workspaces_core_quit(NULL);
		return;
	}
}

/* User selected to open a new window or to launch that application at pop-up menu */
static void _xfce_overview_workspaces_applications_view_on_popup_menu_item_launch(XfceOverviewWorkspacesPopupMenuItem *inMenuItem,
																		gpointer inUserData)
{
	GAppInfo							*appInfo;
	XfceOverviewWorkspacesApplicationTracker		*appTracker;
	GIcon								*gicon;
	const gchar							*iconName;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_POPUP_MENU_ITEM(inMenuItem));
	g_return_if_fail(G_IS_APP_INFO(inUserData));

	appInfo=G_APP_INFO(inUserData);
	iconName=NULL;

	/* Get icon of application */
	gicon=g_app_info_get_icon(appInfo);
	if(gicon) iconName=g_icon_to_string(gicon);

	/* Check if we should launch that application or to open a new window */
	appTracker=xfce_overview_workspaces_core_get_application_tracker(NULL);
	if(!xfce_overview_workspaces_application_tracker_is_running_by_app_info(appTracker, appInfo))
	{
		GAppLaunchContext			*context;
		GError						*error;

		/* Create context to start application at */
		context=xfce_overview_workspaces_create_app_context(NULL);

		/* Try to launch application */
		error=NULL;
		if(!g_app_info_launch(appInfo, NULL, context, &error))
		{
			/* Show notification about failed application launch */
			xfce_overview_workspaces_notify(CLUTTER_ACTOR(inMenuItem),
								iconName,
								_("Launching application '%s' failed: %s"),
								g_app_info_get_display_name(appInfo),
								(error && error->message) ? error->message : _("unknown error"));
			g_warning("Launching application '%s' failed: %s",
						g_app_info_get_display_name(appInfo),
						(error && error->message) ? error->message : "unknown error");
			if(error) g_error_free(error);
		}
			else
			{
				/* Show notification about successful application launch */
				xfce_overview_workspaces_notify(CLUTTER_ACTOR(inMenuItem),
									iconName,
									_("Application '%s' launched"),
									g_app_info_get_display_name(appInfo));

				/* Emit signal for successful application launch */
				g_signal_emit_by_name(xfce_overview_workspaces_core_get_default(), "application-launched", appInfo);

				/* Quit application */
				xfce_overview_workspaces_core_quit(NULL);
			}

		/* Release allocated resources */
		g_object_unref(context);
	}

	/* Release allocated resources */
	g_object_unref(appTracker);
	g_object_unref(gicon);
}

/* A right-click might have happened on an application icon */
static void _xfce_overview_workspaces_applications_view_on_popup_menu(XfceOverviewWorkspacesApplicationsView *self,
															ClutterActor *inActor,
															gpointer inUserData)
{
	XfceOverviewWorkspacesApplicationButton				*button;
	XfceOverviewWorkspacesClickAction						*action;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_APPLICATIONS_VIEW(self));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_APPLICATION_BUTTON(inActor));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_CLICK_ACTION(inUserData));

	button=XFCE_OVERVIEW_WORKSPACES_APPLICATION_BUTTON(inActor);
	action=XFCE_OVERVIEW_WORKSPACES_CLICK_ACTION(inUserData);

	/* Check if right button was used when the application button was clicked */
	if(xfce_overview_workspaces_click_action_get_button(action)==XFCE_OVERVIEW_WORKSPACES_CLICK_ACTION_RIGHT_BUTTON)
	{
		ClutterActor							*popup;
		ClutterActor							*menuItem;
		GAppInfo								*appInfo;
		XfceOverviewWorkspacesApplicationTracker			*appTracker;

		/* Get app info for application button as it is needed most the time */
		appInfo=xfce_overview_workspaces_application_button_get_app_info(button);
		if(!appInfo)
		{
			g_critical("No application information available for clicked application button.");
			return;
		}

		/* Create pop-up menu */
		popup=xfce_overview_workspaces_popup_menu_new_for_source(CLUTTER_ACTOR(self));
		xfce_overview_workspaces_popup_menu_set_destroy_on_cancel(XFCE_OVERVIEW_WORKSPACES_POPUP_MENU(popup), TRUE);
		xfce_overview_workspaces_popup_menu_set_title(XFCE_OVERVIEW_WORKSPACES_POPUP_MENU(popup), g_app_info_get_display_name(appInfo));
		xfce_overview_workspaces_popup_menu_set_title_gicon(XFCE_OVERVIEW_WORKSPACES_POPUP_MENU(popup), g_app_info_get_icon(appInfo));

		/* Add each open window to pop-up of application */
		if(xfce_overview_workspaces_application_button_add_popup_menu_items_for_windows(button, XFCE_OVERVIEW_WORKSPACES_POPUP_MENU(popup))>0)
		{
			/* Add a separator to split windows from other actions in pop-up menu */
			menuItem=xfce_overview_workspaces_popup_menu_item_separator_new();
			clutter_actor_set_x_expand(menuItem, TRUE);
			xfce_overview_workspaces_popup_menu_add_item(XFCE_OVERVIEW_WORKSPACES_POPUP_MENU(popup), XFCE_OVERVIEW_WORKSPACES_POPUP_MENU_ITEM(menuItem));
		}

		/* Add menu item to launch application if it is not running */
		appTracker=xfce_overview_workspaces_core_get_application_tracker(NULL);
		if(!xfce_overview_workspaces_application_tracker_is_running_by_app_info(appTracker, appInfo))
		{
			menuItem=xfce_overview_workspaces_popup_menu_item_button_new();
			xfce_overview_workspaces_label_set_text(XFCE_OVERVIEW_WORKSPACES_LABEL(menuItem), _("Launch"));
			clutter_actor_set_x_expand(menuItem, TRUE);
			xfce_overview_workspaces_popup_menu_add_item(XFCE_OVERVIEW_WORKSPACES_POPUP_MENU(popup), XFCE_OVERVIEW_WORKSPACES_POPUP_MENU_ITEM(menuItem));

			g_signal_connect(menuItem,
								"activated",
								G_CALLBACK(_xfce_overview_workspaces_applications_view_on_popup_menu_item_launch),
								appInfo);
		}
		g_object_unref(appTracker);

		/* Add application actions */
		xfce_overview_workspaces_application_button_add_popup_menu_items_for_actions(button, XFCE_OVERVIEW_WORKSPACES_POPUP_MENU(popup));

		/* Activate pop-up menu */
		xfce_overview_workspaces_popup_menu_activate(XFCE_OVERVIEW_WORKSPACES_POPUP_MENU(popup));
	}
}

/* Parent menu of "All applications" was clicked */
static void _xfce_overview_workspaces_applications_view_on_all_applications_menu_parent_menu_clicked(XfceOverviewWorkspacesApplicationsView *self, gpointer inUserData)
{
	XfceOverviewWorkspacesApplicationsViewPrivate	*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_APPLICATIONS_VIEW(self));

	priv=self->priv;

	/* Go to root menu */
	priv->currentRootMenuElement=NULL;
	xfce_overview_workspaces_applications_menu_model_filter_by_section(priv->apps, NULL);
	xfce_overview_workspaces_view_scroll_to(XFCE_OVERVIEW_WORKSPACES_VIEW(self), -1, 0);

	/* Force signal emission of "filter-changed" signal at application menu
	 * model. The problem is that the "all applications" menu can only be
	 * called from root section. Setting the section to root section again
	 * by calling the model's filtering function with NULL parameter, see
	 * xfce_overview_workspaces_applications_menu_model_filter_by_section(..., NULL) above,
	 * this signal is not emitted as nothing changes. So force it here.
	 */
	g_signal_emit_by_name(priv->apps, "filter-changed");
}

/* Show sub-menu with all installed applications.
 * The list is sorted with precomputed lower-cased keys (see above) instead of
 * allocating lower-cased strings on each comparison.
 */
static GList* _xfce_overview_workspaces_applications_view_sort_all_applications(GList *inAllApps)
{
	GList								*entries;
	GList								*iter;
	GList								*sortedApps;

	/* Decorate each application with its precomputed sort keys */
	entries=NULL;
	for(iter=inAllApps; iter; iter=g_list_next(iter))
	{
		/* Takes over the reference of the input list item */
		entries=g_list_prepend(entries, _all_apps_sort_entry_new(XFCE_OVERVIEW_WORKSPACES_DESKTOP_APP_INFO(iter->data)));
	}

	/* The input list container is not needed anymore as all references
	 * were transferred to the sort entries.
	 */
	g_list_free(inAllApps);

	/* Sort entries by precomputed keys */
	entries=g_list_sort(entries, (GCompareFunc)_cmp_all_apps_sort_entry);

	/* Undecorate back to a sorted application list, keeping references */
	sortedApps=NULL;
	for(iter=entries; iter; iter=g_list_next(iter))
	{
		AllAppsSortEntry				*entry=(AllAppsSortEntry*)iter->data;

		sortedApps=g_list_prepend(sortedApps, g_object_ref(entry->appInfo));
	}
	sortedApps=g_list_reverse(sortedApps);

	/* Release sort entries and their taken-over references */
	g_list_free_full(entries, (GDestroyNotify)_all_apps_sort_entry_free);

	return(sortedApps);
}

static void _xfce_overview_workspaces_applications_view_on_all_applications_menu_clicked(XfceOverviewWorkspacesApplicationsView *self, gpointer inUserData)
{
	XfceOverviewWorkspacesApplicationsViewPrivate	*priv;
	ClutterActor						*actor;
	GList								*allApps;
	GList								*iter;
	XfceOverviewWorkspacesDesktopAppInfo			*appInfo;
	XfceOverviewWorkspacesApplicationDatabase		*appDB;
	ClutterAction						*clickAction;
	ClutterAction						*dragAction;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_APPLICATIONS_VIEW(self));

	priv=self->priv;

	/* Destroy all children */
	xfce_overview_workspaces_focusable_set_selection(XFCE_OVERVIEW_WORKSPACES_FOCUSABLE(self), NULL);
	xfce_overview_workspaces_actor_destroy_all_children(CLUTTER_ACTOR(self));
	clutter_layout_manager_layout_changed(priv->layout);

	/* Create parent menu item (via helper) */
	actor=_add_back_button(self,
							priv->parentMenuIcon,
							_("Back"),
							_("Go back to previous menu"),
							G_CALLBACK(_xfce_overview_workspaces_applications_view_on_all_applications_menu_parent_menu_clicked));

	/* Create menu items for all installed applications */
	appDB=xfce_overview_workspaces_core_get_application_database(NULL);

	allApps=xfce_overview_workspaces_application_database_get_all_applications(appDB);
	allApps=_xfce_overview_workspaces_applications_view_sort_all_applications(allApps);

	for(iter=allApps; iter; iter=g_list_next(iter))
	{
		/* Get app info of application currently iterated */
		appInfo=XFCE_OVERVIEW_WORKSPACES_DESKTOP_APP_INFO(iter->data);

		/* If desktop app info should be hidden then continue with next one */
		if(!g_app_info_should_show(G_APP_INFO(appInfo)))
		{
			continue;
		}

		/* Create actor for app info */
		actor=xfce_overview_workspaces_application_button_new_from_app_info(G_APP_INFO(appInfo));

		g_signal_connect_swapped(actor, "clicked", G_CALLBACK(_xfce_overview_workspaces_applications_view_on_item_clicked), self);

		/* Set up and add pop-up menu click action */
		clickAction=xfce_overview_workspaces_click_action_new();
		g_signal_connect_swapped(clickAction, "clicked", G_CALLBACK(_xfce_overview_workspaces_applications_view_on_popup_menu), self);
		clutter_actor_add_action(actor, clickAction);

		/* Add to view and layout */
		_xfce_overview_workspaces_applications_view_setup_actor_for_view_mode(self, CLUTTER_ACTOR(actor));
		clutter_actor_add_child(CLUTTER_ACTOR(self), CLUTTER_ACTOR(actor));
		clutter_actor_show(actor);

		/* Add drag action to actor */
		dragAction=xfce_overview_workspaces_drag_action_new_with_source(CLUTTER_ACTOR(self));
		clutter_drag_action_set_drag_threshold(CLUTTER_DRAG_ACTION(dragAction), -1, -1);
		clutter_actor_add_action(actor, dragAction);
		g_signal_connect(dragAction, "drag-begin", G_CALLBACK(_xfce_overview_workspaces_applications_view_on_drag_begin), self);
		g_signal_connect(dragAction, "drag-end", G_CALLBACK(_xfce_overview_workspaces_applications_view_on_drag_end), self);

		/* If no item was selected (i.e. no "parent menu" item) select this one
		 * which is usually the first menu item.
		 */
		if(xfce_overview_workspaces_view_has_focus(XFCE_OVERVIEW_WORKSPACES_VIEW(self)) &&
			!xfce_overview_workspaces_focusable_get_selection(XFCE_OVERVIEW_WORKSPACES_FOCUSABLE(self)))
		{
			xfce_overview_workspaces_focusable_set_selection(XFCE_OVERVIEW_WORKSPACES_FOCUSABLE(self), CLUTTER_ACTOR(actor));
		}
	}

	g_list_free_full(allApps, g_object_unref);
	g_object_unref(appDB);
}

/* Filter to display applications has changed */
static void _xfce_overview_workspaces_applications_view_on_filter_changed(XfceOverviewWorkspacesApplicationsView *self, gpointer inUserData)
{
	XfceOverviewWorkspacesApplicationsViewPrivate	*priv;
	XfceOverviewWorkspacesModelIter				*iterator;
	ClutterActor						*actor;
	GarconMenuElement					*menuElement=NULL;
	GarconMenu							*parentMenu=NULL;
	ClutterAction						*clickAction;
	ClutterAction						*dragAction;
	GAppInfo							*appInfo;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_APPLICATIONS_VIEW(self));

	priv=XFCE_OVERVIEW_WORKSPACES_APPLICATIONS_VIEW(self)->priv;

	/* Destroy all children */
	xfce_overview_workspaces_focusable_set_selection(XFCE_OVERVIEW_WORKSPACES_FOCUSABLE(self), NULL);
	xfce_overview_workspaces_actor_destroy_all_children(CLUTTER_ACTOR(self));
	clutter_layout_manager_layout_changed(priv->layout);

	/* Get parent menu */
	if(priv->currentRootMenuElement &&
		GARCON_IS_MENU(priv->currentRootMenuElement))
	{
		parentMenu=garcon_menu_get_parent(GARCON_MENU(priv->currentRootMenuElement));
	}

	/* If menu element to filter by is not the root menu element, add an "up ..." entry */
	if(parentMenu)
	{
		/* Create "parent menu" button (via helper) */
		actor=_add_back_button(self,
									priv->parentMenuIcon,
									_("Back"),
									_("Go back to previous menu"),
									G_CALLBACK(_xfce_overview_workspaces_applications_view_on_parent_menu_clicked));
	}

	if(priv->showAllAppsMenu &&
		(!priv->currentRootMenuElement || !parentMenu))
	{
		/* Create "all applications" button (via helper) */
		actor=_add_back_button(self,
									ALL_APPLICATIONS_MENU_ICON,
									_("All applications"),
									_("List of all installed applications"),
									G_CALLBACK(_xfce_overview_workspaces_applications_view_on_all_applications_menu_clicked));
	}

	/* Iterate through (filtered) data model and create actor for each entry */
	iterator=xfce_overview_workspaces_model_iter_new(XFCE_OVERVIEW_WORKSPACES_MODEL(priv->apps));
	if(iterator)
	{
		while(xfce_overview_workspaces_model_iter_next(iterator))
		{
			/* If row is filtered continue with next one immediately */
			if(!xfce_overview_workspaces_model_iter_filter(iterator)) continue;

			/* Get data from model */
			xfce_overview_workspaces_applications_menu_model_get(priv->apps,
													iterator,
													XFCE_OVERVIEW_WORKSPACES_APPLICATIONS_MENU_MODEL_COLUMN_MENU_ELEMENT, &menuElement,
													-1);

			if(!menuElement) continue;

			/* Create actor for menu element. Support drag'n'drop at actor if
			 * menu element is a menu item.
			 */
			if(GARCON_IS_MENU_ITEM(menuElement))
			{
				appInfo=xfce_overview_workspaces_desktop_app_info_new_from_menu_item(GARCON_MENU_ITEM(menuElement));
				actor=xfce_overview_workspaces_application_button_new_from_app_info(appInfo);
				g_object_unref(appInfo);

				g_signal_connect_swapped(actor, "clicked", G_CALLBACK(_xfce_overview_workspaces_applications_view_on_item_clicked), self);
			}
				else
				{
					gchar		*actorText;
					const gchar	*iconName;
					const gchar	*title;
					const gchar	*description;

					actor=xfce_overview_workspaces_button_new();

					iconName=garcon_menu_element_get_icon_name(menuElement);
					if(iconName) xfce_overview_workspaces_label_set_icon_name(XFCE_OVERVIEW_WORKSPACES_LABEL(actor), iconName);

					title=garcon_menu_element_get_name(menuElement);
					description=garcon_menu_element_get_comment(menuElement);

					if(priv->viewMode==XFCE_OVERVIEW_WORKSPACES_VIEW_MODE_LIST)
					{
						actorText=g_markup_printf_escaped(priv->formatTitleDescription,
															title ? title : "",
															description ? description : "");
					}
						else
						{
							actorText=g_markup_printf_escaped(priv->formatTitleOnly,
																title ? title : "");
						}
					xfce_overview_workspaces_label_set_text(XFCE_OVERVIEW_WORKSPACES_LABEL(actor), actorText);
					g_free(actorText);

					g_signal_connect(actor, "clicked", G_CALLBACK(_xfce_overview_workspaces_applications_view_on_menu_clicked), menuElement);
				}

			/* Add to view and layout */
			_xfce_overview_workspaces_applications_view_setup_actor_for_view_mode(self, CLUTTER_ACTOR(actor));
			clutter_actor_add_child(CLUTTER_ACTOR(self), CLUTTER_ACTOR(actor));
			clutter_actor_show(actor);

			/* Set up and add pop-up menu click action and drag action */
			if(GARCON_IS_MENU_ITEM(menuElement))
			{
				clickAction=xfce_overview_workspaces_click_action_new();
				g_signal_connect_swapped(clickAction, "clicked", G_CALLBACK(_xfce_overview_workspaces_applications_view_on_popup_menu), self);
				clutter_actor_add_action(actor, clickAction);

				dragAction=xfce_overview_workspaces_drag_action_new_with_source(CLUTTER_ACTOR(self));
				clutter_drag_action_set_drag_threshold(CLUTTER_DRAG_ACTION(dragAction), -1, -1);
				clutter_actor_add_action(actor, dragAction);
				g_signal_connect(dragAction, "drag-begin", G_CALLBACK(_xfce_overview_workspaces_applications_view_on_drag_begin), self);
				g_signal_connect(dragAction, "drag-end", G_CALLBACK(_xfce_overview_workspaces_applications_view_on_drag_end), self);
			}

			/* If no item was selected (i.e. no "parent menu" item) select this one
			 * which is usually the first menu item.
			 */
			if(xfce_overview_workspaces_view_has_focus(XFCE_OVERVIEW_WORKSPACES_VIEW(self)) &&
				!xfce_overview_workspaces_focusable_get_selection(XFCE_OVERVIEW_WORKSPACES_FOCUSABLE(self)))
			{
				xfce_overview_workspaces_focusable_set_selection(XFCE_OVERVIEW_WORKSPACES_FOCUSABLE(self), CLUTTER_ACTOR(actor));
			}

			/* Release allocated resources */
			g_object_unref(menuElement);
			menuElement=NULL;
		}
		g_object_unref(iterator);
	}
}

/* Application model has fully loaded */
static void _xfce_overview_workspaces_applications_view_on_model_loaded(XfceOverviewWorkspacesApplicationsView *self, gpointer inUserData)
{
	XfceOverviewWorkspacesApplicationsViewPrivate	*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_APPLICATIONS_VIEW(self));

	priv=XFCE_OVERVIEW_WORKSPACES_APPLICATIONS_VIEW(self)->priv;

	/* Reset to root menu as menu referenced will not be available anymore
	 * and re-filter to update view
	 */
	priv->currentRootMenuElement=NULL;
	xfce_overview_workspaces_applications_menu_model_filter_by_section(priv->apps, GARCON_MENU(priv->currentRootMenuElement));
}

/* The application will be resumed */
static void _xfce_overview_workspaces_applications_view_on_application_resume(XfceOverviewWorkspacesApplicationsView *self, gpointer inUserData)
{
	XfceOverviewWorkspacesApplicationsViewPrivate	*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_APPLICATIONS_VIEW(self));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_CORE(inUserData));

	priv=self->priv;

	/* Go to top-level entry */
	priv->currentRootMenuElement=NULL;
	xfce_overview_workspaces_applications_menu_model_filter_by_section(priv->apps, NULL);
}

/* IMPLEMENTATION: Interface XfceOverviewWorkspacesFocusable */

/* Determine if actor can get the focus */
static gboolean _xfce_overview_workspaces_applications_view_focusable_can_focus(XfceOverviewWorkspacesFocusable *inFocusable)
{
	XfceOverviewWorkspacesApplicationsView			*self;
	XfceOverviewWorkspacesFocusableInterface		*selfIface;
	XfceOverviewWorkspacesFocusableInterface		*parentIface;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_FOCUSABLE(inFocusable), FALSE);
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_APPLICATIONS_VIEW(inFocusable), FALSE);

	self=XFCE_OVERVIEW_WORKSPACES_APPLICATIONS_VIEW(inFocusable);

	/* Call parent class interface function */
	selfIface=XFCE_OVERVIEW_WORKSPACES_FOCUSABLE_GET_IFACE(inFocusable);
	parentIface=g_type_interface_peek_parent(selfIface);

	if(parentIface && parentIface->can_focus)
	{
		if(!parentIface->can_focus(inFocusable)) return(FALSE);
	}

	/* If this view is not enabled it is not focusable */
	if(!xfce_overview_workspaces_view_get_enabled(XFCE_OVERVIEW_WORKSPACES_VIEW(self))) return(FALSE);

	/* If we get here this actor can be focused */
	return(TRUE);
}

/* Determine if this actor supports selection */
static gboolean _xfce_overview_workspaces_applications_view_focusable_supports_selection(XfceOverviewWorkspacesFocusable *inFocusable)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_FOCUSABLE(inFocusable), FALSE);
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_APPLICATIONS_VIEW(inFocusable), FALSE);

	/* This actor supports selection */
	return(TRUE);
}

/* Get current selection */
static ClutterActor* _xfce_overview_workspaces_applications_view_focusable_get_selection(XfceOverviewWorkspacesFocusable *inFocusable)
{
	XfceOverviewWorkspacesApplicationsView				*self;
	XfceOverviewWorkspacesApplicationsViewPrivate		*priv;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_FOCUSABLE(inFocusable), NULL);
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_APPLICATIONS_VIEW(inFocusable), NULL);

	self=XFCE_OVERVIEW_WORKSPACES_APPLICATIONS_VIEW(inFocusable);
	priv=self->priv;

	/* Return current selection */
	return(priv->selectedItem);
}

/* Set new selection */
static gboolean _xfce_overview_workspaces_applications_view_focusable_set_selection(XfceOverviewWorkspacesFocusable *inFocusable,
																		ClutterActor *inSelection)
{
	XfceOverviewWorkspacesApplicationsView				*self;
	XfceOverviewWorkspacesApplicationsViewPrivate		*priv;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_FOCUSABLE(inFocusable), FALSE);
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_APPLICATIONS_VIEW(inFocusable), FALSE);
	g_return_val_if_fail(!inSelection || CLUTTER_IS_ACTOR(inSelection), FALSE);

	self=XFCE_OVERVIEW_WORKSPACES_APPLICATIONS_VIEW(inFocusable);
	priv=self->priv;

	/* Check that selection is a child of this actor */
	if(inSelection &&
		!clutter_actor_contains(CLUTTER_ACTOR(self), inSelection))
	{
		g_warning("%s is not a child of %s and cannot be selected",
					G_OBJECT_TYPE_NAME(inSelection),
					G_OBJECT_TYPE_NAME(self));

		return(FALSE);
	}

	/* Remove weak reference at current selection */
	if(priv->selectedItem)
	{
		g_object_remove_weak_pointer(G_OBJECT(priv->selectedItem), &priv->selectedItem);
	}

	/* Set new selection */
	priv->selectedItem=inSelection;
	if(priv->selectedItem)
	{
		/* Add weak reference at new selection */
		g_object_add_weak_pointer(G_OBJECT(priv->selectedItem), &priv->selectedItem);

		/* Ensure new selection is visible */
		xfce_overview_workspaces_view_child_ensure_visible(XFCE_OVERVIEW_WORKSPACES_VIEW(self), priv->selectedItem);
	}

	/* New selection was set successfully */
	return(TRUE);
}

/* Find requested selection target depending of current selection */
static ClutterActor* _xfce_overview_workspaces_applications_view_get_selection_from_icon_mode(XfceOverviewWorkspacesApplicationsView *self,
																				ClutterActor *inSelection,
																				XfceOverviewWorkspacesSelectionTarget inDirection)
{
	XfceOverviewWorkspacesApplicationsViewPrivate		*priv;
	ClutterActor							*selection;
	ClutterActor							*newSelection;
	gint									numberChildren;
	gint									rows;
	gint									columns;
	gint									currentSelectionIndex;
	gint									currentSelectionRow;
	gint									currentSelectionColumn;
	gint									newSelectionIndex;
	ClutterActorIter						iter;
	ClutterActor							*child;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_APPLICATIONS_VIEW(self), NULL);
	g_return_val_if_fail(CLUTTER_IS_ACTOR(inSelection), NULL);

	priv=self->priv;
	selection=inSelection;
	newSelection=NULL;

	/* Get number of rows and columns and also get number of children
	 * of layout manager.
	 */
	numberChildren=xfce_overview_workspaces_dynamic_table_layout_get_number_children(XFCE_OVERVIEW_WORKSPACES_DYNAMIC_TABLE_LAYOUT(priv->layout));
	rows=xfce_overview_workspaces_dynamic_table_layout_get_rows(XFCE_OVERVIEW_WORKSPACES_DYNAMIC_TABLE_LAYOUT(priv->layout));
	columns=xfce_overview_workspaces_dynamic_table_layout_get_columns(XFCE_OVERVIEW_WORKSPACES_DYNAMIC_TABLE_LAYOUT(priv->layout));

	/* Get index of current selection */
	currentSelectionIndex=0;
	clutter_actor_iter_init(&iter, CLUTTER_ACTOR(self));
	while(clutter_actor_iter_next(&iter, &child) &&
			child!=inSelection)
	{
		currentSelectionIndex++;
	}

	currentSelectionRow=(currentSelectionIndex / columns);
	currentSelectionColumn=(currentSelectionIndex % columns);

	/* Find target selection */
	switch(inDirection)
	{
		case XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_LEFT:
			currentSelectionColumn--;
			if(currentSelectionColumn<0)
			{
				currentSelectionRow++;
				newSelectionIndex=(currentSelectionRow*columns)-1;
			}
				else newSelectionIndex=currentSelectionIndex-1;

			newSelectionIndex=MIN(newSelectionIndex, numberChildren-1);
			newSelection=clutter_actor_get_child_at_index(CLUTTER_ACTOR(self), newSelectionIndex);
			break;

		case XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_RIGHT:
			currentSelectionColumn++;
			if(currentSelectionColumn==columns ||
				currentSelectionIndex==numberChildren)
			{
				newSelectionIndex=(currentSelectionRow*columns);
			}
				else newSelectionIndex=currentSelectionIndex+1;

			newSelectionIndex=MIN(newSelectionIndex, numberChildren-1);
			newSelection=clutter_actor_get_child_at_index(CLUTTER_ACTOR(self), newSelectionIndex);
			break;

		case XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_UP:
			currentSelectionRow--;
			if(currentSelectionRow<0) currentSelectionRow=rows-1;
			newSelectionIndex=(currentSelectionRow*columns)+currentSelectionColumn;

			newSelectionIndex=MIN(newSelectionIndex, numberChildren-1);
			newSelection=clutter_actor_get_child_at_index(CLUTTER_ACTOR(self), newSelectionIndex);
			break;

		case XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_DOWN:
			currentSelectionRow++;
			if(currentSelectionRow>=rows) currentSelectionRow=0;
			newSelectionIndex=(currentSelectionRow*columns)+currentSelectionColumn;

			newSelectionIndex=MIN(newSelectionIndex, numberChildren-1);
			newSelection=clutter_actor_get_child_at_index(CLUTTER_ACTOR(self), newSelectionIndex);
			break;

		case XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_PAGE_LEFT:
			newSelectionIndex=(currentSelectionRow*columns);
			newSelectionIndex=MIN(newSelectionIndex, numberChildren-1);
			newSelection=clutter_actor_get_child_at_index(CLUTTER_ACTOR(self), newSelectionIndex);
			break;

		case XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_PAGE_RIGHT:
			newSelectionIndex=((currentSelectionRow+1)*columns)-1;
			newSelectionIndex=MIN(newSelectionIndex, numberChildren-1);
			newSelection=clutter_actor_get_child_at_index(CLUTTER_ACTOR(self), newSelectionIndex);
			break;

		case XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_PAGE_UP:
			newSelectionIndex=currentSelectionColumn;
			newSelectionIndex=MIN(newSelectionIndex, numberChildren-1);
			newSelection=clutter_actor_get_child_at_index(CLUTTER_ACTOR(self), newSelectionIndex);
			break;

		case XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_PAGE_DOWN:
			newSelectionIndex=((rows-1)*columns)+currentSelectionColumn;
			newSelectionIndex=MIN(newSelectionIndex, numberChildren-1);
			newSelection=clutter_actor_get_child_at_index(CLUTTER_ACTOR(self), newSelectionIndex);
			break;

		default:
			{
				gchar					*valueName;

				valueName=xfce_overview_workspaces_get_enum_value_name(XFCE_OVERVIEW_WORKSPACES_TYPE_SELECTION_TARGET, inDirection);
				g_critical("Focusable object %s does not handle selection direction of type %s in icon mode.",
							G_OBJECT_TYPE_NAME(self),
							valueName);
				g_free(valueName);
			}
			break;
	}

	/* If new selection could be found override current selection with it */
	if(newSelection) selection=newSelection;

	/* Return new selection */
	XFCE_OVERVIEW_WORKSPACES_DEBUG(self, ACTOR,
						"Selecting %s at %s for current selection %s in direction %u",
						selection ? G_OBJECT_TYPE_NAME(selection) : "<nil>",
						G_OBJECT_TYPE_NAME(self),
						inSelection ? G_OBJECT_TYPE_NAME(inSelection) : "<nil>",
						inDirection);
	return(selection);
}

static ClutterActor* _xfce_overview_workspaces_applications_view_get_selection_from_list_mode(XfceOverviewWorkspacesApplicationsView *self,
																				ClutterActor *inSelection,
																				XfceOverviewWorkspacesSelectionTarget inDirection)
{
	ClutterActor							*selection;
	ClutterActor							*newSelection;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_APPLICATIONS_VIEW(self), NULL);
	g_return_val_if_fail(CLUTTER_IS_ACTOR(inSelection), NULL);

	selection=inSelection;
	newSelection=NULL;

	/* Find target selection */
	switch(inDirection)
	{
		case XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_LEFT:
		case XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_RIGHT:
		case XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_PAGE_LEFT:
		case XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_PAGE_RIGHT:
			/* Do nothing here in list mode */
			break;

		case XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_UP:
			newSelection=clutter_actor_get_previous_sibling(inSelection);
			if(!newSelection) newSelection=clutter_actor_get_last_child(CLUTTER_ACTOR(self));
			break;

		case XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_DOWN:
			newSelection=clutter_actor_get_next_sibling(inSelection);
			if(!newSelection) newSelection=clutter_actor_get_first_child(CLUTTER_ACTOR(self));
			break;

		case XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_PAGE_UP:
		case XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_PAGE_DOWN:
			{
				ClutterActor				*child;
				gfloat						topY;
				gfloat						bottomY;
				gfloat						pageSize;
				gfloat						currentY;
				gfloat						limitY;
				gfloat						childY1, childY2;
				ClutterActorIter			iter;

				/* Beginning from current selection go up and first child which needs scrolling */
				child=clutter_actor_get_previous_sibling(inSelection);
				while(child && !xfce_overview_workspaces_view_child_needs_scroll(XFCE_OVERVIEW_WORKSPACES_VIEW(self), child))
				{
					child=clutter_actor_get_previous_sibling(child);
				}
				if(!child) child=clutter_actor_get_first_child(CLUTTER_ACTOR(self));
				topY=clutter_actor_get_y(child);

				/* Beginning from current selection go down and first child which needs scrolling */
				child=clutter_actor_get_next_sibling(inSelection);
				while(child && !xfce_overview_workspaces_view_child_needs_scroll(XFCE_OVERVIEW_WORKSPACES_VIEW(self), child))
				{
					child=clutter_actor_get_next_sibling(child);
				}
				if(!child) child=clutter_actor_get_last_child(CLUTTER_ACTOR(self));
				bottomY=clutter_actor_get_y(child);

				/* Get distance between top and bottom actor we found because that's the page size */
				pageSize=bottomY-topY;

				/* Find child in distance of page size from current selection */
				currentY=clutter_actor_get_y(inSelection);

				if(inDirection==XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_PAGE_UP) limitY=currentY-pageSize;
					else limitY=currentY+pageSize;

				clutter_actor_iter_init(&iter, CLUTTER_ACTOR(self));
				while(!newSelection && clutter_actor_iter_next(&iter, &child))
				{
					childY1=clutter_actor_get_y(child);
					childY2=childY1+clutter_actor_get_height(child);
					if(childY1>limitY || childY2>limitY) newSelection=child;
				}

				/* If no child could be found select last one */
				if(!newSelection)
				{
					if(inDirection==XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_PAGE_UP)
					{
						newSelection=clutter_actor_get_first_child(CLUTTER_ACTOR(self));
					}
						else
						{
							newSelection=clutter_actor_get_last_child(CLUTTER_ACTOR(self));
						}
				}
			}
			break;

		default:
			{
				gchar					*valueName;

				valueName=xfce_overview_workspaces_get_enum_value_name(XFCE_OVERVIEW_WORKSPACES_TYPE_SELECTION_TARGET, inDirection);
				g_critical("Focusable object %s does not handle selection direction of type %s in list mode.",
							G_OBJECT_TYPE_NAME(self),
							valueName);
				g_free(valueName);
			}
			break;
	}

	/* If new selection could be found override current selection with it */
	if(newSelection) selection=newSelection;

	/* Return new selection */
	XFCE_OVERVIEW_WORKSPACES_DEBUG(self, ACTOR,
						"Selecting %s at %s for current selection %s in direction %u",
						selection ? G_OBJECT_TYPE_NAME(selection) : "<nil>",
						G_OBJECT_TYPE_NAME(self),
						inSelection ? G_OBJECT_TYPE_NAME(inSelection) : "<nil>",
						inDirection);
	return(selection);
}

static ClutterActor* _xfce_overview_workspaces_applications_view_focusable_find_selection(XfceOverviewWorkspacesFocusable *inFocusable,
																				ClutterActor *inSelection,
																				XfceOverviewWorkspacesSelectionTarget inDirection)
{
	XfceOverviewWorkspacesApplicationsView				*self;
	XfceOverviewWorkspacesApplicationsViewPrivate		*priv;
	ClutterActor							*selection;
	ClutterActor							*newSelection;
	gchar									*valueName;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_FOCUSABLE(inFocusable), NULL);
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_APPLICATIONS_VIEW(inFocusable), NULL);
	g_return_val_if_fail(!inSelection || CLUTTER_IS_ACTOR(inSelection), NULL);
	g_return_val_if_fail(inDirection>=0 && inDirection<=XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_NEXT, NULL);

	self=XFCE_OVERVIEW_WORKSPACES_APPLICATIONS_VIEW(inFocusable);
	priv=self->priv;
	selection=inSelection;
	newSelection=NULL;

	/* If there is nothing selected, select first actor and return */
	if(!inSelection)
	{
		newSelection=clutter_actor_get_first_child(CLUTTER_ACTOR(self));

		valueName=xfce_overview_workspaces_get_enum_value_name(XFCE_OVERVIEW_WORKSPACES_TYPE_SELECTION_TARGET, inDirection);
		XFCE_OVERVIEW_WORKSPACES_DEBUG(self, ACTOR,
							"No selection at %s, so select first child %s for direction %s",
							G_OBJECT_TYPE_NAME(self),
							newSelection ? G_OBJECT_TYPE_NAME(newSelection) : "<nil>",
							valueName);
		g_free(valueName);

		return(newSelection);
	}

	/* Check that selection is a child of this actor otherwise return NULL */
	if(!clutter_actor_contains(CLUTTER_ACTOR(self), inSelection))
	{
		ClutterActor						*parent;

		parent=clutter_actor_get_parent(inSelection);
		g_warning("Cannot lookup selection target at %s because %s is a child of %s",
					G_OBJECT_TYPE_NAME(self),
					G_OBJECT_TYPE_NAME(inSelection),
					parent ? G_OBJECT_TYPE_NAME(parent) : "<nil>");

		return(NULL);
	}

	/* Find target selection */
	switch(inDirection)
	{
		case XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_LEFT:
		case XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_RIGHT:
		case XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_UP:
		case XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_DOWN:
		case XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_PAGE_LEFT:
		case XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_PAGE_RIGHT:
		case XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_PAGE_UP:
		case XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_PAGE_DOWN:
			if(priv->viewMode==XFCE_OVERVIEW_WORKSPACES_VIEW_MODE_LIST)
			{
				newSelection=_xfce_overview_workspaces_applications_view_get_selection_from_list_mode(self, inSelection, inDirection);
			}
				else
				{
					newSelection=_xfce_overview_workspaces_applications_view_get_selection_from_icon_mode(self, inSelection, inDirection);
				}
			break;

		case XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_FIRST:
			newSelection=clutter_actor_get_first_child(CLUTTER_ACTOR(self));
			break;

		case XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_LAST:
			newSelection=clutter_actor_get_last_child(CLUTTER_ACTOR(self));
			break;

		case XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_NEXT:
			newSelection=clutter_actor_get_next_sibling(inSelection);
			if(!newSelection) newSelection=clutter_actor_get_previous_sibling(inSelection);
			break;

		default:
			{
				valueName=xfce_overview_workspaces_get_enum_value_name(XFCE_OVERVIEW_WORKSPACES_TYPE_SELECTION_TARGET, inDirection);
				g_critical("Focusable object %s does not handle selection direction of type %s.",
							G_OBJECT_TYPE_NAME(self),
							valueName);
				g_free(valueName);
			}
			break;
	}

	/* If new selection could be found override current selection with it */
	if(newSelection) selection=newSelection;

	/* Return new selection found */
	XFCE_OVERVIEW_WORKSPACES_DEBUG(self, ACTOR,
						"Selecting %s at %s for current selection %s in direction %u",
						selection ? G_OBJECT_TYPE_NAME(selection) : "<nil>",
						G_OBJECT_TYPE_NAME(self),
						inSelection ? G_OBJECT_TYPE_NAME(inSelection) : "<nil>",
						inDirection);

	return(selection);
}

/* Activate selection */
static gboolean _xfce_overview_workspaces_applications_view_focusable_activate_selection(XfceOverviewWorkspacesFocusable *inFocusable,
																			ClutterActor *inSelection)
{
	XfceOverviewWorkspacesApplicationsView				*self;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_FOCUSABLE(inFocusable), FALSE);
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_APPLICATIONS_VIEW(inFocusable), FALSE);
	g_return_val_if_fail(CLUTTER_IS_ACTOR(inSelection), FALSE);

	self=XFCE_OVERVIEW_WORKSPACES_APPLICATIONS_VIEW(inFocusable);

	/* Check that selection is a child of this actor */
	if(!clutter_actor_contains(CLUTTER_ACTOR(self), inSelection))
	{
		ClutterActor						*parent;

		parent=clutter_actor_get_parent(inSelection);
		g_warning("%s is a child of %s and cannot be activated at %s",
					G_OBJECT_TYPE_NAME(inSelection),
					parent ? G_OBJECT_TYPE_NAME(parent) : "<nil>",
					G_OBJECT_TYPE_NAME(self));

		return(FALSE);
	}

	/* Activate selection */
	g_signal_emit_by_name(inSelection, "clicked");

	return(TRUE);
}

/* Interface initialization
 * Set up default functions
 */
void _xfce_overview_workspaces_applications_view_focusable_iface_init(XfceOverviewWorkspacesFocusableInterface *iface)
{
	iface->can_focus=_xfce_overview_workspaces_applications_view_focusable_can_focus;

	iface->supports_selection=_xfce_overview_workspaces_applications_view_focusable_supports_selection;
	iface->get_selection=_xfce_overview_workspaces_applications_view_focusable_get_selection;
	iface->set_selection=_xfce_overview_workspaces_applications_view_focusable_set_selection;
	iface->find_selection=_xfce_overview_workspaces_applications_view_focusable_find_selection;
	iface->activate_selection=_xfce_overview_workspaces_applications_view_focusable_activate_selection;
}

/* IMPLEMENTATION: GObject */

/* Dispose this object */
static void _xfce_overview_workspaces_applications_view_dispose(GObject *inObject)
{
	XfceOverviewWorkspacesApplicationsView			*self=XFCE_OVERVIEW_WORKSPACES_APPLICATIONS_VIEW(inObject);
	XfceOverviewWorkspacesApplicationsViewPrivate	*priv=self->priv;

	/* Release allocated resources */
	if(priv->selectedItem)
	{
		g_object_remove_weak_pointer(G_OBJECT(priv->selectedItem), &priv->selectedItem);
		priv->selectedItem=NULL;
	}

	if(priv->layout)
	{
		priv->layout=NULL;
	}

	if(priv->settingsShowAllAppsMenuBinding)
	{
		g_object_unref(priv->settingsShowAllAppsMenuBinding);
		priv->settingsShowAllAppsMenuBinding=NULL;
	}

	if(priv->apps)
	{
		g_object_unref(priv->apps);
		priv->apps=NULL;
	}

	if(priv->parentMenuIcon)
	{
		g_free(priv->parentMenuIcon);
		priv->parentMenuIcon=NULL;
	}

	if(priv->formatTitleDescription)
	{
		g_free(priv->formatTitleDescription);
		priv->formatTitleDescription=NULL;
	}

	if(priv->formatTitleOnly)
	{
		g_free(priv->formatTitleOnly);
		priv->formatTitleOnly=NULL;
	}

	/* Call parent's class dispose method */
	G_OBJECT_CLASS(xfce_overview_workspaces_applications_view_parent_class)->dispose(inObject);
}

/* Set/get properties */
static void _xfce_overview_workspaces_applications_view_set_property(GObject *inObject,
														guint inPropID,
														const GValue *inValue,
														GParamSpec *inSpec)
{
	XfceOverviewWorkspacesApplicationsView				*self;

	self=XFCE_OVERVIEW_WORKSPACES_APPLICATIONS_VIEW(inObject);

	switch(inPropID)
	{
		case PROP_VIEW_MODE:
			xfce_overview_workspaces_applications_view_set_view_mode(self, (XfceOverviewWorkspacesViewMode)g_value_get_enum(inValue));
			break;

		case PROP_SPACING:
			xfce_overview_workspaces_applications_view_set_spacing(self, g_value_get_float(inValue));
			break;

		case PROP_PARENT_MENU_ICON:
			xfce_overview_workspaces_applications_view_set_parent_menu_icon(self, g_value_get_string(inValue));
			break;

		case PROP_FORMAT_TITLE_ONLY:
			xfce_overview_workspaces_applications_view_set_format_title_only(self, g_value_get_string(inValue));
			break;

		case PROP_FORMAT_TITLE_DESCRIPTION:
			xfce_overview_workspaces_applications_view_set_format_title_description(self, g_value_get_string(inValue));
			break;

		case PROP_SHOW_ALL_APPS:
			xfce_overview_workspaces_applications_view_set_show_all_apps(self, g_value_get_boolean(inValue));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(inObject, inPropID, inSpec);
			break;
	}
}

static void _xfce_overview_workspaces_applications_view_get_property(GObject *inObject,
														guint inPropID,
														GValue *outValue,
														GParamSpec *inSpec)
{
	XfceOverviewWorkspacesApplicationsView				*self;
	XfceOverviewWorkspacesApplicationsViewPrivate		*priv;

	self=XFCE_OVERVIEW_WORKSPACES_APPLICATIONS_VIEW(inObject);
	priv=self->priv;

	switch(inPropID)
	{
		case PROP_VIEW_MODE:
			g_value_set_enum(outValue, priv->viewMode);
			break;

		case PROP_SPACING:
			g_value_set_float(outValue, priv->spacing);
			break;

		case PROP_PARENT_MENU_ICON:
			g_value_set_string(outValue, priv->parentMenuIcon);
			break;

		case PROP_FORMAT_TITLE_ONLY:
			g_value_set_string(outValue, priv->formatTitleOnly);
			break;

		case PROP_FORMAT_TITLE_DESCRIPTION:
			g_value_set_string(outValue, priv->formatTitleDescription);
			break;

		case PROP_SHOW_ALL_APPS:
			g_value_set_boolean(outValue, priv->showAllAppsMenu);
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(inObject, inPropID, inSpec);
			break;
	}
}

/* Class initialization
 * Override functions in parent classes and define properties
 * and signals
 */
static void xfce_overview_workspaces_applications_view_class_init(XfceOverviewWorkspacesApplicationsViewClass *klass)
{
	XfceOverviewWorkspacesActorClass	*actorClass=XFCE_OVERVIEW_WORKSPACES_ACTOR_CLASS(klass);
	GObjectClass			*gobjectClass=G_OBJECT_CLASS(klass);

	/* Override functions */
	gobjectClass->dispose=_xfce_overview_workspaces_applications_view_dispose;
	gobjectClass->set_property=_xfce_overview_workspaces_applications_view_set_property;
	gobjectClass->get_property=_xfce_overview_workspaces_applications_view_get_property;

	/* Define properties */
	XfceOverviewWorkspacesApplicationsViewProperties[PROP_VIEW_MODE]=
		g_param_spec_enum("view-mode",
							"View mode",
							"The view mode used in this view",
							XFCE_OVERVIEW_WORKSPACES_TYPE_VIEW_MODE,
							XFCE_OVERVIEW_WORKSPACES_VIEW_MODE_LIST,
							G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	XfceOverviewWorkspacesApplicationsViewProperties[PROP_SPACING]=
		g_param_spec_float("spacing",
							"Spacing",
							"Spacing between each element in view",
							0.0f, G_MAXFLOAT,
							0.0f,
							G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	XfceOverviewWorkspacesApplicationsViewProperties[PROP_PARENT_MENU_ICON]=
		g_param_spec_string("parent-menu-icon",
								"Parent menu icon",
								"Name of icon to use for 'go-back-to-parent-menu' entries",
								NULL,
								G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	XfceOverviewWorkspacesApplicationsViewProperties[PROP_FORMAT_TITLE_ONLY]=
		g_param_spec_string("format-title-only",
								"Format title only",
								"Format string used when only title is display",
								NULL,
								G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	XfceOverviewWorkspacesApplicationsViewProperties[PROP_FORMAT_TITLE_DESCRIPTION]=
		g_param_spec_string("format-title-description",
								"Format title and description",
								"Format string used when title and description is display. First argument is title and second one is description.",
								NULL,
								G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	XfceOverviewWorkspacesApplicationsViewProperties[PROP_SHOW_ALL_APPS]=
		g_param_spec_boolean("show-all-apps",
								"Show all applications",
								"Whether to show a menu for all installed applications at root menu",
								FALSE,
								G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(gobjectClass, PROP_LAST, XfceOverviewWorkspacesApplicationsViewProperties);

	/* Define stylable properties */
	xfce_overview_workspaces_actor_install_stylable_property(actorClass, XfceOverviewWorkspacesApplicationsViewProperties[PROP_VIEW_MODE]);
	xfce_overview_workspaces_actor_install_stylable_property(actorClass, XfceOverviewWorkspacesApplicationsViewProperties[PROP_SPACING]);
	xfce_overview_workspaces_actor_install_stylable_property(actorClass, XfceOverviewWorkspacesApplicationsViewProperties[PROP_PARENT_MENU_ICON]);
	xfce_overview_workspaces_actor_install_stylable_property(actorClass, XfceOverviewWorkspacesApplicationsViewProperties[PROP_FORMAT_TITLE_ONLY]);
	xfce_overview_workspaces_actor_install_stylable_property(actorClass, XfceOverviewWorkspacesApplicationsViewProperties[PROP_FORMAT_TITLE_DESCRIPTION]);
}

/* Object initialization
 * Create private structure and set up default values
 */
static void xfce_overview_workspaces_applications_view_init(XfceOverviewWorkspacesApplicationsView *self)
{
	XfceOverviewWorkspacesApplicationsViewPrivate	*priv;
	XfceOverviewWorkspacesCore						*core;
	XfceOverviewWorkspacesSettings					*settings;

	self->priv=priv=xfce_overview_workspaces_applications_view_get_instance_private(self);

	/* Set up default values */
	priv->apps=XFCE_OVERVIEW_WORKSPACES_APPLICATIONS_MENU_MODEL(xfce_overview_workspaces_applications_menu_model_new());
	priv->currentRootMenuElement=NULL;
	priv->viewMode=-1;
	priv->spacing=0.0f;
	priv->parentMenuIcon=NULL;
	priv->formatTitleOnly=g_strdup("%s");
	priv->formatTitleDescription=g_strdup("%s\n%s");
	priv->selectedItem=NULL;
	priv->showAllAppsMenu=FALSE;
	priv->settingsShowAllAppsMenuBinding=NULL;

	/* Set up view */
	xfce_overview_workspaces_view_set_name(XFCE_OVERVIEW_WORKSPACES_VIEW(self), _("Applications"));
	xfce_overview_workspaces_view_set_icon(XFCE_OVERVIEW_WORKSPACES_VIEW(self), "go-home");

	/* Set up actor */
	xfce_overview_workspaces_actor_set_can_focus(XFCE_OVERVIEW_WORKSPACES_ACTOR(self), TRUE);

	xfce_overview_workspaces_view_set_view_fit_mode(XFCE_OVERVIEW_WORKSPACES_VIEW(self), XFCE_OVERVIEW_WORKSPACES_VIEW_FIT_MODE_HORIZONTAL);
	xfce_overview_workspaces_applications_view_set_view_mode(self, XFCE_OVERVIEW_WORKSPACES_VIEW_MODE_LIST);

	/* Connect signals */
	g_signal_connect_swapped(priv->apps, "filter-changed", G_CALLBACK(_xfce_overview_workspaces_applications_view_on_filter_changed), self);
	g_signal_connect_swapped(priv->apps, "loaded", G_CALLBACK(_xfce_overview_workspaces_applications_view_on_model_loaded), self);

	/* Connect signal to core */
	core=xfce_overview_workspaces_core_get_default();
	g_signal_connect_swapped(core, "resume", G_CALLBACK(_xfce_overview_workspaces_applications_view_on_application_resume), self);

	/* Bind to settings to react on changes */
	settings=xfce_overview_workspaces_core_get_settings(NULL);
	priv->settingsShowAllAppsMenuBinding=
		g_object_bind_property(settings,
								"show-all-applications",
								self,
								"show-all-apps",
								G_BINDING_SYNC_CREATE);
}

/* Get/set view mode of view */
XfceOverviewWorkspacesViewMode xfce_overview_workspaces_applications_view_get_view_mode(XfceOverviewWorkspacesApplicationsView *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_APPLICATIONS_VIEW(self), XFCE_OVERVIEW_WORKSPACES_VIEW_MODE_LIST);

	return(self->priv->viewMode);
}

void xfce_overview_workspaces_applications_view_set_view_mode(XfceOverviewWorkspacesApplicationsView *self, const XfceOverviewWorkspacesViewMode inMode)
{
	XfceOverviewWorkspacesApplicationsViewPrivate	*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_APPLICATIONS_VIEW(self));
	g_return_if_fail(inMode<=XFCE_OVERVIEW_WORKSPACES_VIEW_MODE_ICON);

	priv=self->priv;

	/* Set value if changed */
	if(priv->viewMode!=inMode)
	{
		/* Set value */
		if(priv->layout)
		{
			clutter_actor_set_layout_manager(CLUTTER_ACTOR(self), NULL);
			priv->layout=NULL;
		}

		priv->viewMode=inMode;

		/* Set new layout manager */
		switch(priv->viewMode)
		{
			case XFCE_OVERVIEW_WORKSPACES_VIEW_MODE_LIST:
				priv->layout=clutter_box_layout_new();
				clutter_box_layout_set_orientation(CLUTTER_BOX_LAYOUT(priv->layout), CLUTTER_ORIENTATION_VERTICAL);
				clutter_box_layout_set_spacing(CLUTTER_BOX_LAYOUT(priv->layout), priv->spacing);
				clutter_actor_set_layout_manager(CLUTTER_ACTOR(self), priv->layout);
				break;

			case XFCE_OVERVIEW_WORKSPACES_VIEW_MODE_ICON:
				priv->layout=xfce_overview_workspaces_dynamic_table_layout_new();
				xfce_overview_workspaces_dynamic_table_layout_set_spacing(XFCE_OVERVIEW_WORKSPACES_DYNAMIC_TABLE_LAYOUT(priv->layout), priv->spacing);
				clutter_actor_set_layout_manager(CLUTTER_ACTOR(self), priv->layout);
				break;

			default:
				g_assert_not_reached();
		}

		/* Rebuild view */
		_xfce_overview_workspaces_applications_view_on_filter_changed(self, NULL);

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesApplicationsViewProperties[PROP_VIEW_MODE]);
	}
}

/* Get/set spacing between elements */
gfloat xfce_overview_workspaces_applications_view_get_spacing(XfceOverviewWorkspacesApplicationsView *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_APPLICATIONS_VIEW(self), 0.0f);

	return(self->priv->spacing);
}

void xfce_overview_workspaces_applications_view_set_spacing(XfceOverviewWorkspacesApplicationsView *self, const gfloat inSpacing)
{
	XfceOverviewWorkspacesApplicationsViewPrivate		*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_APPLICATIONS_VIEW(self));
	g_return_if_fail(inSpacing>=0.0f);

	priv=self->priv;

	/* Set value if changed */
	if(priv->spacing!=inSpacing)
	{
		/* Set value */
		priv->spacing=inSpacing;

		/* Update layout manager */
		switch(priv->viewMode)
		{
			case XFCE_OVERVIEW_WORKSPACES_VIEW_MODE_LIST:
				clutter_box_layout_set_spacing(CLUTTER_BOX_LAYOUT(priv->layout), priv->spacing);
				break;

			case XFCE_OVERVIEW_WORKSPACES_VIEW_MODE_ICON:
				xfce_overview_workspaces_dynamic_table_layout_set_spacing(XFCE_OVERVIEW_WORKSPACES_DYNAMIC_TABLE_LAYOUT(priv->layout), priv->spacing);
				break;

			default:
				g_assert_not_reached();
		}

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesApplicationsViewProperties[PROP_SPACING]);
	}
}

/* IMPLEMENTATION: Public API */

/* Get/set icon name for 'go-back-to-parent-menu' entries */
const gchar* xfce_overview_workspaces_applications_view_get_parent_menu_icon(XfceOverviewWorkspacesApplicationsView *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_APPLICATIONS_VIEW(self), NULL);

	return(self->priv->parentMenuIcon);
}

void xfce_overview_workspaces_applications_view_set_parent_menu_icon(XfceOverviewWorkspacesApplicationsView *self, const gchar *inIconName)
{
	XfceOverviewWorkspacesApplicationsViewPrivate		*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_APPLICATIONS_VIEW(self));

	priv=self->priv;

	/* Set value if changed */
	if(g_strcmp0(priv->parentMenuIcon, inIconName)!=0)
	{
		/* Set value */
		if(priv->parentMenuIcon)
		{
			g_free(priv->parentMenuIcon);
			priv->parentMenuIcon=NULL;
		}

		if(inIconName) priv->parentMenuIcon=g_strdup(inIconName);

		/* Update actor */
		_xfce_overview_workspaces_applications_view_on_filter_changed(self, NULL);

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesApplicationsViewProperties[PROP_PARENT_MENU_ICON]);
	}
}

/* Get/set format string to use when displaying only title */
const gchar* xfce_overview_workspaces_applications_view_get_format_title_only(XfceOverviewWorkspacesApplicationsView *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_APPLICATIONS_VIEW(self), NULL);

	return(self->priv->formatTitleOnly);
}

void xfce_overview_workspaces_applications_view_set_format_title_only(XfceOverviewWorkspacesApplicationsView *self, const gchar *inFormat)
{
	XfceOverviewWorkspacesApplicationsViewPrivate		*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_APPLICATIONS_VIEW(self));
	g_return_if_fail(inFormat);

	priv=self->priv;

	/* Set value if changed */
	if(g_strcmp0(priv->formatTitleOnly, inFormat)!=0)
	{
		/* Set value */
		if(priv->formatTitleOnly) g_free(priv->formatTitleOnly);
		priv->formatTitleOnly=g_strdup(inFormat);

		/* Update view only if view mode is list which uses this format string */
		if(priv->viewMode==XFCE_OVERVIEW_WORKSPACES_VIEW_MODE_ICON) _xfce_overview_workspaces_applications_view_on_filter_changed(self, NULL);

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesApplicationsViewProperties[PROP_FORMAT_TITLE_ONLY]);
	}
}

/* Get/set format string to use when displaying title and description */
const gchar* xfce_overview_workspaces_applications_view_get_format_title_description(XfceOverviewWorkspacesApplicationsView *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_APPLICATIONS_VIEW(self), NULL);

	return(self->priv->formatTitleDescription);
}

void xfce_overview_workspaces_applications_view_set_format_title_description(XfceOverviewWorkspacesApplicationsView *self, const gchar *inFormat)
{
	XfceOverviewWorkspacesApplicationsViewPrivate		*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_APPLICATIONS_VIEW(self));
	g_return_if_fail(inFormat);

	priv=self->priv;

	/* Set value if changed */
	if(g_strcmp0(priv->formatTitleDescription, inFormat)!=0)
	{
		/* Set value */
		if(priv->formatTitleDescription) g_free(priv->formatTitleDescription);
		priv->formatTitleDescription=g_strdup(inFormat);

		/* Update view only if view mode is list which uses this format string */
		if(priv->viewMode==XFCE_OVERVIEW_WORKSPACES_VIEW_MODE_LIST) _xfce_overview_workspaces_applications_view_on_filter_changed(self, NULL);

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesApplicationsViewProperties[PROP_FORMAT_TITLE_DESCRIPTION]);
	}
}

/* Get/set flag whether to show an "all applications" menu at root menu */
gboolean xfce_overview_workspaces_applications_view_get_show_all_apps(XfceOverviewWorkspacesApplicationsView *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_APPLICATIONS_VIEW(self), FALSE);

	return(self->priv->showAllAppsMenu);
}

void xfce_overview_workspaces_applications_view_set_show_all_apps(XfceOverviewWorkspacesApplicationsView *self, gboolean inShowAllApps)
{
	XfceOverviewWorkspacesApplicationsViewPrivate		*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_APPLICATIONS_VIEW(self));

	priv=self->priv;

	/* Set value if changed */
	if(priv->showAllAppsMenu!=inShowAllApps)
	{
		/* Set value */
		priv->showAllAppsMenu=inShowAllApps;

		/* Update view if currently at root menu */
		if(!priv->currentRootMenuElement ||
			!garcon_menu_get_parent(GARCON_MENU(priv->currentRootMenuElement)))
		{
			_xfce_overview_workspaces_applications_view_on_filter_changed(self, NULL);
		}

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesApplicationsViewProperties[PROP_SHOW_ALL_APPS]);
	}
}
