/* operation.c
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

#include "operation.h"

#include <json-glib/json-glib.h>

G_DEFINE_BOXED_TYPE_WITH_CODE (TextOperation, text_operation,
                               text_operation_copy,
                               text_operation_free,
                               register_json_funcs)

typedef enum
{
    TEXT_OPERATION_INSERT,
    TEXT_OPERATION_DELETE,
    TEXT_OPERATION_RETAIN
} TextOperationType;

struct _TextOperation
{
    TextOperationType op_type;
    GHashTable *attr_table; // Use GVariant?
};

/**
 * text_operation_new:
 *
 * Creates a new #TextOperation.
 *
 * Returns: (transfer full): A newly created #TextOperation
 */
TextOperation *
text_operation_new (void)
{
    TextOperation *self;

    self = g_slice_new0 (TextOperation);

    return self;
}

/**
 * text_operation_copy:
 * @self: a #TextOperation
 *
 * Makes a deep copy of a #TextOperation.
 *
 * Returns: (transfer full): A newly created #TextOperation with the same
 *   contents as @self
 */
TextOperation *
text_operation_copy (TextOperation *self)
{
    TextOperation *copy;

    g_return_val_if_fail (self, NULL);

    copy = text_operation_new ();

    return copy;
}

/**
 * text_operation_free:
 * @self: a #TextOperation
 *
 * Frees a #TextOperation allocated using text_operation_new()
 * or text_operation_copy().
 */
void
text_operation_free (TextOperation *self)
{
    g_return_if_fail (self);

    g_slice_free (TextOperation, self);
}

static JsonNode *
text_operation_serialize (gconstpointer boxed)
{

}

static gpointer
text_operation_deserialize (JsonNode *node)
{

}

static void
register_json_funcs (GType boxed_type)
{
    json_boxed_register_serialize_func (boxed_type, JSON_NODE_OBJECT,
                                        text_operation_serialize);
    json_boxed_register_deserialize_func (boxed_type, JSON_NODE_OBJECT,
                                          text_operation_deserialize);
}
