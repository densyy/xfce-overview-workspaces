/*
 * model: A simple and generic data model holding one value per row
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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_MODEL__
#define __LIBXFCE_OVERVIEW_WORKSPACES_MODEL__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <glib-object.h>

G_BEGIN_DECLS

/* Object declaration: XfceOverviewWorkspacesModelIter */
#define XFCE_OVERVIEW_WORKSPACES_TYPE_MODEL_ITER				(xfce_overview_workspaces_model_iter_get_type())
#define XFCE_OVERVIEW_WORKSPACES_MODEL_ITER(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_MODEL_ITER, XfceOverviewWorkspacesModelIter))
#define XFCE_OVERVIEW_WORKSPACES_IS_MODEL_ITER(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_MODEL_ITER))
#define XFCE_OVERVIEW_WORKSPACES_MODEL_ITER_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_MODEL_ITER, XfceOverviewWorkspacesModelIterClass))
#define XFCE_OVERVIEW_WORKSPACES_IS_MODEL_ITER_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_MODEL_ITER))
#define XFCE_OVERVIEW_WORKSPACES_MODEL_ITER_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_MODEL_ITER, XfceOverviewWorkspacesModelIterClass))

typedef struct _XfceOverviewWorkspacesModelIter			XfceOverviewWorkspacesModelIter;
typedef struct _XfceOverviewWorkspacesModelIterClass		XfceOverviewWorkspacesModelIterClass;
typedef struct _XfceOverviewWorkspacesModelIterPrivate		XfceOverviewWorkspacesModelIterPrivate;

struct _XfceOverviewWorkspacesModelIter
{
	/*< private >*/
	/* Parent instance */
	GObjectClass					parent_instance;

	/* Private structure */
	XfceOverviewWorkspacesModelIterPrivate		*priv;
};

struct _XfceOverviewWorkspacesModelIterClass
{
	/*< private >*/
	/* Parent class */
	GObjectClass					parent_class;

	/*< public >*/
	/* Virtual functions */
};


/* Object declaration: XfceOverviewWorkspacesModel */
#define XFCE_OVERVIEW_WORKSPACES_TYPE_MODEL				(xfce_overview_workspaces_model_get_type())
#define XFCE_OVERVIEW_WORKSPACES_MODEL(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_MODEL, XfceOverviewWorkspacesModel))
#define XFCE_OVERVIEW_WORKSPACES_IS_MODEL(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_MODEL))
#define XFCE_OVERVIEW_WORKSPACES_MODEL_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_MODEL, XfceOverviewWorkspacesModelClass))
#define XFCE_OVERVIEW_WORKSPACES_IS_MODEL_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass), XFCE_OVERVIEW_WORKSPACES_TYPE_MODEL))
#define XFCE_OVERVIEW_WORKSPACES_MODEL_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), XFCE_OVERVIEW_WORKSPACES_TYPE_MODEL, XfceOverviewWorkspacesModelClass))

typedef struct _XfceOverviewWorkspacesModel			XfceOverviewWorkspacesModel;
typedef struct _XfceOverviewWorkspacesModelClass		XfceOverviewWorkspacesModelClass;
typedef struct _XfceOverviewWorkspacesModelPrivate		XfceOverviewWorkspacesModelPrivate;

struct _XfceOverviewWorkspacesModel
{
	/* Parent instance */
	GObjectClass					parent_instance;

	/* Private structure */
	XfceOverviewWorkspacesModelPrivate			*priv;
};

struct _XfceOverviewWorkspacesModelClass
{
	/*< private >*/
	/* Parent class */
	GObjectClass					parent_class;

	/*< public >*/
	/* Virtual functions */
	void (*row_added)(XfceOverviewWorkspacesModel *self,
						XfceOverviewWorkspacesModelIter *inIter);
	void (*row_removed)(XfceOverviewWorkspacesModel *self,
						XfceOverviewWorkspacesModelIter *inIter);
	void (*row_changed)(XfceOverviewWorkspacesModel *self,
						XfceOverviewWorkspacesModelIter *inIter);

	void (*sort_changed)(XfceOverviewWorkspacesModel *self);

	void (*filter_changed)(XfceOverviewWorkspacesModel *self);
};


/* Public API */
typedef void (*XfceOverviewWorkspacesModelForeachFunc)(XfceOverviewWorkspacesModelIter *inIter,
											gpointer inData,
											gpointer inUserData);
typedef gint (*XfceOverviewWorkspacesModelSortFunc)(XfceOverviewWorkspacesModelIter *inLeftIter,
											XfceOverviewWorkspacesModelIter *inRightIter,
											gpointer inUserData);
typedef gboolean (*XfceOverviewWorkspacesModelFilterFunc)(XfceOverviewWorkspacesModelIter *inIter,
												gpointer inUserData);

