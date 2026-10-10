/*
 * theme: Top-level theme object (parses key file and manages loading
 *        resources like css style files, xml layout files etc.)
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
 * SECTION:theme
 * @short_description: A theme
 * @include: libxfce-overview-workspaces/theme.h
 *
 * #XfceOverviewWorkspacesTheme is used to load a named theme searched at the theme
 * search path as returned by xfce_overview_workspaces_settings_get_theme_search_paths()
 * and loads the key file, parses it to retrieve the file locations of all
 * resources to load for the theme like CSS, animations, effects, layout,
 * etc.
 *
 * # File location and structure {#XfceOverviewWorkspacesTheme.File-location-and-structure}
 *
 * `xfce-overview-workspaces` will look up the theme at the paths in the order as returned
 * by xfce_overview_workspaces_settings_get_theme_search_paths(). In case of the application
 * `xfce-overview-workspaces` this is the following order:
 *
 *   * (if environment variable is set) ${XFCE_OVERVIEW_WORKSPACES_THEME_PATH}/
 *   * ${XDG_DATA_HOME}/themes/THEME/xfce-overview-workspaces-1.0/
 *   * ${HOME}/.themes/THEME/xfce-overview-workspaces-1.0/
 *   * ${SYSTEM-WIDE-DATA}/themes/THEME/xfce-overview-workspaces-1.0/
 *
 * `xfce-overview-workspaces` expects at least the theme index file `xfce-overview-workspaces.theme` at
 * the theme's path which contains information about the theme and the resources
 * to load for styling and layout. The content of the theme file should be
 * similar to the following:
 *
 * |[
 *   [XfceOverviewWorkspaces Theme]
 *   Name=<Display name of theme>
 *   Comment=<A description for the theme>
 *   Author=<A list of authors of the theme seperated by semicolon>
 *   Version=<The version of the theme>
 *   Style=<A list of CSS files for styling separated by semicolon>
 *   Layout=<A list of XML files for layout separated by semicolon>
 *   Effects=<A list of XML files for effects separated by semicolon>
 *   Animation=<A lilst of of XML files for animations seperated by semicolon>
 *   Screenshot=<A list of paths to screenshot images separated by semicolon>
 * ]|
 *
 * The following keys are required: `Name`, `Comment`, `Style` and `Layout`. All
 * other keys are optional.
 *
 * The key `Style` contains the file name of one CSS file or a semicolon-separated
 * list of CSS files (without spaces in the list) which describe how actors should
 * be styled. The files should be located in the theme path or in a sub-folder.
 * See section <link linkend="XfceOverviewWorkspacesThemeCSS.Styling">Styling</link> for
 * further documentation about styling. <!-- EXTERNAL: There is also a full list
 * about all [#Elements and stylables properties], [#classes] and [#pseudo-classes].
 * -->
 *
 * The key `Layout` contains the file name of one XML file or a semicolon-separated
 * list of XML files (without spaces in the list) which describes how the different
 * stages are to be built. The files should be located in the theme path or in a
 * sub-folder. See section <link linkend="XfceOverviewWorkspacesThemeLayout.Layout">Layout</link>
 * for further documentation about layouting.
 *
 * The key `Effects` contains the file name of one XML file or a semicolon-separated
 * list of XML files (without spaces in the list) which describes effects which can
 * be used in actors and how they are built. The files should be located in the
 * theme path or in a sub-folder. See section<link linkend="XfceOverviewWorkspacesThemeEffects.Effects">
 * Effects</link> for further documentation about creating and using effects.
 *
 * The key `Animation` contains the file name of one XML file or a semicolon-separated
 * list of XML files (without spaces in the list) which describes animations which can
 * be used in actors and how they are built. The files should be located in the
 * theme path or in a sub-folder. See section <link linkend="XfceOverviewWorkspacesThemeAnimation.Animations">
 * Animations</link> for further documentation about creating and using animations.
 *
 * The key `Screenshot` contains the file name of one image or a semicolon-separated
 * list of images (without spaces in the list) which can be used as preview images
 * for the theme, e.g. in settings application. The files should be located in the
 * theme path or in a sub-folder and given as relative paths.
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <libxfce-overview-workspaces/theme.h>

#include <glib/gi18n-lib.h>
#include <glib.h>
#include <gio/gio.h>

#include <libxfce-overview-workspaces/core.h>
#include <libxfce-overview-workspaces/settings.h>
#include <libxfce-overview-workspaces/debug.h>


/* Define this class in GObject system */
struct _XfceOverviewWorkspacesThemePrivate
{
	/* Properties related */
	gchar						*themePath;

	gchar						*themeName;
	gchar						*themeDisplayName;
	gchar						*themeComment;

	/* Instance related */
	gboolean					loaded;

	XfceOverviewWorkspacesThemeCSS			*styling;
	XfceOverviewWorkspacesThemeLayout		*layout;
	XfceOverviewWorkspacesThemeEffects		*effects;
	XfceOverviewWorkspacesThemeAnimation	*animation;

	gchar						*userThemeStyleFile;
	gchar						*userGlobalStyleFile;

	/* Deferred (non-first-frame) resources: effects/animations are
	 * parsed after first frame to cut startup I/O. Lists are owned
	 * copies of theme-relative entries stashed during initial load.
	 */
	gchar						**pendingEffectsFiles;
	gchar						**pendingAnimationFiles;
	guint						deferredIdleId;
	gboolean					deferredLoaded;
};

G_DEFINE_TYPE_WITH_PRIVATE(XfceOverviewWorkspacesTheme,
							xfce_overview_workspaces_theme,
							G_TYPE_OBJECT)

/* Properties */
enum
{
	PROP_0,

	PROP_PATH,

	PROP_NAME,
	PROP_DISPLAY_NAME,
	PROP_COMMENT,

	PROP_LAST
};

static GParamSpec* XfceOverviewWorkspacesThemeProperties[PROP_LAST]={ 0, };

