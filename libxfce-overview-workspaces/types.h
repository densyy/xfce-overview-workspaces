/*
 * types: Define application specific but common types
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
 * SECTION:types
 * @title: Enums and types
 * @short_description: Common enums and types
 * @include: xfce-overview-workspaces/types.h
 *
 * Various miscellaneous utilility functions.
 */

#ifndef __LIBXFCE_OVERVIEW_WORKSPACES_TYPES__
#define __LIBXFCE_OVERVIEW_WORKSPACES_TYPES__

#if !defined(__LIBXFCE_OVERVIEW_WORKSPACES_H_INSIDE__) && !defined(LIBXFCE_OVERVIEW_WORKSPACES_COMPILATION)
#error "Only <libxfce-overview-workspaces/libxfce-overview-workspaces.h> can be included directly."
#endif

#include <glib.h>

G_BEGIN_DECLS

/**
 * XfceOverviewWorkspacesViewMode:
 * @XFCE_OVERVIEW_WORKSPACES_VIEW_MODE_LIST: Show items in view as list
 * @XFCE_OVERVIEW_WORKSPACES_VIEW_MODE_ICON: Show items in view as icons
 *
 * Determines how to display items of a view.
 */
typedef enum /*< prefix=XFCE_OVERVIEW_WORKSPACES_VIEW_MODE >*/
{
	XFCE_OVERVIEW_WORKSPACES_VIEW_MODE_LIST=0,
	XFCE_OVERVIEW_WORKSPACES_VIEW_MODE_ICON
} XfceOverviewWorkspacesViewMode;

/**
 * XfceOverviewWorkspacesVisibilityPolicy:
 * @XFCE_OVERVIEW_WORKSPACES_VISIBILITY_POLICY_NEVER: The actor is always visible.
 * @XFCE_OVERVIEW_WORKSPACES_VISIBILITY_POLICY_AUTOMATIC: The actor will appear and disappear as necessary. For example, when a view does not fit into viewpad the scrollbar will be visible.
 * @XFCE_OVERVIEW_WORKSPACES_VISIBILITY_POLICY_ALWAYS: The actor will never appear.
 *
 * Determines when an actor will be visible, e.g. scrollbars in views.
 */
typedef enum /*< prefix=XFCE_OVERVIEW_WORKSPACES_VISIBILITY_POLICY >*/
{
	XFCE_OVERVIEW_WORKSPACES_VISIBILITY_POLICY_NEVER=0,
	XFCE_OVERVIEW_WORKSPACES_VISIBILITY_POLICY_AUTOMATIC,
	XFCE_OVERVIEW_WORKSPACES_VISIBILITY_POLICY_ALWAYS
} XfceOverviewWorkspacesVisibilityPolicy;

/**
 * XfceOverviewWorkspacesOrientation:
 * @XFCE_OVERVIEW_WORKSPACES_ORIENTATION_LEFT: The actor is justified to left boundary.
 * @XFCE_OVERVIEW_WORKSPACES_ORIENTATION_RIGHT: The actor is justified to right boundary.
 * @XFCE_OVERVIEW_WORKSPACES_ORIENTATION_TOP: The actor is justified to top boundary.
 * @XFCE_OVERVIEW_WORKSPACES_ORIENTATION_BOTTOM: The actor is justified to bottom boundary.
 *
 * Determines the side to which an actor is justified to. It can mostly be switched on-the-fly.
 */
typedef enum /*< prefix=XFCE_OVERVIEW_WORKSPACES_ORIENTATION >*/
{
	XFCE_OVERVIEW_WORKSPACES_ORIENTATION_LEFT=0,
	XFCE_OVERVIEW_WORKSPACES_ORIENTATION_RIGHT,
	XFCE_OVERVIEW_WORKSPACES_ORIENTATION_TOP,
	XFCE_OVERVIEW_WORKSPACES_ORIENTATION_BOTTOM
} XfceOverviewWorkspacesOrientation;

/**
 * XfceOverviewWorkspacesCorners:
 * @XFCE_OVERVIEW_WORKSPACES_CORNERS_NONE: No corner is affected.
 * @XFCE_OVERVIEW_WORKSPACES_CORNERS_TOP_LEFT: Affects top-left corner of actor.
 * @XFCE_OVERVIEW_WORKSPACES_CORNERS_TOP_RIGHT: Affects top-right corner of actor.
 * @XFCE_OVERVIEW_WORKSPACES_CORNERS_BOTTOM_LEFT: Affects bottom-left corner of actor.
 * @XFCE_OVERVIEW_WORKSPACES_CORNERS_BOTTOM_RIGHT: Affects bottom-right corner of actor.
 * @XFCE_OVERVIEW_WORKSPACES_CORNERS_TOP: Affects corners at top side of actor - top-left and top-right.
 * @XFCE_OVERVIEW_WORKSPACES_CORNERS_BOTTOM: Affects corners at bottom side of actor - bottom-left and bottom-right.
 * @XFCE_OVERVIEW_WORKSPACES_CORNERS_LEFT: Affects corners at left side of actor - top-left and bottom-left.
 * @XFCE_OVERVIEW_WORKSPACES_CORNERS_RIGHT: Affects corners at right side of actor - top-right and bottom-right.
 * @XFCE_OVERVIEW_WORKSPACES_CORNERS_ALL: Affects all corners of actor.
 *
 * Specifies which corner of an actor is affected, e.g. used in background for rounded rectangles.
 */
