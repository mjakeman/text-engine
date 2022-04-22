/* split.c
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
    TextRun *run4;
    TextRun *run5;
    TextParagraph *para1;
    TextParagraph *para2;
    TextParagraph *para3;
} SplitFixture;

#define RUN1 "abcdefghij"
#define RUN2 "1234567890"
#define RUN3 "!@#$%^&*()"
#define RUN4 "zxcvbnm,./"
#define RUN5 "0987654321"

static void
split_fixture_set_up (SplitFixture *fixture,
                        gconstpointer   user_data)
{
    TextFrame *frame;
    TextParagraph *para1, *para2, *para3;
    TextRun *run1, *run2, *run3, *run4, *run5;

    frame = text_frame_new ();

    para1 = text_paragraph_new ();
    run1 = text_run_new (RUN1);
    run2 = text_run_new (RUN2);
    run3 = text_run_new (RUN3);
    text_paragraph_append_run (para1, run1);
    text_paragraph_append_run (para1, run2);
    text_paragraph_append_run (para1, run3);
    text_frame_append_block (frame, TEXT_BLOCK (para1));

    para2 = text_paragraph_new ();
    run4 = text_run_new (RUN4);
    text_paragraph_append_run (para2, run4);
    text_frame_append_block (frame, TEXT_BLOCK (para2));

    para3 = text_paragraph_new ();
    run5 = text_run_new (RUN5);
    text_paragraph_append_run (para3, run5);
    text_frame_append_block (frame, TEXT_BLOCK (para3));

    fixture->doc = text_document_new ();
    fixture->doc->frame = frame;

    fixture->editor = text_editor_new (fixture->doc);

    text_editor_move_first (fixture->editor, TEXT_EDITOR_CURSOR);

    fixture->run1 = run1;
    fixture->run2 = run2;
    fixture->run3 = run3;
    fixture->run4 = run4;
    fixture->run5 = run5;
    fixture->para1 = para1;
    fixture->para2 = para2;
    fixture->para3 = para3;
}

static void
split_fixture_tear_down (SplitFixture *fixture,
                           gconstpointer   user_data)
{
    g_object_unref (fixture->editor);
    g_object_unref (fixture->doc);
}

static void
test_end_of_paragraph (SplitFixture  *fixture,
                       gconstpointer  user_data)
{
    gchar *text;
    int length;
    TextParagraph *new;

    text_editor_move_right (fixture->editor, TEXT_EDITOR_CURSOR, 30);
    text_editor_split (fixture->editor, TEXT_EDITOR_CURSOR);

    // before:
    //     abcdefghij1234567890!@#$%^&*()
    //     zxcvbnm,./
    //     0987654321
    // after:
    //     abcdefghij1234567890!@#$%^&*()
    //     <empty run>                      <-- new paragraph
    //     zxcvbnm,./
    //     0987654321

    new = TEXT_PARAGRAPH (text_node_get_next (TEXT_NODE (fixture->para1)));

    // check paragraph contents
    text = text_paragraph_get_text (fixture->para1);
    g_assert_cmpstr (text, ==, "abcdefghij1234567890!@#$%^&*()");
    g_free (text);

    text = text_paragraph_get_text (new);
    g_assert_cmpstr (text, ==, "");
    g_free (text);

    text = text_paragraph_get_text (fixture->para2);
    g_assert_cmpstr (text, ==, "zxcvbnm,./");
    g_free (text);

    // check new length
    length = text_paragraph_get_length (new);
    g_assert_cmpint (length, ==, 0);

    // assert cursor is on new paragraph
    g_assert_cmpint (fixture->doc->cursor->index, ==, 0);
    g_assert_true (fixture->doc->cursor->paragraph == new);
}

static void
test_start_of_paragraph (SplitFixture  *fixture,
                         gconstpointer  user_data)
{
    gchar *text;
    int length;
    TextParagraph *new;

    text_editor_move_right (fixture->editor, TEXT_EDITOR_CURSOR, 31);
    text_editor_split (fixture->editor, TEXT_EDITOR_CURSOR);

    // before:
    //     abcdefghij1234567890!@#$%^&*()
    //     zxcvbnm,./
    //     0987654321
    // after:
    //     abcdefghij1234567890!@#$%^&*()
    //     <empty run>                      <-- modified paragraph two
    //     zxcvbnm,./                       <-- new paragraph
    //     0987654321

    // check paragraph contents
    text = text_paragraph_get_text (fixture->para1);
    g_assert_cmpstr (text, ==, "abcdefghij1234567890!@#$%^&*()");
    g_free (text);

    // check that paragraph two is now empty
    text = text_paragraph_get_text (fixture->para2);
    g_assert_cmpstr (text, ==, "");
    g_free (text);

    // check paragraph two length
    length = text_paragraph_get_length (fixture->para2);
    g_assert_cmpint (length, ==, 0);

    // get newly-inserted paragraph
    new = TEXT_PARAGRAPH (text_node_get_next (TEXT_NODE (fixture->para2)));

    // check contents
    text = text_paragraph_get_text (new);
    g_assert_cmpstr (text, ==, "zxcvbnm,./");
    g_free (text);

    // check length
    length = text_paragraph_get_length (new);
    g_assert_cmpint (length, ==, 10);

    // assert cursor is at start of new paragraph
    g_assert_cmpint (fixture->doc->cursor->index, ==, 0);
    g_assert_true (fixture->doc->cursor->paragraph == new);
}

static void
test_middle_of_paragraph (SplitFixture  *fixture,
                          gconstpointer  user_data)
{
    gchar *text;
    int length;
    TextParagraph *new;

    text_editor_move_right (fixture->editor, TEXT_EDITOR_CURSOR, 15);
    text_editor_split (fixture->editor, TEXT_EDITOR_CURSOR);

    // before:
    //     abcdefghij1234567890!@#$%^&*()
    //     zxcvbnm,./
    //     0987654321
    // after:
    //     abcdefghij12345                  <-- modified paragraph one
    //     67890!@#$%^&*()                  <-- new paragraph
    //     zxcvbnm,./
    //     0987654321

    new = TEXT_PARAGRAPH (text_node_get_next (TEXT_NODE (fixture->para1)));

    // check paragraph contents
    text = text_paragraph_get_text (fixture->para1);
    g_assert_cmpstr (text, ==, "abcdefghij12345");
    g_free (text);

    text = text_paragraph_get_text (new);
    g_assert_cmpstr (text, ==, "67890!@#$%^&*()");
    g_free (text);

    text = text_paragraph_get_text (fixture->para2);
    g_assert_cmpstr (text, ==, "zxcvbnm,./");
    g_free (text);

    // check lengths
    length = text_paragraph_get_length (fixture->para1);
    g_assert_cmpint (length, ==, 15);

    length = text_paragraph_get_length (new);
    g_assert_cmpint (length, ==, 15);

    // assert cursor is at start of new paragraph
    g_assert_cmpint (fixture->doc->cursor->index, ==, 0);
    g_assert_true (fixture->doc->cursor->paragraph == new);
}

int
main (int argc, char *argv[])
{
    setlocale (LC_ALL, "");

    g_test_init (&argc, &argv, NULL);

    // Define the tests.
    g_test_add ("/text-engine/editor/split/test-end-of-paragraph", SplitFixture, NULL,
                split_fixture_set_up, test_end_of_paragraph,
                split_fixture_tear_down);
    g_test_add ("/text-engine/editor/split/test-start-of-paragraph", SplitFixture, NULL,
                split_fixture_set_up, test_start_of_paragraph,
                split_fixture_tear_down);
    g_test_add ("/text-engine/editor/split/test-middle-of-paragraph", SplitFixture, NULL,
                split_fixture_set_up, test_middle_of_paragraph,
                split_fixture_tear_down);

    return g_test_run ();
}

