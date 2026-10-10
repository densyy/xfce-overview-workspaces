/*
 * image-content: An asynchronous loaded and cached image content
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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_IMAGE_CONTENT__
#define __LIBXFCE_OVERVIEW_WORKSPACES_IMAGE_CONTENT__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <clutter/clutter.h>
#include <gdk/gdk.h>

G_BEGIN_DECLS

#define XFCE_OVERVIEW_WORKSPACES_TYPE_IMAGE_CONTENT				(xfce_overview_workspaces_image_content_get_type())
#define XFCE_OVERVIEW_WORKSPACES_IMAGE_CONTENT(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_IMAGE_CONTENT, XfceOverviewWorkspacesImageContent))
#define XFCE_OVERVIEW_WORKSPACES_IS_IMAGE_CONTENT(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_IMAGE_CONTENT))
#define XFCE_OVERVIEW_WORKSPACES_IMAGE_CONTENT_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_IMAGE_CONTENT, XfceOverviewWorkspacesImageContentClass))
#define XFCE_OVERVIEW_WORKSPACES_IS_IMAGE_CONTENT_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_IMAGE_CONTENT))
#define XFCE_OVERVIEW_WORKSPACES_IMAGE_CONTENT_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_IMAGE_CONTENT, XfceOverviewWorkspacesImageContentClass))

typedef struct _XfceOverviewWorkspacesImageContent				XfceOverviewWorkspacesImageContent;
typedef struct _XfceOverviewWorkspacesImageContentClass		XfceOverviewWorkspacesImageContentClass;
typedef struct _XfceOverviewWorkspacesImageContentPrivate		XfceOverviewWorkspacesImageContentPrivate;

struct _XfceOverviewWorkspacesImageContent
{
	/*< private >*/
	/* Parent instance */
	ClutterImage						parent_instance;

	/* Private structure */
	XfceOverviewWorkspacesImageContentPrivate		*priv;
};

struct _XfceOverviewWorkspacesImageContentClass
{
	/*< private >*/
	/* Parent class */
	ClutterImageClass					parent_class;

	/*< public >*/
	/* Virtual functions */
	void (*loaded)(XfceOverviewWorkspacesImageContent *self);
	void (*loading_failed)(XfceOverviewWorkspacesImageContent *self);
};

/* Public API */
typedef enum /*< prefix=XFCE_OVERVIEW_WORKSPACES_IMAGE_CONTENT_LOADING_STATE >*/
{
	XFCE_OVERVIEW_WORKSPACES_IMAGE_CONTENT_LOADING_STATE_NONE=0,
	XFCE_OVERVIEW_WORKSPACES_IMAGE_CONTENT_LOADING_STATE_LOADING,
	XFCE_OVERVIEW_WORKSPACES_IMAGE_CONTENT_LOADING_STATE_LOADED_SUCCESSFULLY,
	XFCE_OVERVIEW_WORKSPACES_IMAGE_CONTENT_LOADING_STATE_LOADED_FAILED
} XfceOverviewWorkspacesImageContentLoadingState;

GType xfce_overview_workspaces_image_content_get_type(void) G_GNUC_CONST;

ClutterContent* xfce_overview_workspaces_image_content_new_for_icon_name(const gchar *inIconName, gint inSize);
ClutterContent* xfce_overview_workspaces_image_content_new_for_gicon(GIcon *inIcon, gint inSize);
ClutterContent* xfce_overview_workspaces_image_content_new_for_pixbuf(GdkPixbuf *inPixbuf);

const gchar* xfce_overview_workspaces_image_content_get_missing_icon_name(XfceOverviewWorkspacesImageContent *self);
void xfce_overview_workspaces_image_content_set_missing_icon_name(XfceOverviewWorkspacesImageContent *self, const gchar *inMissingIconName);

XfceOverviewWorkspacesImageContentLoadingState xfce_overview_workspaces_image_content_get_state(XfceOverviewWorkspacesImageContent *self);

void xfce_overview_workspaces_image_content_force_load(XfceOverviewWorkspacesImageContent *self);

G_END_DECLS

#endif	/* __LIBXFCE_OVERVIEW_WORKSPACES_IMAGE_CONTENT__ */
