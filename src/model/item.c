/* item.c
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

#include "item.h"

G_DEFINE_TYPE (TextItem, text_item, TEXT_TYPE_NODE)

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
