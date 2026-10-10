/*
 * window: A managed window of window manager
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

#define COGL_ENABLE_EXPERIMENTAL_API
#define CLUTTER_ENABLE_EXPERIMENTAL_API

#include <libxfce-overview-workspaces/x11/window-content-x11.h>

#include <glib/gi18n-lib.h>
#include <clutter/x11/clutter-x11.h>
#ifdef CLUTTER_WINDOWING_GDK
#include <clutter/gdk/clutter-gdk.h>
#endif
#include <cogl/cogl-texture-pixmap-x11.h>
#ifdef HAVE_XCOMPOSITE
#include <X11/extensions/Xcomposite.h>
#endif
#ifdef HAVE_XDAMAGE
#include <X11/extensions/Xdamage.h>
#endif
#include <gdk/gdkx.h>

#include <libxfce-overview-workspaces/window-content.h>
#include <libxfce-overview-workspaces/x11/window-tracker-x11.h>
#include <libxfce-overview-workspaces/x11/window-tracker-window-x11.h>
#include <libxfce-overview-workspaces/x11/thumbnail-cache.h>
#include <libxfce-overview-workspaces/x11/window-content-frozen.h>
#include <libxfce-overview-workspaces/core.h>
#include <libxfce-overview-workspaces/marshal.h>
#include <libxfce-overview-workspaces/stylable.h>
#include <libxfce-overview-workspaces/window-tracker.h>
#include <libxfce-overview-workspaces/settings.h>
#include <libxfce-overview-workspaces/enums.h>
#include <libxfce-overview-workspaces/debug.h>


/* Definitions */
typedef enum /*< skip,prefix=XFCE_OVERVIEW_WORKSPACES_WINDOW_CONTENT_X11_WORKAROUND_MODE >*/
{
	XFCE_OVERVIEW_WORKSPACES_WINDOW_CONTENT_X11_WORKAROUND_MODE_NONE=0,
	XFCE_OVERVIEW_WORKSPACES_WINDOW_CONTENT_X11_WORKAROUND_MODE_UNMINIMIZING,
	XFCE_OVERVIEW_WORKSPACES_WINDOW_CONTENT_X11_WORKAROUND_MODE_REMINIMIZING,
	XFCE_OVERVIEW_WORKSPACES_WINDOW_CONTENT_X11_WORKAROUND_MODE_DONE
} XfceOverviewWorkspacesWindowContentX11WorkaroundMode;

/* Define this class in GObject system */
static void _xfce_overview_workspaces_window_content_clutter_content_iface_init(ClutterContentIface *iface);
static void _xfce_overview_workspaces_window_content_x11_stylable_iface_init(XfceOverviewWorkspacesStylableInterface *iface);

struct _XfceOverviewWorkspacesWindowContentX11Private
{
	/* Properties related */
	XfceOverviewWorkspacesWindowTrackerWindowX11			*window;
	ClutterColor								*outlineColor;
	gfloat										outlineWidth;
	gboolean									isSuspended;
	gboolean									includeWindowFrame;

	gboolean									unmappedWindowIconXFill;
	gboolean									unmappedWindowIconYFill;
	gfloat										unmappedWindowIconXAlign;
	gfloat										unmappedWindowIconYAlign;
	gfloat										unmappedWindowIconXScale;
	gfloat										unmappedWindowIconYScale;
	XfceOverviewWorkspacesAnchorPoint						unmappedWindowIconAnchorPoint;

	gchar										*styleClasses;
	gchar										*stylePseudoClasses;

	/* Instance related */
	gboolean									isFallback;
	CoglTexture									*texture;
	Window										xWindowID;
	Pixmap										pixmap;
#ifdef HAVE_XDAMAGE
	Damage										damage;
#endif

	guint										suspendSignalID;
	gboolean									isMapped;
	gboolean									isAppSuspended;

	XfceOverviewWorkspacesWindowTracker					*windowTracker;
	XfceOverviewWorkspacesWindowContentX11WorkaroundMode	workaroundMode;
	guint										workaroundStateSignalID;

	gboolean									suspendAfterResumeOnIdle;

	guint										windowClosedSignalID;

	XfceOverviewWorkspacesSettings							*settings;

	/* On-disk frozen cache key (client XID). Set in window-content-frozen. */
	Window										frozenCacheXID;

	/* NOTE: throttle for damage-triggered frozen saves (monotonic usec). */
	gint64										lastFrozenSaveUsec;
};

G_DEFINE_TYPE_WITH_CODE(XfceOverviewWorkspacesWindowContentX11,
						xfce_overview_workspaces_window_content_x11,
						XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_CONTENT,
						G_ADD_PRIVATE(XfceOverviewWorkspacesWindowContentX11)
						G_IMPLEMENT_INTERFACE(CLUTTER_TYPE_CONTENT, _xfce_overview_workspaces_window_content_clutter_content_iface_init)
						G_IMPLEMENT_INTERFACE(XFCE_OVERVIEW_WORKSPACES_TYPE_STYLABLE, _xfce_overview_workspaces_window_content_x11_stylable_iface_init))

/* Properties */
enum
{
	PROP_0,

	PROP_WINDOW,

	PROP_SUSPENDED,

	PROP_OUTLINE_COLOR,
	PROP_OUTLINE_WIDTH,

	PROP_INCLUDE_WINDOW_FRAME,

	PROP_UNMAPPED_WINDOW_ICON_X_FILL,
	PROP_UNMAPPED_WINDOW_ICON_Y_FILL,
	PROP_UNMAPPED_WINDOW_ICON_X_ALIGN,
	PROP_UNMAPPED_WINDOW_ICON_Y_ALIGN,
	PROP_UNMAPPED_WINDOW_ICON_X_SCALE,
	PROP_UNMAPPED_WINDOW_ICON_Y_SCALE,
	PROP_UNMAPPED_WINDOW_ICON_ANCHOR_POINT,

	/* From interface: XfceOverviewWorkspacesStylable */
	PROP_STYLE_CLASSES,
	PROP_STYLE_PSEUDO_CLASSES,

	PROP_LAST
};

static GParamSpec* XfceOverviewWorkspacesWindowContentX11Properties[PROP_LAST]={ 0, };

/* IMPLEMENTATION: Private variables and methods */
#define COMPOSITE_VERSION_MIN_MAJOR		0
#define COMPOSITE_VERSION_MIN_MINOR		2

struct _XfceOverviewWorkspacesWindowContentX11PriorityMap
{
	const gchar		*name;
	gint			priority;
};
typedef struct _XfceOverviewWorkspacesWindowContentX11PriorityMap		XfceOverviewWorkspacesWindowContentX11PriorityMap;

static gboolean									_xfce_overview_workspaces_window_content_x11_have_checked_extensions=FALSE;
static gboolean									_xfce_overview_workspaces_window_content_x11_have_composite_extension=FALSE;
static gboolean									_xfce_overview_workspaces_window_content_x11_have_damage_extension=FALSE;
static int										_xfce_overview_workspaces_window_content_x11_damage_event_base=0;

static GList*									_xfce_overview_workspaces_window_content_x11_resume_idle_queue=NULL;
static guint									_xfce_overview_workspaces_window_content_x11_resume_idle_id=0;
static guint									_xfce_overview_workspaces_window_content_x11_resume_shutdown_signal_id=0;

static guint									_xfce_overview_workspaces_window_content_x11_settings_priority_notify_id=0;
static gint										_xfce_overview_workspaces_window_content_x11_window_creation_priority=-1;
static XfceOverviewWorkspacesWindowContentX11PriorityMap	_xfce_overview_workspaces_window_content_x11_window_creation_priority_map[]=
												{
													{ "immediate", -1 }, /* First entry is default value */
													{ "high", G_PRIORITY_HIGH_IDLE },
													{ "normal", G_PRIORITY_DEFAULT_IDLE },
													{ "low", G_PRIORITY_LOW },
													{ NULL, 0 },
												};
static guint									_xfce_overview_workspaces_window_content_x11_window_creation_shutdown_signal_id=0;

/* Forward declarations */
static void _xfce_overview_workspaces_window_content_x11_suspend(XfceOverviewWorkspacesWindowContentX11 *self);
static void _xfce_overview_workspaces_window_content_x11_resume(XfceOverviewWorkspacesWindowContentX11 *self);
static gboolean _xfce_overview_workspaces_window_content_x11_resume_on_idle(gpointer inUserData);


/* Remove all entries from resume queue and release all allocated resources */
static void _xfce_overview_workspaces_window_content_x11_destroy_resume_queue(void)
{
	XfceOverviewWorkspacesCore							*core;
	gint									queueSize;

	/* Disconnect application "shutdown" signal handler */
	if(_xfce_overview_workspaces_window_content_x11_resume_shutdown_signal_id)
	{
		XFCE_OVERVIEW_WORKSPACES_DEBUG(NULL, WINDOWS,
							"Disconnecting shutdown signal handler %u because of resume queue destruction",
							_xfce_overview_workspaces_window_content_x11_resume_shutdown_signal_id);

		core=xfce_overview_workspaces_core_get_default();
		g_signal_handler_disconnect(core, _xfce_overview_workspaces_window_content_x11_resume_shutdown_signal_id);
		_xfce_overview_workspaces_window_content_x11_resume_shutdown_signal_id=0;
	}

	/* Remove idle source if available */
	if(_xfce_overview_workspaces_window_content_x11_resume_idle_id)
	{
		XFCE_OVERVIEW_WORKSPACES_DEBUG(NULL, WINDOWS,
							"Removing resume window content idle source with ID %u",
							_xfce_overview_workspaces_window_content_x11_resume_idle_id);

		g_source_remove(_xfce_overview_workspaces_window_content_x11_resume_idle_id);
		_xfce_overview_workspaces_window_content_x11_resume_idle_id=0;
	}

	/* Destroy resume-on-idle queue if available*/
	if(_xfce_overview_workspaces_window_content_x11_resume_idle_queue)
	{
		queueSize=g_list_length(_xfce_overview_workspaces_window_content_x11_resume_idle_queue);
		if(queueSize>0) g_warning("Destroying window content resume queue containing %d windows.", queueSize);
#ifdef DEBUG
		if(queueSize>0)
		{
			GList								*iter;
			XfceOverviewWorkspacesWindowContentX11			*content;
			XfceOverviewWorkspacesWindowTrackerWindow		*window;

			for(iter=_xfce_overview_workspaces_window_content_x11_resume_idle_queue; iter; iter=g_list_next(iter))
			{
				content=XFCE_OVERVIEW_WORKSPACES_WINDOW_CONTENT_X11(iter->data);
				window=xfce_overview_workspaces_window_content_x11_get_window(content);
				g_print("Window content in resume queue: Item %s@%p for window '%s'\n",
							G_OBJECT_TYPE_NAME(content), content,
							xfce_overview_workspaces_window_tracker_window_get_name(window));
			}
		}
#endif

		XFCE_OVERVIEW_WORKSPACES_DEBUG(NULL, WINDOWS, "Destroying window content resume queue");
		g_list_free(_xfce_overview_workspaces_window_content_x11_resume_idle_queue);
		_xfce_overview_workspaces_window_content_x11_resume_idle_queue=NULL;
	}
}

/* Remove window content from resume on idle queue */
static void _xfce_overview_workspaces_window_content_x11_resume_on_idle_remove(XfceOverviewWorkspacesWindowContentX11 *self)
{
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_CONTENT_X11(self));

	/* Remove window content from queue */
	if(_xfce_overview_workspaces_window_content_x11_resume_idle_queue)
	{
		GList								*queueEntry;

		/* Lookup window content in queue and remove it from queue. If queue is empty
		 * after removal, remove idle source also.
		 */
		queueEntry=g_list_find(_xfce_overview_workspaces_window_content_x11_resume_idle_queue, self);
		if(queueEntry)
		{
			/* Remove window content from queue */
			_xfce_overview_workspaces_window_content_x11_resume_idle_queue=g_list_delete_link(_xfce_overview_workspaces_window_content_x11_resume_idle_queue, queueEntry);
			XFCE_OVERVIEW_WORKSPACES_DEBUG(self, WINDOWS,
								"Removed queue entry %p for window '%s' because of releasing resources",
								queueEntry,
								xfce_overview_workspaces_window_tracker_window_get_name(XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW(self->priv->window)));
		}
	}

	/* If queue is empty remove idle source as well */
	if(!_xfce_overview_workspaces_window_content_x11_resume_idle_queue &&
		_xfce_overview_workspaces_window_content_x11_resume_idle_id)
	{
		XFCE_OVERVIEW_WORKSPACES_DEBUG(self, WINDOWS,
							"Removing idle source with ID %u because queue is empty",
							_xfce_overview_workspaces_window_content_x11_resume_idle_id);

		g_source_remove(_xfce_overview_workspaces_window_content_x11_resume_idle_id);
		_xfce_overview_workspaces_window_content_x11_resume_idle_id=0;
	}
}

/* Add window content to resume on idle queue */
static void _xfce_overview_workspaces_window_content_x11_resume_on_idle_add(XfceOverviewWorkspacesWindowContentX11 *self)
{
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_CONTENT_X11(self));

	XFCE_OVERVIEW_WORKSPACES_DEBUG(self, WINDOWS,
						"Using resume on idle for window '%s'",
						xfce_overview_workspaces_window_tracker_window_get_name(XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW(self->priv->window)));

	/* Only add callback to resume window content if no one was added */
	if(!g_list_find(_xfce_overview_workspaces_window_content_x11_resume_idle_queue, self))
	{
		/* Queue window content for resume */
		_xfce_overview_workspaces_window_content_x11_resume_idle_queue=g_list_append(_xfce_overview_workspaces_window_content_x11_resume_idle_queue, self);
		XFCE_OVERVIEW_WORKSPACES_DEBUG(self, WINDOWS,
							"Queued window resume of '%s'",
							xfce_overview_workspaces_window_tracker_window_get_name(XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW(self->priv->window)));
	}

	/* Create idle source for resuming queued window contents but with
	 * high priority to get window content created as soon as possible.
	 */
	if(_xfce_overview_workspaces_window_content_x11_resume_idle_queue &&
		!_xfce_overview_workspaces_window_content_x11_resume_idle_id)
	{
		_xfce_overview_workspaces_window_content_x11_resume_idle_id=clutter_threads_add_idle_full(_xfce_overview_workspaces_window_content_x11_window_creation_priority,
																				_xfce_overview_workspaces_window_content_x11_resume_on_idle,
																				NULL,
																				NULL);
		XFCE_OVERVIEW_WORKSPACES_DEBUG(self, WINDOWS,
							"Created idle source with ID %u with priority of %d because of new resume queue created for window resume of '%s'",
							_xfce_overview_workspaces_window_content_x11_resume_idle_id,
							_xfce_overview_workspaces_window_content_x11_window_creation_priority,
							xfce_overview_workspaces_window_tracker_window_get_name(XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW(self->priv->window)));
	}

	/* Connect to "shutdown" signal of application to clean up resume queue */
	if(!_xfce_overview_workspaces_window_content_x11_resume_shutdown_signal_id)
	{
		XfceOverviewWorkspacesCore					*core;

		core=xfce_overview_workspaces_core_get_default();
		_xfce_overview_workspaces_window_content_x11_resume_shutdown_signal_id=g_signal_connect(core,
																					"shutdown",
																					G_CALLBACK(_xfce_overview_workspaces_window_content_x11_destroy_resume_queue),
																					NULL);
		XFCE_OVERVIEW_WORKSPACES_DEBUG(self, WINDOWS,
							"Connected to shutdown signal with handler ID %u for resume queue destruction",
							_xfce_overview_workspaces_window_content_x11_resume_shutdown_signal_id);
	}
}

