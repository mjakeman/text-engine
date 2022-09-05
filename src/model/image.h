/* image.h
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

#include "item.h"
#include "fragment.h"

G_BEGIN_DECLS

#define TEXT_TYPE_IMAGE (text_image_get_type())

G_DECLARE_FINAL_TYPE (TextImage, text_image, TEXT, IMAGE, TextFragment)

TextImage *text_image_new        (const gchar *src);

G_END_DECLS
