/* move.c
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
} MoveFixture;

#define RUN1 "Once upon a time there was a little dog, "
#define RUN2 "and his name was Rover."
#define RUN3 "By J. R. R. Tolkien"

static void
move_fixture_set_up_single (MoveFixture   *fixture,
                            gconstpointer  user_data)
{
    TextFrame *frame;
    TextParagraph *para1;
    TextRun *run1;

    frame = text_frame_new ();

    para1 = text_paragraph_new ();
    run1 = text_run_new (RUN1);
    text_paragraph_append_run (para1, run1);
    text_frame_append_block (frame, TEXT_BLOCK (para1));

    fixture->doc = text_document_new ();
    fixture->doc->frame = frame;

    fixture->editor = text_editor_new (fixture->doc);

    text_editor_move_first (fixture->editor, TEXT_EDITOR_CURSOR);

    fixture->run1 = run1;
    fixture->run2 = NULL;
    fixture->run3 = NULL;
}

static void
move_fixture_set_up_multi (MoveFixture   *fixture,
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
move_fixture_tear_down (MoveFixture   *fixture,
                        gconstpointer  user_data)
{
    g_object_unref (fixture->editor);
    g_object_unref (fixture->doc);
}

static void
test_left_guard (MoveFixture   *fixture,
                 gconstpointer  user_data)
{
    int amount;
    amount = (int)user_data;

    // test moving left at the start of the document
    text_editor_move_left (fixture->editor, TEXT_EDITOR_CURSOR, amount);

    // cursor position unchanged
    g_assert_cmpint (fixture->doc->cursor->index, ==, 0);
}

static void
test_right_guard (MoveFixture   *fixture,
                  gconstpointer  user_data)
{
    int amount;
    amount = (int)user_data;

    // go to end
    text_editor_move_right (fixture->editor, TEXT_EDITOR_CURSOR, 41);

    // test moving right at the end of the document
    text_editor_move_right (fixture->editor, TEXT_EDITOR_CURSOR, amount);

    // cursor position unchanged
    g_assert_cmpint (fixture->doc->cursor->index, ==, 41);
}

int
main (int argc, char *argv[])
{
    setlocale (LC_ALL, "");

    g_test_init (&argc, &argv, NULL);

    // Define the tests.
    g_test_add ("/text-engine/editor/move/test-left-guard-one", MoveFixture, (void*)1,
                move_fixture_set_up_single, test_left_guard,
                move_fixture_tear_down);
    g_test_add ("/text-engine/editor/move/test-left-guard-ten", MoveFixture, (void*)10,
                move_fixture_set_up_single, test_left_guard,
                move_fixture_tear_down);
    g_test_add ("/text-engine/editor/move/test-right-guard-one", MoveFixture, (void*)1,
                move_fixture_set_up_single, test_right_guard,
                move_fixture_tear_down);
    g_test_add ("/text-engine/editor/move/test-right-guard-ten", MoveFixture, (void*)10,
                move_fixture_set_up_single, test_right_guard,
                move_fixture_tear_down);

    return g_test_run ();
}