/* Value for window creation priority in settings has changed */
static void _xfce_overview_workspaces_window_content_x11_on_window_creation_priority_value_changed(XfceOverviewWorkspacesSettings *inSettings,
																						const gchar *inProperty,
																						const GValue *inValue,
																						gpointer inUserData)
{
	const gchar									*priorityValue;
	XfceOverviewWorkspacesWindowContentX11PriorityMap		*found;

	g_return_if_fail(g_strcmp0(inProperty, "window-content-creation-priority")==0);
	g_return_if_fail(inValue && G_VALUE_HOLDS_STRING(inValue));

	/* Determine priority from new value */
	priorityValue=g_value_get_string(inValue);
	found=_xfce_overview_workspaces_window_content_x11_window_creation_priority_map;
	while(found->name && g_strcmp0(priorityValue, found->name)!=0) found++;

	/* Set default value if no match was found in priority map was found */
	if(!found || !found->name)
	{
		/* Default value is the first one in mapping */
		found=_xfce_overview_workspaces_window_content_x11_window_creation_priority_map;

		g_warning("Unknown value '%s' for property '%s' - defaulting to '%s' with priority of %d",
					priorityValue,
					inProperty,
					found->name,
					found->priority);
	}

	/* Set priority */
	if(found)
	{
		_xfce_overview_workspaces_window_content_x11_window_creation_priority=found->priority;
		XFCE_OVERVIEW_WORKSPACES_DEBUG(NULL, WINDOWS,
							"Setting window creation priority to '%s' with priority of %d",
							found->name,
							found->priority);
	}
}

/* Disconnect signal handlers for settings value changed notifications */
static void _xfce_overview_workspaces_window_content_x11_on_window_creation_priority_shutdown(XfceOverviewWorkspacesCore *inCore, gpointer inUserData)
{
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_CORE(inCore));

	/* Disconnect application "shutdown" signal handler */
	if(_xfce_overview_workspaces_window_content_x11_window_creation_shutdown_signal_id)
	{

		XFCE_OVERVIEW_WORKSPACES_DEBUG(NULL, WINDOWS,
							"Disconnecting shutdown signal handler %u for window creation priority value change notifications",
							_xfce_overview_workspaces_window_content_x11_window_creation_shutdown_signal_id);

		g_signal_handler_disconnect(inCore, _xfce_overview_workspaces_window_content_x11_window_creation_shutdown_signal_id);
		_xfce_overview_workspaces_window_content_x11_window_creation_shutdown_signal_id=0;
	}

	/* Disconnect property changed signal handler */
	if(_xfce_overview_workspaces_window_content_x11_settings_priority_notify_id)
	{
		XfceOverviewWorkspacesSettings				*settings;

		XFCE_OVERVIEW_WORKSPACES_DEBUG(NULL, WINDOWS,
							"Disconnecting property changed signal handler %u for window creation priority value change notifications",
							_xfce_overview_workspaces_window_content_x11_settings_priority_notify_id);

		settings=xfce_overview_workspaces_core_get_settings(inCore);
		g_signal_handler_disconnect(settings, _xfce_overview_workspaces_window_content_x11_settings_priority_notify_id);
		_xfce_overview_workspaces_window_content_x11_settings_priority_notify_id=0;
	}
}

/* Check if we should workaround unmapped window for requested window and set up workaround */
static void _xfce_overview_workspaces_window_content_x11_on_workaround_state_changed(XfceOverviewWorkspacesWindowContentX11 *self,
																		gpointer inUserData)
{
	XfceOverviewWorkspacesWindowContentX11Private		*priv;
	XfceOverviewWorkspacesWindowTrackerWindowState		windowState;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_CONTENT_X11(self));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_WINDOW_X11(inUserData));

	priv=self->priv;

	/* Handle state change of window */
	windowState=xfce_overview_workspaces_window_tracker_window_get_state(XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW(priv->window));

	switch(priv->workaroundMode)
	{
		case XFCE_OVERVIEW_WORKSPACES_WINDOW_CONTENT_X11_WORKAROUND_MODE_UNMINIMIZING:
			/* Check if window is unminized now, then update content texture and
			 * minimize window again.
			 */
			if(!(windowState & XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW_STATE_MINIMIZED))
			{
				if(priv->texture &&
					priv->workaroundMode!=XFCE_OVERVIEW_WORKSPACES_WINDOW_CONTENT_X11_WORKAROUND_MODE_NONE &&
					priv->isMapped==TRUE)
				{
					/* Copy current texture as it might get inaccessible. If we copy it now
					 * when can draw the last image known. If we can copy it successfully
					 * replace current texture with the copied one.
					 */
					CoglTextureComponents	textureComponents;
					CoglPixelFormat			textureFormat;
					guint					textureWidth;
					guint					textureHeight;
					gint					textureSize;
					guint8					*textureData;

					textureComponents=cogl_texture_get_components(priv->texture);
					textureFormat=(textureComponents==COGL_TEXTURE_COMPONENTS_RGBA ? COGL_PIXEL_FORMAT_RGBA_8888_PRE : COGL_PIXEL_FORMAT_RGB_888);
					textureSize=cogl_texture_get_data(priv->texture, textureFormat, 0, NULL);
					textureWidth=cogl_texture_get_width(priv->texture);
					textureHeight=cogl_texture_get_height(priv->texture);
					textureData=g_malloc(textureSize);
					if(textureData)
					{
						CoglTexture			*copyTexture;
						gint				copyTextureSize;
						ClutterBackend		*backend;
						CoglContext			*context;
						CoglError			*error;

						copyTextureSize=cogl_texture_get_data(priv->texture, textureFormat, 0, textureData);
						if(copyTextureSize)
						{
							error=NULL;

							backend=clutter_get_default_backend();
							context=clutter_backend_get_cogl_context(backend);
							copyTexture=cogl_texture_2d_new_from_data(context,
																		textureWidth,
																		textureHeight,
																		textureFormat,
																		0,
																		textureData,
																		&error);

							if(!copyTexture || error)
							{
								g_warning("Could not create copy of texture of mininized window '%s': %s",
											xfce_overview_workspaces_window_tracker_window_get_name(XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW(priv->window)),
											(error && error->message) ? error->message : "Unknown error");

								/* Release allocated resources */
								if(copyTexture)
								{
									cogl_object_unref(copyTexture);
									copyTexture=NULL;
								}

								if(error)
								{
									cogl_error_free(error);
									error=NULL;
								}
							}

							if(copyTexture)
							{
								cogl_object_unref(priv->texture);
								priv->texture=copyTexture;
							}
						}
							else g_warning("Could not determine size of texture of minimized window '%s'",
											xfce_overview_workspaces_window_tracker_window_get_name(XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW(priv->window)));
					}
						else
						{
							g_warning("Could not allocate memory for copy of texture of mininized window '%s'",
										xfce_overview_workspaces_window_tracker_window_get_name(XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW(priv->window)));
						}
				}

				xfce_overview_workspaces_window_tracker_window_hide(XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW(priv->window));
				priv->workaroundMode=XFCE_OVERVIEW_WORKSPACES_WINDOW_CONTENT_X11_WORKAROUND_MODE_REMINIMIZING;
			}
			break;

		case XFCE_OVERVIEW_WORKSPACES_WINDOW_CONTENT_X11_WORKAROUND_MODE_REMINIMIZING:
			/* Check if window is now minized again, so stop workaround and
			 * disconnecting signals.
			 */
			if(windowState & XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW_STATE_MINIMIZED)
			{
				priv->workaroundMode=XFCE_OVERVIEW_WORKSPACES_WINDOW_CONTENT_X11_WORKAROUND_MODE_DONE;
				if(priv->workaroundStateSignalID)
				{
					g_signal_handler_disconnect(priv->windowTracker, priv->workaroundStateSignalID);
					priv->workaroundStateSignalID=0;
				}
			}
			break;

		default:
			/* We should never get here but if we do it is more or less
			 * a critical error. Ensure that window is minimized (again)
			 * and stop xfce-overview-workspaces.
			 */
			xfce_overview_workspaces_window_tracker_window_hide(XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW(priv->window));
			g_assert_not_reached();
			break;
	}
}

static void _xfce_overview_workspaces_window_content_x11_setup_workaround(XfceOverviewWorkspacesWindowContentX11 *self, XfceOverviewWorkspacesWindowTrackerWindowX11 *inWindow)
{
	XfceOverviewWorkspacesWindowContentX11Private		*priv;
	gboolean								doWorkaround;
	XfceOverviewWorkspacesWindowTrackerWindowState		windowState;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_CONTENT_X11(self));
	g_return_if_fail(inWindow!=NULL && XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_WINDOW_X11(inWindow));

	priv=self->priv;

	/* Check if should workaround unmapped windows at all */
	doWorkaround=xfce_overview_workspaces_settings_get_enable_workaround_unmapped_window(priv->settings);
	if(!doWorkaround) return;

	/* Only workaround unmapped windows */
	windowState=xfce_overview_workspaces_window_tracker_window_get_state(XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW(inWindow));
	if(!(windowState & XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW_STATE_MINIMIZED)) return;

	/* Check if workaround is already set up */
	if(priv->workaroundMode!=XFCE_OVERVIEW_WORKSPACES_WINDOW_CONTENT_X11_WORKAROUND_MODE_NONE) return;

	/* Set flag that workaround is (going to be) set up */
	priv->workaroundMode=XFCE_OVERVIEW_WORKSPACES_WINDOW_CONTENT_X11_WORKAROUND_MODE_UNMINIMIZING;

	/* The workaround is as follows:
	 *
	 * 1.) Set up signal handlers to get notified about changes of window
	 * 2.) Unminimize window
	 * 3.) If window is visible it will be activated by design, so reactivate
	 *     last active window
	 * 4.) Minimize window again
	 * 5.) Stop watching for changes of window by disconnecting signal handlers
	 */
	priv->workaroundStateSignalID=g_signal_connect_swapped(priv->windowTracker,
															"window-state-changed",
															G_CALLBACK(_xfce_overview_workspaces_window_content_x11_on_workaround_state_changed),
															self);
	xfce_overview_workspaces_window_tracker_window_show(XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW(inWindow));
}

/* Check extension and set up basics */
static void _xfce_overview_workspaces_window_content_x11_check_extension(void)
{
	Display		*display G_GNUC_UNUSED;
#ifdef HAVE_XDAMAGE
	int			damageError=0;
#endif
#ifdef HAVE_XCOMPOSITE
	int			compositeEventBase=0;
	int			compositeError=0;
	int			compositeMajor, compositeMinor;
#endif

	/* Check if we have already checked extensions */
	if(_xfce_overview_workspaces_window_content_x11_have_checked_extensions!=FALSE) return;

	/* Mark that we have check for extensions regardless of any error */
	_xfce_overview_workspaces_window_content_x11_have_checked_extensions=TRUE;

	/* NOTE: the on-disk frozen cache is intentionally never purged here.
	 * Stale files are removed on window close and overwritten on recapture,
	 * so inactive workspaces keep their last frame across restarts.
	 */

	/* Get display */
	display=xfce_overview_workspaces_window_tracker_x11_get_display();
	if(G_UNLIKELY(display==None))
	{
		g_critical("No default X11 display found to check X extensions");
	}

	/* Check for composite extenstion */
	_xfce_overview_workspaces_window_content_x11_have_composite_extension=FALSE;
#ifdef HAVE_XCOMPOSITE
	if(G_LIKELY(display!=None) &&
		XCompositeQueryExtension(display, &compositeEventBase, &compositeError))
	{
		compositeMajor=compositeMinor=0;
		if(XCompositeQueryVersion(display, &compositeMajor, &compositeMinor))
		{
			if(compositeMajor>=COMPOSITE_VERSION_MIN_MAJOR && compositeMinor>=COMPOSITE_VERSION_MIN_MINOR)
			{
				_xfce_overview_workspaces_window_content_x11_have_composite_extension=TRUE;
			}
				else
				{
					g_warning("Need at least version %d.%d of composite extension but found %d.%d - using only fallback images",
								COMPOSITE_VERSION_MIN_MAJOR, COMPOSITE_VERSION_MIN_MINOR, compositeMajor, compositeMinor);
				}
		}
			else g_warning("Query for X composite extension failed - using only fallback imagess");
	}
		else g_warning("X does not support composite extension - using only fallback images");
#endif

	/* Get base of damage event in X */
	_xfce_overview_workspaces_window_content_x11_have_damage_extension=FALSE;
	_xfce_overview_workspaces_window_content_x11_damage_event_base=0;

#ifdef HAVE_XDAMAGE
	if(G_LIKELY(display!=None) &&
		XDamageQueryExtension(display, &_xfce_overview_workspaces_window_content_x11_damage_event_base, &damageError))
	{
		_xfce_overview_workspaces_window_content_x11_have_damage_extension=TRUE;
	}
		else
		{
			g_warning("Query for X damage extension resulted in error code %d - using only still images of windows",
						damageError);
		}
#endif
}

/* Frozen frame persistence lives in window-content-frozen.[hc] */

