/* layout-box.h
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

#include "../tree/node.h"
#include "../model/item.h"

#include "types.h"

G_BEGIN_DECLS

#define TEXT_TYPE_LAYOUT_BOX (text_layout_box_get_type())

G_DECLARE_DERIVABLE_TYPE (TextLayoutBox, text_layout_box, TEXT, LAYOUT_BOX, TextNode)

struct _TextLayoutBoxClass
{
    TextNodeClass parent_class;
};

TextLayoutBox *text_layout_box_new (void);

void
text_layout_box_set_item (TextLayoutBox *self,
                          TextItem      *item);

TextItem *
text_layout_box_get_item (TextLayoutBox *self);

void
text_layout_box_layout (TextLayoutBox *self,
                        PangoContext  *context,
                        int            width);

const TextDimensions *
text_layout_box_get_bbox (TextLayoutBox *self);

PangoLayout *
text_layout_box_get_pango_layout (TextLayoutBox *self);

G_END_DECLS
