/*
 * label: An actor representing a label and an icon (both optional)
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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_LABEL__
#define __LIBXFCE_OVERVIEW_WORKSPACES_LABEL__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <clutter/clutter.h>

#include <libxfce-overview-workspaces/background.h>
#include <libxfce-overview-workspaces/types.h>

G_BEGIN_DECLS

/* Public definitions */
/**
 * XfceOverviewWorkspacesLabelStyle:
 * @XFCE_OVERVIEW_WORKSPACES_LABEL_STYLE_TEXT: The actor will show only text labels.
 * @XFCE_OVERVIEW_WORKSPACES_LABEL_STYLE_ICON: The actor will show only icons.
 * @XFCE_OVERVIEW_WORKSPACES_LABEL_STYLE_BOTH: The actor will show both, text labels and icons.
 *
 * Determines the style of an actor, e.g. text labels and icons at labels.
 */
typedef enum /*< prefix=XFCE_OVERVIEW_WORKSPACES_LABEL_STYLE >*/
{
	XFCE_OVERVIEW_WORKSPACES_LABEL_STYLE_TEXT=0,
	XFCE_OVERVIEW_WORKSPACES_LABEL_STYLE_ICON,
	XFCE_OVERVIEW_WORKSPACES_LABEL_STYLE_BOTH
} XfceOverviewWorkspacesLabelStyle;


/* Object declaration */
#define XFCE_OVERVIEW_WORKSPACES_TYPE_LABEL					(xfce_overview_workspaces_label_get_type())
#define XFCE_OVERVIEW_WORKSPACES_LABEL(obj)					(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_LABEL, XfceOverviewWorkspacesLabel))
#define XFCE_OVERVIEW_WORKSPACES_IS_LABEL(obj)				(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_LABEL))
#define XFCE_OVERVIEW_WORKSPACES_LABEL_CLASS(klass)			(G_TYPE_CHECK_CLASS_CAST((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_LABEL, XfceOverviewWorkspacesLabelClass))
#define XFCE_OVERVIEW_WORKSPACES_IS_LABEL_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_LABEL))
#define XFCE_OVERVIEW_WORKSPACES_LABEL_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_LABEL, XfceOverviewWorkspacesLabelClass))

typedef struct _XfceOverviewWorkspacesLabel				XfceOverviewWorkspacesLabel;
typedef struct _XfceOverviewWorkspacesLabelClass			XfceOverviewWorkspacesLabelClass;
typedef struct _XfceOverviewWorkspacesLabelPrivate			XfceOverviewWorkspacesLabelPrivate;

struct _XfceOverviewWorkspacesLabel
{
	/*< private >*/
	/* Parent instance */
	XfceOverviewWorkspacesBackground		parent_instance;

	/* Private structure */
	XfceOverviewWorkspacesLabelPrivate	*priv;
};

struct _XfceOverviewWorkspacesLabelClass
{
	/*< private >*/
	/* Parent class */
	XfceOverviewWorkspacesBackgroundClass	parent_class;

	/*< public >*/
	/* Virtual functions */
	void (*clicked)(XfceOverviewWorkspacesLabel *self);
};

/* Public API */
GType xfce_overview_workspaces_label_get_type(void) G_GNUC_CONST;

ClutterActor* xfce_overview_workspaces_label_new(void);
ClutterActor* xfce_overview_workspaces_label_new_with_text(const gchar *inText);
ClutterActor* xfce_overview_workspaces_label_new_with_icon_name(const gchar *inIconName);
ClutterActor* xfce_overview_workspaces_label_new_with_gicon(GIcon *inIcon);
ClutterActor* xfce_overview_workspaces_label_new_full_with_icon_name(const gchar *inIconName, const gchar *inText);
ClutterActor* xfce_overview_workspaces_label_new_full_with_gicon(GIcon *inIcon, const gchar *inText);