/* Suspension state of application changed */
static void _xfce_overview_workspaces_window_content_x11_on_core_suspended_changed(XfceOverviewWorkspacesWindowContentX11 *self,
																		GParamSpec *inSpec,
																		gpointer inUserData)
{
	XfceOverviewWorkspacesWindowContentX11Private		*priv;
	XfceOverviewWorkspacesCore							*core;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_CONTENT_X11(self));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_CORE(inUserData));

	priv=self->priv;
	core=XFCE_OVERVIEW_WORKSPACES_CORE(inUserData);

	/* Get application suspend state */
	priv->isAppSuspended=xfce_overview_workspaces_core_is_suspended(core);

	/* If application is suspended then suspend this window too ... */
	if(priv->isAppSuspended)
	{
		_xfce_overview_workspaces_window_content_x11_suspend(self);
	}
		/* ... otherwise resume window unconditionally.
	 * NOTE: pre-existing windows (opened before the daemon) never got a
	 * MapNotify, so priv->isMapped can be FALSE for a perfectly Viewable
	 * window. Gating on isMapped here left them suspended forever — only
	 * apps opened after the daemon appeared. Always resume; resume itself
	 * degrades gracefully (fallback + idle retry) for truly offscreen ones.
	 */
		else
		{
			_xfce_overview_workspaces_window_content_x11_resume(self);
		}
}

/* Filter X events for damages */
static void _xfce_overview_workspaces_window_content_x11_handle_x_event(XfceOverviewWorkspacesWindowContentX11 *self,
															XEvent *inXEvent)
{
	XfceOverviewWorkspacesWindowContentX11Private		*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_CONTENT_X11(self));
	g_return_if_fail(inXEvent);

	priv=self->priv;

	/* Check for mapped, unmapped related X events as pixmap, damage, texture etc.
	 * needs to get resumed (acquired) or suspended (released)
	 */
	if(inXEvent->xany.window==priv->xWindowID)
	{
		switch(inXEvent->type)
		{
			case MapNotify:
			case ConfigureNotify:
				priv->isMapped=TRUE;
				if(!priv->isAppSuspended) _xfce_overview_workspaces_window_content_x11_resume(self);
				break;

			case UnmapNotify:
			{
				XfceOverviewWorkspacesWindowTrackerWindowState unmapState=0;

				if(priv->window)
				{
					unmapState=xfce_overview_workspaces_window_tracker_window_get_state(XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW(priv->window));
				}

				/* Minimized or closed windows lose their live content, so
				 * suspend live updates. The last texture is kept as frozen
				 * frame.
				 */
				if(priv->window==NULL ||
					(unmapState & XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW_STATE_MINIMIZED)!=0)
				{
					priv->isMapped=FALSE;
					_xfce_overview_workspaces_window_content_x11_suspend(self);
				}
				/* Windows hidden by a workspace switch (HIDDEN without MINIMIZED)
				 * keep their last frame: only mark them as unmapped and stop
				 * invalidating. Do NOT suspend here (that would destroy damage
				 * and pixmap) and keep the frozen live texture with
				 * isFallback==FALSE, so paint keeps showing the last frame
				 * instead of the fallback icon.
				 */
				else if((unmapState & XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW_STATE_HIDDEN)!=0)
				{
					priv->isMapped=FALSE;
				}
				/* Transient unmap (neither minimized nor hidden):
				 * keep live updates running.
				 */
				break;
			}
			case DestroyNotify:
				priv->isMapped=FALSE;
				_xfce_overview_workspaces_window_content_x11_suspend(self);
				break;

			default:
				/* We do not handle this type of X event, drop through ... */
				break;
		}
	}

	/* Check for damage event */
#ifdef HAVE_XDAMAGE
	if(_xfce_overview_workspaces_window_content_x11_have_damage_extension &&
		_xfce_overview_workspaces_window_content_x11_damage_event_base &&
		inXEvent->type==(_xfce_overview_workspaces_window_content_x11_damage_event_base + XDamageNotify) &&
		((XDamageNotifyEvent*)inXEvent)->damage==priv->damage &&
		priv->isMapped &&
		priv->workaroundMode==XFCE_OVERVIEW_WORKSPACES_WINDOW_CONTENT_X11_WORKAROUND_MODE_NONE)
	{
		/* Update texture for live window content */
		clutter_content_invalidate(CLUTTER_CONTENT(self));

		/* Persist frozen frame on damage too (throttled 5s): pre-existing
		 * static windows (e.g. terminals) rarely re-resume, so the resume-
		 * only save left them without any JPG while video windows kept
		 * refreshing. Damage proves the pixmap is live-readable.
		 */
		if(xfce_overview_workspaces_window_content_frozen_should_save_damage(&priv->lastFrozenSaveUsec))
		{
			xfce_overview_workspaces_window_content_frozen_save(priv->isFallback, priv->texture, priv->xWindowID, priv->window, &priv->frozenCacheXID);
		}
	}
#endif
}

static ClutterX11FilterReturn _xfce_overview_workspaces_window_content_x11_on_x_event(XEvent *inXEvent, ClutterEvent *inEvent, gpointer inUserData)
{
	XfceOverviewWorkspacesWindowContentX11				*self;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_CONTENT_X11(inUserData), CLUTTER_X11_FILTER_CONTINUE);

	self=XFCE_OVERVIEW_WORKSPACES_WINDOW_CONTENT_X11(inUserData);

	/* Call X event handler */
	_xfce_overview_workspaces_window_content_x11_handle_x_event(self, inXEvent);

	/* Always return FILTER_CONTINUE value to let other components handle this
	 * event also.
	 */
	return(CLUTTER_X11_FILTER_CONTINUE);
}

#ifdef CLUTTER_WINDOWING_GDK
static GdkFilterReturn _xfce_overview_workspaces_window_content_x11_on_gdkx_event(GdkXEvent *inXEvent, GdkEvent *inEvent, gpointer inUserData)
{
	XfceOverviewWorkspacesWindowContentX11				*self;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_CONTENT_X11(inUserData), GDK_FILTER_CONTINUE);

	self=XFCE_OVERVIEW_WORKSPACES_WINDOW_CONTENT_X11(inUserData);

	/* Call X event handler */
	_xfce_overview_workspaces_window_content_x11_handle_x_event(self, inXEvent);

	/* Always return FILTER_CONTINUE value to let other components handle this
	 * event also.
	 */
	return(GDK_FILTER_CONTINUE);
}
#endif

/* Release all resources used by this instance */
static void _xfce_overview_workspaces_window_content_x11_release_resources(XfceOverviewWorkspacesWindowContentX11 *self)
{
	XfceOverviewWorkspacesWindowContentX11Private		*priv;
	Display									*display;
	gint									trapError;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_CONTENT_X11(self));

	priv=self->priv;

	/* This live update will be suspended so remove it from queue */
	_xfce_overview_workspaces_window_content_x11_resume_on_idle_remove(self);

	display=xfce_overview_workspaces_window_tracker_x11_get_display();
	if(G_UNLIKELY(display==None))
	{
		g_critical("No default X11 display found to release clutter content resources");
	}

	/* Release resources. It might be important to release them
	 * in reverse order as they were created.
	 */
	clutter_x11_trap_x_errors();
	{
		if(priv->texture)
		{
			cogl_object_unref(priv->texture);
			priv->texture=NULL;
		}

#ifdef HAVE_XDAMAGE
		if(priv->damage!=None)
		{
			XDamageDestroy(display, priv->damage);
			XSync(display, False);
			priv->damage=None;
		}
#endif
		if(priv->pixmap!=None)
		{
			XFreePixmap(display, priv->pixmap);
			priv->pixmap=None;
		}

		if(priv->xWindowID!=None)
		{
#ifdef HAVE_XCOMPOSITE
			if(_xfce_overview_workspaces_window_content_x11_have_composite_extension)
			{
				XCompositeUnredirectWindow(display, priv->xWindowID, CompositeRedirectAutomatic);
				XSync(display, False);
			}
#endif
			/* NOTE: do NOT invalidate the on-disk frozen cache here. This
			 * function runs on dispose, which also happens transiently
			 * during a workspace move: the source LiveWorkspace destroys
			 * its actor before the destination creates a new one, dropping
			 * the shared WindowContent refcount to zero. Deleting the JPG
			 * then leaves the destination with no frozen fallback and the
			 * thumbnail disappears. Stale files are removed on window
			 * close (see _on_window_closed) and overwritten on recapture.
			 */
			priv->xWindowID=None;
		}

		/* Window is suspended now */
		if(priv->isSuspended!=TRUE)
		{
			priv->isSuspended=TRUE;

			/* Notify about property change */
			g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesWindowContentX11Properties[PROP_SUSPENDED]);
		}
	}

	/* Check if everything went well */
	trapError=clutter_x11_untrap_x_errors();
	if(trapError!=0)
	{
		XFCE_OVERVIEW_WORKSPACES_DEBUG(self, WINDOWS,
							"X error %d occured while releasing resources for window '%s'",
							trapError,
							xfce_overview_workspaces_window_tracker_window_get_name(XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW(priv->window)));
		return;
	}

	XFCE_OVERVIEW_WORKSPACES_DEBUG(self, WINDOWS,
						"Released resources for window '%s' to handle live texture updates",
						xfce_overview_workspaces_window_tracker_window_get_name(XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW(priv->window)));
}

/* Suspend from handling live updates */
static void _xfce_overview_workspaces_window_content_x11_suspend(XfceOverviewWorkspacesWindowContentX11 *self)
{
	XfceOverviewWorkspacesWindowContentX11Private		*priv;
	Display									*display;
	gint									trapError;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_CONTENT_X11(self));

	priv=self->priv;

	/* This live update will be suspended so remove it from queue */
	_xfce_overview_workspaces_window_content_x11_resume_on_idle_remove(self);

	display=xfce_overview_workspaces_window_tracker_x11_get_display();
	if(G_UNLIKELY(display==None))
	{
		g_critical("No default X11 display found for suspend");
	}

	/* Release resources */
	clutter_x11_trap_x_errors();
	{
		/* Suspend live updates from texture */
		if(priv->texture && !priv->isFallback)
		{
#ifdef HAVE_XDAMAGE
			cogl_texture_pixmap_x11_set_damage_object(COGL_TEXTURE_PIXMAP_X11(priv->texture), 0, 0);
#endif
		}

		/* Release damage */
#ifdef HAVE_XDAMAGE
		if(priv->damage!=None)
		{
			XDamageDestroy(display, priv->damage);
			XSync(display, False);
			priv->damage=None;
		}
#endif

		/* Release pixmap */
		if(priv->pixmap!=None)
		{
			XFreePixmap(display, priv->pixmap);
			priv->pixmap=None;
		}

		/* Window is suspended now */
		if(priv->isSuspended!=TRUE)
		{
			priv->isSuspended=TRUE;

			/* Notify about property change */
			g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesWindowContentX11Properties[PROP_SUSPENDED]);
		}
	}

	/* Check if everything went well */
	trapError=clutter_x11_untrap_x_errors();
	if(trapError!=0)
	{
		XFCE_OVERVIEW_WORKSPACES_DEBUG(self, WINDOWS,
							"X error %d occured while suspending '%s",
							trapError,
							xfce_overview_workspaces_window_tracker_window_get_name(XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW(priv->window)));
		return;
	}

	XFCE_OVERVIEW_WORKSPACES_DEBUG(self, WINDOWS,
						"Successfully suspended live texture updates for window '%s'",
						xfce_overview_workspaces_window_tracker_window_get_name(XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW(priv->window)));
}

