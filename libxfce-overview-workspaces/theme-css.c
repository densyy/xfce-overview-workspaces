/*
 * theme-css: A theme used for rendering xfce-overview-workspaces actors with CSS.
 *            The parser and the handling of CSS files is heavily based
 *            on mx-css, mx-style and mx-stylable of library mx
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

/**
 * SECTION:theme-css
 * @short_description: A theme CSS engine and resource manager
 * @include: libxfce-overview-workspaces/theme-css.h
 *
 * #XfceOverviewWorkspacesThemeCSS is a CSS engine and resource manager to apply property
 * values to actors derived from #XfceOverviewWorkspacesStylable from matching selectors
 * in CSS resources depending on their state.
 *
 * ## Note about resources parser
 *
 * Because all resources for CSS and XML are texts, the parser does convert texts
 * to their needed and native type like booleans, integers etc. The parser can
 * convert textual representation for most common property types. Boolean textual
 * representations for FALSE are "FALSE, "f", "no", "n", "0" and for TRUE they are
 * "TRUE", "t", "yes", "y", "1". Enumeration and flags can be specified either by
 * their names, their nicks or integer values. Flags can be combined with "|"
 * additionally.
 *
 * # Styling {#XfceOverviewWorkspacesThemeCSS.Styling}
 *
 * Simply said: Styling is writing a CSS file and describing which properties of
 * which actor to set to which value at certain conditions.
 *
 * The location of the CSS file is specified in theme's index file at the key
 * `Style` and should be a relative path to theme's path. See <link
 * linkend="XfceOverviewWorkspacesTheme.File-location-and-structure">File location
 * and structure</link> for further documentation of files and folders.
 *
 * The CSS file must follow the CSS 1.1 specification with one known exception:
 * The keyword `!important` is not supported. Elements in CSS relate to the
 * class name of the corresponding #GObject in case-sensitive manner, e.g.
 * #XfceOverviewWorkspacesButton. IDs in CSS relate to actor names as set in property `name`
 * at #ClutterActor or set with clutter_actor_set_name(), e.g. "#viewpad"
 * for viewpad actor. Classes and pseudo-classes are used as expected.
 *
 * For a list of actor names which can used as ID in CSS file, see section <link
 * linkend="XfceOverviewWorkspacesThemeLayout.Layout">Layout</link>.
 *
 * # @-rules {#XfceOverviewWorkspacesThemeCSS.@-rules}
 *
 * @-rules are used to define and access constant values or to use any built-in
 * function, e.g. coloring functions.
 *
 * ## @constants {#XfceOverviewWorkspacesThemeCSS.@constants}
 *
 * Constants will be defined as a `@constants` selector. The property names in
 * this selector are the constants' names accessible via `@` followed by the name
 * within the other selectors in the CSS - called @-reference. The constants'
 * values can either be numbers, colors, strings or functions whereby strings
 * must be enclosed either by single or double quotation marks. The difference
 * between these two are, that strings enclosed by double quotation marks can
 * contain special character sequences like `\n` or `\r` etc. which will be
 * unescaped to new-lines and so. Strings enclosed in single quotation marks
 * will not be unescaped and used as-is.
 *
 * <warning>
 *   <para>
 *     Regardless if strings are enclosed in single or double quotation
 *     marks, you cannot reference any constant value or use any built-in
 *     function within the string. You have to break it into two strings
 *     with the @-reference at the point where the string is broken into
 *     two parts, e.g.
 *   </para>
 *
 * |[<!-- language="text" -->
 * "Value of constant-name is: "@constant-name"."
 * ]|
 * </warning>
 *
 * Instead of fixed values you can use any of the built-in functions to calculate
 * the property's value. These functions cannot only be used in `@constants`
 * selectors but in any selector. To call a function, do prefix its name `@`
 * followed by a comma-seperated list of function parameters enclosed by brackets,
 * e.g.
 *
 * |[<!-- language="text" -->
 * @rgba(17, 34, 51, 100%)
 * ]|
 *
 * For a list of built-in function and their function paramters, see section
 * <link linkend="XfceOverviewWorkspacesThemeLayout.css-functions">List of built-in CSS
 * functions</link>.
 *
 * The `@constants` selector can be declared repeatedly. The latter block will
 * overwrite the values of any previously defined constant with the same constant
 * name. The `@-references` always refer the latest definition parsed so far.
 *
 * The following example will define the constants named `color-fixed` and
 * `color-by-function` and both with value `#112233`.
 *
 * |[<!-- language="css" -->
 * @constants
 * {
 *   color-fixed: #112233;
 *   color-by-function: @rga(17, 34, 51);
 * }
 * ]|
 *
 * The value of these constants can later be used in any following selector
 * by their @-references.
 *
 * |[<!-- language="css" -->
 * XfceOverviewWorkspacesButton:focus
 * {
 *   background-color: @color-fixed;
 * }
 *
 * XfceOverviewWorkspacesApplicationButton:focus
 * {
 *   background-color: @color-by-function;
 * }
 * ]|
 *
 * ## CSS function: @import {#XfceOverviewWorkspacesThemeCSS.css-function-import}
 *
 * The `@import` function allows to import other CSS styles from other files.
 *
 * <warning>
 *   <para>
 *     This function cannot be used or referenced within any CSS block. It must precede any other rules.
 *   </para>
 * </warning>
 *
 * <table>
 *   <thead>
 *     <row>
 *       <entry>Syntax</entry>
 *     </row>
 *   </thead>
 *   <tbody>
 *     <row>
 *       <entry><code>@</code>import(filename)</entry>
 *     </row>
 *   </tbody>
 * </table>
 * Imports a CSS files into the current CSS file.
 * <table>
 *   <thead>
 *     <row>
 *       <entry>Parameter</entry>
 *       <entry>Description</entry>
 *     </row>
 *   </thead>
 *   <tbody>
 *     <row>
 *       <entry>filename</entry>
 *       <entry>
 *         An absolute or relative path to the CSS file to import. The filename
 *         should be quoted with either by single or double quotation marks.
 *         Relative paths will be looked up first relative to the path of current
 *         CSS file being parsed. If no such file exists the relative path will
 *         be used relative the theme's path.
 *       </entry>
 *     </row>
 *   </tbody>
 * </table>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <libxfce-overview-workspaces/theme-css.h>

#include <glib/gi18n-lib.h>
#include <glib.h>
#include <gio/gio.h>
#include <gio/gfiledescriptorbased.h>
#include <clutter/clutter.h>
#include <gtk/gtk.h>

#include <libxfce-overview-workspaces/stylable.h>
#include <libxfce-overview-workspaces/css-selector.h>
#include <libxfce-overview-workspaces/debug.h>


/* Define this class in GObject system */
struct _XfceOverviewWorkspacesThemeCSSPrivate
{
	/* Instance related */
	gchar		*themePath;

	GList		*selectors;
	GList		*styles;
	GSList		*names;

	GHashTable	*registeredFunctions;

	gint		offsetLine;

	/* Startup I/O caches (no visual change, only skip repeated work):
	 * - loadedFiles: absolute CSS paths already parsed (dedup @import
	 *   cycles and duplicated Style entries -> cuts open+read+GScanner).
	 * - iconMemo: icon-name -> found flag (cuts repeated g_file_test and
	 *   gtk_icon_theme_has_icon scans when try_icons repeats).
	 * - pathExistsMemo: full path -> exists flag (cuts repeated stat in
	 *   @import resolution and try_icons relative checks).
	 */
	GHashTable	*loadedFiles;
	GHashTable	*iconMemo;
	GHashTable	*pathExistsMemo;
};

G_DEFINE_TYPE_WITH_PRIVATE(XfceOverviewWorkspacesThemeCSS,
							xfce_overview_workspaces_theme_css,
							G_TYPE_OBJECT)

/* Properties */
enum
{
	PROP_0,

	PROP_THEME_PATH,

	PROP_LAST
};

static GParamSpec* XfceOverviewWorkspacesThemeCSSProperties[PROP_LAST]={ 0, };

/* IMPLEMENTATION: Private variables and methods */
typedef enum /*< skip,prefix=XFCE_OVERVIEW_WORKSPACES_THEME_CSS_SELECTOR_TYPE >*/
{
	XFCE_OVERVIEW_WORKSPACES_THEME_CSS_SELECTOR_TYPE_NONE=0,
	XFCE_OVERVIEW_WORKSPACES_THEME_CSS_SELECTOR_TYPE_SELECTOR,
	XFCE_OVERVIEW_WORKSPACES_THEME_CSS_SELECTOR_TYPE_CONSTANT
} XfceOverviewWorkspacesThemeCSSSelectorType;

typedef struct _XfceOverviewWorkspacesThemeCSSSelector			XfceOverviewWorkspacesThemeCSSSelector;
struct _XfceOverviewWorkspacesThemeCSSSelector
{
	XfceOverviewWorkspacesThemeCSSSelectorType	type;
	XfceOverviewWorkspacesCssSelector			*selector;
	GHashTable						*style;
};

typedef struct _XfceOverviewWorkspacesThemeCSSSelectorMatch	XfceOverviewWorkspacesThemeCSSSelectorMatch;
struct _XfceOverviewWorkspacesThemeCSSSelectorMatch
{
	XfceOverviewWorkspacesThemeCSSSelector		*selector;
	gint							score;
};

typedef struct _XfceOverviewWorkspacesThemeCSSTableCopyData	XfceOverviewWorkspacesThemeCSSTableCopyData;
struct _XfceOverviewWorkspacesThemeCSSTableCopyData
{
	GHashTable						*table;
	const gchar						*name;
};

#define XFCE_OVERVIEW_WORKSPACES_THEME_CSS_FUNCTION_CALLBACK(f)	((XfceOverviewWorkspacesThemeCSSFunctionCallback)(f))
typedef gboolean (*XfceOverviewWorkspacesThemeCSSFunctionCallback)(XfceOverviewWorkspacesThemeCSS *self,
														const gchar *inName,
														GList *inArguments,
														GValue *outResult,
														GError **outError);

/* Forward declarations */
static void _xfce_overview_workspaces_theme_css_set_error(XfceOverviewWorkspacesThemeCSS *self,
												GError **outError,
												XfceOverviewWorkspacesThemeCSSError inCode,
												const gchar *inFormat,
												...) G_GNUC_PRINTF (4, 5);

static gchar* _xfce_overview_workspaces_theme_css_resolve_at_identifier_internal(XfceOverviewWorkspacesThemeCSS *self,
																	GScanner *inScanner,
																	GScanner *inScopeScanner,
																	GList *inScopeSelectors,
																	const gchar *inRecursionIdentifier);

static gchar* _xfce_overview_workspaces_theme_css_resolve_at_identifier_by_string(XfceOverviewWorkspacesThemeCSS *self,
																		const gchar *inValue,
																		GScanner *inScopeScanner,
																		GList *inScopeSelectors,
																		const gchar *inRecursionIdentifier);

/* Helper function to set up GError object in this parser */
static void _xfce_overview_workspaces_theme_css_set_error(XfceOverviewWorkspacesThemeCSS *self,
												GError **outError,
												XfceOverviewWorkspacesThemeCSSError inCode,
												const gchar *inFormat,
												...)
{
	GError		*tempError;
	gchar		*message;
	va_list		args;

	/* Get error message */
	va_start(args, inFormat);
	message=g_strdup_vprintf(inFormat, args);
	va_end(args);

	/* Create error object */
	tempError=g_error_new_literal(XFCE_OVERVIEW_WORKSPACES_THEME_CSS_ERROR, inCode, message);

	/* Set error */
	g_propagate_error(outError, tempError);

	/* Release allocated resources */
	g_free(message);
}

/* Create css value instance */
static XfceOverviewWorkspacesThemeCSSValue* _xfce_overview_workspaces_theme_css_value_new(void)
{
	return(g_slice_new0(XfceOverviewWorkspacesThemeCSSValue));
}

/* Destroy css value instance */
static void _xfce_overview_workspaces_theme_css_value_free(XfceOverviewWorkspacesThemeCSSValue *self)
{
	g_slice_free(XfceOverviewWorkspacesThemeCSSValue, self);
}


/* Copy a property */
static void _xfce_overview_workspaces_theme_css_copy_table(gpointer *inKey, gpointer *inValue, XfceOverviewWorkspacesThemeCSSTableCopyData *inData)
{
	XfceOverviewWorkspacesThemeCSSValue		*value;

	value=_xfce_overview_workspaces_theme_css_value_new();
	value->source=inData->name;
	value->value=(gchar*)inValue;

	g_hash_table_insert(inData->table, inKey, value);
}

/* Free selector match */
static void _xfce_overview_workspaces_themes_css_selector_match_free(XfceOverviewWorkspacesThemeCSSSelectorMatch *inData)
{
	g_slice_free(XfceOverviewWorkspacesThemeCSSSelectorMatch, inData);
}

/* Append strings or characters to string in-place */
static gchar* _xfce_overview_workspaces_theme_css_append_string(gchar *ioString1, const gchar *inString2)
{
	gchar	*tmp;

	if(!ioString1) return(g_strdup(inString2));

	if(!inString2) return(ioString1);

	tmp=g_strconcat(ioString1, inString2, NULL);
	g_free(ioString1);
	return(tmp);
}

