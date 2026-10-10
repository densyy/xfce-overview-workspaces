/*
 * gradient-color: A special purpose boxed color type for solid colors
 *                 or gradients
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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_GRADIENT_COLOR__
#define __LIBXFCE_OVERVIEW_WORKSPACES_GRADIENT_COLOR__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <clutter/clutter.h>


G_BEGIN_DECLS

/* Public definitions */
/**
 * XfceOverviewWorkspacesGradientType:
 * @XFCE_OVERVIEW_WORKSPACES_GRADIENT_TYPE_SOLID: The color is a single-color solid color
 * @XFCE_OVERVIEW_WORKSPACES_GRADIENT_TYPE_PATH_GRADIENT: The color is a linear gradient
 *
 * Determines the type of color.
 */
typedef enum /*< prefix=XFCE_OVERVIEW_WORKSPACES_GRADIENT_TYPE >*/
{
	XFCE_OVERVIEW_WORKSPACES_GRADIENT_TYPE_NONE=0,
	XFCE_OVERVIEW_WORKSPACES_GRADIENT_TYPE_SOLID,
	XFCE_OVERVIEW_WORKSPACES_GRADIENT_TYPE_LINEAR_GRADIENT,
	XFCE_OVERVIEW_WORKSPACES_GRADIENT_TYPE_PATH_GRADIENT
} XfceOverviewWorkspacesGradientType;

/**
 * XFCE_OVERVIEW_WORKSPACES_VALUE_HOLDS_GRADIENT_COLOR:
 * @x: a #GValue
 *
 * Evaluates to %TRUE if @x holds a #XfceOverviewWorkspacesGradientColor<!-- -->.
 */
#define XFCE_OVERVIEW_WORKSPACES_VALUE_HOLDS_GRADIENT_COLOR(x)			(G_VALUE_HOLDS((x), XFCE_OVERVIEW_WORKSPACES_TYPE_GRADIENT_COLOR))


/* Object declaration (XfceOverviewWorkspacesGradientColor) */
#define XFCE_OVERVIEW_WORKSPACES_TYPE_GRADIENT_COLOR						(xfce_overview_workspaces_gradient_color_get_type())

typedef struct _XfceOverviewWorkspacesGradientColor					XfceOverviewWorkspacesGradientColor;


/* Public API (XfceOverviewWorkspacesGradientColor) */
GType xfce_overview_workspaces_gradient_color_get_type(void) G_GNUC_CONST;

/* Creation functions */
XfceOverviewWorkspacesGradientColor* xfce_overview_workspaces_gradient_color_new_solid(const ClutterColor *inColor);
XfceOverviewWorkspacesGradientColor* xfce_overview_workspaces_gradient_color_new_linear_gradient(const ClutterColor *inStartColor, const ClutterColor *inEndColor);
XfceOverviewWorkspacesGradientColor* xfce_overview_workspaces_gradient_color_new_path_gradient(const ClutterColor *inStartColor, const ClutterColor *inEndColor);

XfceOverviewWorkspacesGradientColor* xfce_overview_workspaces_gradient_color_copy(const XfceOverviewWorkspacesGradientColor *self);
void xfce_overview_workspaces_gradient_color_free(XfceOverviewWorkspacesGradientColor *self);

/* Utility functions */
gint xfce_overview_workspaces_gradient_color_compare(const XfceOverviewWorkspacesGradientColor *inLeft, const XfceOverviewWorkspacesGradientColor *inRight);
gboolean xfce_overview_workspaces_gradient_color_equal(const XfceOverviewWorkspacesGradientColor *inLeft, const XfceOverviewWorkspacesGradientColor *inRight);

XfceOverviewWorkspacesGradientColor* xfce_overview_workspaces_gradient_color_from_string(const gchar *inString);
gchar* xfce_overview_workspaces_gradient_color_to_string(const XfceOverviewWorkspacesGradientColor *self);

