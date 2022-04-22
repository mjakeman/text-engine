/* layout.h
 *
 * Copyright 2022 Matthew Jakeman <mjakeman26@outlook.co.nz>
 *
 * This file is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 3 of the
 * License, or (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#pragma once

#include <glib-object.h>
#include <pango/pango.h>
#include <graphene.h>

#include "layout-box.h"

#include "../model/frame.h"
#include "../model/paragraph.h"
#include "../model/mark.h"

G_BEGIN_DECLS

#define TEXT_TYPE_LAYOUT (text_layout_get_type())

G_DECLARE_FINAL_TYPE (TextLayout, text_layout, TEXT, LAYOUT, GObject)

TextLayout *text_layout_new (void);

TextLayoutBox *
text_layout_build_layout_tree (TextLayout   *self,
                               PangoContext *context,
                               TextMark     *cursor,
                               TextFrame    *frame,
                               int           width);

G_END_DECLS