/* General functions */
gfloat xfce_overview_workspaces_label_get_padding(XfceOverviewWorkspacesLabel *self);
void xfce_overview_workspaces_label_set_padding(XfceOverviewWorkspacesLabel *self, const gfloat inPadding);

gfloat xfce_overview_workspaces_label_get_spacing(XfceOverviewWorkspacesLabel *self);
void xfce_overview_workspaces_label_set_spacing(XfceOverviewWorkspacesLabel *self, const gfloat inSpacing);

XfceOverviewWorkspacesLabelStyle xfce_overview_workspaces_label_get_style(XfceOverviewWorkspacesLabel *self);
void xfce_overview_workspaces_label_set_style(XfceOverviewWorkspacesLabel *self, const XfceOverviewWorkspacesLabelStyle inStyle);

/* Icon functions */
const gchar* xfce_overview_workspaces_label_get_icon_name(XfceOverviewWorkspacesLabel *self);
void xfce_overview_workspaces_label_set_icon_name(XfceOverviewWorkspacesLabel *self, const gchar *inIconName);

GIcon* xfce_overview_workspaces_label_get_gicon(XfceOverviewWorkspacesLabel *self);
void xfce_overview_workspaces_label_set_gicon(XfceOverviewWorkspacesLabel *self, GIcon *inIcon);

ClutterImage* xfce_overview_workspaces_label_get_icon_image(XfceOverviewWorkspacesLabel *self);
void xfce_overview_workspaces_label_set_icon_image(XfceOverviewWorkspacesLabel *self, ClutterImage *inIconImage);

gint xfce_overview_workspaces_label_get_icon_size(XfceOverviewWorkspacesLabel *self);
void xfce_overview_workspaces_label_set_icon_size(XfceOverviewWorkspacesLabel *self, gint inSize);

gboolean xfce_overview_workspaces_label_get_sync_icon_size(XfceOverviewWorkspacesLabel *self);
void xfce_overview_workspaces_label_set_sync_icon_size(XfceOverviewWorkspacesLabel *self, gboolean inSync);

XfceOverviewWorkspacesOrientation xfce_overview_workspaces_label_get_icon_orientation(XfceOverviewWorkspacesLabel *self);
void xfce_overview_workspaces_label_set_icon_orientation(XfceOverviewWorkspacesLabel *self, const XfceOverviewWorkspacesOrientation inOrientation);

/* Label functions */
const gchar* xfce_overview_workspaces_label_get_text(XfceOverviewWorkspacesLabel *self);
void xfce_overview_workspaces_label_set_text(XfceOverviewWorkspacesLabel *self, const gchar *inMarkupText);

const gchar* xfce_overview_workspaces_label_get_font(XfceOverviewWorkspacesLabel *self);
void xfce_overview_workspaces_label_set_font(XfceOverviewWorkspacesLabel *self, const gchar *inFont);

const ClutterColor* xfce_overview_workspaces_label_get_color(XfceOverviewWorkspacesLabel *self);
void xfce_overview_workspaces_label_set_color(XfceOverviewWorkspacesLabel *self, const ClutterColor *inColor);

PangoEllipsizeMode xfce_overview_workspaces_label_get_ellipsize_mode(XfceOverviewWorkspacesLabel *self);
void xfce_overview_workspaces_label_set_ellipsize_mode(XfceOverviewWorkspacesLabel *self, const PangoEllipsizeMode inMode);

gboolean xfce_overview_workspaces_label_get_single_line_mode(XfceOverviewWorkspacesLabel *self);
void xfce_overview_workspaces_label_set_single_line_mode(XfceOverviewWorkspacesLabel *self, const gboolean inSingleLine);

PangoAlignment xfce_overview_workspaces_label_get_text_justification(XfceOverviewWorkspacesLabel *self);
void xfce_overview_workspaces_label_set_text_justification(XfceOverviewWorkspacesLabel *self, const PangoAlignment inJustification);

G_END_DECLS

#endif	/* __LIBXFCE_OVERVIEW_WORKSPACES_LABEL__ */