/* IMPLEMENTATION: Private variables and methods */
#define XFCE_OVERVIEW_WORKSPACES_THEME_SUBPATH						"xfce-overview-workspaces-1.0"
#define XFCE_OVERVIEW_WORKSPACES_THEME_FILE							"xfce-overview-workspaces.theme"
#define XFCE_OVERVIEW_WORKSPACES_USER_GLOBAL_CSS_FILE				"global.css"

#define XFCE_OVERVIEW_WORKSPACES_THEME_GROUP							"XfceOverviewWorkspaces Theme"
#define XFCE_OVERVIEW_WORKSPACES_THEME_GROUP_KEY_NAME				"Name"
#define XFCE_OVERVIEW_WORKSPACES_THEME_GROUP_KEY_COMMENT				"Comment"
#define XFCE_OVERVIEW_WORKSPACES_THEME_GROUP_KEY_STYLE				"Style"
#define XFCE_OVERVIEW_WORKSPACES_THEME_GROUP_KEY_LAYOUT				"Layout"
#define XFCE_OVERVIEW_WORKSPACES_THEME_GROUP_KEY_EFFECTS				"Effects"
#define XFCE_OVERVIEW_WORKSPACES_THEME_GROUP_KEY_ANIMATIONS			"Animations"

/* Memoized theme-name -> theme-path lookups. Avoids re-scanning all
 * search paths (2×g_file_test per path) when theme object is recreated
 * for an already-seen name. Process-lifetime cache; entries are
 * short strings.
 */
static GHashTable				*_xfce_overview_workspaces_theme_path_cache=NULL;

/* Forward declarations for deferred loading */
static void _xfce_overview_workspaces_theme_ensure_deferred_loaded(XfceOverviewWorkspacesTheme *self);
static gboolean _xfce_overview_workspaces_theme_load_deferred_idle(gpointer inUserData);

/* Unified loader signature for a single theme resource file. The priority
 * is only used by CSS resources; other loaders ignore it but the shared
 * counter is still increased to keep original ordering and priority semantics.
 */
typedef gboolean (*XfceOverviewWorkspacesThemeResourceLoader)(gpointer inInstance,
													const gchar *inResourceFile,
													gint inPriority,
													GError **outError);

static gboolean _xfce_overview_workspaces_theme_load_css_resource(gpointer inInstance,
														const gchar *inResourceFile,
														gint inPriority,
														GError **outError)
{
	return(xfce_overview_workspaces_theme_css_add_file(XFCE_OVERVIEW_WORKSPACES_THEME_CSS(inInstance), inResourceFile, inPriority, outError));
}

static gboolean _xfce_overview_workspaces_theme_load_layout_resource(gpointer inInstance,
														const gchar *inResourceFile,
														gint inPriority,
														GError **outError)
{
	(void)inPriority;
	return(xfce_overview_workspaces_theme_layout_add_file(XFCE_OVERVIEW_WORKSPACES_THEME_LAYOUT(inInstance), inResourceFile, outError));
}

static gboolean _xfce_overview_workspaces_theme_load_effects_resource(gpointer inInstance,
															const gchar *inResourceFile,
															gint inPriority,
															GError **outError)
{
	(void)inPriority;
	return(xfce_overview_workspaces_theme_effects_add_file(XFCE_OVERVIEW_WORKSPACES_THEME_EFFECTS(inInstance), inResourceFile, outError));
}

static gboolean _xfce_overview_workspaces_theme_load_animation_resource(gpointer inInstance,
															const gchar *inResourceFile,
															gint inPriority,
															GError **outError)
{
	(void)inPriority;
	return(xfce_overview_workspaces_theme_animation_add_file(XFCE_OVERVIEW_WORKSPACES_THEME_ANIMATION(inInstance), inResourceFile, outError));
}

typedef struct _XfceOverviewWorkspacesThemeResourceEntry		XfceOverviewWorkspacesThemeResourceEntry;
struct _XfceOverviewWorkspacesThemeResourceEntry
{
	const gchar							*key;
	gboolean							optional;
	XfceOverviewWorkspacesThemeResourceLoader		loader;
	gpointer							instance;
	const gchar							*debugFormat;
	gboolean							usePriority;
};

/* Load a single NULL-terminated list of resource names for a theme.
 * Builds each full path from the theme path, logs with the given debug
 * format (which must contain "%s for theme %s" and additionally "%d" for
 * priority when inUsePriority is TRUE), calls the unified loader and
 * increases the shared counter (used as CSS priority) on each success.
 * Returns TRUE on success or FALSE with outError set on first failure.
 */
static gboolean _load_single_list(XfceOverviewWorkspacesTheme *self,
									const gchar **inList,
									XfceOverviewWorkspacesThemeResourceLoader inLoader,
									gpointer inLoaderInstance,
									const gchar *inDebugFormat,
									gboolean inUsePriority,
									gint *ioCounter,
									GHashTable *inSeen,
									GError **outError)
{
	XfceOverviewWorkspacesThemePrivate		*priv;
	const gchar					**iter;
	GError						*error;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_THEME(self), FALSE);
	g_return_val_if_fail(inList!=NULL, FALSE);
	g_return_val_if_fail(inLoader!=NULL, FALSE);
	g_return_val_if_fail(inDebugFormat!=NULL, FALSE);
	g_return_val_if_fail(ioCounter!=NULL, FALSE);
	g_return_val_if_fail(outError==NULL || *outError==NULL, FALSE);

	priv=self->priv;
	error=NULL;

	for(iter=inList; iter && *iter; iter++)
	{
		gchar					*resourceFile;
		gchar					*debugMessage;
		gboolean				success;

		/* Get path and file for resource */
		resourceFile=g_build_filename(priv->themePath, *iter, NULL);

		/* Deduplicate: same absolute resource listed twice (e.g. in
		 * Style list and via user CSS, or duplicated entries) must not
		 * be re-read and re-parsed. Keep counter increment to preserve
		 * CSS priority spacing for later files, so visual result is
		 * unchanged while I/O + GScanner/GMarkup work is cut.
		 */
		if(inSeen && g_hash_table_contains(inSeen, resourceFile))
		{
			XFCE_OVERVIEW_WORKSPACES_DEBUG(self, THEME,
								"Skipping already loaded resource %s for theme %s (deduplicated)",
								resourceFile,
								priv->themeName);
			g_free(resourceFile);
			(*ioCounter)++;
			continue;
		}

		/* Try to load resource (log first with identical messages) */
		if(inUsePriority) debugMessage=g_strdup_printf(inDebugFormat, resourceFile, priv->themeName, *ioCounter);
			else debugMessage=g_strdup_printf(inDebugFormat, resourceFile, priv->themeName);
		XFCE_OVERVIEW_WORKSPACES_DEBUG(self, THEME, "%s", debugMessage);
		g_free(debugMessage);

		success=inLoader(inLoaderInstance, resourceFile, *ioCounter, &error);
		if(inSeen && success) g_hash_table_insert(inSeen, g_strdup(resourceFile), GINT_TO_POINTER(1));
		g_free(resourceFile);

		if(!success)
		{
			/* Set error */
			g_propagate_error(outError, error);

			/* Return FALSE to indicate error */
			return(FALSE);
		}

		/* Continue with next entry */
		(*ioCounter)++;
	}

	/* Return TRUE to indicate success */
	return(TRUE);
}


