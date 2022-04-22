/* node.c
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

#include "node.h"

typedef struct
{
    TextNode *parent;
    TextNode *prev;
    TextNode *next;

    TextNode *first_child;
    TextNode *last_child;
    int n_children;
} TextNodePrivate;

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE (TextNode, text_node, G_TYPE_OBJECT)

enum {
    PROP_0,
    N_PROPS
};

static GParamSpec *properties [N_PROPS];

/**
 * text_node_new:
 *
 * Create a new #TextNode.
 *
 * Returns: (transfer full): a newly created #TextNode
 */
TextNode *
text_node_new (void)
{
    return g_object_new (TEXT_TYPE_NODE, NULL);
}

static void
text_node_dispose (GObject *object)
{
    TextNode *iter;

    TextNode *self = (TextNode *)object;
    TextNodePrivate *priv = text_node_get_instance_private (self);

    for (iter = text_node_get_first_child (self);
         iter != NULL;
         iter = text_node_get_next (iter))
    {
        g_object_unref (iter);
    }

    priv->first_child = NULL;
    priv->last_child = NULL;

    G_OBJECT_CLASS (text_node_parent_class)->dispose (object);
}

static void
text_node_finalize (GObject *object)
{
    TextNode *self = (TextNode *)object;
    TextNodePrivate *priv = text_node_get_instance_private (self);

    G_OBJECT_CLASS (text_node_parent_class)->finalize (object);
}

