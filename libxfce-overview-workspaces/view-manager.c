/*
 * view-manager: Single-instance managing views
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <libxfce-overview-workspaces/view-manager.h>

#include <glib/gi18n-lib.h>

#include <libxfce-overview-workspaces/view.h>
#include <libxfce-overview-workspaces/utils.h>
#include <libxfce-overview-workspaces/debug.h>


/* Define this class in GObject system */
struct _XfceOverviewWorkspacesViewManagerPrivate
{
	/* Instance related */
	GList		*registeredViews;
};

G_DEFINE_TYPE_WITH_PRIVATE(XfceOverviewWorkspacesViewManager,
							xfce_overview_workspaces_view_manager,
							G_TYPE_OBJECT)

/* Signals */
enum
{
	SIGNAL_REGISTERED,
	SIGNAL_UNREGISTERED,

	SIGNAL_LAST
};

static guint XfceOverviewWorkspacesViewManagerSignals[SIGNAL_LAST]={ 0, };

/* IMPLEMENTATION: Private variables and methods */
typedef struct _XfceOverviewWorkspacesViewManagerData		XfceOverviewWorkspacesViewManagerData;
struct _XfceOverviewWorkspacesViewManagerData
{
	gchar		*ID;
	GType		gtype;
};

/* Free an registered view entry */
static void _xfce_overview_workspaces_view_manager_entry_free(XfceOverviewWorkspacesViewManagerData *inData)
{
	g_return_if_fail(inData);

	/* Release allocated resources */
	if(inData->ID) g_free(inData->ID);
	g_free(inData);
}

/* Create an entry for a registered view */
static XfceOverviewWorkspacesViewManagerData* _xfce_overview_workspaces_view_manager_entry_new(const gchar *inID, GType inType)
{
	XfceOverviewWorkspacesViewManagerData		*data;

	g_return_val_if_fail(inID && *inID, NULL);

	/* Create new entry */
	data=g_new0(XfceOverviewWorkspacesViewManagerData, 1);
	if(!data) return(NULL);

	data->ID=g_strdup(inID);
	data->gtype=inType;

	/* Return newly created entry */
	return(data);
}

/* Find entry for a registered view by ID */
static GList* _xfce_overview_workspaces_view_manager_entry_find_list_entry_by_id(XfceOverviewWorkspacesViewManager *self,
																	const gchar *inID)
{
	XfceOverviewWorkspacesViewManagerPrivate	*priv;
	GList							*iter;
	XfceOverviewWorkspacesViewManagerData		*data;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_VIEW_MANAGER(self), NULL);
	g_return_val_if_fail(inID && *inID, NULL);

	priv=self->priv;

	/* Iterate through list and lookup list entry whose data has requested ID */
	for(iter=priv->registeredViews; iter; iter=g_list_next(iter))
	{
		/* Get data of currently iterated list entry */
		data=(XfceOverviewWorkspacesViewManagerData*)(iter->data);
		if(!data) continue;

		/* Check if ID of data matches requested one and
		 * return list entry if it does.
		 */
		if(g_strcmp0(data->ID, inID)==0) return(iter);
	}

	/* If we get here we did not find a matching list entry */
	return(NULL);
}

static XfceOverviewWorkspacesViewManagerData* _xfce_overview_workspaces_view_manager_entry_find_data_by_id(XfceOverviewWorkspacesViewManager *self,
																					const gchar *inID)
{
	GList							*iter;
	XfceOverviewWorkspacesViewManagerData		*data;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_VIEW_MANAGER(self), NULL);
	g_return_val_if_fail(inID && *inID, NULL);

	/* Find list entry matching requested ID */
	iter=_xfce_overview_workspaces_view_manager_entry_find_list_entry_by_id(self, inID);
	if(!iter) return(NULL);

	/* We found a matching list entry so return its data */
	data=(XfceOverviewWorkspacesViewManagerData*)(iter->data);

	/* Return data of matching list entry */
	return(data);
}

/* IMPLEMENTATION: GObject */

/* Dispose this object */
static void _xfce_overview_workspaces_view_manager_dispose_unregister_view(gpointer inData, gpointer inUserData)
{
	XfceOverviewWorkspacesViewManagerData		*data;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_VIEW_MANAGER(inUserData));

	data=(XfceOverviewWorkspacesViewManagerData*)inData;
	xfce_overview_workspaces_view_manager_unregister(XFCE_OVERVIEW_WORKSPACES_VIEW_MANAGER(inUserData), data->ID);
}

