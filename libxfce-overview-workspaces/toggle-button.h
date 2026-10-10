/*
 * toggle-button: A button which can toggle its state between on and off
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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_TOGGLE_BUTTON__
#define __LIBXFCE_OVERVIEW_WORKSPACES_TOGGLE_BUTTON__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <libxfce-overview-workspaces/button.h>

G_BEGIN_DECLS

#define XFCE_OVERVIEW_WORKSPACES_TYPE_TOGGLE_BUTTON				(xfce_overview_workspaces_toggle_button_get_type())
#define XFCE_OVERVIEW_WORKSPACES_TOGGLE_BUTTON(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_TOGGLE_BUTTON, XfceOverviewWorkspacesToggleButton))
#define XFCE_OVERVIEW_WORKSPACES_IS_TOGGLE_BUTTON(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_TOGGLE_BUTTON))
#define XFCE_OVERVIEW_WORKSPACES_TOGGLE_BUTTON_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_TOGGLE_BUTTON, XfceOverviewWorkspacesToggleButtonClass))
#define XFCE_OVERVIEW_WORKSPACES_IS_TOGGLE_BUTTON_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_TOGGLE_BUTTON))
#define XFCE_OVERVIEW_WORKSPACES_TOGGLE_BUTTON_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_TOGGLE_BUTTON, XfceOverviewWorkspacesToggleButtonClass))

typedef struct _XfceOverviewWorkspacesToggleButton				XfceOverviewWorkspacesToggleButton;
typedef struct _XfceOverviewWorkspacesToggleButtonClass		XfceOverviewWorkspacesToggleButtonClass;
typedef struct _XfceOverviewWorkspacesToggleButtonPrivate		XfceOverviewWorkspacesToggleButtonPrivate;

/**
 * XfceOverviewWorkspacesToggleButton:
 *
 * The #XfceOverviewWorkspacesToggleButton structure contains only private data and
 * should be accessed using the provided API
 */
struct _XfceOverviewWorkspacesToggleButton
{
	/*< private >*/
	/* Parent instance */
	XfceOverviewWorkspacesButton				parent_instance;

	/* Private structure */
	XfceOverviewWorkspacesToggleButtonPrivate	*priv;
};

/**
 * XfceOverviewWorkspacesToggleButtonClass:
 * @toggled: Class handler for the #XfceOverviewWorkspacesToggleButtonClass::toggled signal
 *
 * The #XfceOverviewWorkspacesToggleButtonClass structure contains only private data
 */
struct _XfceOverviewWorkspacesToggleButtonClass
{
	/*< private >*/
	/* Parent class */
	XfceOverviewWorkspacesButtonClass			parent_class;

	/*< public >*/
	/* Virtual functions */
	void (*toggled)(XfceOverviewWorkspacesToggleButton *self);
};

/* Public API */
GType xfce_overview_workspaces_toggle_button_get_type(void) G_GNUC_CONST;

ClutterActor* xfce_overview_workspaces_toggle_button_new(void);
ClutterActor* xfce_overview_workspaces_toggle_button_new_with_text(const gchar *inText);
ClutterActor* xfce_overview_workspaces_toggle_button_new_with_icon_name(const gchar *inIconName);
ClutterActor* xfce_overview_workspaces_toggle_button_new_with_gicon(GIcon *inIcon);
ClutterActor* xfce_overview_workspaces_toggle_button_new_full_with_icon_name(const gchar *inIconName, const gchar *inText);
ClutterActor* xfce_overview_workspaces_toggle_button_new_full_with_gicon(GIcon *inIcon, const gchar *inText);

gboolean xfce_overview_workspaces_toggle_button_get_toggle_state(XfceOverviewWorkspacesToggleButton *self);
void xfce_overview_workspaces_toggle_button_set_toggle_state(XfceOverviewWorkspacesToggleButton *self, gboolean inToggleState);

gboolean xfce_overview_workspaces_toggle_button_get_auto_toggle(XfceOverviewWorkspacesToggleButton *self);
void xfce_overview_workspaces_toggle_button_set_auto_toggle(XfceOverviewWorkspacesToggleButton *self, gboolean inAuto);

void xfce_overview_workspaces_toggle_button_toggle(XfceOverviewWorkspacesToggleButton *self);

G_END_DECLS

#endif	/* __LIBXFCE_OVERVIEW_WORKSPACES_TOGGLE_BUTTON__ */
