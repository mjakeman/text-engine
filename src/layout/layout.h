/* layout.h
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

TextLayoutBox *
text_layout_pick (TextLayoutBox *root,
                  int            x,
                  int            y);

G_END_DECLS
