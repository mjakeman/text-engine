/* layoutbox-impl.h
 *
 * Copyright 2022 Matthew Jakeman <mjakeman26@outlook.co.nz>
 *
 * This file is dual-licensed under the terms of the Mozilla Public
 * License 2.0 and the Lesser General Public License 2.1 (or any
 * later version).
 *
 * SPDX-License-Identifier: MPL-2.0 OR LGPL-2.1-or-later
 */

#pragma once

#include "layoutbox.h"

G_BEGIN_DECLS

TextDimensions *
text_layout_box_get_mutable_bbox (TextLayoutBox *self);

G_END_DECLS
