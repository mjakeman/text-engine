/* cursor.rs
 *
 * Copyright 2023 Matthew Jakeman <mjakeman26@outlook.co.nz>
 *
 * This file is dual-licensed under the terms of the Mozilla Public
 * License 2.0 and the Lesser General Public License 2.1 (or any
 * later version).
 *
 * SPDX-License-Identifier: MPL-2.0 OR LGPL-2.1-or-later
 */

use crate::model::Document;

struct Cursor<'a> {
    document: &'a Document,
    index: usize
}

enum Amount {
    Character,
    Word,
    Sentence,
    Paragraph
}

impl Cursor<'_> {
    fn new(document: &Document) -> Cursor {
        Cursor {
            document,
            index: 0
        }
    }

    fn move_forward(&self, quantity: usize, amount: Amount) {
        match amount {
            Amount::Character => {
                let index = self.index;
                let mut travelled = 0;

                loop {
                    /*if let Some(run) = self.document.get_run_at_index(index) {

                        let len = self.document.get_text(run).chars().count();
                        if travelled + len > quantity {
                            // within
                        }

                    } else { break }*/
                }
            }
            Amount::Word => {}
            Amount::Sentence => {}
            Amount::Paragraph => {}
        }
    }
}