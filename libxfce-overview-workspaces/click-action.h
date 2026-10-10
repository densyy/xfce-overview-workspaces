/*
 * click-action: Bad workaround for click action which prevent drag actions
 *               to work properly since clutter version 1.12 at least.
 *               This object/file is a complete copy of the original
 *               clutter-click-action.{c,h} files of clutter 1.12 except
 *               for one line, the renamed function names and the applied
 *               coding style. The clutter-click-action.{c,h} files of
 *               later clutter versions do not differ much from this one
 *               so this object should work also for this versions.
 *
 *               See bug: https://bugzilla.gnome.org/show_bug.cgi?id=714993
 * 
 * Copyright 2012-2021 Stephan Haller <nomad@froevel.de>
 *         original by Emmanuele Bassi <ebassi@linux.intel.com>
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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_CLICK_ACTION__
#define __LIBXFCE_OVERVIEW_WORKSPACES_CLICK_ACTION__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <clutter/clutter.h>

G_BEGIN_DECLS

/* Public definitions */
/**
 * XFCE_OVERVIEW_WORKSPACES_CLICK_ACTION_LEFT_BUTTON:
 *
 * A helper macro to determine left button clicks using
 * the function xfce_overview_workspaces_click_action_get_button().
 * The value for this macro is an unsinged integer.
 **/
#define XFCE_OVERVIEW_WORKSPACES_CLICK_ACTION_LEFT_BUTTON 		((guint)1)

/**
 * XFCE_OVERVIEW_WORKSPACES_CLICK_ACTION_MIDDLE_BUTTON:
 *
 * A helper macro to determine middle button clicks using
 * the function xfce_overview_workspaces_click_action_get_button().
 * The value for this macro is an unsinged integer.
 **/
#define XFCE_OVERVIEW_WORKSPACES_CLICK_ACTION_MIDDLE_BUTTON 		((guint)2)

/**
 * XFCE_OVERVIEW_WORKSPACES_CLICK_ACTION_RIGHT_BUTTON:
 *
 * A helper macro to determine right button clicks using
 * the function xfce_overview_workspaces_click_action_get_button().
 * The value for this macro is an unsinged integer.
 **/
#define XFCE_OVERVIEW_WORKSPACES_CLICK_ACTION_RIGHT_BUTTON 		((guint)3)


/* Object declaration */
#define XFCE_OVERVIEW_WORKSPACES_TYPE_CLICK_ACTION				(xfce_overview_workspaces_click_action_get_type ())
#define XFCE_OVERVIEW_WORKSPACES_CLICK_ACTION(obj)				(G_TYPE_CHECK_INSTANCE_CAST ((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_CLICK_ACTION, XfceOverviewWorkspacesClickAction))
#define XFCE_OVERVIEW_WORKSPACES_IS_CLICK_ACTION(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_CLICK_ACTION))
#define XFCE_OVERVIEW_WORKSPACES_CLICK_ACTION_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_CLICK_ACTION, XfceOverviewWorkspacesClickActionClass))
#define XFCE_OVERVIEW_WORKSPACES_IS_CLICK_ACTION_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_CLICK_ACTION))
#define XFCE_OVERVIEW_WORKSPACES_CLICK_ACTION_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS ((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_CLICK_ACTION, XfceOverviewWorkspacesClickActionClass))

typedef struct _XfceOverviewWorkspacesClickAction				XfceOverviewWorkspacesClickAction;
typedef struct _XfceOverviewWorkspacesClickActionPrivate		XfceOverviewWorkspacesClickActionPrivate;
typedef struct _XfceOverviewWorkspacesClickActionClass			XfceOverviewWorkspacesClickActionClass;

/**
 * XfceOverviewWorkspacesClickAction:
 *
 * The #XfceOverviewWorkspacesClickAction structure contains only private data and
 * should be accessed using the provided API
 */
struct _XfceOverviewWorkspacesClickAction
{
	/*< private >*/
	/* Parent instance */
	ClutterAction					parent_instance;

	XfceOverviewWorkspacesClickActionPrivate	*priv;
};

/**
 * XfceOverviewWorkspacesClickActionClass:
 * @clicked: Class handler for the #XfceOverviewWorkspacesClickActionClass::clicked signal
 * @long_press: Class handler for the #XfceOverviewWorkspacesClickActionClass::long-press signal
 *
 * The #XfceOverviewWorkspacesClickActionClass structure contains only private data
 */
struct _XfceOverviewWorkspacesClickActionClass
{
	/*< private >*/
	/* Parent class */
	ClutterActionClass				parent_class;

	/*< public >*/
	/* Virtual functions */
	void (*clicked)(XfceOverviewWorkspacesClickAction *self, ClutterActor *inActor);
	gboolean (*long_press)(XfceOverviewWorkspacesClickAction *self, ClutterActor *inActor, ClutterLongPressState inState);
};

/* Public API */
GType xfce_overview_workspaces_click_action_get_type(void) G_GNUC_CONST;

ClutterAction* xfce_overview_workspaces_click_action_new(void);

guint xfce_overview_workspaces_click_action_get_button(XfceOverviewWorkspacesClickAction *self);
ClutterModifierType xfce_overview_workspaces_click_action_get_state(XfceOverviewWorkspacesClickAction *self);
void xfce_overview_workspaces_click_action_get_coords(XfceOverviewWorkspacesClickAction *self, gfloat *outPressX, gfloat *outPressY);

void xfce_overview_workspaces_click_action_release(XfceOverviewWorkspacesClickAction *self);

gboolean xfce_overview_workspaces_click_action_is_left_button_or_tap(XfceOverviewWorkspacesClickAction *self);

G_END_DECLS

#endif	/* __LIBXFCE_OVERVIEW_WORKSPACES_CLICK_ACTION__ */
