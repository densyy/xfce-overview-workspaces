/*
 * application: Single-instance managing application and single-instance
 *              objects like window manager and so on.
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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_APPLICATION__
#define __LIBXFCE_OVERVIEW_WORKSPACES_APPLICATION__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <gio/gio.h>

#include <libxfce-overview-workspaces/focusable.h>

G_BEGIN_DECLS

/* Public definitions */
/**
 * XfceOverviewWorkspacesApplicationErrorCode:
 * @XFCE_OVERVIEW_WORKSPACES_APPLICATION_ERROR_NONE: Application started successfully without any problems
 * @XFCE_OVERVIEW_WORKSPACES_APPLICATION_ERROR_FAILED: Application failed to start
 * @XFCE_OVERVIEW_WORKSPACES_APPLICATION_ERROR_RESTART: Application needs to be restarted to start-up successfully
 * @XFCE_OVERVIEW_WORKSPACES_APPLICATION_ERROR_QUIT: Application was quitted and shuts down
 *
 * The start-up status codes returned by XfceOverviewWorkspacesApplication.
 */
typedef enum /*< skip,prefix=XFCE_OVERVIEW_WORKSPACES_APPLICATION_ERROR >*/
{
	XFCE_OVERVIEW_WORKSPACES_APPLICATION_ERROR_NONE=0,
	XFCE_OVERVIEW_WORKSPACES_APPLICATION_ERROR_FAILED,
	XFCE_OVERVIEW_WORKSPACES_APPLICATION_ERROR_RESTART,
	XFCE_OVERVIEW_WORKSPACES_APPLICATION_ERROR_QUIT
} XfceOverviewWorkspacesApplicationErrorCode;


/* Object declaration */
#define XFCE_OVERVIEW_WORKSPACES_TYPE_APPLICATION				(xfce_overview_workspaces_application_get_type())
#define XFCE_OVERVIEW_WORKSPACES_APPLICATION(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_APPLICATION, XfceOverviewWorkspacesApplication))
#define XFCE_OVERVIEW_WORKSPACES_IS_APPLICATION(obj)				(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_APPLICATION))
#define XFCE_OVERVIEW_WORKSPACES_APPLICATION_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_APPLICATION, XfceOverviewWorkspacesApplicationClass))
#define XFCE_OVERVIEW_WORKSPACES_IS_APPLICATION_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_APPLICATION))
#define XFCE_OVERVIEW_WORKSPACES_APPLICATION_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_APPLICATION, XfceOverviewWorkspacesApplicationClass))

typedef struct _XfceOverviewWorkspacesApplication				XfceOverviewWorkspacesApplication;
typedef struct _XfceOverviewWorkspacesApplicationClass			XfceOverviewWorkspacesApplicationClass;
typedef struct _XfceOverviewWorkspacesApplicationPrivate		XfceOverviewWorkspacesApplicationPrivate;

/**
 * XfceOverviewWorkspacesApplication:
 *
 * The #XfceOverviewWorkspacesApplication structure contains only private data and
 * should be accessed using the provided API
 */
struct _XfceOverviewWorkspacesApplication
{
	/*< private >*/
	/* Parent instance */
	GApplication					parent_instance;

	/* Private structure */
	XfceOverviewWorkspacesApplicationPrivate	*priv;
};

/**
 * XfceOverviewWorkspacesApplicationClass:
 * @initialized: Class handler for the #XfceOverviewWorkspacesApplicationClass::initialized signal
 * @suspend: Class handler for the #XfceOverviewWorkspacesApplicationClass::suspend signal
 * @resume: Class handler for the #XfceOverviewWorkspacesApplicationClass::resume signal
 * @quit: Class handler for the #XfceOverviewWorkspacesApplicationClass::quit signal
 * @shutdown_final: Class handler for the #XfceOverviewWorkspacesApplicationClass::shutdown_final signal
 * @theme_changed: Class handler for the #XfceOverviewWorkspacesApplicationClass::theme_changed signal
 * @application_launched: Class handler for the #XfceOverviewWorkspacesApplicationClass::application_launched signal
 * @exit: Class handler for the #XfceOverviewWorkspacesApplicationClass::exit signal
 *
 * The #XfceOverviewWorkspacesApplicationClass structure contains only private data
 */
struct _XfceOverviewWorkspacesApplicationClass
{
	/*< private >*/
	/* Parent class */
	GApplicationClass				parent_class;

	/*< public >*/
	/* Binding actions */
	gboolean (*exit)(XfceOverviewWorkspacesApplication *self,
						XfceOverviewWorkspacesFocusable *inSource,
						const gchar *inAction,
						ClutterEvent *inEvent);
};


/* Public API */
GType xfce_overview_workspaces_application_get_type(void) G_GNUC_CONST;

XfceOverviewWorkspacesApplication* xfce_overview_workspaces_application_new(void);

gboolean xfce_overview_workspaces_application_is_daemonized(XfceOverviewWorkspacesApplication *self);

void xfce_overview_workspaces_application_quit_forced(XfceOverviewWorkspacesApplication *self);

G_END_DECLS

#endif	/* __LIBXFCE_OVERVIEW_WORKSPACES_APPLICATION__ */