static gchar* _xfce_overview_workspaces_theme_css_append_char(gchar *ioString, gchar inChar)
{
	gchar	*tmp;
	gint	length;

	if(!ioString)
	{
		tmp=g_malloc(2);
		length=0;
	}
		else
		{
			length=strlen(ioString);
			tmp=g_realloc(ioString, length+2);
		}

	tmp[length]=inChar;
	tmp[length+1]='\0';

	return(tmp);
}

/* Destroy selector */
static void _xfce_overview_workspaces_theme_css_selector_free(XfceOverviewWorkspacesThemeCSSSelector *inSelector)
{
	g_return_if_fail(inSelector);

	/* Free allocated resources */
	if(inSelector->selector) g_object_unref(inSelector->selector);
	if(inSelector->style) g_hash_table_unref(inSelector->style);

	/* Free selector itself */
	g_slice_free(XfceOverviewWorkspacesThemeCSSSelector, inSelector);
}

/* Create selector */
static XfceOverviewWorkspacesThemeCSSSelector* _xfce_overview_workspaces_theme_css_selector_new(const gchar *inName)
{
	XfceOverviewWorkspacesThemeCSSSelector		*selector;

	selector=g_slice_new0(XfceOverviewWorkspacesThemeCSSSelector);
	selector->type=XFCE_OVERVIEW_WORKSPACES_THEME_CSS_SELECTOR_TYPE_NONE;
  
	return(selector);
}

/* Get function argument and transform it to requested type.
 * Returned value must first be cleared with g_value_unset and
 * then freed with g_free.
 */
static GValue* _xfce_overview_workspaces_theme_css_get_argument(XfceOverviewWorkspacesThemeCSS *self,
													GList *inArguments,
													guint inArgumentIndex,
													GType inRequestedType,
													GError **outError)
{
	GValue			argValue=G_VALUE_INIT;
	GValue			*resultValue;
	guint			numberArguments;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_THEME_CSS(self), NULL);
	g_return_val_if_fail(inArguments, NULL);
	g_return_val_if_fail(!outError || *outError==NULL, NULL);

	/* Check if argument index is in range */
	numberArguments=g_list_length(inArguments);
	if(inArgumentIndex>=numberArguments)
	{
		_xfce_overview_workspaces_theme_css_set_error(self,
											outError,
											XFCE_OVERVIEW_WORKSPACES_THEME_CSS_ERROR_FUNCTION_ERROR,
											"Cannot get argument %d because only %d arguments are available",
											inArgumentIndex,
											numberArguments);
		return(NULL);
	}

	/* Create empty result value of requested type */
	resultValue=g_new0(GValue, 1);
	if(!resultValue)
	{
		_xfce_overview_workspaces_theme_css_set_error(self,
											outError,
											XFCE_OVERVIEW_WORKSPACES_THEME_CSS_ERROR_FUNCTION_ERROR,
											"Cannot allocate memory for argument %d",
											inArgumentIndex);
		return(NULL);
	}

	resultValue=g_value_init(resultValue, inRequestedType);

	/* Get argument value which is a string */
	g_value_init(&argValue, G_TYPE_STRING);
	g_value_set_string(&argValue, (gchar*)g_list_nth_data(inArguments, inArgumentIndex));

	/* Transform argument value to result value of requested type */
	if(!g_value_transform(&argValue, resultValue))
	{
		/* Set error */
		_xfce_overview_workspaces_theme_css_set_error(self,
											outError,
											XFCE_OVERVIEW_WORKSPACES_THEME_CSS_ERROR_FUNCTION_ERROR,
											"Cannot transform argument %d from type '%s' to type '%s'",
											inArgumentIndex,
											g_type_name(G_VALUE_TYPE(&argValue)),
											g_type_name(G_VALUE_TYPE(resultValue)));

		/* Unset result value so that NULL will returned at the end */
		g_value_unset(resultValue);
		g_free(resultValue);
		resultValue=NULL;
	}

	/* Release allocated resources */
	g_value_unset(&argValue);

	/* Return argument value of requested type */
	return(resultValue);
}

/* Helper function to convert a string containing either a number or a percentage
 * to color component value which must be between 0 and 255.
 */
static gboolean _xfce_overview_workspaces_theme_css_parse_string_to_color_component(XfceOverviewWorkspacesThemeCSS *self,
																		const gchar *inString,
																		guint8 *outValue,
																		GError **outError)
{
	gdouble			componentValue;
	gchar			*end;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_THEME_CSS(self), FALSE);

	/* Check if string is given */
	if(!inString || *inString==0)
	{
		_xfce_overview_workspaces_theme_css_set_error(self,
											outError,
											XFCE_OVERVIEW_WORKSPACES_THEME_CSS_ERROR_FUNCTION_ERROR,
											"Missing string to convert to color component value.");
		return(FALSE);
	}

	/* Parse string to value between 0.0 and 255.0 or percentage between 0.0 and 100.0
	 * which will be transformed to a value between 0.0 and 255.0
	 */
	componentValue=g_ascii_strtod(inString, &end);
	if(*end)
	{
		if(*end=='%')
		{
			componentValue=(componentValue/100.0f)*256.0f;
			end++;
		}

		while(*end)
		{
			if(!g_ascii_isspace(*end))
			{
				gchar		*message;

				/* Set error message */
				message=g_strdup_printf("Cannot convert string '%s' to color component value.", inString);
				_xfce_overview_workspaces_theme_css_set_error(self,
													outError,
													XFCE_OVERVIEW_WORKSPACES_THEME_CSS_ERROR_FUNCTION_ERROR,
													"%s",
													message);
				g_free(message);

				/* Return with error status */
				return(FALSE);
			}
		}
	}

	/* Check if component value is in range (between 0.0 and 255.0) */
	if(componentValue<0.0 || componentValue>=256.0)
	{
		gchar		*message;

		/* Set error message */
		message=g_strdup_printf("Color component value %.2f out of range", componentValue);
		_xfce_overview_workspaces_theme_css_set_error(self,
											outError,
											XFCE_OVERVIEW_WORKSPACES_THEME_CSS_ERROR_FUNCTION_ERROR,
											"%s",
											message);
		g_free(message);

		/* Return with error status */
		return(FALSE);
	}

	/* If we get here we could compute color component value so set up result */
	if(outValue) *outValue=(guint8)componentValue;
	return(TRUE);
}

/* Cached g_file_test wrapper. Cuts repeated stat() for the same full
 * path during startup (imports + try_icons hit the same candidates for
 * every actor). Semantics unchanged: first call stats, later calls reuse.
 */
static gboolean _xfce_overview_workspaces_theme_css_cached_file_exists(XfceOverviewWorkspacesThemeCSS *self,
															const gchar *inPath)
{
	XfceOverviewWorkspacesThemeCSSPrivate	*priv;
	gpointer					value;
	gboolean					exists;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_THEME_CSS(self), FALSE);
	g_return_val_if_fail(inPath && *inPath, FALSE);

	priv=self->priv;
	if(priv->pathExistsMemo &&
		g_hash_table_lookup_extended(priv->pathExistsMemo, inPath, NULL, &value))
	{
		return(GPOINTER_TO_INT(value)!=0);
	}

	exists=g_file_test(inPath, G_FILE_TEST_EXISTS);
	if(!priv->pathExistsMemo)
	{
		priv->pathExistsMemo=g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
	}
	g_hash_table_insert(priv->pathExistsMemo, g_strdup(inPath), GINT_TO_POINTER(exists ? 1 : 0));

	return(exists);
}

/* CSS function: try_icons(icon_name[, ...])
 * Takes a variable number of arguments. Each argument is a string containing
 * the icon name to lookup. The first one which could be found successfully
 * will be returned. If no one could be found an empty string will be returned.
 */
static gboolean _xfce_overview_workspaces_theme_css_function_try_icons(XfceOverviewWorkspacesThemeCSS *self,
															const gchar *inName,
															GList *inArguments,
															GValue *outResult,
															GError **outError)
{
	XfceOverviewWorkspacesThemeCSSPrivate	*priv;
	GtkIconTheme				*iconTheme;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_THEME_CSS(self), FALSE);
	g_return_val_if_fail(inName && *inName, FALSE);
	g_return_val_if_fail(inArguments, FALSE);
	g_return_val_if_fail(outResult, FALSE);
	g_return_val_if_fail(!outError || *outError==NULL, FALSE);

	priv=self->priv;
	iconTheme=gtk_icon_theme_get_default();

	/* Initialize result with empty string. It will be overriden with found
	 * icon name if successful.
	 */
	g_value_init(outResult, G_TYPE_STRING);
	g_value_set_string(outResult, "\'\'");

	/* Iterate through arguments, try to find the icon and return first one found */
	while(inArguments)
	{
		const gchar				*iconName;
		gchar					*iconFilename;
		gpointer				memoValue;
		gboolean				memoFound;

		/* Get icon name */
		iconName=(const gchar*)inArguments->data;

		/* Memoized per-icon result: same icon names repeat for every actor
		 * styled, so skip all disk/GTK scans on repeats. No visual change.
		 */
		memoFound=FALSE;
		if(priv->iconMemo &&
			g_hash_table_lookup_extended(priv->iconMemo, iconName, NULL, &memoValue))
		{
			memoFound=(GPOINTER_TO_INT(memoValue)!=0);
			if(memoFound)
			{
				g_value_set_string(outResult, iconName);
				return(TRUE);
			}
			/* Known miss for this single name -> try next argument without stat */
			inArguments=g_list_next(inArguments);
			continue;
		}

		/* If icon name is an absolute path to a file then check if it exists */
		if(g_path_is_absolute(iconName) &&
			_xfce_overview_workspaces_theme_css_cached_file_exists(self, iconName))
		{
			/* Set up result */
			g_value_set_string(outResult, iconName);

			if(!priv->iconMemo) priv->iconMemo=g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
			g_hash_table_insert(priv->iconMemo, g_strdup(iconName), GINT_TO_POINTER(1));

			return(TRUE);
		}

		/* Check if it is a relative path to a file and check if it exists */
		iconFilename=g_build_filename(priv->themePath, iconName, NULL);
		if(_xfce_overview_workspaces_theme_css_cached_file_exists(self, iconFilename))
		{
			/* Release allocated resources */
			g_free(iconFilename);

			/* Set up result */
			g_value_set_string(outResult, iconName);

			if(!priv->iconMemo) priv->iconMemo=g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
			g_hash_table_insert(priv->iconMemo, g_strdup(iconName), GINT_TO_POINTER(1));

			return(TRUE);
		}
		g_free(iconFilename);

		/* Icon name is neither an absolute nor relative path to a file
		 * so check if it is a stock icon.
		 */
		if(gtk_icon_theme_has_icon(iconTheme, iconName))
		{
			/* Set up result */
			g_value_set_string(outResult, iconName);

			if(!priv->iconMemo) priv->iconMemo=g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
			g_hash_table_insert(priv->iconMemo, g_strdup(iconName), GINT_TO_POINTER(1));

			return(TRUE);
		}

		/* Remember miss for this single icon name to skip future stats */
		if(!priv->iconMemo) priv->iconMemo=g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
		if(!g_hash_table_contains(priv->iconMemo, iconName))
		{
			g_hash_table_insert(priv->iconMemo, g_strdup(iconName), GINT_TO_POINTER(0));
		}

		/* The icon name could not be found try next one */
		inArguments=g_list_next(inArguments);
	}

	return(TRUE);
}

/* CSS function: lighter(color) and darken(color)
 * Takes one argument which is a color and lightens or darkens it.
 */
static gboolean _xfce_overview_workspaces_theme_css_function_lighter_darker(XfceOverviewWorkspacesThemeCSS *self,
																const gchar *inName,
																GList *inArguments,
																GValue *outResult,
																GError **outError)
{
	GError			*error;
	GValue			*value;
	ClutterColor	*color;
	ClutterColor	result;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_THEME_CSS(self), FALSE);
	g_return_val_if_fail(inName && *inName, FALSE);
	g_return_val_if_fail(inArguments, FALSE);
	g_return_val_if_fail(outResult, FALSE);
	g_return_val_if_fail(!outError || *outError==NULL, FALSE);

	error=NULL;

	/* Get color to lighten or to darken */
	value=_xfce_overview_workspaces_theme_css_get_argument(self, inArguments, 0, CLUTTER_TYPE_COLOR, &error);
	if(!value)
	{
		g_propagate_error(outError, error);
		return(FALSE);
	}

	color=clutter_color_copy(clutter_value_get_color(value));
	g_value_unset(value);
	g_free(value);

	/* Lighten or darken color */
	if(g_strcmp0(inName, "lighter")==0) clutter_color_lighten(color, &result);
		else clutter_color_darken(color, &result);

	/* Set up result */
	g_value_init(outResult, CLUTTER_TYPE_COLOR);
	clutter_value_set_color(outResult, &result);

	return(TRUE);
}

/* CSS function: alpha(color, factor)
 * Takes two arguments. First one is a color and the second one is the factor to increase
 * or descrease alpha value of color with.
 */
