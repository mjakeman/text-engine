/* parser.c
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

#include "parser.h"

#include <model/paragraph.h>
#include <model/block.h>
#include <model/run.h>

#include <libxml/HTMLparser.h>
#include <libxml/HTMLtree.h>

static void
build_text_frame_recursive (xmlNode        *nodes,
                            TextFrame      *frame,
                            TextParagraph **current)
{
    g_return_if_fail (TEXT_IS_FRAME (frame));
    g_return_if_fail (current);

    xmlNode *cur_node = NULL;

    if (nodes == NULL)
        return;

    for (cur_node = nodes; cur_node; cur_node = cur_node->next)
    {
        if (cur_node->type == XML_ELEMENT_NODE)
        {
            if (g_str_equal (cur_node->name, "p") ||
                g_str_equal (cur_node->name, "br"))
            {
                *current = text_paragraph_new ();
                text_frame_append_block (frame, TEXT_BLOCK (*current));
            }
            else
            {
                // Catch-all for not-yet implemented elements
                g_print ("Ignored element %s\n", cur_node->name);
            }
        }
        else if (cur_node->type == XML_TEXT_NODE)
        {
            // Append text as new run
            const gchar *content = (gchar *)cur_node->content;
            text_paragraph_append_run (*current, text_run_new (content));
        }
        build_text_frame_recursive (cur_node->children, frame, current);
    }
}

/**
 * build_text_frame:
 * @nodes: Array of #xmlNode structures to parse
 * @frame: An initialised #TextFrame to populate with data
 *
 * Recursively builds a new #TextFrame and populates it with data
 * from the provided normalised HTML tree.
 */
static void
build_text_frame (xmlNode   *nodes,
                  TextFrame *frame)
{
    TextParagraph *current = NULL;
    build_text_frame_recursive (nodes, frame, &current);
}

TextFrame *
parse_comment_html (const gchar *html)
{
    htmlDocPtr doc;
    xmlNode *root;
    TextFrame *frame;

    g_print ("%s\n", html);

    doc = htmlParseDoc ((const guchar *)html, "utf-8");

    if (doc == NULL)
    {
        g_critical ("Could not parse HTML document.");
        return NULL;
    }

    root = xmlDocGetRootElement (doc);

    if (root == NULL)
    {
        g_warning ("Empty HTML document.");
        xmlFreeDoc (doc);
        xmlCleanupParser ();
        return NULL;
    }

    frame = text_frame_new ();

    g_print ("Root Node discovered: %s\n", root->name);

    build_text_frame (root, frame);

    xmlFreeDoc (doc);
    xmlCleanupParser ();

    return frame;
}