/* Resume to handle live window updates */
static gboolean _xfce_overview_workspaces_window_content_x11_resume_on_idle(gpointer inUserData)
{
	XfceOverviewWorkspacesWindowContentX11				*self;
	XfceOverviewWorkspacesWindowContentX11Private		*priv;
	GList									*queueEntry;
	Display									*display;
	CoglContext								*context;
	GError									*error;
	gint									trapError;
	CoglTexture								*windowTexture;
	gboolean								doContinueSource;

	error=NULL;
	windowTexture=NULL;

	/* Get window content object from first entry in queue and remove it from queue */
	queueEntry=g_list_first(_xfce_overview_workspaces_window_content_x11_resume_idle_queue);
	if(!queueEntry)
	{
		g_warning("Resume handler called for empty queue.");

		/* Queue must be empty but ensure it will */
		if(_xfce_overview_workspaces_window_content_x11_resume_idle_queue)
		{
			XFCE_OVERVIEW_WORKSPACES_DEBUG(NULL, WINDOWS, "Ensuring that window content resume queue is empty");
			g_list_free(_xfce_overview_workspaces_window_content_x11_resume_idle_queue);
			_xfce_overview_workspaces_window_content_x11_resume_idle_queue=NULL;
		}

		/* Queue must be empty so remove idle source */
		_xfce_overview_workspaces_window_content_x11_resume_idle_id=0;
		return(G_SOURCE_REMOVE);
	}

	self=XFCE_OVERVIEW_WORKSPACES_WINDOW_CONTENT_X11(queueEntry->data);
	priv=self->priv;
	XFCE_OVERVIEW_WORKSPACES_DEBUG(self, WINDOWS,
						"Entering idle source with ID %u for window resume of '%s'",
						_xfce_overview_workspaces_window_content_x11_resume_idle_id,
						xfce_overview_workspaces_window_tracker_window_get_name(XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW(priv->window)));

	XFCE_OVERVIEW_WORKSPACES_DEBUG(self, WINDOWS,
						"Removing queued entry %p for window resume of '%s'",
						queueEntry,
						xfce_overview_workspaces_window_tracker_window_get_name(XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW(priv->window)));
	_xfce_overview_workspaces_window_content_x11_resume_idle_queue=g_list_delete_link(_xfce_overview_workspaces_window_content_x11_resume_idle_queue, queueEntry);
	if(_xfce_overview_workspaces_window_content_x11_resume_idle_queue)
	{
		doContinueSource=G_SOURCE_CONTINUE;
	}
		else
		{
			XFCE_OVERVIEW_WORKSPACES_DEBUG(self, WINDOWS,
								"Resume idle source with ID %u will be remove because queue is empty",
								_xfce_overview_workspaces_window_content_x11_resume_idle_id);

			doContinueSource=G_SOURCE_REMOVE;
			_xfce_overview_workspaces_window_content_x11_resume_idle_id=0;
		}


	/* We need at least the X composite extension to display images of windows
	 * if still images or live updated ones
	 */
	if(!_xfce_overview_workspaces_window_content_x11_have_composite_extension)
	{
		return(doContinueSource);
	}

	display=xfce_overview_workspaces_window_tracker_x11_get_display();
	if(G_UNLIKELY(display==None))
	{
		g_critical("No default X11 display found for resume");
	}

	/* Set up resources */
	clutter_x11_trap_x_errors();
	while(1)
	{
#ifdef HAVE_XCOMPOSITE
		/* Get pixmap to render texture for */
		priv->pixmap=XCompositeNameWindowPixmap(display, priv->xWindowID);
		XSync(display, False);
		if(priv->pixmap==None)
		{
			g_warning("Could not get pixmap for window '%s", xfce_overview_workspaces_window_tracker_window_get_name(XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW(priv->window)));

			/* Pixmap None for an inactive-workspace (still mapped)
			 * window must not become a permanent fallback. Only
			 * suspend after resume for really minimized/closed
			 * windows (minimized); otherwise keep the frozen live
			 * texture or fallback without suspending so a later
			 * resume (Map/Configure/workspace switch) retries
			 * NamePixmap. On success below isFallback=FALSE +
			 * invalidate is emitted.
			 */
			{
				gboolean pixmapIdleShouldSuspend=FALSE;

				if(!priv->window) pixmapIdleShouldSuspend=TRUE;
				else
				{
					XfceOverviewWorkspacesWindowTrackerWindowState pixmapIdleState;

					pixmapIdleState=xfce_overview_workspaces_window_tracker_window_get_state(XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW(priv->window));
					if((pixmapIdleState & (XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW_STATE_MINIMIZED))!=0)
					{
						pixmapIdleShouldSuspend=TRUE;
					}
				}

				if(pixmapIdleShouldSuspend)
				{
					/* Set flag to suspend window content after resuming because of error */
					priv->suspendAfterResumeOnIdle=TRUE;
				}
			}
			break;
		}
#else
		/* We should never get here as existance of composite extension was checked before */
		g_critical("Cannot resume window '%s' as composite extension is not available",
					xfce_overview_workspaces_window_tracker_window_get_name(XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW(priv->window)));
		break;
#endif

		/* Create cogl X11 texture for live updates */
		context=clutter_backend_get_cogl_context(clutter_get_default_backend());
		windowTexture=COGL_TEXTURE(cogl_texture_pixmap_x11_new(context, priv->pixmap, FALSE, &error));
		if(!windowTexture || error)
		{
			XfceOverviewWorkspacesWindowTrackerWindowState		textureFailState=0;
			gboolean								shouldSuspendAfterFail=FALSE;

			/* Creating texture may fail. Only suspend for really minimized
			 * windows (minimized) or closed windows. In particular
			 * do NOT suspend just because the window is not on the active
			 * workspace: windows on inactive workspaces must keep the
			 * chance for a live thumbnail (the frozen live frame or fallback stays until a later
			 * resume can retry).
			 */
			XFCE_OVERVIEW_WORKSPACES_DEBUG(self, WINDOWS,
								"Could not create texture for window '%s': %s",
								xfce_overview_workspaces_window_tracker_window_get_name(XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW(priv->window)),
								error ? error->message : "Unknown error");
			if(error)
			{
				g_error_free(error);
				error=NULL;
			}

			if(windowTexture)
			{
				cogl_object_unref(windowTexture);
				windowTexture=NULL;
			}

			if(!priv->window) shouldSuspendAfterFail=TRUE;
			else
			{
				textureFailState=xfce_overview_workspaces_window_tracker_window_get_state(XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW(priv->window));
				if((textureFailState & (XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW_STATE_MINIMIZED))!=0)
				{
					shouldSuspendAfterFail=TRUE;
				}
			}

			if(shouldSuspendAfterFail)
			{
				/* Set flag to suspend window content after resuming because of error */
				priv->suspendAfterResumeOnIdle=TRUE;
			}
				else
				{
					/* Not a minimized/closed window (e.g. window on
					 * inactive workspace): free the just-named pixmap to
					 * avoid a leak and keep the frozen live or fallback texture without
					 * suspending the live path. A later resume (Map,
					 * Configure, workspace switch) can retry.
					 */
					if(priv->pixmap!=None)
					{
						XFreePixmap(display, priv->pixmap);
						XSync(display, False);
						priv->pixmap=None;
					}
				}

			break;
		}

		/* Set up damage to get notified about changed in pixmap */
#ifdef HAVE_XDAMAGE
		if(_xfce_overview_workspaces_window_content_x11_have_damage_extension)
		{
			priv->damage=XDamageCreate(display, priv->pixmap, XDamageReportBoundingBox);
			XSync(display, False);
			if(priv->damage==None)
			{
				g_warning("Could not create damage for window '%s' - using still image of window", xfce_overview_workspaces_window_tracker_window_get_name(XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW(priv->window)));
			}
		}
#endif

		/* Release old texture (should be the fallback texture) and set new texture */
		if(priv->texture)
		{
			cogl_object_unref(priv->texture);
			priv->texture=windowTexture;
		}

		/* Set damage to new window texture */
#ifdef HAVE_XDAMAGE
		if(_xfce_overview_workspaces_window_content_x11_have_damage_extension &&
			priv->damage!=None)
		{
			cogl_texture_pixmap_x11_set_damage_object(COGL_TEXTURE_PIXMAP_X11(priv->texture), priv->damage, COGL_TEXTURE_PIXMAP_X11_DAMAGE_BOUNDING_BOX);
		}
#endif

		/* Now we use the window as texture and not the fallback texture anymore */
		priv->isFallback=FALSE;

		/* Window is not suspended anymore */
		if(priv->isSuspended!=FALSE)
		{
			priv->isSuspended=FALSE;

			/* Notify about property change */
			g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesWindowContentX11Properties[PROP_SUSPENDED]);
		}

		/* Live capture succeeded: persist frozen frame (best effort).
		 * Uses priv->xWindowID already set for this content.
		 */
		xfce_overview_workspaces_window_content_frozen_save(priv->isFallback, priv->texture, priv->xWindowID, priv->window, &priv->frozenCacheXID);

		/* Invalidate content to get it redrawn as soon as possible */
		clutter_content_invalidate(CLUTTER_CONTENT(self));

		/* We were able to set up window content so this window is definitely mapped */
		priv->isMapped=TRUE;

		/* End reached so break to get out of while loop */
		break;
	}

	/* Check if window content should be suspended again after resume was done,
	 * e.g. initial window content creation in suspended daemon mode.
	 */
	if(priv->suspendAfterResumeOnIdle)
	{
		_xfce_overview_workspaces_window_content_x11_suspend(self);
		priv->suspendAfterResumeOnIdle=FALSE;
	}

	/* Check if everything went well */
	trapError=clutter_x11_untrap_x_errors();
	if(trapError!=0)
	{
		XFCE_OVERVIEW_WORKSPACES_DEBUG(self, WINDOWS,
							"X error %d occured while resuming window '%s",
							trapError,
							xfce_overview_workspaces_window_tracker_window_get_name(XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW(priv->window)));
		return(doContinueSource);
	}

	XFCE_OVERVIEW_WORKSPACES_DEBUG(self, WINDOWS,
						"Resuming live texture updates for window '%s'",
						xfce_overview_workspaces_window_tracker_window_get_name(XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW(priv->window)));
	return(doContinueSource);
}

static void _xfce_overview_workspaces_window_content_x11_resume(XfceOverviewWorkspacesWindowContentX11 *self)
{
	XfceOverviewWorkspacesWindowContentX11Private		*priv;
	Display									*display;
	CoglContext								*context;
	GError									*error;
	gint									trapError;
	CoglTexture								*windowTexture;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_CONTENT_X11(self));
	g_return_if_fail(self->priv->window);

	priv=self->priv;
	error=NULL;
	windowTexture=NULL;

	/* Check if to use new experimental code to resume window content
	 * in an idle source.
	 */
	if(_xfce_overview_workspaces_window_content_x11_window_creation_priority>0)
	{
		_xfce_overview_workspaces_window_content_x11_resume_on_idle_add(self);
		return;
	}

	/* Even with "immediate" creation priority, do not create the live
	 * thumbnail (XCompositeNameWindowPixmap + XSync + texture + damage)
	 * synchronously for really offscreen windows: minimized or
	 * paged-out windows (MINIMIZED|SKIP_PAGER|SKIP_TASKLIST).
	 * Windows on inactive workspaces (HIDDEN without MINIMIZED, still
	 * logically mapped) must still attempt synchronous capture
	 * (NamePixmap + Damage) like windows on the active workspace,
	 * so LiveWorkspace actors for all workspaces show live thumbnails
	 * (GNOME style). Only truly offscreen windows get the fallback icon
	 * texture now and the live texture from the idle queue. Visible
	 * windows keep the synchronous path, so the first frame is unchanged.
	 */
	if(priv->windowTracker)
	{
		XfceOverviewWorkspacesWindowTrackerWindowState		state;

		state=xfce_overview_workspaces_window_tracker_window_get_state(XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW(priv->window));

		if((state & (XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW_STATE_MINIMIZED |
						XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW_STATE_SKIP_PAGER |
						XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW_STATE_SKIP_TASKLIST))!=0)
		{
			_xfce_overview_workspaces_window_content_x11_resume_on_idle_add(self);
			return;
		}
	}

	/* We need at least the X composite extension to display images of windows
	 * if still images or live updated ones
	 */
	if(!_xfce_overview_workspaces_window_content_x11_have_composite_extension) return;

	/* If a live (non-fallback) frame exists for a window on an inactive
	 * workspace (HIDDEN without MINIMIZED), keep the frozen photo visible:
	 * do not touch the visible texture and only queue an idle re-capture
	 * which retries without degrading it to the fallback icon.
	 */
	if(priv->texture &&
		!priv->isFallback &&
		priv->window)
	{
		XfceOverviewWorkspacesWindowTrackerWindowState	frozenState;

		frozenState=xfce_overview_workspaces_window_tracker_window_get_state(XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW(priv->window));
		if((frozenState & XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW_STATE_HIDDEN)!=0 &&
			(frozenState & XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW_STATE_MINIMIZED)==0)
		{
			_xfce_overview_workspaces_window_content_x11_resume_on_idle_add(self);
			return;
		}
	}

	display=xfce_overview_workspaces_window_tracker_x11_get_display();
	if(G_UNLIKELY(display==None))
	{
		g_critical("No default X11 display found for resume");
	}

	/* Set up resources */
	clutter_x11_trap_x_errors();
	while(1)
	{
#ifdef HAVE_XCOMPOSITE
		/* Get pixmap to render texture for */
		priv->pixmap=XCompositeNameWindowPixmap(display, priv->xWindowID);
		XSync(display, False);
		if(priv->pixmap==None)
		{
			g_warning("Could not get pixmap for window '%s", xfce_overview_workspaces_window_tracker_window_get_name(XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW(priv->window)));
			/* Pixmap None happens for unmapped windows, e.g. a mapped
			 * window on an inactive workspace (Unmap by WM) where
			 * NameWindowPixmap has no content yet. Do not fall back
			 * permanently: only suspend for really gone windows
			 * (minimized/closed), otherwise retry via idle and
			 * keep the frozen live or fallback texture until a later resume succeeds.
			 * On success below isFallback=FALSE + invalidate is emitted.
			 */
			{
				gboolean pixmapFailShouldSuspend=FALSE;

				if(!priv->window) pixmapFailShouldSuspend=TRUE;
				else
				{
					XfceOverviewWorkspacesWindowTrackerWindowState pixmapFailState;

					pixmapFailState=xfce_overview_workspaces_window_tracker_window_get_state(XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW(priv->window));
					if((pixmapFailState & (XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW_STATE_MINIMIZED))!=0)
					{
						pixmapFailShouldSuspend=TRUE;
					}
				}

				if(pixmapFailShouldSuspend)
				{
					_xfce_overview_workspaces_window_content_x11_suspend(self);
				}
					else
					{
						_xfce_overview_workspaces_window_content_x11_resume_on_idle_add(self);
					}
			}
			break;
		}
#else
		/* We should never get here as existance of composite extension was checked before */
		g_critical("Cannot resume window '%s' as composite extension is not available",
					xfce_overview_workspaces_window_tracker_window_get_name(XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW(priv->window)));
		break;
#endif

		/* Create cogl X11 texture for live updates */
		context=clutter_backend_get_cogl_context(clutter_get_default_backend());
		windowTexture=COGL_TEXTURE(cogl_texture_pixmap_x11_new(context, priv->pixmap, FALSE, &error));
		if(!windowTexture || error)
		{
			XfceOverviewWorkspacesWindowTrackerWindowState		textureFailState=0;
			gboolean								shouldSuspendAfterFail=FALSE;

			/* Creating texture may fail. Only suspend for really minimized
			 * windows (minimized) or closed windows. In particular
			 * do NOT suspend just because the window is not on the active
			 * workspace: windows on inactive workspaces must keep the
			 * chance for a live thumbnail.
			 */
			XFCE_OVERVIEW_WORKSPACES_DEBUG(self, WINDOWS,
								"Could not create texture for window '%s': %s",
								xfce_overview_workspaces_window_tracker_window_get_name(XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW(priv->window)),
								error ? error->message : "Unknown error");
			if(error)
			{
				g_error_free(error);
				error=NULL;
			}

			if(windowTexture)
			{
				cogl_object_unref(windowTexture);
				windowTexture=NULL;
			}

			if(!priv->window) shouldSuspendAfterFail=TRUE;
			else
			{
				textureFailState=xfce_overview_workspaces_window_tracker_window_get_state(XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW(priv->window));
				if((textureFailState & (XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW_STATE_MINIMIZED))!=0)
				{
					shouldSuspendAfterFail=TRUE;
				}
			}

			if(shouldSuspendAfterFail)
			{
				_xfce_overview_workspaces_window_content_x11_suspend(self);
			}
				else
				{
					/* Not a minimized/closed window (e.g. window on
					 * inactive workspace): free the just-named pixmap to
					 * avoid a leak and keep the frozen live or fallback texture without
					 * suspending the live path. A later resume can retry.
					 */
					if(priv->pixmap!=None)
					{
						XFreePixmap(display, priv->pixmap);
						XSync(display, False);
						priv->pixmap=None;
					}
				}

			break;
		}

		/* Set up damage to get notified about changed in pixmap */
#ifdef HAVE_XDAMAGE
		if(_xfce_overview_workspaces_window_content_x11_have_damage_extension)
		{
			priv->damage=XDamageCreate(display, priv->pixmap, XDamageReportBoundingBox);
			XSync(display, False);
			if(priv->damage==None)
			{
				g_warning("Could not create damage for window '%s' - using still image of window", xfce_overview_workspaces_window_tracker_window_get_name(XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW(priv->window)));
			}
		}
#endif

		/* Release old texture (should be the fallback texture) and set new texture */
		if(priv->texture)
		{
			cogl_object_unref(priv->texture);
			priv->texture=windowTexture;
		}

		/* Set damage to new window texture */
#ifdef HAVE_XDAMAGE
		if(_xfce_overview_workspaces_window_content_x11_have_damage_extension &&
			priv->damage!=None)
		{
			cogl_texture_pixmap_x11_set_damage_object(COGL_TEXTURE_PIXMAP_X11(priv->texture), priv->damage, COGL_TEXTURE_PIXMAP_X11_DAMAGE_BOUNDING_BOX);
		}
#endif

		/* Now we use the window as texture and not the fallback texture anymore */
		priv->isFallback=FALSE;

		/* Window is not suspended anymore */
		if(priv->isSuspended!=FALSE)
		{
			priv->isSuspended=FALSE;

			/* Notify about property change */
			g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesWindowContentX11Properties[PROP_SUSPENDED]);
		}

		/* Live capture succeeded: persist frozen frame (best effort).
		 * Uses priv->xWindowID already set for this content.
		 */
		xfce_overview_workspaces_window_content_frozen_save(priv->isFallback, priv->texture, priv->xWindowID, priv->window, &priv->frozenCacheXID);

		/* Invalidate content to get it redrawn as soon as possible */
		clutter_content_invalidate(CLUTTER_CONTENT(self));

		/* We were able to set up window content so this window is definitely mapped */
		priv->isMapped=TRUE;

		/* End reached so break to get out of while loop */
		break;
	}

	/* Check if everything went well */
	trapError=clutter_x11_untrap_x_errors();
	if(trapError!=0)
	{
		XFCE_OVERVIEW_WORKSPACES_DEBUG(self, WINDOWS,
							"X error %d occured while resuming window '%s",
							trapError,
							xfce_overview_workspaces_window_tracker_window_get_name(XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW(priv->window)));
		return;
	}

	XFCE_OVERVIEW_WORKSPACES_DEBUG(self, WINDOWS,
						"Resuming live texture updates for window '%s'",
						xfce_overview_workspaces_window_tracker_window_get_name(XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW(priv->window)));
}