static void _xfce_overview_workspaces_view_manager_dispose(GObject *inObject)
{
	XfceOverviewWorkspacesViewManager			*self=XFCE_OVERVIEW_WORKSPACES_VIEW_MANAGER(inObject);
	XfceOverviewWorkspacesViewManagerPrivate	*priv=self->priv;

	/* Release allocated resouces */
	if(priv->registeredViews)
	{
		g_list_foreach(priv->registeredViews, _xfce_overview_workspaces_view_manager_dispose_unregister_view, self);
		g_list_free(priv->registeredViews);
		priv->registeredViews=NULL;
	}

	/* Call parent's class dispose method */
	G_OBJECT_CLASS(xfce_overview_workspaces_view_manager_parent_class)->dispose(inObject);
}

/* Class initialization
 * Override functions in parent classes and define properties
 * and signals
 */
static void xfce_overview_workspaces_view_manager_class_init(XfceOverviewWorkspacesViewManagerClass *klass)
{
	GObjectClass		*gobjectClass=G_OBJECT_CLASS(klass);

	/* Override functions */
	gobjectClass->dispose=_xfce_overview_workspaces_view_manager_dispose;

	/* Define signals */
	XfceOverviewWorkspacesViewManagerSignals[SIGNAL_REGISTERED]=
		g_signal_new("registered",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST,
						G_STRUCT_OFFSET(XfceOverviewWorkspacesViewManagerClass, registered),
						NULL,
						NULL,
						g_cclosure_marshal_VOID__OBJECT,
						G_TYPE_NONE,
						1,
						G_TYPE_GTYPE);

	XfceOverviewWorkspacesViewManagerSignals[SIGNAL_UNREGISTERED]=
		g_signal_new("unregistered",
						G_TYPE_FROM_CLASS(klass),
						G_SIGNAL_RUN_LAST,
						G_STRUCT_OFFSET(XfceOverviewWorkspacesViewManagerClass, unregistered),
						NULL,
						NULL,
						g_cclosure_marshal_VOID__OBJECT,
						G_TYPE_NONE,
						1,
						G_TYPE_GTYPE);
}

/* Object initialization
 * Create private structure and set up default values
 */
static void xfce_overview_workspaces_view_manager_init(XfceOverviewWorkspacesViewManager *self)
{
	XfceOverviewWorkspacesViewManagerPrivate	*priv;

	priv=self->priv=xfce_overview_workspaces_view_manager_get_instance_private(self);

	/* Set default values */
	priv->registeredViews=NULL;
}

/* IMPLEMENTATION: Public API */

/* Register a view */
gboolean xfce_overview_workspaces_view_manager_register(XfceOverviewWorkspacesViewManager *self, const gchar *inID, GType inViewType)
{
	XfceOverviewWorkspacesViewManagerPrivate		*priv;
	XfceOverviewWorkspacesViewManagerData			*data;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_VIEW_MANAGER(self), FALSE);
	g_return_val_if_fail(inID && *inID, FALSE);

	priv=self->priv;

	/* Check if given type is not a XfceOverviewWorkspacesView but a derived type from it */
	if(inViewType==XFCE_OVERVIEW_WORKSPACES_TYPE_VIEW ||
		g_type_is_a(inViewType, XFCE_OVERVIEW_WORKSPACES_TYPE_VIEW)!=TRUE)
	{
		g_warning("View %s of type %s is not a %s and cannot be registered",
					inID,
					g_type_name(inViewType),
					g_type_name(XFCE_OVERVIEW_WORKSPACES_TYPE_VIEW));
		return(FALSE);
	}

	/* Check if view is registered already */
	if(_xfce_overview_workspaces_view_manager_entry_find_list_entry_by_id(self, inID))
	{
		g_warning("View %s of type %s is registered already",
					inID,
					g_type_name(inViewType));
		return(FALSE);
	}

	/* Register view */
	XFCE_OVERVIEW_WORKSPACES_DEBUG(self, MISC,
						"Registering view %s of type %s",
						inID,
						g_type_name(inViewType));

	data=_xfce_overview_workspaces_view_manager_entry_new(inID, inViewType);
	if(!data)
	{
		g_warning("Failed to register view %s of type %s",
					inID,
					g_type_name(inViewType));
		return(FALSE);
	}

	priv->registeredViews=g_list_append(priv->registeredViews, data);
	g_signal_emit(self, XfceOverviewWorkspacesViewManagerSignals[SIGNAL_REGISTERED], 0, data->ID);

	/* View was registered successfully so return TRUE here */
	return(TRUE);
}