/* Type functions */
XfceOverviewWorkspacesGradientType xfce_overview_workspaces_gradient_color_get_gradient_type(const XfceOverviewWorkspacesGradientColor *self);

/* Solid color functions */
const ClutterColor* xfce_overview_workspaces_gradient_color_get_solid_color(const XfceOverviewWorkspacesGradientColor *self);
void xfce_overview_workspaces_gradient_color_set_solid_color(XfceOverviewWorkspacesGradientColor *self, const ClutterColor *inColor);

/* Linear gradient functions */
gdouble xfce_overview_workspaces_gradient_color_get_angle(const XfceOverviewWorkspacesGradientColor *self);
void xfce_overview_workspaces_gradient_color_set_angle(XfceOverviewWorkspacesGradientColor *self, gdouble inRadians);

gboolean xfce_overview_workspaces_gradient_color_get_repeat(const XfceOverviewWorkspacesGradientColor *self);
gdouble xfce_overview_workspaces_gradient_color_get_length(const XfceOverviewWorkspacesGradientColor *self);
void xfce_overview_workspaces_gradient_color_set_repeat(XfceOverviewWorkspacesGradientColor *self, gboolean inRepeat, gdouble inLength);

/* Common gradient functions */
guint xfce_overview_workspaces_gradient_color_get_number_stops(const XfceOverviewWorkspacesGradientColor *self);

void xfce_overview_workspaces_gradient_color_get_stop(const XfceOverviewWorkspacesGradientColor *self,
											guint inIndex,
											gdouble *outOffset,
											ClutterColor *outColor);
gboolean xfce_overview_workspaces_gradient_color_add_stop(XfceOverviewWorkspacesGradientColor *self,
												gdouble inOffset,
												const ClutterColor *inColor);

void xfce_overview_workspaces_gradient_color_interpolate(const XfceOverviewWorkspacesGradientColor *self,
											gdouble inProgress,
											ClutterColor *outColor);


/* Object declaration (XfceOverviewWorkspacesParamSpecGradientColor) */

#define XFCE_OVERVIEW_WORKSPACES_TYPE_PARAM_SPEC_GRADIENT_COLOR			(xfce_overview_workspaces_param_spec_gradient_color_get_type())
#define XFCE_OVERVIEW_WORKSPACES_PARAM_SPEC_GRADIENT_COLOR(pspec)		(G_TYPE_CHECK_INSTANCE_CAST((pspec), XFCE_OVERVIEW_WORKSPACES_TYPE_PARAM_SPEC_GRADIENT_COLOR, XfceOverviewWorkspacesParamSpecGradientColor))
#define XFCE_OVERVIEW_WORKSPACES_IS_PARAM_SPEC_GRADIENT_COLOR(pspec)		(G_TYPE_CHECK_INSTANCE_TYPE((pspec), XFCE_OVERVIEW_WORKSPACES_TYPE_PARAM_SPEC_GRADIENT_COLOR))

typedef struct _XfceOverviewWorkspacesParamSpecGradientColor			XfceOverviewWorkspacesParamSpecGradientColor;


/* Public API (XfceOverviewWorkspacesParamSpecGradientColor) */
GType xfce_overview_workspaces_param_spec_gradient_color_get_type(void);

GParamSpec* xfce_overview_workspaces_param_spec_gradient_color(const gchar *inName,
													const gchar *inNick,
													const gchar *inBlurb,
													const XfceOverviewWorkspacesGradientColor *inDefaultValue,
													GParamFlags inFlags);


/* IMPLEMENTATION: Public API (GValue) */

void xfce_overview_workspaces_value_set_gradient_color(GValue *ioValue, const XfceOverviewWorkspacesGradientColor *inColor);
const XfceOverviewWorkspacesGradientColor* xfce_overview_workspaces_value_get_gradient_color(const GValue *inValue);

G_END_DECLS

#endif	/* __LIBXFCE_OVERVIEW_WORKSPACES_GRADIENT_COLOR__ */
