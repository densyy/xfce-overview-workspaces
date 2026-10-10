/*
 * tooltip-action: An action to display a tooltip after a short timeout
 *                 without movement at the referred actor
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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_TOOLTIP_ACTION__
#define __LIBXFCE_OVERVIEW_WORKSPACES_TOOLTIP_ACTION__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <clutter/clutter.h>

G_BEGIN_DECLS

#define XFCE_OVERVIEW_WORKSPACES_TYPE_TOOLTIP_ACTION				(xfce_overview_workspaces_tooltip_action_get_type ())
#define XFCE_OVERVIEW_WORKSPACES_TOOLTIP_ACTION(obj)				(G_TYPE_CHECK_INSTANCE_CAST ((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_TOOLTIP_ACTION, XfceOverviewWorkspacesTooltipAction))
#define XFCE_OVERVIEW_WORKSPACES_IS_TOOLTIP_ACTION(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_TOOLTIP_ACTION))
#define XFCE_OVERVIEW_WORKSPACES_TOOLTIP_ACTION_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_TOOLTIP_ACTION, XfceOverviewWorkspacesTooltipActionClass))
#define XFCE_OVERVIEW_WORKSPACES_IS_TOOLTIP_ACTION_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_TOOLTIP_ACTION))
#define XFCE_OVERVIEW_WORKSPACES_TOOLTIP_ACTION_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_TOOLTIP_ACTION, XfceOverviewWorkspacesTooltipActionClass))

typedef struct _XfceOverviewWorkspacesTooltipAction			XfceOverviewWorkspacesTooltipAction;
typedef struct _XfceOverviewWorkspacesTooltipActionPrivate		XfceOverviewWorkspacesTooltipActionPrivate;
typedef struct _XfceOverviewWorkspacesTooltipActionClass		XfceOverviewWorkspacesTooltipActionClass;

struct _XfceOverviewWorkspacesTooltipAction
{
	/*< private >*/
	/* Parent instance */
	ClutterAction						parent_instance;

	/* Private structure */
	XfceOverviewWorkspacesTooltipActionPrivate		*priv;
};

struct _XfceOverviewWorkspacesTooltipActionClass
{
	/*< private >*/
	/* Parent class */
	ClutterActionClass					parent_class;

	/*< public >*/
	/* Virtual functions */
	void (*activating)(XfceOverviewWorkspacesTooltipAction *self);
};

/* Public API */
GType xfce_overview_workspaces_tooltip_action_get_type(void) G_GNUC_CONST;

ClutterAction* xfce_overview_workspaces_tooltip_action_new(void);

const gchar* xfce_overview_workspaces_tooltip_action_get_text(XfceOverviewWorkspacesTooltipAction *self);
void xfce_overview_workspaces_tooltip_action_set_text(XfceOverviewWorkspacesTooltipAction *self, const gchar *inTooltipText);

void xfce_overview_workspaces_tooltip_action_get_position(XfceOverviewWorkspacesTooltipAction *self, gfloat *outX, gfloat *outY);

G_END_DECLS

#endif	/* __LIBXFCE_OVERVIEW_WORKSPACES_TOOLTIP_ACTION__ */
