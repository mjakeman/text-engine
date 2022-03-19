/* layout-box.h
 *
 * Copyright 2022 Matthew Jakeman <mjakeman26@outlook.co.nz>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * SPDX-License-Identifier: MIT
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
text_layout_box_get_item (TextLayoutBox *self,
                          TextItem      *item);

void
text_layout_box_layout (TextLayoutBox *self,
                        PangoContext  *context,
                        int            width);

const TextDimensions *
text_layout_box_get_bbox (TextLayoutBox *self);

PangoLayout *
text_layout_box_get_pango_layout (TextLayoutBox *self);

G_END_DECLS