static gboolean _xfce_overview_workspaces_theme_css_function_alpha(XfceOverviewWorkspacesThemeCSS *self,
														const gchar *inName,
														GList *inArguments,
														GValue *outResult,
														GError **outError)
{
	GError			*error;
	GValue			*value;
	ClutterColor	*color;
	gdouble			factor;
	gdouble			alpha;
	ClutterColor	result;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_THEME_CSS(self), FALSE);
	g_return_val_if_fail(inName && *inName, FALSE);
	g_return_val_if_fail(inArguments, FALSE);
	g_return_val_if_fail(outResult, FALSE);
	g_return_val_if_fail(!outError || *outError==NULL, FALSE);

	error=NULL;

	/* Get color to adjust alpha value */
	value=_xfce_overview_workspaces_theme_css_get_argument(self, inArguments, 0, CLUTTER_TYPE_COLOR, &error);
	if(!value)
	{
		g_propagate_error(outError, error);
		return(FALSE);
	}

	color=clutter_color_copy(clutter_value_get_color(value));
	g_value_unset(value);
	g_free(value);

	/* Get alpha change factor */
	value=_xfce_overview_workspaces_theme_css_get_argument(self, inArguments, 1, G_TYPE_DOUBLE, &error);
	if(!value)
	{
		/* Release allocated resources */
		clutter_color_free(color);

		/* Propagate error message and return failure result */
		g_propagate_error(outError, error);
		return(FALSE);
	}

	factor=g_value_get_double(value);
	g_value_unset(value);
	g_free(value);

	/* Change alpha value of color by factor */
	alpha=(gdouble)color->alpha * factor;
	if(alpha<0.0) alpha=0.0;
		else if(alpha>255.0) alpha=255.0;

	clutter_color_init(&result,
						color->red,
						color->green,
						color->blue,
						(guint8)alpha);

	/* Set up result */
	g_value_init(outResult, CLUTTER_TYPE_COLOR);
	clutter_value_set_color(outResult, &result);

	/* Release allocated resources */
	clutter_color_free(color);

	return(TRUE);
}

/* CSS function: shade(color, factor)
 * Takes two arguments. First one is a color and the second one is the shade factor.
 */
static gboolean _xfce_overview_workspaces_theme_css_function_shade(XfceOverviewWorkspacesThemeCSS *self,
														const gchar *inName,
														GList *inArguments,
														GValue *outResult,
														GError **outError)
{
	GError			*error;
	GValue			*value;
	ClutterColor	*color;
	gdouble			factor;
	ClutterColor	result;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_THEME_CSS(self), FALSE);
	g_return_val_if_fail(inName && *inName, FALSE);
	g_return_val_if_fail(inArguments, FALSE);
	g_return_val_if_fail(outResult, FALSE);
	g_return_val_if_fail(!outError || *outError==NULL, FALSE);

	error=NULL;

	/* Get color to shade */
	value=_xfce_overview_workspaces_theme_css_get_argument(self, inArguments, 0, CLUTTER_TYPE_COLOR, &error);
	if(!value)
	{
		g_propagate_error(outError, error);
		return(FALSE);
	}

	color=clutter_color_copy(clutter_value_get_color(value));
	g_value_unset(value);
	g_free(value);

	/* Get shade factor */
	value=_xfce_overview_workspaces_theme_css_get_argument(self, inArguments, 1, G_TYPE_DOUBLE, &error);
	if(!value)
	{
		/* Release allocated resources */
		clutter_color_free(color);

		/* Propagate error message and return failure result */
		g_propagate_error(outError, error);
		return(FALSE);
	}

	factor=g_value_get_double(value);
	g_value_unset(value);
	g_free(value);

	/* Shade color by factor */
	clutter_color_shade(color, factor, &result);

	/* Set up result */
	g_value_init(outResult, CLUTTER_TYPE_COLOR);
	clutter_value_set_color(outResult, &result);

	return(TRUE);
}

/* CSS function: mix(color1, color2, factor)
 * Takes three arguments. First two are colors and the third one is a factor
 * between 0.0 and 1.0 defining how much the resulting color moved from color1
 * to color2. The factor is more or less a progress fraction of the transformation
 * of color1 to color2.
 */
static gboolean _xfce_overview_workspaces_theme_css_function_mix(XfceOverviewWorkspacesThemeCSS *self,
													const gchar *inName,
													GList *inArguments,
													GValue *outResult,
													GError **outError)
{
	GError			*error;
	GValue			*value;
	ClutterColor	*color1;
	ClutterColor	*color2;
	gdouble			factor;
	ClutterColor	result;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_THEME_CSS(self), FALSE);
	g_return_val_if_fail(inName && *inName, FALSE);
	g_return_val_if_fail(inArguments, FALSE);
	g_return_val_if_fail(outResult, FALSE);
	g_return_val_if_fail(!outError || *outError==NULL, FALSE);

	error=NULL;

	/* Get first color (initial) */
	value=_xfce_overview_workspaces_theme_css_get_argument(self, inArguments, 0, CLUTTER_TYPE_COLOR, &error);
	if(!value)
	{
		g_propagate_error(outError, error);
		return(FALSE);
	}

	color1=clutter_color_copy(clutter_value_get_color(value));
	g_value_unset(value);
	g_free(value);

	/* Get second color (final) */
	value=_xfce_overview_workspaces_theme_css_get_argument(self, inArguments, 1, CLUTTER_TYPE_COLOR, &error);
	if(!value)
	{
		/* Release allocated resources */
		clutter_color_free(color1);

		/* Propagate error message and return failure result */
		g_propagate_error(outError, error);
		return(FALSE);
	}

	color2=clutter_color_copy(clutter_value_get_color(value));
	g_value_unset(value);
	g_free(value);

	/* Get factor (progress fraction) */
	value=_xfce_overview_workspaces_theme_css_get_argument(self, inArguments, 2, G_TYPE_DOUBLE, &error);
	if(!value)
	{
		/* Release allocated resources */
		clutter_color_free(color2);
		clutter_color_free(color1);

		/* Propagate error message and return failure result */
		g_propagate_error(outError, error);
		return(FALSE);
	}

	factor=g_value_get_double(value);
	g_value_unset(value);
	g_free(value);

	if(factor<0.0 || factor>1.0)
	{
		gchar		*message;

		/* Release allocated resources */
		clutter_color_free(color2);
		clutter_color_free(color1);

		/* Set error message */
		message=g_strdup_printf("Factor %.2f is out of range", factor);
		_xfce_overview_workspaces_theme_css_set_error(self,
											outError,
											XFCE_OVERVIEW_WORKSPACES_THEME_CSS_ERROR_FUNCTION_ERROR,
											"%s",
											message);
		g_free(message);

		/* Return with error status */
		return(FALSE);
	}

	/* Compute mixed color by factor */
	clutter_color_interpolate(color1, color2, factor, &result);

	/* Set up result */
	g_value_init(outResult, CLUTTER_TYPE_COLOR);
	clutter_value_set_color(outResult, &result);

	return(TRUE);
}

/* CSS function: rgb(r, g, b) and rgba(r, g, b, a)
 * Take variable number of arguments which must all be string. Iterates through
 * string argument and returns first one resolving in a valid icon (file or stock icon).
 */
static gboolean _xfce_overview_workspaces_theme_css_function_rgb_rgba(XfceOverviewWorkspacesThemeCSS *self,
															const gchar *inName,
															GList *inArguments,
															GValue *outResult,
															GError **outError)
{
	GError			*error;
	GValue			*value;
	gboolean		isRGBA;
	gint			i;
	guint8			color[4];
	ClutterColor	result;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_THEME_CSS(self), FALSE);
	g_return_val_if_fail(inName && *inName, FALSE);
	g_return_val_if_fail(inArguments, FALSE);
	g_return_val_if_fail(outResult, FALSE);
	g_return_val_if_fail(!outError || *outError==NULL, FALSE);

	error=NULL;

	/* Check if RGB or RGBA should be computed */
	isRGBA=(g_strcmp0(inName, "rgba")==0 ? TRUE : FALSE);

	/* Get arguments and compute color component values but stop on
	 * first argument failing.
	 */
	for(i=0; i<3; i++)
	{
		/* Get argument */
		value=_xfce_overview_workspaces_theme_css_get_argument(self, inArguments, i, G_TYPE_STRING, &error);
		if(!value)
		{
			g_propagate_error(outError, error);
			return(FALSE);
		}

		/* Compute color component value */
		if(!_xfce_overview_workspaces_theme_css_parse_string_to_color_component(self,
																	g_value_get_string(value),
																	&color[i],
																	&error))
		{
			g_propagate_error(outError, error);

			/* Release allocated resources */
			g_value_unset(value);
			g_free(value);

			return(FALSE);
		}

		/* Release allocated resources */
		g_value_unset(value);
		g_free(value);
	}

	if(isRGBA)
	{
		gfloat		alpha;

		/* Get alpha factor */
		value=_xfce_overview_workspaces_theme_css_get_argument(self, inArguments, 3, G_TYPE_DOUBLE, &error);
		if(!value)
		{
			/* Propagate error message and return failure result */
			g_propagate_error(outError, error);
			return(FALSE);
		}

		alpha=g_value_get_double(value);
		g_value_unset(value);
		g_free(value);

		if(alpha<0.0 || alpha>1.0)
		{
			gchar		*message;

			/* Set error message */
			message=g_strdup_printf("Alpha factor %.2f is out of range", alpha);
			_xfce_overview_workspaces_theme_css_set_error(self,
												outError,
												XFCE_OVERVIEW_WORKSPACES_THEME_CSS_ERROR_FUNCTION_ERROR,
												"%s",
												message);
			g_free(message);

			/* Return with error status */
			return(FALSE);
		}

		/* Set alpha factor */
		color[3]=(guint8)(alpha*255.0f);
	}
		else color[3]=0xff;

	/* Set up result */
	clutter_color_init(&result, color[0], color[1], color[2], color[3]);

	g_value_init(outResult, CLUTTER_TYPE_COLOR);
	clutter_value_set_color(outResult, &result);

	return(TRUE);
}

/* Register CSS function */
static void _xfce_overview_workspaces_theme_css_register_function(XfceOverviewWorkspacesThemeCSS *self,
														const gchar *inName,
														XfceOverviewWorkspacesThemeCSSFunctionCallback inCallback)
{
	XfceOverviewWorkspacesThemeCSSPrivate		*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_THEME_CSS(self));
	g_return_if_fail(inName);
	g_return_if_fail(inCallback);

	priv=self->priv;

	/* Create hash-table for mapping function names to function callback
	 * if this hash-table does not exist yet.
	 */
	if(!priv->registeredFunctions)
	{
		priv->registeredFunctions=g_hash_table_new_full(g_str_hash,
														g_str_equal,
														g_free,
														NULL);
	}

	/* Check if a function with this name is already registered */
	if(g_hash_table_lookup_extended(priv->registeredFunctions, inName, NULL, NULL))
	{
		g_warning("CSS function '%s' is already registered.", inName);
		return;
	}

	/* Register function by adding name (as key) and callback (as value) to
	 * hash-table of registered functions.
	 */
	g_hash_table_insert(priv->registeredFunctions, g_strdup(inName), inCallback);
}

/* Resolve '@' identifier.
 * Prints error message and returns NULL if unresolvable.
 */