/* Load theme file and all listed resources in this file */
static gboolean _xfce_overview_workspaces_theme_load_resources(XfceOverviewWorkspacesTheme *self,
													GError **outError)
{
	XfceOverviewWorkspacesThemePrivate		*priv;
	GError						*error;
	gchar						*themeFile;
	GKeyFile					*themeKeyFile;
	gchar						**resources;
	gint						counter;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_THEME(self), FALSE);
	g_return_val_if_fail(outError==NULL || *outError==NULL, FALSE);

	priv=self->priv;
	error=NULL;

	/* Check that theme was found */
	if(!priv->themePath)
	{
		/* Set error */
		g_set_error(outError,
					XFCE_OVERVIEW_WORKSPACES_THEME_ERROR,
					XFCE_OVERVIEW_WORKSPACES_THEME_ERROR_THEME_NOT_FOUND,
					"Theme '%s' not found",
					priv->themeName);

		/* Return FALSE to indicate error */
		return(FALSE);
	}

	/* Load theme file */
	themeFile=g_build_filename(priv->themePath, XFCE_OVERVIEW_WORKSPACES_THEME_FILE, NULL);

	themeKeyFile=g_key_file_new();
	if(!g_key_file_load_from_file(themeKeyFile,
									themeFile,
									G_KEY_FILE_NONE,
									&error))
	{
		/* Set error */
		g_propagate_error(outError, error);

		/* Release allocated resources */
		if(themeFile) g_free(themeFile);
		if(themeKeyFile) g_key_file_free(themeKeyFile);

		/* Return FALSE to indicate error */
		return(FALSE);
	}

	g_free(themeFile);

	/* Get display name and notify about property change (regardless of success result) */
	priv->themeDisplayName=g_key_file_get_locale_string(themeKeyFile,
														XFCE_OVERVIEW_WORKSPACES_THEME_GROUP,
														XFCE_OVERVIEW_WORKSPACES_THEME_GROUP_KEY_NAME,
														NULL,
														&error);
	g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesThemeProperties[PROP_DISPLAY_NAME]);

	if(!priv->themeDisplayName)
	{
		/* Set error */
		g_propagate_error(outError, error);

		/* Release allocated resources */
		if(themeKeyFile) g_key_file_free(themeKeyFile);

		/* Return FALSE to indicate error */
		return(FALSE);
	}

	/* Get comment and notify about property change (regardless of success result) */
	priv->themeComment=g_key_file_get_locale_string(themeKeyFile,
														XFCE_OVERVIEW_WORKSPACES_THEME_GROUP,
														XFCE_OVERVIEW_WORKSPACES_THEME_GROUP_KEY_COMMENT,
														NULL,
														&error);
	g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesThemeProperties[PROP_COMMENT]);

	if(!priv->themeComment)
	{
		/* Set error */
		g_propagate_error(outError, error);

		/* Release allocated resources */
		if(themeKeyFile) g_key_file_free(themeKeyFile);

		/* Return FALSE to indicate error */
		return(FALSE);
	}

	/* Load style and layout synchronously (needed for first frame).
	 * Effects and animations are deferred until after show (idle) and
	 * also loaded on-demand via getters, so startup I/O (open+read) and
	 * GMarkup parses for non-first-frame resources are cut.
	 * Style and layout are required, effects and animations are optional.
	 * The shared counter is used as CSS priority. User stylesheets
	 * ("global.css" and "user-[THEME_NAME].css" in this order) are loaded
	 * right after the style list to allow user overrides.
	 */
	counter=0;
	{
		XfceOverviewWorkspacesThemeResourceEntry		resourceEntries[]=
			{
				{ XFCE_OVERVIEW_WORKSPACES_THEME_GROUP_KEY_STYLE, FALSE, _xfce_overview_workspaces_theme_load_css_resource, NULL, "Loading CSS file %s for theme %s with priority %d", TRUE },
				{ XFCE_OVERVIEW_WORKSPACES_THEME_GROUP_KEY_LAYOUT, FALSE, _xfce_overview_workspaces_theme_load_layout_resource, NULL, "Loading XML layout file %s for theme %s", FALSE },
				{ XFCE_OVERVIEW_WORKSPACES_THEME_GROUP_KEY_EFFECTS, TRUE, _xfce_overview_workspaces_theme_load_effects_resource, NULL, "Loading XML effects file %s for theme %s", FALSE },
				{ XFCE_OVERVIEW_WORKSPACES_THEME_GROUP_KEY_ANIMATIONS, TRUE, _xfce_overview_workspaces_theme_load_animation_resource, NULL, "Loading XML animation file %s for theme %s", FALSE },
			};
		guint								entryIndex;
		GHashTable							*seen;

		resourceEntries[0].instance=priv->styling;
		resourceEntries[1].instance=priv->layout;
		resourceEntries[2].instance=priv->effects;
		resourceEntries[3].instance=priv->animation;

		/* Deduplication set for absolute resource paths within this load.
		 * Prevents re-read + re-parse when theme lists the same file twice.
		 */
		seen=g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);

		for(entryIndex=0; entryIndex<G_N_ELEMENTS(resourceEntries); entryIndex++)
		{
			XfceOverviewWorkspacesThemeResourceEntry		*entry;

			entry=&resourceEntries[entryIndex];

			/* Skip optional lists when key is missing */
			if(entry->optional &&
				!g_key_file_has_key(themeKeyFile,
									XFCE_OVERVIEW_WORKSPACES_THEME_GROUP,
									entry->key,
									NULL))
			{
				continue;
			}

			resources=g_key_file_get_string_list(themeKeyFile,
													XFCE_OVERVIEW_WORKSPACES_THEME_GROUP,
													entry->key,
													NULL,
													&error);
			if(!resources)
			{
				/* Set error */
				g_propagate_error(outError, error);

				/* Release allocated resources */
				g_hash_table_destroy(seen);
				if(themeKeyFile) g_key_file_free(themeKeyFile);

				/* Return FALSE to indicate error */
				return(FALSE);
			}

			/* Defer effects (index 2) and animations (index 3): stash
			 * theme-relative entries and skip I/O + GMarkup parse now.
			 * They will be loaded in idle after first frame or on first
			 * use via xfce_overview_workspaces_theme_get_effects/animation().
			 */
			if(entryIndex>=2)
			{
				gchar			**stashed;
				gsize			len;
				guint			i;

				len=g_strv_length(resources);
				stashed=g_new0(gchar*, len+1);
				for(i=0; i<len; i++) stashed[i]=g_strdup(resources[i]);

				if(entryIndex==2)
				{
					g_strfreev(priv->pendingEffectsFiles);
					priv->pendingEffectsFiles=stashed;
					XFCE_OVERVIEW_WORKSPACES_DEBUG(self, THEME,
										"Deferring %u effects file(s) for theme %s until after first frame",
										(guint)len,
										priv->themeName);
				}
					else
					{
						g_strfreev(priv->pendingAnimationFiles);
						priv->pendingAnimationFiles=stashed;
						XFCE_OVERVIEW_WORKSPACES_DEBUG(self, THEME,
											"Deferring %u animation file(s) for theme %s until after first frame",
											(guint)len,
											priv->themeName);
					}

				g_strfreev(resources);
				resources=NULL;
				continue;
			}

			if(!_load_single_list(self,
									(const gchar**)resources,
									entry->loader,
									entry->instance,
									entry->debugFormat,
									entry->usePriority,
									&counter,
									seen,
									&error))
			{
				/* Set error */
				g_propagate_error(outError, error);

				/* Release allocated resources */
				g_hash_table_destroy(seen);
				if(resources) g_strfreev(resources);
				if(themeKeyFile) g_key_file_free(themeKeyFile);

				/* Return FALSE to indicate error */
				return(FALSE);
			}
			g_strfreev(resources);
			resources=NULL;

			/* After style list, load user stylesheets (theme unrelated
			 * "global.css" and theme related "user-[THEME_NAME].css" in this
			 * order) at last to allow user to override theme styles.
			 */
			if(entryIndex==0)
			{
				if(priv->userGlobalStyleFile)
				{
					XFCE_OVERVIEW_WORKSPACES_DEBUG(self, THEME,
											"Loading user's global CSS file %s for theme %s with priority %d",
											priv->userGlobalStyleFile,
											priv->themeName,
											counter);

					/* Load user's theme unrelated (global) stylesheet as it exists */
					if(!g_hash_table_contains(seen, priv->userGlobalStyleFile))
					{
						if(!xfce_overview_workspaces_theme_css_add_file(priv->styling, priv->userGlobalStyleFile, counter, &error))
						{
							/* Set error */
							g_propagate_error(outError, error);

							/* Release allocated resources */
							g_hash_table_destroy(seen);
							if(themeKeyFile) g_key_file_free(themeKeyFile);

							/* Return FALSE to indicate error */
							return(FALSE);
						}
						g_hash_table_insert(seen, g_strdup(priv->userGlobalStyleFile), GINT_TO_POINTER(1));
					}
						else
						{
							XFCE_OVERVIEW_WORKSPACES_DEBUG(self, THEME,
												"Skipping already loaded user global CSS %s",
												priv->userGlobalStyleFile);
						}

					/* Increase counter used for CSS priority for next user CSS file */
					counter++;
				}

				if(priv->userThemeStyleFile)
				{
					XFCE_OVERVIEW_WORKSPACES_DEBUG(self, THEME,
											"Loading user's theme CSS file %s for theme %s with priority %d",
											priv->userThemeStyleFile,
											priv->themeName,
											counter);

					/* Load user's theme related stylesheet as it exists */
					if(!g_hash_table_contains(seen, priv->userThemeStyleFile))
					{
						if(!xfce_overview_workspaces_theme_css_add_file(priv->styling, priv->userThemeStyleFile, counter, &error))
						{
							/* Set error */
							g_propagate_error(outError, error);

							/* Release allocated resources */
							g_hash_table_destroy(seen);
							if(themeKeyFile) g_key_file_free(themeKeyFile);

							/* Return FALSE to indicate error */
							return(FALSE);
						}
						g_hash_table_insert(seen, g_strdup(priv->userThemeStyleFile), GINT_TO_POINTER(1));
					}
						else
						{
							XFCE_OVERVIEW_WORKSPACES_DEBUG(self, THEME,
												"Skipping already loaded user theme CSS %s",
												priv->userThemeStyleFile);
						}

					/* Increase counter used for CSS priority for next user CSS file */
					counter++;
				}
			}
		}
		g_hash_table_destroy(seen);
		resources=NULL;
	}

	/* Release allocated resources */
	if(themeKeyFile) g_key_file_free(themeKeyFile);

	/* Return TRUE to indicate success */
	return(TRUE);
}

