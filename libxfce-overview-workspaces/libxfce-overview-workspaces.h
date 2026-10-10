/*
 * libxfce-overview-workspaces: Maybe a Gnome shell like dashboard for Xfce
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

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_H__
#define __LIBXFCE_OVERVIEW_WORKSPACES_H__

#define __LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__

#include <libxfce-overview-workspaces/action-button.h>
#include <libxfce-overview-workspaces/actor.h>
#include <libxfce-overview-workspaces/animation.h>
#include <libxfce-overview-workspaces/application-button.h>
#include <libxfce-overview-workspaces/application-database.h>
#include <libxfce-overview-workspaces/application-tracker.h>
#include <libxfce-overview-workspaces/applications-menu-model.h>
#include <libxfce-overview-workspaces/applications-view.h>
#include <libxfce-overview-workspaces/background.h>
#include <libxfce-overview-workspaces/binding.h>
#include <libxfce-overview-workspaces/bindings-pool.h>
#include <libxfce-overview-workspaces/box-layout.h>
#include <libxfce-overview-workspaces/button.h>
#include <libxfce-overview-workspaces/click-action.h>
#include <libxfce-overview-workspaces/collapse-box.h>
#include <libxfce-overview-workspaces/core.h>
#include <libxfce-overview-workspaces/css-selector.h>
#include <libxfce-overview-workspaces/debug.h>
#include <libxfce-overview-workspaces/desktop-app-info.h>
#include <libxfce-overview-workspaces/desktop-app-info-action.h>
#include <libxfce-overview-workspaces/drag-action.h>
#include <libxfce-overview-workspaces/drop-action.h>
#include <libxfce-overview-workspaces/dynamic-table-layout.h>
#include <libxfce-overview-workspaces/emblem-effect.h>
#include <libxfce-overview-workspaces/fill-box-layout.h>
#include <libxfce-overview-workspaces/focusable.h>
#include <libxfce-overview-workspaces/focus-manager.h>
#include <libxfce-overview-workspaces/gradient-color.h>
#include <libxfce-overview-workspaces/image-content.h>
#include <libxfce-overview-workspaces/label.h>
#include <libxfce-overview-workspaces/live-window.h>
#include <libxfce-overview-workspaces/live-window-simple.h>
#include <libxfce-overview-workspaces/live-workspace.h>
#include <libxfce-overview-workspaces/model.h>
#include <libxfce-overview-workspaces/outline-effect.h>
#include <libxfce-overview-workspaces/plugin.h>
#include <libxfce-overview-workspaces/plugin-settings.h>
#include <libxfce-overview-workspaces/plugins-manager.h>
#include <libxfce-overview-workspaces/popup-menu.h>
#include <libxfce-overview-workspaces/popup-menu-item.h>
#include <libxfce-overview-workspaces/popup-menu-item-button.h>
#include <libxfce-overview-workspaces/popup-menu-item-separator.h>
#include <libxfce-overview-workspaces/scaled-table-layout.h>
#include <libxfce-overview-workspaces/scrollbar.h>
#include <libxfce-overview-workspaces/settings.h>
#include <libxfce-overview-workspaces/stage.h>
#include <libxfce-overview-workspaces/stage-interface.h>
#include <libxfce-overview-workspaces/stylable.h>
#include <libxfce-overview-workspaces/text-box.h>
#include <libxfce-overview-workspaces/theme.h>
#include <libxfce-overview-workspaces/theme-animation.h>
#include <libxfce-overview-workspaces/theme-css.h>
#include <libxfce-overview-workspaces/theme-effects.h>
#include <libxfce-overview-workspaces/theme-layout.h>
#include <libxfce-overview-workspaces/toggle-button.h>
#include <libxfce-overview-workspaces/tooltip-action.h>
#include <libxfce-overview-workspaces/transition-group.h>
#include <libxfce-overview-workspaces/types.h>
#include <libxfce-overview-workspaces/utils.h>
#include <libxfce-overview-workspaces/version.h>
#include <libxfce-overview-workspaces/view.h>
#include <libxfce-overview-workspaces/view-manager.h>
#include <libxfce-overview-workspaces/viewpad.h>
#include <libxfce-overview-workspaces/window-content.h>
#include <libxfce-overview-workspaces/windows-view.h>
#include <libxfce-overview-workspaces/window-tracker.h>
#include <libxfce-overview-workspaces/window-tracker-backend.h>
#include <libxfce-overview-workspaces/window-tracker-monitor.h>
#include <libxfce-overview-workspaces/window-tracker-window.h>
#include <libxfce-overview-workspaces/window-tracker-workspace.h>
#include <libxfce-overview-workspaces/workspace-selector.h>

#undef __LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__

#endif