static gchar* _xfce_overview_workspaces_theme_css_parse_at_identifier(XfceOverviewWorkspacesThemeCSS *self,
															GScanner *ioScanner,
															GScanner *inScopeScanner,
															GList *inScopeSelectors,
															const gchar *inRecursionIdentifier)
{
	XfceOverviewWorkspacesThemeCSSPrivate				*priv;
	GTokenType								token;
	gchar									*identifier;
	GList									*iter;
	XfceOverviewWorkspacesThemeCSSSelector				*selector;
	gpointer								value;
	gchar									*errorMessage;
	XfceOverviewWorkspacesThemeCSSFunctionCallback		functionCallback;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_THEME_CSS(self), NULL);
	g_return_val_if_fail(ioScanner, NULL);
	g_return_val_if_fail(!inRecursionIdentifier || *inRecursionIdentifier, NULL);

	priv=self->priv;

	/* Get identifier */
	token=g_scanner_get_next_token(ioScanner);
	if(token!=G_TOKEN_IDENTIFIER)
	{
		g_scanner_unexp_token(inScopeScanner,
								G_TOKEN_IDENTIFIER,
								NULL,
								NULL,
								NULL,
								"An identifier must follow '@'",
								FALSE);
		return(NULL);
	}

	identifier=g_strdup(ioScanner->value.v_identifier);

	/* Check if identifier is a function then parse function argument and
	 * call function.
	 */
	if(g_hash_table_lookup_extended(priv->registeredFunctions, identifier, NULL, (gpointer*)&functionCallback))
	{
		gboolean				error;
		GList					*arguments;
		gchar					*arg;
		GScannerConfig			*scannerConfig;
		GScannerConfig			*oldScannerConfig;
		gchar					*result;

		error=FALSE;
		arguments=NULL;
		result=NULL;
		arg=NULL;

		/* Set up scanner config for parsing function arguments */
		oldScannerConfig=ioScanner->config;

#if GLIB_CHECK_VERSION(2, 67, 4)
G_GNUC_BEGIN_IGNORE_DEPRECATIONS
		scannerConfig=(GScannerConfig*)g_memdup2(ioScanner->config, sizeof(GScannerConfig));
G_GNUC_END_IGNORE_DEPRECATIONS
#else
		scannerConfig=(GScannerConfig*)g_memdup(ioScanner->config, sizeof(GScannerConfig));
#endif
		scannerConfig->cset_skip_characters=" \t\n";
		scannerConfig->char_2_token=TRUE;
		scannerConfig->scan_string_sq=TRUE;
		scannerConfig->scan_string_dq=TRUE;
		ioScanner->config=scannerConfig;

		/* Function arguments must begin with '('. Check for it and parse function arguments ... */
		XFCE_OVERVIEW_WORKSPACES_DEBUG(self, THEME,
							"Fetching arguments to for calling function '%s'",
							identifier);

		token=g_scanner_get_next_token(ioScanner);
		if(token==G_TOKEN_LEFT_PAREN)
		{
			token=g_scanner_get_next_token(ioScanner);
			while(!error && token!=G_TOKEN_RIGHT_PAREN && token!=G_TOKEN_EOF)
			{
				switch((gint)token)
				{
					case '@':
						token=g_scanner_peek_next_token(ioScanner);
						if(token==G_TOKEN_IDENTIFIER)
						{
							gchar		*orginalValue;
							gchar		*resolvedValue;

							/* Remember identifier to resolve */
							orginalValue=g_strdup(ioScanner->value.v_identifier);

							/* Restore old scanner config */
							ioScanner->config=oldScannerConfig;

							/* Resolve '@'-identifier */
							resolvedValue=_xfce_overview_workspaces_theme_css_parse_at_identifier(self,
																						ioScanner,
																						inScopeScanner,
																						inScopeSelectors,
																						inRecursionIdentifier);

							/* Resolve resolved value to get final value */
							if(resolvedValue)
							{
								gchar	*finalResolvedValue;

								/* Resolve resolved value */
								finalResolvedValue=_xfce_overview_workspaces_theme_css_resolve_at_identifier_by_string(self,
																											resolvedValue,
																											inScopeScanner,
																											inScopeSelectors,
																											inRecursionIdentifier);

								/* Release old value and set new one */
								g_free(resolvedValue);
								resolvedValue=finalResolvedValue;
							}

							if(resolvedValue)
							{
								arg=_xfce_overview_workspaces_theme_css_append_string(arg, resolvedValue);
								g_free(resolvedValue);
							}
								else
								{
									error=TRUE;
									XFCE_OVERVIEW_WORKSPACES_DEBUG(self, THEME,
														"Could not resolve '%s' for argument #%d of function '%s'",
														orginalValue,
														g_list_length(arguments),
														identifier);
								}

							/* Set new scanner config again to parse remaining arguments */
							ioScanner->config=scannerConfig;

							/* Forget identifier */
							g_free(orginalValue);
						}
							else
							{
								error=TRUE;
								g_scanner_unexp_token(inScopeScanner,
														G_TOKEN_IDENTIFIER,
														NULL,
														NULL,
														NULL,
														"An identifier must follow '@'",
														FALSE);
							}
						break;

					case G_TOKEN_COMMA:
						if(arg)
						{
							/* Add function argument to list of arguments */
							arguments=g_list_append(arguments, arg);
							XFCE_OVERVIEW_WORKSPACES_DEBUG(self, THEME,
												"Added argument #%d: '%s'",
												g_list_length(arguments),
												arg);

							/* Prepare for next argument */
							arg=NULL;
						}
							else
							{
								error=TRUE;
								g_scanner_unexp_token(inScopeScanner,
														G_TOKEN_ERROR,
														NULL,
														NULL,
														NULL,
														"Missing function argument",
														FALSE);
							}
						break;

					case G_TOKEN_IDENTIFIER:
						arg=_xfce_overview_workspaces_theme_css_append_string(arg, ioScanner->value.v_identifier);
						break;

					case G_TOKEN_STRING:
						arg=_xfce_overview_workspaces_theme_css_append_string(arg, ioScanner->value.v_string);
						break;

					case G_TOKEN_LEFT_PAREN:
					case G_TOKEN_LEFT_CURLY:
					case G_TOKEN_RIGHT_CURLY:
					case G_TOKEN_LEFT_BRACE:
					case G_TOKEN_RIGHT_BRACE:
						{
							gchar		*message;

							error=TRUE;

							/* Set error message */
							message=g_strdup_printf("Invalid character '%c' in function argument", (gchar)token);
							g_scanner_unexp_token(inScopeScanner,
													G_TOKEN_ERROR,
													NULL,
													NULL,
													NULL,
													message,
													FALSE);
							g_free(message);
						}
						break;

					default:
						if(token<128 && g_ascii_isprint((gchar)token))
						{
							arg=_xfce_overview_workspaces_theme_css_append_char(arg, (gchar)token);
						}
							else
							{
								error=TRUE;
								g_scanner_unexp_token(inScopeScanner,
														G_TOKEN_ERROR,
														NULL,
														NULL,
														NULL,
														"Invalid character in function argument",
														FALSE);
							}
						break;
				}

				/* Continue with next token */
				token=g_scanner_get_next_token(ioScanner);
			}

			/* Add final function argument to list of arguments if available
			 * because we will not see a comma as function argument seperator
			 * which will cause the argument to be added. So check now for a
			 * remaining and final argument and add it.
			 */
			if(arg)
			{
				/* Add function argument to list of arguments */
				arguments=g_list_append(arguments, arg);
				XFCE_OVERVIEW_WORKSPACES_DEBUG(self, THEME,
									"Added final argument #%d: '%s'",
									g_list_length(arguments),
									arg);

				/* Does not make sense but just in case - prepare for next argument ;) */
				arg=NULL;
			}

			/* A function must end at ')'. If it does not it is an error. */
			if(!error && token!=G_TOKEN_RIGHT_PAREN)
			{
				error=TRUE;
				g_scanner_unexp_token(inScopeScanner,
										G_TOKEN_RIGHT_PAREN,
										NULL,
										NULL,
										NULL,
										"Missing ')' after function",
										FALSE);
			}
		}
			/* ... otherwise it an error because the function arguments did not begin
			 * with '(' and could not be parsed.
			 */
			else
			{
				error=TRUE;
				g_scanner_unexp_token(inScopeScanner,
										G_TOKEN_LEFT_PAREN,
										NULL,
										NULL,
										NULL,
										"Missing '(' after function",
										FALSE);
			}

		/* Restore old scanner config */
		ioScanner->config=oldScannerConfig;
		g_free(scannerConfig);

		/* Do function call if no error occured so far */
		if(!error)
		{
			GValue				functionValue=G_VALUE_INIT;
			GError				*functionError;
			gboolean			functionSuccess;

			/* Initialize for function call */
			functionError=NULL;

			XFCE_OVERVIEW_WORKSPACES_DEBUG(self, THEME,
								"Calling registered function %s with %d arguments",
								identifier,
								g_list_length(arguments));
			functionSuccess=(functionCallback)(self, identifier, arguments, &functionValue, &functionError);
			if(functionSuccess)
			{
				GValue			stringValue=G_VALUE_INIT;

				/* Transform function value into a string */
				g_value_init(&stringValue, G_TYPE_STRING);

				if(g_value_transform(&functionValue, &stringValue))
				{
					result=g_value_dup_string(&stringValue);
				}
					else
					{
						error=TRUE;

						/* Set error message */
						errorMessage=g_strdup_printf("Could not transform result of function '%s' to a string",
														identifier);
						g_scanner_unexp_token(inScopeScanner,
												G_TOKEN_ERROR,
												NULL,
												NULL,
												NULL,
												errorMessage,
												FALSE);
						g_free(errorMessage);
					}

				/* Release allocated resources */
				g_value_unset(&stringValue);
				g_value_unset(&functionValue);

				XFCE_OVERVIEW_WORKSPACES_DEBUG(self, THEME,
									"Calling function %s with %d arguments succeeded with result: %s",
									identifier,
									g_list_length(arguments),
									result);
			}
				else
				{
					gchar		*message;

					XFCE_OVERVIEW_WORKSPACES_DEBUG(self, THEME,
										"Calling function %s with %d arguments failed: %s",
										identifier,
										g_list_length(arguments),
										(functionError && functionError->message) ? functionError->message : "Unknown error");

					/* Set error message */
					if(functionError && functionError->message)
					{
						message=g_strdup_printf("Function '%s' failed with error: %s",
													identifier,
													functionError->message);
					}
						else
						{
							message=g_strdup_printf("Function '%s' failed!",
														identifier);
						}

					g_scanner_unexp_token(inScopeScanner,
											G_TOKEN_ERROR,
											NULL,
											NULL,
											NULL,
											message,
											FALSE);

					/* Release allocated resources */
					if(functionError) g_error_free(functionError);
					if(message) g_free(message);
				}
		}

		/* Release allocated resources */
		if(arguments) g_list_free_full(arguments, (GDestroyNotify)g_free);
		g_free(identifier);

		/* Return value found */
		return(result);
	}

	/* Identifier is a constant so lookup constant by iterating through all
	 * constants selectors backwards and use first value found. We iterate
	 * through these selectors backwards (first in selectors of current file/scope
	 * then all previous ones) to let last definition win.
	 */
	if(inRecursionIdentifier &&
		g_strcmp0(identifier, inRecursionIdentifier)==0)
	{
		/* Identifier was unresolvable because of indefinite recursion,
		 * so print error message and also a critical message.
		 */
		g_critical("Indefinte recursion of @-constant '%s' detected - aborting parsing", inRecursionIdentifier);

		errorMessage=g_strdup_printf("Cannot resolve identifier '@%s' because of indefinite recursion", identifier);
		g_scanner_unexp_token(inScopeScanner,
								G_TOKEN_ERROR,
								NULL,
								NULL,
								NULL,
								errorMessage,
								FALSE);
		g_free(errorMessage);

		/* Release allocated resources */
		g_free(identifier);

		/* Identifier was unresolvable so return NULL */
		return(NULL);
	}

	for(iter=g_list_last(inScopeSelectors); iter; iter=g_list_previous(iter))
	{
		selector=(XfceOverviewWorkspacesThemeCSSSelector*)iter->data;

		/* Handle only constants selectors */
		if(selector->type==XFCE_OVERVIEW_WORKSPACES_THEME_CSS_SELECTOR_TYPE_CONSTANT)
		{
			if(g_hash_table_lookup_extended(selector->style, identifier, NULL, &value))
			{
				/* Release allocated resources */
				g_free(identifier);

				/* Return value found */
				return(g_strdup(value));
			}
		}
	}

	for(iter=g_list_last(priv->selectors); iter; iter=g_list_previous(iter))
	{
		selector=(XfceOverviewWorkspacesThemeCSSSelector*)iter->data;

		/* Handle only constants selectors */
		if(selector->type==XFCE_OVERVIEW_WORKSPACES_THEME_CSS_SELECTOR_TYPE_CONSTANT)
		{
			if(g_hash_table_lookup_extended(selector->style, identifier, NULL, &value))
			{
				/* Release allocated resources */
				g_free(identifier);

				/* Return value found */
				return(g_strdup(value));
			}
		}
	}

	/* Identifier was unresolvable so print error message */
	errorMessage=g_strdup_printf("Unresolvable identifier '@%s'", identifier);
	g_scanner_unexp_token(inScopeScanner,
							G_TOKEN_ERROR,
							NULL,
							NULL,
							NULL,
							errorMessage,
							FALSE);
	g_free(errorMessage);

	/* Release allocated resources */
	g_free(identifier);

	/* Identifier was unresolvable so return NULL */
	return(NULL);
}

