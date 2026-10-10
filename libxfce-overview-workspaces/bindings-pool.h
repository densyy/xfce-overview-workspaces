/*
 * bindings: Customizable keyboard and pointer bindings for focusable actors
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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_BINDINGS_POOL__
#define __LIBXFCE_OVERVIEW_WORKSPACES_BINDINGS_POOL__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <clutter/clutter.h>

#include <libxfce-overview-workspaces/binding.h>
#include <libxfce-overview-workspaces/types.h>

G_BEGIN_DECLS

#define XFCE_OVERVIEW_WORKSPACES_TYPE_BINDINGS_POOL				(xfce_overview_workspaces_bindings_pool_get_type())
#define XFCE_OVERVIEW_WORKSPACES_BINDINGS_POOL(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_BINDINGS_POOL, XfceOverviewWorkspacesBindingsPool))
#define XFCE_OVERVIEW_WORKSPACES_IS_BINDINGS_POOL(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_BINDINGS_POOL))
#define XFCE_OVERVIEW_WORKSPACES_BINDINGS_POOL_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_BINDINGS_POOL, XfceOverviewWorkspacesBindingsPoolClass))
#define XFCE_OVERVIEW_WORKSPACES_IS_BINDINGS_POOL_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_BINDINGS_POOL))
#define XFCE_OVERVIEW_WORKSPACES_BINDINGS_POOL_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_BINDINGS_POOL, XfceOverviewWorkspacesBindingsPoolClass))

typedef struct _XfceOverviewWorkspacesBindingsPool				XfceOverviewWorkspacesBindingsPool;
typedef struct _XfceOverviewWorkspacesBindingsPoolClass		XfceOverviewWorkspacesBindingsPoolClass;
typedef struct _XfceOverviewWorkspacesBindingsPoolPrivate		XfceOverviewWorkspacesBindingsPoolPrivate;

struct _XfceOverviewWorkspacesBindingsPool
{
	/*< private >*/
	/* Parent instance */
	GObject							parent_instance;

	/* Private structure */
	XfceOverviewWorkspacesBindingsPoolPrivate		*priv;
};

struct _XfceOverviewWorkspacesBindingsPoolClass
{
	/*< private >*/
	/* Parent class */
	GObjectClass					parent_class;
};

/* Errors */
#define XFCE_OVERVIEW_WORKSPACES_BINDINGS_POOL_ERROR					(xfce_overview_workspaces_bindings_pool_error_quark())

GQuark xfce_overview_workspaces_bindings_pool_error_quark(void);

typedef enum /*< prefix=XFCE_OVERVIEW_WORKSPACES_BINDINGS_POOL_ERROR >*/
{
	XFCE_OVERVIEW_WORKSPACES_BINDINGS_POOL_ERROR_FILE_NOT_FOUND,
	XFCE_OVERVIEW_WORKSPACES_BINDINGS_POOL_ERROR_PARSER_INTERNAL_ERROR,
	XFCE_OVERVIEW_WORKSPACES_BINDINGS_POOL_ERROR_MALFORMED,
	XFCE_OVERVIEW_WORKSPACES_BINDINGS_POOL_ERROR_INTERNAL_ERROR
} XfceOverviewWorkspacesBindingsPoolErrorEnum;

/* Public API */
GType xfce_overview_workspaces_bindings_pool_get_type(void) G_GNUC_CONST;

gboolean xfce_overview_workspaces_bindings_pool_load(XfceOverviewWorkspacesBindingsPool *self, GError **outError);

const XfceOverviewWorkspacesBinding* xfce_overview_workspaces_bindings_pool_find_for_event(XfceOverviewWorkspacesBindingsPool *self, ClutterActor *inActor, const ClutterEvent *inEvent);

G_END_DECLS

#endif	/* __LIBXFCE_OVERVIEW_WORKSPACES_BINDINGS_POOL__ */
