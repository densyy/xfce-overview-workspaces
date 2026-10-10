/*
 * restart: Quit running primary instance via DBus and wait for it to vanish
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

#ifndef __XFCE_OVERVIEW_WORKSPACES_RESTART__
#define __XFCE_OVERVIEW_WORKSPACES_RESTART__

#include <glib.h>

#include "application.h"

G_BEGIN_DECLS

/**
 * xfce_overview_workspaces_restart:
 * @application: The remote #XfceOverviewWorkspacesApplication instance requesting restart
 *
 * Quit the running primary instance via DBus and wait until its bus name
 * vanishes. Used to handle `-r`/`--restart` and `-q`/`--quit` flows.
 *
 * Returns: %TRUE if the running instance quitted, otherwise %FALSE.
 */
gboolean xfce_overview_workspaces_restart(XfceOverviewWorkspacesApplication *application);

G_END_DECLS

#endif	/* __XFCE_OVERVIEW_WORKSPACES_RESTART__ */