static gchar* _xfce_overview_workspaces_theme_css_resolve_at_identifier_internal(XfceOverviewWorkspacesThemeCSS *self,
																	GScanner *inScanner,
																	GScanner *inScopeScanner,
																	GList *inScopeSelectors,
																	const gchar *inRecursionIdentifier)
{
	GTokenType		token;
	gchar			*value;
	gchar			*haveResolvedAtIdentifier;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_THEME_CSS(self), NULL);
	g_return_val_if_fail(inScanner, NULL);
	g_return_val_if_fail(inScopeScanner, NULL);
	g_return_val_if_fail(!inRecursionIdentifier || *inRecursionIdentifier, NULL);

	/* Parse value and resolve '@' identifier */
	haveResolvedAtIdentifier=NULL;
	value=NULL;

	token=g_scanner_get_next_token(inScanner);
	while(token!=G_TOKEN_EOF)
	{
		switch(token)
		{
			case G_TOKEN_IDENTIFIER:
				value=_xfce_overview_workspaces_theme_css_append_string(value, inScanner->value.v_identifier);
				break;

			case G_TOKEN_STRING:
				value=_xfce_overview_workspaces_theme_css_append_string(value, inScanner->value.v_string);
				break;

			case G_TOKEN_CHAR:
				/* Check for character '@' identifing an identifier which needs to get resolved ... */
				if(inScanner->value.v_char=='@')
				{
					gchar		*constantValue;

					/* Resolve value and append resolved value but stop parsing and return NULL
					 * if unresolvable. Also remember '@' identifier we have resolved to get the
					 * new value resolved.
					 */
					token=g_scanner_peek_next_token(inScanner);
					if(token!=G_TOKEN_IDENTIFIER) return(NULL);

					haveResolvedAtIdentifier=g_strdup(inScanner->next_value.v_identifier);

					constantValue=_xfce_overview_workspaces_theme_css_parse_at_identifier(self, inScanner, inScopeScanner, inScopeSelectors, inRecursionIdentifier);
					if(!constantValue)
					{
						g_free(value);
						return(NULL);
					}

					value=_xfce_overview_workspaces_theme_css_append_string(value, constantValue);

					/* Release allocated resources */
					g_free(constantValue);
				}
					/* ... otherwise just add character to value */
					else value=_xfce_overview_workspaces_theme_css_append_char(value, inScanner->value.v_char);
				break;

			default:
				/* This code should never be reached! */
				g_assert_not_reached();
				break;
		}

		/* Continue with next token */
		token=g_scanner_get_next_token(inScanner);
	}

	/* If any '@' identifier was found and resolved, we need to re-parse resolved value
	 * for further '@' identifiers.
	 */
	if(haveResolvedAtIdentifier)
	{
		gchar		*resolvedValue;

		/* Call ourselve recursive (via _xfce_overview_workspaces_theme_css_resolve_at_identifier_by_string)
		 * to resolve any '@' identifier which might be in value resolved this time.
		 */
		XFCE_OVERVIEW_WORKSPACES_DEBUG(self, THEME,
							"Resolving css value '%s' because of constant identifier '%s'",
							value,
							haveResolvedAtIdentifier);

		resolvedValue=_xfce_overview_workspaces_theme_css_resolve_at_identifier_by_string(self,
																				value,
																				inScopeScanner,
																				inScopeSelectors,
																				haveResolvedAtIdentifier);

		XFCE_OVERVIEW_WORKSPACES_DEBUG(self, THEME,
							"Resolved css value '%s' to '%s' recursively because of constant identifier '%s'",
							value,
							resolvedValue,
							haveResolvedAtIdentifier);

		/* Release old value and new one */
		g_free(value);
		value=resolvedValue;

		/* Release allocated resources */
		g_free(haveResolvedAtIdentifier);
	}

	/* Return resolved value */
	return(value);
}

static gchar* _xfce_overview_workspaces_theme_css_resolve_at_identifier_by_string(XfceOverviewWorkspacesThemeCSS *self,
																		const gchar *inText,
																		GScanner *inScopeScanner,
																		GList *inScopeSelectors,
																		const gchar *inRecursionIdentifier)
{
	GScanner		*scanner;
	gchar			*value;
	static gint		callDepth=0;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_THEME_CSS(self), NULL);
	g_return_val_if_fail(inScopeScanner, NULL);
	g_return_val_if_fail(inText, NULL);
	g_return_val_if_fail(!inRecursionIdentifier || *inRecursionIdentifier, NULL);

	/* Increment call depth and check if it is too deep now to avoid recursive
	 * resolves of '@' identifier definitions.
	 */
	callDepth++;
	if(callDepth>128)
	{
		gchar		*error;

		error=g_strdup_printf("Possible recursion detected because call depth is too deep while resolving '%s'",
								inText);
		g_scanner_unexp_token(inScopeScanner,
								G_TOKEN_ERROR,
								NULL,
								NULL,
								NULL,
								error,
								FALSE);
		g_free(error);

		return(NULL);
	}

	/* Create scanner to resolve value of '@' identifier */
	scanner=g_scanner_new(NULL);

	/* Set up scanner config
	 * - Identifiers are allowed to contain '-' (minus sign) as non-first characters
	 * - Disallow scanning float values as we need '.' for identifiers
	 * - Set up single comment line not to include '#' as this character is need for identifiers
	 * - Disable parsing HEX values
	 * - Identifiers cannot be single quoted
	 * - Identifiers cannot be double quoted
	 */
	scanner->config->cset_identifier_first=G_CSET_a_2_z "#_-0123456789" G_CSET_A_2_Z G_CSET_LATINS G_CSET_LATINC;
	scanner->config->cset_identifier_nth=scanner->config->cset_identifier_first;
	scanner->config->scan_identifier_1char=1;
	scanner->config->char_2_token=FALSE;
	scanner->config->cset_skip_characters="";
	scanner->config->scan_string_sq=TRUE;
	scanner->config->scan_string_dq=TRUE;
	scanner->config->scan_float=FALSE;

	/* Set string to parse and resolve */
	g_scanner_input_text(scanner, inText, strlen(inText));

	/* Resolve '@' identifier */
	value=_xfce_overview_workspaces_theme_css_resolve_at_identifier_internal(self,
																scanner,
																inScopeScanner,
																inScopeSelectors,
																inRecursionIdentifier);

	/* Destroy scanner */
	g_scanner_destroy(scanner);

	/* Decrement call depth counter again */
	callDepth--;

	/* Return resolved '@' identifier which may be NULL in case of error */
	return(value);
}

/* Parse CSS from stream */
static GTokenType _xfce_overview_workspaces_theme_css_parse_css_key_value(XfceOverviewWorkspacesThemeCSS *self,
																GScanner *inScanner,
																GList *inScopeSelectors,
																gboolean inDoResolveAt,
																gchar **outKey,
																gchar **outValue)
{
	GTokenType		token;
	gboolean		propertyStartsWithDash;
	GScannerConfig	*scannerConfig;
	GScannerConfig	*oldScannerConfig;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_THEME_CSS(self), G_TOKEN_ERROR);
	g_return_val_if_fail(inScanner, G_TOKEN_ERROR);
	g_return_val_if_fail(outKey && *outKey==NULL, G_TOKEN_ERROR);
	g_return_val_if_fail(outValue && *outValue==NULL, G_TOKEN_ERROR);

	/* Parse property name. Property names can start with '-' but it needs
	 * an identifier at least
	 */
	propertyStartsWithDash=FALSE;

	token=g_scanner_get_next_token(inScanner);
	if(token=='-')
	{
		token=g_scanner_get_next_token(inScanner);
		propertyStartsWithDash=TRUE;
	}

	if(token!=G_TOKEN_IDENTIFIER)
	{
		g_scanner_unexp_token(inScanner,
								G_TOKEN_IDENTIFIER,
								NULL,
								NULL,
								NULL,
								"Invalid property name",
								TRUE);

		return(G_TOKEN_IDENTIFIER);
	}

	/* Build key */
	if(propertyStartsWithDash) *outKey=g_strconcat("-", inScanner->value.v_identifier, NULL);
		else *outKey=g_strdup(inScanner->value.v_identifier);

	/* Key and value must be seperated by a colon */
	token=g_scanner_get_next_token(inScanner);
	if(token!=':')
	{
		/* Show parser error message */
		g_scanner_unexp_token(inScanner,
								':',
								NULL,
								NULL,
								NULL,
								"Property names and values must be separated by colon",
								TRUE);

		/* Return error result */
		return(':');
	}

	/* Set parser option to parse property value and parse them */
#if GLIB_CHECK_VERSION(2, 67, 4)
G_GNUC_BEGIN_IGNORE_DEPRECATIONS
	scannerConfig=(GScannerConfig*)g_memdup2(inScanner->config, sizeof(GScannerConfig));
G_GNUC_END_IGNORE_DEPRECATIONS
#else
	scannerConfig=(GScannerConfig*)g_memdup(inScanner->config, sizeof(GScannerConfig));
#endif
	scannerConfig->cset_identifier_first=G_CSET_a_2_z "#_-0123456789" G_CSET_A_2_Z G_CSET_LATINS G_CSET_LATINC;
	scannerConfig->cset_identifier_nth=scannerConfig->cset_identifier_first;
	scannerConfig->scan_identifier_1char=1;
	scannerConfig->char_2_token=FALSE;
	scannerConfig->cset_skip_characters="\n";
	scannerConfig->scan_string_sq=TRUE;
	scannerConfig->scan_string_dq=TRUE;

	oldScannerConfig=inScanner->config;
	inScanner->config=scannerConfig;

	while(inScanner->next_token!=G_TOKEN_CHAR ||
			inScanner->next_value.v_char!=';')
	{
		token=g_scanner_get_next_token(inScanner);
		switch(token)
		{
			case G_TOKEN_IDENTIFIER:
				*outValue=_xfce_overview_workspaces_theme_css_append_string(*outValue, inScanner->value.v_identifier);
				break;

			case G_TOKEN_CHAR:
				*outValue=_xfce_overview_workspaces_theme_css_append_char(*outValue, inScanner->value.v_char);
				break;

			case G_TOKEN_STRING:
				/* If we should not re-scan the value to resolve it then we need to enclose the string
				 * we get from scanner with single quotation marks. It must be single quotation marks
				 * because the value is already escaped and they are needed to get G_TOKEN_STRING tokens
				 * from subsequent scanner in further resolving attempts.
				 */
				if(!inDoResolveAt) *outValue=_xfce_overview_workspaces_theme_css_append_char(*outValue, '\'');
				*outValue=_xfce_overview_workspaces_theme_css_append_string(*outValue, inScanner->value.v_string);
				if(!inDoResolveAt) *outValue=_xfce_overview_workspaces_theme_css_append_char(*outValue, '\'');
				break;

			default:
				/* Restore old parser options */
				inScanner->config=oldScannerConfig;
				g_free(scannerConfig);

				/* Show parser error message */
				g_scanner_unexp_token(inScanner,
										';',
										NULL,
										NULL,
										NULL,
										"Invalid property value",
										TRUE);

				/* Return error result */
				return(';');
		}

		g_scanner_peek_next_token(inScanner);
	}

	/* Property values must end at a semi-colon */
	token=g_scanner_get_next_token(inScanner);
	if(token!=G_TOKEN_CHAR ||
		inScanner->value.v_char!=';')
	{
		/* Restore old parser options */
		inScanner->config=oldScannerConfig;
		g_free(scannerConfig);

		/* Show parser error message */
		g_scanner_unexp_token(inScanner,
								';',
								NULL,
								NULL,
								NULL,
								"Property values must end with semi-colon",
								TRUE);

		/* Return error result */
		return(';');
	}

	/* Resolve '@' identifiers if requested */
	if(inDoResolveAt && *outValue)
	{
		gchar		*resolvedValue;

		/* Resolve value */
		XFCE_OVERVIEW_WORKSPACES_DEBUG(self, THEME,
							"Resolving css value '%s'",
							*outValue);
		resolvedValue=_xfce_overview_workspaces_theme_css_resolve_at_identifier_by_string(self,
																				*outValue,
																				inScanner,
																				inScopeSelectors,
																				NULL);
		XFCE_OVERVIEW_WORKSPACES_DEBUG(self, THEME,
							"Resolved css value '%s' to '%s'",
							*outValue,
							resolvedValue);

		/* Release old value and set new one */
		g_free(*outValue);
		*outValue=resolvedValue;
	}

	/* Strip leading and trailing whitespace from value */
	if(*outValue) g_strstrip(*outValue);

	/* Restore old parser options */
	inScanner->config=oldScannerConfig;
	g_free(scannerConfig);

	/* If no value (means NULL value) is set when '@' identifiers were resolved
	 * then an error is occurred.
	 */
	if(inDoResolveAt && !*outValue) return(G_TOKEN_ERROR);

	/* Successfully parsed */
	return(G_TOKEN_NONE);
}

static GTokenType _xfce_overview_workspaces_theme_css_parse_css_styles(XfceOverviewWorkspacesThemeCSS *self,
															GScanner *inScanner,
															GList *inScopeSelectors,
															gboolean inDoResolveAt,
															GHashTable *ioHashtable)
{
	GTokenType		token;
	gchar			*key;
	gchar			*value;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_THEME_CSS(self), G_TOKEN_ERROR);
	g_return_val_if_fail(inScanner, G_TOKEN_ERROR);
	g_return_val_if_fail(ioHashtable, G_TOKEN_ERROR);

	/* Check that style begin with left curly bracket */
	token=g_scanner_get_next_token(inScanner);
	if(token!=G_TOKEN_LEFT_CURLY) return(G_TOKEN_LEFT_CURLY);

	/* Parse styles until closing right curly bracket is reached */
	token=g_scanner_peek_next_token(inScanner);
	while(token!=G_TOKEN_RIGHT_CURLY)
	{
		/* Reset key and value variables */
		key=value=NULL;

		/* Parse key and value */
		token=_xfce_overview_workspaces_theme_css_parse_css_key_value(self,
															inScanner,
															inScopeSelectors,
															inDoResolveAt,
															&key,
															&value);
		if(token!=G_TOKEN_NONE) return(token);

		/* Insert key and value into hashtable */
		g_hash_table_insert(ioHashtable, key, value);

		/* Get next token */
		token=g_scanner_peek_next_token(inScanner);
	}

	/* If we get here we expect the right curly bracket */
	token=g_scanner_get_next_token(inScanner);
	if(token!=G_TOKEN_RIGHT_CURLY) return(G_TOKEN_RIGHT_CURLY);

	/* Successfully parsed */
	return(G_TOKEN_NONE);
}

