/* item.c
 *
 * Copyright 2022 Matthew Jakeman <mjakeman26@outlook.co.nz>
 *
 * This file is dual-licensed under the terms of the Mozilla Public
 * License 2.0 and the Lesser General Public License 2.1 (or any
 * later version).
 *
 * SPDX-License-Identifier: MPL-2.0 OR LGPL-2.1-or-later
 */

#include "item.h"

typedef struct
{
    TextNode *renderer;
} TextItemPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (TextItem, text_item, TEXT_TYPE_NODE)

enum {
    PROP_0,
    N_PROPS
};

static GParamSpec *properties [N_PROPS];

/**
 * text_item_new:
 *
 * Create a new #TextItem.
 *
 * Returns: (transfer full): a newly created #TextItem
 */
TextItem *
text_item_new (void)
{
    return g_object_new (TEXT_TYPE_ITEM, NULL);
}

void
text_item_attach (TextItem *self,
                  TextNode *attachment)
{
    TextItemPrivate *priv = text_item_get_instance_private (self);

    g_return_if_fail (!priv->renderer);

    priv->renderer = g_object_ref (attachment);
}

TextNode *
text_item_get_attachment (TextItem *self)
{
    TextItemPrivate *priv = text_item_get_instance_private (self);
    return TEXT_NODE (priv->renderer);
}

void
text_item_detach (TextItem *self)
{
    TextItemPrivate *priv = text_item_get_instance_private (self);
    if (priv->renderer)
        g_clear_object (&priv->renderer);
}

static void
text_item_finalize (GObject *object)
{
    TextItem *self = (TextItem *)object;

    G_OBJECT_CLASS (text_item_parent_class)->finalize (object);
}

static void
text_item_get_property (GObject    *object,
                        guint       prop_id,
                        GValue     *value,
                        GParamSpec *pspec)
{
    TextItem *self = TEXT_ITEM (object);

    switch (prop_id)
      {
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      }
}

static void
text_item_set_property (GObject      *object,
                        guint         prop_id,
                        const GValue *value,
                        GParamSpec   *pspec)
{
    TextItem *self = TEXT_ITEM (object);

    switch (prop_id)
      {
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      }
}

static void
text_item_class_init (TextItemClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = text_item_finalize;
    object_class->get_property = text_item_get_property;
    object_class->set_property = text_item_set_property;
}

static void
text_item_init (TextItem *self)
{
}