/* Find X window for window frame of given X window content */
static Window _xfce_overview_workspaces_window_content_x11_get_window_frame_xid(Display *inDisplay,
																	XfceOverviewWorkspacesWindowTrackerWindowX11 *inWindow)
{
	Window				xWindowID;
	Window				iterXWindowID;
	Window				rootXWindowID;
	Window				foundXWindowID;
	GdkDisplay			*gdkDisplay;
	GdkWindow			*gdkWindow;
	GdkWMDecoration		gdkWindowDecoration;

	g_return_val_if_fail(inDisplay, 0);
	g_return_val_if_fail(inWindow, 0);

	/* Get X window */
	xWindowID=xfce_overview_workspaces_window_tracker_window_x11_get_xid(inWindow);
	g_return_val_if_fail(xWindowID!=0, 0);

	/* Check if window is client side decorated and if it has no decorations
	 * so skip finding window frame and behave like we did not found it.
	 */
	XSync(inDisplay, False);

	gdkDisplay=gdk_x11_lookup_xdisplay(inDisplay);
	if(!gdkDisplay) gdkDisplay=gdk_display_get_default();

	gdkWindow=gdk_x11_window_foreign_new_for_display(gdkDisplay, xWindowID);
	if(gdkWindow)
	{
		if(!gdk_window_get_decorations(gdkWindow, &gdkWindowDecoration) ||
			gdkWindowDecoration==0)
		{
			XFCE_OVERVIEW_WORKSPACES_DEBUG(inWindow, WINDOWS,
								"Window '%s' has either CSD not enabled or no decorations applied so skip finding window frame",
								xfce_overview_workspaces_window_tracker_window_get_name(XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW(inWindow)));

			/* Release allocated resources */
			g_object_unref(gdkWindow);

			/* Skip finding window frame and return "not-found" result */
			return(0);
		}
		g_object_unref(gdkWindow);
	}
		else
		{
			XFCE_OVERVIEW_WORKSPACES_DEBUG(inWindow, WINDOWS,
								"Could not get window decoration from window '%s'",
								xfce_overview_workspaces_window_tracker_window_get_name(XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW(inWindow)));
		}

	/* Iterate through X window tree list upwards until root window reached.
	 * The last X window before root window is the one we are looking for.
	 */
	rootXWindowID=0;
	foundXWindowID=0;
	for(iterXWindowID=xWindowID; iterXWindowID && iterXWindowID!=rootXWindowID; )
	{
		Window	*children;
		guint	numberChildren;

		children=NULL;
		numberChildren=0;
		foundXWindowID=iterXWindowID;
		if(!XQueryTree(inDisplay, iterXWindowID, &rootXWindowID, &iterXWindowID, &children, &numberChildren))
		{
			iterXWindowID=0;
		}
		if(children) XFree(children);
	}

	/* Return found X window ID */
	return(foundXWindowID);
}

/* Window's was closed */
static void _xfce_overview_workspaces_window_content_x11_on_window_closed(XfceOverviewWorkspacesWindowContentX11 *self,
																gpointer inUserData)
{
	XfceOverviewWorkspacesWindowContentX11Private	*priv;
	XfceOverviewWorkspacesWindowTrackerWindow		*window G_GNUC_UNUSED;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_CONTENT_X11(self));
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_WINDOW(inUserData));

	priv=self->priv;
	window=XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW(inUserData);

	/* X11 window was closed so suspend this window content
	 * to keep current texture of window and prevent futher
	 * live updates.
	 */
	_xfce_overview_workspaces_window_content_x11_suspend(self);

	/* Drop stale frozen frame: XID may be reused by a new window */
	if(priv->xWindowID!=None)
	{
		xfce_overview_workspaces_thumbnail_cache_invalidate_xid((gulong)priv->xWindowID);
	}
	if(priv->window)
	{
		xfce_overview_workspaces_thumbnail_cache_invalidate_xid((gulong)xfce_overview_workspaces_window_tracker_window_x11_get_xid(priv->window));
	}

	/* Disconnect all signal handler from window as it was closed */
	if(priv->windowClosedSignalID)
	{
		g_signal_handler_disconnect(priv->window, priv->windowClosedSignalID);
		priv->windowClosedSignalID=0;
	}

	g_signal_handlers_disconnect_by_data(priv->window, self);

	/* libwnck resources should never be freed. Just set to NULL */
	priv->window=NULL;
}

/* Set window to handle and to display */
static void _xfce_overview_workspaces_window_content_x11_set_window(XfceOverviewWorkspacesWindowContentX11 *self, XfceOverviewWorkspacesWindowTrackerWindowX11 *inWindow)
{
	XfceOverviewWorkspacesWindowContentX11Private		*priv;
	XfceOverviewWorkspacesCore							*core;
	Display									*display;
	GdkPixbuf								*windowIcon;
	XWindowAttributes						windowAttrs;
	ClutterBackend							*backend;
	CoglContext								*context;
	CoglError								*error;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_CONTENT_X11(self));
	g_return_if_fail(inWindow!=NULL && XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_WINDOW_X11(inWindow));
	g_return_if_fail(self->priv->window==NULL);
	g_return_if_fail(self->priv->xWindowID==0);
	g_return_if_fail(self->priv->windowClosedSignalID==0);

	priv=self->priv;

	/* Freeze notifications and collect them */
	g_object_freeze_notify(G_OBJECT(self));

	display=xfce_overview_workspaces_window_tracker_x11_get_display();
	if(G_UNLIKELY(display==None))
	{
		g_critical("No default X11 display found to set window for clutter content");
	}

	/* Set new value */
	priv->window=inWindow;

	/* Connect signal handler to get notified if window was closed
	 * to suspend this window content.
	 */
	priv->windowClosedSignalID=g_signal_connect_swapped(priv->window,
														"closed",
														G_CALLBACK(_xfce_overview_workspaces_window_content_x11_on_window_closed),
														self);

	/* Create fallback texture first in case we cannot create
	 * a live updated texture for window in the next steps
	 */
	windowIcon=xfce_overview_workspaces_window_tracker_window_get_icon(XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW(priv->window));
	error=NULL;

	backend=clutter_get_default_backend();
	context=clutter_backend_get_cogl_context(backend);
	priv->texture=cogl_texture_2d_new_from_data(context,
												gdk_pixbuf_get_width(windowIcon),
												gdk_pixbuf_get_height(windowIcon),
												gdk_pixbuf_get_has_alpha(windowIcon) ? COGL_PIXEL_FORMAT_RGBA_8888 : COGL_PIXEL_FORMAT_RGB_888,
												gdk_pixbuf_get_rowstride(windowIcon),
												gdk_pixbuf_get_pixels(windowIcon),
												&error);

	if(!priv->texture || error)
	{
		g_warning("Could not create fallback texture for window '%s': %s",
					xfce_overview_workspaces_window_tracker_window_get_name(XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW(priv->window)),
					(error && error->message) ? error->message : "Unknown error");

		/* Release allocated resources */
		if(priv->texture)
		{
			cogl_object_unref(priv->texture);
			priv->texture=NULL;
		}

		if(error)
		{
			cogl_error_free(error);
			error=NULL;
		}
	}

	priv->isFallback=TRUE;

	/* Get X window and its attributes */
	if(priv->includeWindowFrame)
	{
		priv->xWindowID=_xfce_overview_workspaces_window_content_x11_get_window_frame_xid(display, priv->window);
	}

	if(!priv->xWindowID)
	{
		priv->xWindowID=xfce_overview_workspaces_window_tracker_window_x11_get_xid(priv->window);
	}

	if(!XGetWindowAttributes(display, priv->xWindowID, &windowAttrs))
	{
		g_warning("Could not get attributes of window '%s'", xfce_overview_workspaces_window_tracker_window_get_name(XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW(priv->window)));
		XSync(display, False);
	}

	/* We need at least the X composite extension to display images of windows
	 * if still images or live updated ones by redirecting window
	 */
#ifdef HAVE_XCOMPOSITE
	if(_xfce_overview_workspaces_window_content_x11_have_composite_extension)
	{
		/* Redirect window */
		XCompositeRedirectWindow(display, priv->xWindowID, CompositeRedirectAutomatic);
		XSync(display, False);
	}
#endif

	/* We are interested in receiving mapping events of windows */
	XSelectInput(display, priv->xWindowID, windowAttrs.your_event_mask | StructureNotifyMask);

	/* Acquire new window and handle live updates */
	_xfce_overview_workspaces_window_content_x11_resume(self);
	priv->isMapped=!priv->isSuspended;

	/* But suspend window immediately again if core is suspended */
	core=xfce_overview_workspaces_core_get_default();
	if(xfce_overview_workspaces_core_is_suspended(core))
	{
		if(_xfce_overview_workspaces_window_content_x11_window_creation_priority>0)
		{
			priv->suspendAfterResumeOnIdle=TRUE;
		}
			else
			{
				_xfce_overview_workspaces_window_content_x11_suspend(self);
			}
	}

	/* Notify about property change */
	g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesWindowContentX11Properties[PROP_WINDOW]);

	/* Thaw notifications and send them now */
	g_object_thaw_notify(G_OBJECT(self));

	/* Set up workaround mechanism for unmapped windows if wanted and needed */
	_xfce_overview_workspaces_window_content_x11_setup_workaround(self, inWindow);
}


/* IMPLEMENTATION: ClutterContent */

