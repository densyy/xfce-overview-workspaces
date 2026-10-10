/*
 * core: Single-instance core library object managing the library and
 *       single-instance objects like window manager and so on.
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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_CORE__
#define __LIBXFCE_OVERVIEW_WORKSPACES_CORE__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <gio/gio.h>

#include <libxfce-overview-workspaces/theme.h>
#include <libxfce-overview-workspaces/focusable.h>
#include <libxfce-overview-workspaces/stage.h>
#include <libxfce-overview-workspaces/settings.h>
#include <libxfce-overview-workspaces/application-database.h>
#include <libxfce-overview-workspaces/application-tracker.h>
#include <libxfce-overview-workspaces/bindings-pool.h>
#include <libxfce-overview-workspaces/focus-manager.h>
#include <libxfce-overview-workspaces/plugins-manager.h>
#include <libxfce-overview-workspaces/view-manager.h>
#include <libxfce-overview-workspaces/window-tracker-backend.h>
#include <libxfce-overview-workspaces/window-tracker.h>

G_BEGIN_DECLS


/* Object declaration */
#define XFCE_OVERVIEW_WORKSPACES_TYPE_CORE					(xfce_overview_workspaces_core_get_type())
#define XFCE_OVERVIEW_WORKSPACES_CORE(obj)					(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_CORE, XfceOverviewWorkspacesCore))
#define XFCE_OVERVIEW_WORKSPACES_IS_CORE(obj)				(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_CORE))
#define XFCE_OVERVIEW_WORKSPACES_CORE_CLASS(klass)			(G_TYPE_CHECK_CLASS_CAST((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_CORE, XfceOverviewWorkspacesCoreClass))
#define XFCE_OVERVIEW_WORKSPACES_IS_CORE_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_CORE))
#define XFCE_OVERVIEW_WORKSPACES_CORE_GET_CLASS(obj)			(G_TYPE_INSTANCE_GET_CLASS((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_CORE, XfceOverviewWorkspacesCoreClass))

typedef struct _XfceOverviewWorkspacesCore					XfceOverviewWorkspacesCore;
typedef struct _XfceOverviewWorkspacesCoreClass			XfceOverviewWorkspacesCoreClass;
typedef struct _XfceOverviewWorkspacesCorePrivate			XfceOverviewWorkspacesCorePrivate;

/**
 * XfceOverviewWorkspacesCore:
 *
 * The #XfceOverviewWorkspacesCore structure contains only private data and
 * should be accessed using the provided API
 */
struct _XfceOverviewWorkspacesCore
{
	/*< private >*/
	/* Parent instance */
	GObject							parent_instance;

	/* Private structure */
	XfceOverviewWorkspacesCorePrivate			*priv;
};

/**
 * XfceOverviewWorkspacesCoreClass:
 * @initialized: Class handler for the #XfceOverviewWorkspacesCoreClass::initialized signal
 * @suspend: Class handler for the #XfceOverviewWorkspacesCoreClass::suspend signal
 * @resume: Class handler for the #XfceOverviewWorkspacesCoreClass::resume signal
 * @quit: Class handler for the #XfceOverviewWorkspacesCoreClass::quit signal
 * @shutdown_final: Class handler for the #XfceOverviewWorkspacesCoreClass::shutdown_final signal
 * @theme_changed: Class handler for the #XfceOverviewWorkspacesCoreClass::theme_changed signal
 * @application_launched: Class handler for the #XfceOverviewWorkspacesCoreClass::application_launched signal
 * @exit: Class handler for the #XfceOverviewWorkspacesCoreClass::exit signal
 *
 * The #XfceOverviewWorkspacesCoreClass structure contains only private data
 */
struct _XfceOverviewWorkspacesCoreClass
{
	/*< private >*/
	/* Parent class */
	GObjectClass					parent_class;

	/*< public >*/
	/* Virtual functions */
	void (*initialized)(XfceOverviewWorkspacesCore *self);

	void (*quit)(XfceOverviewWorkspacesCore *self);
	void (*shutdown)(XfceOverviewWorkspacesCore *self);

	gboolean (*can_suspend)(XfceOverviewWorkspacesCore *self);
	void (*suspend)(XfceOverviewWorkspacesCore *self);
	void (*resume)(XfceOverviewWorkspacesCore *self);

