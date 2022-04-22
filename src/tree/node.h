/* node.h
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
int       text_node_get_num_children    (TextNode *self);

void      text_node_insert_child        (TextNode *self, TextNode *child, int index);
void      text_node_prepend_child       (TextNode *self, TextNode *child);
void      text_node_append_child        (TextNode *self, TextNode *child);
void      text_node_insert_child_before (TextNode *self, TextNode *child, TextNode *compare);
void      text_node_insert_child_after  (TextNode *self, TextNode *child, TextNode *compare);

TextNode *text_node_unparent            (TextNode *self);
TextNode *text_node_unparent_child      (TextNode *self, TextNode *child);
void      text_node_delete              (TextNode *self);
void      text_node_delete_child        (TextNode *self, TextNode *child);
void      text_node_clear               (TextNode **self);
void      text_node_clear_child         (TextNode *self, TextNode **child);

G_END_DECLS