/* Load deferred effects/animations synchronously. Called from idle after
 * first frame and on-demand from getters. Safe to call repeatedly; second
 * and later calls are no-ops. Failures are logged but do not fail theme
 * load (resources were optional in original synchronous path as well when
 * key was missing; per-file failures here mirror original error text).
 */
static void _xfce_overview_workspaces_theme_ensure_deferred_loaded(XfceOverviewWorkspacesTheme *self)
{
	XfceOverviewWorkspacesThemePrivate		*priv;
	GError						*error;
	gint						dummyCounter;
	GHashTable					*seen;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_THEME(self));

	priv=self->priv;
	if(priv->deferredLoaded) return;
	if(!priv->loaded) return;

	/* If nothing was deferred, just mark done */
	if(!priv->pendingEffectsFiles && !priv->pendingAnimationFiles)
	{
		priv->deferredLoaded=TRUE;
		return;
	}

	error=NULL;
	dummyCounter=0;
	seen=g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);

	if(priv->pendingEffectsFiles)
	{
		if(!_load_single_list(self,
								(const gchar**)priv->pendingEffectsFiles,
								_xfce_overview_workspaces_theme_load_effects_resource,
								priv->effects,
								"Loading XML effects file %s for theme %s",
								FALSE,
								&dummyCounter,
								seen,
								&error))
		{
			g_critical("Could not load deferred effects for theme '%s': %s",
						priv->themeName,
						(error && error->message) ? error->message : "unknown error");
			if(error) g_error_free(error);
			error=NULL;
		}
		g_strfreev(priv->pendingEffectsFiles);
		priv->pendingEffectsFiles=NULL;
	}

	if(priv->pendingAnimationFiles)
	{
		if(!_load_single_list(self,
								(const gchar**)priv->pendingAnimationFiles,
								_xfce_overview_workspaces_theme_load_animation_resource,
								priv->animation,
								"Loading XML animation file %s for theme %s",
								FALSE,
								&dummyCounter,
								seen,
								&error))
		{
			g_critical("Could not load deferred animations for theme '%s': %s",
						priv->themeName,
						(error && error->message) ? error->message : "unknown error");
			if(error) g_error_free(error);
			error=NULL;
		}
		g_strfreev(priv->pendingAnimationFiles);
		priv->pendingAnimationFiles=NULL;
	}

	g_hash_table_destroy(seen);
	priv->deferredLoaded=TRUE;
}