/* Paint texture */
static void _xdashboard_window_content_clutter_content_iface_paint_content(ClutterContent *inContent,
																			ClutterActor *inActor,
																			ClutterPaintNode *inRootNode)
{
	XfceOverviewWorkspacesWindowContentX11				*self=XFCE_OVERVIEW_WORKSPACES_WINDOW_CONTENT_X11(inContent);
	XfceOverviewWorkspacesWindowContentX11Private		*priv=self->priv;
	ClutterScalingFilter					minFilter, magFilter;
	ClutterPaintNode						*node;
	ClutterActorBox							textureAllocationBox;
	ClutterActorBox							textureCoordBox;
	ClutterActorBox							outlineBox;
	ClutterColor							color;
	guint8									opacity;
	ClutterColor							outlineColor;
	ClutterActorBox							outlinePath;

	/* Check if we have a texture to paint */
	if(priv->texture==NULL) return;

	/* Get needed data for painting */
	clutter_actor_box_init(&textureCoordBox, 0.0f, 0.0f, 1.0f, 1.0f);
	clutter_actor_get_content_box(inActor, &textureAllocationBox);
	clutter_actor_get_content_box(inActor, &outlineBox);
	clutter_actor_get_content_scaling_filters(inActor, &minFilter, &magFilter);
	opacity=clutter_actor_get_paint_opacity(inActor);

	color.red=opacity;
	color.green=opacity;
	color.blue=opacity;
	color.alpha=opacity;

	/* Draw background if texture is a fallback */
	if(priv->isFallback)
	{
		ClutterColor					backgroundColor;

		/* Set up background color */
		backgroundColor.red=0;
		backgroundColor.green=0;
		backgroundColor.blue=0;
		backgroundColor.alpha=opacity;

		/* Draw background */
		node=clutter_color_node_new(&backgroundColor);
		clutter_paint_node_set_name(node, "fallback-background");
		clutter_paint_node_add_rectangle(node, &outlineBox);
		clutter_paint_node_add_child(inRootNode, node);
		clutter_paint_node_unref(node);
	}

	/* Determine actor box allocation to draw texture into when unmapped window
	 * icon (fallback) will be drawn. We can skip calculation if unmapped window
	 * icon should be expanded in both (x and y) direction.
	 */
	if(priv->isFallback &&
		(!priv->unmappedWindowIconXFill || !priv->unmappedWindowIconYFill))
	{
		gfloat							allocationWidth;
		gfloat							allocationHeight;

		/* Get width and height of allocation */
		allocationWidth=(outlineBox.x2-outlineBox.x1);
		allocationHeight=(outlineBox.y2-outlineBox.y1);

		/* Determine left and right boundary of unmapped window icon
		 * if unmapped window icon should not expand in X axis.
		 */
		if(!priv->unmappedWindowIconXFill)
		{
			gfloat						offset;
			gfloat						textureWidth;
			gfloat						oversize;

			/* Get scaled width of unmapped window icon */
			textureWidth=cogl_texture_get_width(priv->texture);
			textureWidth*=priv->unmappedWindowIconXScale;

			/* Get boundary in X axis depending on gravity and scaled width */
			offset=(priv->unmappedWindowIconXAlign*allocationWidth);
			switch(priv->unmappedWindowIconAnchorPoint)
			{
				/* Align to left boundary.
				 * This is also the default if gravity is none or undefined.
				 */
				default:
				case XFCE_OVERVIEW_WORKSPACES_ANCHOR_POINT_NONE:
				case XFCE_OVERVIEW_WORKSPACES_ANCHOR_POINT_WEST:
				case XFCE_OVERVIEW_WORKSPACES_ANCHOR_POINT_NORTH_WEST:
				case XFCE_OVERVIEW_WORKSPACES_ANCHOR_POINT_SOUTH_WEST:
					break;

				/* Align to center of X axis */
				case XFCE_OVERVIEW_WORKSPACES_ANCHOR_POINT_CENTER:
				case XFCE_OVERVIEW_WORKSPACES_ANCHOR_POINT_NORTH:
				case XFCE_OVERVIEW_WORKSPACES_ANCHOR_POINT_SOUTH:
					offset-=(textureWidth/2.0f);
					break;

				/* Align to right boundary */
				case XFCE_OVERVIEW_WORKSPACES_ANCHOR_POINT_EAST:
				case XFCE_OVERVIEW_WORKSPACES_ANCHOR_POINT_NORTH_EAST:
				case XFCE_OVERVIEW_WORKSPACES_ANCHOR_POINT_SOUTH_EAST:
					offset-=textureWidth;
					break;
			}

			/* Set boundary in X axis */
			textureAllocationBox.x1=outlineBox.x1+offset;
			textureAllocationBox.x2=textureAllocationBox.x1+textureWidth;

			/* Clip texture in X axis if it does not fit into allocation */
			if(textureAllocationBox.x1<outlineBox.x1)
			{
				oversize=outlineBox.x1-textureAllocationBox.x1;
				textureCoordBox.x1=oversize/textureWidth;
				textureAllocationBox.x1=outlineBox.x1;
			}

			if(textureAllocationBox.x2>outlineBox.x2)
			{
				oversize=textureAllocationBox.x2-outlineBox.x2;
				textureCoordBox.x2=1.0f-(oversize/textureWidth);
				textureAllocationBox.x2=outlineBox.x2;
			}
		}

		/* Determine left and right boundary of unmapped window icon
		 * if unmapped window icon should not expand in X axis.
		 */
		if(!priv->unmappedWindowIconYFill)
		{
			gfloat						offset;
			gfloat						textureHeight;
			gfloat						oversize;

			/* Get scaled width of unmapped window icon */
			textureHeight=cogl_texture_get_height(priv->texture);
			textureHeight*=priv->unmappedWindowIconYScale;

			/* Get boundary in Y axis depending on gravity and scaled width */
			offset=(priv->unmappedWindowIconYAlign*allocationHeight);
			switch(priv->unmappedWindowIconAnchorPoint)
			{
				/* Align to upper boundary.
				 * This is also the default if gravity is none or undefined.
				 */
				default:
				case XFCE_OVERVIEW_WORKSPACES_ANCHOR_POINT_NONE:
				case XFCE_OVERVIEW_WORKSPACES_ANCHOR_POINT_NORTH:
				case XFCE_OVERVIEW_WORKSPACES_ANCHOR_POINT_NORTH_WEST:
				case XFCE_OVERVIEW_WORKSPACES_ANCHOR_POINT_NORTH_EAST:
					break;

				/* Align to center of Y axis */
				case XFCE_OVERVIEW_WORKSPACES_ANCHOR_POINT_CENTER:
				case XFCE_OVERVIEW_WORKSPACES_ANCHOR_POINT_WEST:
				case XFCE_OVERVIEW_WORKSPACES_ANCHOR_POINT_EAST:
					offset-=(textureHeight/2.0f);
					break;

				/* Align to lower boundary */
				case XFCE_OVERVIEW_WORKSPACES_ANCHOR_POINT_SOUTH:
				case XFCE_OVERVIEW_WORKSPACES_ANCHOR_POINT_SOUTH_WEST:
				case XFCE_OVERVIEW_WORKSPACES_ANCHOR_POINT_SOUTH_EAST:
					offset-=textureHeight;
					break;
			}

			/* Set boundary in Y axis */
			textureAllocationBox.y1=outlineBox.y1+offset;
			textureAllocationBox.y2=textureAllocationBox.y1+textureHeight;

			/* Clip texture in Y axis if it does not fit into allocation */
			if(textureAllocationBox.y1<outlineBox.y1)
			{
				oversize=outlineBox.y1-textureAllocationBox.y1;
				textureCoordBox.y1=oversize/textureHeight;
				textureAllocationBox.y1=outlineBox.y1;
			}

			if(textureAllocationBox.y2>outlineBox.y2)
			{
				oversize=textureAllocationBox.y2-outlineBox.y2;
				textureCoordBox.y2=1.0f-(oversize/textureHeight);
				textureAllocationBox.y2=outlineBox.y2;
			}
		}
	}

	/* Set up paint nodes for texture */
	node=clutter_texture_node_new(priv->texture, &color, minFilter, magFilter);
	clutter_paint_node_set_name(node, G_OBJECT_TYPE_NAME(self));
	clutter_paint_node_add_texture_rectangle(node,
												&textureAllocationBox,
												textureCoordBox.x1,
												textureCoordBox.y1,
												textureCoordBox.x2,
												textureCoordBox.y2);
	clutter_paint_node_add_child(inRootNode, node);
	clutter_paint_node_unref(node);

	/* Draw outline (color is depending is texture is fallback or not.
	 * That should be done last to get outline always visible
	 */
	if(priv->isFallback || priv->outlineColor==NULL)
	{
		outlineColor.red=0xff;
		outlineColor.green=0xff;
		outlineColor.blue=0xff;
		outlineColor.alpha=opacity;
	}
		else
		{
			outlineColor.red=priv->outlineColor->red;
			outlineColor.green=priv->outlineColor->green;
			outlineColor.blue=priv->outlineColor->blue;
			outlineColor.alpha=opacity;
		}

	node=clutter_color_node_new(&outlineColor);
	clutter_paint_node_set_name(node, "outline-top");
	clutter_actor_box_init_rect(&outlinePath, outlineBox.x1, 0.0f, outlineBox.x2-outlineBox.x1, priv->outlineWidth);
	clutter_paint_node_add_rectangle(node, &outlinePath);
	clutter_paint_node_add_child(inRootNode, node);
	clutter_paint_node_unref(node);

	node=clutter_color_node_new(&outlineColor);
	clutter_paint_node_set_name(node, "outline-bottom");
	clutter_actor_box_init_rect(&outlinePath, outlineBox.x1, outlineBox.y2-priv->outlineWidth, outlineBox.x2-outlineBox.x1, priv->outlineWidth);
	clutter_paint_node_add_rectangle(node, &outlinePath);
	clutter_paint_node_add_child(inRootNode, node);
	clutter_paint_node_unref(node);

	node=clutter_color_node_new(&outlineColor);
	clutter_paint_node_set_name(node, "outline-left");
	clutter_actor_box_init_rect(&outlinePath, outlineBox.x1, outlineBox.y1, priv->outlineWidth, outlineBox.y2-outlineBox.y1);
	clutter_paint_node_add_rectangle(node, &outlinePath);
	clutter_paint_node_add_child(inRootNode, node);
	clutter_paint_node_unref(node);

	node=clutter_color_node_new(&outlineColor);
	clutter_paint_node_set_name(node, "outline-right");
	clutter_actor_box_init_rect(&outlinePath, outlineBox.x2-priv->outlineWidth, outlineBox.y1, priv->outlineWidth, outlineBox.y2-outlineBox.y1);
	clutter_paint_node_add_rectangle(node, &outlinePath);
	clutter_paint_node_add_child(inRootNode, node);
	clutter_paint_node_unref(node);
}

/* Get preferred size of texture */
static gboolean _xdashboard_window_content_clutter_content_iface_get_preferred_size(ClutterContent *inContent,
																					gfloat *outWidth,
																					gfloat *outHeight)
{
	XfceOverviewWorkspacesWindowContentX11Private		*priv=XFCE_OVERVIEW_WORKSPACES_WINDOW_CONTENT_X11(inContent)->priv;
	gfloat									w, h;

	/* No texture - no size to retrieve */
	if(priv->texture==NULL) return(FALSE);

	/* If window is suspended or if we use the fallback image
	 * get real window size ...
	 */
	if(priv->isFallback || priv->isSuspended)
	{
		/* Is a fallback texture so get real window size */
		gint							windowW, windowH;

		xfce_overview_workspaces_window_tracker_window_get_geometry(XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW(priv->window), NULL, NULL, &windowW, &windowH);
		w=windowW;
		h=windowH;
	}
		else
		{
			/* ... otherwise get size of texture */
			w=cogl_texture_get_width(priv->texture);
			h=cogl_texture_get_height(priv->texture);
		}

	/* Set result values */
	if(outWidth) *outWidth=w;
	if(outHeight) *outHeight=h;

	return(TRUE);
}

/* Initialize interface of type ClutterContent */
static void _xfce_overview_workspaces_window_content_clutter_content_iface_init(ClutterContentIface *iface)
{
	iface->get_preferred_size=_xdashboard_window_content_clutter_content_iface_get_preferred_size;
	iface->paint_content=_xdashboard_window_content_clutter_content_iface_paint_content;
}

/* IMPLEMENTATION: Interface XfceOverviewWorkspacesStylable */

/* Get stylable properties of stage */
static void _xfce_overview_workspaces_window_content_x11_stylable_get_stylable_properties(XfceOverviewWorkspacesStylable *self,
																			GHashTable *ioStylableProperties)
{
	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_STYLABLE(self));

	/* Add stylable properties to hashtable */
	xfce_overview_workspaces_stylable_add_stylable_property(self, ioStylableProperties, "include-window-frame");
	xfce_overview_workspaces_stylable_add_stylable_property(self, ioStylableProperties, "unmapped-window-icon-x-fill");
	xfce_overview_workspaces_stylable_add_stylable_property(self, ioStylableProperties, "unmapped-window-icon-y-fill");
	xfce_overview_workspaces_stylable_add_stylable_property(self, ioStylableProperties, "unmapped-window-icon-x-align");
	xfce_overview_workspaces_stylable_add_stylable_property(self, ioStylableProperties, "unmapped-window-icon-y-align");
	xfce_overview_workspaces_stylable_add_stylable_property(self, ioStylableProperties, "unmapped-window-icon-x-scale");
	xfce_overview_workspaces_stylable_add_stylable_property(self, ioStylableProperties, "unmapped-window-icon-y-scale");
	xfce_overview_workspaces_stylable_add_stylable_property(self, ioStylableProperties, "unmapped-window-icon-anchor-point");
}

/* Get/set style classes of stage */
static const gchar* _xfce_overview_workspaces_window_content_x11_stylable_get_classes(XfceOverviewWorkspacesStylable *inStylable)
{
	/* Not implemented */
	return(NULL);
}

static void _xfce_overview_workspaces_window_content_x11_stylable_set_classes(XfceOverviewWorkspacesStylable *inStylable, const gchar *inStyleClasses)
{
	/* Not implemented */
}

/* Get/set style pseudo-classes of stage */
static const gchar* _xfce_overview_workspaces_window_content_x11_stylable_get_pseudo_classes(XfceOverviewWorkspacesStylable *inStylable)
{
	/* Not implemented */
	return(NULL);
}

static void _xfce_overview_workspaces_window_content_x11_stylable_set_pseudo_classes(XfceOverviewWorkspacesStylable *inStylable, const gchar *inStylePseudoClasses)
{
	/* Not implemented */
}

/* Interface initialization
 * Set up default functions
 */
void _xfce_overview_workspaces_window_content_x11_stylable_iface_init(XfceOverviewWorkspacesStylableInterface *iface)
{
	iface->get_stylable_properties=_xfce_overview_workspaces_window_content_x11_stylable_get_stylable_properties;
	iface->get_classes=_xfce_overview_workspaces_window_content_x11_stylable_get_classes;
	iface->set_classes=_xfce_overview_workspaces_window_content_x11_stylable_set_classes;
	iface->get_pseudo_classes=_xfce_overview_workspaces_window_content_x11_stylable_get_pseudo_classes;
	iface->set_pseudo_classes=_xfce_overview_workspaces_window_content_x11_stylable_set_pseudo_classes;
}

/* IMPLEMENTATION: GObject */

/* Dispose this object */
static void _xfce_overview_workspaces_window_content_x11_dispose(GObject *inObject)
{
	XfceOverviewWorkspacesWindowContentX11				*self=XFCE_OVERVIEW_WORKSPACES_WINDOW_CONTENT_X11(inObject);
	XfceOverviewWorkspacesWindowContentX11Private		*priv=self->priv;

	/* Dispose allocated resources */
#ifdef CLUTTER_WINDOWING_X11
	if(clutter_check_windowing_backend(CLUTTER_WINDOWING_X11))
	{
		clutter_x11_remove_filter(_xfce_overview_workspaces_window_content_x11_on_x_event, self);
	}
#endif

#ifdef CLUTTER_WINDOWING_GDK
	if(clutter_check_windowing_backend(CLUTTER_WINDOWING_GDK))
	{
		gdk_window_remove_filter(NULL, _xfce_overview_workspaces_window_content_x11_on_gdkx_event, self);
	}
#endif

	_xfce_overview_workspaces_window_content_x11_release_resources(self);

	if(priv->workaroundStateSignalID)
	{
		g_signal_handler_disconnect(priv->windowTracker, priv->workaroundStateSignalID);
		priv->workaroundStateSignalID=0;

		/* This signal was still connected to the window tracker so the window may be unminized
		 * and need to ensure it is minimized again. We need to do this now, before we release
		 * our handle to the window (priv->window).
		 */
		xfce_overview_workspaces_window_tracker_window_hide(XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW(priv->window));
	}

	if(priv->windowTracker)
	{
		g_signal_handlers_disconnect_by_data(priv->windowTracker, self);
		g_object_unref(priv->windowTracker);
		priv->windowTracker=NULL;
	}

	if(priv->window)
	{
		/* Disconnect signal handler to get notified when window is closed */
		if(priv->windowClosedSignalID)
		{
			g_signal_handler_disconnect(priv->window, priv->windowClosedSignalID);
			priv->windowClosedSignalID=0;
		}

		/* Disconnect signals */
		g_signal_handlers_disconnect_by_data(priv->window, self);

		/* libwnck resources should never be freed. Just set to NULL */
		priv->window=NULL;
	}

	if(priv->suspendSignalID)
	{
		g_signal_handler_disconnect(xfce_overview_workspaces_core_get_default(), priv->suspendSignalID);
		priv->suspendSignalID=0;
	}

	if(priv->outlineColor)
	{
		clutter_color_free(priv->outlineColor);
		priv->outlineColor=NULL;
	}

	if(priv->styleClasses)
	{
		g_free(priv->styleClasses);
		priv->styleClasses=NULL;
	}

	if(priv->stylePseudoClasses)
	{
		g_free(priv->stylePseudoClasses);
		priv->stylePseudoClasses=NULL;
	}

	if(priv->settings)
	{
		g_object_unref(priv->settings);
		priv->settings=NULL;
	}

	/* Call parent's class dispose method */
	G_OBJECT_CLASS(xfce_overview_workspaces_window_content_x11_parent_class)->dispose(inObject);
}

