/* delete.c
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

#include <glib.h>
#include <locale.h>
#include <model/document.h>
#include <model/paragraph.h>
#include <model/run.h>
#include <editor/editor.h>

typedef struct {
    TextDocument *doc;
    TextEditor *editor;

    TextRun *run1;
    TextRun *run2;
    TextRun *run3;
} DeleteFixture;

#define RUN1 "abcdefghij"
#define RUN2 "1234567890"
#define RUN3 "!@#$%^&*()"

static void
delete_fixture_set_up (DeleteFixture *fixture,
                       gconstpointer  user_data)
{
    TextFrame *frame;
    TextParagraph *para1, *para2;
    TextRun *run1, *run2, *run3;

    frame = text_frame_new ();

    para1 = text_paragraph_new ();
    run1 = text_run_new (RUN1);
    run2 = text_run_new (RUN2);
    text_paragraph_append_run (para1, run1);
    text_paragraph_append_run (para1, run2);
    text_frame_append_block (frame, TEXT_BLOCK (para1));

    para2 = text_paragraph_new ();
    run3 = text_run_new (RUN3);
    text_paragraph_append_run (para2, run3);
    text_frame_append_block (frame, TEXT_BLOCK (para2));

    fixture->doc = text_document_new ();
    fixture->doc->frame = frame;

    fixture->editor = text_editor_new (fixture->doc);

    text_editor_move_first (fixture->editor, TEXT_EDITOR_CURSOR);

    fixture->run1 = run1;
    fixture->run2 = run2;
    fixture->run3 = run3;
}

static void
delete_fixture_tear_down (DeleteFixture *fixture,
                          gconstpointer  user_data)
{
    g_object_unref (fixture->editor);
    g_object_unref (fixture->doc);
}

static void
test_within_run (DeleteFixture *fixture,
                 gconstpointer  user_data)
{
    gchar *text;

    text_editor_move_right (fixture->editor, TEXT_EDITOR_CURSOR, 2);
    text_editor_delete (fixture->editor, TEXT_EDITOR_CURSOR, 5);

    g_object_get (fixture->run1, "text", &text, NULL);
    g_assert_cmpstr (text, ==, "abhij");
}

static void
test_nothing (DeleteFixture *fixture,
              gconstpointer  user_data)
{
    gchar *text;

    text_editor_move_right (fixture->editor, TEXT_EDITOR_CURSOR, 2);
    text_editor_delete (fixture->editor, TEXT_EDITOR_CURSOR, 0);

    g_object_get (fixture->run1, "text", &text, NULL);
    g_assert_cmpstr (text, ==, "abcdefghij");
}

int
main (int argc, char *argv[])
{
    setlocale (LC_ALL, "");

    g_test_init (&argc, &argv, NULL);

    // Test within run
    g_test_add ("/text-engine/editor/delete/test-within-run", DeleteFixture, NULL,
                delete_fixture_set_up, test_within_run,
                delete_fixture_tear_down);

    // Test across runs
    // Test run boundary
    // Test across paragraphs
    // Test paragraph boundary
    // Test inverse
    // Test nothing
    g_test_add ("/text-engine/editor/delete/test-nothing", DeleteFixture, NULL,
                delete_fixture_set_up, test_nothing,
                delete_fixture_tear_down);

    return g_test_run ();
}

