/* layout-box.h
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

void
text_layout_box_set_cursor (TextLayoutBox *self,
                            int index);

gboolean
text_layout_box_get_cursor (TextLayoutBox         *self,
                            const TextDimensions **cursor);

G_END_DECLS