/* Unregister a view */
gboolean xfce_overview_workspaces_view_manager_unregister(XfceOverviewWorkspacesViewManager *self, const gchar *inID)
{
	XfceOverviewWorkspacesViewManagerPrivate		*priv;
	GList								*iter;
	XfceOverviewWorkspacesViewManagerData			*data;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_VIEW_MANAGER(self), FALSE);
	g_return_val_if_fail(inID && *inID, FALSE);

	priv=self->priv;

	/* Check if view is registered  */
	iter=_xfce_overview_workspaces_view_manager_entry_find_list_entry_by_id(self, inID);
	if(!iter)
	{
		g_warning("View %s is not registered and cannot be unregistered", inID);
		return(FALSE);
	}

	/* Get data from found list entry */
	data=(XfceOverviewWorkspacesViewManagerData*)(iter->data);

	/* Remove from list of registered views */
	XFCE_OVERVIEW_WORKSPACES_DEBUG(self, MISC,
						"Unregistering view %s of type %s",
						data->ID,
						g_type_name(data->gtype));

	priv->registeredViews=g_list_remove_link(priv->registeredViews, iter);
	g_signal_emit(self, XfceOverviewWorkspacesViewManagerSignals[SIGNAL_UNREGISTERED], 0, data->ID);

	/* Free data entry and list element at iterator */
	_xfce_overview_workspaces_view_manager_entry_free(data);
	g_list_free(iter);

	/* View was unregistered successfully so return TRUE here */
	return(TRUE);
}

/* Get list of registered views types.
 * Returned GList must be freed with g_list_free_full(result, g_free) by caller.
 */
GList* xfce_overview_workspaces_view_manager_get_registered(XfceOverviewWorkspacesViewManager *self)
{
	GList						*copy;
	GList						*iter;
	XfceOverviewWorkspacesViewManagerData	*data;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_VIEW_MANAGER(self), NULL);

	/* Return a copy of all IDs stored in list of registered view types */
	copy=NULL;
	for(iter=self->priv->registeredViews; iter; iter=g_list_next(iter))
	{
		data=(XfceOverviewWorkspacesViewManagerData*)(iter->data);

		copy=g_list_prepend(copy, g_strdup(data->ID));
	}

	/* Restore order in copied list to match origin */
	copy=g_list_reverse(copy);

	/* Return copied list of IDs of registered views */
	return(copy);
}

/* Check if a view for requested ID is registered */
gboolean xfce_overview_workspaces_view_manager_has_registered_id(XfceOverviewWorkspacesViewManager *self, const gchar *inID)
{
	GList							*iter;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_VIEW_MANAGER(self), FALSE);
	g_return_val_if_fail(inID && *inID, FALSE);

	/* Check if view is registered by getting pointer to list element
	 * in list of registered views.
	 */
	iter=_xfce_overview_workspaces_view_manager_entry_find_list_entry_by_id(self, inID);
	if(iter) return(TRUE);

	/* If we get here we did not find a view for requested ID */
	return(FALSE);
}

/* Create view for requested ID */
GObject* xfce_overview_workspaces_view_manager_create_view(XfceOverviewWorkspacesViewManager *self, const gchar *inID)
{
	XfceOverviewWorkspacesViewManagerData			*data;
	GObject								*view;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_VIEW_MANAGER(self), NULL);
	g_return_val_if_fail(inID && *inID, NULL);

	/* Check if view is registered and get its data */
	data=_xfce_overview_workspaces_view_manager_entry_find_data_by_id(self, inID);
	if(!data)
	{
		g_warning("Cannot create view %s because it is not registered", inID);
		return(NULL);
	}

	/* Create view */
	view=g_object_new(data->gtype, "view-id", data->ID, NULL);

	/* Return newly created view */
	return(view);
}
