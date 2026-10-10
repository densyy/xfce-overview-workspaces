/*
 * stage: Global stage of application
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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_STAGE__
#define __LIBXFCE_OVERVIEW_WORKSPACES_STAGE__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <clutter/clutter.h>

#include <libxfce-overview-workspaces/types.h>

G_BEGIN_DECLS

#define XFCE_OVERVIEW_WORKSPACES_TYPE_STAGE				(xfce_overview_workspaces_stage_get_type())
#define XFCE_OVERVIEW_WORKSPACES_STAGE(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_STAGE, XfceOverviewWorkspacesStage))
#define XFCE_OVERVIEW_WORKSPACES_IS_STAGE(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_STAGE))
#define XFCE_OVERVIEW_WORKSPACES_STAGE_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_STAGE, XfceOverviewWorkspacesStageClass))
#define XFCE_OVERVIEW_WORKSPACES_IS_STAGE_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_STAGE))
#define XFCE_OVERVIEW_WORKSPACES_STAGE_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_STAGE, XfceOverviewWorkspacesStageClass))

typedef struct _XfceOverviewWorkspacesStage			XfceOverviewWorkspacesStage;
typedef struct _XfceOverviewWorkspacesStageClass		XfceOverviewWorkspacesStageClass;
typedef struct _XfceOverviewWorkspacesStagePrivate		XfceOverviewWorkspacesStagePrivate;

struct _XfceOverviewWorkspacesStage
{
	/*< private >*/
	/* Parent instance */
	ClutterStage							parent_instance;

	/* Private structure */
	XfceOverviewWorkspacesStagePrivate					*priv;
};

struct _XfceOverviewWorkspacesStageClass
{
	/*< private >*/
	/* Parent class */
	ClutterStageClass						parent_class;

	/*< public >*/
	/* Virtual functions */
	void (*actor_created)(XfceOverviewWorkspacesStage *self, ClutterActor *inActor);

	void (*show_tooltip)(XfceOverviewWorkspacesStage *self, ClutterAction *inAction);
	void (*hide_tooltip)(XfceOverviewWorkspacesStage *self, ClutterAction *inAction);
};

/* Public API */
GType xfce_overview_workspaces_stage_get_type(void) G_GNUC_CONST;

ClutterActor* xfce_overview_workspaces_stage_new(void);

XfceOverviewWorkspacesStageBackgroundImageType xfce_overview_workspaces_stage_get_background_image_type(XfceOverviewWorkspacesStage *self);
void xfce_overview_workspaces_stage_set_background_image_type(XfceOverviewWorkspacesStage *self, XfceOverviewWorkspacesStageBackgroundImageType inType);

ClutterColor* xfce_overview_workspaces_stage_get_background_color(XfceOverviewWorkspacesStage *self);
void xfce_overview_workspaces_stage_set_background_color(XfceOverviewWorkspacesStage *self, const ClutterColor *inColor);

const gchar* xfce_overview_workspaces_stage_get_switch_to_view(XfceOverviewWorkspacesStage *self);
void xfce_overview_workspaces_stage_set_switch_to_view(XfceOverviewWorkspacesStage *self, const gchar *inViewInternalName);

void xfce_overview_workspaces_stage_show_notification(XfceOverviewWorkspacesStage *self, const gchar *inIconName, const gchar *inText);

G_END_DECLS

#endif	/* __LIBXFCE_OVERVIEW_WORKSPACES_STAGE__ */