static GTokenType _xfce_overview_workspaces_theme_css_command_import(XfceOverviewWorkspacesThemeCSS *self,
														GScanner *inScanner,
														GList **ioSelectors)
{
	XfceOverviewWorkspacesThemeCSSPrivate	*priv;
	GTokenType					token;
	GScannerConfig				*scannerConfig;
	GScannerConfig				*oldScannerConfig;
	gchar						*filename;
	GError						*error;
	gint						oldLineOffset;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_THEME_CSS(self), G_TOKEN_ERROR);
	g_return_val_if_fail(inScanner, G_TOKEN_ERROR);
	g_return_val_if_fail(ioSelectors, G_TOKEN_ERROR);

	priv=self->priv;
	filename=NULL;
	error=NULL;

	/* Set parser option to parse property value and parse them */
#if GLIB_CHECK_VERSION(2, 67, 4)
G_GNUC_BEGIN_IGNORE_DEPRECATIONS
	scannerConfig=(GScannerConfig*)g_memdup2(inScanner->config, sizeof(GScannerConfig));
G_GNUC_END_IGNORE_DEPRECATIONS
#else
	scannerConfig=(GScannerConfig*)g_memdup(inScanner->config, sizeof(GScannerConfig));
#endif
	scannerConfig->scan_identifier_1char=1;
	scannerConfig->char_2_token=FALSE;
	scannerConfig->scan_string_sq=TRUE;
	scannerConfig->scan_string_dq=TRUE;

	oldScannerConfig=inScanner->config;
	inScanner->config=scannerConfig;

	/* Syntax is: @import(filename)
	 * Expect '(' as next character in stream.
	 */
	token=g_scanner_get_next_token(inScanner);
	if(token!=G_TOKEN_CHAR ||
		inScanner->value.v_char!='(')
	{
		/* Restore old parser options */
		inScanner->config=oldScannerConfig;
		g_free(scannerConfig);

		/* Show parser error message */
		g_scanner_unexp_token(inScanner,
								G_TOKEN_LEFT_PAREN,
								NULL,
								NULL,
								NULL,
								"Missing '(' after @import keyword.",
								TRUE);
		return(G_TOKEN_LEFT_PAREN);
	}

	/* Expect only identifiers, characters and strings and concetenate
	 * them to a filename. But stop at closing ')' character.
	 */
	token=g_scanner_get_next_token(inScanner);
	while((token==G_TOKEN_CHAR && inScanner->value.v_char!=')') ||
			token==G_TOKEN_IDENTIFIER ||
			token==G_TOKEN_STRING)
	{
		switch(token)
		{
			case G_TOKEN_CHAR:
				filename=_xfce_overview_workspaces_theme_css_append_char(filename, inScanner->value.v_char);
				break;

			case G_TOKEN_STRING:
				filename=_xfce_overview_workspaces_theme_css_append_string(filename, inScanner->value.v_string);
				break;

			case G_TOKEN_IDENTIFIER:
				filename=_xfce_overview_workspaces_theme_css_append_string(filename, inScanner->value.v_identifier);
				break;

			default:
				/* Restore old parser options */
				inScanner->config=oldScannerConfig;
				g_free(scannerConfig);

				/* Release allocated resources */
				if(filename) g_free(filename);

				/* Show parser error message */
				g_scanner_unexp_token(inScanner,
										G_TOKEN_ERROR,
										NULL,
										NULL,
										NULL,
										"Unexpected token in filename to import",
										TRUE);
				return(token);
		}

		token=g_scanner_get_next_token(inScanner);
	}

	if(!filename)
	{
		/* Restore old parser options */
		inScanner->config=oldScannerConfig;
		g_free(scannerConfig);

		/* Show parser error message */
		g_scanner_unexp_token(inScanner,
								G_TOKEN_ERROR,
								NULL,
								NULL,
								NULL,
								"Missing filename to import",
								TRUE);
		return(G_TOKEN_ERROR);
	}

	/* Expect closing ')' as next character in stream */
	if(token!=G_TOKEN_CHAR ||
		inScanner->value.v_char!=')')
	{
		/* Restore old parser options */
		inScanner->config=oldScannerConfig;
		g_free(scannerConfig);

		/* Release allocated resources */
		if(filename) g_free(filename);

		/* Show parser error message */
		g_scanner_unexp_token(inScanner,
								G_TOKEN_RIGHT_PAREN,
								NULL,
								NULL,
								NULL,
								"Missing closing ')' at @import keyword.",
								TRUE);
		return(G_TOKEN_RIGHT_PAREN);
	}

	/* Import and parse CSS file */
	if(!g_path_is_absolute(filename))
	{
		gchar					*tempFilename;
		gchar					*cssPath;
		gboolean				foundFile;

		foundFile=FALSE;

		/* Path to file is relative so check if a file, relative to
		 * path of CSS file currently parsed, exists.
		 */
		if(inScanner->input_fd>0 &&
			inScanner->input_name)
		{
			cssPath=g_path_get_dirname(inScanner->input_name);

			tempFilename=g_build_filename(cssPath, filename, NULL);
			if(_xfce_overview_workspaces_theme_css_cached_file_exists(self, tempFilename))
			{
				XFCE_OVERVIEW_WORKSPACES_DEBUG(self, THEME,
									"Resolved relative path '%s' to import to '%s' which is relative to current css file '%s'.",
									filename,
									tempFilename,
									inScanner->input_name);

				g_free(filename);
				filename=g_strdup(tempFilename);
				foundFile=TRUE;
			}

			g_free(cssPath);
			g_free(tempFilename);
		}

		/* If current CSS being parsed is not a file or the relative path
		 * to current CSS file to import does not exist, assume it is a
		 * relative path to theme.
		 */
		if(!foundFile)
		{
			tempFilename=g_build_filename(priv->themePath, filename, NULL);

			XFCE_OVERVIEW_WORKSPACES_DEBUG(self, THEME,
								"Resolved relative path '%s' to import to '%s' which is relative to theme path '%s'.",
								filename,
								tempFilename,
								priv->themePath);

			g_free(filename);
			filename=g_strdup(tempFilename);
			g_free(tempFilename);
		}
	}

	oldLineOffset=priv->offsetLine;
	priv->offsetLine+=g_scanner_cur_line(inScanner);
	if(!xfce_overview_workspaces_theme_css_add_file(self, filename, GPOINTER_TO_INT(inScanner->user_data), &error))
	{
		gchar					*errorMessage;

		/* Build error message */
		errorMessage=g_strdup_printf("Failed to import CSS file '%s': %s",
										filename,
										error && error->message ? error->message : "Unknown error");

		/* Show parser error message */
		g_scanner_unexp_token(inScanner,
								G_TOKEN_ERROR,
								NULL,
								NULL,
								NULL,
								errorMessage,
								TRUE);

		/* Restore old parser options */
		inScanner->config=oldScannerConfig;
		g_free(scannerConfig);

		/* Release allocated resources */
		if(error) g_error_free(error);
		if(errorMessage) g_free(errorMessage);
		if(filename) g_free(filename);

		/* Correct line offset */
		priv->offsetLine-=oldLineOffset;

		/* Return error result */
		return(G_TOKEN_ERROR);
	}

	XFCE_OVERVIEW_WORKSPACES_DEBUG(self, THEME,
						"Imported CSS file '%s'",
						filename);

	/* Correct line offset */
	priv->offsetLine-=oldLineOffset;

	/* Restore old parser options */
	inScanner->config=oldScannerConfig;
	g_free(scannerConfig);

	/* Release allocated resources */
	if(error) g_error_free(error);
	if(filename) g_free(filename);

	/* Import was successful so return success value */
	return(G_TOKEN_NONE);
}

static gboolean _xfce_overview_workspaces_theme_css_parse_css_ruleset_finish(XfceOverviewWorkspacesCssSelector *inSelector,
																GScanner *inScanner,
																GTokenType inPeekNextToken,
																gpointer inUserData)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_CSS_SELECTOR(inSelector), XFCE_OVERVIEW_WORKSPACES_CSS_SELECTOR_PARSE_FINISH_BAD_STATE);
	g_return_val_if_fail(inScanner, XFCE_OVERVIEW_WORKSPACES_CSS_SELECTOR_PARSE_FINISH_BAD_STATE);
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_THEME_CSS(inUserData), XFCE_OVERVIEW_WORKSPACES_CSS_SELECTOR_PARSE_FINISH_BAD_STATE);

	/* Scanner is in good state if current token in stream is a comma
	 * or a left curly bracket.
	 */
	if(inPeekNextToken=='{') return(XFCE_OVERVIEW_WORKSPACES_CSS_SELECTOR_PARSE_FINISH_OK);

	if(inPeekNextToken==',')
	{
		/* Eat comma from stream to adjust cursor to next token */
		g_scanner_get_next_token(inScanner);

		return(XFCE_OVERVIEW_WORKSPACES_CSS_SELECTOR_PARSE_FINISH_OK);
	}

	/* If we get here scanner is in bad state now */
	return(XFCE_OVERVIEW_WORKSPACES_CSS_SELECTOR_PARSE_FINISH_BAD_STATE);
}

static GTokenType _xfce_overview_workspaces_theme_css_parse_css_ruleset(XfceOverviewWorkspacesThemeCSS *self,
															GScanner *inScanner,
															GList **ioSelectors)
{
	XfceOverviewWorkspacesThemeCSSPrivate		*priv;
	GTokenType						token;
	XfceOverviewWorkspacesThemeCSSSelector		*selector;
	gboolean						hasAtSelector;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_THEME_CSS(self), G_TOKEN_ERROR);
	g_return_val_if_fail(inScanner, G_TOKEN_ERROR);
	g_return_val_if_fail(ioSelectors, G_TOKEN_ERROR);

	priv=self->priv;

	/* Parse comma-seperated selectors until a left curly bracket is found */
	selector=NULL;
	hasAtSelector=FALSE;

	token=g_scanner_peek_next_token(inScanner);
	while(token!=G_TOKEN_LEFT_CURLY)
	{
		/* Check if a @-identifier was parsed because it can only be one
		 * in a ruleset and @-identifers and selectors cannot be mixed.
		 */
		if(hasAtSelector)
		{
			g_scanner_get_next_token(inScanner);
			g_scanner_unexp_token(inScanner,
									G_TOKEN_LEFT_CURLY,
									NULL,
									NULL,
									NULL,
									"Mixing selectors and '@' identifiers or defining more than one '@' identifier at once is not allowed",
									TRUE);
			return(G_TOKEN_LEFT_CURLY);
		}

		switch((guint)token)
		{
			case G_TOKEN_IDENTIFIER:
			case '*':
			case '#':
			case '.':
			case ':':
			case ',':
				/* A selector must have been defined before other one can follow comma-separated */
				if(token==',' && g_list_length(*ioSelectors)==0)
				{
					g_scanner_unexp_token(inScanner,
											G_TOKEN_IDENTIFIER,
											NULL,
											NULL,
											NULL,
											"A selector must have been defined before other one can follow comma-separated.",
											TRUE);
					return(token);
				}

				/* Create new selector and add it to list of read-in selectors */
				selector=_xfce_overview_workspaces_theme_css_selector_new(inScanner->input_name);
				*ioSelectors=g_list_prepend(*ioSelectors, selector);

				/* Parse selector */
				selector->selector=xfce_overview_workspaces_css_selector_new_from_scanner_with_priority(inScanner,
																							GPOINTER_TO_INT(inScanner->user_data),
																							(XfceOverviewWorkspacesCssSelectorParseFinishCallback)_xfce_overview_workspaces_theme_css_parse_css_ruleset_finish,
																							self);
				if(!selector->selector) return(G_TOKEN_ERROR);

				xfce_overview_workspaces_css_selector_adjust_to_offset(selector->selector, priv->offsetLine, 0);

				/* If we get here selector could be parse so set type */
				selector->type=XFCE_OVERVIEW_WORKSPACES_THEME_CSS_SELECTOR_TYPE_SELECTOR;
				break;

			case '@':
				g_scanner_get_next_token(inScanner);

				/* An identifier must follow the character '@' */
				token=g_scanner_get_next_token(inScanner);
				if(token!=G_TOKEN_IDENTIFIER)
				{
					g_scanner_unexp_token(inScanner,
											G_TOKEN_IDENTIFIER,
											NULL,
											NULL,
											NULL,
											"An identifier must follow '@'",
											TRUE);
					return(token);
				}

				/* Handle '@constants'-identifier */
				if(g_strcmp0(inScanner->value.v_identifier, "constants")==0)
				{
					/* Create selector for constant @-identifier */
					selector=_xfce_overview_workspaces_theme_css_selector_new(inScanner->input_name);
					selector->type=XFCE_OVERVIEW_WORKSPACES_THEME_CSS_SELECTOR_TYPE_CONSTANT;

					/* Add it to list of read-in selectors */
					*ioSelectors=g_list_prepend(*ioSelectors, selector);

					/* Set flag that an '@'-selector was parsed */
					hasAtSelector=TRUE;
				}
					/* Handle '@import'-identifier */
					else if(g_strcmp0(inScanner->value.v_identifier, "import")==0)
					{
						token=_xfce_overview_workspaces_theme_css_command_import(self,
																	inScanner,
																	ioSelectors);
						if(token!=G_TOKEN_NONE) return(token);
					}
					/* If we get here the '@'-identifier is unknown and could not be handled, skip it */
					else
					{
						gchar		*message;

						/* Build warning message */
						message=g_strdup_printf("Skipping block of unknown identifier '@%s'",
												inScanner->value.v_identifier);

						/* Set warning */
						g_scanner_unexp_token(inScanner,
												G_TOKEN_IDENTIFIER,
												"'@' identifier",
												NULL,
												NULL,
												message,
												FALSE);

						/* Release allocated resources */
						g_free(message);

						/* As this is just a warning return G_TOKEN_NONE to continue
						 * parsing CSS. Otherwise parses would stop with error.
						 */
						return(G_TOKEN_NONE);
					}

				break;

			case G_TOKEN_EOF:
				/* If any selector was parsed then a css block must follow.
				 * Reaching end of file now is an error ...
				 */
				if(g_list_length(*ioSelectors)!=0 || selector)
				{
					g_scanner_get_next_token(inScanner);
					g_scanner_unexp_token(inScanner,
											G_TOKEN_ERROR,
											NULL,
											NULL,
											NULL,
											"Unhandled selector",
											TRUE);
					return(G_TOKEN_LEFT_CURLY);
				}

				/* ... otherwise it is ok so return G_TOKEN_EOF for
				 * success result and to stop further parses.
				 */
				return(G_TOKEN_EOF);

			default:
				g_scanner_get_next_token(inScanner);
				g_scanner_unexp_token(inScanner,
										G_TOKEN_ERROR,
										NULL,
										NULL,
										NULL,
										"Unhandled selector",
										TRUE);
				return(G_TOKEN_LEFT_CURLY);
		}

		/* Continue parsing with next token */
		token=g_scanner_peek_next_token(inScanner);
	}

	/* Successfully parsed */
	return(G_TOKEN_NONE);
}