static gboolean _xfce_overview_workspaces_theme_load_deferred_idle(gpointer inUserData)
{
	XfceOverviewWorkspacesTheme			*self=XFCE_OVERVIEW_WORKSPACES_THEME(inUserData);

	/* No extra reference is held (theme is owned by core for app lifetime;
	 * dispose cancels this idle). Clear ID before loading so a re-entrant
	 * getter does not try to remove this already-dispatching source.
	 */
	self->priv->deferredIdleId=0;
	_xfce_overview_workspaces_theme_ensure_deferred_loaded(self);

	return(G_SOURCE_REMOVE);
}

/* Lookup path for named theme.
 * Caller must free returned path with g_free if not needed anymore.
 */
static gchar* _xfce_overview_workspaces_theme_lookup_path_for_theme(XfceOverviewWorkspacesTheme *self,
														const gchar *inThemeName)
{
	XfceOverviewWorkspacesSettings		*settings;
	const gchar				**searchPaths;
	gchar					*themeFile;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_THEME(self), FALSE);
	g_return_val_if_fail(inThemeName!=NULL && *inThemeName!=0, FALSE);

	themeFile=NULL;

	/* Memoized lookup: avoid re-scanning search paths (up to 2 stats per
	 * path) when the same theme name is requested again in this process
	 * (e.g. theme switch back-and-forth or second window). Cache stores
	 * theme-name -> theme-path strings.
	 */
	if(_xfce_overview_workspaces_theme_path_cache)
	{
		const gchar		*cachedPath;

		cachedPath=(const gchar*)g_hash_table_lookup(_xfce_overview_workspaces_theme_path_cache, inThemeName);
		if(cachedPath)
		{
			XFCE_OVERVIEW_WORKSPACES_DEBUG(self, THEME,
								"Theme '%s' path cache hit: %s (skipped search-path stats)",
								inThemeName,
								cachedPath);
			return(g_strdup(cachedPath));
		}
	}

	/* Get search path for themes */
	settings=xfce_overview_workspaces_core_get_settings(NULL);
	searchPaths=xfce_overview_workspaces_settings_get_theme_search_paths(settings);

	/* Iterate through search paths and look up theme */
	while(searchPaths && *searchPaths && !themeFile)
	{
		/* Restore old behaviour to force a theme path via an environment
		 * variable but this time with theme search paths. The behaviour
		 * was to take the provided path and add the theme file without
		 * any additional subpath like theme name or theme subpath folder.
		 * If this built path provides the theme file use it directly
		 * although it might not match the theme name (that is for sure
		 * as the theme name will not be added to the path built). This
		 * behaviour makes development easier to test themes without
		 * changing theme by settings or changing symlinks in any of
		 * theme searched paths.
		 */
		themeFile=g_build_filename(*searchPaths, XFCE_OVERVIEW_WORKSPACES_THEME_FILE, NULL);
		XFCE_OVERVIEW_WORKSPACES_DEBUG(self, THEME,
							"Trying theme file: %s",
							themeFile);
		if(!g_file_test(themeFile, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_REGULAR))
		{
			/* File does not exists so release allocated resources */
			g_free(themeFile);
			themeFile=NULL;

			/* Next build theme file path depending on theme name and
			 * required sub-folders and test for existence.
			 * If it does exist, keep theme file path. If it does not exist,
			 * unset theme file path and continue iterating.
			 */
			themeFile=g_build_filename(*searchPaths, inThemeName, XFCE_OVERVIEW_WORKSPACES_THEME_SUBPATH, XFCE_OVERVIEW_WORKSPACES_THEME_FILE, NULL);
			XFCE_OVERVIEW_WORKSPACES_DEBUG(self, THEME,
								"Trying theme file: %s",
								themeFile);
			if(!g_file_test(themeFile, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_REGULAR))
			{
				g_free(themeFile);
				themeFile=NULL;
			}
		}

		/* Move iterator to next entry */
		searchPaths++;
	}
	
	/* If file was found, get path contaning file and return it */
	if(themeFile)
	{
		gchar			*themePath;
		gchar			*cachePath;

		themePath=g_path_get_dirname(themeFile);
		g_free(themeFile);

		/* Remember successful lookup to skip future search-path stats */
		if(!_xfce_overview_workspaces_theme_path_cache)
		{
			_xfce_overview_workspaces_theme_path_cache=g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
		}
		cachePath=(gchar*)g_hash_table_lookup(_xfce_overview_workspaces_theme_path_cache, inThemeName);
		if(!cachePath)
		{
			g_hash_table_insert(_xfce_overview_workspaces_theme_path_cache,
								g_strdup(inThemeName),
								g_strdup(themePath));
		}

		return(themePath);
	}

	/* If we get here theme was not found so return NULL */
	return(NULL);
}