/* Set/get properties */
static void _xfce_overview_workspaces_window_content_x11_set_property(GObject *inObject,
												guint inPropID,
												const GValue *inValue,
												GParamSpec *inSpec)
{
	XfceOverviewWorkspacesWindowContentX11		*self=XFCE_OVERVIEW_WORKSPACES_WINDOW_CONTENT_X11(inObject);

	switch(inPropID)
	{
		case PROP_WINDOW:
			_xfce_overview_workspaces_window_content_x11_set_window(self, XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW_X11(g_value_get_object(inValue)));
			break;

		case PROP_OUTLINE_COLOR:
			xfce_overview_workspaces_window_content_x11_set_outline_color(self, clutter_value_get_color(inValue));
			break;

		case PROP_OUTLINE_WIDTH:
			xfce_overview_workspaces_window_content_x11_set_outline_width(self, g_value_get_float(inValue));
			break;

		case PROP_INCLUDE_WINDOW_FRAME:
			xfce_overview_workspaces_window_content_x11_set_include_window_frame(self, g_value_get_boolean(inValue));
			break;

		case PROP_UNMAPPED_WINDOW_ICON_X_FILL:
			xfce_overview_workspaces_window_content_x11_set_unmapped_window_icon_x_fill(self, g_value_get_boolean(inValue));
			break;

		case PROP_UNMAPPED_WINDOW_ICON_Y_FILL:
			xfce_overview_workspaces_window_content_x11_set_unmapped_window_icon_y_fill(self, g_value_get_boolean(inValue));
			break;

		case PROP_UNMAPPED_WINDOW_ICON_X_ALIGN:
			xfce_overview_workspaces_window_content_x11_set_unmapped_window_icon_x_align(self, g_value_get_float(inValue));
			break;

		case PROP_UNMAPPED_WINDOW_ICON_Y_ALIGN:
			xfce_overview_workspaces_window_content_x11_set_unmapped_window_icon_y_align(self, g_value_get_float(inValue));
			break;

		case PROP_UNMAPPED_WINDOW_ICON_X_SCALE:
			xfce_overview_workspaces_window_content_x11_set_unmapped_window_icon_x_scale(self, g_value_get_float(inValue));
			break;

		case PROP_UNMAPPED_WINDOW_ICON_Y_SCALE:
			xfce_overview_workspaces_window_content_x11_set_unmapped_window_icon_y_scale(self, g_value_get_float(inValue));
			break;

		case PROP_UNMAPPED_WINDOW_ICON_ANCHOR_POINT:
			xfce_overview_workspaces_window_content_x11_set_unmapped_window_icon_anchor_point(self, g_value_get_enum(inValue));
			break;

		case PROP_STYLE_CLASSES:
			_xfce_overview_workspaces_window_content_x11_stylable_set_classes(XFCE_OVERVIEW_WORKSPACES_STYLABLE(self), g_value_get_string(inValue));
			break;

		case PROP_STYLE_PSEUDO_CLASSES:
			_xfce_overview_workspaces_window_content_x11_stylable_set_pseudo_classes(XFCE_OVERVIEW_WORKSPACES_STYLABLE(self), g_value_get_string(inValue));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(inObject, inPropID, inSpec);
			break;
	}
}

static void _xfce_overview_workspaces_window_content_x11_get_property(GObject *inObject,
												guint inPropID,
												GValue *outValue,
												GParamSpec *inSpec)
{
	XfceOverviewWorkspacesWindowContentX11				*self=XFCE_OVERVIEW_WORKSPACES_WINDOW_CONTENT_X11(inObject);
	XfceOverviewWorkspacesWindowContentX11Private		*priv=self->priv;

	switch(inPropID)
	{
		case PROP_WINDOW:
			g_value_set_object(outValue, priv->window);
			break;

		case PROP_SUSPENDED:
			g_value_set_boolean(outValue, priv->isSuspended);
			break;

		case PROP_OUTLINE_COLOR:
			clutter_value_set_color(outValue, priv->outlineColor);
			break;

		case PROP_OUTLINE_WIDTH:
			g_value_set_float(outValue, priv->outlineWidth);
			break;

		case PROP_INCLUDE_WINDOW_FRAME:
			g_value_set_boolean(outValue, priv->includeWindowFrame);
			break;

		case PROP_UNMAPPED_WINDOW_ICON_X_FILL:
			g_value_set_boolean(outValue, priv->unmappedWindowIconXFill);
			break;

		case PROP_UNMAPPED_WINDOW_ICON_Y_FILL:
			g_value_set_boolean(outValue, priv->unmappedWindowIconYFill);
			break;

		case PROP_UNMAPPED_WINDOW_ICON_X_ALIGN:
			g_value_set_float(outValue, priv->unmappedWindowIconXAlign);
			break;

		case PROP_UNMAPPED_WINDOW_ICON_Y_ALIGN:
			g_value_set_float(outValue, priv->unmappedWindowIconYAlign);
			break;

		case PROP_UNMAPPED_WINDOW_ICON_X_SCALE:
			g_value_set_float(outValue, priv->unmappedWindowIconXScale);
			break;

		case PROP_UNMAPPED_WINDOW_ICON_Y_SCALE:
			g_value_set_float(outValue, priv->unmappedWindowIconYScale);
			break;

		case PROP_UNMAPPED_WINDOW_ICON_ANCHOR_POINT:
			g_value_set_enum(outValue, priv->unmappedWindowIconAnchorPoint);
			break;

		case PROP_STYLE_CLASSES:
			g_value_set_string(outValue, priv->styleClasses);
			break;

		case PROP_STYLE_PSEUDO_CLASSES:
			g_value_set_string(outValue, priv->stylePseudoClasses);
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
void xfce_overview_workspaces_window_content_x11_class_init(XfceOverviewWorkspacesWindowContentX11Class *klass)
{
	GObjectClass					*gobjectClass=G_OBJECT_CLASS(klass);
	XfceOverviewWorkspacesStylableInterface	*stylableIface;
	GParamSpec						*paramSpec;

	/* Override functions */
	gobjectClass->dispose=_xfce_overview_workspaces_window_content_x11_dispose;
	gobjectClass->set_property=_xfce_overview_workspaces_window_content_x11_set_property;
	gobjectClass->get_property=_xfce_overview_workspaces_window_content_x11_get_property;

	stylableIface=g_type_default_interface_ref(XFCE_OVERVIEW_WORKSPACES_TYPE_STYLABLE);

	/* Define properties */
	XfceOverviewWorkspacesWindowContentX11Properties[PROP_WINDOW]=
		g_param_spec_object("window",
							"Window",
							"The window to handle and display",
							XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_TRACKER_WINDOW,
							G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY);

	XfceOverviewWorkspacesWindowContentX11Properties[PROP_SUSPENDED]=
		g_param_spec_boolean("suspended",
							"Suspended",
							"Is this window suspended",
							TRUE,
							G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

	XfceOverviewWorkspacesWindowContentX11Properties[PROP_OUTLINE_COLOR]=
		clutter_param_spec_color("outline-color",
									"Outline color",
									"Color to draw outline of mapped windows with",
									CLUTTER_COLOR_Black,
									G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	XfceOverviewWorkspacesWindowContentX11Properties[PROP_OUTLINE_WIDTH]=
		g_param_spec_float("outline-width",
							"Outline width",
							"Width of line used to draw outline of mapped windows",
							0.0f, G_MAXFLOAT,
							1.0f,
							G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	XfceOverviewWorkspacesWindowContentX11Properties[PROP_INCLUDE_WINDOW_FRAME]=
		g_param_spec_boolean("include-window-frame",
							"Include window frame",
							"Whether the window frame should be included or only the window content should be shown",
							FALSE,
							G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	XfceOverviewWorkspacesWindowContentX11Properties[PROP_UNMAPPED_WINDOW_ICON_X_FILL]=
		g_param_spec_boolean("unmapped-window-icon-x-fill",
							"Unmapped window icon X fill",
							"Whether the unmapped window icon should fill up horizontal space",
							TRUE,
							G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	XfceOverviewWorkspacesWindowContentX11Properties[PROP_UNMAPPED_WINDOW_ICON_Y_FILL]=
		g_param_spec_boolean("unmapped-window-icon-y-fill",
							"Unmapped window icon y fill",
							"Whether the unmapped window icon should fill up vertical space",
							TRUE,
							G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	XfceOverviewWorkspacesWindowContentX11Properties[PROP_UNMAPPED_WINDOW_ICON_X_ALIGN]=
		g_param_spec_float("unmapped-window-icon-x-align",
							"Unmapped window icon X align",
							"The alignment of the unmapped window icon on the X axis within the allocation in normalized coordinate between 0 and 1",
							0.0f, 1.0f,
							0.0f,
							G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	XfceOverviewWorkspacesWindowContentX11Properties[PROP_UNMAPPED_WINDOW_ICON_Y_ALIGN]=
		g_param_spec_float("unmapped-window-icon-y-align",
							"Unmapped window icon Y align",
							"The alignment of the unmapped window icon on the Y axis within the allocation in normalized coordinate between 0 and 1",
							0.0f, 1.0f,
							0.0f,
							G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	XfceOverviewWorkspacesWindowContentX11Properties[PROP_UNMAPPED_WINDOW_ICON_X_SCALE]=
		g_param_spec_float("unmapped-window-icon-x-scale",
							"Unmapped window icon X scale",
							"Scale factor of unmapped window icon on the X axis",
							0.0f, G_MAXFLOAT,
							1.0f,
							G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	XfceOverviewWorkspacesWindowContentX11Properties[PROP_UNMAPPED_WINDOW_ICON_Y_SCALE]=
		g_param_spec_float("unmapped-window-icon-y-scale",
							"Unmapped window icon Y scale",
							"Scale factor of unmapped window icon on the Y axis",
							0.0f, G_MAXFLOAT,
							1.0f,
							G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	XfceOverviewWorkspacesWindowContentX11Properties[PROP_UNMAPPED_WINDOW_ICON_ANCHOR_POINT]=
		g_param_spec_enum("unmapped-window-icon-anchor-point",
							"Unmapped window icon anchor point",
							"The anchor point of unmapped window icon",
							XFCE_OVERVIEW_WORKSPACES_TYPE_ANCHOR_POINT,
							XFCE_OVERVIEW_WORKSPACES_ANCHOR_POINT_NONE,
							G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	paramSpec=g_object_interface_find_property(stylableIface, "style-classes");
	XfceOverviewWorkspacesWindowContentX11Properties[PROP_STYLE_CLASSES]=
		g_param_spec_override("style-classes", paramSpec);

	paramSpec=g_object_interface_find_property(stylableIface, "style-pseudo-classes");
	XfceOverviewWorkspacesWindowContentX11Properties[PROP_STYLE_PSEUDO_CLASSES]=
		g_param_spec_override("style-pseudo-classes", paramSpec);

	g_object_class_install_properties(gobjectClass, PROP_LAST, XfceOverviewWorkspacesWindowContentX11Properties);

	/* Release allocated resources */
	g_type_default_interface_unref(stylableIface);
}

/* Object initialization
 * Create private structure and set up default values
 */
void xfce_overview_workspaces_window_content_x11_init(XfceOverviewWorkspacesWindowContentX11 *self)
{
	XfceOverviewWorkspacesWindowContentX11Private		*priv;
	XfceOverviewWorkspacesCore							*core;

	priv=self->priv=xfce_overview_workspaces_window_content_x11_get_instance_private(self);

	/* Set default values */
	priv->window=NULL;
	priv->texture=NULL;
	priv->xWindowID=None;
	priv->pixmap=None;
#ifdef HAVE_XDAMAGE
	priv->damage=None;
#endif
	priv->isFallback=FALSE;
	priv->outlineColor=clutter_color_copy(CLUTTER_COLOR_Black);
	priv->outlineWidth=1.0f;
	priv->isSuspended=TRUE;
	priv->suspendSignalID=0;
	priv->isMapped=FALSE;
	priv->includeWindowFrame=FALSE;
	priv->styleClasses=NULL;
	priv->stylePseudoClasses=NULL;
	priv->windowTracker=xfce_overview_workspaces_core_get_window_tracker(NULL);
	priv->workaroundMode=XFCE_OVERVIEW_WORKSPACES_WINDOW_CONTENT_X11_WORKAROUND_MODE_NONE;
	priv->workaroundStateSignalID=0;
	priv->unmappedWindowIconXFill=FALSE;
	priv->unmappedWindowIconYFill=FALSE;
	priv->unmappedWindowIconXAlign=0.0f;
	priv->unmappedWindowIconYAlign=0.0f;
	priv->unmappedWindowIconXScale=1.0f;
	priv->unmappedWindowIconYScale=1.0f;
	priv->unmappedWindowIconAnchorPoint=XFCE_OVERVIEW_WORKSPACES_ANCHOR_POINT_NONE;
	priv->suspendAfterResumeOnIdle=FALSE;
	priv->windowClosedSignalID=0;
	priv->settings=g_object_ref(xfce_overview_workspaces_core_get_settings(NULL));

	/* Check extensions (will only be done once) */
	_xfce_overview_workspaces_window_content_x11_check_extension();

	/* Add event filter for this instance */
#ifdef CLUTTER_WINDOWING_X11
	if(clutter_check_windowing_backend(CLUTTER_WINDOWING_X11))
	{
		clutter_x11_add_filter(_xfce_overview_workspaces_window_content_x11_on_x_event, self);
	}
#endif

#ifdef CLUTTER_WINDOWING_GDK
	if(clutter_check_windowing_backend(CLUTTER_WINDOWING_GDK))
	{
		gdk_window_add_filter(NULL, _xfce_overview_workspaces_window_content_x11_on_gdkx_event, self);
	}
#endif

	/* Style content */
	xfce_overview_workspaces_stylable_invalidate(XFCE_OVERVIEW_WORKSPACES_STYLABLE(self));

	/* Handle suspension signals from application */
	core=xfce_overview_workspaces_core_get_default();
	priv->suspendSignalID=g_signal_connect_swapped(core,
													"notify::is-suspended",
													G_CALLBACK(_xfce_overview_workspaces_window_content_x11_on_core_suspended_changed),
													self);
	priv->isAppSuspended=xfce_overview_workspaces_core_is_suspended(core);

	/* Register global signal handler for settings value change notification
	 * if not done already.
	 */
	if(!_xfce_overview_workspaces_window_content_x11_settings_priority_notify_id)
	{
		/* Connect to property changed signal in settings */
		_xfce_overview_workspaces_window_content_x11_settings_priority_notify_id=
			g_signal_connect(priv->settings,
								"notify::window-content-creation-priority",
								G_CALLBACK(_xfce_overview_workspaces_window_content_x11_on_window_creation_priority_value_changed),
								NULL);
		XFCE_OVERVIEW_WORKSPACES_DEBUG(self, WINDOWS,
							"Connected to property changed signal with handler ID %u for window creation value changed notifications",
							_xfce_overview_workspaces_window_content_x11_settings_priority_notify_id);

		/* Connect to core shutdown signal to disconnect handler for
		 * settings value changed notifications.
		 */
		_xfce_overview_workspaces_window_content_x11_window_creation_shutdown_signal_id=
			g_signal_connect(core,
								"shutdown",
								G_CALLBACK(_xfce_overview_workspaces_window_content_x11_on_window_creation_priority_shutdown),
								self);
		XFCE_OVERVIEW_WORKSPACES_DEBUG(self, WINDOWS,
							"Connected to shutdown signal with handler ID %u for disconnecting handler for window creation value changed notifications",
							_xfce_overview_workspaces_window_content_x11_window_creation_shutdown_signal_id);
	}
}

/* IMPLEMENTATION: Public API */

/* Create new instance */
ClutterContent* xfce_overview_workspaces_window_content_x11_new_for_window(XfceOverviewWorkspacesWindowTrackerWindowX11 *inWindow)
{
	ClutterContent		*content;

	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_TRACKER_WINDOW_X11(inWindow), NULL);

	/* Create window content */
	content=CLUTTER_CONTENT(g_object_new(XFCE_OVERVIEW_WORKSPACES_TYPE_WINDOW_CONTENT_X11,
											"window", inWindow,
											NULL));

	return(content);
}

/* Get window to handle and to display */
XfceOverviewWorkspacesWindowTrackerWindow* xfce_overview_workspaces_window_content_x11_get_window(XfceOverviewWorkspacesWindowContentX11 *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_CONTENT_X11(self), NULL);

	return(XFCE_OVERVIEW_WORKSPACES_WINDOW_TRACKER_WINDOW(self->priv->window));
}

/* Get state of suspension */
gboolean xfce_overview_workspaces_window_content_x11_is_suspended(XfceOverviewWorkspacesWindowContentX11 *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_CONTENT_X11(self), TRUE);

	return(self->priv->isSuspended);
}

/* Get/set color to draw outline with */
const ClutterColor* xfce_overview_workspaces_window_content_x11_get_outline_color(XfceOverviewWorkspacesWindowContentX11 *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_CONTENT_X11(self), NULL);

	return(self->priv->outlineColor);
}

void xfce_overview_workspaces_window_content_x11_set_outline_color(XfceOverviewWorkspacesWindowContentX11 *self, const ClutterColor *inColor)
{
	XfceOverviewWorkspacesWindowContentX11Private		*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_CONTENT_X11(self));
	g_return_if_fail(inColor);

	priv=self->priv;

	/* Set value if changed */
	if(priv->outlineColor==NULL || clutter_color_equal(inColor, priv->outlineColor)==FALSE)
	{
		/* Set value */
		if(priv->outlineColor) clutter_color_free(priv->outlineColor);
		priv->outlineColor=clutter_color_copy(inColor);

		/* Invalidate ourselve to get us redrawn */
		clutter_content_invalidate(CLUTTER_CONTENT(self));

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesWindowContentX11Properties[PROP_OUTLINE_COLOR]);
	}
}

/* Get/set line width for outline */
gfloat xfce_overview_workspaces_window_content_x11_get_outline_width(XfceOverviewWorkspacesWindowContentX11 *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_CONTENT_X11(self), 0.0f);

	return(self->priv->outlineWidth);
}

void xfce_overview_workspaces_window_content_x11_set_outline_width(XfceOverviewWorkspacesWindowContentX11 *self, const gfloat inWidth)
{
	XfceOverviewWorkspacesWindowContentX11Private		*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_CONTENT_X11(self));
	g_return_if_fail(inWidth>=0.0f);

	priv=self->priv;

	/* Set value if changed */
	if(priv->outlineWidth!=inWidth)
	{
		/* Set value */
		priv->outlineWidth=inWidth;

		/* Invalidate ourselve to get us redrawn */
		clutter_content_invalidate(CLUTTER_CONTENT(self));

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesWindowContentX11Properties[PROP_OUTLINE_WIDTH]);
	}
}

/* Get/set flag to indicate whether to include the window frame or not */
gboolean xfce_overview_workspaces_window_content_x11_get_include_window_frame(XfceOverviewWorkspacesWindowContentX11 *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_CONTENT_X11(self), TRUE);

	return(self->priv->includeWindowFrame);
}

void xfce_overview_workspaces_window_content_x11_set_include_window_frame(XfceOverviewWorkspacesWindowContentX11 *self, const gboolean inIncludeFrame)
{
	XfceOverviewWorkspacesWindowContentX11Private				*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_CONTENT_X11(self));

	priv=self->priv;

	/* Set value if changed */
	if(priv->includeWindowFrame!=inIncludeFrame)
	{
		/* Set value */
		priv->includeWindowFrame=inIncludeFrame;

		/* (Re-)Setup window content */
		if(priv->window)
		{
			XfceOverviewWorkspacesWindowTrackerWindowX11		*window;

			/* Re-setup window by releasing all resources first and unsetting window
			 * but remember window to set it again.
			 */
			_xfce_overview_workspaces_window_content_x11_release_resources(self);

			/* libwnck resources should never be freed. Just set to NULL */
			window=priv->window;
			priv->window=NULL;

			/* Now set same window again which causes this object to set up all
			 * needed X resources again.
			 */
			_xfce_overview_workspaces_window_content_x11_set_window(self, window);
		}

		/* Invalidate ourselve to get us redrawn */
		clutter_content_invalidate(CLUTTER_CONTENT(self));

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesWindowContentX11Properties[PROP_INCLUDE_WINDOW_FRAME]);
	}
}

/* Get/set x fill of unmapped window icon */
gboolean xfce_overview_workspaces_window_content_x11_get_unmapped_window_icon_x_fill(XfceOverviewWorkspacesWindowContentX11 *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_CONTENT_X11(self), FALSE);

	return(self->priv->unmappedWindowIconXFill);
}