static GTokenType _xfce_overview_workspaces_theme_css_parse_css_block(XfceOverviewWorkspacesThemeCSS *self,
															GScanner *inScanner,
															GList **ioSelectors,
															GList **ioStyles)
{
	GTokenType						token;
	GList							*selectors;
	GList							*iter;
	GHashTable						*styles;
	XfceOverviewWorkspacesThemeCSSSelector		*selector;
	gboolean						doResolveAt;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_THEME_CSS(self), G_TOKEN_ERROR);
	g_return_val_if_fail(inScanner, G_TOKEN_ERROR);
	g_return_val_if_fail(ioSelectors, G_TOKEN_ERROR);
	g_return_val_if_fail(ioStyles, G_TOKEN_ERROR);

	selectors=NULL;
	styles=NULL;

	/* CSS blocks begin with rulesets (list of selectors) - parse them */
	token=_xfce_overview_workspaces_theme_css_parse_css_ruleset(self, inScanner, &selectors);
	if(token!=G_TOKEN_NONE)
	{
		g_list_free_full(selectors, (GDestroyNotify)_xfce_overview_workspaces_theme_css_selector_free);

		return(token);
	}

	/* Determine if we should resolve '@' identifiers or to copy property value only
	 * because a '@' selector is parsed.
	 */
	doResolveAt=TRUE;
	for(iter=selectors; iter; iter=g_list_next(iter))
	{
		selector=(XfceOverviewWorkspacesThemeCSSSelector*)iter->data;
		if(selector->type==XFCE_OVERVIEW_WORKSPACES_THEME_CSS_SELECTOR_TYPE_CONSTANT)
		{
			g_assert(g_list_length(selectors)==1);
			doResolveAt=FALSE;
		}
	}

	/* Create a hash table for the properties */
	styles=g_hash_table_new_full(g_str_hash,
									g_str_equal,
									g_free,
									(GDestroyNotify)g_free);

	token=_xfce_overview_workspaces_theme_css_parse_css_styles(self,
													inScanner,
													*ioSelectors,
													doResolveAt,
													styles);
	if(token!=G_TOKEN_NONE)
	{
		g_list_free_full(selectors, (GDestroyNotify)_xfce_overview_workspaces_theme_css_selector_free);

		g_hash_table_unref(styles);

		return(token);
	}

	/* Assign all the selectors to this style */
	for(iter=selectors; iter; iter=g_list_next(iter))
	{
		selector=(XfceOverviewWorkspacesThemeCSSSelector*)iter->data;
		selector->style=g_hash_table_ref(styles);
	}

	/* Store selectors and styles */
	*ioSelectors=g_list_concat(*ioSelectors, selectors);
	*ioStyles=g_list_append(*ioStyles, styles);

	return(token);
}

static gboolean _xfce_overview_workspaces_theme_css_parse_css(XfceOverviewWorkspacesThemeCSS *self,
													GInputStream *inStream,
													const gchar *inName,
													gint inPriority,
													GList **outSelectors,
													GList **outStyles,
													GError **outError)
{
	XfceOverviewWorkspacesThemeCSSPrivate		*priv;
	GScanner						*scanner;
	GTokenType						token;
	gboolean						success;
	GList							*selectors;
	GList							*styles;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_THEME_CSS(self), FALSE);
	g_return_val_if_fail(G_IS_INPUT_STREAM(inStream), FALSE);
	g_return_val_if_fail(outError==NULL || *outError==NULL, FALSE);
	g_return_val_if_fail(outSelectors && *outSelectors==NULL, FALSE);
	g_return_val_if_fail(outStyles && *outStyles==NULL, FALSE);

	priv=self->priv;
	success=TRUE;
	selectors=NULL;
	styles=NULL;

	/* Create scanner object with default settings */
	scanner=g_scanner_new(NULL);
	scanner->input_name=inName;
	scanner->user_data=GINT_TO_POINTER(inPriority);

	/* Set up scanner config
	 * - Identifiers are allowed to contain '-' (minus sign) as non-first characters
	 * - Disallow scanning float values as we need '.' for identifiers
	 * - Set up single comment line not to include '#' as this character is need for identifiers
	 * - Disable parsing HEX values
	 * - Identifiers cannot be single quoted
	 * - Identifiers cannot be double quoted
	 */
	scanner->config->cset_identifier_nth=G_CSET_a_2_z "-_0123456789" G_CSET_A_2_Z G_CSET_LATINS G_CSET_LATINC;
	scanner->config->scan_float=FALSE;
	scanner->config->cpair_comment_single="\1\n";
	scanner->config->scan_hex=FALSE;
	scanner->config->scan_string_sq=FALSE;
	scanner->config->scan_string_dq=FALSE;

	/* Set input stream */
	if(G_IS_FILE_DESCRIPTOR_BASED(inStream))
	{
		g_scanner_input_file(scanner,
								g_file_descriptor_based_get_fd(G_FILE_DESCRIPTOR_BASED(inStream)));
	}
		else
		{
			/* Set error */
			_xfce_overview_workspaces_theme_css_set_error(self,
												outError,
												XFCE_OVERVIEW_WORKSPACES_THEME_CSS_ERROR_UNSUPPORTED_STREAM,
												"The input stream of type %s is not supported",
												G_OBJECT_TYPE_NAME(inStream));

			/* Destroy scanner */
			g_scanner_destroy(scanner);

			/* Return failure result */
			return(FALSE);
		}

	/* Parse input stream */
	token=g_scanner_peek_next_token(scanner);
	while(token!=G_TOKEN_EOF)
	{
		token=_xfce_overview_workspaces_theme_css_parse_css_block(self, scanner, &selectors, &styles);
		if(token!=G_TOKEN_NONE) break;

		/* Get next token of input stream */
		token=g_scanner_peek_next_token(scanner);
	}

	/* Check that we reached end of stream when we stopped parsing */
	if(token!=G_TOKEN_EOF)
	{
		/* It is not the end of stream so print parser error message
		 * and set error
		 */
		g_scanner_unexp_token(scanner,
								G_TOKEN_EOF,
								NULL,
								NULL,
								NULL,
								"Parser did not reach end of stream",
								TRUE);

		_xfce_overview_workspaces_theme_css_set_error(self,
											outError,
											XFCE_OVERVIEW_WORKSPACES_THEME_CSS_ERROR_PARSER_ERROR,
											"Parser did not reach end of stream");

		success=FALSE;

		/* Free selectors and styles */
		g_list_free_full(selectors, (GDestroyNotify)_xfce_overview_workspaces_theme_css_selector_free);
		selectors=NULL;

		g_list_free_full(styles, (GDestroyNotify)g_hash_table_unref);
		styles=NULL;
	}

	/* Add lines parsed in scanner to line offset */
	priv->offsetLine+=g_scanner_cur_line(scanner)+1;

	/* Return selectors and styles */
	if(outSelectors) *outSelectors=selectors;
	if(outStyles) *outStyles=styles;

	/* Destroy scanner */
	g_scanner_destroy(scanner);

	/* Return success result */
	return(success);
}

/* Callback for sorting selector matches by score */
static gint _xfce_overview_workspaces_theme_css_sort_by_score(XfceOverviewWorkspacesThemeCSSSelectorMatch *inLeft,
													XfceOverviewWorkspacesThemeCSSSelectorMatch *inRight)
{
	gint							priority;
	guint							line;
	guint							position;
	gint							score;
	XfceOverviewWorkspacesCssSelectorRule		*leftRule;
	XfceOverviewWorkspacesCssSelectorRule		*rightRule;

	g_assert(inLeft);
	g_assert(inRight);

	score=inLeft->score-inRight->score;
	if(score!=0) return(score);

	/* Get CSS selector rule to compare them */
	g_assert(inLeft->selector && inLeft->selector->selector);
	g_assert(inRight->selector && inRight->selector->selector);

	leftRule=xfce_overview_workspaces_css_selector_get_rule(inLeft->selector->selector);
	rightRule=xfce_overview_workspaces_css_selector_get_rule(inRight->selector->selector);

	/* Compare CSS selector rules */
	priority=xfce_overview_workspaces_css_selector_rule_get_priority(leftRule);
	priority-=xfce_overview_workspaces_css_selector_rule_get_priority(rightRule);
	if(priority!=0) return(priority);

	line=xfce_overview_workspaces_css_selector_rule_get_line(leftRule);
	line-=xfce_overview_workspaces_css_selector_rule_get_line(rightRule);
	if(line!=0) return(line);

	position=xfce_overview_workspaces_css_selector_rule_get_position(leftRule);
	position-=xfce_overview_workspaces_css_selector_rule_get_position(rightRule);
	if(position!=0) return(position);

	return(0);
}

/* IMPLEMENTATION: GObject */

/* Dispose this object */
static void _xfce_overview_workspaces_theme_css_dispose(GObject *inObject)
{
	XfceOverviewWorkspacesThemeCSS				*self=XFCE_OVERVIEW_WORKSPACES_THEME_CSS(inObject);
	XfceOverviewWorkspacesThemeCSSPrivate		*priv=self->priv;

	/* Release allocated resources */
	if(priv->themePath)
	{
		g_free(priv->themePath);
		priv->themePath=NULL;
	}

	if(priv->selectors)
	{
		g_list_free_full(priv->selectors, (GDestroyNotify)_xfce_overview_workspaces_theme_css_selector_free);
		priv->selectors=NULL;
	}

	if(priv->styles)
	{
		g_list_free_full(priv->styles, (GDestroyNotify)g_hash_table_unref);
		priv->styles=NULL;
	}

	if(priv->names)
	{
		g_slist_free_full(priv->names, (GDestroyNotify)g_free);
		priv->names=NULL;
	}

	if(priv->registeredFunctions)
	{
		g_hash_table_unref(priv->registeredFunctions);
		priv->registeredFunctions=NULL;
	}

	if(priv->loadedFiles)
	{
		g_hash_table_destroy(priv->loadedFiles);
		priv->loadedFiles=NULL;
	}

	if(priv->iconMemo)
	{
		g_hash_table_destroy(priv->iconMemo);
		priv->iconMemo=NULL;
	}

	if(priv->pathExistsMemo)
	{
		g_hash_table_destroy(priv->pathExistsMemo);
		priv->pathExistsMemo=NULL;
	}

	/* Call parent's class dispose method */
	G_OBJECT_CLASS(xfce_overview_workspaces_theme_css_parent_class)->dispose(inObject);
}