typedef enum /*< flags,prefix=XFCE_OVERVIEW_WORKSPACES_CORNERS >*/
{
	XFCE_OVERVIEW_WORKSPACES_CORNERS_NONE=0,

	XFCE_OVERVIEW_WORKSPACES_CORNERS_TOP_LEFT=1 << 0,
	XFCE_OVERVIEW_WORKSPACES_CORNERS_TOP_RIGHT=1 << 1,
	XFCE_OVERVIEW_WORKSPACES_CORNERS_BOTTOM_LEFT=1 << 2,
	XFCE_OVERVIEW_WORKSPACES_CORNERS_BOTTOM_RIGHT=1 << 3,

	XFCE_OVERVIEW_WORKSPACES_CORNERS_TOP=(XFCE_OVERVIEW_WORKSPACES_CORNERS_TOP_LEFT | XFCE_OVERVIEW_WORKSPACES_CORNERS_TOP_RIGHT),
	XFCE_OVERVIEW_WORKSPACES_CORNERS_BOTTOM=(XFCE_OVERVIEW_WORKSPACES_CORNERS_BOTTOM_LEFT | XFCE_OVERVIEW_WORKSPACES_CORNERS_BOTTOM_RIGHT),
	XFCE_OVERVIEW_WORKSPACES_CORNERS_LEFT=(XFCE_OVERVIEW_WORKSPACES_CORNERS_TOP_LEFT | XFCE_OVERVIEW_WORKSPACES_CORNERS_BOTTOM_LEFT),
	XFCE_OVERVIEW_WORKSPACES_CORNERS_RIGHT=(XFCE_OVERVIEW_WORKSPACES_CORNERS_TOP_RIGHT | XFCE_OVERVIEW_WORKSPACES_CORNERS_BOTTOM_RIGHT),

	XFCE_OVERVIEW_WORKSPACES_CORNERS_ALL=(XFCE_OVERVIEW_WORKSPACES_CORNERS_TOP_LEFT | XFCE_OVERVIEW_WORKSPACES_CORNERS_TOP_RIGHT | XFCE_OVERVIEW_WORKSPACES_CORNERS_BOTTOM_LEFT | XFCE_OVERVIEW_WORKSPACES_CORNERS_BOTTOM_RIGHT)
} XfceOverviewWorkspacesCorners;

/**
 * XfceOverviewWorkspacesBorders:
 * @XFCE_OVERVIEW_WORKSPACES_BORDERS_NONE: No side is affected.
 * @XFCE_OVERVIEW_WORKSPACES_BORDERS_LEFT: Affects left side of actor.
 * @XFCE_OVERVIEW_WORKSPACES_BORDERS_TOP: Affects top side of actor.
 * @XFCE_OVERVIEW_WORKSPACES_BORDERS_RIGHT: Affects right side of actor.
 * @XFCE_OVERVIEW_WORKSPACES_BORDERS_BOTTOM: Affects bottom side of actor.
 * @XFCE_OVERVIEW_WORKSPACES_BORDERS_ALL: Affects all sides of actor.
 *
 * Determines which side of an actor is affected, e.g. used in outlines.
 */
typedef enum /*< flags,prefix=XFCE_OVERVIEW_WORKSPACES_BORDERS >*/
{
	XFCE_OVERVIEW_WORKSPACES_BORDERS_NONE=0,

	XFCE_OVERVIEW_WORKSPACES_BORDERS_LEFT=1 << 0,
	XFCE_OVERVIEW_WORKSPACES_BORDERS_TOP=1 << 1,
	XFCE_OVERVIEW_WORKSPACES_BORDERS_RIGHT=1 << 2,
	XFCE_OVERVIEW_WORKSPACES_BORDERS_BOTTOM=1 << 3,

	XFCE_OVERVIEW_WORKSPACES_BORDERS_ALL=(XFCE_OVERVIEW_WORKSPACES_BORDERS_LEFT | XFCE_OVERVIEW_WORKSPACES_BORDERS_TOP | XFCE_OVERVIEW_WORKSPACES_BORDERS_RIGHT | XFCE_OVERVIEW_WORKSPACES_BORDERS_BOTTOM)
} XfceOverviewWorkspacesBorders;

/**
 * XfceOverviewWorkspacesStageBackgroundImageType:
 * @XFCE_OVERVIEW_WORKSPACES_STAGE_BACKGROUND_IMAGE_TYPE_NONE: Do not show anything at background of stage actor.
 * @XFCE_OVERVIEW_WORKSPACES_STAGE_BACKGROUND_IMAGE_TYPE_DESKTOP: Show current desktop image at background of stage actor.
 *
 * Determine what to show at background of a stage actor.
 */