GType xfce_overview_workspaces_model_get_type(void) G_GNUC_CONST;
GType xfce_overview_workspaces_model_iter_get_type(void) G_GNUC_CONST;

/* Model creation functions */
XfceOverviewWorkspacesModel* xfce_overview_workspaces_model_new(void);
XfceOverviewWorkspacesModel* xfce_overview_workspaces_model_new_with_free_data(GDestroyNotify inFreeDataFunc);

/* Model information functions */
gint xfce_overview_workspaces_model_get_rows_count(XfceOverviewWorkspacesModel *self);

/* Model access functions */
gpointer xfce_overview_workspaces_model_get(XfceOverviewWorkspacesModel *self, gint inRow);

/* Model manipulation functions */
gboolean xfce_overview_workspaces_model_append(XfceOverviewWorkspacesModel *self,
									gpointer inData,
									XfceOverviewWorkspacesModelIter **outIter);
gboolean xfce_overview_workspaces_model_prepend(XfceOverviewWorkspacesModel *self,
									gpointer inData,
									XfceOverviewWorkspacesModelIter **outIter);
gboolean xfce_overview_workspaces_model_insert(XfceOverviewWorkspacesModel *self,
									gint inRow,
									gpointer inData,
									XfceOverviewWorkspacesModelIter **outIter);
gboolean xfce_overview_workspaces_model_set(XfceOverviewWorkspacesModel *self,
								gint inRow,
								gpointer inData,
								XfceOverviewWorkspacesModelIter **outIter);
gboolean xfce_overview_workspaces_model_remove(XfceOverviewWorkspacesModel *self, gint inRow);
void xfce_overview_workspaces_model_remove_all(XfceOverviewWorkspacesModel *self);

/* Model foreach functions */
void xfce_overview_workspaces_model_foreach(XfceOverviewWorkspacesModel *self,
								XfceOverviewWorkspacesModelForeachFunc inForeachCallback,
								gpointer inUserData);

/* Model sort functions */
gboolean xfce_overview_workspaces_model_is_sorted(XfceOverviewWorkspacesModel *self);
void xfce_overview_workspaces_model_set_sort(XfceOverviewWorkspacesModel *self,
								XfceOverviewWorkspacesModelSortFunc inSortCallback,
								gpointer inUserData,
								GDestroyNotify inUserDataDestroyCallback);
void xfce_overview_workspaces_model_resort(XfceOverviewWorkspacesModel *self);

/* Model filter functions */
gboolean xfce_overview_workspaces_model_is_filtered(XfceOverviewWorkspacesModel *self);
void xfce_overview_workspaces_model_set_filter(XfceOverviewWorkspacesModel *self,
									XfceOverviewWorkspacesModelFilterFunc inFilterCallback,
									gpointer inUserData,
									GDestroyNotify inUserDataDestroyCallback);
gboolean xfce_overview_workspaces_model_filter_row(XfceOverviewWorkspacesModel *self, gint inRow);


/* Model iterator functions */
XfceOverviewWorkspacesModelIter* xfce_overview_workspaces_model_iter_new(XfceOverviewWorkspacesModel *inModel);
XfceOverviewWorkspacesModelIter* xfce_overview_workspaces_model_iter_new_for_row(XfceOverviewWorkspacesModel *inModel, gint inRow);
XfceOverviewWorkspacesModelIter* xfce_overview_workspaces_model_iter_copy(XfceOverviewWorkspacesModelIter *self);
gboolean xfce_overview_workspaces_model_iter_next(XfceOverviewWorkspacesModelIter *self);
gboolean xfce_overview_workspaces_model_iter_prev(XfceOverviewWorkspacesModelIter *self);
gboolean xfce_overview_workspaces_model_iter_move_to_row(XfceOverviewWorkspacesModelIter *self, gint inRow);

/* Model iterator information functions */
XfceOverviewWorkspacesModel* xfce_overview_workspaces_model_iter_get_model(XfceOverviewWorkspacesModelIter *self);
guint xfce_overview_workspaces_model_iter_get_row(XfceOverviewWorkspacesModelIter *self);

/* Model iterator access functions */
gpointer xfce_overview_workspaces_model_iter_get(XfceOverviewWorkspacesModelIter *self);

/* Model iterator manipulation functions */
gboolean xfce_overview_workspaces_model_iter_set(XfceOverviewWorkspacesModelIter *self, gpointer inData);
gboolean xfce_overview_workspaces_model_iter_remove(XfceOverviewWorkspacesModelIter *self);

/* Model filter functions */
gboolean xfce_overview_workspaces_model_iter_filter(XfceOverviewWorkspacesModelIter *self);

G_END_DECLS

#endif	/* __LIBXFCE_OVERVIEW_WORKSPACES_MODEL__ */
