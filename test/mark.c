/* mark.c
 *
 * Copyright 2022 Matthew Jakeman <mjakeman26@outlook.co.nz>
 *
 * This file is dual-licensed under the terms of the Mozilla Public
 * License 2.0 and the Lesser General Public License 2.1 (or any
 * later version).
 *
 * SPDX-License-Identifier: MPL-2.0 OR LGPL-2.1-or-later
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
} MarkFixture;

#define RUN1 "abcdefghij"
#define RUN2 "1234567890"
#define RUN3 "!@#$%^&*()"
#define RUN4 "zxcvbnm,./"
#define RUN5 "0987654321"

static void
mark_fixture_set_up (MarkFixture   *fixture,
                     gconstpointer  user_data)
{
    TextFrame *frame;
    TextParagraph *para1, *para2, *para3;
    TextRun *run1, *run2, *run3, *run4, *run5;

    frame = text_frame_new ();

    para1 = text_paragraph_new ();
    run1 = text_run_new (RUN1);
    run2 = text_run_new (RUN2);
    run3 = text_run_new (RUN3);
    text_paragraph_append_fragment(para1, TEXT_FRAGMENT (run1));
    text_paragraph_append_fragment(para1, TEXT_FRAGMENT (run2));
    text_paragraph_append_fragment(para1, TEXT_FRAGMENT (run3));
    text_frame_append_block (frame, TEXT_BLOCK (para1));

    para2 = text_paragraph_new ();
    run4 = text_run_new (RUN4);
    text_paragraph_append_fragment(para2, TEXT_FRAGMENT (run4));
    text_frame_append_block (frame, TEXT_BLOCK (para2));

    para3 = text_paragraph_new ();
    run5 = text_run_new (RUN5);
    text_paragraph_append_fragment(para3, TEXT_FRAGMENT (run5));
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
mark_fixture_tear_down (MarkFixture   *fixture,
                        gconstpointer  user_data)
{
    g_object_unref (fixture->editor);
    g_object_unref (fixture->doc);
}

static void
test_delete_single_within (MarkFixture   *fixture,
                           gconstpointer  user_data)
{
    TextMark *mark;
    TextMark *cursor;
    TextGravity gravity;

    gravity = (TextGravity)user_data;
    cursor = fixture->doc->cursor;

    // Create mark
    mark = text_document_create_mark (fixture->doc, fixture->para1, 15, gravity);

    // Perform deletion
    text_editor_move_right (fixture->editor, TEXT_EDITOR_CURSOR, 10);
    text_editor_delete (fixture->editor, TEXT_EDITOR_CURSOR, 5);

    // before:
    //       cursor ><   >< mark
    //     abcdefghij1234567890!@#$%^&*()
    // after:
    //       cursor >< mark
    //     abcdefghij67890!@#$%^&*()

    g_assert_cmpint (mark->index, ==, 10);
    g_assert_true (mark->paragraph == fixture->para1);

    g_assert_cmpint (cursor->index, ==, 10);
    g_assert_true (cursor->paragraph == fixture->para1);
}

static void
test_delete_single_after (MarkFixture   *fixture,
                          gconstpointer  user_data)
{
    TextMark *mark;
    TextMark *cursor;
    TextGravity gravity;

    gravity = (TextGravity)user_data;
    cursor = fixture->doc->cursor;

    // Create mark
    mark = text_document_create_mark (fixture->doc, fixture->para1, 18, gravity);

    // Perform deletion
    text_editor_move_right (fixture->editor, TEXT_EDITOR_CURSOR, 10);
    text_editor_delete (fixture->editor, TEXT_EDITOR_CURSOR, 5);

    // before:
    //       cursor ><      >< mark
    //     abcdefghij1234567890!@#$%^&*()
    // after:
    //       cursor >< >< mark
    //     abcdefghij67890!@#$%^&*()

    g_assert_cmpint (mark->index, ==, 13);
    g_assert_true (mark->paragraph == fixture->para1);

    g_assert_cmpint (cursor->index, ==, 10);
    g_assert_true (cursor->paragraph == fixture->para1);
}

static void
test_delete_multi_start (MarkFixture   *fixture,
                         gconstpointer  user_data)
{
    TextMark *mark;
    TextMark *cursor;
    TextGravity gravity;

    gravity = (TextGravity)user_data;
    cursor = fixture->doc->cursor;

    // Create mark
    mark = text_document_create_mark (fixture->doc, fixture->para1, 18, gravity);

    // Perform deletion
    text_editor_move_right (fixture->editor, TEXT_EDITOR_CURSOR, 15);
    text_editor_delete (fixture->editor, TEXT_EDITOR_CURSOR, 32);

    // before:
    //            cursor >< >< mark
    //     abcdefghij1234567890!@#$%^&*()
    //     zxcvbnm,./
    //     0987654321
    // after:
    //            cursor >< mark
    //     abcdefghij1234554321

    g_assert_cmpint (mark->index, ==, 15);
    g_assert_true (mark->paragraph == fixture->para1);

    g_assert_cmpint (cursor->index, ==, 15);
    g_assert_true (cursor->paragraph == fixture->para1);
}

static void
test_delete_multi_inbetween (MarkFixture   *fixture,
                             gconstpointer  user_data)
{
    TextMark *mark;
    TextMark *cursor;
    TextGravity gravity;

    gravity = (TextGravity)user_data;
    cursor = fixture->doc->cursor;

    // Create mark
    mark = text_document_create_mark (fixture->doc, fixture->para2, 4, gravity);

    // Perform deletion
    text_editor_move_right (fixture->editor, TEXT_EDITOR_CURSOR, 15);
    text_editor_delete (fixture->editor, TEXT_EDITOR_CURSOR, 32);

    // before:
    //            cursor ><
    //     abcdefghij1234567890!@#$%^&*()
    //        >< mark
    //     zxcvbnm,./
    //     0987654321
    // after:
    //            cursor >< mark
    //     abcdefghij1234554321

    g_assert_cmpint (mark->index, ==, 15);
    g_assert_true (mark->paragraph == fixture->para1);

    g_assert_cmpint (cursor->index, ==, 15);
    g_assert_true (cursor->paragraph == fixture->para1);
}

static void
test_delete_multi_end (MarkFixture   *fixture,
                       gconstpointer  user_data)
{
    TextMark *mark;
    TextMark *cursor;
    TextGravity gravity;

    gravity = (TextGravity)user_data;
    cursor = fixture->doc->cursor;

    // Create mark
    mark = text_document_create_mark (fixture->doc, fixture->para3, 2, gravity);

    // Perform deletion
    text_editor_move_right (fixture->editor, TEXT_EDITOR_CURSOR, 15);
    text_editor_delete (fixture->editor, TEXT_EDITOR_CURSOR, 32);

    // before:
    //            cursor ><
    //     abcdefghij1234567890!@#$%^&*()
    //     zxcvbnm,./
    //      >< mark
    //     0987654321
    // after:
    //            cursor >< mark
    //     abcdefghij1234554321

    g_assert_cmpint (mark->index, ==, 15);
    g_assert_true (mark->paragraph == fixture->para1);

    g_assert_cmpint (cursor->index, ==, 15);
    g_assert_true (cursor->paragraph == fixture->para1);
}

static void
test_delete_multi_after (MarkFixture   *fixture,
                         gconstpointer  user_data)
{
    TextMark *mark;
    TextMark *cursor;
    TextGravity gravity;

    gravity = (TextGravity)user_data;
    cursor = fixture->doc->cursor;

    // Create mark
    mark = text_document_create_mark (fixture->doc, fixture->para3, 9, gravity);

    // Perform deletion
    text_editor_move_right (fixture->editor, TEXT_EDITOR_CURSOR, 15);
    text_editor_delete (fixture->editor, TEXT_EDITOR_CURSOR, 32);

    // before:
    //            cursor ><
    //     abcdefghij1234567890!@#$%^&*()
    //     zxcvbnm,./
    //             >< mark
    //     0987654321
    // after:
    //            cursor ><  >< mark
    //     abcdefghij1234554321

    g_assert_cmpint (mark->index, ==, 19);
    g_assert_true (mark->paragraph == fixture->para1);

    g_assert_cmpint (cursor->index, ==, 15);
    g_assert_true (cursor->paragraph == fixture->para1);
}

static void
test_insert_on (MarkFixture   *fixture,
                gconstpointer  user_data)
{
    TextMark *mark;
    TextMark *cursor;

    cursor = fixture->doc->cursor;

    // Create mark
    mark = text_document_create_mark (fixture->doc, fixture->para1, 9, TEXT_GRAVITY_LEFT);

    // Perform insertion
    text_editor_move_right (fixture->editor, TEXT_EDITOR_CURSOR, 9);
    text_editor_insert_text(fixture->editor, TEXT_EDITOR_CURSOR, "Hello");

    // before:
    //      cursor >< mark
    //     abcdefghij1234567890!@#$%^&*()
    // after:
    //      cursor >< mark
    //     abcdefghiHelloj1234567890!@#$%^&*()

    // mark - left gravity
    g_assert_cmpint (mark->index, ==, 9);
    g_assert_true (mark->paragraph == fixture->para1);

    // cursor - right gravity
    g_assert_cmpint (cursor->index, ==, 14);
    g_assert_true (cursor->paragraph == fixture->para1);
}

static void
test_insert_after (MarkFixture   *fixture,
                   gconstpointer  user_data)
{
    TextMark *mark;
    TextMark *cursor;
    TextGravity gravity;

    gravity = (TextGravity)user_data;
    cursor = fixture->doc->cursor;

    // Create mark
    mark = text_document_create_mark (fixture->doc, fixture->para1, 17, gravity);

    // Perform insertion
    text_editor_move_right (fixture->editor, TEXT_EDITOR_CURSOR, 9);
    text_editor_insert_text(fixture->editor, TEXT_EDITOR_CURSOR, "Hello");

    // before:
    //      cursor ><      >< mark
    //     abcdefghij1234567890!@#$%^&*()
    // after:
    //      cursor ><           >< mark
    //     abcdefghiHelloj1234567890!@#$%^&*()

    g_assert_cmpint (mark->index, ==, 22);
    g_assert_true (mark->paragraph == fixture->para1);

    g_assert_cmpint (cursor->index, ==, 14);
    g_assert_true (cursor->paragraph == fixture->para1);
}

static void
test_split_on (MarkFixture   *fixture,
               gconstpointer  user_data)
{
    TextMark *mark;
    TextMark *cursor;
    TextParagraph *new;

    cursor = fixture->doc->cursor;

    // Create mark
    mark = text_document_create_mark (fixture->doc, fixture->para1, 9, TEXT_GRAVITY_LEFT);

    // Perform split
    text_editor_move_right (fixture->editor, TEXT_EDITOR_CURSOR, 9);
    text_editor_split (fixture->editor, TEXT_EDITOR_CURSOR);

    // before:
    //      cursor >< mark
    //     abcdefghij1234567890!@#$%^&*()
    // after:
    //             >< mark
    //     abcdefghi
    //    >< cursor
    //     j1234567890!@#$%^&*()

    new = TEXT_PARAGRAPH (text_node_get_next (TEXT_NODE (fixture->para1)));

    // mark - left gravity
    g_assert_cmpint (mark->index, ==, 9);
    g_assert_true (mark->paragraph == fixture->para1);

    // cursor - right gravity
    g_assert_cmpint (cursor->index, ==, 0);
    g_assert_true (cursor->paragraph == new);
}

static void
test_split_after (MarkFixture   *fixture,
                  gconstpointer  user_data)
{
    TextMark *mark;
    TextMark *cursor;
    TextParagraph *new;

    cursor = fixture->doc->cursor;

    // Create mark
    mark = text_document_create_mark (fixture->doc, fixture->para1, 24, TEXT_GRAVITY_LEFT);

    // Perform split
    text_editor_move_right (fixture->editor, TEXT_EDITOR_CURSOR, 9);
    text_editor_split (fixture->editor, TEXT_EDITOR_CURSOR);

    // before:
    //      cursor ><             >< mark
    //     abcdefghij1234567890!@#$%^&*()
    // after:
    //     abcdefghi
    //    >< cursor      >< mark
    //     j1234567890!@#$%^&*()

    new = TEXT_PARAGRAPH (text_node_get_next (TEXT_NODE (fixture->para1)));

    // mark - left gravity
    g_assert_cmpint (mark->index, ==, 15);
    g_assert_true (mark->paragraph == new);

    // cursor - right gravity
    g_assert_cmpint (cursor->index, ==, 0);
    g_assert_true (cursor->paragraph == new);
}

int
main (int argc, char *argv[])
{
    setlocale (LC_ALL, "");

    g_test_init (&argc, &argv, NULL);

    // Delete tests
    g_test_add ("/text-engine/editor/mark/test-delete-single-within-gravity-left", MarkFixture, (gconstpointer) TEXT_GRAVITY_LEFT,
                mark_fixture_set_up, test_delete_single_within,
                mark_fixture_tear_down);
    g_test_add ("/text-engine/editor/mark/test-delete-single-within-gravity-right", MarkFixture, (gconstpointer) TEXT_GRAVITY_RIGHT,
                mark_fixture_set_up, test_delete_single_within,
                mark_fixture_tear_down);
    g_test_add ("/text-engine/editor/mark/test-delete-single-after-gravity-left", MarkFixture, (gconstpointer) TEXT_GRAVITY_LEFT,
                mark_fixture_set_up, test_delete_single_after,
                mark_fixture_tear_down);
    g_test_add ("/text-engine/editor/mark/test-delete-single-after-gravity-right", MarkFixture, (gconstpointer) TEXT_GRAVITY_RIGHT,
                mark_fixture_set_up, test_delete_single_after,
                mark_fixture_tear_down);

    g_test_add ("/text-engine/editor/mark/test-delete-multi-start-gravity-left", MarkFixture, (gconstpointer) TEXT_GRAVITY_LEFT,
                mark_fixture_set_up, test_delete_multi_start,
                mark_fixture_tear_down);
    g_test_add ("/text-engine/editor/mark/test-delete-multi-start-gravity-right", MarkFixture, (gconstpointer) TEXT_GRAVITY_RIGHT,
                mark_fixture_set_up, test_delete_multi_start,
                mark_fixture_tear_down);
    g_test_add ("/text-engine/editor/mark/test-delete-multi-inbetween-gravity-left", MarkFixture, (gconstpointer) TEXT_GRAVITY_LEFT,
                mark_fixture_set_up, test_delete_multi_inbetween,
                mark_fixture_tear_down);
    g_test_add ("/text-engine/editor/mark/test-delete-multi-inbetween-gravity-right", MarkFixture, (gconstpointer) TEXT_GRAVITY_RIGHT,
                mark_fixture_set_up, test_delete_multi_inbetween,
                mark_fixture_tear_down);
    g_test_add ("/text-engine/editor/mark/test-delete-multi-end-gravity-left", MarkFixture, (gconstpointer) TEXT_GRAVITY_LEFT,
                mark_fixture_set_up, test_delete_multi_end,
                mark_fixture_tear_down);
    g_test_add ("/text-engine/editor/mark/test-delete-multi-end-gravity-right", MarkFixture, (gconstpointer) TEXT_GRAVITY_RIGHT,
                mark_fixture_set_up, test_delete_multi_end,
                mark_fixture_tear_down);
    g_test_add ("/text-engine/editor/mark/test-delete-multi-after-gravity-left", MarkFixture, (gconstpointer) TEXT_GRAVITY_LEFT,
                mark_fixture_set_up, test_delete_multi_after,
                mark_fixture_tear_down);
    g_test_add ("/text-engine/editor/mark/test-delete-multi-after-gravity-right", MarkFixture, (gconstpointer) TEXT_GRAVITY_RIGHT,
                mark_fixture_set_up, test_delete_multi_after,
                mark_fixture_tear_down);

    // Insert tests
    g_test_add ("/text-engine/editor/mark/test-insert-on", MarkFixture, NULL,
                mark_fixture_set_up, test_insert_on,
                mark_fixture_tear_down);
    g_test_add ("/text-engine/editor/mark/test-insert-after-gravity-left", MarkFixture, (gconstpointer) TEXT_GRAVITY_LEFT,
                mark_fixture_set_up, test_insert_after,
                mark_fixture_tear_down);
    g_test_add ("/text-engine/editor/mark/test-insert-after-gravity-right", MarkFixture, (gconstpointer) TEXT_GRAVITY_RIGHT,
                mark_fixture_set_up, test_insert_after,
                mark_fixture_tear_down);

    // Replace tests
    // A replacement is simply a deletion followed
    // by an insertion, so covered by the above tests

    // Split tests
    g_test_add ("/text-engine/editor/mark/test-split-on", MarkFixture, NULL,
                mark_fixture_set_up, test_split_on,
                mark_fixture_tear_down);
    g_test_add ("/text-engine/editor/mark/test-split-after", MarkFixture, NULL,
                mark_fixture_set_up, test_split_after,
                mark_fixture_tear_down);

    return g_test_run ();
}