/* Theme's name was set so lookup pathes and initialize but do not load resources */
static void _xfce_overview_workspaces_theme_set_theme_name(XfceOverviewWorkspacesTheme *self, const gchar *inThemeName)
{
	XfceOverviewWorkspacesThemePrivate		*priv;
	gchar						*themePath;
	gchar						*resourceFile;
	gchar						*userThemeStylesheet;
	XfceOverviewWorkspacesSettings			*settings;
	const gchar					*configPath;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_THEME(self));
	g_return_if_fail(inThemeName && *inThemeName);

	priv=self->priv;

	/* The theme name must not be set already */
	if(priv->themeName)
	{
		/* Show error message */
		g_critical("Cannot change theme name to '%s' because it is already set to '%s'",
					inThemeName,
					priv->themeName);

		return;
	}

	/* Lookup path of theme by lookup at all possible paths for theme file */
	themePath=_xfce_overview_workspaces_theme_lookup_path_for_theme(self, inThemeName);
	if(!themePath)
	{
		g_critical("Theme '%s' not found", inThemeName);

		/* Return here because looking up path failed */
		return;
	}

	/* Set up internal variable and notify about property changes */
	priv->themeName=g_strdup(inThemeName);
	g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesThemeProperties[PROP_NAME]);

	priv->themePath=g_strdup(themePath);
	g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesThemeProperties[PROP_PATH]);

	/* Initialize theme resources */
	priv->styling=xfce_overview_workspaces_theme_css_new(priv->themePath);
	priv->layout=xfce_overview_workspaces_theme_layout_new();
	priv->effects=xfce_overview_workspaces_theme_effects_new();
	priv->animation=xfce_overview_workspaces_theme_animation_new();

	/* Check for user resource files */
	settings=xfce_overview_workspaces_core_get_settings(NULL);
	configPath=xfce_overview_workspaces_settings_get_config_path(settings);
	if(configPath)
	{
		resourceFile=g_build_filename(configPath, "themes", XFCE_OVERVIEW_WORKSPACES_USER_GLOBAL_CSS_FILE, NULL);
		if(g_file_test(resourceFile, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_REGULAR))
		{
			priv->userGlobalStyleFile=g_strdup(resourceFile);
		}
			else
			{
				XFCE_OVERVIEW_WORKSPACES_DEBUG(self, THEME,
									"No user global stylesheet found at %s for theme %s - skipping",
									resourceFile,
									priv->themeName);
			}
		g_free(resourceFile);

		userThemeStylesheet=g_strdup_printf("user-%s.css", priv->themeName);
		resourceFile=g_build_filename(configPath, "themes", userThemeStylesheet, NULL);
		if(g_file_test(resourceFile, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_REGULAR))
		{
			priv->userThemeStyleFile=g_strdup(resourceFile);
		}
			else
			{
				XFCE_OVERVIEW_WORKSPACES_DEBUG(self, THEME,
									"No user theme stylesheet found at %s for theme %s - skipping",
									resourceFile,
									priv->themeName);
			}
		g_free(resourceFile);
	}

	/* Release allocated resources */
	if(themePath) g_free(themePath);
}

/* IMPLEMENTATION: GObject */

/* Dispose this object */
static void _xfce_overview_workspaces_theme_dispose(GObject *inObject)
{
	XfceOverviewWorkspacesTheme			*self=XFCE_OVERVIEW_WORKSPACES_THEME(inObject);
	XfceOverviewWorkspacesThemePrivate		*priv=self->priv;

	/* Cancel pending deferred load */
	if(priv->deferredIdleId)
	{
		g_source_remove(priv->deferredIdleId);
		priv->deferredIdleId=0;
	}

	if(priv->pendingEffectsFiles)
	{
		g_strfreev(priv->pendingEffectsFiles);
		priv->pendingEffectsFiles=NULL;
	}

	if(priv->pendingAnimationFiles)
	{
		g_strfreev(priv->pendingAnimationFiles);
		priv->pendingAnimationFiles=NULL;
	}

	/* Release allocated resources */
	if(priv->themeName)
	{
		g_free(priv->themeName);
		priv->themeName=NULL;
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesThemeProperties[PROP_NAME]);
	}

	if(priv->themePath)
	{
		g_free(priv->themePath);
		priv->themePath=NULL;
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesThemeProperties[PROP_PATH]);
	}

	if(priv->themeDisplayName)
	{
		g_free(priv->themeDisplayName);
		priv->themeDisplayName=NULL;
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesThemeProperties[PROP_DISPLAY_NAME]);
	}

	if(priv->themeComment)
	{
		g_free(priv->themeComment);
		priv->themeComment=NULL;
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesThemeProperties[PROP_COMMENT]);
	}

	if(priv->userThemeStyleFile)
	{
		g_free(priv->userThemeStyleFile);
		priv->userThemeStyleFile=NULL;
	}

	if(priv->userGlobalStyleFile)
	{
		g_free(priv->userGlobalStyleFile);
		priv->userGlobalStyleFile=NULL;
	}

	if(priv->styling)
	{
		g_object_unref(priv->styling);
		priv->styling=NULL;
	}

	if(priv->layout)
	{
		g_object_unref(priv->layout);
		priv->layout=NULL;
	}

	if(priv->effects)
	{
		g_object_unref(priv->effects);
		priv->effects=NULL;
	}

	if(priv->animation)
	{
		g_object_unref(priv->animation);
		priv->animation=NULL;
	}

	/* Call parent's class dispose method */
	G_OBJECT_CLASS(xfce_overview_workspaces_theme_parent_class)->dispose(inObject);
}