void xfce_overview_workspaces_window_content_x11_set_unmapped_window_icon_x_fill(XfceOverviewWorkspacesWindowContentX11 *self, const gboolean inFill)
{
	XfceOverviewWorkspacesWindowContentX11Private		*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_CONTENT_X11(self));

	priv=self->priv;

	/* Set value if changed */
	if(priv->unmappedWindowIconXFill!=inFill)
	{
		/* Set value */
		priv->unmappedWindowIconXFill=inFill;

		/* Invalidate ourselve to get us redrawn */
		clutter_content_invalidate(CLUTTER_CONTENT(self));

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesWindowContentX11Properties[PROP_UNMAPPED_WINDOW_ICON_X_FILL]);
	}
}

/* Get/set y fill of unmapped window icon */
gboolean xfce_overview_workspaces_window_content_x11_get_unmapped_window_icon_y_fill(XfceOverviewWorkspacesWindowContentX11 *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_CONTENT_X11(self), FALSE);

	return(self->priv->unmappedWindowIconYFill);
}

void xfce_overview_workspaces_window_content_x11_set_unmapped_window_icon_y_fill(XfceOverviewWorkspacesWindowContentX11 *self, const gboolean inFill)
{
	XfceOverviewWorkspacesWindowContentX11Private		*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_CONTENT_X11(self));

	priv=self->priv;

	/* Set value if changed */
	if(priv->unmappedWindowIconYFill!=inFill)
	{
		/* Set value */
		priv->unmappedWindowIconYFill=inFill;

		/* Invalidate ourselve to get us redrawn */
		clutter_content_invalidate(CLUTTER_CONTENT(self));

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesWindowContentX11Properties[PROP_UNMAPPED_WINDOW_ICON_Y_FILL]);
	}
}

/* Get/set x align of unmapped window icon */
gfloat xfce_overview_workspaces_window_content_x11_get_unmapped_window_icon_x_align(XfceOverviewWorkspacesWindowContentX11 *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_CONTENT_X11(self), 0.0f);

	return(self->priv->unmappedWindowIconXAlign);
}

void xfce_overview_workspaces_window_content_x11_set_unmapped_window_icon_x_align(XfceOverviewWorkspacesWindowContentX11 *self, const gfloat inAlign)
{
	XfceOverviewWorkspacesWindowContentX11Private		*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_CONTENT_X11(self));
	g_return_if_fail(inAlign>=0.0f && inAlign<=1.0f);

	priv=self->priv;

	/* Set value if changed */
	if(priv->unmappedWindowIconXAlign!=inAlign)
	{
		/* Set value */
		priv->unmappedWindowIconXAlign=inAlign;

		/* Invalidate ourselve to get us redrawn */
		clutter_content_invalidate(CLUTTER_CONTENT(self));

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesWindowContentX11Properties[PROP_UNMAPPED_WINDOW_ICON_X_ALIGN]);
	}
}

/* Get/set y align of unmapped window icon */
gfloat xfce_overview_workspaces_window_content_x11_get_unmapped_window_icon_y_align(XfceOverviewWorkspacesWindowContentX11 *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_CONTENT_X11(self), 0.0f);

	return(self->priv->unmappedWindowIconYAlign);
}

void xfce_overview_workspaces_window_content_x11_set_unmapped_window_icon_y_align(XfceOverviewWorkspacesWindowContentX11 *self, const gfloat inAlign)
{
	XfceOverviewWorkspacesWindowContentX11Private		*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_CONTENT_X11(self));
	g_return_if_fail(inAlign>=0.0f && inAlign<=1.0f);

	priv=self->priv;

	/* Set value if changed */
	if(priv->unmappedWindowIconYAlign!=inAlign)
	{
		/* Set value */
		priv->unmappedWindowIconYAlign=inAlign;

		/* Invalidate ourselve to get us redrawn */
		clutter_content_invalidate(CLUTTER_CONTENT(self));

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesWindowContentX11Properties[PROP_UNMAPPED_WINDOW_ICON_Y_ALIGN]);
	}
}

/* Get/set x scale of unmapped window icon */
gfloat xfce_overview_workspaces_window_content_x11_get_unmapped_window_icon_x_scale(XfceOverviewWorkspacesWindowContentX11 *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_CONTENT_X11(self), 0.0f);

	return(self->priv->unmappedWindowIconXScale);
}

void xfce_overview_workspaces_window_content_x11_set_unmapped_window_icon_x_scale(XfceOverviewWorkspacesWindowContentX11 *self, const gfloat inScale)
{
	XfceOverviewWorkspacesWindowContentX11Private		*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_CONTENT_X11(self));
	g_return_if_fail(inScale>=0.0f);

	priv=self->priv;

	/* Set value if changed */
	if(priv->unmappedWindowIconXScale!=inScale)
	{
		/* Set value */
		priv->unmappedWindowIconXScale=inScale;

		/* Invalidate ourselve to get us redrawn */
		clutter_content_invalidate(CLUTTER_CONTENT(self));

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesWindowContentX11Properties[PROP_UNMAPPED_WINDOW_ICON_X_SCALE]);
	}
}

/* Get/set y scale of unmapped window icon */
gfloat xfce_overview_workspaces_window_content_x11_get_unmapped_window_icon_y_scale(XfceOverviewWorkspacesWindowContentX11 *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_CONTENT_X11(self), 0.0f);

	return(self->priv->unmappedWindowIconYScale);
}

void xfce_overview_workspaces_window_content_x11_set_unmapped_window_icon_y_scale(XfceOverviewWorkspacesWindowContentX11 *self, const gfloat inScale)
{
	XfceOverviewWorkspacesWindowContentX11Private		*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_CONTENT_X11(self));
	g_return_if_fail(inScale>=0.0f);

	priv=self->priv;

	/* Set value if changed */
	if(priv->unmappedWindowIconYScale!=inScale)
	{
		/* Set value */
		priv->unmappedWindowIconYScale=inScale;

		/* Invalidate ourselve to get us redrawn */
		clutter_content_invalidate(CLUTTER_CONTENT(self));

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesWindowContentX11Properties[PROP_UNMAPPED_WINDOW_ICON_Y_SCALE]);
	}
}

/* Get/set gravity (anchor point) of unmapped window icon */
XfceOverviewWorkspacesAnchorPoint xfce_overview_workspaces_window_content_x11_get_unmapped_window_icon_anchor_point(XfceOverviewWorkspacesWindowContentX11 *self)
{
	g_return_val_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_CONTENT_X11(self), XFCE_OVERVIEW_WORKSPACES_ANCHOR_POINT_NONE);

	return(self->priv->unmappedWindowIconAnchorPoint);
}

void xfce_overview_workspaces_window_content_x11_set_unmapped_window_icon_anchor_point(XfceOverviewWorkspacesWindowContentX11 *self, const XfceOverviewWorkspacesAnchorPoint inAnchorPoint)
{
	XfceOverviewWorkspacesWindowContentX11Private		*priv;

	g_return_if_fail(XFCE_OVERVIEW_WORKSPACES_IS_WINDOW_CONTENT_X11(self));
	g_return_if_fail(inAnchorPoint>=XFCE_OVERVIEW_WORKSPACES_ANCHOR_POINT_NONE);
	g_return_if_fail(inAnchorPoint<=XFCE_OVERVIEW_WORKSPACES_ANCHOR_POINT_CENTER);

	priv=self->priv;

	/* Set value if changed */
	if(priv->unmappedWindowIconAnchorPoint!=inAnchorPoint)
	{
		/* Set value */
		priv->unmappedWindowIconAnchorPoint=inAnchorPoint;

		/* Invalidate ourselve to get us redrawn */
		clutter_content_invalidate(CLUTTER_CONTENT(self));

		/* Notify about property change */
		g_object_notify_by_pspec(G_OBJECT(self), XfceOverviewWorkspacesWindowContentX11Properties[PROP_UNMAPPED_WINDOW_ICON_ANCHOR_POINT]);
	}
}