static void
text_node_get_property (GObject    *object,
                        guint       prop_id,
                        GValue     *value,
                        GParamSpec *pspec)
{
    TextNode *self = TEXT_NODE (object);

    switch (prop_id)
    {
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
text_node_set_property (GObject      *object,
                        guint         prop_id,
                        const GValue *value,
                        GParamSpec   *pspec)
{
    TextNode *self = TEXT_NODE (object);

    switch (prop_id)
      {
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      }
}

TextNode *
text_node_get_first_child (TextNode *self)
{
    TextNodePrivate *priv = text_node_get_instance_private (self);
    return priv->first_child;
}

TextNode *
text_node_get_last_child (TextNode *self)
{
    TextNodePrivate *priv = text_node_get_instance_private (self);
    return priv->last_child;
}

TextNode *
text_node_get_previous (TextNode *self)
{
    TextNodePrivate *priv = text_node_get_instance_private (self);
    return priv->prev;
}

TextNode *
text_node_get_next (TextNode *self)
{
    TextNodePrivate *priv = text_node_get_instance_private (self);
    return priv->next;
}

TextNode *
text_node_get_parent (TextNode *self)
{
    TextNodePrivate *priv = text_node_get_instance_private (self);
    return priv->parent;
}

int
text_node_get_num_children (TextNode *self)
{
    TextNodePrivate *priv = text_node_get_instance_private (self);
    return priv->n_children;
}

static void
_insert_between (TextNode *parent,
                 TextNode *node,
                 TextNode *before,
                 TextNode *after)
{
    TextNodePrivate *node_priv, *before_priv, *after_priv, *parent_priv;

    g_assert (node != NULL);
    g_assert (before != NULL);
    g_assert (after != NULL);
    g_assert (parent != NULL);

    node_priv = text_node_get_instance_private (node);
    before_priv = text_node_get_instance_private (before);
    after_priv = text_node_get_instance_private (after);
    parent_priv = text_node_get_instance_private (parent);

    node_priv->prev = before;
    before_priv->next = node;

    node_priv->next = after;
    after_priv->prev = node;

    parent_priv->n_children++;
    node_priv->parent = parent;
}

static int
_get_index_of (TextNode *self,
               TextNode *child)
{
    TextNode *iter;
    int index;

    index = 0;

    for (iter = text_node_get_first_child (self);
         iter != NULL;
         iter = text_node_get_next (iter))
    {
        if (iter == child)
            return index;

        index++;
    }

    return -1;
}

void
text_node_insert_child (TextNode *self,
                        TextNode *child,
                        int       index)
{
    int cmp_index;
    TextNode *before, *after, *iter;
    TextNodePrivate *priv, *child_priv, *before_priv, *after_priv;

    priv = text_node_get_instance_private (self);
    child_priv = text_node_get_instance_private (child);

    g_assert (index >= 0 && index <= priv->n_children);

    g_object_ref_sink (child);

    // No children
    if (priv->n_children == 0)
    {
        priv->first_child = child;
        priv->last_child = child;
        priv->n_children = 1;

        // TODO: Weak reference?
        child_priv->parent = self;
        return;
    }

    // Prepend
    if (index == 0)
    {
        after = priv->first_child;

        after_priv = text_node_get_instance_private (after);

        after_priv->prev = child;
        child_priv->next = after;
        child_priv->prev = NULL;

        priv->first_child = child;
        priv->n_children++;

        // TODO: Weak reference?
        child_priv->parent = self;
        return;
    }

    // Append
    if (index == priv->n_children)
    {
        before = priv->last_child;

        before_priv = text_node_get_instance_private (before);

        before_priv->next = child;
        child_priv->prev = before;
        child_priv->next = NULL;

        priv->last_child = child;
        priv->n_children++;

        // TODO: Weak reference?
        child_priv->parent = self;
        return;
    }

    // Insert (At Index)
    cmp_index = 0;
    iter = text_node_get_first_child (self);

    while (++cmp_index < index) {
        iter = text_node_get_next (iter);
        g_assert (iter != NULL);
    }

    // Insert between index-1 and index
    _insert_between (self, child, iter, text_node_get_next (iter));
}

void
text_node_prepend_child (TextNode *self,
                         TextNode *child)
{
    text_node_insert_child (self, child, 0);
}

void
text_node_append_child (TextNode *self,
                        TextNode *child)
{
    TextNodePrivate *priv = text_node_get_instance_private (self);
    text_node_insert_child (self, child, priv->n_children);
}

void
text_node_insert_child_before (TextNode *self,
                               TextNode *child,
                               TextNode *compare)
{
    int index;

    index = _get_index_of (self, compare);

    if (index == -1)
    {
        g_critical ("Provided compare node is not a child of this text node.");
        return;
    }

    if ((index - 1) == 0)
    {
        g_object_ref_sink (child);
        text_node_prepend_child (self, child);
        return;
    }

    text_node_insert_child (self, child, index);
}

void
text_node_insert_child_after (TextNode *self,
                              TextNode *child,
                              TextNode *compare)
{
    int index;

    index = _get_index_of (self, compare);

    if (index == -1)
    {
        g_critical ("Provided compare node is not a child of this text node.");
        return;
    }

    text_node_insert_child (self, child, index+1);
}

TextNode *
text_node_unparent_child (TextNode *self,
                          TextNode *child)
{
    TextNode *iter;
    TextNodePrivate *iter_priv;
    TextNodePrivate *other_priv;
    TextNodePrivate *parent_priv;

    g_return_val_if_fail (child != NULL, NULL);
    g_return_val_if_fail (TEXT_IS_NODE (child), NULL);
    g_return_val_if_fail (TEXT_IS_NODE (self), NULL);

    for (iter = text_node_get_first_child (self);
         iter != NULL;
         iter = text_node_get_next (iter))
    {
        if (iter != child)
            continue;

        iter_priv = text_node_get_instance_private (iter);
        parent_priv = text_node_get_instance_private (self);

        if (iter_priv->prev) {
            other_priv = text_node_get_instance_private (iter_priv->prev);
            other_priv->next = iter_priv->next;
        } else {
            // we are the first child
            parent_priv->first_child = iter_priv->next;
        }

        if (iter_priv->next) {
            other_priv = text_node_get_instance_private (iter_priv->next);
            other_priv->prev = iter_priv->prev;
        } else {
            // we are the last child
            parent_priv->last_child = iter_priv->prev;
        }

        parent_priv->n_children--;

        return iter;
    }

    return NULL;
}

TextNode *
text_node_unparent (TextNode *self)
{
    TextNode *parent;

    g_return_val_if_fail (self != NULL, NULL);
    g_return_val_if_fail (TEXT_IS_NODE (self), NULL);

    parent = text_node_get_parent (self);

    if (parent == NULL)
        return self;
    else
        return text_node_unparent_child (parent, self);
}

void
text_node_delete_child (TextNode *self,
                        TextNode *child)
{
    g_return_if_fail (self != NULL);
    g_return_if_fail (child != NULL);
    g_return_if_fail (TEXT_IS_NODE (self));
    g_return_if_fail (TEXT_IS_NODE (child));

    if (text_node_unparent_child (self, child))
        g_object_unref (child);
}

void
text_node_delete (TextNode *self)
{
    g_return_if_fail (self != NULL);
    g_return_if_fail (TEXT_IS_NODE (self));

    if (text_node_unparent (self))
        g_object_unref (self);
}

void
text_node_clear_child (TextNode  *self,
                       TextNode **child)
{
    g_return_if_fail (self != NULL);
    g_return_if_fail (child != NULL);
    g_return_if_fail (TEXT_IS_NODE (self));
    g_return_if_fail (TEXT_IS_NODE (*child));

    text_node_delete_child (self, *child);
    *child = NULL;
}

void
text_node_clear (TextNode **self)
{
    g_return_if_fail (self != NULL);
    g_return_if_fail (TEXT_IS_NODE (*self));

    text_node_delete (*self);
    *self = NULL;
}

static void
text_node_class_init (TextNodeClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = text_node_finalize;
    object_class->dispose = text_node_dispose;
    object_class->get_property = text_node_get_property;
    object_class->set_property = text_node_set_property;
}

static void
text_node_init (TextNode *self)
{
    TextNodePrivate *priv = text_node_get_instance_private (self);
    priv->first_child = NULL;
    priv->last_child = NULL;
    priv->n_children = 0;
}
