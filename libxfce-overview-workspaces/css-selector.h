/*
 * css-selector: A CSS simple selector class
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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_CSS_SELECTOR__
#define __LIBXFCE_OVERVIEW_WORKSPACES_CSS_SELECTOR__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <glib-object.h>
#include <glib.h>

#include <libxfce-overview-workspaces/stylable.h>

G_BEGIN_DECLS

#define XFCE_OVERVIEW_WORKSPACES_TYPE_CSS_SELECTOR				(xfce_overview_workspaces_css_selector_get_type())
#define XFCE_OVERVIEW_WORKSPACES_CSS_SELECTOR(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_CSS_SELECTOR, XfceOverviewWorkspacesCssSelector))
#define XFCE_OVERVIEW_WORKSPACES_IS_CSS_SELECTOR(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_CSS_SELECTOR))
#define XFCE_OVERVIEW_WORKSPACES_CSS_SELECTOR_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_CSS_SELECTOR, XfceOverviewWorkspacesCssSelectorClass))
#define XFCE_OVERVIEW_WORKSPACES_IS_CSS_SELECTOR_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_CSS_SELECTOR))
#define XFCE_OVERVIEW_WORKSPACES_CSS_SELECTOR_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_CSS_SELECTOR, XfceOverviewWorkspacesCssSelectorClass))

typedef struct _XfceOverviewWorkspacesCssSelector				XfceOverviewWorkspacesCssSelector; 
typedef struct _XfceOverviewWorkspacesCssSelectorPrivate		XfceOverviewWorkspacesCssSelectorPrivate;
typedef struct _XfceOverviewWorkspacesCssSelectorClass			XfceOverviewWorkspacesCssSelectorClass;

struct _XfceOverviewWorkspacesCssSelector
{
	/*< private >*/
	/* Parent instance */
	GObject							parent_instance;

	/* Private structure */
	XfceOverviewWorkspacesCssSelectorPrivate	*priv;
};

struct _XfceOverviewWorkspacesCssSelectorClass
{
	/*< private >*/
	/* Parent class */
	GObjectClass					parent_class;

	/*< public >*/
	/* Virtual functions */
};

typedef struct _XfceOverviewWorkspacesCssSelectorRule			XfceOverviewWorkspacesCssSelectorRule;

/* Public declarations */
#define XFCE_OVERVIEW_WORKSPACES_CSS_SELECTOR_PARSE_FINISH_OK			TRUE
#define XFCE_OVERVIEW_WORKSPACES_CSS_SELECTOR_PARSE_FINISH_BAD_STATE		FALSE

typedef gboolean (*XfceOverviewWorkspacesCssSelectorParseFinishCallback)(XfceOverviewWorkspacesCssSelector *inSelector,
																GScanner *inScanner,
																GTokenType inPeekNextToken,
																gpointer inUserData);

/* Public API */
GType xfce_overview_workspaces_css_selector_get_type(void) G_GNUC_CONST;

XfceOverviewWorkspacesCssSelector* xfce_overview_workspaces_css_selector_new_from_string(const gchar *inSelector);
XfceOverviewWorkspacesCssSelector* xfce_overview_workspaces_css_selector_new_from_string_with_priority(const gchar *inSelector, gint inPriority);
XfceOverviewWorkspacesCssSelector* xfce_overview_workspaces_css_selector_new_from_scanner(GScanner *ioScanner,
																	XfceOverviewWorkspacesCssSelectorParseFinishCallback inFinishCallback,
																	gpointer inUserData);
XfceOverviewWorkspacesCssSelector* xfce_overview_workspaces_css_selector_new_from_scanner_with_priority(GScanner *ioScanner,
																				gint inPriority,
																				XfceOverviewWorkspacesCssSelectorParseFinishCallback inFinishCallback,
																				gpointer inUserData);

gchar* xfce_overview_workspaces_css_selector_to_string(XfceOverviewWorkspacesCssSelector *self);

gint xfce_overview_workspaces_css_selector_score(XfceOverviewWorkspacesCssSelector *self, XfceOverviewWorkspacesStylable *inStylable);

void xfce_overview_workspaces_css_selector_adjust_to_offset(XfceOverviewWorkspacesCssSelector *self, gint inLine, gint inPosition);

XfceOverviewWorkspacesCssSelectorRule* xfce_overview_workspaces_css_selector_get_rule(XfceOverviewWorkspacesCssSelector *self);

const gchar* xfce_overview_workspaces_css_selector_rule_get_type(XfceOverviewWorkspacesCssSelectorRule *inRule);
const gchar* xfce_overview_workspaces_css_selector_rule_get_id(XfceOverviewWorkspacesCssSelectorRule *inRule);
const gchar* xfce_overview_workspaces_css_selector_rule_get_classes(XfceOverviewWorkspacesCssSelectorRule *inRule);
const gchar* xfce_overview_workspaces_css_selector_rule_get_pseudo_classes(XfceOverviewWorkspacesCssSelectorRule *inRule);
XfceOverviewWorkspacesCssSelectorRule* xfce_overview_workspaces_css_selector_rule_get_parent(XfceOverviewWorkspacesCssSelectorRule *inRule);
XfceOverviewWorkspacesCssSelectorRule* xfce_overview_workspaces_css_selector_rule_get_ancestor(XfceOverviewWorkspacesCssSelectorRule *inRule);
const gchar* xfce_overview_workspaces_css_selector_rule_get_source(XfceOverviewWorkspacesCssSelectorRule *inRule);
gint xfce_overview_workspaces_css_selector_rule_get_priority(XfceOverviewWorkspacesCssSelectorRule *inRule);
guint xfce_overview_workspaces_css_selector_rule_get_line(XfceOverviewWorkspacesCssSelectorRule *inRule);
guint xfce_overview_workspaces_css_selector_rule_get_position(XfceOverviewWorkspacesCssSelectorRule *inRule);

G_END_DECLS

#endif	/* __LIBXFCE_OVERVIEW_WORKSPACES_CSS_SELECTOR__ */
