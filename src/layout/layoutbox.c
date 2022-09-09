/* layoutbox.c
 *
 * Copyright 2022 Matthew Jakeman <mjakeman26@outlook.co.nz>
 *
 * This file is dual-licensed under the terms of the Mozilla Public
 * License 2.0 and the Lesser General Public License 2.1 (or any
 * later version).
 *
 * SPDX-License-Identifier: MPL-2.0 OR LGPL-2.1-or-later
 */

#include "layoutbox.h"
#include "layoutbox-impl.h"

typedef struct
{
    TextItem *item;
    TextDimensions bbox;
} TextLayoutBoxPrivate;

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE (TextLayoutBox, text_layout_box, TEXT_TYPE_NODE)

enum {
    PROP_0,
    N_PROPS
};

static GParamSpec *properties [N_PROPS];

/**
 * text_layout_box_new:
 *
 * Create a new #TextLayoutBox.
 *
 * Returns: (transfer full): a newly created #TextLayoutBox
 */
TextLayoutBox *
text_layout_box_new (void)
{
    return g_object_new (TEXT_TYPE_LAYOUT_BOX, NULL);
}

static void
text_layout_box_finalize (GObject *object)
{
    TextLayoutBox *self = (TextLayoutBox *)object;
    TextLayoutBoxPrivate *priv = text_layout_box_get_instance_private (self);

    // TODO: Dispose of children

    G_OBJECT_CLASS (text_layout_box_parent_class)->finalize (object);
}

static void
text_layout_box_get_property (GObject    *object,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
    TextLayoutBox *self = TEXT_LAYOUT_BOX (object);

    switch (prop_id)
    {
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
text_layout_box_set_property (GObject      *object,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
    TextLayoutBox *self = TEXT_LAYOUT_BOX (object);

    switch (prop_id)
      {
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      }
}

void
text_layout_box_layout (TextLayoutBox *self,
                        PangoContext  *context,
                        int            width,
                        int            offset_x,
                        int            offset_y)
{
    TEXT_LAYOUT_BOX_GET_CLASS (self)->layout (self, context, width, offset_x, offset_y);
}

void
text_layout_box_real_layout (TextLayoutBox *self,
                             PangoContext  *context,
                             int            width,
                             int            offset_x,
                             int            offset_y)
{
    g_return_if_fail (TEXT_IS_LAYOUT_BOX (self));

    TextLayoutBoxPrivate *priv;

    priv = text_layout_box_get_instance_private (self);

    priv->bbox.x = offset_x;
    priv->bbox.y = offset_y;
    priv->bbox.width = width;
    priv->bbox.height = 0;

    g_warning ("%s does not override text_layout_box_layout(), no layout will occur.\n",
               g_type_name_from_instance ((GTypeInstance *) self));
}

void
text_layout_box_set_item (TextLayoutBox *self,
                          TextItem      *item)
{
    TextLayoutBoxPrivate *priv = text_layout_box_get_instance_private (self);
    priv->item = item;
}

TextItem *
text_layout_box_get_item (TextLayoutBox *self)
{
    TextLayoutBoxPrivate *priv = text_layout_box_get_instance_private (self);
    return priv->item;
}

TextDimensions *
text_layout_box_get_mutable_bbox (TextLayoutBox *self)
{
    TextLayoutBoxPrivate *priv = text_layout_box_get_instance_private (self);
    return &priv->bbox;
}

const TextDimensions *
text_layout_box_get_bbox (TextLayoutBox *self)
{
    return text_layout_box_get_mutable_bbox (self);
}

static void
text_layout_box_class_init (TextLayoutBoxClass *klass)
{
    klass->layout = text_layout_box_real_layout;

    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = text_layout_box_finalize;
    object_class->get_property = text_layout_box_get_property;
    object_class->set_property = text_layout_box_set_property;
}

static void
text_layout_box_init (TextLayoutBox *self)
{
    TextLayoutBoxPrivate *priv = text_layout_box_get_instance_private (self);
}