typedef enum /*< prefix=XFCE_OVERVIEW_WORKSPACES_STAGE_BACKGROUND_IMAGE_TYPE >*/
{
	XFCE_OVERVIEW_WORKSPACES_STAGE_BACKGROUND_IMAGE_TYPE_NONE=0,
	XFCE_OVERVIEW_WORKSPACES_STAGE_BACKGROUND_IMAGE_TYPE_DESKTOP
} XfceOverviewWorkspacesStageBackgroundImageType;

/**
 * XfceOverviewWorkspacesSelectionTarget:
 * @XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_LEFT: Move to next selectable actor at left side.
 * @XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_RIGHT: Move to next selectable actor at right side.
 * @XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_UP: Move to next selectable actor at top side.
 * @XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_DOWN: Move to next selectable actor at bottom side.
 * @XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_FIRST: Move to first selectable actor.
 * @XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_LAST: Move to last selectable actor.
 * @XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_PAGE_LEFT: Move to next selectable actor at left side page-width.
 * @XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_PAGE_RIGHT: Move to next selectable actor at right side page-width.
 * @XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_PAGE_UP: Move to next selectable actor at top side page-width.
 * @XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_PAGE_DOWN: Move to next selectable actor at bottom side page-width.
 * @XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_NEXT: Move to next selectable actor to current one.
 *
 * Determines the movement of selection within an actor which supports selections.
 */
typedef enum /*< prefix=XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET >*/
{
	XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_LEFT=0,
	XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_RIGHT,
	XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_UP,
	XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_DOWN,

	XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_FIRST,
	XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_LAST,

	XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_PAGE_LEFT,
	XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_PAGE_RIGHT,
	XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_PAGE_UP,
	XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_PAGE_DOWN,

	XFCE_OVERVIEW_WORKSPACES_SELECTION_TARGET_NEXT
} XfceOverviewWorkspacesSelectionTarget;

/* Anchor points */
/**
 * XfceOverviewWorkspacesAnchorPoint:
 * @XFCE_OVERVIEW_WORKSPACES_ANCHOR_POINT_NONE: Use default anchor of actor, usually top-left.
 * @XFCE_OVERVIEW_WORKSPACES_ANCHOR_POINT_NORTH_WEST: The anchor is at the top-left of the object. 
 * @XFCE_OVERVIEW_WORKSPACES_ANCHOR_POINT_NORTH: The anchor is at the top of the object, centered horizontally. 
 * @XFCE_OVERVIEW_WORKSPACES_ANCHOR_POINT_NORTH_EAST: The anchor is at the top-right of the object. 
 * @XFCE_OVERVIEW_WORKSPACES_ANCHOR_POINT_EAST: The anchor is on the right of the object, centered vertically. 
 * @XFCE_OVERVIEW_WORKSPACES_ANCHOR_POINT_SOUTH_EAST:  The anchor is at the bottom-right of the object. 
 * @XFCE_OVERVIEW_WORKSPACES_ANCHOR_POINT_SOUTH: The anchor is at the bottom of the object, centered horizontally. 
 * @XFCE_OVERVIEW_WORKSPACES_ANCHOR_POINT_SOUTH_WEST:  The anchor is at the bottom-left of the object. 
 * @XFCE_OVERVIEW_WORKSPACES_ANCHOR_POINT_WEST: The anchor is on the left of the object, centered vertically. 
 * @XFCE_OVERVIEW_WORKSPACES_ANCHOR_POINT_CENTER: The anchor is in the center of the object. 
 * 
 * Specifys the position of an object relative to a particular anchor point.
 */
typedef enum /*< prefix=XFCE_OVERVIEW_WORKSPACES_ANCHOR_POINT >*/
{
	XFCE_OVERVIEW_WORKSPACES_ANCHOR_POINT_NONE=0,
	XFCE_OVERVIEW_WORKSPACES_ANCHOR_POINT_NORTH_WEST,
	XFCE_OVERVIEW_WORKSPACES_ANCHOR_POINT_NORTH,
	XFCE_OVERVIEW_WORKSPACES_ANCHOR_POINT_NORTH_EAST,
	XFCE_OVERVIEW_WORKSPACES_ANCHOR_POINT_EAST,
	XFCE_OVERVIEW_WORKSPACES_ANCHOR_POINT_SOUTH_EAST,
	XFCE_OVERVIEW_WORKSPACES_ANCHOR_POINT_SOUTH,
	XFCE_OVERVIEW_WORKSPACES_ANCHOR_POINT_SOUTH_WEST,
	XFCE_OVERVIEW_WORKSPACES_ANCHOR_POINT_WEST,
	XFCE_OVERVIEW_WORKSPACES_ANCHOR_POINT_CENTER
} XfceOverviewWorkspacesAnchorPoint;

G_END_DECLS

#endif	/* __LIBXFCE_OVERVIEW_WORKSPACES_TYPES__ */