/* Set/get properties */
static void _xfce_overview_workspaces_theme_set_property(GObject *inObject,
											guint inPropID,
											const GValue *inValue,
											GParamSpec *inSpec)
{
	XfceOverviewWorkspacesTheme			*self=XFCE_OVERVIEW_WORKSPACES_THEME(inObject);

	switch(inPropID)
	{
		case PROP_NAME:
			_xfce_overview_workspaces_theme_set_theme_name(self, g_value_get_string(inValue));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(inObject, inPropID, inSpec);
			break;
	}
}

static void _xfce_overview_workspaces_theme_get_property(GObject *inObject,
											guint inPropID,
											GValue *outValue,
											GParamSpec *inSpec)
{
	XfceOverviewWorkspacesTheme			*self=XFCE_OVERVIEW_WORKSPACES_THEME(inObject);
	XfceOverviewWorkspacesThemePrivate		*priv=self->priv;

	switch(inPropID)
	{
		case PROP_PATH:
			g_value_set_string(outValue, priv->themePath);
			break;

		case PROP_NAME:
			g_value_set_string(outValue, priv->themeName);
			break;

		case PROP_DISPLAY_NAME:
			g_value_set_string(outValue, priv->themeDisplayName);
			break;

		case PROP_COMMENT:
			g_value_set_string(outValue, priv->themeComment);
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
void xfce_overview_workspaces_theme_class_init(XfceOverviewWorkspacesThemeClass *klass)
{
	GObjectClass		*gobjectClass=G_OBJECT_CLASS(klass);

	/* Override functions */
	gobjectClass->dispose=_xfce_overview_workspaces_theme_dispose;
	gobjectClass->set_property=_xfce_overview_workspaces_theme_set_property;
	gobjectClass->get_property=_xfce_overview_workspaces_theme_get_property;

	/* Define properties */
	XfceOverviewWorkspacesThemeProperties[PROP_NAME]=
		g_param_spec_string("theme-name",
							"Theme name",
							"Short name of theme which was used to lookup theme and folder name where theme is stored in",
							NULL,
							G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY);

	XfceOverviewWorkspacesThemeProperties[PROP_PATH]=
		g_param_spec_string("theme-path",
							"Theme path",
							"Path where theme was found and loaded from",
							NULL,
							G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

	XfceOverviewWorkspacesThemeProperties[PROP_DISPLAY_NAME]=
		g_param_spec_string("theme-display-name",
							"Theme display name",
							"The name of theme",
							NULL,
							G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

	XfceOverviewWorkspacesThemeProperties[PROP_COMMENT]=
		g_param_spec_string("theme-comment",
							"Theme comment",
							"The comment of theme used as description",
							NULL,
							G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(gobjectClass, PROP_LAST, XfceOverviewWorkspacesThemeProperties);
}

/* Object initialization
 * Create private structure and set up default values
 */
void xfce_overview_workspaces_theme_init(XfceOverviewWorkspacesTheme *self)
{
	XfceOverviewWorkspacesThemePrivate		*priv;

	priv=self->priv=xfce_overview_workspaces_theme_get_instance_private(self);

	/* Set default values */
	priv->loaded=FALSE;
	priv->themeName=NULL;
	priv->themePath=NULL;
	priv->themeDisplayName=NULL;
	priv->themeComment=NULL;
	priv->userThemeStyleFile=NULL;
	priv->userGlobalStyleFile=NULL;
	priv->styling=NULL;
	priv->layout=NULL;
	priv->effects=NULL;
	priv->animation=NULL;
	priv->pendingEffectsFiles=NULL;
	priv->pendingAnimationFiles=NULL;
	priv->deferredIdleId=0;
	priv->deferredLoaded=FALSE;
}

/* IMPLEMENTATION: Errors */

G_DEFINE_QUARK(xfce-overview-workspaces-theme-error-quark, xfce_overview_workspaces_theme_error);


/* IMPLEMENTATION: Public API */

/**
 * xfce_overview_workspaces_theme_new:
 * @inThemeName: The name of theme
 *
 * Creates a new #XfceOverviewWorkspacesTheme object and initializes the object instance.
 * It will not load any resources of the theme. It is neccessary to call
 * xfce_overview_workspaces_theme_load() to load its resources.
 *
 * Return value: An initialized #XfceOverviewWorkspacesTheme
 */
XfceOverviewWorkspacesTheme* xfce_overview_workspaces_theme_new(const gchar *inThemeName)
{
	return(XFCE_OVERVIEW_WORKSPACES_THEME(g_object_new(XFCE_OVERVIEW_WORKSPACES_TYPE_THEME,
											"theme-name", inThemeName,
											NULL)));
}

/**
 * xfce_overview_workspaces_theme_get_path:
 * @self: A #XfceOverviewWorkspacesTheme
 *
 * Returns the base path where the theme at @self was found and will load all
 * its resources from.
 *
 * Return value: The theme's base path
 */
const gchar* xfce_overview_workspaces_theme_get_path(XfceOverviewWorkspacesTheme *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_THEME(self), NULL);

	return(self->priv->themePath);
}

/**
 * xfce_overview_workspaces_theme_get_theme_name:
 * @self: A #XfceOverviewWorkspacesTheme
 *
 * Returns the name of theme at @self.
 *
 * Return value: The theme's name
 */
const gchar* xfce_overview_workspaces_theme_get_theme_name(XfceOverviewWorkspacesTheme *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_THEME(self), NULL);

	return(self->priv->themeName);
}

/**
 * xfce_overview_workspaces_theme_get_display_name:
 * @self: A #XfceOverviewWorkspacesTheme
 *
 * Returns the display name of theme at @self.
 *
 * Return value: The theme's name to display
 */
const gchar* xfce_overview_workspaces_theme_get_display_name(XfceOverviewWorkspacesTheme *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_THEME(self), NULL);

	return(self->priv->themeDisplayName);
}

/**
 * xfce_overview_workspaces_theme_get_comment:
 * @self: A #XfceOverviewWorkspacesTheme
 *
 * Returns the comment of theme at @self.
 *
 * Return value: The theme's comment
 */
const gchar* xfce_overview_workspaces_theme_get_comment(XfceOverviewWorkspacesTheme *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_THEME(self), NULL);

	return(self->priv->themeComment);
}

/**
 * xfce_overview_workspaces_theme_load:
 * @self: A #XfceOverviewWorkspacesTheme
 * @outError: A return location for a #GError or %NULL
 *
 * Lookups named theme at @self and loads all its resources like CSS, layout,
 * animation etc.
 *
 * If loading theme and its resources fails, the error message will be placed
 * inside error at @outError (if not %NULL).
 *
 * Return value: %TRUE if instance of #XfceOverviewWorkspacesTheme could be loaded fully
 *   or %FALSE if not and error is stored at @outError.
 */
gboolean xfce_overview_workspaces_theme_load(XfceOverviewWorkspacesTheme *self,
								GError **outError)
{
	XfceOverviewWorkspacesThemePrivate		*priv;
	GError						*error;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_THEME(self), FALSE);
	g_return_val_if_fail(outError==NULL || *outError==NULL, FALSE);

	priv=self->priv;
	error=NULL;

	/* Check if a theme was already loaded */
	if(priv->loaded)
	{
		g_set_error(outError,
					XFCE_OVERVIEW_WORKSPACES_THEME_ERROR,
					XFCE_OVERVIEW_WORKSPACES_THEME_ERROR_ALREADY_LOADED,
					"Theme '%s' was already loaded",
					priv->themeName);

		return(FALSE);
	}

	/* We set the loaded flag regardless if loading will be successful or not
	 * because if loading theme fails this object is in undefined state for
	 * re-using it to load theme again.
	 */
	priv->loaded=TRUE;

	/* Load theme key file */
	if(!_xfce_overview_workspaces_theme_load_resources(self, &error))
	{
		/* Set returned error */
		g_propagate_error(outError, error);

		/* Return FALSE to indicate error */
		return(FALSE);
	}

	/* Schedule deferred load of effects/animations after first frame.
	 * Getters also ensure synchronous load on first use, so behavior is
	 * identical but startup I/O + parses no longer block initial show.
	 * No extra ref: theme is owned by core; dispose cancels idle.
	 */
	if(priv->pendingEffectsFiles || priv->pendingAnimationFiles)
	{
		priv->deferredLoaded=FALSE;
		priv->deferredIdleId=g_idle_add_full(G_PRIORITY_LOW,
												_xfce_overview_workspaces_theme_load_deferred_idle,
												self,
												NULL);
	}
		else priv->deferredLoaded=TRUE;

	/* If we found named themed and could load all resources successfully */
	return(TRUE);
}

