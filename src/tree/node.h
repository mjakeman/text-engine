/* node.h
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

G_BEGIN_DECLS

#define TEXT_TYPE_NODE (text_node_get_type())

G_DECLARE_DERIVABLE_TYPE (TextNode, text_node, TEXT, NODE, GObject)

struct _TextNodeClass
{
    GObjectClass parent_class;
};

// Implementors Only
TextNode *text_node_get_parent          (TextNode *self);
TextNode *text_node_get_next            (TextNode *self);
TextNode *text_node_get_previous        (TextNode *self);
TextNode *text_node_get_first_child     (TextNode *self);
TextNode *text_node_get_last_child      (TextNode *self);

void      text_node_insert_child        (TextNode *self, TextNode *child, int index);
void      text_node_prepend_child       (TextNode *self, TextNode *child);
void      text_node_append_child        (TextNode *self, TextNode *child);
void      text_node_insert_child_before (TextNode *self, TextNode *child, TextNode *compare);
void      text_node_insert_child_after  (TextNode *self, TextNode *child, TextNode *compare);

void      text_node_delete              (TextNode **self);
void      text_node_delete_child        (TextNode *self, TextNode **child);

G_END_DECLS