/* Set properties */
static void _xfce_overview_workspaces_theme_css_set_property(GObject *inObject,
												guint inPropID,
												const GValue *inValue,
												GParamSpec *inSpec)
{
	XfceOverviewWorkspacesThemeCSS			*self=XFCE_OVERVIEW_WORKSPACES_THEME_CSS(inObject);
	XfceOverviewWorkspacesThemeCSSPrivate	*priv=self->priv;

	switch(inPropID)
	{
		case PROP_THEME_PATH:
			if(priv->themePath)
			{
				g_free(priv->themePath);
				priv->themePath=NULL;
			}

			priv->themePath=g_value_dup_string(inValue);
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
void xfce_overview_workspaces_theme_css_class_init(XfceOverviewWorkspacesThemeCSSClass *klass)
{
	GObjectClass		*gobjectClass=G_OBJECT_CLASS(klass);

	/* Override functions */
	gobjectClass->dispose=_xfce_overview_workspaces_theme_css_dispose;
	gobjectClass->set_property=_xfce_overview_workspaces_theme_css_set_property;

	/* Define properties */
	XfceOverviewWorkspacesThemeCSSProperties[PROP_THEME_PATH]=
		g_param_spec_string("theme-path",
							"Theme path",
							"Path of theme loading from",
							NULL,
							G_PARAM_WRITABLE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY);

	g_object_class_install_properties(gobjectClass, PROP_LAST, XfceOverviewWorkspacesThemeCSSProperties);
}

/* Object initialization
 * Create private structure and set up default values
 */
void xfce_overview_workspaces_theme_css_init(XfceOverviewWorkspacesThemeCSS *self)
{
	XfceOverviewWorkspacesThemeCSSPrivate		*priv;

	priv=self->priv=xfce_overview_workspaces_theme_css_get_instance_private(self);

	/* Set default values */
	priv->themePath=NULL;
	priv->selectors=NULL;
	priv->styles=NULL;
	priv->names=NULL;
	priv->registeredFunctions=NULL;
	priv->offsetLine=0;
	priv->loadedFiles=NULL;
	priv->iconMemo=NULL;
	priv->pathExistsMemo=NULL;

	/* Register CSS functions */
#define REGISTER_CSS_FUNC(name, callback) \
	_xfce_overview_workspaces_theme_css_register_function(self, \
												name, \
												XFCE_OVERVIEW_WORKSPACES_THEME_CSS_FUNCTION_CALLBACK(callback));

	REGISTER_CSS_FUNC("rgb", _xfce_overview_workspaces_theme_css_function_rgb_rgba);
	REGISTER_CSS_FUNC("rgba", _xfce_overview_workspaces_theme_css_function_rgb_rgba);
	REGISTER_CSS_FUNC("mix", _xfce_overview_workspaces_theme_css_function_mix);
	REGISTER_CSS_FUNC("lighter", _xfce_overview_workspaces_theme_css_function_lighter_darker);
	REGISTER_CSS_FUNC("darker", _xfce_overview_workspaces_theme_css_function_lighter_darker);
	REGISTER_CSS_FUNC("shade", _xfce_overview_workspaces_theme_css_function_shade);
	REGISTER_CSS_FUNC("alpha", _xfce_overview_workspaces_theme_css_function_alpha);
	REGISTER_CSS_FUNC("try_icons", _xfce_overview_workspaces_theme_css_function_try_icons)

#undef REGISTER_CSS_FUNC
}


/* IMPLEMENTATION: Errors */

G_DEFINE_QUARK(xfce-overview-workspaces-theme-css-error-quark, xfce_overview_workspaces_theme_css_error);


/* IMPLEMENTATION: Public API */

/**
 * xfce_overview_workspaces_theme_css_new:
 * @inThemePath: The base path of theme
 *
 * Creates a new #XfceOverviewWorkspacesThemeCSS object with base path as provided at
 * @inThemePath. The base path is used to resolve relative paths at CSS
 * functions, e.g. \@try_icons.
 *
 * It is neccessary to call xfce_overview_workspaces_theme_css_add_file() for each CSS
 * file to load before any style can be applied to actors.
 *
 * Return value: An initialized and empty #XfceOverviewWorkspacesThemeCSS
 */
/* Create new instance */
XfceOverviewWorkspacesThemeCSS* xfce_overview_workspaces_theme_css_new(const gchar *inThemePath)
{
	return(XFCE_OVERVIEW_WORKSPACES_THEME_CSS(g_object_new(XFCE_OVERVIEW_WORKSPACES_TYPE_THEME_CSS,
												"theme-path", inThemePath,
												NULL)));
}

/**
 * xfce_overview_workspaces_theme_css_add_file:
 * @self: A #XfceOverviewWorkspacesThemeCSS
 * @inPath: The path to CSS file to load
 * @inPriority: The priority of the entries at this CSS file
 * @outError: A return location for a #GError or %NULL
 *
 * Loads the CSS file at @inPath into theme CSS object at @self with given
 * priority at @inPriority.
 *
 * If loading CSS resource fails, the error message will be placed inside error
 * at @outError (if not %NULL).
 *
 * Return value: %TRUE if CSS file could be loaded or %FALSE if not and error
 *   is stored at @outError.
 */
/* Load a CSS file into theme */
gboolean xfce_overview_workspaces_theme_css_add_file(XfceOverviewWorkspacesThemeCSS *self,
											const gchar *inPath,
											gint inPriority,
											GError **outError)
{
	XfceOverviewWorkspacesThemeCSSPrivate		*priv;
	GFile							*file;
	GFileInputStream				*stream;
	GError							*error;
	GList							*selectors;
	GList							*styles;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_THEME_CSS(self), FALSE);
	g_return_val_if_fail(inPath!=NULL && *inPath!=0, FALSE);
	g_return_val_if_fail(outError==NULL || *outError==NULL, FALSE);

	priv=self->priv;
	selectors=NULL;
	styles=NULL;

	/* Deduplicate: same CSS file listed twice or re-imported via @import
	 * must not be re-opened and re-parsed with GScanner. Priority spacing
	 * is preserved by caller (theme.c keeps incrementing counter), so
	 * skipping here does not change cascade for already-loaded content.
	 */
	if(!priv->loadedFiles)
	{
		priv->loadedFiles=g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
	}
	if(g_hash_table_contains(priv->loadedFiles, inPath))
	{
		XFCE_OVERVIEW_WORKSPACES_DEBUG(self, THEME,
							"Skipping already loaded CSS file '%s' (deduplicated, no re-parse)",
							inPath);
		return(TRUE);
	}

	/* Load and parse CSS file */
	file=g_file_new_for_path(inPath);
	if(!file)
	{
		_xfce_overview_workspaces_theme_css_set_error(self,
											outError,
											XFCE_OVERVIEW_WORKSPACES_THEME_CSS_ERROR_UNSUPPORTED_STREAM,
											"Could not get file for path '%s'",
											inPath);

		return(FALSE);
	}

	error=NULL;
	stream=g_file_read(file, NULL, &error);
	if(error)
	{
		g_propagate_error(outError, error);

		/* Release allocated resources */
		g_object_unref(file);

		return(FALSE);
	}

	_xfce_overview_workspaces_theme_css_parse_css(self,
										G_INPUT_STREAM(stream),
										inPath,
										inPriority,
										&selectors,
										&styles,
										&error);
	if(error)
	{
		g_propagate_error(outError, error);

		/* Release allocated resources */
		if(stream) g_object_unref(stream);
		g_object_unref(file);

		return(FALSE);
	}

	/* If we get here adding, loading and parsing CSS file was successful
	 * so add filename to list of loaded sources, add selectors and styles
	 * to list of selectors and styles in this theme and return TRUE
	 */
	priv->names=g_slist_prepend(priv->names, strdup(inPath));
	g_hash_table_insert(priv->loadedFiles, g_strdup(inPath), GINT_TO_POINTER(1));

	if(selectors)
	{
		priv->selectors=g_list_concat(priv->selectors, selectors);
		XFCE_OVERVIEW_WORKSPACES_DEBUG(self, THEME,
							"Successfully parsed '%s' and added %d selectors - total %d selectors",
							inPath,
							g_list_length(selectors),
							g_list_length(priv->selectors));
	}

	if(styles)
	{
		priv->styles=g_list_concat(priv->styles, styles);
		XFCE_OVERVIEW_WORKSPACES_DEBUG(self, THEME,
							"Successfully parsed '%s' and added %d styles - total %d styles",
							inPath,
							g_list_length(styles),
							g_list_length(priv->styles));
	}

	/* Release allocated resources */
	if(stream) g_object_unref(stream);
	g_object_unref(file);

	return(TRUE);
}

/***
 * xfce_overview_workspaces_theme_css_get_properties:
 * @self: A #XfceOverviewWorkspacesThemeCSS
 * @inStylable: The actor to find matching CSS styles for
 *
 * Collects all properties and their values of all matching selectors for the
 * actor @inStylable from all loaded CSS files at @self. The actor to find the
 * styles for must implement the #XfceOverviewWorkspacesStylable interface.
 *
 * This function will lookup all matching selectors for the actor and collect
 * the properties and value. If multiple properties with same property will be
 * found, the best match (best matching selector, priority etc.) will be kept.
 * The resulting properties will stored in a hash-table. The key of hash-table
 * will contain the property name as string and the value of hash-table is of
 * type #XfceOverviewWorkspacesThemeCSSValue.
 *
 * The caller is responsible to free the returned hash.table with
 * g_hash_table_destroy().
 *
 * Return value: (transfer full): A #GHashtable with all matching properties and
 *   their values or %NULL in case of errors. The returned hash-table must be
 *   destroyed with g_hash_table_destroy() after use.
 */
/* Return properties for a stylable actor */
GHashTable* xfce_overview_workspaces_theme_css_get_properties(XfceOverviewWorkspacesThemeCSS *self,
													XfceOverviewWorkspacesStylable *inStylable)
{
	XfceOverviewWorkspacesThemeCSSPrivate			*priv;
	GList								*entry, *matches;
	XfceOverviewWorkspacesThemeCSSSelectorMatch	*match;
	GHashTable							*result;
#ifdef DEBUG
	GTimer								*timer=NULL;
	const gchar							*styleID;
	const gchar							*styleClasses;
	const gchar							*stylePseudoClasses;
	const gchar							*styleTypeName;
	gchar								*styleSelector;
#endif

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_THEME_CSS(self), NULL);
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_STYLABLE(inStylable), NULL);

	priv=self->priv;
	matches=NULL;
	match=NULL;

#ifdef DEBUG
	styleID=xfce_overview_workspaces_stylable_get_name(inStylable);
	styleClasses=xfce_overview_workspaces_stylable_get_classes(inStylable);
	stylePseudoClasses=xfce_overview_workspaces_stylable_get_pseudo_classes(inStylable);
	styleTypeName=G_OBJECT_TYPE_NAME(inStylable);
	styleSelector=g_strdup_printf("%s%s%s%s%s%s%s",
									(styleTypeName) ? styleTypeName : "",
									(styleClasses) ? "." : "",
									(styleClasses) ? styleClasses : "",
									(styleID) ? "#" : "",
									(styleID) ? styleID : "",
									(stylePseudoClasses) ? ":" : "",
									(stylePseudoClasses) ? stylePseudoClasses : "");
	XFCE_OVERVIEW_WORKSPACES_DEBUG(self, STYLE,
						"Looking up matches for %s ",
						styleSelector);

	timer=g_timer_new();
#endif

	/* Find and collect matching selectors */
	for(entry=priv->selectors; entry; entry=g_list_next(entry))
	{
		gint							score;
		XfceOverviewWorkspacesThemeCSSSelector		*selector;

		selector=(XfceOverviewWorkspacesThemeCSSSelector*)entry->data;
		if(selector->type==XFCE_OVERVIEW_WORKSPACES_THEME_CSS_SELECTOR_TYPE_SELECTOR)
		{
			score=xfce_overview_workspaces_css_selector_score(selector->selector, inStylable);
			if(score>=0)
			{
				match=g_slice_new(XfceOverviewWorkspacesThemeCSSSelectorMatch);
				match->selector=entry->data;
				match->score=score;
				matches=g_list_prepend(matches, match);
			}
		}
	}

	/* Sort matching selectors by their score */
	matches=g_list_sort(matches,
						(GCompareFunc)_xfce_overview_workspaces_theme_css_sort_by_score);

	/* Get properties from matching selectors' styles */
	result=g_hash_table_new_full(g_str_hash,
									g_str_equal,
									NULL,
									(GDestroyNotify)_xfce_overview_workspaces_theme_css_value_free);
	for(entry=matches; entry; entry=g_list_next(entry))
	{
		XfceOverviewWorkspacesThemeCSSTableCopyData	copyData;

		/* Get selector */
		match=(XfceOverviewWorkspacesThemeCSSSelectorMatch*)entry->data;

		/* Copy selector properties to result set */
		copyData.name=xfce_overview_workspaces_css_selector_rule_get_source(xfce_overview_workspaces_css_selector_get_rule(match->selector->selector));
		copyData.table=result;

		g_hash_table_foreach(match->selector->style,
								(GHFunc)_xfce_overview_workspaces_theme_css_copy_table,
								&copyData);
	}

	g_list_free_full(matches, (GDestroyNotify)_xfce_overview_workspaces_themes_css_selector_match_free);

#ifdef DEBUG
	XFCE_OVERVIEW_WORKSPACES_DEBUG(self, STYLE,
						"Found %u properties for %s in %f seconds" ,
						g_hash_table_size(result),
						styleSelector,
						g_timer_elapsed(timer, NULL));
	g_timer_destroy(timer);
	g_free(styleSelector);
#endif

	/* Return found properties */
	return(result);
}