/**
 * xfce_overview_workspaces_theme_get_css:
 * @self: A #XfceOverviewWorkspacesTheme
 *
 * Returns the CSS resources of type #XfceOverviewWorkspacesThemeCSS of theme at @self.
 *
 * Return value: The theme's CSS resources of type #XfceOverviewWorkspacesThemeCSS
 */
XfceOverviewWorkspacesThemeCSS* xfce_overview_workspaces_theme_get_css(XfceOverviewWorkspacesTheme *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_THEME(self), NULL);

	return(self->priv->styling);
}

/**
 * xfce_overview_workspaces_theme_get_layout:
 * @self: A #XfceOverviewWorkspacesTheme
 *
 * Returns the layout resources of type #XfceOverviewWorkspacesThemeLayout of theme at @self.
 *
 * Return value: The theme's layout resources of type #XfceOverviewWorkspacesThemeLayout
 */
XfceOverviewWorkspacesThemeLayout* xfce_overview_workspaces_theme_get_layout(XfceOverviewWorkspacesTheme *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_THEME(self), NULL);

	return(self->priv->layout);
}

/**
 * xfce_overview_workspaces_theme_get_effects:
 * @self: A #XfceOverviewWorkspacesTheme
 *
 * Returns the effect resources of type #XfceOverviewWorkspacesThemeEffects of theme at @self.
 *
 * Return value: The theme's effect resources of type #XfceOverviewWorkspacesThemeEffects
 */
XfceOverviewWorkspacesThemeEffects* xfce_overview_workspaces_theme_get_effects(XfceOverviewWorkspacesTheme *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_THEME(self), NULL);

	/* On-demand load if idle has not run yet (e.g. actor sets effects
	 * before first frame). After that this is a no-op.
	 */
	_xfce_overview_workspaces_theme_ensure_deferred_loaded(self);

	/* If idle is still pending and we just loaded synchronously, cancel it */
	if(self->priv->deferredLoaded && self->priv->deferredIdleId)
	{
		g_source_remove(self->priv->deferredIdleId);
		self->priv->deferredIdleId=0;
	}

	return(self->priv->effects);
}

/**
 * xfce_overview_workspaces_theme_get_animation:
 * @self: A #XfceOverviewWorkspacesTheme
 *
 * Returns the animation resources of type #XfceOverviewWorkspacesThemeAnimation of theme at @self.
 *
 * Return value: The theme's animation resources of type #XfceOverviewWorkspacesThemeAnimation
 */
XfceOverviewWorkspacesThemeAnimation* xfce_overview_workspaces_theme_get_animation(XfceOverviewWorkspacesTheme *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_THEME(self), NULL);

	/* On-demand load if idle has not run yet. After that no-op. */
	_xfce_overview_workspaces_theme_ensure_deferred_loaded(self);

	if(self->priv->deferredLoaded && self->priv->deferredIdleId)
	{
		g_source_remove(self->priv->deferredIdleId);
		self->priv->deferredIdleId=0;
	}

	return(self->priv->animation);
}
