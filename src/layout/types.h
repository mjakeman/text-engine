/* types.h
 *
 * Copyright 2022 Matthew Jakeman <mjakeman26@outlook.co.nz>
 *
 * This file is dual-licensed under the terms of the Mozilla Public
 * License 2.0 and the Lesser General Public License 2.1 (or any
 * later version).
 *
 * SPDX-License-Identifier: MPL-2.0 OR LGPL-2.1-or-later
 */

/* TODO: Make these GBoxed for introspection support! */

#include <glib-object.h>

typedef struct
{
    gdouble x;
    gdouble y;
    gdouble width;
    gdouble height;

    // TODO: Also consider padding/margin/border?
} TextDimensions;
