/*
 * debug: Helpers for debugging
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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_DEBUG__
#define __LIBXFCE_OVERVIEW_WORKSPACES_DEBUG__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <glib.h>

G_BEGIN_DECLS

/* Public definitions */
/**
 * XfceOverviewWorkspacesDebugFlags:
 * @XFCE_OVERVIEW_WORKSPACES_DEBUG_MISC: Miscellaneous, if debug message does not fit in any other category
 * @XFCE_OVERVIEW_WORKSPACES_DEBUG_ACTOR: Actor related debug messages
 * @XFCE_OVERVIEW_WORKSPACES_DEBUG_STYLE: Style at actor debug messages (resolving CSS, applying style etc.)
 * @XFCE_OVERVIEW_WORKSPACES_DEBUG_THEME: Theme related debug messages (loading theme and resources etc.)
 * @XFCE_OVERVIEW_WORKSPACES_DEBUG_APPLICATIONS: Application related debug message (spawing application process, application database and tracker etc.)
 * @XFCE_OVERVIEW_WORKSPACES_DEBUG_IMAGES: Images related debug message (image cache etc.)
 * @XFCE_OVERVIEW_WORKSPACES_DEBUG_WINDOWS: Windows related debug message (window tracker, workspaces, windows, monitors etc.)
 * @XFCE_OVERVIEW_WORKSPACES_DEBUG_PLUGINS: Plug-ins related debug message (plugin manager and plugin base class)
 * @XFCE_OVERVIEW_WORKSPACES_DEBUG_ANIMATION: Animation related debug message
 *
 * Debug categories
 */
typedef enum /*< skip,flags,prefix=XFCE_OVERVIEW_WORKSPACES_DEBUG >*/
{
	XFCE_OVERVIEW_WORKSPACES_DEBUG_MISC			= 1 << 0,
	XFCE_OVERVIEW_WORKSPACES_DEBUG_ACTOR			= 1 << 1,
	XFCE_OVERVIEW_WORKSPACES_DEBUG_STYLE			= 1 << 2,
	XFCE_OVERVIEW_WORKSPACES_DEBUG_THEME			= 1 << 3,
	XFCE_OVERVIEW_WORKSPACES_DEBUG_APPLICATIONS	= 1 << 4,
	XFCE_OVERVIEW_WORKSPACES_DEBUG_IMAGES		= 1 << 5,
	XFCE_OVERVIEW_WORKSPACES_DEBUG_WINDOWS		= 1 << 6,
	XFCE_OVERVIEW_WORKSPACES_DEBUG_PLUGINS		= 1 << 7,
	XFCE_OVERVIEW_WORKSPACES_DEBUG_ANIMATION		= 1 << 8
} XfceOverviewWorkspacesDebugFlags;

#ifdef XFCE_OVERVIEW_WORKSPACES_ENABLE_DEBUG

#define XFCE_OVERVIEW_WORKSPACES_HAS_DEBUG(inCategory) \
	((xfce_overview_workspaces_debug_flags & XFCE_OVERVIEW_WORKSPACES_DEBUG_##inCategory) != FALSE)

#ifndef __GNUC__

/* Try the GCC extension for valists in macros */
#define XFCE_OVERVIEW_WORKSPACES_DEBUG(self, inCategory, inMessage, inArgs...)              \
	G_STMT_START \
	{ \
		if(G_UNLIKELY(XFCE_OVERVIEW_WORKSPACES_HAS_DEBUG(inCategory)) ||                    \
			G_UNLIKELY(xfce_overview_workspaces_debug_classes && self && g_strv_contains((const gchar * const *)xfce_overview_workspaces_debug_classes, G_OBJECT_TYPE_NAME(self))))\
		{ \
			xfce_overview_workspaces_debug_message("[%s@%p]:[" #inCategory "]:" G_STRLOC ": " inMessage, (self ? G_OBJECT_TYPE_NAME(self) : ""), self, ##inArgs);\
		} \
	} \
	G_STMT_END

#else /* !__GNUC__ */

/* Try the C99 version; unfortunately, this does not allow us to pass empty
 * arguments to the macro, which means we have to do an intemediate printf.
 */
#define XFCE_OVERVIEW_WORKSPACES_DEBUG(self, inCategory, ...)                               \
	G_STMT_START \
	{ \
		if(G_UNLIKELY(XFCE_OVERVIEW_WORKSPACES_HAS_DEBUG(inCategory)) ||                    \
			G_UNLIKELY(xfce_overview_workspaces_debug_classes && self && g_strv_contains((const gchar * const *)xfce_overview_workspaces_debug_classes, G_OBJECT_TYPE_NAME(self))))\
		{ \
			gchar	*_xfce_overview_workspaces_debug_format=g_strdup_printf(__VA_ARGS__);     \
			xfce_overview_workspaces_debug_message("[%s@%p]:[" #inCategory "]:" G_STRLOC ": %s", (self ? G_OBJECT_TYPE_NAME(self) : ""), self, _xfce_overview_workspaces_debug_format);\
			g_free(_xfce_overview_workspaces_debug_format);                                 \
		} \
	} \
	G_STMT_END
#endif

#else /* !XFCE_OVERVIEW_WORKSPACES_ENABLE_DEBUG */

#define XFCE_OVERVIEW_WORKSPACES_HAS_DEBUG(inCategory)		FALSE
#define XFCE_OVERVIEW_WORKSPACES_DEBUG(inCategory, ...)		G_STMT_START { } G_STMT_END

#endif /* XFCE_OVERVIEW_WORKSPACES_ENABLE_DEBUG */

/* Public data */

extern guint xfce_overview_workspaces_debug_flags;
extern gchar **xfce_overview_workspaces_debug_classes;

/* Public API */

void xfce_overview_workspaces_debug_messagev(const char *inFormat, va_list inArgs);
void xfce_overview_workspaces_debug_message(const char *inFormat, ...) G_GNUC_PRINTF(1, 2);

G_END_DECLS

#endif	/* __LIBXFCE_OVERVIEW_WORKSPACES_DEBUG__ */