	void (*theme_loading)(XfceOverviewWorkspacesCore *self, XfceOverviewWorkspacesTheme *inTheme);
	void (*theme_loaded)(XfceOverviewWorkspacesCore *self, XfceOverviewWorkspacesTheme *inTheme);
	void (*theme_changed)(XfceOverviewWorkspacesCore *self, XfceOverviewWorkspacesTheme *inTheme);

	void (*application_launched)(XfceOverviewWorkspacesCore *self, GAppInfo *inAppInfo);

	/* Binding actions */
	gboolean (*exit)(XfceOverviewWorkspacesCore *self,
						XfceOverviewWorkspacesFocusable *inSource,
						const gchar *inAction,
						ClutterEvent *inEvent);
};


/* Errors */
/**
 * XFCE_OVERVIEW_WORKSPACES_CORE_ERROR:
 *
 * Error domain for core errors.
 * Errors in this domain will be from the #XfceOverviewWorkspacesCoreError enumeration.
 */
#define XFCE_OVERVIEW_WORKSPACES_CORE_ERROR				(xfce_overview_workspaces_core_error_quark())

GQuark xfce_overview_workspaces_core_error_quark(void);

/**
 * XfceOverviewWorkspacesCoreError:
 * @XFCE_OVERVIEW_WORKSPACES_CORE_ERROR_INITIALIZATION_FAILED: Initialization of core
 *   object failed and cannot be used
 *
 * Error codes returned by core.
 */
typedef enum /*< prefix=XFCE_OVERVIEW_WORKSPACES_CORE_ERROR >*/
{
	XFCE_OVERVIEW_WORKSPACES_CORE_ERROR_INITIALIZATION_FAILED,
} XfceOverviewWorkspacesCoreError;


/* Public API */
GType xfce_overview_workspaces_core_get_type(void) G_GNUC_CONST;

gboolean xfce_overview_workspaces_core_initialize(XfceOverviewWorkspacesCore *self, GError **outError);

gboolean xfce_overview_workspaces_core_has_default(void);
XfceOverviewWorkspacesCore* xfce_overview_workspaces_core_get_default(void);

/* Quit functions */
gboolean xfce_overview_workspaces_core_is_quitting(XfceOverviewWorkspacesCore *self);
void xfce_overview_workspaces_core_quit(XfceOverviewWorkspacesCore *self);

/* Suspend/resume functions */
gboolean xfce_overview_workspaces_core_can_suspend(XfceOverviewWorkspacesCore *self);
gboolean xfce_overview_workspaces_core_is_suspended(XfceOverviewWorkspacesCore *self);
void xfce_overview_workspaces_core_suspend(XfceOverviewWorkspacesCore *self);
void xfce_overview_workspaces_core_resume(XfceOverviewWorkspacesCore *self);

/* Access function for components and subsystems */
XfceOverviewWorkspacesSettings* xfce_overview_workspaces_core_get_settings(XfceOverviewWorkspacesCore *self);
XfceOverviewWorkspacesStage* xfce_overview_workspaces_core_get_stage(XfceOverviewWorkspacesCore *self);
XfceOverviewWorkspacesTheme* xfce_overview_workspaces_core_get_theme(XfceOverviewWorkspacesCore *self);

XfceOverviewWorkspacesApplicationDatabase* xfce_overview_workspaces_core_get_application_database(XfceOverviewWorkspacesCore *self);
XfceOverviewWorkspacesApplicationTracker* xfce_overview_workspaces_core_get_application_tracker(XfceOverviewWorkspacesCore *self);
XfceOverviewWorkspacesBindingsPool* xfce_overview_workspaces_core_get_bindings_pool(XfceOverviewWorkspacesCore *self);
XfceOverviewWorkspacesFocusManager* xfce_overview_workspaces_core_get_focus_manager(XfceOverviewWorkspacesCore *self);
XfceOverviewWorkspacesPluginsManager* xfce_overview_workspaces_core_get_plugins_manager(XfceOverviewWorkspacesCore *self);
XfceOverviewWorkspacesViewManager* xfce_overview_workspaces_core_get_view_manager(XfceOverviewWorkspacesCore *self);
XfceOverviewWorkspacesWindowTracker* xfce_overview_workspaces_core_get_window_tracker(XfceOverviewWorkspacesCore *self);
XfceOverviewWorkspacesWindowTrackerBackend* xfce_overview_workspaces_core_get_window_tracker_backend(XfceOverviewWorkspacesCore *self);

G_END_DECLS

#endif	/* __LIBXFCE_OVERVIEW_WORKSPACES_CORE__ */
