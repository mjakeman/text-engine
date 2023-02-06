/* lib.rs
 *
 * Copyright 2023 Matthew Jakeman <mjakeman26@outlook.co.nz>
 *
 * This file is dual-licensed under the terms of the Mozilla Public
 * License 2.0 and the Lesser General Public License 2.1 (or any
 * later version).
 *
 * SPDX-License-Identifier: MPL-2.0 OR LGPL-2.1-or-later
 */

mod model;
mod cursor;
mod layout;

pub use crate::model::Document;
pub use crate::layout::{LayoutManager, DefaultLayoutManager, RenderCommand};

#[no_mangle]
pub extern "C" fn init() {
    let mut document = Document::new(Some("Hell🌍 World"));

    // let text = document.get_all_text();
    // println!("Version 0: {}", text);

    /*document.insert(15, ", again!");

    let text = document.get_all_text();
    println!("Version 1: {}", text);

    document.insert(23, " (no really!)");

    let text = document.get_all_text();
    println!("Version 2: {}", text);

    document.insert(8, " to the entire");

    let text = document.get_all_text();
    println!("Version 3: {}", text);*/
}

pub fn get_sample_document() -> Document {
    let mut document = Document::new(Some("The cat is white. 猫は白です。\"Meow\"「ニャア」"));
    /*document.insert(15, ", again!");
    document.insert(23, " (no really!)");
    document.insert(8, " to the entire");*/
    document
}