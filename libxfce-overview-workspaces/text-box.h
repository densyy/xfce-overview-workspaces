/*
 * textbox: An actor representing an editable or read-only text-box
 *          with optinal icons
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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_TEXT_BOX__
#define __LIBXFCE_OVERVIEW_WORKSPACES_TEXT_BOX__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <libxfce-overview-workspaces/background.h>

G_BEGIN_DECLS

#define XFCE_OVERVIEW_WORKSPACES_TYPE_TEXT_BOX				(xfce_overview_workspaces_text_box_get_type())
#define XFCE_OVERVIEW_WORKSPACES_TEXT_BOX(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_TEXT_BOX, XfceOverviewWorkspacesTextBox))
#define XFCE_OVERVIEW_WORKSPACES_IS_TEXT_BOX(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_TEXT_BOX))
#define XFCE_OVERVIEW_WORKSPACES_TEXT_BOX_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_TEXT_BOX, XfceOverviewWorkspacesTextBoxClass))
#define XFCE_OVERVIEW_WORKSPACES_IS_TEXT_BOX_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_TEXT_BOX))
#define XFCE_OVERVIEW_WORKSPACES_TEXT_BOX_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_TEXT_BOX, XfceOverviewWorkspacesTextBoxClass))

typedef struct _XfceOverviewWorkspacesTextBox				XfceOverviewWorkspacesTextBox;
typedef struct _XfceOverviewWorkspacesTextBoxClass			XfceOverviewWorkspacesTextBoxClass;
typedef struct _XfceOverviewWorkspacesTextBoxPrivate		XfceOverviewWorkspacesTextBoxPrivate;

struct _XfceOverviewWorkspacesTextBox
{
	/*< private >*/
	/* Parent instance */
	XfceOverviewWorkspacesBackground			parent_instance;

	/* Private structure */
	XfceOverviewWorkspacesTextBoxPrivate		*priv;
};

struct _XfceOverviewWorkspacesTextBoxClass
{
	/*< private >*/
	/* Parent class */
	XfceOverviewWorkspacesBackgroundClass		parent_class;

	/*< public >*/
	/* Virtual functions */
	void (*text_changed)(XfceOverviewWorkspacesTextBox *self, gchar *inText);
	
	void (*primary_icon_clicked)(XfceOverviewWorkspacesTextBox *self);
	void (*secondary_icon_clicked)(XfceOverviewWorkspacesTextBox *self);
};

/* Public API */
GType xfce_overview_workspaces_text_box_get_type(void) G_GNUC_CONST;

ClutterActor* xfce_overview_workspaces_text_box_new(void);

gfloat xfce_overview_workspaces_text_box_get_padding(XfceOverviewWorkspacesTextBox *self);
void xfce_overview_workspaces_text_box_set_padding(XfceOverviewWorkspacesTextBox *self, gfloat inPadding);

gfloat xfce_overview_workspaces_text_box_get_spacing(XfceOverviewWorkspacesTextBox *self);
void xfce_overview_workspaces_text_box_set_spacing(XfceOverviewWorkspacesTextBox *self, gfloat inSpacing);

gboolean xfce_overview_workspaces_text_box_get_editable(XfceOverviewWorkspacesTextBox *self);
void xfce_overview_workspaces_text_box_set_editable(XfceOverviewWorkspacesTextBox *self, gboolean isEditable);

gboolean xfce_overview_workspaces_text_box_is_empty(XfceOverviewWorkspacesTextBox *self);
gint xfce_overview_workspaces_text_box_get_length(XfceOverviewWorkspacesTextBox *self);
const gchar* xfce_overview_workspaces_text_box_get_text(XfceOverviewWorkspacesTextBox *self);
void xfce_overview_workspaces_text_box_set_text(XfceOverviewWorkspacesTextBox *self, const gchar *inMarkupText);
void xfce_overview_workspaces_text_box_set_text_va(XfceOverviewWorkspacesTextBox *self, const gchar *inFormat, ...) G_GNUC_PRINTF(2, 3);

const gchar* xfce_overview_workspaces_text_box_get_text_font(XfceOverviewWorkspacesTextBox *self);
void xfce_overview_workspaces_text_box_set_text_font(XfceOverviewWorkspacesTextBox *self, const gchar *inFont);

const ClutterColor* xfce_overview_workspaces_text_box_get_text_color(XfceOverviewWorkspacesTextBox *self);
void xfce_overview_workspaces_text_box_set_text_color(XfceOverviewWorkspacesTextBox *self, const ClutterColor *inColor);

const ClutterColor* xfce_overview_workspaces_text_box_get_selection_text_color(XfceOverviewWorkspacesTextBox *self);
void xfce_overview_workspaces_text_box_set_selection_text_color(XfceOverviewWorkspacesTextBox *self, const ClutterColor *inColor);

const ClutterColor* xfce_overview_workspaces_text_box_get_selection_background_color(XfceOverviewWorkspacesTextBox *self);
void xfce_overview_workspaces_text_box_set_selection_background_color(XfceOverviewWorkspacesTextBox *self, const ClutterColor *inColor);

gboolean xfce_overview_workspaces_text_box_is_hint_text_set(XfceOverviewWorkspacesTextBox *self);
const gchar* xfce_overview_workspaces_text_box_get_hint_text(XfceOverviewWorkspacesTextBox *self);
void xfce_overview_workspaces_text_box_set_hint_text(XfceOverviewWorkspacesTextBox *self, const gchar *inMarkupText);
void xfce_overview_workspaces_text_box_set_hint_text_va(XfceOverviewWorkspacesTextBox *self, const gchar *inFormat, ...) G_GNUC_PRINTF(2, 3);

const gchar* xfce_overview_workspaces_text_box_get_hint_text_font(XfceOverviewWorkspacesTextBox *self);
void xfce_overview_workspaces_text_box_set_hint_text_font(XfceOverviewWorkspacesTextBox *self, const gchar *inFont);

const ClutterColor* xfce_overview_workspaces_text_box_get_hint_text_color(XfceOverviewWorkspacesTextBox *self);
void xfce_overview_workspaces_text_box_set_hint_text_color(XfceOverviewWorkspacesTextBox *self, const ClutterColor *inColor);

const gchar* xfce_overview_workspaces_text_box_get_primary_icon(XfceOverviewWorkspacesTextBox *self);
void xfce_overview_workspaces_text_box_set_primary_icon(XfceOverviewWorkspacesTextBox *self, const gchar *inIconName);

const gchar* xfce_overview_workspaces_text_box_get_secondary_icon(XfceOverviewWorkspacesTextBox *self);
void xfce_overview_workspaces_text_box_set_secondary_icon(XfceOverviewWorkspacesTextBox *self, const gchar *inIconName);

G_END_DECLS

#endif	/* __LIBXFCE_OVERVIEW_WORKSPACES_TEXT_BOX__ */
